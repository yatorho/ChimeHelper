#ifndef GRPC_HELPER_MATRIX_H_
#define GRPC_HELPER_MATRIX_H_

#include <memory>
#include <random>

namespace matrix {

struct FloatMatrix {
  FloatMatrix(int col, int row) : col(col), row(row) {
    // Random init data
    data.reset(new float[row * col]);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    for (int i = 0; i < row * col; i++) {
      data.get()[i] = dis(gen);
    }
  }

  FloatMatrix() = default;

  int col;
  int row;
  std::unique_ptr<float> data;
};

} // namespace matrix

#endif // GRPC_HELPER_MATRIX_H_
