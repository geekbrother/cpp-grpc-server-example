#include "sample.grpc.pb.h"
#include "sample.pb.h"
#include <grpc++/grpc++.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

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
  Status SampleStreamMethod(ServerContext *context, const SampleRequest *request, grpc::ServerWriter<SampleResponse> *writer) override
  {
    std::mutex writerMutex;
    std::atomic_bool writerIsReady = true;

    auto respondToWriter = [&](SampleResponse response)
    {
      std::lock_guard<std::mutex> lock(writerMutex);
      if (!writer->Write(response))
      {
        writerIsReady = false;
        return false;
      }
      return true;
    };

    // Pinging thread
    auto sendPings = [&]()
    {
      SampleResponse response;
      response.mutable_ping();
      while (true)
      {
        if (!writerIsReady)
        {
          return;
        }
        if (!respondToWriter(response))
        {
          std::cerr << "gRPC: 'Get' writer error on sending data to the client in `sendpings`" << std::endl;
          return;
        }
        std::cout << "Ping sent" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    };
    std::thread ping_thread(sendPings);

    try
    {
      int i;
      while (true)
      {
        i++;
        SampleResponse response;
        response.mutable_sample_field()->set_response_sample_field("Hello " + request->request_sample_field());
        if (!writerIsReady)
        {
          ping_thread.join();
          std::cerr << "gRPC: Writer is not ready to send response" << std::endl;
          return grpc::Status(grpc::StatusCode::INTERNAL, "got error from Writer");
        }
        if (!respondToWriter(response))
        {
          ping_thread.join();
          std::cerr << "gRPC: 'Get' writer error on sending data to the client in `responses`" << std::endl;
          return grpc::Status(grpc::StatusCode::INTERNAL, "got error from Writer");
        }
        std::cout << "Response " << i << " to stream" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
      }
    }
    catch (...)
    {
      ping_thread.join();
      std::cerr << "Got error from grpc." << std::endl;
      return grpc::Status(grpc::StatusCode::INTERNAL, "got error from Writer");
    }
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
