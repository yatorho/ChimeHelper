#include "dis_helper/worker_loop.h"
#include "dis_helper/env.pb.h"

#include <glog/logging.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <string>
#include <thread>

#include "dis_helper/dis_info.h"
#include "dis_helper/env.grpc.pb.h"
#include "dis_helper/matrix.h"

using grpc::Channel;
using grpc::ClientContext;

namespace dis {

DisServiceClient* SynceEnv() {
  ::dis::core::Address local_address{
      std::stoi(std::string(getenv("LOCAL_PORT"))),
      std::string(getenv("LOCAL_ADDRESS"))};
  ::dis::core::Address master_address{
      std::stoi(std::string(getenv("MASTER_PORT"))),
      std::string(getenv("MASTER_ADDRESS"))};

  grpc::ChannelArguments channel_args;
  channel_args.SetInt("grpc.max_receive_message_length",
                      std::numeric_limits<int>::max());

  DisServiceClient *client = new DisServiceClient;
  client->Register(master_address);

  if (!(local_address == master_address)) {
    CHECK(client->SendAddress(local_address));
    // Sleep for 10ms
    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      ::dis::core::AddressMap address_map = client->GetAddressMap();
      if (address_map.size() ==
          static_cast<size_t>(std::stoi(std::string(getenv("WORLD_SIZE"))))) {
        ::dis::core::GetGlobalAddressMap() = address_map;
        LOG(INFO) << "Sync Env Success!";
        break;
      }
    }
  } else {
    while (::dis::core::GetGlobalAddressMap().size() !=
           static_cast<size_t>(std::stoi(std::string(getenv("WORLD_SIZE"))))) {
      // Sleep for 200ms
      // TODO(dza): It occupies too many resources.
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    LOG(INFO) << "EnvStart success! ";
  }

  // Registe stub from global addressmap
  for (auto &pair : ::dis::core::GetGlobalAddressMap()) {
    client->Register(pair.second);
  }

  return client;

}

void WorkerLoop() { SynceEnv(); }

} // namespace dis
