#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <so_5/all.hpp>
#include <bitset>
#include <cmath>

using namespace std::chrono_literals;

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

// Cost of registering and deregistering agents bound to thread pools
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

INSTANTIATE_TEST_SUITE_P(benchmarks_cooperation_fifo, skynet_thread_pool_benchmarks,
	testing::Values(
		skynet_bench_params{ .thread_pool_size=1, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=2, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=3, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=4, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation },
		skynet_bench_params{ .thread_pool_size=8, .fifo_strategy=so_5::disp::thread_pool::fifo_t::cooperation }
));

INSTANTIATE_TEST_SUITE_P(benchmarks_individual_fifo, skynet_thread_pool_benchmarks,
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

// Cost of exchanging messages in 1:1 interaction - pinger and ponger on the same thread
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

// Cost of exchanging messages in 1:1 interaction - pinger and ponger on dedicated threads
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

// Cost of exchanging messages in 1:1 interaction - pinger and ponger on thread pool (size=2, cooperation fifo)
TEST(benchmarks, ping_pong_thread_pool_cooperation_fifo)
{
	so_5::launch([](so_5::environment_t& env) {
		env.introduce_coop(so_5::disp::thread_pool::make_dispatcher(env, 2).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::cooperation)), [&](so_5::coop_t& coop) {
			const auto pinger_agent = coop.make_agent<pinger>(100000);
			const auto ponger_agent = coop.make_agent<ponger>();
			pinger_agent->set_ponger_channel(ponger_agent->so_direct_mbox());
			ponger_agent->set_pinger_channel(pinger_agent->so_direct_mbox());
		});
	});
}

// Cost of exchanging messages in 1:1 interaction - pinger and ponger on thread pool (size=2, individual fifo)
TEST(benchmarks, ping_pong_thread_pool_individual_fifo)
{
	so_5::launch([](so_5::environment_t& env) {
		env.introduce_coop(so_5::disp::thread_pool::make_dispatcher(env, 2).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual)), [&](so_5::coop_t& coop) {
			const auto pinger_agent = coop.make_agent<pinger>(100000);
			const auto ponger_agent = coop.make_agent<ponger>();
			pinger_agent->set_ponger_channel(ponger_agent->so_direct_mbox());
			ponger_agent->set_pinger_channel(pinger_agent->so_direct_mbox());
		});
	});
}

class pinger_named final : public so_5::agent_t
{
public:
	pinger_named(so_5::agent_context_t ctx, unsigned count)
		: agent_t(std::move(ctx)), m_pings_count(count), m_pings_left(count), m_ponger_channel(so_environment().create_mbox("ponger"))
	{
	}

	void so_define_agent() override
	{
		so_subscribe(so_environment().create_mbox("pinger")).event([this](so_5::mhood_t<pong_signal>) {
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
			std::cout << std::format("ping-pong count={} elapsed={} throughput={:.2f} mex/s real-throughput={:.2f} mex/s\n", m_pings_count, diff, freq, freq * 2);
			so_environment().stop();
		}
	}

	std::chrono::time_point<std::chrono::steady_clock> m_start;
	unsigned m_pings_count;
	unsigned m_pings_left;
	so_5::mbox_t m_ponger_channel;
};

class ponger_named final : public so_5::agent_t
{
public:
	ponger_named(so_5::agent_context_t ctx)
		: agent_t(std::move(ctx)), m_pinger_channel(so_environment().create_mbox("pinger"))
	{
	}

	void so_define_agent() override
	{
		so_subscribe(so_environment().create_mbox("ponger")).event([this](so_5::mhood_t<ping_signal>) {
			so_5::send<pong_signal>(m_pinger_channel);
		});
	}

private:
	so_5::mbox_t m_pinger_channel;
};

// Cost of exchanging messages in 1:1 interaction using named channels (multi-producer multi-consumer) - pinger and ponger on the same thread
TEST(benchmarks, ping_pong_named_channels)
{
	so_5::launch([](so_5::environment_t& env) {
		env.introduce_coop([&](so_5::coop_t& coop) {
			coop.make_agent<pinger_named>(100000);
			coop.make_agent<ponger_named>();
		});
	});
}

//// 1:N messaging

template<unsigned workers_count>
class producer final : public so_5::agent_t
{
public:
	producer(so_5::agent_context_t c, unsigned message_count)
		: agent_t(std::move(c)), m_message_count(message_count)
	{

	}

