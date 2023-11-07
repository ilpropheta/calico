#include "virtual_image_producer.h"
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include "../signals.h"

using namespace std::chrono_literals;

calico::producers::virtual_image_producer::virtual_image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t commands, std::filesystem::path path)
	: agent_t(std::move(ctx)), m_channel(std::move(channel)), m_commands(std::move(commands)), m_path(std::move(path))
{
}

void calico::producers::virtual_image_producer::so_define_agent()
{
	st_started.event([this](so_5::mhood_t<grab_image>) {
		const auto tic = std::chrono::steady_clock::now();
		cv::Mat mat;
		m_current_it = std::find_if(m_current_it, {}, [](const auto& e) {
			return e.is_regular_file();
		});
		so_5::send<cv::Mat>(m_channel, cv::imread(m_current_it->path().string()));
		const auto elapsed = std::min(20ms, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tic));
		so_5::send_delayed<grab_image>(*this, 20ms - elapsed);
		if (++m_current_it == std::filesystem::directory_iterator{})
		{
			m_current_it = std::filesystem::directory_iterator{ m_path };
		}
	}).event(m_commands, [this](so_5::mhood_t<stop_acquisition_command>) {
		st_stopped.activate();
	});

	st_stopped.event(m_commands, [this](so_5::mhood_t<start_acquisition_command>) {
		st_started.activate();
		so_5::send<grab_image>(*this);
	});

	st_stopped.activate();
}

void calico::producers::virtual_image_producer::so_evt_start()
{
	if (!is_directory(m_path))
	{
		throw std::runtime_error("Can't open virtual device directory");
	}

	m_current_it = std::filesystem::directory_iterator{ m_path };
}
