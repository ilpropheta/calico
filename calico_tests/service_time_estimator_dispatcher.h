#pragma once
#include <so_5/all.hpp>
#include <thread>

// estimates the service time of a certain agent's handler
class service_time_estimator_dispatcher : public so_5::event_queue_t, public so_5::disp_binder_t
{
public:
	service_time_estimator_dispatcher(so_5::environment_t& env, so_5::mbox_t output, unsigned messages_count);
	void push(so_5::execution_demand_t demand) override;
	void push_evt_start(so_5::execution_demand_t demand) override;
	void push_evt_finish(so_5::execution_demand_t demand) noexcept override;
	void preallocate_resources(so_5::agent_t&) override;
	void undo_preallocation(so_5::agent_t&) noexcept override;
	void bind(so_5::agent_t& agent) noexcept override;
	void unbind(so_5::agent_t&) noexcept override;
	[[nodiscard]] static so_5::disp_binder_shptr_t make(so_5::environment_t& env, so_5::mbox_t output, unsigned messages_count);
private:
	std::jthread m_worker;
	so_5::mchain_t m_event_queue;
	so_5::mchain_t m_start_finish_queue;
	so_5::mbox_t m_output;
	double m_total_elapsed = 0.0;
};
