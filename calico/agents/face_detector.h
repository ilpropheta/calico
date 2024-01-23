#pragma once
#include <so_5/all.hpp>
#include <opencv2/objdetect.hpp>
#include <queue>

namespace calico::agents
{
	// overlays a rectangle onto each identified face within the frames and sends the changed image to the output channel (original images are not modified)
	// requires 'haarcascade_frontalface_default.xml' available in the module directory
	class face_detector final : public so_5::agent_t
	{
		struct process_one_buffered_image final : so_5::signal_t {};
	public:
		face_detector(so_5::agent_context_t ctx, so_5::mbox_t input, so_5::mbox_t output);
		face_detector(so_5::agent_context_t ctx, so_5::mbox_t input);
		void so_evt_start() override;
		so_5::mbox_t output() const;
		void so_define_agent() override;
	private:
		so_5::mbox_t m_input;
		so_5::mbox_t m_output;
		std::queue<so_5::message_holder_t<cv::Mat>> m_buffer;
		cv::CascadeClassifier m_classifier;
	};
}