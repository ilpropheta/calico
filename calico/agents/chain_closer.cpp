#include "chain_closer.h"

calico::agents::chain_closer::chain_closer(so_5::agent_context_t ctx, so_5::mchain_t chain)
	: agent_t(std::move(ctx)), m_chain(std::move(chain))
{
}

void calico::agents::chain_closer::so_evt_finish()
{
	close_drop_content(so_5::terminate_if_throws, m_chain);
}
