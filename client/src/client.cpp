#include "sample.grpc.pb.h"
#include <grpc++/grpc++.h>
#include <memory>
#include <iostream>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using sample::SampleRequest;
using sample::SampleResponse;
using sample::SampleService;

class SampleClient
{
public:
  SampleClient(std::shared_ptr<Channel> channel) : _stub{SampleService::NewStub(channel)} {}

  void SampleMethod(const std::string &request_sample_field)
  {
    // Prepare request
    SampleRequest request;
    request.set_request_sample_field(request_sample_field);

    // Send request
    SampleResponse response;
    ClientContext context;
    std::unique_ptr<::grpc::ClientReader<::sample::SampleResponse>> reader(_stub->SampleStreamMethod(&context, request));
    while (reader->Read(&response))
    {
      if (!response.response_sample_field().empty())
      {
        std::cout << "Found response_sample_field "
                  << response.response_sample_field() << std::endl;
      }
      if (response.ping())
      {
        std::cout << "Found ping "
                  << response.ping() << std::endl;
      }
    }
  }

private:
  std::unique_ptr<SampleService::Stub> _stub;
};

int main(int argc, char **argv)
{
  grpc::ChannelArguments chArgs = grpc::ChannelArguments();
  // chArgs.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 1000);
  // chArgs.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 1000);
  // chArgs.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
  // chArgs.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);
  // chArgs.SetInt(GRPC_ARG_HTTP2_MIN_SENT_PING_INTERVAL_WITHOUT_DATA_MS, 1000);
  // chArgs.SetInt(GRPC_ARG_HTTP2_BDP_PROBE, 0);

  std::string server_address{"localhost:2510"};
  SampleClient client{grpc::CreateCustomChannel(server_address, grpc::InsecureChannelCredentials(), chArgs)};
  std::string request_sample_field{"world"};
  client.SampleMethod(request_sample_field);
  return 0;
}
