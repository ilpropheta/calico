#include "virtual_image_producer.h"
#include <opencv2/imgcodecs/imgcodecs.hpp>

using namespace std::chrono_literals;

calico::producers::virtual_image_producer::virtual_image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, std::stop_token st, std::filesystem::path path)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_stop(std::move(st)), m_path(std::move(path))
{
}

void calico::producers::virtual_image_producer::so_evt_start()
{
	if (!is_directory(m_path))
	{
		throw std::runtime_error("Can't open virtual device directory");
	}
	auto it = std::filesystem::directory_iterator{ m_path };
	while (!m_stop.stop_requested())
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
