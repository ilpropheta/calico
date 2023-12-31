#include "image_producer.h"
#include "../signals.h"
#include "../constants.h"
#include "../errors.h"
#include <opencv2/videoio.hpp>

calico::producers::image_producer::image_producer_broker::image_producer_broker(so_5::agent_context_t c, image_producer* parent, so_5::mbox_t commands)
	: agent_t(std::move(c)), m_parent(parent), m_commands(std::move(commands))
{
}

void calico::producers::image_producer::image_producer_broker::so_define_agent()
{
	so_subscribe(m_commands).event([this](so_5::mhood_t<start_acquisition_command>) {
		m_parent->start();
	}).event([this](so_5::mhood_t<stop_acquisition_command>) {
		m_parent->stop();
	});
}

void calico::producers::image_producer::image_producer_broker::so_evt_finish()
{
	m_parent->cancel();
}

calico::producers::image_producer::image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t commands)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_commands(std::move(commands))
{
}

void calico::producers::image_producer::so_evt_start()
{
	introduce_child_coop(*this, so_5::disp::active_obj::make_dispatcher(so_environment()).binder(), [this](so_5::coop_t& c) {
		c.make_agent<image_producer_broker>(this, m_commands);
	});

	cv::VideoCapture capture(0, constants::cv_system_detected_video_capture_api);
	if (!capture.isOpened())
	{
		throw std::runtime_error("Can't connect to the webcam");
	}

	while (m_state != acquisition_state::st_cancelled)
	{
		m_state.wait(acquisition_state::st_stopped);
		while (m_state == acquisition_state::st_started)
		{
			cv::Mat image;
			if (capture.read(image))
			{
				so_5::send<cv::Mat>(m_channel, std::move(image));
			}
			else
			{
				so_5::send<device_error>(m_channel, "read error", device_error_type::read_error);
			}
		}
	}
}

void calico::producers::image_producer::start()
{
	change_state(acquisition_state::st_started);
}

void calico::producers::image_producer::stop()
{
	change_state(acquisition_state::st_stopped);
}

void calico::producers::image_producer::cancel()
{
	change_state(acquisition_state::st_cancelled);
}

void calico::producers::image_producer::change_state(acquisition_state st)
{
	m_state = st;
	m_state.notify_one();
}
