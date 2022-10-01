#ifndef DIS_HELPER_MASTER_H_
#define DIS_HELPER_MASTER_H_

#include <map>
#include <thread>
#include <memory>
#include <grpc++/grpc++.h>

namespace dis {

void Master();

class DisServiceImpl;
using grpc::ServerBuilder;
using grpc::Server;

typedef std::tuple<DisServiceImpl *, ServerBuilder *, std::unique_ptr<Server> *> RpcServerType;

std::pair<std::thread *, RpcServerType> EnvStart();

void FreeDisSerivce(std::pair<std::thread *, RpcServerType> rpc);

}

#endif // DIS_HELPER_MASTER_H_
