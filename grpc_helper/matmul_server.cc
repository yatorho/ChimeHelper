#include <cstdint>
#include <glog/logging.h>
#include <grpc++/grpc++.h>

#include "chime/core/platform/cpu_info.h"
#include "chime/core/platform/env.hpp"
#include "chime/core/platform/threadpool.h"
#include "grpc_helper/data.grpc.pb.h"

// #include "matrix.h"
#include <google/protobuf/repeated_field.h>

#include <thread>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using matmul_call::MatMul;

using chime::platform::Env;
using chime::platform::ThreadPool;

constexpr int64_t COST_PER_UNIT = 1ll << 30;

class MatMulServiceImpl final : public MatMul::Service {
public:
  MatMulServiceImpl(ThreadPool *pool) : MatMul::Service(), _pool(pool) {}

  Status Compute(ServerContext *context, const matmul_call::MatMulInput *input,
                 matmul_call::MatMulOutput *reply) override {
    uint64_t start_time = Env::Default()->NowNanos();

    int len1 = input->input1_size();
    int len2 = input->input2_size();
    int col1 = input->col1();
    int col2 = input->col2();
    int row1 = input->row1();
    int row2 = input->row2();

    if (col1 * row1 != len1 || col2 * row2 != len2 || row1 != col2) {

      LOG(ERROR) << "len1: " << len1 << ", len2: " << len2 << ", col1: " << col1
                 << ", col2: " << col2 << ", row1: " << row1
                 << ", row2: " << row2;
      return Status::CANCELLED;
    }
    reply->mutable_output()->Resize(col1 * row2, 0);


    /////////////////////////////////////////////////////////////
    for (int i = 0; i < col1; i++) {
      for (int j = 0; j < row2; j++) {
        float sum = 0.f;
        for (int k = 0; k < row1; k++) {
          sum += input->input1(i * row1 + k) * input->input2(k * row2 + j);
        }
        reply->set_output(i * row2 + j, sum);
      }
    }

    /////////////////////////////////////////////////////////////

    // _pool->ParallelFor(col1, COST_PER_UNIT, [&](int64_t start, int64_t end) {
    //   for (int i = start; i < end; i++) {
    //     for (int j = 0; j < row2; j++) {
    //       float sum = 0.f;
    //       for (int k = 0; k < row1; k++) {
    //         sum += input->input1(i * row1 + k) * input->input2(k * row2 + j);
    //       }
    //       reply->set_output(i * row2 + j, sum);
    //     }
    //   }
    // });

    reply->set_col(col1);
    reply->set_row(row2);

    uint64_t end_time = Env::Default()->NowNanos();
    LOG(INFO) << "Cost time: " << (end_time - start_time)<< "ns";
    return Status::OK;
  }

private:
  ThreadPool *_pool;
};

void RunServer() {
  std::string address("0.0.0.0:50051");

  MatMulServiceImpl service(new ThreadPool(Env::Default(), "MatMulService",
                                           chime::port::NumTotalCPUs()));
  ServerBuilder builder;

  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << address << std::endl;

  server->Wait();
}

int main(int argc, char **argv) {
  RunServer();
  return 0;
}
