#pragma once
#include <so_5/all.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>

namespace calico::agents
{
	namespace utils
	{
		// Manages the given stop_guard in a RAII fashion
		struct guard_remover
		{
			guard_remover(so_5::environment_t& env, so_5::stop_guard_shptr_t guard)
				: m_env(env), m_guard(std::move(guard))
			{
				m_env.setup_stop_guard(m_guard);
			}

			guard_remover(const guard_remover&) = delete;
			guard_remover(guard_remover&&) = delete;
			guard_remover& operator=(const guard_remover&) = delete;
			guard_remover& operator=(guard_remover&&) = delete;

			~guard_remover()
			{
				remove();
			}

			void remove() const
			{
				m_env.remove_stop_guard(m_guard);
			}

			so_5::environment_t& m_env;
			so_5::stop_guard_shptr_t m_guard;
		};
	}

	// Caches exactly 'size' images and then sends them to the output channel, one by one.
	// At the shutdown, any remaining data will be delivered to the output channel in any case.
	class image_cache final : public so_5::agent_t
	{
		struct shutdown_requested final : so_5::signal_t {};

		class cache_stop_guard final : public so_5::stop_guard_t
		{
		public:
			cache_stop_guard(so_5::mbox_t channel);
			void stop() noexcept override;
		private:
			so_5::mbox_t m_channel;
		};
	public:
		image_cache(so_5::agent_context_t ctx, so_5::mbox_t input, so_5::mbox_t output, unsigned size);
		void so_define_agent() override;
	private:
		void flush();

		so_5::mbox_t m_input;
		so_5::mbox_t m_output;
		unsigned m_session_size;
		unsigned m_accumulated;
		std::vector<so_5::message_holder_t<cv::Mat>> m_cache;
		utils::guard_remover m_guard_remover;
	};
}