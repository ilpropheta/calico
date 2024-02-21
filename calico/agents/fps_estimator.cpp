#include "fps_estimator.h"
#include <syncstream>
#include <format>
#include <ranges>
#include <opencv2/core/mat.hpp>

using namespace std::chrono_literals;

calico::agents::fps_estimator::fps_estimator(so_5::agent_context_t ctx, std::vector<so_5::mbox_t> inputs)
	: agent_t(std::move(ctx)), m_inputs(move(inputs))
{
}

void calico::agents::fps_estimator::so_define_agent()
{
	for (const auto& input : m_inputs)
	{
		st_handling_images.event(input, [channel_name = input->query_name(), this](const cv::Mat&) {
			++m_counters[channel_name];
			st_handling_images.time_limit(500ms, st_stream_down);
		});

		st_stream_down
			.transfer_to_state<cv::Mat>(input, st_handling_images);
	}

	st_handling_images.on_enter([this] {
      for (auto& counter : m_counters | std::views::values)
          counter = 0;
      m_timer = send_periodic<measure_fps>(so_direct_mbox(), 5s, 5s);
      m_start = std::chrono::steady_clock::now();
	})
	.on_exit([this] {
      m_timer.release();
	}).event([this](mhood_t<measure_fps>) {
      const auto elapsed_time = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_start);
      m_start = std::chrono::steady_clock::now();
      for (auto& [id, counter] : m_counters)
      {
          const auto fps = counter / elapsed_time.count();
          std::osyncstream(std::cout) << format("Estimated fps @{}=~{:.2f} ({} frames in ~{})\n", id, fps, counter, std::chrono::round<std::chrono::seconds>(elapsed_time));
          counter = 0;
      }
	});

	st_stream_down.activate();
}
