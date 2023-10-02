#include "utils.h"

int main()
{
	const auto ctrl_c = calico::utils::get_ctrlc_token();
	// ...
	calico::utils::wait_for_stop(ctrl_c);
}