#include "async_helper/tensor.h"

#include <iostream>

namespace async_helper {
void ShowTensor(const Tensor &matrix) {
  for (int64_t i = 0; i < matrix.rows; i++) {
    std::cout << "[";
    for (int64_t j = 0; j < matrix.cols; j++) {
      std::cout << matrix.data.get()[i * matrix.cols + j] << " ";
    }
    std::cout << "]" <<  std::endl;
  }
}
} // namespace async_helper
