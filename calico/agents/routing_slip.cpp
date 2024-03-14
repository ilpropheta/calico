#include "routing_slip.h"

calico::agents::routing_slip::route_slip::route_slip(std::vector<so_5::mbox_t> routes)
	: m_routes(std::move(routes))
{
}

const so_5::mbox_t& calico::agents::routing_slip::route_slip::next()
{
	return m_routes[m_current++];
}
