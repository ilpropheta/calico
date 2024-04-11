#include "calico.h"
#include "utils.h"
#include "constants.h"
#include "gui_handling.h"
#include "agents/error_logger.h"
#include "agents/face_detector.h"
#include "agents/fps_estimator.h"
#include "agents/image_resizer.h"
#include "agents/image_tracer.h"
#include "agents/image_viewer.h"
#include "agents/remote_control.h"
#include "agents/service_facade.h"
#include "agents/stream_heartbeat.h"
#include "agents/telemetry_agent.h"
#include "producers/image_producer.h"
#include "manager/agent_manager.h"

int calico::run()
{
	const auto ctrl_c = utils::get_ctrlc_token();

	agent_manager manager;
	auto session = manager.create_session("webcam");

	const auto main_channel = session.get_channel("main");
	const auto commands_channel = session.get_channel("commands");
	const auto message_queue = session.make_chain();
	const auto waitkey_out = session.get_channel(constants::waitkey_channel_name);

	session.add_dedicated_thread_agent<producers::image_producer>(main_channel, commands_channel);
	session.add_dedicated_thread_agent<agents::service_facade>();

	session.add_monitoring_agent<agents::image_tracer>(main_channel);
	session.add_monitoring_agent<agents::fps_estimator>(std::vector{ main_channel });
	session.add_monitoring_agent<agents::telemetry_agent>();
	session.add_monitoring_agent<agents::stream_heartbeat>(main_channel);
	session.add_monitoring_agent<agents::error_logger>(main_channel);

	session.add_core_agent<agents::maint_gui::remote_control>(commands_channel, message_queue);

	auto resized = session.get_channel();
	session.add_core_agent<agents::image_resizer>(main_channel, resized, 0.5);
	auto faces = session.get_channel();
	session.add_core_agent<agents::face_detector>(resized, faces);

	session.add_core_agent<agents::maint_gui::image_viewer>(faces, message_queue);

	do_gui_message_loop(ctrl_c, message_queue, waitkey_out);
	return 0;
}
