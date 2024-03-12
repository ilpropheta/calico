#pragma once
#include <so_5/all.hpp>

// Some agents to demonstrate the application of the "routing slip pattern"
// The user only sees "slip_router" which automatically creates the available agents and make them available.
namespace calico::agents::dynamic_pipeline
{
	class resize_step final : public so_5::agent_t
	{
	public:
		resize_step(so_5::agent_context_t ctx);
		void so_define_agent() override;
	};

	class add_crosshairs_step final : public so_5::agent_t
	{
	public:
		add_crosshairs_step(so_5::agent_context_t ctx);
		void so_define_agent() override;
	};

	class to_grayscale_step final : public so_5::agent_t
	{
	public:
		to_grayscale_step(so_5::agent_context_t ctx);
		void so_define_agent() override;
	};

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
