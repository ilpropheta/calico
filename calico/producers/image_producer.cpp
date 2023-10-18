#include "image_producer.h"

calico::producers::image_producer::image_producer_broker::image_producer_broker(so_5::agent_context_t c, std::stop_source stop_source)
	: agent_t(std::move(c)), m_stop_source(std::move(stop_source))
{
}

void calico::producers::image_producer::image_producer_broker::so_evt_finish()
{
	m_stop_source.request_stop();
}

calico::producers::image_producer::image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::producers::image_producer::so_evt_start()
{
	cv::VideoCapture cap;
	if (!cap.open(0, cv::CAP_DSHOW))
	{
		throw std::runtime_error("Can't connect to the webcam");
	}

	introduce_child_coop(*this, so_5::disp::active_obj::make_dispatcher(so_environment()).binder(), [this](so_5::coop_t& c) {
		c.make_agent<image_producer_broker>(m_stop_source);
	});

	cv::Mat image;
	const auto st = m_stop_source.get_token();
	while (!st.stop_requested())
	{
		cap >> image;
		so_5::send<cv::Mat>(m_channel, std::move(image));
	}
}
