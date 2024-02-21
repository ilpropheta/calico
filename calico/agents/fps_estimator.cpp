#include "fps_estimator.h"
#include <syncstream>
#include <format>
#include <opencv2/core/mat.hpp>

using namespace std::chrono_literals;

calico::agents::fps_estimator::fps_estimator(so_5::agent_context_t ctx, so_5::mbox_t input)
	: agent_t(std::move(ctx)), m_input(std::move(input))
{

}

void calico::agents::fps_estimator::so_evt_start()
{
	m_timer = so_5::send_periodic<measure_fps>(so_direct_mbox(), 5s, 5s);
	m_start = std::chrono::steady_clock::now();
}

void calico::agents::fps_estimator::so_define_agent()
{
	so_subscribe(m_input).event([this](const cv::Mat&) {
		++m_counter;
	});
	so_subscribe_self().event([this](mhood_t<measure_fps>) {
		const auto elapsed_time = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_start);
		const auto fps = m_counter / elapsed_time.count();
		std::osyncstream(std::cout) << format("Estimated fps @{}=~{:.2f} ({} frames in ~{})\n", m_input->query_name(), fps, m_counter, std::chrono::round<std::chrono::seconds>(elapsed_time));
		m_start = std::chrono::steady_clock::now();
		m_counter = 0;
	});
}
