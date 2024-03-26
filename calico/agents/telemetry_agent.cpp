#include "telemetry_agent.h"
#include "../signals.h"
#include <syncstream>

using namespace std::chrono_literals;

calico::agents::telemetry_agent::telemetry_agent(so_5::agent_context_t ctx)
	: agent_t(ctx), m_commands(so_environment().create_mbox("commands"))
{
}

void calico::agents::telemetry_agent::so_define_agent()
{
	st_turned_off.event(m_commands, [this](so_5::mhood_t<enable_telemetry_command>) {
		so_environment().stats_controller().turn_on();
		so_environment().stats_controller().set_distribution_period(1s);
		st_turned_on.activate();
	});
	st_turned_on.event(m_commands, [this](so_5::mhood_t<enable_telemetry_command>) {
		so_environment().stats_controller().turn_off();
		st_turned_off.activate();
	});

	so_set_delivery_filter(so_environment().stats_controller().mbox(), [](const so_5::stats::messages::quantity<size_t>& q) {
		return std::string_view(q.m_prefix.c_str()).find("/ao/calico_active_object") != std::string_view::npos;
	});

	st_turned_on.event(so_environment().stats_controller().mbox(), [](const so_5::stats::messages::quantity<size_t>& q) {
		std::osyncstream(std::cout) << q.m_prefix << q.m_suffix << "=" << q.m_value << "\n";
	});

	st_turned_off.activate();
}
