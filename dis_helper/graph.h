#ifndef DIS_HELPER_GRAPH_H_
#define DIS_HELPER_GRAPH_H_

#include "dis_helper/matrix.h"
#include <cstddef>
#include <glog/logging.h>
#include <map>
#include <string>

namespace dis {
namespace core {

class Graph {
public:
  typedef std::map<std::string, dis::core::Matrix *> MatricesGroupType;

  ~Graph() {
    for (auto iter = _matrices_group.begin(); iter != _matrices_group.end();
         iter++) {
      delete (*iter).second;
      (*iter).second = nullptr;
      _matrices_group.erase(iter);
    }
    CHECK(_matrices_group.empty());
  }
  
  MatricesGroupType &GetMatricesGroup() { return _matrices_group; }
  void AddMatrixToGroup(dis::core::Matrix *matrix) {
    _matrices_group[matrix->name] = matrix;
  }

  dis::core::Matrix *FindTargetMatrix(const std::string &name) {
    auto iter = _matrices_group.find(name);
    if (iter != _matrices_group.end()) {
      return (*iter).second;
    }

    return nullptr;
  }


private:
  MatricesGroupType _matrices_group;
};

Graph *GetDefaultGraph();

} // namespace core
} // namespace dis

#endif // DIS_HELPER_GRAPH_H_
