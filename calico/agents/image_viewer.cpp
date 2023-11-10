#include "image_viewer.h"
#include <opencv2/highgui/highgui.hpp>

using namespace std::chrono_literals;

calico::agents::image_viewer::image_viewer(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::agents::image_viewer::so_define_agent()
{
	st_handling_images
		.event(m_channel, [this](const cv::Mat& image) {
			imshow(m_title, image);
			cv::waitKey(25);
			st_handling_images.time_limit(500ms, st_stream_down);
		})
		.on_exit([this] { cv::destroyWindow(m_title); });

	st_stream_down
		.transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_stream_down.activate();
}
