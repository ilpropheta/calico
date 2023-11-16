#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// displays images in a dedicated window, closing it if no new frames are received within 500 milliseconds
	class image_viewer : public so_5::agent_t
	{
		so_5::state_t st_handling_images{ this };
		so_5::state_t st_stream_down{ this };
	public:
		image_viewer(so_5::agent_context_t ctx, so_5::mbox_t channel);
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
		std::string m_title = "image viewer";
	};
}
