#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// Enables/disables telemetry collection by handling enable_telemetry_command, and prints some filtered telemetry data to console
	class telemetry_agent final : public so_5::agent_t
	{
	public:
		telemetry_agent(so_5::agent_context_t ctx);
		void so_define_agent() override;
	private:
		state_t st_turned_on{ this }, st_turned_off{ this };
		so_5::mbox_t m_commands;
	};
}