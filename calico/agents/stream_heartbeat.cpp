#include "stream_heartbeat.h"
#include "stream_detector.h"
#include <opencv2/core/mat.hpp>
#include <syncstream>

using namespace std::chrono_literals;

calico::agents::stream_heartbeat::stream_heartbeat(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx) + limit_then_drop<any_unspecified_message>(100)), m_channel(std::move(channel))
{
}

void calico::agents::stream_heartbeat::so_define_agent()
{
	st_handling_images
		.on_enter([this] {
			m_start_time = std::chrono::steady_clock::now();
			m_timer = so_5::send_periodic<log_heartbeat>(so_direct_mbox(), 5s, 5s);
		})
		.event(m_channel, [this](const cv::Mat&) {
			st_handling_images.time_limit(500ms, st_stream_down);
		}).event([this](so_5::mhood_t<log_heartbeat>) {
			std::osyncstream(std::cout) << std::format("[Heartbeat] Uptime: {:%H:%M:%S}\n", std::chrono::floor<std::chrono::seconds>(std::chrono::steady_clock::now() - m_start_time));
		}).on_exit([this] {
			m_timer.release();
		});

	st_stream_down
		.transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_stream_down.activate();
}

calico::agents::stream_heartbeat_with_detector::stream_heartbeat_with_detector(so_5::agent_context_t ctx, so_5::mbox_t detector_channel)
	: agent_t(std::move(ctx)), m_channel(std::move(detector_channel))
{
}

void calico::agents::stream_heartbeat_with_detector::so_define_agent()
{
	so_subscribe(m_channel).event([this](so_5::mhood_t<stream_detector::stream_up>) {
		m_start_time = std::chrono::steady_clock::now();
		m_timer = so_5::send_periodic<log_heartbeat>(so_direct_mbox(), 5s, 5s);
	}).event([this](so_5::mhood_t<stream_detector::stream_down>) {
		m_timer.release();
	});

	so_subscribe_self().event([this](so_5::mhood_t<log_heartbeat>) {
		std::osyncstream(std::cout) << std::format("[Heartbeat] Uptime: {:%H:%M:%S}\n", std::chrono::floor<std::chrono::seconds>(std::chrono::steady_clock::now() - m_start_time));
	});
}
