#pragma once
#include <thread>
#include <opencv2/videoio.hpp>
#include "../errors.h"

namespace calico::devices
{
	// fake and minimal implementation of callback-based cv::VideoCapture
	class observable_videocapture
	{
	public:
		observable_videocapture();

		void start(auto on_next, auto on_error)
		{
			if (!m_capture.isOpened())
			{
				throw std::runtime_error("Can't connect to the webcam");
			}

			m_worker = std::jthread{ [this, on_image = std::move(on_next), on_err = std::move(on_error)](std::stop_token st) {
				while (!st.stop_requested())
				{
					cv::Mat image;
					if (m_capture.read(image))
					{
						on_image(std::move(image));
					}
					else
					{
						on_err(device_error{ "read error", device_error_type::read_error });
					}
				}
			} };
		}

		void stop();

	private:
		cv::VideoCapture m_capture;
		std::jthread m_worker;
	};
}
