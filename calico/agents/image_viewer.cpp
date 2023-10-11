#include "image_viewer.h"
#include <opencv2/highgui/highgui.hpp>

calico::agents::image_viewer::image_viewer(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::agents::image_viewer::so_define_agent()
{
	so_subscribe(m_channel).event([](const cv::Mat& image) {
		imshow("Image Viewer", image);
		cv::waitKey(25);
	});
}
