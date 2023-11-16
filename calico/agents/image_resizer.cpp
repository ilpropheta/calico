#include "image_resizer.h"
#include <opencv2/imgproc.hpp>

calico::agents::image_resizer::image_resizer(so_5::agent_context_t ctx, so_5::mbox_t input, so_5::mbox_t output, double factor)
	: agent_t(std::move(ctx)), m_input(std::move(input)), m_output(std::move(output)), m_factor(factor)
{
}

void calico::agents::image_resizer::so_define_agent()
{
	so_subscribe(m_input).event([this](const cv::Mat& image) {
		cv::Mat resized;
		resize(image, resized, {}, m_factor, m_factor);
		so_5::send<cv::Mat>(m_output, std::move(resized));
	});
}
