#include "dis_helper/worker_loop.h"
#include "dis_helper/env.pb.h"

#include <glog/logging.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <string>
#include <thread>

#include "dis_helper/dis_info.h"
#include "dis_helper/env.grpc.pb.h"



int main () {

 grpc::ChannelArguments channel_args;
  channel_args.SetInt("grpc.max_receive_message_length",
                      std::numeric_limits<int>::max());

  auto stub = dis::DisService::NewStub(grpc::CreateCustomChannel(
      "127.0.1.1:50051", grpc::InsecureChannelCredentials(), channel_args));

  grpc::ClientContext context;
  ::dis::Address address;
  address.set_port(3424);
  address.set_ip("fff");
  ::dis::Status status;
  
  auto g_s = stub->SendAddress(&context, address, &status);
  if (!g_s.ok()) {
    LOG(ERROR) << "SendAddress failed: " << g_s.error_message();
  }
}
