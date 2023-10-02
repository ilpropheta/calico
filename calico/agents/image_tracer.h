#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// prints image dimensions to the standard output
	class image_tracer : public so_5::agent_t
	{
	public:
		image_tracer(so_5::agent_context_t ctx, so_5::mbox_t channel);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
	};
}
