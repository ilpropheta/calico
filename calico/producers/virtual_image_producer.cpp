#include "virtual_image_producer.h"
#include <opencv2/imgcodecs/imgcodecs.hpp>

using namespace std::chrono_literals;

calico::producers::virtual_image_producer::virtual_image_producer_broker::virtual_image_producer_broker(so_5::agent_context_t c, std::stop_source stop_source)
	: agent_t(std::move(c)), m_stop_source(std::move(stop_source))
{
}

void calico::producers::virtual_image_producer::virtual_image_producer_broker::so_evt_finish()
{
	m_stop_source.request_stop();
}

calico::producers::virtual_image_producer::virtual_image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, std::filesystem::path path)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_path(std::move(path))
{
}

void calico::producers::virtual_image_producer::so_evt_start()
{
	if (!is_directory(m_path))
	{
		throw std::runtime_error("Can't open virtual device directory");
	}

	introduce_child_coop(*this, so_5::disp::active_obj::make_dispatcher(so_environment()).binder(), [this](so_5::coop_t& c) {
		c.make_agent<virtual_image_producer_broker>(m_stop_source);
	});

	auto it = std::filesystem::directory_iterator{ m_path };
	const auto st = m_stop_source.get_token();
	while (!st.stop_requested())
	{
		if (it->is_regular_file())
		{
			so_5::send<cv::Mat>(m_channel, cv::imread(it->path().string()));
			std::this_thread::sleep_for(20ms); // we assume 50 FPS
		}
		if (++it == std::filesystem::directory_iterator{})
		{
			it = std::filesystem::directory_iterator{ m_path };
		}
	}
}
