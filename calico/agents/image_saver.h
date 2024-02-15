#pragma once
#include <so_5/all.hpp>
#include <filesystem>

namespace calico::agents
{
	// saves the input images to a specified folder
	class image_saver final : public so_5::agent_t
	{
	public:
		image_saver(so_5::agent_context_t ctx, so_5::mbox_t input, std::filesystem::path root_folder);
		void so_evt_start() override;
		void so_evt_finish() override;
	private:
		so_5::mbox_t m_input;
		so_5::mchain_t m_chain;
		std::filesystem::path m_root_folder;
	};

	// saves the input images to a specified folder.
	// This variant demonstrates the usage of thread-safe handlers
	// and does not introduce any additional worker
	class image_saver_one_agent final : public so_5::agent_t
	{
	public:
		image_saver_one_agent(so_5::agent_context_t ctx, so_5::mbox_t input, std::filesystem::path root_folder);
		void so_evt_start() override;
		void so_define_agent() override;
	private:
		so_5::mbox_t m_input;
		std::filesystem::path m_root_folder;
	};
}
