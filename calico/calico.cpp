#include "calico.h"
#include "utils.h"
#include "constants.h"
#include "gui_handling.h"
#include "agents/error_logger.h"
#include "agents/face_detector.h"
#include "agents/fps_estimator.h"
#include "agents/image_resizer.h"
#include "agents/image_tracer.h"
#include "agents/image_viewer.h"
#include "agents/remote_control.h"
#include "agents/service_facade.h"
#include "agents/stream_heartbeat.h"
#include "agents/telemetry_agent.h"
#include "producers/image_producer.h"

int calico::run()
{
	const auto ctrl_c = utils::get_ctrlc_token();

	const so_5::wrapped_env_t sobjectizer;
	const auto main_channel = sobjectizer.environment().create_mbox("main");
	const auto commands_channel = sobjectizer.environment().create_mbox("commands");
	const auto message_queue = create_mchain(sobjectizer.environment());
	const auto waitkey_out = sobjectizer.environment().create_mbox(constants::waitkey_channel_name);

	sobjectizer.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(sobjectizer.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<producers::image_producer>(main_channel, commands_channel);
		c.make_agent<agents::service_facade>();
	});

	sobjectizer.environment().introduce_coop(so_5::disp::active_group::make_dispatcher(sobjectizer.environment()).binder("monitoring"), [&](so_5::coop_t& c) {
		c.make_agent<agents::image_tracer>(main_channel);
		c.make_agent<agents::fps_estimator>(std::vector{ main_channel });
		c.make_agent<agents::telemetry_agent>();
		c.make_agent<agents::stream_heartbeat>(main_channel);
		c.make_agent<agents::error_logger>(main_channel);
	});

	sobjectizer.environment().introduce_coop(so_5::disp::active_group::make_dispatcher(sobjectizer.environment()).binder("pipeline"), [&](so_5::coop_t& c) {
		c.make_agent<agents::maint_gui::remote_control>(commands_channel, message_queue);
		const auto faces = c.make_agent<agents::face_detector>(c.make_agent<agents::image_resizer>(main_channel, 0.5)->output())->output();
		c.make_agent<agents::maint_gui::image_viewer>(faces, message_queue);
	});

	do_gui_message_loop(ctrl_c, message_queue, sobjectizer.environment().create_mbox(constants::waitkey_channel_name));
	return 0;
}
