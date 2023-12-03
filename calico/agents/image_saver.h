#pragma once
#include <so_5/all.hpp>
#include <filesystem>

namespace calico::agents
{
	// saves the input images to a specified folder
	class image_save_worker final : public so_5::agent_t
	{
	public:
		image_save_worker(so_5::agent_context_t ctx, so_5::mchain_t input, std::filesystem::path root_folder);
		void so_evt_start() override;
	private:
		so_5::mchain_t m_input;
		std::filesystem::path m_root_folder;
		int m_counter = 0;
		static inline int global_id = 0;
		int m_worker_id = global_id++;
	};
}
