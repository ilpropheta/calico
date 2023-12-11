#include "utils.h"
#include "gui_handling.h"
#include "agents/image_viewer.h"
#include "agents/remote_control.h"
#include "producers/image_producer_recursive.h"

int main()
{
	const auto ctrl_c = calico::utils::get_ctrlc_token();

	const so_5::wrapped_env_t sobjectizer;
	const auto main_channel = sobjectizer.environment().create_mbox("main");
	const auto commands_channel = sobjectizer.environment().create_mbox("commands");
	const auto message_queue = create_mchain(sobjectizer.environment());

	sobjectizer.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(sobjectizer.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<calico::producers::image_producer_recursive>(main_channel, commands_channel);
		c.make_agent<calico::agents::remote_control>(commands_channel);
		c.make_agent<calico::agents::maint_gui::image_viewer>(main_channel, message_queue);
	});

	calico::do_gui_message_loop(ctrl_c, message_queue);
}
