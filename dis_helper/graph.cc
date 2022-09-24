#include "graph.h"

namespace dis {
namespace core {

dis::core::Graph static_graph;

Graph* GetDefaultGraph(){
return &static_graph;
}
}
} // namespace dis