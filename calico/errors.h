#pragma once

namespace calico
{
	enum class device_error_type
	{
		read_error
	};

	struct device_error
	{
		std::string message;
		device_error_type type;
	};
}