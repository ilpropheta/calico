#include "error_logger.h"
#include "../errors.h"
#include <syncstream>
#include <format>

calico::agents::error_logger::error_logger(so_5::agent_context_t ctx, so_5::mbox_t input)
	: agent_t(std::move(ctx)), m_input(std::move(input))
{

}

void calico::agents::error_logger::so_define_agent()
{
	so_subscribe(m_input).event([this, chan_name = m_input->query_name()](const device_error& error) {
		++m_error_count;
		std::osyncstream{ std::cout } << std::format("Got error on channel '{}': {}\n", chan_name, error.message);
	});
}

void calico::agents::error_logger::so_evt_finish()
{
	std::osyncstream{ std::cout } << std::format("Total errors on channel '{}': {}\n", m_input->query_name(), m_error_count);
}
