#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// resizes images by 'factor' and sends them to the output channel (original images are not modified)
	class image_resizer final : public so_5::agent_t
	{
	public:
		image_resizer(so_5::agent_context_t ctx, so_5::mbox_t input, so_5::mbox_t output, double factor);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_input;
		so_5::mbox_t m_output;
		double m_factor;
	};
}
