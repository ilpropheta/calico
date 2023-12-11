#pragma once
#include <string>
#include <opencv2/core.hpp>
#include <so_5/all.hpp>

namespace calico
{
	namespace gui_messages
	{
		// represents a cv::imshow request
		struct imshow_message
		{
			std::string window;
			cv::Mat image;
		};

		// represents a cv::destroyWindow request
		struct close_window_message
		{
			std::string window;
		};

		// represents the result of cv::waitKey
		struct waitkey_message
		{
			int pressed;
		};
	}

	// receives and handles gui_messages from 'message_queue', terminating when the stop_token is triggered
	void do_gui_message_loop(std::stop_token st, so_5::mchain_t message_queue, so_5::mbox_t waitkey_out);
}
