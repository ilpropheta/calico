#include "fps_estimator.h"
#include <syncstream>
#include <format>
#include <opencv2/core/mat.hpp>

using namespace std::chrono_literals;

calico::agents::fps_estimator::fps_estimator(so_5::agent_context_t ctx, std::vector<so_5::mbox_t> inputs)
	: agent_t(std::move(ctx)), m_inputs(std::move(inputs))
{
}

void calico::agents::fps_estimator::so_evt_start()
{
	m_timer = so_5::send_periodic<measure_fps>(so_direct_mbox(), 5s, 5s);
	m_start = std::chrono::steady_clock::now();
}

void calico::agents::fps_estimator::so_define_agent()
{
	for (const auto& input : m_inputs)
	{
		m_counters[input->query_name()] = 0;
		so_subscribe(input).event([channel_name = input->query_name(), this](const cv::Mat&) {
			++m_counters[channel_name];
		});
	}

	so_subscribe_self().event([this](so_5::mhood_t<measure_fps>) {
		const auto elapsed_time = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_start);
		for (auto& [id, counter] : m_counters)
		{
			const auto fps = counter / elapsed_time.count();
			std::osyncstream(std::cout) << std::format("Estimated fps @{}=~{:.2f} ({} frames in ~{})\n", id, fps, counter, std::chrono::round<std::chrono::seconds>(elapsed_time));
			counter = 0;
		}
		m_start = std::chrono::steady_clock::now();
	});
}
