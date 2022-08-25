#include "async_helper/graph.h"
#include "async_helper/op.h"

namespace async_helper {

RandomInitOp::RandomInitOp(Tensor *tensor) {
  _inputs.push_back(tensor);
  _outputs.push_back(tensor);

  // auto graph = GetDefaultStaticGraph();
  // graph->AddOperator(this);
}

void RandomInitOp::Compute() {

  if (!_computed) {
    _outputs[0]->data.reset(new float[_outputs[0]->rows * _outputs[0]->cols]);
    float *data = _outputs[0]->data.get();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    for (int64_t i = 0; i < _outputs[0]->rows * _outputs[0]->cols; i++) {
      data[i] = dis(gen);
    }
    _computed = true;
  }
  // LOG(INFO) << "Called RandomInitOp::Compute() once";
};

void MallocOp::Compute() {
  if (!_computed) {
    _outputs[0]->data.reset(new float[_outputs[0]->rows * _outputs[0]->cols]);
    _computed = true;
  }
}

MallocOp::MallocOp(Tensor *tensor) {
  _inputs.push_back(tensor);
  _outputs.push_back(tensor);

  // auto graph = GetDefaultStaticGraph();
  // graph->AddOperator(this);
}

AddOp::AddOp(Tensor *input1, Tensor *input2, Tensor *output) {
  _inputs.push_back(input1);
  _inputs.push_back(input2);
  _outputs.push_back(output);
  
}

void AddOp::Compute() {
  CHECK(_inputs[0]->rows == _inputs[1]->rows) << "rows not match";
  CHECK(_inputs[0]->cols == _inputs[1]->cols) << "cols not match";
  if (!_computed) {
    float *data1_ptr = _inputs[0]->data.get();
    float *data2_ptr = _inputs[1]->data.get();
    float *output_ptr = _outputs[0]->data.get();

    for (int64_t i = 0; i < _inputs[0]->rows * _inputs[0]->cols; ++i) {
      output_ptr[i] = data1_ptr[i] + data2_ptr[i];
    }
    _computed = true;
  }
  // LOG(INFO) << "Called AddOp::Compute() once";
}

void MulOp::Compute() {
  CHECK(_inputs[0]->rows == _inputs[1]->rows) << "rows not match";
  CHECK(_inputs[0]->cols == _inputs[1]->cols) << "cols not match";
  if (!_computed) {
    float *data1_ptr = _inputs[0]->data.get();
    float *data2_ptr = _inputs[1]->data.get();
    float *output_ptr = _outputs[0]->data.get();

    for (int64_t i = 0; i < _inputs[0]->rows * _inputs[0]->cols; ++i) {
      output_ptr[i] = data1_ptr[i] * data2_ptr[i];
    }
    _computed = true;
  }
}

} // namespace async_helper
