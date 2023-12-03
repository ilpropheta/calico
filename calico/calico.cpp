#include "utils.h"
#include "agents/image_saver.h"
#include "agents/image_viewer_live.h"
#include "agents/remote_control.h"
#include "agents/image_resizer.h"
#include "producers/image_producer_recursive.h"

int main()
{
	const auto ctrl_c = calico::utils::get_ctrlc_token();

	const so_5::wrapped_env_t sobjectizer;
	const auto main_channel = sobjectizer.environment().create_mbox("main");
	auto commands_channel = sobjectizer.environment().create_mbox("commands");
	auto save_buffer = create_mchain(sobjectizer.environment());
	const auto chain_guard = auto_close_retain_content(save_buffer);

	sobjectizer.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(sobjectizer.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<calico::producers::image_producer_recursive>(main_channel, commands_channel);
		c.make_agent<calico::agents::remote_control>(commands_channel);
		c.make_agent<calico::agents::image_viewer_live>(main_channel);
		const auto resized = c.make_agent<calico::agents::image_resizer>(main_channel, 3.0)->output();

		const auto binding = c.take_under_control(std::make_unique<so_5::single_sink_binding_t>());
		binding->bind<cv::Mat>(resized, wrap_to_msink(save_buffer->as_mbox()));

		c.make_agent<calico::agents::image_save_worker>(save_buffer, "c:/images");
		c.make_agent<calico::agents::image_save_worker>(save_buffer, "c:/images");
	});

	calico::utils::wait_for_stop(ctrl_c);
}
