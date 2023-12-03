#include "image_saver.h"
#include <opencv2/imgcodecs.hpp>

calico::agents::image_save_worker::image_save_worker(so_5::agent_context_t ctx, so_5::mchain_t input, std::filesystem::path root_folder)
	: agent_t(std::move(ctx)), m_input(std::move(input)), m_root_folder(std::move(root_folder))
{
}

void calico::agents::image_save_worker::so_evt_start()
{
	receive(from(m_input).handle_all(), [this](const cv::Mat& image) {
		imwrite((m_root_folder / std::format("image_{}_{}.jpg", m_worker_id, m_counter++)).string(), image);
	});
}
