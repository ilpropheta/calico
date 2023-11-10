#include "stream_heartbeat.h"
#include <opencv2/core/mat.hpp>
#include <syncstream>

using namespace std::chrono_literals;

calico::agents::stream_heartbeat::stream_heartbeat(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::agents::stream_heartbeat::so_define_agent()
{
	st_handling_images
		.on_enter([this] {
			m_startTime = std::chrono::steady_clock::now();
			m_timer = so_5::send_periodic<log_heartbeat>(so_direct_mbox(), 5s, 5s);
		})
		.event(m_channel, [this](so_5::mhood_t<cv::Mat>) {
			st_handling_images.time_limit(500ms, st_stream_down);
		}).event([this](so_5::mhood_t<log_heartbeat>) {
			std::osyncstream(std::cout) << std::format("[Heartbeat] Uptime: {:%H:%M:%S}\n", std::chrono::floor<std::chrono::seconds>(std::chrono::steady_clock::now() - m_startTime));
		}).on_exit([this] {
			m_timer.release();
		});

	st_stream_down
		.transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_stream_down.activate();
}
