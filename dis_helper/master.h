#ifndef DIS_HELPER_MASTER_H_
#define DIS_HELPER_MASTER_H_

#include <map>
#include <thread>
#include <memory>

#include <grpc++/grpc++.h>

namespace dis {

void Master();

std::pair<std::thread *, std::unique_ptr<grpc::Server> *> EnvStart();

}

#endif // DIS_HELPER_MASTER_H_
