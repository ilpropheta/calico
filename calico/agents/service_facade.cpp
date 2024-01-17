#include "service_facade.h"
#include <opencv2/imgcodecs.hpp>
#include "../signals.h"

using namespace std::chrono_literals;

// handles a single client subscription
class subscribe_client_agent final : public so_5::agent_t
{
	struct check_client_disconnection final : so_5::signal_t {};
public:
	subscribe_client_agent(so_5::agent_context_t ctx, grpc::ServerContext& context, grpc::ServerWriter<subscribe_response>& writer, std::vector<so_5::mbox_t> channels)
		: agent_t(std::move(ctx)), m_context(context), m_writer(writer), m_channels(std::move(channels))
	{
	}

	void so_evt_start() override
	{
		m_check_disconnection_timer = so_5::send_periodic<check_client_disconnection>(so_direct_mbox(), 5s, 5s);
	}

	void so_define_agent() override
	{
		for (const auto& channel : m_channels)
		{
			so_subscribe(channel).event([chan_name = channel->query_name(), this](const cv::Mat& image) {
				subscribe_response response;
				response.set_channel_name(chan_name);
				imencode(".jpg", image, m_raw_data, { cv::IMWRITE_JPEG_QUALITY, 95 });
				response.mutable_image()->set_data(m_raw_data.data(), m_raw_data.size());
				if (!m_writer.Write(response))
				{
					so_deactivate_agent();
					so_deregister_agent_coop_normally();
				}
			});
		}

		so_subscribe_self().event([this](so_5::mhood_t<check_client_disconnection>) {
			if (m_context.IsCancelled())
			{
				so_deactivate_agent();
				so_deregister_agent_coop_normally();
			}
		});
	}

	std::future<grpc::Status> get_status_future()
	{
		return m_status_promise.get_future();
	}

	void so_evt_finish() override
	{
		m_status_promise.set_value(grpc::Status::OK);
	}

private:
	grpc::ServerContext& m_context;
	grpc::ServerWriter<subscribe_response>& m_writer;
	std::vector<so_5::mbox_t> m_channels;
	so_5::timer_id_t m_check_disconnection_timer;
	std::promise<grpc::Status> m_status_promise;
	std::vector<uchar> m_raw_data;
};

calico::service_impl::service_impl(service_params params)
	: m_params(params), m_binder(so_5::disp::thread_pool::make_dispatcher(m_params.environment, 4).binder())
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

static std::vector<so_5::mbox_t> get_channels_from(so_5::environment_t& env, const subscribe_request& request)
{
	std::vector<so_5::mbox_t> channels(request.channels_size());
	std::ranges::transform(request.channels(), begin(channels), [&](const auto& name) {
		return env.create_mbox(name);
	});
	return channels;
}

grpc::Status calico::service_impl::subscribe(grpc::ServerContext* context, const subscribe_request* request, grpc::ServerWriter<subscribe_response>* writer)
{
	return m_params.environment.introduce_coop(m_binder, [&, this](so_5::coop_t& coop) {
		return coop.make_agent<subscribe_client_agent>(*context, *writer, get_channels_from(m_params.environment, *request))->get_status_future();
	}).get();
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