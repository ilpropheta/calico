#include "stream_detector.h"
#include <opencv2/core/mat.hpp>

using namespace std::chrono_literals;

calico::agents::stream_detector::stream_detector(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t output, std::chrono::milliseconds stream_end_timeout)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_out_channel(std::move(output)), m_stream_end_timeout(stream_end_timeout)
{
}

void calico::agents::stream_detector::so_define_agent()
{
	st_handling_images
		.on_enter([this] {
			so_5::send<stream_up>(m_out_channel);
		})
		.event(m_channel, [this](const cv::Mat&) {
			st_handling_images.time_limit(m_stream_end_timeout, st_stream_down);
		}).on_exit([this] {
			so_5::send<stream_down>(m_out_channel);
		});

	st_stream_down
		.transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_stream_down.activate();
}
