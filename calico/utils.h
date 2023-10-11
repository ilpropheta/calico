#include <stop_token>

namespace calico::utils
{
	// returns a global stop_token that is fired when either SIGTERM or SIGINT are sent to this process (commonly, when CTRL+C is typed on the terminal)
	std::stop_token get_ctrlc_token();

	// blocks the calling thread until the stop token is triggered
	void wait_for_stop(const std::stop_token& st);
}