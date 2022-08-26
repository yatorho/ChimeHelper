#ifndef ASYNC_HELPER_DYNAMIC_ASYNC_VIRTUAL_MACHINE_ENGINE_H_
#define ASYNC_HELPER_DYNAMIC_ASYNC_VIRTUAL_MACHINE_ENGINE_H_

#include "chime/core/platform/cpu_info.h"
#include "chime/core/platform/threadpool.h"

namespace async_helper {

using chime::platform::ThreadPool;

class VirtualMachineEngine {
public:
  VirtualMachineEngine() {
    _pool = new ThreadPool(chime::platform::Env::Default(),
                           "CHIME_VIRTUAL_MACHINE_ENGINE",
                           chime::port::NumTotalCPUs());
  }

  void Recieve(std::function<void()> &&func) { _pool->Schedule(func); }

  void Synce() { _pool->Wait(); }

  ~VirtualMachineEngine() {
    _pool->Wait();
    delete _pool;
  }

private:
  ThreadPool *_pool; // owned
};

} // namespace async_helper

#endif // ASYNC_HELPER_DYNAMIC_ASYNC_VIRTUAL_MACHINE_ENGINE_H_
