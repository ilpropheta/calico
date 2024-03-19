#include "image_cache.h"
#include <ranges>

calico::agents::image_cache::image_cache(so_5::agent_context_t ctx, so_5::mbox_t input, so_5::mbox_t output, unsigned size)
	: agent_t(ctx), m_input(std::move(input)), m_output(std::move(output)), m_session_size(size), m_accumulated(0), m_cache(size)
{
}

void calico::agents::image_cache::so_define_agent()
{
	so_subscribe(m_input).event([this](so_5::mhood_t<cv::Mat> img) {
		if (m_session_size != m_accumulated)
		{
			m_cache[m_accumulated++] = img.make_holder();
		}
		else
		{
			flush();
		}
	});
}

void calico::agents::image_cache::flush()
{
	for (auto& img : std::views::take(m_cache, m_accumulated))
	{
		send(m_output, std::move(img));
	}
	m_accumulated = 0;
}
