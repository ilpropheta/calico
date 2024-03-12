#pragma once

#include <so_5/all.hpp>

// Demonstrates the use of mutable messages to efficiently exchange data within a statically defined processing pipeline.
// It tackles scenarios where copying data becomes unsustainable, especially in "hot path" processing situations.
// Applying the "pipes and filters" pattern, the problem is decomposed into distinct components (filters) interconnected
// through direct channels (pipes).
// By leveraging mutable messages, operations are performed directly on the data, avoiding redundant copies.
namespace calico::agents::static_pipeline
{
	// receives data from a message box called "main"
	// and outputs to 'step_2_dst'
	class step_1 : public so_5::agent_t
	{
	public:
		step_1(so_5::agent_context_t ctx, so_5::mbox_t step_2_dst);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_step_2_dst;
	};

	// receives from its own message box and outputs to 'step_3_dst'
	class step_2 : public so_5::agent_t
	{
	public:
		step_2(so_5::agent_context_t ctx, so_5::mbox_t step_3_dst);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_step_3_dst;
	};

	// receives from its own message box
	// and outputs data to a named message box called "output"
	class step_3 : public so_5::agent_t
	{
	public:
		step_3(so_5::agent_context_t ctx);
		void so_define_agent() override;
	};
}