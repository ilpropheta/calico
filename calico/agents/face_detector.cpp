#include "face_detector.h"
#include "chain_closer.h"
#include <opencv2/imgproc.hpp>

calico::agents::face_detector::face_detector(so_5::agent_context_t ctx, so_5::mbox_t input, so_5::mbox_t output)
	: agent_t(std::move(ctx)), m_input(std::move(input)), m_output(std::move(output))
{
}

calico::agents::face_detector::face_detector(so_5::agent_context_t ctx, so_5::mbox_t input)
	: face_detector(ctx, std::move(input), ctx.environment().create_mbox())
{
}

so_5::mbox_t calico::agents::face_detector::output() const
{
	return m_output;
}

void calico::agents::face_detector::so_evt_start()
{
	if (!m_classifier.load("haarcascade_frontalface_default.xml"))
	{
		throw std::runtime_error("Can't load face detector classifier 'haarcascade_frontalface_default.xml'");
	}

	so_5::single_sink_binding_t binding;
	const auto buffer = create_mchain(so_environment());
	binding.bind<cv::Mat>(m_input, wrap_to_msink(buffer->as_mbox()));

	so_environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(so_environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<chain_closer>(buffer);
	});

	receive(from(buffer).handle_all(), [this](const cv::Mat& src) {
		cv::Mat gray;
		cvtColor(src, gray, cv::COLOR_BGR2GRAY);
		std::vector<cv::Rect> faces;
		m_classifier.detectMultiScale(gray, faces, 1.1, 4);
		auto cloned = src.clone();
		for (const auto& [x, y, w, h] : faces)
		{
			rectangle(cloned, { x, y }, { x + w, y + h }, { 255, 0, 0 }, 2);
		}
		so_5::send<cv::Mat>(m_output, std::move(cloned));
	});
}
