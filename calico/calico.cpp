#include "calico.h"
#include "utils.h"
#include "constants.h"
#include "gui_handling.h"
#include "agents/face_detector.h"
#include "agents/remote_control.h"
#include "agents/telemetry_agent.h"
#include "producers/image_producer_recursive.h"

int calico::run()
{
	const auto ctrl_c = utils::get_ctrlc_token();

	const so_5::wrapped_env_t sobjectizer;
	const auto main_channel = sobjectizer.environment().create_mbox("main");
	const auto commands_channel = sobjectizer.environment().create_mbox("commands");
	const auto message_queue = create_mchain(sobjectizer.environment());

	sobjectizer.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(sobjectizer.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<producers::image_producer_recursive>(main_channel, commands_channel);
		c.make_agent<agents::maint_gui::remote_control>(commands_channel, message_queue);

		c.make_agent_with_binder<agents::face_detector>(
			make_dispatcher(sobjectizer.environment(), "face_detector",
				so_5::disp::active_obj::disp_params_t{}.turn_work_thread_activity_tracking_on()).binder(),
			main_channel);

		c.make_agent<agents::telemetry_agent>();
	});

	do_gui_message_loop(ctrl_c, message_queue, sobjectizer.environment().create_mbox(constants::waitkey_channel_name));
	return 0;
}
