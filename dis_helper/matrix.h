#ifndef DIS_HELPER_MATRIX_H_
#define DIS_HELPER_MATRIX_H_

#include <memory>

namespace dis {
namespace core {
struct Matrix {
  Matrix(int rows, int cols, std::string name);
  Matrix() : rows(0), cols(0) {}

  int rows;
  int cols;

  std::string name;
  std::unique_ptr<float> data;
};

} // namespace core
} // namespace dis

#endif // DIS_HELPER_MATRIX_H_
