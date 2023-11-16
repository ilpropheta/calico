#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// logs the stream's uptime (aka: the duration it remains active) every 5 seconds until frames are no longer received
	// for a duration of 500 milliseconds
	class stream_heartbeat final : public so_5::agent_t
	{
		struct log_heartbeat final : so_5::signal_t {};
		so_5::state_t st_handling_images{ this };
		so_5::state_t st_stream_down{ this };
	public:
		stream_heartbeat(so_5::agent_context_t ctx, so_5::mbox_t channel);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
		std::chrono::time_point<std::chrono::steady_clock> m_start_time;
		so_5::timer_id_t m_timer;
	};

	// logs the stream's uptime (aka: the duration it remains active) by reacting to 'stream_up' and 'stream_down' signals
	class stream_heartbeat_with_detector final : public so_5::agent_t
	{
		struct log_heartbeat final : so_5::signal_t {};
	public:
		stream_heartbeat_with_detector(so_5::agent_context_t ctx, so_5::mbox_t detector_channel);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
		std::chrono::time_point<std::chrono::steady_clock> m_start_time;
		so_5::timer_id_t m_timer;
	};
}
