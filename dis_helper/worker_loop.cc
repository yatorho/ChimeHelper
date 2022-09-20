#include "dis_helper/env.pb.h"
#include "dis_helper/worker_loop.h"

#include <glog/logging.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <thread>

#include "dis_helper/dis_info.h"
#include "dis_helper/env.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;

namespace dis {

class DisServiceClient {
public:
  void Register(const ::dis::core::Address &address) {
    for (auto &pair : _stubs) {
      if (pair.first == address) {
        return;
      }
    }

    std::string add_s = address.ip + ":" + std::to_string(address.port);
    std::shared_ptr<Channel> channel =
        grpc::CreateChannel(add_s, grpc::InsecureChannelCredentials());
    auto stub = dis::DisService::NewStub(channel);

    _stubs.emplace_back(::dis::core::Address(address), std::move(stub));
  }

  ::dis::core::AddressMap GetAddressMap() {
    ClientContext context;
    ::dis::Empty empty;
    ::dis::AddressMap address_map;

    ::grpc::Status status =
        FindMasterStub()->get()->GetAddressMap(&context, empty, &address_map);
    if (!status.ok()) {
      LOG(ERROR) << "GetAddressMap failed: " << status.error_message();
    }

    ::dis::core::AddressMap result;
    for (auto &pair : address_map.address_map()) {
      result[pair.first] =
          ::dis::core::Address{pair.second.port(), pair.second.ip()};
    }
    return result;
  }

  std::unique_ptr<DisService::Stub> *FindMasterStub() {
    ::dis::core::Address master_address{
        std::stoi(std::string(getenv("MASTER_PORT"))),
        std::string(getenv("MASTER_ADDRESS"))};
    for (auto &pair : _stubs) {
      if (pair.first == master_address) {
        return &pair.second;
      }
    }
    return nullptr;
  }

  bool SendAddress(const ::dis::core::Address &addr) {
    ClientContext context;
    ::dis::Address address;
    address.set_port(addr.port);
    address.set_ip(addr.ip);

    ::dis::Status status;

    ::grpc::Status g_status =
        FindMasterStub()->get()->SendAddress(&context, address, &status);
    if (!g_status.ok()) {
      LOG(ERROR) << "GRPC SendAddress failed: " << g_status.error_message();
      return false;
    }
    if (!status.sucess()) {
      LOG(ERROR) << "SendAddress failed! ";
      return false;
    }
    LOG(INFO) << status.message();
    return true;
  }

private:
  std::vector<
      std::pair<::dis::core::Address, std::unique_ptr<DisService::Stub>>>
      _stubs;
};

void SynceEnv() {
  std::string address = std::string(getenv("MASTER_ADDRESS")) + ":" +
                        std::string(getenv("MASTER_PORT"));

  grpc::ChannelArguments channel_args;
  channel_args.SetInt("grpc.max_receive_message_length",
                      std::numeric_limits<int>::max());

  DisServiceClient *client = new DisServiceClient;
  client->Register(
      ::dis::core::Address{std::stoi(std::string(getenv("MASTER_PORT"))),
                           std::string(getenv("MASTER_ADDRESS"))});

  CHECK(client->SendAddress(
      ::dis::core::Address{std::stoi(std::string(getenv("LOCAL_PORT"))),
                           std::string(getenv("LOCAL_ADDRESS"))}));
  // sleep for 10ms
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  ::dis::core::AddressMap address_map = client->GetAddressMap();
  
  ::dis::core::SetGlobalAddressMap(address_map);
  LOG(INFO) << "EnvStart success! ";
}

void WorkerLoop() { 
  SynceEnv(); 
}

} // namespace dis