#pragma once
#include <opencv2/videoio.hpp>
#include <string>

namespace calico::constants
{
	inline const std::string waitkey_channel_name = "waitkey_out";
	constexpr cv::VideoCaptureAPIs cv_cap_api = CAP_API;
}
