#include "op.h"

#include <algorithm>
#include <mutex>
#include <utility>

#ifndef ASYNC_HELPER_STATIC_SYNC_GRAPH_H_
#define ASYNC_HELPER_STATIC_SYNC_GRAPH_H_

namespace async_helper {

class Graph {
public:
  using Edge = std::pair<Operator *, Operator *>;

  void AddEdge(Operator *from, Operator *to) {
    if (from == nullptr || to == nullptr)
      LOG(FATAL) << "AddEdge: nullptr operator";

    std::lock_guard<std::mutex> lock(_mutex);

    if (std::find(_edges.begin(), _edges.end(), Edge(from, to)) != _edges.end())
      return;
    _edges.push_back(Edge(from, to));

    if (!HaveOperator(from)) {
      std::pair<int, Operator *> from_pair;
      if (from->IsHeaderOp())
        from_pair = std::make_pair(0, from);
      else {
        int schedule_priority = -1;
        for (auto &f : from->_froms) {
          schedule_priority = std::max(schedule_priority, FindPriority(f));
        }
        CHECK(schedule_priority >= 0) << "AddEdge: invalid schedule priority";
        from_pair = std::make_pair(schedule_priority + 1, from);
      }
      _operators_with_priority.push_back(std::move(from_pair));
    }

    if (!HaveOperator(to)) {
      std::pair<int, Operator *> to_pair;
      if (to->IsHeaderOp())
        to_pair = std::make_pair(0, to);
      else {
        int schedule_priority = -1;
        for (auto &t : to->_froms) {
          schedule_priority = std::max(schedule_priority, FindPriority(t));
        }
        CHECK(schedule_priority >= 0) << "AddEdge: invalid schedule priority";
        to_pair = std::make_pair(schedule_priority + 1, to);
      }
      _operators_with_priority.push_back(std::move(to_pair));
    }
  }

  void AddOperator(Operator *op) {
    if (op == nullptr)
      LOG(FATAL) << "AddOperator: nullptr operator";

    if (HaveOperator(op))
      return;

    std::lock_guard<std::mutex> lock(_mutex);
    CHECK(op->IsHeaderOp()) << "AddOperator: invalid schedule priority";
    std::pair<int, Operator *> op_with_schedule_priority =
        std::make_pair(0, op);

    _operators_with_priority.push_back(std::move(op_with_schedule_priority));
  }

  // Return -1 if op is not in the graph.
  int FindPriority(Operator *op) const {

    for (auto &op_p : _operators_with_priority) {
      if (op_p.second == op) {
        return op_p.first;
      }
    }
    return -1;
  }

  // Return -1 if graph does not have op.
  int LagestPriority() const {
    std::lock_guard<std::mutex> lock(_mutex);

    int max_priority = -1;
    for (auto &op_p : _operators_with_priority) {
      max_priority = std::max(max_priority, op_p.first);
    }
    return max_priority;
  }

  void GetOpertorsWithPriority(int priority,
                               std::vector<Operator *> *ops) const {
    std::lock_guard<std::mutex> lock(_mutex);

    ops->clear();
    for (auto &op_p : _operators_with_priority) {
      if (op_p.first == priority) {
        ops->push_back(op_p.second);
      }
    }
  }

  Operator *GetOpertorWithName(const std::string &name) const {
    for (auto &op_p : _operators_with_priority) {
      if (op_p.second->Name() == name) {
        return op_p.second;
      }
    }
    return nullptr;
  }

  bool HaveOperator(Operator *op) const {
    for (auto &op_p : _operators_with_priority) {
      if (op_p.second == op) {
        return true;
      }
    }
    return false;
  }

  void ShowFroms(Operator *op) const;

  void ShowGraph() const;

  size_t NumOperators() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _operators_with_priority.size();
  }

  size_t NumEdges() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _edges.size();
  }

  const std::vector<std::pair<int, Operator *>> &OperatorsWithPriority() const {
    std::lock_guard<std::mutex> lock(_mutex);
    // LOG(INFO) << "OperatorsWithPriority: " << _operators_with_priority.size();
    return _operators_with_priority;
  }

private:
  mutable std::mutex _mutex;
  std::vector<Edge> _edges;
  std::vector<std::pair<int, Operator *>> _operators_with_priority;
};

Graph *GetDefaultGraph();
void SetDefaultGraph(Graph *graph);

} // namespace async_helper

#endif // ASYNC_HELPER_STATIC_SYNC_GRAPH_H_
