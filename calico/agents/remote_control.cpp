#include "remote_control.h"
#include "../signals.h"
#include "../constants.h"
#include "../gui_handling.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std::chrono_literals;

calico::agents::remote_control::remote_control(so_5::agent_context_t ctx, so_5::mbox_t commands)
	: agent_t(std::move(ctx)), m_channel(std::move(commands))
{
}

void calico::agents::remote_control::so_evt_start()
{
	cv::Mat frame = cv::Mat::ones(100, 200, CV_8UC3);
	putText(frame, "Start (Enter)", { 2, 20 }, cv::FONT_HERSHEY_COMPLEX_SMALL, 1, { 240, 200, 1 });
	putText(frame, "Stop (Escape)", { 2, 50 }, cv::FONT_HERSHEY_COMPLEX_SMALL, 1, { 240, 200, 1 });
	imshow("Remote Control", frame);
	so_5::send<keep_on>(*this);
}

void calico::agents::remote_control::so_define_agent()
{
	so_subscribe_self()
		.event([this](so_5::mhood_t<keep_on>) {
			switch (cv::waitKey(1))
			{
			case 13: // Enter
				so_5::send<start_acquisition_command>(m_channel);
				break;
			case 27: // Escape
				so_5::send<stop_acquisition_command>(m_channel);
				break;
			default:
				break;
			}
			so_5::send_delayed<keep_on>(*this, 100ms);
		});
}

calico::agents::maint_gui::remote_control::remote_control(so_5::agent_context_t ctx, so_5::mbox_t commands, so_5::mchain_t ui_queue)
	: agent_t(std::move(ctx)), m_channel(std::move(commands)), m_message_queue(std::move(ui_queue)), m_waitkey_channel(so_environment().create_mbox(constants::waitkey_channel_name))
{
}

void calico::agents::maint_gui::remote_control::so_evt_start()
{
	cv::Mat frame = cv::Mat::ones(100, 200, CV_8UC3);
	putText(frame, "Start (Enter)", { 2, 20 }, cv::FONT_HERSHEY_COMPLEX_SMALL, 1, { 240, 200, 1 });
	putText(frame, "Stop (Escape)", { 2, 50 }, cv::FONT_HERSHEY_COMPLEX_SMALL, 1, { 240, 200, 1 });
	so_5::send<gui_messages::imshow_message>(m_message_queue, "Remote control", std::move(frame));
}

void calico::agents::maint_gui::remote_control::so_define_agent()
{
	so_subscribe(m_waitkey_channel)
		.event([this](const gui_messages::waitkey_message& waitkey_result) {
			switch (waitkey_result.pressed)
			{
			case 13: // Enter
				so_5::send<start_acquisition_command>(m_channel);
				break;
			case 27: // Escape
				so_5::send<stop_acquisition_command>(m_channel);
				break;
			default:
				break;
			}
	});
}
