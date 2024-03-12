#pragma once
#include <so_5/all.hpp>

// Demonstrates the usage of the "routing slip pattern".
// In this version, implementation details of agents are hidden in the slip_router, just to show
// the usage of calico::agents::routing_slip::make_generic_step
namespace calico::agents::dynamic_pipeline
{
	class slip_router : public so_5::agent_t
	{
	public:
		slip_router(so_5::agent_context_t ctx, so_5::mbox_t source, so_5::mbox_t last);
		void so_evt_start() override;
		void so_define_agent() override;
	private:
		so_5::mbox_t m_source;
		so_5::mbox_t m_last;
		std::map<std::string, so_5::mbox_t> m_available_steps;
	};
}
