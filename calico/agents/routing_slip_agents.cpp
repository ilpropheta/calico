#include "routing_slip_agents.h"
#include <opencv2/imgproc.hpp>
#include "routing_slip.h"

calico::agents::dynamic_pipeline::slip_router::slip_router(so_5::agent_context_t ctx, so_5::mbox_t source, so_5::mbox_t last)
	: agent_t(ctx), m_source(std::move(source)), m_last(std::move(last))
{

}

void calico::agents::dynamic_pipeline::slip_router::so_evt_start()
{
	so_environment().introduce_coop(so_5::disp::active_group::make_dispatcher(so_environment()).binder("slip"), [this](so_5::coop_t& c) {
		m_available_steps["resize"] = routing_slip::make_generic_step<cv::Mat>(c, [](auto& msg) {
			resize(msg->payload, msg->payload, {}, 0.5, 0.5);
		});
		m_available_steps["add_crosshairs"] = routing_slip::make_generic_step<cv::Mat>(c, [](auto& msg) {
			auto& img = msg->payload;
			line(img, { img.cols / 2, 0 }, { img.cols / 2, img.rows }, { 0, 0, 0 }, 3);
			line(img, { 0, img.rows / 2 }, { img.cols, img.rows / 2 }, { 0, 0, 0 }, 3);
		});
		m_available_steps["to_grayscale"] = routing_slip::make_generic_step<cv::Mat>(c, [](auto& msg) {
			cvtColor(msg->payload, msg->payload, cv::COLOR_BGR2GRAY);
		});
		m_available_steps["last"] = c.make_agent<routing_slip::slip_last_step<cv::Mat>>(m_last)->so_direct_mbox();
	});
}

void calico::agents::dynamic_pipeline::slip_router::so_define_agent()
{
	so_subscribe(m_source).event([this](const cv::Mat& img) {
		auto local_image = img.clone();

		// imagine this is created dynamically and only when something changes...
		routing_slip::route_slip_message slip{ {{
				m_available_steps.at("add_crosshairs"),
				m_available_steps.at("to_grayscale"),
				m_available_steps.at("last")}}, // we mustn't forget this one!
			std::move(local_image) };

		const auto first_step_channel = slip.next();
		so_5::send<so_5::mutable_msg<routing_slip::route_slip_message<cv::Mat>>>(first_step_channel, std::move(slip));
	});
}
