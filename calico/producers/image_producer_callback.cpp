#include "image_producer_callback.h"

calico::producers::image_producer_callback::image_producer_callback(so_5::agent_context_t ctx, so_5::mbox_t channel)
	: agent_t(std::move(ctx)), m_channel(std::move(channel))
{
}

void calico::producers::image_producer_callback::so_evt_start()
{
	m_device.on_next_image(m_stop_source.get_token(), [this](cv::Mat image) {
		so_5::send<cv::Mat>(m_channel, std::move(image));
	});
}

void calico::producers::image_producer_callback::so_evt_finish()
{
	m_stop_source.request_stop();
}
