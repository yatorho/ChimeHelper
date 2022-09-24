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

class DisServiceClient {
public:
  void Register(const ::dis::core::Address &address) {
    for (auto &pair : _stubs) {
      if (pair.first == address) {
        return;
      }
    }

    grpc::ChannelArguments channel_args;
    channel_args.SetInt("grpc.max_receive_message_length",
                        std::numeric_limits<int>::max());

    std::string add_s = address.ip + ":" + std::to_string(address.port);
    std::shared_ptr<Channel> channel = grpc::CreateCustomChannel(
        add_s, grpc::InsecureChannelCredentials(), channel_args);
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

  std::unique_ptr<DisService::Stub> *FindStub(const dis::core::Address &addr) {
    for (auto &pair : _stubs) {
      if (pair.first == addr) {
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

  dis::core::Matrix *GetMatrixFromRemote(const dis::core::Address &addr,
                                         const std::string &target_name,
                                         const std::string &result_name,
                                         int row_beginl, int row_endl,
                                         int col_beginl, int col_endl) {
    ClientContext context;
    ::dis::MatrixRemoteSynceInfo info;
    info.set_name(target_name);
    info.set_row_beginl(row_beginl);
    info.set_row_endl(row_endl);
    info.set_col_beginl(col_beginl);
    info.set_col_endl(col_endl);

    ::dis::Matrix reply_matrix;

    ::grpc::Status g_status = FindStub(addr)->get()->GetMatrixFromRemote(
        &context, info, &reply_matrix);
    if (!g_status.ok()) {
      LOG(ERROR) << "GRPC SendAddress failed: " << g_status.error_message();
      return nullptr;
    }

    DCHECK(reply_matrix.rows() == (row_endl - row_beginl));
    DCHECK(reply_matrix.cols() == (col_endl - col_beginl));
    DCHECK(reply_matrix.data_size() ==
           (row_endl - row_beginl) * (col_endl - col_beginl));

    dis::core::Matrix *result = new dis::core::Matrix(
        reply_matrix.rows(), reply_matrix.cols(), result_name);

    for (int i = 0; i < result->rows * result->cols; i++) {
      result->data.get()[i] = reply_matrix.data(i);
    }

    return result;
  }

private:
  std::vector<
      std::pair<::dis::core::Address, std::unique_ptr<DisService::Stub>>>
      _stubs;
};

void SynceEnv() {
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
    // sleep for 10ms
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
      // sleep for 200ms
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    LOG(INFO) << "EnvStart success! ";
  }

  // Registe stub from global addressmap
  for (auto &pair : ::dis::core::GetGlobalAddressMap()) {
    client->Register(pair.second);
  }
}

void WorkerLoop() { SynceEnv();}

} // namespace dis
