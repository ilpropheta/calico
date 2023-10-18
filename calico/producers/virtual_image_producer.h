#pragma once
#include <filesystem>
#include <so_5/all.hpp>

namespace calico::producers
{
	// sends images read from a folder to the specified message box, simulating 50fps (not accurate)
	class virtual_image_producer final : public so_5::agent_t
	{
		class virtual_image_producer_broker : public agent_t
		{
		public:
			virtual_image_producer_broker(so_5::agent_context_t c, std::stop_source stop_source);
			void so_evt_finish() override;
		private:
			std::stop_source m_stop_source;
		};
	public:
		virtual_image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, std::filesystem::path path);
		void so_evt_start() override;
	private:
		so_5::mbox_t m_channel;
		std::stop_source m_stop_source;
		std::filesystem::path m_path;
	};
}