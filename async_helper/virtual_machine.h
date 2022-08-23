#ifndef ASYNC_HELPER_VISUAL_MACHINE_H_
#define ASYNC_HELPER_VISUAL_MACHINE_H_

#include "chime/core/platform/cpu_info.h"
#include "chime/core/platform/threadpool.h"

#include <mutex>
#include <queue>

namespace async_helper {

using chime::platform::ThreadPool;

class VirtualMachine {
public:
  VirtualMachine *Singleton();

private:
  VirtualMachine() {
    _pool =
        new ThreadPool(chime::platform::Env::Default(), "CHIME_VIRTUAL_MACHINE",
                       chime::port::NumTotalCPUs());
    for (int i = 0; i < chime::port::NumTotalCPUs(); i++) {
      _queues.push_back(std::queue<std::function<void()>>());
      _mutex.push_back(std::mutex());
    }
  }

  void ScheduleLoop() {
    
    for (int i = 0; i < _pool->NumThreads(); i++) {
      _pool->Schedule([this, i]() {
        while (true) {
          std::function<void()> func;
          {
            std::lock_guard<std::mutex> lock(_mutex[i]);
            if (_queues[i].empty()) {
              continue;
            } else {
              func = std::move(_queues[i].front());
              _queues[i].pop();
            }
          }
          func();
        }
      });
    }
  }

  std::vector<std::mutex> _mutex;
  std::vector<std::queue<std::function<void()>>> _queues;
  ThreadPool *_pool; // owned
};

} // namespace async_helper

#endif // ASYNC_HELPER_VISUAL_MACHINE_H_
