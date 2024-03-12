#include "calico.h"
#include "utils.h"
#include "constants.h"
#include "gui_handling.h"
#include "agents/image_viewer.h"
#include "agents/remote_control.h"
#include "producers/image_producer_recursive.h"
#include "agents/static_pipeline.h"

int calico::run()
{
	const auto ctrl_c = utils::get_ctrlc_token();

	const so_5::wrapped_env_t sobjectizer;
	const auto main_channel = sobjectizer.environment().create_mbox("main");
	const auto commands_channel = sobjectizer.environment().create_mbox("commands");
	const auto message_queue = create_mchain(sobjectizer.environment());

	// setup of the static pipeline
	sobjectizer.environment().introduce_coop(so_5::disp::active_group::make_dispatcher(sobjectizer.environment()).binder("pipeline"), [&](so_5::coop_t& c) {
		auto step_3_dst = c.make_agent<agents::static_pipeline::step_3>()->so_direct_mbox();  // this sends data to "output" channel
		auto step_2_dst = c.make_agent<agents::static_pipeline::step_2>(step_3_dst)->so_direct_mbox();
		c.make_agent<agents::static_pipeline::step_1>(step_2_dst); // this gets data from "main" channel
	});

	sobjectizer.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(sobjectizer.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<producers::image_producer_recursive>(main_channel, commands_channel); // this sends data to "main" channel
		c.make_agent<agents::maint_gui::remote_control>(commands_channel, message_queue);
		c.make_agent<agents::maint_gui::image_viewer>(sobjectizer.environment().create_mbox("output"), message_queue);
	});

	do_gui_message_loop(ctrl_c, message_queue, sobjectizer.environment().create_mbox(constants::waitkey_channel_name));
	return 0;
}
