#pragma once

#include <so_5/all.hpp>
#include "../devices/observable_videocapture.h"

namespace calico::producers
{
	// grabs frames from the default camera (e.g. webcam) and sends them to the specified message box (callback-based device)
	// reacts to 'start_acquisition_command' and 'stop_acquisition_command' to control the acquisition
	class image_producer_callback final : public so_5::agent_t
	{
	public:
		image_producer_callback(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t commands);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
		so_5::mbox_t m_commands;
		devices::observable_videocapture m_device;
	};
}
