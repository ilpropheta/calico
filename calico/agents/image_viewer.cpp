#include "image_viewer.h"
#include "../gui_handling.h"
#include <opencv2/highgui/highgui.hpp>

using namespace std::chrono_literals;

calico::agents::image_viewer::image_viewer(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::agents::image_viewer::so_define_agent()
{
	st_handling_images
		.event(m_channel, [this](const cv::Mat& image) {
			imshow(m_title, image);
			cv::waitKey(25);
			st_handling_images.time_limit(500ms, st_stream_down);
		})
		.on_exit([this] { cv::destroyWindow(m_title); });

	st_stream_down
		.transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_stream_down.activate();
}

calico::agents::maint_gui::image_viewer::image_viewer(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mchain_t ui_queue)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_message_queue(std::move(ui_queue))
{
}

void calico::agents::maint_gui::image_viewer::so_define_agent()
{
	st_handling_images
		.event(m_channel, [this](cv::Mat image) {
			so_5::send<gui_messages::imshow_message>(m_message_queue, m_title, std::move(image));
			st_handling_images.time_limit(500ms, st_stream_down);
		}).on_exit([this] { so_5::send<gui_messages::close_window_message>(m_message_queue, m_title); });
	st_stream_down
		.transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_stream_down.activate();
}
