#include "async_helper/static_sync/op.h"
#include "async_helper/static_sync/graph.h"
#include <memory>
#include <random>

#include "chime/core/platform/logging.hpp"

namespace async_helper {

RandomInitOp::RandomInitOp(const std::string &name, Tensor *t) {
  _outputs.push_back(t);
  _name = name;

  Graph *graph = GetDefaultGraph();
  graph->AddOperator(this);
}

void RandomInitOp::Compute() {
  if (!_computed) {
    Tensor *_t = _outputs[0];

    CHECK(_t->data.get() == nullptr);

    _t->data.reset(new float[_t->rows * _t->cols]);
    float *data = _t->data.get();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    for (int64_t i = 0; i < _t->rows * _t->cols; i++) {
      data[i] = dis(gen);
    }
    _computed = true;
  }
}

AddOp::AddOp(const std::string &name, Operator *add1, Operator *add2) {
  DCHECK(add1->_outputs[0]->rows == add2->_outputs[0]->rows);
  DCHECK(add1->_outputs[0]->cols == add2->_outputs[0]->cols);

  _name = name;

  _froms.push_back(add1);
  _froms.push_back(add2);
  _outputs.push_back(
      new Tensor(add1->_outputs[0]->rows, add1->_outputs[0]->cols));

  Graph *graph = GetDefaultGraph();
  graph->AddEdge(add1, this);
  graph->AddEdge(add2, this);
}

AddOp::~AddOp() {
  for (auto t : _outputs) {
    delete t;
  }
}

void AddOp::Compute() {
  if (!_computed) {
    CHECK(_froms[0]->_computed);
    CHECK(_froms[1]->_computed);

    Tensor *_t1 = _froms[0]->_outputs[0];
    Tensor *_t2 = _froms[1]->_outputs[0];
    Tensor *_t = _outputs[0];

    CHECK(_t1->data.get() != nullptr);
    CHECK(_t2->data.get() != nullptr);
    CHECK(_t->data.get() == nullptr);
    _t->data.reset(new float[_t1->rows * _t1->cols]);
    float *data = _t->data.get();
    for (int64_t i = 0; i < _t1->rows * _t1->cols; i++) {
      data[i] = _t1->data.get()[i] + _t2->data.get()[i];
    }
    _computed = true;
  }
}

SetValueOp::SetValueOp(const std::string &name, Tensor *t, float value) {
  _outputs.push_back(t);
  _name = name;
  _value = value;

  Graph *graph = GetDefaultGraph();
  graph->AddOperator(this);
}

void SetValueOp::Compute() {
  if (!_computed) {
    // LOG(INFO) << "SetValueOp::Compute From " << _name;
    Tensor *_t = _outputs[0];

    CHECK(_t->data.get() == nullptr);

    _t->data.reset(new float[_t->rows * _t->cols]);
    float *data = _t->data.get();
    for (int64_t i = 0; i < _t->rows * _t->cols; i++) {
      data[i] = _value;
    }
    _computed = true;
  }
}

} // namespace async_helper
