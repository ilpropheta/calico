#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <opencv2/core/mat.hpp>
#include "../calico/agents/stream_detector.h"

using namespace std::chrono_literals;

TEST(stream_detector_tests, should_send_stream_up_and_stream_down_properly)
{
	const so_5::wrapped_env_t sobjectizer;
	const auto input = sobjectizer.environment().create_mbox();
	const auto output = create_mchain(sobjectizer.environment());

	constexpr auto stream_end_timeout = 2ms;
	constexpr auto hanging_protection = 100ms;

	sobjectizer.environment().introduce_coop([&](so_5::coop_t& c) {
		c.make_agent<calico::agents::stream_detector>(input, output->as_mbox(), stream_end_timeout);
	});

	// ensure no messages arrive at this point...
	EXPECT_EQ(receive(from(output).handle_all().empty_timeout(hanging_protection)).extracted(), 0);

	// should detect new stream
	so_5::send<cv::Mat>(input, cv::Mat{});

	bool stream_up_received = false;
	receive(from(output).handle_n(1).empty_timeout(hanging_protection), [&](so_5::mhood_t<calico::agents::stream_detector::stream_up>) {
		stream_up_received = true;
	});
	EXPECT_THAT(stream_up_received, testing::IsTrue());

	// nothing should be sent because of these
	so_5::send<cv::Mat>(input, cv::Mat{});
	so_5::send<cv::Mat>(input, cv::Mat{});
	so_5::send<cv::Mat>(input, cv::Mat{});

	// stream down should arrive after some time of inactivity...
	bool stream_down_received = false;
	receive(from(output).handle_n(1).empty_timeout(hanging_protection), [&](so_5::mhood_t<calico::agents::stream_detector::stream_down>) {
		stream_down_received = true;
	});
	EXPECT_THAT(stream_down_received, testing::IsTrue());

	// ensure no messages arrive at this point...
	EXPECT_EQ(receive(from(output).handle_all().empty_timeout(hanging_protection)).extracted(), 0);
}