#include "gui_handling.h"
#include <opencv2/highgui.hpp>

using namespace std::chrono_literals;

void calico::do_gui_message_loop(std::stop_token st, so_5::mchain_t message_queue, so_5::mbox_t waitkey_out)
{
	while (!st.stop_requested())
	{
		receive(from(message_queue).handle_n(1).empty_timeout(100ms),
			[](const gui_messages::imshow_message& mex) {
				imshow(mex.window, mex.image);
			},
			[](const gui_messages::close_window_message& mex) {
				cv::destroyWindow(mex.window);
			}
		);

		if (const auto key = cv::waitKey(1); key != -1)
		{
			so_5::send<gui_messages::waitkey_message>(waitkey_out, key);
		}
	}
}
