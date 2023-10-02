#include "image_tracer.h"
#include <opencv2/core.hpp>
#include <syncstream>

calico::agents::image_tracer::image_tracer(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::agents::image_tracer::so_define_agent()
{
	so_subscribe(m_channel).event([](cv::Mat image) {
		std::osyncstream(std::cout) << "Got image: " << image.size() << "\n";
	});
}