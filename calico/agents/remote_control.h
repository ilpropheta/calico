#pragma once
#include <so_5/all.hpp>

namespace calico::agents
{
	// displays a tiny frame where "Enter", "Escape", and "t" are turned, respectively, into 'start_acquisition_command'
	// 'stop_acquisition_command' and 'enable_telemetry_command', and are sent to the input channel
	class remote_control final : public so_5::agent_t
	{
		struct keep_on final : so_5::signal_t {};
	public:
		remote_control(so_5::agent_context_t ctx, so_5::mbox_t commands);
		void so_evt_start() override;
		void so_define_agent() override;
	private:
		so_5::mbox_t m_channel;
	};

	namespace maint_gui
	{
		// displays a tiny frame where "Enter", "Escape" and "t" are turned, respectively, into 'start_acquisition_command'
		// 'stop_acquisition_command', and 'enable_telemetry_command', and are sent to the input channel
		// This version uses the message loop approach, sending an initial imshow_message to 'ui_queue' and
		// receiving waitkey_messages from 'constants::waitkey_out' channel
		class remote_control final : public so_5::agent_t
		{
		public:
			remote_control(so_5::agent_context_t ctx, so_5::mbox_t commands, so_5::mchain_t ui_queue);
			void so_evt_start() override;
			void so_define_agent() override;
		private:
			so_5::mbox_t m_channel;
			so_5::mchain_t m_message_queue;
			so_5::mbox_t m_waitkey_channel;
		};
	}
}
