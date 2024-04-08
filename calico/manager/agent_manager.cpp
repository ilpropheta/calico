#include "agent_manager.h"
#include <ranges>

void calico::details::chain_closer::operator()(const so_5::mchain_t& chain) const
{
	closer(chain);
}

calico::agent_session::agent_session(details::agent_session_state ctx)
	: m_ctx(std::move(ctx))
{
}

so_5::mbox_t calico::agent_session::get_channel() const
{
	return m_ctx.env.get().create_mbox();
}

so_5::mbox_t calico::agent_session::get_channel(const std::string& name) const
{
	return m_ctx.env.get().create_mbox(name);
}

so_5::mchain_t calico::agent_session::make_chain(chain_close_policy close_policy) const
{
	return m_ctx.create_chain(m_ctx.session_id, close_policy);
}

const std::string& calico::agent_session::get_id() const
{
	return m_ctx.session_id;
}

so_5::environment_t& calico::agent_session::get_env() const
{
	return m_ctx.env;
}

calico::agent_session calico::agent_manager::create_session()
{
	static const std::string empty_session_name;
	return create_session(empty_session_name);
}

calico::agent_session calico::agent_manager::create_session(const std::string& session_name)
{
	const auto session_id = session_name + std::to_string(m_session_progressive_counter);
	details::agent_session_state ctx{ m_sobjectizer.environment(), session_id };

	const auto root_binder = so_5::disp::active_group::make_dispatcher(m_sobjectizer.environment()).binder(std::format("root_{}", session_id));
	ctx.coops.root_coop = { m_sobjectizer.environment().register_coop(m_sobjectizer.environment().make_coop(root_binder)), root_binder };

	// here is the "recipe" - hardcoded in this case, but it might be specified dynamically 
	const auto monitoring_binder = so_5::disp::active_group::make_dispatcher(m_sobjectizer.environment()).binder(std::format("monitoring_{}", session_id));
	ctx.coops.group_to_coop["monitoring"] = { m_sobjectizer.environment().register_coop(m_sobjectizer.environment().make_coop(monitoring_binder)), monitoring_binder };

	const auto core_binder = so_5::disp::active_group::make_dispatcher(m_sobjectizer.environment()).binder(std::format("pipeline_{}", session_id));
	ctx.coops.group_to_coop["core"] = { m_sobjectizer.environment().register_coop(m_sobjectizer.environment().make_coop(core_binder)), core_binder };

	const auto dedicated_binder = so_5::disp::active_obj::make_dispatcher(m_sobjectizer.environment()).binder();
	ctx.coops.group_to_coop["dedicated"] = { m_sobjectizer.environment().register_coop(m_sobjectizer.environment().make_coop(dedicated_binder)), dedicated_binder };

	m_session_to_coop[session_id] = ctx.coops;
	ctx.create_chain = [this](const std::string& idx, chain_close_policy closeMode) { return make_chain(idx, closeMode); };
	++m_session_progressive_counter;
	return agent_session{ std::move(ctx) };
}

bool calico::agent_manager::destroy_session(const std::string& session_id)
{
	m_chains.erase(session_id); // first, close chains
	if (const auto it = m_session_to_coop.find(session_id); it != end(m_session_to_coop))
	{
		// next, deregister every "group"...
		for (auto& [coop_handle, binder] : it->second.group_to_coop | std::ranges::views::values)
		{
			m_sobjectizer.environment().deregister_coop(coop_handle, so_5::dereg_reason::normal);
		}
		// ...including the root
		m_sobjectizer.environment().deregister_coop(it->second.root_coop.first, so_5::dereg_reason::normal);
		m_session_to_coop.erase(it); // finally, drops the corresponding entry from the map
	 	return true;
	}
	return false;
}

so_5::mchain_t calico::agent_manager::make_chain(const std::string& id, chain_close_policy closeMode)
{
	auto chain = m_sobjectizer.environment().create_mchain(so_5::make_unlimited_mchain_params());
	add_chain(id, chain, closeMode);
	return chain;
}

calico::details::chain_closer calico::agent_manager::make_chain_closer(chain_close_policy mode)
{
	switch (mode)
	{
	case chain_close_policy::drop:
		return { [](const so_5::mchain_t& chain) {close_retain_content(so_5::terminate_if_throws, chain); } };
	case chain_close_policy::retain:
		return { [](const so_5::mchain_t& chain) {close_drop_content(so_5::terminate_if_throws, chain); } };
	}
	throw std::runtime_error("Cannot create chain closer");
}

void calico::agent_manager::add_chain(const std::string& id, so_5::mchain_t chain, chain_close_policy closeMode)
{
	m_chains[id].push_back({ std::move(chain), make_chain_closer(closeMode) });
}
