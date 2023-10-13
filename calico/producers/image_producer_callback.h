#pragma once
#include <stop_token>
#include <so_5/all.hpp>
#include "../devices/observable_videocapture.h"

namespace calico::producers
{
	// grabs frames from the default camera (e.g. webcam) and sends them to the specified message box (callback-based device)
	class image_producer_callback final : public so_5::agent_t
	{
	public:
		image_producer_callback(so_5::agent_context_t ctx, so_5::mbox_t channel);
		void so_evt_start() override;
		void so_evt_finish() override;
	private:
		so_5::mbox_t m_channel;
		devices::observable_videocapture m_device;
		std::stop_source m_stop_source;
	};
}