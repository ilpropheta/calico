#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <opencv2/imgcodecs.hpp>
#include "../calico/producers/virtual_image_producer.h"
#include "../calico/signals.h"

using namespace std::chrono_literals;

TEST(virtual_image_producer_tests, should_send_images_from_folder_after_receiving_start_acquisition_command)
{
	const so_5::wrapped_env_t sobjectizer;

	const auto fake_input = create_mchain(sobjectizer.environment());
	const auto commands = sobjectizer.environment().create_mbox();

	sobjectizer.environment().introduce_coop([&](so_5::coop_t& c) {
		c.make_agent<calico::producers::virtual_image_producer>(fake_input->as_mbox(), commands, R"(test_data/replay)");
	});

	// send start command
	so_5::send<calico::start_acquisition_command>(commands);

	// wait until 5 images are received
	std::vector<cv::Mat> actual;
	receive(from(fake_input).handle_n(5).empty_timeout(100ms), [&](cv::Mat img) {
		actual.push_back(std::move(img));
	});
	ASSERT_THAT(actual.size(), testing::Eq(5)) << "expected exactly 5 images";

	// images are strictly the same as the baselines (and are read in the same order)
	EXPECT_THAT(sum(actual[0] != cv::imread(R"(test_data/replay/1.jpg)")), testing::Eq(cv::Scalar(0, 0, 0, 0)));
	EXPECT_THAT(sum(actual[1] != cv::imread(R"(test_data/replay/2.jpg)")), testing::Eq(cv::Scalar(0, 0, 0, 0)));
	EXPECT_THAT(sum(actual[2] != cv::imread(R"(test_data/replay/3.jpg)")), testing::Eq(cv::Scalar(0, 0, 0, 0)));

	// next images are just sent cyclically
	EXPECT_THAT(sum(actual[0] != actual[3]), testing::Eq(cv::Scalar(0, 0, 0, 0)));
	EXPECT_THAT(sum(actual[1] != actual[4]), testing::Eq(cv::Scalar(0, 0, 0, 0)));
}

TEST(virtual_image_producer_tests, when_folder_is_nonexistent_should_throw_exception_after_startup)
{
	const so_5::wrapped_env_t sobjectizer;

	std::atomic cooperation_deregistered_reason = 0;
	
	sobjectizer.environment().introduce_coop([&](so_5::coop_t& c) {
		c.set_exception_reaction(so_5::deregister_coop_on_exception);
		c.make_agent<calico::producers::virtual_image_producer>(sobjectizer.environment().create_mbox(), sobjectizer.environment().create_mbox(), R"(C:/geppo)");

		c.add_dereg_notificator([&](so_5::environment_t&, const so_5::coop_handle_t&, const so_5::coop_dereg_reason_t& why) noexcept {
			cooperation_deregistered_reason = why.reason();
			cooperation_deregistered_reason.notify_one();
			});
		});

	cooperation_deregistered_reason.wait(cooperation_deregistered_reason.load());
	EXPECT_THAT(cooperation_deregistered_reason, testing::Eq(so_5::dereg_reason::unhandled_exception));
}