#include "observable_videocapture.h"
#include "../constants.h"

calico::devices::observable_videocapture::observable_videocapture()
	: m_capture(0, constants::cv_system_detected_video_capture_api)
{
}

void calico::devices::observable_videocapture::stop()
{
	m_worker.request_stop();
}
