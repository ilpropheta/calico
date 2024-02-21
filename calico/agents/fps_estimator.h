#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// estimates and logs the number of frames that are transmitted on a several channels, per second
	class fps_estimator final : public so_5::agent_t
	{
		struct measure_fps final : so_5::signal_t {};
	public:
		fps_estimator(so_5::agent_context_t ctx, std::vector<so_5::mbox_t> inputs);
		void so_evt_start() override;
		void so_define_agent() override;
	private:
		std::vector<so_5::mbox_t> m_inputs;
		so_5::timer_id_t m_timer;
		std::map<std::string, unsigned> m_counters;
		std::chrono::steady_clock::time_point m_start;
	};
}