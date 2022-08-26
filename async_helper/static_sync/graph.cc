#include "graph.h"
#include <iostream>
#include <mutex>

namespace async_helper {

namespace {
static Graph *_graph = new Graph();
}

void Graph::ShowFroms(Operator *op) const {
  if (op->_froms.empty())
    std::cout << op->_name << " has no froms" << std::endl;

  for (auto from : op->_froms) {
    std::cout << op->_name << " <- " << from->_name << std::endl;
  }
}

void Graph::ShowGraph() const {
  std::cout << "Graph has " << _operators_with_priority.size() << " operators "
            << "and " << _edges.size() << " edges" << std::endl;

  for (auto &op_p : _operators_with_priority) {
    std::cout << op_p.second->_name << " Priority: " << op_p.first
              << "  Froms: ";
    if (op_p.second->_froms.empty())
      std::cout << "None";
    for (auto from : op_p.second->_froms) {
      std::cout << from->_name << ", ";
    }
    std::cout << std::endl;
  }
}

Graph *GetDefaultGraph() { 
  return _graph; 
}

void SetDefaultGraph(Graph *graph) { 
  CHECK(graph != nullptr);
  _graph = graph; 
}

} // namespace async_helper
