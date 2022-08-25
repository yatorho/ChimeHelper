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

  ~StaticGraph() { _pool->Wait(); }

  void AddEdge(Operator *from, Operator *to) {
    if (from == nullptr || to == nullptr) {
      LOG(FATAL) << "AddEdge: nullptr operator";
    }
    if (std::find(_edges.begin(), _edges.end(), Edge(from, to)) !=
        _edges.end()) {
      LOG(FATAL) << "AddEdge: edge already exists";
    }
    _edges.push_back(Edge(from, to));
    if (std::find(_operators.begin(), _operators.end(), from) ==
        _operators.end()) {
      _operators.push_back(from);
    }
    if (std::find(_operators.begin(), _operators.end(), to) ==
        _operators.end()) {
      _operators.push_back(to);
    }
  }

  void AddOperator(Operator *op) {
    if (op == nullptr)
      LOG(FATAL) << "AddOperator: nullptr operator";

    if (std::find(_operators.begin(), _operators.end(), op) ==
        _operators.end()) {
      _operators.push_back(op);
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

  void Clear() {
    _edges.clear();
    _operators.clear();
  }

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

StaticGraph *GetDefaultStaticGraph();
void SetDefaultStaticGraph(StaticGraph *graph);

} // namespace async_helper

#endif // ASYNC_HELPER_GRAPH_H_