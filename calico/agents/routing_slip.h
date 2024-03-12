#pragma once
#include <opencv2/core/mat.hpp>
#include <so_5/all.hpp>

// Demonstrates the use of mutable messages to implement the "routing slip pattern".
namespace calico::agents::routing_slip
{
	// represents a "routing slip", that is a processing pipeline dynamically assembled
	class route_slip
	{
	public:
		route_slip(std::vector<so_5::mbox_t> routes);
		const so_5::mbox_t& next();
	private:
		std::vector<so_5::mbox_t> m_routes;
		size_t m_current = 0;
	};

	// combines a message with the (remaining) processing pipeline it must go through
	template<typename T>
	struct route_slip_message
	{
		const so_5::mbox_t& next()
		{
			return slip.next();
		}

		route_slip slip;
		T payload;
	};

	// utility function to encapsulate the logic to send the message to the next step
	template<typename T>
	void send_to_next_step(so_5::mutable_mhood_t<route_slip_message<T>> msg)
	{
		const auto& next_step = msg->next();
		so_5::send(next_step, std::move(msg));
	}
}