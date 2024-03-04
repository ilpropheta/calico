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

// Same as skynet but with custom dispatcher binder support
class skynet_tp final : public so_5::agent_t
{
public:
	skynet_tp(so_5::agent_context_t ctx, so_5::disp_binder_shptr_t disp, so_5::mbox_t parent, size_t num, size_t size)
		: agent_t(std::move(ctx)), m_disp(std::move(disp)), m_parent(std::move(parent)), m_num(num), m_size(size)
	{}

	void so_evt_start() override
	{
		if (1u == m_size)
		{
			so_5::send< size_t >(m_parent, m_num);
		}
		else
		{
			so_subscribe_self().event([this](size_t v) {
				m_sum += v;
				if (++m_received == divider)
				{
					so_5::send< size_t >(m_parent, m_sum);
				}
			});

			so_environment().introduce_coop(
				m_disp,
				[&](so_5::coop_t& coop) {
					const auto sub_size = m_size / divider;
					for (unsigned int i = 0; i != divider; ++i)
					{
						coop.make_agent<skynet_tp>(m_disp, so_direct_mbox(), m_num + i * sub_size, sub_size);
					}
			});
		}
	}

private:
	static inline size_t divider = 10;

	const so_5::disp_binder_shptr_t m_disp;
	const so_5::mbox_t m_parent;
	const size_t m_num;
	const size_t m_size;
	size_t m_sum = 0;
	size_t m_received = 0;
};

std::ostream& operator<<(std::ostream& os, so_5::disp::thread_pool::fifo_t fifo)
{
	return os << (fifo == so_5::disp::thread_pool::fifo_t::cooperation ? "cooperation" : "individual");
}

struct skynet_bench_params
{
	size_t thread_pool_size;
	so_5::disp::thread_pool::fifo_t fifo_strategy;
};

void do_skynet_thread_pool_benchmark(const skynet_bench_params& params)
{
	std::chrono::steady_clock::time_point tic;
	{
		const so_5::wrapped_env_t sobjectizer;
		const auto output = create_mchain(sobjectizer.environment());

		tic = std::chrono::steady_clock::now();
		const auto binder = so_5::disp::thread_pool::make_dispatcher(sobjectizer.environment(), params.thread_pool_size).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(params.fifo_strategy));
		sobjectizer.environment().introduce_coop([&](so_5::coop_t& coop) {
			coop.make_agent<skynet_tp>(binder, output->as_mbox(), 0u, 1000000u);
		});

		size_t result = 0u;
		receive(from(output).handle_n(1), [&result](size_t v) {
			result = v;
		});
		EXPECT_THAT(result, testing::Eq(499999500000));
	}
	std::cout << "skynet thread pool (size=" << params.thread_pool_size << " fifo=" << params.fifo_strategy << ") " << std::chrono::duration<double>(std::chrono::steady_clock::now() - tic) << "\n";
}

class skynet_thread_pool_benchmarks : public testing::TestWithParam<skynet_bench_params>
{
};

TEST_P(skynet_thread_pool_benchmarks, skynet_thread_pool)
{
	do_skynet_thread_pool_benchmark(GetParam());
}

INSTANTIATE_TEST_SUITE_P(cooperation_fifo, skynet_thread_pool_benchmarks,
	testing::Values(
		skynet_bench_params{ .thread_pool_size=1, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=2, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=3, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=4, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=8, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation }
));

INSTANTIATE_TEST_SUITE_P(individual_fifo, skynet_thread_pool_benchmarks,
	testing::Values(
		skynet_bench_params{ .thread_pool_size = 1, .fifo_strategy = so_5::disp::thread_pool::fifo_t::individual },
		skynet_bench_params{ .thread_pool_size = 2, .fifo_strategy = so_5::disp::thread_pool::fifo_t::individual },
		skynet_bench_params{ .thread_pool_size = 3, .fifo_strategy = so_5::disp::thread_pool::fifo_t::individual },
		skynet_bench_params{ .thread_pool_size = 4, .fifo_strategy = so_5::disp::thread_pool::fifo_t::individual },
		skynet_bench_params{ .thread_pool_size = 8, .fifo_strategy = so_5::disp::thread_pool::fifo_t::individual }
));

//// 1:1 messaging

struct ping_signal final : so_5::signal_t {};
struct pong_signal final : so_5::signal_t {};

class pinger final : public so_5::agent_t
{
public:
	pinger(so_5::agent_context_t ctx, unsigned count)
		: agent_t(std::move(ctx)), m_pings_count(count), m_pings_left(count)
	{
	}

	void set_ponger_channel(so_5::mbox_t other)
	{
		m_ponger_channel = std::move(other);
	}

	void so_define_agent() override
	{
		so_subscribe_self().event([this](so_5::mhood_t<pong_signal>) {
			send_ping();
		});
	}

	void so_evt_start() override
	{
		m_start = std::chrono::steady_clock::now();
		send_ping();
	}

private:
	void send_ping() {
		if (m_pings_left)
		{
			so_5::send<ping_signal>(m_ponger_channel);
			--m_pings_left;
		}
		else
		{
			const auto diff = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_start);
			const auto freq = m_pings_count / diff.count();
			std::cout << std::format("ping-pong count={} throughput={:.2f} mex/s real-throughput={:.2f} mex/s\n", m_pings_count, freq, freq * 2);
			so_environment().stop();
		}
	}

	std::chrono::time_point<std::chrono::steady_clock> m_start;
	unsigned m_pings_count;
	unsigned m_pings_left;
	so_5::mbox_t m_ponger_channel;
};

class ponger final : public so_5::agent_t
{
public:
	ponger(so_5::agent_context_t ctx)
		: agent_t(std::move(ctx))
	{
	}

	void set_pinger_channel(so_5::mbox_t other)
	{
		m_pinger_channel = std::move(other);
	}

	void so_define_agent() override
	{
		so_subscribe_self().event([this](so_5::mhood_t<ping_signal>) {
			so_5::send<pong_signal>(m_pinger_channel);
		});
	}

private:
	so_5::mbox_t m_pinger_channel;
};

TEST(benchmarks, ping_pong_single_thread)
{
	so_5::launch([](so_5::environment_t& env) {
		env.introduce_coop([&](so_5::coop_t& coop) {
			const auto pinger_agent = coop.make_agent<pinger>(100000);
			const auto ponger_agent = coop.make_agent<ponger>();
			pinger_agent->set_ponger_channel(ponger_agent->so_direct_mbox());
			ponger_agent->set_pinger_channel(pinger_agent->so_direct_mbox());
		});
	});
}

TEST(benchmarks, ping_pong_multi_thread)
{
	so_5::launch([](so_5::environment_t& env) {
		env.introduce_coop(so_5::disp::active_obj::make_dispatcher(env).binder(), [&](so_5::coop_t& coop) {
			const auto pinger_agent = coop.make_agent<pinger>(100000);
			const auto ponger_agent = coop.make_agent<ponger>();
			pinger_agent->set_ponger_channel(ponger_agent->so_direct_mbox());
			ponger_agent->set_pinger_channel(pinger_agent->so_direct_mbox());
		});
	});
}