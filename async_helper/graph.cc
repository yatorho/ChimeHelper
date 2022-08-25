#include "async_helper/op.h"
#include "async_helper/graph.h"

namespace async_helper {

namespace {

static StaticGraph *_graph = new StaticGraph(new chime::platform::ThreadPool(
    chime::platform::Env::Default(), "async_helper", 12));
}

StaticGraph *GetDefaultStaticGraph() {
  return _graph;
}

void SetDefaultStaticGraph(StaticGraph *graph) {
  _graph = graph;
}

} // namespace async_helper
