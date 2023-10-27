#pragma once
#include <so_5/all.hpp>

namespace calico::producers
{
	// grabs frames from the default camera (e.g. webcam) and sends them to the specified message box
	// reacts to 'start_acquisition_command' and 'stop_acquisition_command' to control the acquisition
	class image_producer final : public so_5::agent_t
	{
		enum class acquisition_state { st_started, st_stopped, st_cancelled };

		class image_producer_broker : public agent_t
		{
		public:
			image_producer_broker(so_5::agent_context_t c, image_producer* parent, so_5::mbox_t commands);
			void so_define_agent() override;
			void so_evt_finish() override;
		private:
			image_producer* m_parent;
			so_5::mbox_t m_commands;
		};

	public:
		image_producer(so_5::agent_context_t ctx, so_5::mbox_t channel, so_5::mbox_t commands);
		void so_evt_start() override;
	private:
		void start();
		void stop();
		void cancel();
		void change_state(acquisition_state st);

		so_5::mbox_t m_channel;
		so_5::mbox_t m_commands;
		std::atomic<acquisition_state> m_state = acquisition_state::st_stopped;
	};
}
