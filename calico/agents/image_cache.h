#pragma once
#include <so_5/all.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>

namespace calico::agents
{
	// Caches exactly 'size' images and then sends them to the output channel, one by one
	class image_cache final : public so_5::agent_t
	{
	public:
		image_cache(so_5::agent_context_t ctx, so_5::mbox_t input, so_5::mbox_t output, unsigned size);
		void so_define_agent() override;
	private:
		void flush();

		so_5::mbox_t m_input;
		so_5::mbox_t m_output;
		unsigned m_session_size;
		unsigned m_accumulated;
		std::vector<so_5::message_holder_t<cv::Mat>> m_cache;
	};
}