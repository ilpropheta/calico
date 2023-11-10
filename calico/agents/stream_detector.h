#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// detects stream activities by sending a 'stream_up' signal upon receiving the first image and
	// a 'stream_down' signal if no new frames arrive within 500 milliseconds
	class stream_detector : public so_5::agent_t
	{
		so_5::state_t st_handling_images{ this };
		so_5::state_t st_stream_down{ this };
	public:
		struct stream_up final : so_5::signal_t {};
		struct stream_down final : so_5::signal_t {};

		stream_detector(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t output);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
		so_5::mbox_t m_out_channel;
	};
}
