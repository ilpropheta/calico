#include "image_saver.h"
#include <opencv2/imgcodecs.hpp>

namespace details
{
	class image_save_worker final : public so_5::agent_t
	{
	public:
		image_save_worker(so_5::agent_context_t ctx, so_5::mchain_t input, std::filesystem::path root_folder)
			: agent_t(std::move(ctx)), m_input(std::move(input)), m_root_folder(std::move(root_folder))
		{
			
		}

		void so_evt_start() override
		{
			receive(from(m_input).handle_all(), [this](const cv::Mat& image) {
				imwrite((m_root_folder / std::format("image_{}_{}.jpg", m_worker_id, m_counter++)).string(), image);
			});
		}
	private:
		so_5::mchain_t m_input;
		std::filesystem::path m_root_folder;
		int m_counter = 0;
		static inline int global_id = 0;
		int m_worker_id = global_id++;
	};
}

calico::agents::image_saver::image_saver(so_5::agent_context_t ctx, so_5::mbox_t input, std::filesystem::path root_folder)
	: agent_t(std::move(ctx)), m_input(std::move(input)), m_chain(create_mchain(so_environment())), m_root_folder(std::move(root_folder))
{
}

void calico::agents::image_saver::so_evt_start()
{
	std::error_code ec;
	if (create_directories(m_root_folder, ec); ec)
	{
		throw std::runtime_error(std::format("image_saver can't create root folder: {}", ec.message()));
	}

	so_environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(so_environment()).binder(), [this](so_5::coop_t& c) {
		const auto binding = c.take_under_control(std::make_unique<so_5::single_sink_binding_t>());
		binding->bind<cv::Mat>(m_input, wrap_to_msink(m_chain->as_mbox()));
		c.make_agent<details::image_save_worker>(m_chain, m_root_folder);
		c.make_agent<details::image_save_worker>(m_chain, m_root_folder);
	});
}

void calico::agents::image_saver::so_evt_finish()
{
	close_retain_content(so_5::terminate_if_throws, m_chain);
}
