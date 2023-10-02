#include <thread>
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <stop_token>

using namespace std;

class ctrlc_handler
{
public:
	ctrlc_handler()
		: worker{ [this] { stopped.wait(false); source.request_stop(); } }
	{
	}

	void raise()
	{
		stopped.test_and_set();
		stopped.notify_one();
	}

	stop_token get_token()
	{
		return source.get_token();
	}
private:
	atomic_flag stopped;
	stop_source source;
	jthread worker;
};

static ctrlc_handler ctrlc;

void ctrlc_signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
	{
		ctrlc.raise();
	}
}

namespace calico::utils
{
	stop_token get_ctrlc_token()
	{
		[[maybe_unused]] static const auto _installed_SIGINT = signal(SIGINT, ctrlc_signal_handler);
		[[maybe_unused]] static const auto _installed_SIGTERM = signal(SIGTERM, ctrlc_signal_handler);
		return ctrlc.get_token();
	}

	void wait_for_stop(const stop_token& st)
	{
		condition_variable_any cv;
		mutex m;
		stop_callback stop_wait{ st, [&cv]() { cv.notify_one(); } };
		unique_lock l{ m };
		cv.wait(l, [&st] { return st.stop_requested(); });
	}
}