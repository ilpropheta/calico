#pragma once

namespace calico
{
	struct start_acquisition_command final : so_5::signal_t {};
	struct stop_acquisition_command final : so_5::signal_t {};
	struct enable_telemetry_command final : so_5::signal_t {};
}