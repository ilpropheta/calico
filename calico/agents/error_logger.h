#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// logs a message on every error received from the input channel, and the total number of errors at the shutdown
	class error_logger final : public so_5::agent_t
	{
	public:
		error_logger(so_5::agent_context_t ctx, so_5::mbox_t input);
		void so_define_agent() override;
		void so_evt_finish() override;
	private:
		so_5::mbox_t m_input;
		int m_error_count = 0;
	};
}
