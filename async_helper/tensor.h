#ifndef ASYNC_HELPER_TENSOR_H_
#define ASYNC_HELPER_TENSOR_H_

#include <cstdint>
#include <memory>

#include "chime/core/platform/logging.hpp"

namespace async_helper {

struct FloatMatrix {
  FloatMatrix(int64_t rows, int64_t cols) : rows(rows), cols(cols) {
    data.reset(nullptr);
  }

  FloatMatrix() : rows(0), cols(0) { data.reset(nullptr); }

  int64_t rows;
  int64_t cols;
  std::unique_ptr<float> data;
};

/// Just use a float matrix to represent a tensor.
using Tensor = FloatMatrix;
using TensorArray = std::vector<Tensor *>;

void ShowTensor(const Tensor &matrix);

} // namespace async_helper

#endif // ASYNC_HELPER_TENSOR_H_
