#include <gtest/gtest.h>
#include <format>
#include <future>
#include <opencv2/imgcodecs.hpp>
#include "frequency_calculator.h"
#include "../calico/agents/image_resizer.h"

TEST(performance_tests, image_resizer_maximum_throughput)
{
	constexpr unsigned messages_count = 100;

	so_5::wrapped_env_t sobjectizer;
	auto& env = sobjectizer.environment();

	auto input_channel = env.create_mbox();
	const auto output_channel = create_mchain(env);

	env.introduce_coop([&](so_5::coop_t& coop) {
		coop.make_agent<calico::agents::image_resizer>(input_channel, output_channel->as_mbox(), 4.0);
	});

	auto throughput = std::async(std::launch::async, frequency_calculator<cv::Mat>{messages_count, output_channel});

	const auto test_frame = cv::imread("test_data/replay/1.jpg");
	for (auto i = 0u; i < messages_count; ++i)
	{
		so_5::send<cv::Mat>(input_channel, test_frame);
	}

	std::cout << std::format("image_resizer maximum throughput measured on {} frames = {:.2f} fps\n", messages_count, throughput.get());
}