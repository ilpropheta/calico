#include "image_producer_callback.h"
#include "../signals.h"

calico::producers::image_producer_callback::image_producer_callback(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t commands)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_commands(std::move(commands))
{
}

void calico::producers::image_producer_callback::so_define_agent()
{
	so_subscribe(m_commands).event([this](so_5::mhood_t<start_acquisition_command>) {
		m_device.start([this](cv::Mat image) {
			so_5::send<cv::Mat>(m_channel, std::move(image));
		}, [this](device_error err) {
			so_5::send<device_error>(m_channel, std::move(err));
		});
	}).event([this](so_5::mhood_t<stop_acquisition_command>) {
		m_device.stop();
	});
}