	void so_evt_start() override
	{
		const auto tic = std::chrono::steady_clock::now();
		const auto destination = so_environment().create_mbox("input");
		for (unsigned i = 0; i < m_message_count; ++i)
		{
			so_5::send<unsigned>(destination, i);
		}
		const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - tic).count();
		std::cout << std::format("1:N benchmark (message_count={} worker_count={}) => sending data: elapsed={}\n", m_message_count, workers_count, elapsed);
	}

	void so_define_agent() override
	{
		so_subscribe_self().event([this](unsigned worker_id) {
			m_done_received[worker_id] = true;
			if (m_done_received.all())
			{
				so_environment().stop();
			}
		});
	}
private:
	unsigned m_message_count;
	std::bitset<workers_count> m_done_received;
};

class noop_worker final : public so_5::agent_t
{
public:
	noop_worker(so_5::agent_context_t c, unsigned message_count, unsigned worker_id, so_5::mbox_t dest)
		: agent_t(std::move(c)), m_message_count(message_count), m_worker_id(worker_id), m_input(so_environment().create_mbox("input")), m_output(std::move(dest))
	{

	}

	void so_define_agent() override
	{
		so_subscribe(m_input).event([this](unsigned msg) {
			if (--m_message_count == 0)
			{
				so_5::send<unsigned>(m_output, m_worker_id);
			}
		});
	}
private:
	unsigned m_message_count;
	unsigned m_worker_id;
	so_5::mbox_t m_input;
	so_5::mbox_t m_output;
};

template<unsigned workers_count, typename worker_type>
void do_messaging_one_to_many_benchmark(unsigned message_count, auto workers_dispatcher_maker)
{
	const auto tic = std::chrono::steady_clock::now();

	so_5::launch([=](so_5::environment_t& env) {
		const auto workers_dispatcher = workers_dispatcher_maker(env);
		env.introduce_coop([&](so_5::coop_t& coop) {
			const auto master_mbox = coop.make_agent<producer<workers_count>>(message_count)->so_direct_mbox();
			
			for (auto i = 0u; i<workers_count; ++i)
			{
				coop.make_agent_with_binder<worker_type>(workers_dispatcher, message_count, i, master_mbox);
			}
		});
	});

	const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - tic).count();
	std::cout << std::format("1:N benchmark (message_count={} worker_count={}) => overall: elapsed={}\n", message_count, workers_count, elapsed);
}

TEST(benchmarks, messaging_one_to_many_noop_shared_thread)
{
	do_messaging_one_to_many_benchmark<100, noop_worker>(100000, [](so_5::environment_t& env) { return so_5::disp::active_group::make_dispatcher(env).binder("workers"); });
}

TEST(benchmarks, messaging_one_to_many_noop_dedicated_thread)
{
	do_messaging_one_to_many_benchmark<100, noop_worker>(100000, [](so_5::environment_t& env) { return so_5::disp::active_obj::make_dispatcher(env).binder(); });
}

TEST(benchmarks, messaging_one_to_many_noop_cooperation_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<100, noop_worker>(100000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::cooperation)); });
}

TEST(benchmarks, messaging_one_to_many_noop_individual_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<100, noop_worker>(100000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual)); });
}

TEST(benchmarks, messaging_one_to_many_noop_individual_size_4_thread_pool_max_demands_at_once_1)
{
	do_messaging_one_to_many_benchmark<100, noop_worker>(100000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual).max_demands_at_once(1)); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_noop_shared_thread)
{
	do_messaging_one_to_many_benchmark<10000, noop_worker>(1000, [](so_5::environment_t& env) { return so_5::disp::active_group::make_dispatcher(env).binder("workers"); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_noop_dedicated_thread)
{
	do_messaging_one_to_many_benchmark<10000, noop_worker>(1000, [](so_5::environment_t& env) { return so_5::disp::active_obj::make_dispatcher(env).binder(); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_noop_cooperation_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<10000, noop_worker>(1000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::cooperation)); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_noop_individual_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<10000, noop_worker>(1000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual)); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_noop_individual_size_4_thread_pool_max_demands_at_once_1)
{
	do_messaging_one_to_many_benchmark<10000, noop_worker>(1000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual).max_demands_at_once(1)); });
}

class sleeping_worker final : public so_5::agent_t
{
public:
	sleeping_worker(so_5::agent_context_t c, unsigned message_count, unsigned worker_id, so_5::mbox_t dest)
		: agent_t(std::move(c)), m_message_count(message_count), m_worker_id(worker_id), m_input(so_environment().create_mbox("input")), m_output(std::move(dest))
	{

	}

