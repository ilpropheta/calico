#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// automatically closes a given message chain at the shutdown, dropping its content
	class chain_closer final : public so_5::agent_t
	{
	public:
		chain_closer(so_5::agent_context_t ctx, so_5::mchain_t chain);
		void so_evt_finish() override;
	private:
		so_5::mchain_t m_chain;
	};
}