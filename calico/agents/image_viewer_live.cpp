#include "image_viewer_live.h"
#include <opencv2/highgui.hpp>

using namespace std::chrono_literals;

calico::agents::image_viewer_live::image_viewer_live(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::agents::image_viewer_live::so_define_agent()
{
	st_handling_images
		.event(m_channel, [this](so_5::mhood_t<cv::Mat> image) {
			imshow(m_title, *image);
			cv::waitKey(25);
			st_handling_images.time_limit(500ms, st_stream_down);
		});

	st_stream_down
		.on_enter([this] {
			so_5::send<call_waitkey>(*this);
		})
		.event([this](so_5::mhood_t<call_waitkey>) {
			cv::waitKey(25);
			so_5::send<call_waitkey>(*this);
		}).transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_handling_images.activate();
}
