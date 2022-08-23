#ifndef ASYNC_HELPER_OP_H_
#define ASYNC_HELPER_OP_H_

#include "chime/core/platform/logging.hpp"

#include "async_helper/tensor.h"

#include <atomic>
#include <random>
#include <vector>

namespace async_helper {

class Operator {
public:
  virtual ~Operator() {}
  virtual void Compute() = 0;
  virtual TensorArray &OutputsWithAsync() { return _outputs; }
  virtual TensorArray &Outputs() {
    while (!_computed)
      ;
    return _outputs;
  }
  virtual bool IsComputed() { return _computed; }

protected:
  friend class StaticGraph;
  TensorArray _inputs;
  TensorArray _outputs;
  std::atomic_bool _computed = {false};
};

class RandomInitOp : public Operator {
public:
  RandomInitOp(Tensor *tensor) {
    _inputs.push_back(tensor);
    _outputs.push_back(tensor);
  }

  void Compute() override {
    float *data = _outputs[0]->data.get();

    if (!_computed) {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<float> dis(0.0, 1.0);
      for (int64_t i = 0; i < _outputs[0]->rows * _outputs[0]->cols; ++i) {
        data[i] = dis(gen);
      }
      _computed = true;
    }
    // LOG(INFO) << "Called RandomInitOp::Compute() once";
  }
};

class AddOp : public Operator {
public:
  AddOp(Tensor *input1, Tensor *input2, Tensor *output) {
    _inputs.push_back(input1);
    _inputs.push_back(input2);
    _outputs.push_back(output);
  }

  void Compute() override {
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
};

class MulOp : public Operator {
public:
  MulOp(Tensor *input1, Tensor *input2, Tensor *output) {
    _inputs.push_back(input1);
    _inputs.push_back(input2);
    _outputs.push_back(output);
  }

  void Compute() override {
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
};

class MatMultOp : public Operator {
public:
  MatMultOp(Tensor *input1, Tensor *input2, Tensor *output) {
    _inputs.push_back(input1);
    _inputs.push_back(input2);
    _outputs.push_back(output);
  }

  void Compute() override {
    CHECK(_inputs[0]->cols == _inputs[1]->rows) << "cols not match";
    if (!_computed) {
      float *data1_ptr = _inputs[0]->data.get();
      float *data2_ptr = _inputs[1]->data.get();
      float *output_ptr = _outputs[0]->data.get();
      for (int64_t i = 0; i < _inputs[0]->rows; ++i) {
        for (int64_t j = 0; j < _inputs[1]->cols; ++j) {
          float sum = 0;
          for (int64_t k = 0; k < _inputs[0]->cols; ++k) {
            sum += data1_ptr[i * _inputs[0]->cols + k] *
                   data2_ptr[k * _inputs[1]->cols + j];
          }
          output_ptr[i * _inputs[1]->cols + j] = sum;
        }
      }
      _computed = true;
    }
  }
};

} // namespace async_helper

#endif // ASYNC_HELPER_OP_H_