	void so_define_agent() override
	{
		so_subscribe(m_input).event([this](unsigned msg) {
			if (--m_message_count == 0)
			{
				so_5::send<unsigned>(m_output, m_worker_id);
			}
			std::this_thread::sleep_for(5ms);
		});
	}
private:
	unsigned m_message_count;
	unsigned m_worker_id;
	so_5::mbox_t m_input;
	so_5::mbox_t m_output;
};

TEST(benchmarks, messaging_one_to_many_low_workers_count_low_message_count_sleeping_shared_thread)
{
	do_messaging_one_to_many_benchmark<100, sleeping_worker>(100, [](so_5::environment_t& env) { return so_5::disp::active_group::make_dispatcher(env).binder("workers"); });
}

TEST(benchmarks, messaging_one_to_many_low_workers_count_low_message_count_sleeping_dedicated_thread)
{
	do_messaging_one_to_many_benchmark<100, sleeping_worker>(100, [](so_5::environment_t& env) { return so_5::disp::active_obj::make_dispatcher(env).binder(); });
}

TEST(benchmarks, messaging_one_to_many_low_workers_count_low_message_count_sleeping_cooperation_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<100, sleeping_worker>(100, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::cooperation)); });
}

TEST(benchmarks, messaging_one_to_many_low_workers_count_low_message_count_sleeping_individual_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<100, sleeping_worker>(100, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual)); });
}

class cpu_worker final : public so_5::agent_t
{
public:
	cpu_worker(so_5::agent_context_t c, unsigned message_count, unsigned worker_id, so_5::mbox_t dest)
		: agent_t(std::move(c)), m_message_count(message_count), m_worker_id(worker_id), m_input(so_environment().create_mbox("input")), m_output(std::move(dest))
	{

	}

	void so_define_agent() override
	{
		so_subscribe(m_input).event([this](unsigned msg) {
			series += std::sph_neumann(msg, 1.2345);
			if (--m_message_count == 0)
			{
				so_5::send<unsigned>(m_output, m_worker_id);
			}
		});
	}
private:
	unsigned m_message_count;
	unsigned m_worker_id;
	so_5::mbox_t m_input;
	so_5::mbox_t m_output;
	double series = 0.0;
};

TEST(benchmarks, messaging_one_to_many_low_workers_count_huge_message_count_cpu_usage_shared_thread)
{
	do_messaging_one_to_many_benchmark<100, cpu_worker>(10000, [](so_5::environment_t& env) { return so_5::disp::active_group::make_dispatcher(env).binder("workers"); });
}

TEST(benchmarks, messaging_one_to_many_low_workers_count_huge_message_count_cpu_usage_dedicated_thread)
{
	do_messaging_one_to_many_benchmark<100, cpu_worker>(10000, [](so_5::environment_t& env) { return so_5::disp::active_obj::make_dispatcher(env).binder(); });
}

TEST(benchmarks, messaging_one_to_many_low_workers_count_huge_message_count_cpu_usage_cooperation_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<100, cpu_worker>(10000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::cooperation)); });
}

TEST(benchmarks, messaging_one_to_many_low_workers_count_huge_message_count_cpu_usage_individual_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<100, cpu_worker>(10000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual)); });
}

TEST(benchmarks, messaging_one_to_many_low_workers_count_huge_message_count_cpu_usage_individual_size_16_thread_pool)
{
	do_messaging_one_to_many_benchmark<100, cpu_worker>(10000, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual)); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_cpu_usage_dedicated_thread)
{
	do_messaging_one_to_many_benchmark<10000, cpu_worker>(100, [](so_5::environment_t& env) { return so_5::disp::active_obj::make_dispatcher(env).binder(); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_cpu_usage_individual_size_4_thread_pool)
{
	do_messaging_one_to_many_benchmark<10000, cpu_worker>(100, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual)); });
}

TEST(benchmarks, messaging_one_to_many_huge_workers_count_low_message_count_cpu_usage_individual_size_4_thread_pool_max_demands_at_once_1)
{
	do_messaging_one_to_many_benchmark<10000, cpu_worker>(100, [](so_5::environment_t& env) { return so_5::disp::thread_pool::make_dispatcher(env, 4).binder(so_5::disp::thread_pool::bind_params_t{}.fifo(so_5::disp::thread_pool::fifo_t::individual).max_demands_at_once(1)); });
}
