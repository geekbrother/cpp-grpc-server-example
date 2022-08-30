#include "sample.grpc.pb.h"
#include <grpc++/grpc++.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using sample::SampleRequest;
using sample::SampleResponse;
using sample::SampleService;

class SampleServiceImpl final : public SampleService::Service
{
  Status SampleStreamMethod(ServerContext *context, const SampleRequest *request, grpc::ServerWriter<SampleResponse> *writer) override
  {
    try
    {
      std::this_thread::sleep_for(std::chrono::seconds(10));
      for (int i = 0; i < 10; ++i)
      {
        SampleResponse response;
        response.set_response_sample_field("Hello " + request->request_sample_field());
        if (!writer->Write(response))
        {
          std::cerr << "Got error from Writer" << std::endl;
          return grpc::Status(grpc::StatusCode::INTERNAL, "got error from Writer");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Response " << i << " to stream" << std::endl;
      }
      return grpc::Status(grpc::StatusCode::INTERNAL, "got error from Writer");
    }
    catch (...)
    {
      std::cerr << "Got error from end stream" << std::endl;
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
  builder.RegisterService(&service);
  std::unique_ptr<Server> server{builder.BuildAndStart()};

  // Run server
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer();
  return 0;
}
