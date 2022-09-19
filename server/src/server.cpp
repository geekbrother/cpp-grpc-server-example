#include "sample.grpc.pb.h"
#include <grpc++/grpc++.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using sample::SampleRequest;
using sample::SampleResponse;
using sample::SampleService;

class SampleServiceImpl final : public SampleService::Service
{
private:
  std::mutex writerMutex;
  bool respondToWriter(ServerContext *context, grpc::ServerWriter<SampleResponse> *writer, SampleResponse response)
  {
    std::lock_guard<std::mutex> lock(this->writerMutex);
    if (context->IsCancelled())
    {
      std::cerr << "Context was cancelled on write" << std::endl;
      return false;
    }
    if (!writer->Write(response))
    {
      return false;
    }
    return true;
  }
  Status SampleStreamMethod(ServerContext *context, const SampleRequest *request, grpc::ServerWriter<SampleResponse> *writer) override
  {
    try
    {
      auto sendPings =
          [&]()
      {
        SampleResponse response;
        response.set_ping(true);
        while (true)
        {
          if (!this->respondToWriter(context, writer, response))
          {
            std::cerr << "gRPC: 'Get' writer error on sending data to the client in `respondToWriter`" << std::endl;
            context->TryCancel();
            return;
          }
          std::cout << "Ping sent" << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds(3));
        };
      };
      // Pinging thread
      std::thread ping_thread(sendPings);

      int i;
      while (true)
      {
        i++;
        SampleResponse response;
        response.set_response_sample_field("Hello " + request->request_sample_field());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!this->respondToWriter(context, writer, response))
        {
          std::cerr << "gRPC: 'Get' writer error on sending data to the client in `respondToWriter`" << std::endl;

          std::cerr << "Try cancel" << std::endl;
          context->TryCancel();

          std::cerr << "Try return" << std::endl;
          return grpc::Status(grpc::StatusCode::INTERNAL, "got error from Writer");
        }
        std::cout << "Response " << i << " to stream" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
      }
      ping_thread.join();
    }
    catch (...)
    {
      std::cerr << "Got error from stream" << std::endl;
    }
    return grpc::Status(grpc::StatusCode::INTERNAL, "got error from Writer");
  }
};

void RunServer()
{
  std::string server_address{"localhost:2510"};
  SampleServiceImpl service;

  // Build server
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 1000);
  // builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 1000);
  // builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
  // builder.AddChannelArgument(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0); // unlimited
  // builder.AddChannelArgument(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 500);
  // builder.AddChannelArgument(GRPC_ARG_HTTP2_BDP_PROBE, 0);

  builder.RegisterService(&service);
  std::unique_ptr<Server> server{builder.BuildAndStart()};

  // Run server
  std::cout << "Server v1 listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer();
  return 0;
}
