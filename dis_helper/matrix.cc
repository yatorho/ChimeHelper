#include "dis_helper/matrix.h"
#include "dis_helper/graph.h"

namespace dis {
namespace core {

Matrix::Matrix(int rows, int cols, std::string name)
    : rows(rows), cols(cols), name(name) {
  data.reset(new float[rows * cols]);
  GetDefaultGraph()->AddMatrixToGroup(this);
}

} // namespace core
} // namespace dis
 