#pragma once
#include <filesystem>
#include <so_5/all.hpp>

namespace calico::producers
{
	// sends images read from a folder to the specified message box, simulating approximately 50fps
	class virtual_image_producer final : public so_5::agent_t
	{
		struct grab_image final : so_5::signal_t {};
	public:
		virtual_image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t commands, std::filesystem::path path);
		void so_define_agent() override;
		void so_evt_start() override;
	private:
		so_5::state_t st_stopped{ this };
		so_5::state_t st_started{ this };

		so_5::mbox_t m_channel;
		so_5::mbox_t m_commands;
		std::filesystem::path m_path;
		std::filesystem::directory_iterator m_current_it;
	};
}