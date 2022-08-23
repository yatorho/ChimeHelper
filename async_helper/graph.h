#ifndef ASYNC_HELPER_GRAPH_H_
#define ASYNC_HELPER_GRAPH_H_

#include "async_helper/op.h"
#include "async_helper/tensor.h"

#include "chime/core/platform/threadpool.h"

#include <algorithm>

namespace async_helper {

using chime::platform::ThreadPool;

class StaticGraph {
public:
  using Edge = std::pair<Operator *, Operator *>;
  using EdgeList = std::vector<Edge>;
  using OperatorList = std::vector<Operator *>;

  StaticGraph(ThreadPool *pool) { _pool = pool; }

  ~StaticGraph() {
    _pool->Wait();
  }

  void AddEdge(Operator *from, Operator *to) {
    _edges.push_back(std::make_pair(from, to));
    if (std::find(_operators.begin(), _operators.end(), from) ==
        _operators.end()) {
      _operators.push_back(from);
    }
    if (std::find(_operators.begin(), _operators.end(), to) ==
        _operators.end()) {
      _operators.push_back(to);
    }
  }

  void Forward() {
    for (auto &op : _operators) {
      _pool->Schedule([op, this]() {
        while (!IsReadyToBeComputed(op))
          ;
        op->Compute();
      });
    }
  }

  void ForwardSynced() {
    Forward();
    _pool->Wait();
  }

  bool IsReadyToBeComputed(Operator *op) {
    OperatorList fathers(GetFathers(op));
    for (auto &father : fathers) {
      if (!father->_computed)
        return false;
    }
    return true;
  }

private:
  OperatorList GetFathers(Operator *op) {
    OperatorList fathers;
    for (auto &edge : _edges) {
      if (edge.second == op) {
        fathers.push_back(edge.first);
      }
    }
    return std::move(fathers);
  }

  ThreadPool *_pool; // not owned
  EdgeList _edges;
  OperatorList _operators;
};

} // namespace async_helper

#endif // ASYNC_HELPER_GRAPH_H_