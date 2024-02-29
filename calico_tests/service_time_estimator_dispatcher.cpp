#include "service_time_estimator_dispatcher.h"

service_time_estimator_dispatcher::service_time_estimator_dispatcher(so_5::environment_t& env, so_5::mbox_t output, unsigned messages_count)
	: m_event_queue(create_mchain(env)), m_start_finish_queue(create_mchain(env)), m_output(std::move(output))
{
	m_worker = std::jthread{ [messages_count, this] {
		const auto thread_id = so_5::query_current_thread_id();

		receive(from(m_start_finish_queue).handle_n(1), [thread_id, this](so_5::execution_demand_t d) {
			d.call_handler(thread_id);
		});

		receive(from(m_event_queue).handle_n(messages_count), [thread_id, this](so_5::execution_demand_t d) {
			const auto tic = std::chrono::steady_clock::now();
			d.call_handler(thread_id);
			const auto toc = std::chrono::steady_clock::now();
			m_total_elapsed += std::chrono::duration<double>(toc - tic).count();
		});

		so_5::send<double>(m_output, m_total_elapsed / messages_count);

		receive(from(m_start_finish_queue).handle_n(1), [thread_id, this](so_5::execution_demand_t d) {
			d.call_handler(thread_id);
		});
	} };
}

void service_time_estimator_dispatcher::push(so_5::execution_demand_t demand)
{
	so_5::send<so_5::execution_demand_t>(m_event_queue, std::move(demand));
}

void service_time_estimator_dispatcher::push_evt_start(so_5::execution_demand_t demand)
{
	so_5::send<so_5::execution_demand_t>(m_start_finish_queue, std::move(demand));
}

void service_time_estimator_dispatcher::push_evt_finish(so_5::execution_demand_t demand) noexcept
{
	so_5::send<so_5::execution_demand_t>(m_start_finish_queue, std::move(demand));
}

void service_time_estimator_dispatcher::preallocate_resources(so_5::agent_t&)
{
}

void service_time_estimator_dispatcher::undo_preallocation(so_5::agent_t&) noexcept
{
}

void service_time_estimator_dispatcher::bind(so_5::agent_t& agent) noexcept
{
	agent.so_bind_to_dispatcher(*this);
}

void service_time_estimator_dispatcher::unbind(so_5::agent_t&) noexcept
{
}

so_5::disp_binder_shptr_t service_time_estimator_dispatcher::make(so_5::environment_t& env, so_5::mbox_t output, unsigned messages_count)
{
	return std::make_shared<service_time_estimator_dispatcher>(env, std::move(output), messages_count);
}
