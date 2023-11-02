#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// displays a tiny frame where "Enter" and "Escape" are turned, respectively, into 'start_acquisition_command'
	// and 'stop_acquisition_command', and are sent to the input channel
	class remote_control final : public so_5::agent_t
	{
		struct keep_on final : so_5::signal_t {};
	public:
		remote_control(so_5::agent_context_t ctx, so_5::mbox_t commands);
		void so_evt_start() override;
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
	};
}
