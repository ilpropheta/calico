#include "utils.h"
#include "agents/image_viewer.h"
#include "agents/image_tracer.h"
#include "signals.h"
#include "producers/image_producer_recursive.h"

int main()
{
	const auto ctrl_c = calico::utils::get_ctrlc_token();

	const so_5::wrapped_env_t sobjectizer;
	const auto main_channel = sobjectizer.environment().create_mbox("main");
	auto commands_channel = sobjectizer.environment().create_mbox("commands");

	sobjectizer.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(sobjectizer.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<calico::producers::image_producer_recursive>(main_channel, commands_channel);
		c.make_agent<calico::agents::image_viewer>(main_channel);
		c.make_agent<calico::agents::image_tracer>(main_channel);
	});

	std::this_thread::sleep_for(std::chrono::seconds(1));
	so_5::send<calico::start_acquisition_command>(commands_channel);

	std::cout << "Acquisition START command sent!\n";

	std::this_thread::sleep_for(std::chrono::seconds(5));
	so_5::send<calico::stop_acquisition_command>(commands_channel);

	std::cout << "Acquisition STOP command sent!\n";

	calico::utils::wait_for_stop(ctrl_c);
}
