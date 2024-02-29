#include <gtest/gtest.h>
#include <format>
#include <future>
#include <opencv2/imgcodecs.hpp>
#include "frequency_calculator.h"
#include "service_time_estimator_dispatcher.h"
#include "../calico/agents/image_resizer.h"
#include "../calico/agents/face_detector.h"

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

TEST(performance_tests, face_detector_maximum_throughput)
{
	constexpr unsigned messages_count = 100;

	so_5::wrapped_env_t sobjectizer;
	auto& env = sobjectizer.environment();

	auto input_channel = env.create_mbox();
	const auto output_channel = create_mchain(env);

	env.introduce_coop([&](so_5::coop_t& coop) {
		coop.make_agent<calico::agents::face_detector>(input_channel, output_channel->as_mbox());
	});

	auto throughput = async(std::launch::async, frequency_calculator<cv::Mat>{messages_count, output_channel});

	const auto test_frame = cv::imread("test_data/replay/1.jpg");
	for (auto i = 0u; i < messages_count; ++i)
	{
		send<cv::Mat>(input_channel, test_frame);
	}

	std::cout << std::format("face_detector maximum throughput measured on {} frames = {:.2f} fps\n", messages_count, throughput.get());
}

TEST(performance_tests, image_resizer_service_time)
{
	constexpr unsigned messages_count = 1000;

	so_5::wrapped_env_t sobjectizer;
	auto& env = sobjectizer.environment();

	auto input_channel = env.create_mbox();
	auto output_channel = env.create_mbox();
	const auto measure_output = create_mchain(env);

	const auto measuring_dispatcher = service_time_estimator_dispatcher::make(env, measure_output->as_mbox(), messages_count);
	env.introduce_coop(measuring_dispatcher, [&](so_5::coop_t& coop) {
		coop.make_agent<calico::agents::image_resizer>(input_channel, output_channel, 4);
	});

	const auto test_frame = cv::imread("test_data/replay/1.jpg");
	for (auto i = 0u; i < messages_count; ++i)
	{
		so_5::send<cv::Mat>(input_channel, test_frame);
	}

	receive(from(measure_output).handle_n(1), [](double service_time_avg) {
		std::cout << std::format("image_resizer average service time calculated on {} frames is: {}s\n", messages_count, service_time_avg);
	});
}

TEST(performance_tests, face_detector_service_time)
{
	constexpr unsigned messages_count = 100;

	so_5::wrapped_env_t sobjectizer;
	auto& env = sobjectizer.environment();

	auto input_channel = env.create_mbox();
	auto output_channel = env.create_mbox();
	const auto measure_output = create_mchain(env);

	const auto measuring_dispatcher = service_time_estimator_dispatcher::make(env, measure_output->as_mbox(), messages_count);
	env.introduce_coop(measuring_dispatcher, [&](so_5::coop_t& coop) {
		coop.make_agent<calico::agents::face_detector>(input_channel, output_channel);
	});

	const auto test_frame = cv::imread("test_data/replay/1.jpg");
	for (auto i = 0u; i < messages_count; ++i)
	{
		so_5::send<cv::Mat>(input_channel, test_frame);
	}

	receive(from(measure_output).handle_n(1), [](double service_time_avg) {
		std::cout << std::format("face_detector average service time calculated on {} frames is: {}s\n", messages_count, service_time_avg);
	});
}

TEST(performance_tests, two_image_resizers_in_parallel)
{
	constexpr unsigned messages_count = 100;

	so_5::wrapped_env_t sobjectizer;
	auto& env = sobjectizer.environment();

	auto input_channel = env.create_mbox();
	auto output_channel = env.create_mbox();

	auto measure_output1 = create_mchain(env);
	auto measure_output2 = create_mchain(env);

	env.introduce_coop(service_time_estimator_dispatcher::make(env, measure_output1->as_mbox(), messages_count), [&](so_5::coop_t& coop) {
		coop.make_agent<calico::agents::image_resizer>(input_channel, output_channel, 4);
	});

	env.introduce_coop(service_time_estimator_dispatcher::make(env, measure_output2->as_mbox(), messages_count), [&](so_5::coop_t& coop) {
		coop.make_agent<calico::agents::image_resizer>(input_channel, output_channel, 4);
	});

	const auto test_frame = cv::imread("test_data/replay/1.jpg");
	for (auto i = 0u; i < messages_count; ++i)
	{
		so_5::send<cv::Mat>(input_channel, test_frame);
	}

	receive(from(measure_output1).handle_n(1), [](double service_time_avg) {
		std::cout << std::format("image_resizer(1) average service time calculated on {} frames is: {:.5f}s\n", messages_count, service_time_avg);
	});

	receive(from(measure_output2).handle_n(1), [](double service_time_avg) {
		std::cout << std::format("image_resizer(2) average service time calculated on {} frames is: {:.5f}s\n", messages_count, service_time_avg);
	});
}