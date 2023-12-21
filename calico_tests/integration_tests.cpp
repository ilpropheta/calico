#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../calico/producers/virtual_image_producer.h"
#include "../calico/agents/stream_detector.h"
#include "../calico/signals.h"

using namespace std::chrono_literals;

TEST(integration_tests, stream_detector_should_detect_stream_activity_when_virtual_image_producer_sends_images)
{
	const so_5::wrapped_env_t sobjectizer;
	const auto channel = sobjectizer.environment().create_mbox();
	const auto commands = sobjectizer.environment().create_mbox();
	const auto output = create_mchain(sobjectizer.environment());

	sobjectizer.environment().introduce_coop([&](so_5::coop_t& c) {
		c.make_agent<calico::producers::virtual_image_producer>(channel, commands, R"(test_data/replay)");
		c.make_agent<calico::agents::stream_detector>(channel, output->as_mbox());
	});

	so_5::send<calico::start_acquisition_command>(commands);

	bool stream_up_received = false;
	receive(from(output).handle_n(1).empty_timeout(100ms), [&](so_5::mhood_t<calico::agents::stream_detector::stream_up>) {
		stream_up_received = true;
	});
	EXPECT_THAT(stream_up_received, testing::IsTrue());

	so_5::send<calico::stop_acquisition_command>(commands);

	bool stream_down_received = false;
	receive(from(output).handle_n(1).empty_timeout(700ms), [&](so_5::mhood_t<calico::agents::stream_detector::stream_down>) {
		stream_down_received = true;
	});
	EXPECT_THAT(stream_down_received, testing::IsTrue());
}