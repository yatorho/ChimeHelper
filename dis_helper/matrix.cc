#include "dis_helper/matrix.h"
#include "dis_helper/graph.h"
#include <iostream>
#include <string>

namespace dis {
namespace core {

Matrix::Matrix(int rows, int cols, const std::string &name)
    : rows(rows), cols(cols), name(name) {
  data.reset(new float[rows * cols]);
  GetDefaultGraph()->AddMatrixToGroup(this);
}

Matrix *MatrixMatMul(const Matrix *a, const Matrix *b,
                     const std::string &name) {
  CHECK(a->cols == b->rows) << "MatrixMatMul: a->cols != b->rows";

  Matrix *c = new Matrix(a->rows, b->cols, name);

  for (int i = 0; i < a->rows; i++) {
    for (int j = 0; j < b->cols; j++) {
      float sum = 0;
      for (int k = 0; k < a->cols; k++) {
        sum += a->data.get()[i * a->cols + k] * b->data.get()[k * b->cols + j];
      }
      c->data.get()[i * c->cols + j] = sum;
    }
  }

  c->computed = true;
  return c;
}

Matrix *CreateMatrix(int rows, int cols, const std::string &name) {
  Matrix *m = new Matrix(rows, cols, name);
  m->computed = true;
  return m;
}

void ShowMatrixValue(const Matrix *a) {
  int rows = a->rows;
  int cols = a->cols;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      std::cout << a->data.get()[i * cols + j] << " ";
    }
    std::cout << std::endl;
  }
}

Matrix *MergeMatrixVertically(const Matrix *a, const Matrix *b,
                              const std::string &name) {
  int a_rows = a->rows;
  int a_cols = a->cols;
  int b_rows = b->rows;
  int b_cols = b->cols;

  CHECK(a_cols = b_cols);

  Matrix *result = new Matrix(a_rows + b_rows, a_cols, name);
  for (int i = 0; i < a_rows + b_rows; i++) {
    for (int j = 0; j < a_cols; j++) {
      if (i < a_rows) {
        result->data.get()[i * a_cols + j] = a->data.get()[i * a_cols + j];
      } else {
        result->data.get()[i * a_cols + j] =
            b->data.get()[(i - a_rows) * a_cols + j];
      }
    }
  }

  result->computed = true;
  return result;
}

Matrix *MergeMatrixParallely(const Matrix *a, const Matrix *b,
                            const std::string &name) {
  int a_rows = a->rows;
  int a_cols = a->cols;
  int b_rows = b->rows;
  int b_cols = b->cols;

  CHECK(a_rows = b_rows);

  Matrix *result = new Matrix(a_rows, a_cols + b_cols, name);

  for (int i = 0; i < a_rows; i++) {
    for (int j = 0; j < a_cols + b_cols; j++) {
      if (j < a_cols) {
        result->data.get()[i * (a_cols + b_cols) + j] =
            a->data.get()[i * a_cols + j];
      } else {
        result->data.get()[i * (a_cols + b_cols) + j] =
            b->data.get()[i * b_cols + (j - a_cols)];
      }
    }
  }

  result->computed = true;
  return result;
}

float SumMatrix(Matrix *a) {
  int rows = a->rows;
  int cols = a->cols;

  float sum = 0;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) 
      sum += a->data.get()[i * cols + j];
  }
  a->computed = true;
  return  sum;
}

} // namespace core
} // namespace dis
