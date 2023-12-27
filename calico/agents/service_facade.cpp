#include "service_facade.h"
#include "../signals.h"

calico::service_impl::service_impl(service_params params)
	: m_params(params)
{
}

grpc::Status calico::service_impl::send_command(grpc::ServerContext* context, const send_command_request* request, send_command_response* response)
{
	const auto& channel_name = request->channel_name();
	const auto& command_name = request->command_name();
	auto status = grpc::Status::OK;

	if (command_name == "start_acquisition")
	{
		so_5::send<start_acquisition_command>(m_params.environment.create_mbox(channel_name));
	}
	else if (command_name == "stop_acquisition")
	{
		so_5::send<stop_acquisition_command>(m_params.environment.create_mbox(channel_name));
	}
	else
	{
		status = grpc::Status{ grpc::StatusCode::INVALID_ARGUMENT, std::format("command {} not found", command_name) };
	}
	return status;
}

calico::agents::service_facade::service_facade(so_5::agent_context_t ctx)
	: agent_t(std::move(ctx)), m_service_impl({ so_environment() })
{
}

void calico::agents::service_facade::so_evt_start()
{
	grpc::ServerBuilder builder;
	builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
	builder.RegisterService(&m_service_impl);
	m_server = builder.BuildAndStart();
}

void calico::agents::service_facade::so_evt_finish()
{
	m_server->Shutdown();
	m_server->Wait();
}