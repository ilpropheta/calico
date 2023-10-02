#include "utils.h"
#include "producers/image_producer.h"
#include "agents/image_viewer.h"

int main()
{
	const auto ctrl_c = calico::utils::get_ctrlc_token();

	const so_5::wrapped_env_t env;
	const auto main_channel = env.environment().create_mbox("main");

	env.environment().introduce_coop(so_5::disp::active_obj::make_dispatcher(env.environment()).binder(), [&](so_5::coop_t& c) {
		c.make_agent<calico::producers::image_producer>(main_channel, ctrl_c);
		c.make_agent<calico::agents::image_viewer>(main_channel);
	});

	calico::utils::wait_for_stop(ctrl_c);
}