#pragma once
#include <so_5\all.hpp>

namespace calico::agents
{
	// displays images into a dedicated window
	class image_viewer final : public so_5::agent_t
	{
	public:
		image_viewer(so_5::agent_context_t ctx, so_5::mbox_t channel);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
	};
}