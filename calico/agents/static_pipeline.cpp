#include "static_pipeline.h"
#include <opencv2/imgproc.hpp>

calico::agents::static_pipeline::step_1::step_1(so_5::agent_context_t ctx, so_5::mbox_t step_2_dst)
	: agent_t(ctx), m_step_2_dst(std::move(step_2_dst))
{

}

void calico::agents::static_pipeline::step_1::so_define_agent()
{
	so_subscribe(so_environment().create_mbox("main")).event([this](const cv::Mat& img) {
		auto local_image = img.clone();
		resize(local_image, local_image, {}, 0.5, 0.5);
		so_5::send<so_5::mutable_msg<cv::Mat>>(m_step_2_dst, std::move(local_image));
	});
}

calico::agents::static_pipeline::step_2::step_2(so_5::agent_context_t ctx, so_5::mbox_t step_3_dst)
	: agent_t(ctx), m_step_3_dst(std::move(step_3_dst))
{

}

void calico::agents::static_pipeline::step_2::so_define_agent()
{
	so_subscribe_self().event([this](so_5::mutable_mhood_t<cv::Mat> img) {
		line(*img, { img->cols / 2, 0 }, { img->cols / 2, img->rows }, { 0, 0, 0 }, 3);
		line(*img, { 0, img->rows / 2 }, { img->cols, img->rows / 2 }, { 0, 0, 0 }, 3);
		send(m_step_3_dst, std::move(img));
	});
}

calico::agents::static_pipeline::step_3::step_3(so_5::agent_context_t ctx)
	: agent_t(ctx)
{

}

void calico::agents::static_pipeline::step_3::so_define_agent()
{
	so_subscribe_self().event([this](so_5::mutable_mhood_t<cv::Mat> img) {
		cvtColor(*img, *img, cv::COLOR_BGR2GRAY);
		send(so_environment().create_mbox("output"), to_immutable(std::move(img)));
	});
}
