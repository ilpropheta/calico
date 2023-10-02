#include "image_producer.h"

calico::producers::image_producer::image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, std::stop_token st)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_stop(std::move(st))
{
}

void calico::producers::image_producer::so_evt_start()
{
	cv::VideoCapture cap;
	if (!cap.open(0, cv::CAP_DSHOW))
	{
		throw std::runtime_error("Can't connect to the webcam");
	}
	cv::Mat image;
	while (!m_stop.stop_requested())
	{
		cap >> image;
		so_5::send<cv::Mat>(m_channel, std::move(image));
	}
}
