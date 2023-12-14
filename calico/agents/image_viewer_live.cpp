#include "image_viewer_live.h"
#include <opencv2/highgui.hpp>
#include "../gui_handling.h"

using namespace std::chrono_literals;

calico::agents::image_viewer_live::image_viewer_live(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::agents::image_viewer_live::so_define_agent()
{
	st_handling_images
		.event(m_channel, [this](const cv::Mat& image) {
			imshow(m_title, image);
			cv::waitKey(25);
			st_handling_images.time_limit(500ms, st_stream_down);
		});

	st_stream_down
		.on_enter([this] {
			so_5::send<call_waitkey>(*this);
		})
		.event([this](so_5::mhood_t<call_waitkey>) {
			cv::waitKey(25);
			so_5::send<call_waitkey>(*this);
		}).transfer_to_state<cv::Mat>(m_channel, st_handling_images);

	st_handling_images.activate();
}

calico::agents::maint_gui::image_viewer_live::image_viewer_live(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mchain_t ui_queue)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_message_queue(std::move(ui_queue))
{
}

void calico::agents::maint_gui::image_viewer_live::so_define_agent()
{
	so_subscribe(m_channel).event([this](cv::Mat mat) {
		so_5::send<gui_messages::imshow_message>(m_message_queue, m_title, std::move(mat));
	});
}
