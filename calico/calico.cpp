#include "utils.h"
#include "agents/image_resizer.h"
#include "agents/image_viewer_live.h"
#include "agents/remote_control.h"
#include "agents/face_detector.h"
#include "agents/error_logger.h"
#include "producers/image_producer_recursive.h"

int main()
{
	const auto ctrl_c = calico::utils::get_ctrlc_token();

	const so_5::wrapped_env_t sobjectizer;
	const auto main_channel = sobjectizer.environment().create_mbox("main");
	auto commands_channel = sobjectizer.environment().create_mbox("commands");
	auto resized_images = sobjectizer.environment().create_mbox("resized");

	sobjectizer.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(sobjectizer.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<calico::producers::image_producer_recursive>(main_channel, commands_channel);
		c.make_agent<calico::agents::remote_control>(commands_channel);

		auto not_empty_images = sobjectizer.environment().create_mbox();
		const auto binding = c.take_under_control(std::make_unique<so_5::single_sink_binding_t>());
		binding->bind<cv::Mat>(main_channel, so_5::wrap_to_msink(not_empty_images), [](const cv::Mat& image) {
			return !image.empty();
		});

		c.make_agent<calico::agents::image_resizer>(not_empty_images, resized_images, 0.5);
		c.make_agent<calico::agents::image_viewer_live>(resized_images);

		c.make_agent<calico::agents::image_viewer_live>(not_empty_images);
	});

	calico::utils::wait_for_stop(ctrl_c);
}
