#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <so_5/all.hpp>

// implementation of https://github.com/atemerev/skynet/
class skynet final : public so_5::agent_t
{
public:
	skynet(so_5::agent_context_t ctx, so_5::mbox_t parent, size_t num, size_t size)
		: agent_t(ctx), m_parent(std::move(parent)), m_num(num), m_size(size)
	{}

	void so_evt_start() override
	{
		if (1u == m_size)
		{
			so_5::send<size_t>(m_parent, m_num);
		}
		else
		{
			so_subscribe_self().event([this](size_t v) {
				m_sum += v;
				if (++m_received == divider)
				{
					so_5::send<size_t>(m_parent, m_sum);
				}
				});

			so_environment().introduce_coop([&](so_5::coop_t& coop) {
				const auto sub_size = m_size / divider;
				for (unsigned int i = 0; i != divider; ++i)
				{
					coop.make_agent<skynet>(so_direct_mbox(), m_num + i * sub_size, sub_size);
				}
			});
		}
	}

private:
	static inline size_t divider = 10;

	const so_5::mbox_t m_parent;
	const size_t m_num;
	const size_t m_size;
	size_t m_sum = 0;
	unsigned int m_received = 0;
};

// Cost of registering agents
TEST(benchmarks, skynet)
{
	const so_5::wrapped_env_t sobjectizer;
	const auto output = create_mchain(sobjectizer.environment());

	const auto tic = std::chrono::steady_clock::now();
	sobjectizer.environment().introduce_coop([&](so_5::coop_t& c) {
		c.make_agent<skynet>(output->as_mbox(), 0u, 1000000u);
	});

	size_t result = 0;
	receive(from(output).handle_n(1), [&](size_t i) {
		result = i;
	});
	std::cout << std::chrono::duration<double>(std::chrono::steady_clock::now() - tic) << "\n";

	EXPECT_THAT(result, testing::Eq(499999500000));
}

// Cost of registering and deregistering agents
TEST(benchmarks, skynet_with_deregistration)
{
	std::chrono::steady_clock::time_point tic;

	{
		const so_5::wrapped_env_t sobjectizer;
		const auto output = create_mchain(sobjectizer.environment());
		tic = std::chrono::steady_clock::now();

		sobjectizer.environment().introduce_coop([&](so_5::coop_t& c) {
			c.make_agent<skynet>(output->as_mbox(), 0u, 1000000u);
		});

		size_t result = 0;
		receive(from(output).handle_n(1), [&](size_t i) {
			result = i;
		});
		EXPECT_THAT(result, testing::Eq(499999500000));
	}

	std::cout << std::chrono::duration<double>(std::chrono::steady_clock::now() - tic) << "\n";
}