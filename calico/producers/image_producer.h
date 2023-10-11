#pragma once
#include <opencv2/opencv.hpp>
#include <so_5/all.hpp>
#include <stop_token>

namespace calico::producers
{
	// grabs frames from the default camera (e.g. webcam) and sends them to the specified message box
	class image_producer final : public so_5::agent_t
	{
	public:
		image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, std::stop_token st);
		void so_evt_start() override;
	private:
		so_5::mbox_t m_channel;
		std::stop_token m_stop;
	};
}