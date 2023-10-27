#include "observable_videocapture.h"

calico::devices::observable_videocapture::observable_videocapture()
	: m_capture(0, cv::CAP_DSHOW)
{
}

void calico::devices::observable_videocapture::stop()
{
	m_worker.request_stop();
}
