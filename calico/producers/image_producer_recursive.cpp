#include "image_producer_recursive.h"
#include "../signals.h"
#include "../errors.h"
#include "../constants.h"

calico::producers::image_producer_recursive::image_producer_recursive(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t commands)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_commands(std::move(commands)), m_capture(0, calico::constants::cv_cap_api)
{
}

void calico::producers::image_producer_recursive::so_define_agent()
{
	st_started.event([this](so_5::mhood_t<grab_image>) {
		cv::Mat image;
		if (m_capture.read(image))
		{
			so_5::send<cv::Mat>(m_channel, std::move(image));
		}
		else
		{
			so_5::send<device_error>(m_channel, "read error", device_error_type::read_error);
		}
		so_5::send<grab_image>(*this);
	}).event(m_commands, [this](so_5::mhood_t<stop_acquisition_command>) {
		st_stopped.activate();
	});

	st_stopped.event(m_commands, [this](so_5::mhood_t<start_acquisition_command>) {
		st_started.activate();
		so_5::send<grab_image>(*this);
	});

	st_stopped.activate();
}

void calico::producers::image_producer_recursive::so_evt_start()
{
	if (!m_capture.isOpened())
	{
		throw std::runtime_error("Can't connect to the webcam");
	}
}
