#pragma once
#include <opencv2/videoio.hpp>
#include <so_5/all.hpp>

namespace calico::producers
{
	// grabs frames from the default camera (e.g. webcam) and sends them to the specified message box
	class image_producer_recursive final : public so_5::agent_t
	{
		struct grab_image final : so_5::signal_t {};
	public:
		image_producer_recursive(so_5::agent_context_t ctx, so_5::mbox_t channel);
		void so_define_agent() override;
		void so_evt_start() override;
	private:
		so_5::mbox_t m_channel;
		cv::VideoCapture m_capture;
	};
}