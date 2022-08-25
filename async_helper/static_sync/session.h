#ifndef ASYNC_HELPER_STATIC_SYNC_SESSION_H_
#define ASYNC_HELPER_STATIC_SYNC_SESSION_H_

#include "async_helper/static_sync/op.h"
#include "chime/core/platform/threadpool.h"
#include "graph.h"
#include <queue>

namespace async_helper {

using chime::platform::ThreadPool;

class Session {
public:
  Session()
      : _pool(new ThreadPool(chime::platform::Env::Default(), "async_helper",
                             12)) {}

  ~Session() {
    _pool->Wait();
    delete _pool;
  }

  void Create(Graph *graph) {
    CHECK(_graph != nullptr) << "Session already created";
    _graph = graph;
  }

  void Run(const std::vector<std::string> &names,
           std::vector<std::vector<Tensor *>> *result) {
    CHECK(_graph->LagestPriority() > 0) << "No operators in graph";
    result->clear();

    for (int priority = 0; priority <= _graph->LagestPriority(); priority++) {
      std::vector<Operator *> ops;
      _graph->GetOpertorsWithPriority(priority, &ops);

      for (auto &op : ops) {
        _pool->Schedule([op]() { op->Compute(); });
      }
      _pool->Wait();
    }

    // for (auto &output : _graph->GetOpertorWithName(name)->_outputs) {
    //   result->push_back(output);
    // }
    for (auto &name : names) {
      std::vector<Tensor *> tensors;
      Operator *op = _graph->GetOpertorWithName(name);
      if (op == nullptr) {
        LOG(FATAL) << "Operator " << name << " not found";
      }
      for (auto &output : op->_outputs) {
        tensors.push_back(output);
      }
      result->push_back(tensors);
    }
  }

private:
  ThreadPool *_pool;
  Graph *_graph;
};

} // namespace async_helper

#endif // ASYNC_HELPER_STATIC_SYNC_SESSION_H_
