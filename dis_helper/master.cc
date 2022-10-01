#include "dis_helper/master.h"
#include "dis_helper/graph.h"
#include "dis_helper/matrix.h"
#include "dis_helper/worker_loop.h"
#include "dis_info.h"

#include <cstddef>
#include <grpc++/grpc++.h>
#include <map>
#include <memory>
#include <thread>

#include <glog/logging.h>

#include "dis_helper/env.grpc.pb.h"
#include "dis_helper/env.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

namespace dis {

int current_rank = 1;

class DisServiceImpl final : public dis::DisService::Service {
  ::grpc::Status GetAddressMap(::grpc::ServerContext *context,
                               const ::dis::Empty *request,
                               ::dis::AddressMap *response) override {
    auto map_ptr = response->mutable_address_map();
    CHECK(map_ptr->empty());

    for (auto &pair : ::dis::core::GetGlobalAddressMap()) {
      ::dis::Address addr;
      addr.set_port(pair.second.port);
      addr.set_ip(pair.second.ip);

      map_ptr->insert(google::protobuf::MapPair<int, ::dis::Address>(
          pair.first, std::move(addr)));
    }
    return ::grpc::Status::OK;
  }

  ::grpc::Status SendAddress(::grpc::ServerContext *context,
                             const ::dis::Address *adr,
                             ::dis::Status *status) override {
    ::dis::core::Address address{adr->port(), adr->ip()};

    ::dis::core::GetGlobalAddressMap().insert(
        std::make_pair(current_rank, std::move(address)));
    current_rank++;

    if (current_rank > std::stoi(std::string(getenv("WORLD_SIZE")))) {
      LOG(INFO) << "All workers are ready!";
      status->set_sucess(false);
      status->set_message("All workers are ready!");
      return ::grpc::Status::OK;
    }

    status->set_sucess(true);
    if (current_rank == std::stoi(std::string(getenv("WORLD_SIZE")))) {
      LOG(INFO) << "All address received";
      status->set_message("All address received");
    }
    status->set_message("Address received");
    return ::grpc::Status::OK;
  }

  ::grpc::Status GetMatrixFromRemote(::grpc::ServerContext *context,
                                     const ::dis::MatrixRemoteSynceInfo *info,
                                     ::dis::Matrix *matrix) override {

    int row_beginl = info->row_beginl();
    int row_endl = info->row_endl();
    int col_beginl = info->col_beginl();
    int col_endl = info->col_endl();
    // Check begin less than end
    CHECK(row_beginl < row_endl);
    CHECK(col_beginl < col_endl);

    matrix->mutable_data()->Resize(
        (row_endl - row_beginl) * (col_endl - col_beginl), 0);

    dis::core::Graph *graph = dis::core::GetDefaultGraph();
    dis::core::Matrix *matrix_get = graph->FindTargetMatrix(info->name());
    if (matrix_get != nullptr) {

      // Wait for matrix ready
      // Caution: this is a blocking call
      // TODO(Dza): this code may cause deadlock, `while` should be improved
      while (!matrix_get->computed)
        ;

      for (int i = row_beginl; i < row_endl; i++) {
        for (int j = col_beginl; j < col_endl; j++) {
          matrix->set_data((i - row_beginl) * (col_endl - col_beginl) +
                               (j - col_beginl),
                           matrix_get->data.get()[i * matrix_get->cols + j]);
        }
      }

      matrix->set_rows(row_endl - row_beginl);
      matrix->set_cols(col_endl - col_beginl);
      matrix->set_name(info->name());
          // LOG(INFO) << "find target matrix in graph";
      return ::grpc::Status::OK;
    }

    // LOG(INFO) << "Couldn't find target matrix in graph";
    return ::grpc::Status::OK;
  }
};

RpcServerType RunServer() {
  std::string address(getenv("LOCAL_ADDRESS"));
  address += ":";
  address += std::string(getenv("LOCAL_PORT"));
  CHECK(address != "") << "LOCAL_ADDRESS not set";

  auto service = new DisServiceImpl;
  auto builder = new ServerBuilder;
  builder->AddListeningPort(address, grpc::InsecureServerCredentials());
  builder->RegisterService(service);
  builder->SetMaxMessageSize(std::numeric_limits<int>::max());

  std::unique_ptr<Server> server(builder->BuildAndStart());
  LOG(INFO) << "Server listening on " << address << std::endl;

  return std::make_tuple(service, builder,
                         new std::unique_ptr<Server>(std::move(server)));
}

std::pair<std::thread *, RpcServerType> EnvStart() {

  // Insert first address to address map.
  // Get port from environment variable 'MASTER_PORT'
  int port = std::stoi(std::string(getenv("MASTER_PORT")));

  ::dis::core::GetGlobalAddressMap().insert(std::make_pair(
      0, ::dis::core::Address{port, std::string(getenv("MASTER_ADDRESS"))}));

  auto rpc = RunServer();
  std::unique_ptr<Server> *server = std::get<2>(rpc);
  // Must use value passing in new thread instead of reference.
  std::thread *server_thread = new std::thread([server] {
    (*server)->Wait(); // Wait for server to shutdown.
  });

  return std::make_pair(server_thread, std::move(rpc));
}

void FreeDisSerivce(std::pair<std::thread *, RpcServerType> rpc) {
  // Step 1: Shutdown server
  std::unique_ptr<Server> *server = std::get<2>(rpc.second);
  server->get()->Shutdown();
  // Step 2: Join server thread
  rpc.first->join();
  // Step 3: Delete server thread
  delete rpc.first;
  // Step 4: Delete server
  delete server;
  // Step 5: Delete service
  delete std::get<0>(rpc.second);
  // Step 6: Delete builder
  delete std::get<1>(rpc.second);
}

} // namespace dis
