#ifndef DIS_HELPER_MATRIX_H_
#define DIS_HELPER_MATRIX_H_

#include <memory>

namespace dis {
namespace core {

struct Matrix {
  Matrix(int rows, int cols, const std::string& name);
  Matrix() : rows(0), cols(0) {}

  int rows;
  int cols;

  std::string name;
  std::unique_ptr<float> data;
  bool computed = false;
};

Matrix *MatrixMatMul(const Matrix *a, const Matrix *b, const std::string& name);

Matrix *CreateMatrix(int rows, int cols, const std::string& name);

void ShowMatrixValue(const Matrix* a);

Matrix *MergeMatrixVertically(const Matrix *a, const Matrix *b, const std::string& name);

Matrix *MergeMatrixParallely(const Matrix *a, const Matrix *b, const std::string& name);

float SumMatrix(Matrix *a);

} // namespace core
} // namespace dis

#endif // DIS_HELPER_MATRIX_H_
