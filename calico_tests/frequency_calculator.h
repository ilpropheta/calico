#pragma once

#include <so_5/all.hpp>
#include <chrono>

// approximates the frequency of messages of a specified type on a certain message chain
template<typename Message>
class frequency_calculator
{
public:
	frequency_calculator(unsigned messages_count, so_5::mchain_t output)
		: m_messages_count(messages_count), m_output(std::move(output))
	{
	}

	double operator()() const
	{
		std::chrono::steady_clock::time_point tic;
		receive(from(m_output).handle_n(1), [&](so_5::mhood_t<Message>) { // first message
			tic = std::chrono::steady_clock::now();
		});

		receive(from(m_output).handle_n(m_messages_count - 2), [&](so_5::mhood_t<Message>) {});

		double frequency;
		receive(from(m_output).handle_n(1), [&](so_5::mhood_t<Message>) { // last message
			frequency = m_messages_count / std::chrono::duration<double>(std::chrono::steady_clock::now() - tic).count();
		});
		return frequency;
	}
private:
	unsigned m_messages_count;
	so_5::mchain_t m_output;
};