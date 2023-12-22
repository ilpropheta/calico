#pragma once
#include <so_5/all.hpp>
#include "service.grpc.pb.h"
#include <grpc++/server_builder.h>

namespace calico
{
	struct service_params
	{
		so_5::environment_t& environment;
	};

	// implements calico_service by using the supplied 'service_params'.
	// The generated files are produced automatically every time the proto file changes
	class service_impl : public calico_service::Service
	{
	public:
		explicit service_impl(service_params params);
		grpc::Status send_command(grpc::ServerContext* context, const send_command_request* request, send_command_response* response) override;
	private:
		service_params m_params;
	};
}

namespace calico::agents
{
	// brings calico to grpc through service_impl.
	// It serves as an abstraction that conceals the intricacies of grpc.
	// Essentially, it is responsible for initiating and terminating a grpc server
	// (respectively in so_evt_start() and so_evt_finish()),
	// wherein calico_impl has been registered at 0.0.0.0::50051
	class service_facade final : public so_5::agent_t
	{
	public:
		explicit service_facade(so_5::agent_context_t ctx);
		void so_evt_start() override;
		void so_evt_finish() override;
	private:
		std::unique_ptr<grpc::Server> m_server;
		service_impl m_service_impl;
	};
}