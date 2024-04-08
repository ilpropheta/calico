#pragma once
#include <so_5/all.hpp>

// A simple implementation of a domain-based layer on top of SObjectizer

namespace calico
{
	enum class chain_close_policy
	{
		retain = 0,
		drop
	};

	namespace details
	{
		using chain_action = std::function<void(const so_5::mchain_t&)>;

		struct chain_closer
		{
			using pointer = so_5::mchain_t;
			void operator()(const so_5::mchain_t& chain) const;
			chain_action closer;
		};

		using chain_holder = std::unique_ptr<so_5::mchain_t, chain_closer>;

		using coop_pair = std::pair<so_5::coop_handle_t, so_5::disp_binder_shptr_t>;

		struct session_coops
		{
			details::coop_pair root_coop;
			std::map<std::string, details::coop_pair, std::less<>> group_to_coop;
		};

		struct agent_session_state
		{
			std::reference_wrapper<so_5::environment_t> env;
			std::string session_id;
			session_coops coops;
			std::function<so_5::mchain_t(const std::string&, chain_close_policy)> create_chain;
		};
	}

	// what the user will interact with to add agents and create channels
	// destroying a session does NOT imply dropping its agents
	// In this case, the session does not have any further specialization/recipe
	// but in general it might be policy-based to expose the user to a certain domain-based "add_agent"-like functions
	// e.g. agent_session<a_certain_domain_policy>
	class agent_session
	{
	public:
		explicit agent_session(details::agent_session_state ctx);

		template<typename T, typename... Args>
		void add_monitoring_agent(Args&&... args)
		{
			add_agent<T>("monitoring", std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		void add_core_agent(Args&&... args)
		{
			add_agent<T>("core", std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		void add_dedicated_thread_agent(Args&&... args)
		{
			add_agent<T>("dedicated", std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		void add_agent(std::string_view group_name, Args&&... args)
		{
			if (const auto it = m_ctx.coops.group_to_coop.find(group_name); it != end(m_ctx.coops.group_to_coop))
			{
				add_agent_internal<T>(it->second, std::forward<Args>(args)...);
			}
			else
			{
				throw std::runtime_error("group not found");
			}
		}

		[[nodiscard]] so_5::mbox_t get_channel() const;
		[[nodiscard]] so_5::mbox_t get_channel(const std::string& name) const;
		[[nodiscard]] so_5::mchain_t make_chain(chain_close_policy close_policy = chain_close_policy::drop) const;
		[[nodiscard]] const std::string& get_id() const;
		[[nodiscard]] so_5::environment_t& get_env() const;
	private:
		template<typename T, typename... Args>
		void add_agent_internal(const details::coop_pair& coop, Args&&... args) const
		{
			so_5::introduce_child_coop(coop.first, coop.second, [&](so_5::coop_t& c) {
				c.make_agent<T>(std::forward<Args>(args)...);
			});
		}

		details::agent_session_state m_ctx;
	};

	// Creates and manages "sessions"
	// Supports explicit session destruction that deregisters every agents within it, and closes its chains
	class agent_manager
	{
	public:
		[[nodiscard]] agent_session create_session();
		[[nodiscard]] agent_session create_session(const std::string& session_name);
		bool destroy_session(const std::string& session_id);
	private:
		[[nodiscard]] so_5::mchain_t make_chain(const std::string& id, chain_close_policy closeMode);
		static details::chain_closer make_chain_closer(chain_close_policy mode);
		void add_chain(const std::string& id, so_5::mchain_t chain, chain_close_policy closeMode);

		so_5::wrapped_env_t m_sobjectizer;
		std::map<std::string, details::session_coops> m_session_to_coop;
		std::map<std::string, std::vector<details::chain_holder>> m_chains;
		int m_session_progressive_counter = 0;
	};
}