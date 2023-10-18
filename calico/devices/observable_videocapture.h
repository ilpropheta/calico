#pragma once
#include <thread>
#include <opencv2/videoio.hpp>

namespace calico::devices
{
	// fake and minimal implementation of callback-based cv::VideoCapture
	class observable_videocapture
	{
	public:
		observable_videocapture();

		void on_next_image(std::stop_token st, auto on_next)
		{
			if (!m_capture.isOpened())
			{
				throw std::runtime_error("Can't connect to the webcam");
			}

			m_worker = std::jthread{ [this, st, action = std::move(on_next)] {
				cv::Mat image;
				while (!st.stop_requested())
				{
					m_capture >> image;
					action(std::move(image));
				}
			} };
		}
	private:
		cv::VideoCapture m_capture;
		std::jthread m_worker;
	};
}
