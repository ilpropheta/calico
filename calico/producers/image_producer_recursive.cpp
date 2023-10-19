#include "image_producer_recursive.h"

calico::producers::image_producer_recursive::image_producer_recursive(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_capture(0, cv::CAP_DSHOW)
{
}

void calico::producers::image_producer_recursive::so_define_agent()
{
	so_subscribe_self().event([this](so_5::mhood_t<grab_image>) {
		cv::Mat image;
		m_capture >> image;
		so_5::send<cv::Mat>(m_channel, std::move(image));
		so_5::send<grab_image>(*this);
	});
}

void calico::producers::image_producer_recursive::so_evt_start()
{
	if (!m_capture.isOpened())
	{
		throw std::runtime_error("Can't connect to the webcam");
	}

	so_5::send<grab_image>(*this);
}
