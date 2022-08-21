#include <glog/logging.h>
#include <grpc++/grpc++.h>
#include <memory>

#include "grpc_helper/data.grpc.pb.h"

#include "matrix.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using matmul_call::MatMul;

using matrix::FloatMatrix;

class RPCMatMulClient {
public:
  RPCMatMulClient(std::shared_ptr<Channel> channel)
      : _stub(MatMul::NewStub(channel)) {}

  FloatMatrix *RPCMatMul(FloatMatrix *matrix1, FloatMatrix *matrix2) {
    matmul_call::MatMulInput input;

    input.set_row1(matrix1->row);
    input.set_col1(matrix1->col);
    input.set_row2(matrix2->row);
    input.set_col2(matrix2->col);

    std::unique_ptr<float> &matrix1_ptr = matrix1->data;
    std::unique_ptr<float> &matrix2_ptr = matrix2->data;

    google::protobuf::RepeatedField<float> copy1(
        matrix1_ptr.get(), matrix1_ptr.get() + matrix1->row * matrix1->col);

    input.mutable_input1()->Swap(&copy1);
    DCHECK(input.input1_size() == matrix1->row * matrix1->col);

    google::protobuf::RepeatedField<float> copy2(
        matrix2_ptr.get(), matrix2_ptr.get() + matrix2->row * matrix2->col);
    input.mutable_input2()->Swap(&copy2);
    DCHECK(input.input2_size() == matrix2->row * matrix2->col);

    ClientContext context;
    matmul_call::MatMulOutput output;

    Status status = _stub->Compute(&context, input, &output);

    if (!status.ok()) {
      LOG(ERROR) << "RPCMatMul failed: " << status.error_code() << ": "
                 << status.error_message();
      return nullptr;
    }
    DCHECK(output.output_size() == matrix1->col * matrix2->row);

    FloatMatrix *result = new FloatMatrix;
    result->row = output.row();
    result->col = output.col();
    result->data.reset(new float[result->row * result->col]);

    const float *output_ptr = output.output().data();
    ::memcpy(result->data.get(), output_ptr,
             result->row * result->col * sizeof(float));

    return result;
  }

private:
  std::unique_ptr<MatMul::Stub> _stub;
};

bool Check(const FloatMatrix *matrix1, const FloatMatrix *matrix2,
           const FloatMatrix *result) {

  const float err = 1e-5;
  if (matrix1->col != result->col || matrix2->row != result->row) {
    return false;
  }
  for (int i = 0; i < matrix1->col; i++) {
    for (int j = 0; j < matrix1->row; j++) {
      float sum = 0.f;
      for (int k = 0; k < matrix2->col; k++) {
        sum += matrix1->data.get()[i * matrix1->row + k] *
               matrix2->data.get()[k * matrix2->row + j];
      }
      if (std::fabs(sum - result->data.get()[i * result->row + j]) > err) {
        return false;
      }
    }
  }
  return true;
}

int main() {

  int col1 = 4;
  int row1 = 124;
  int col2 = 124;
  int row2 = 204;

  FloatMatrix *matrix1 = new FloatMatrix(col1, row1);
  FloatMatrix *matrix2 = new FloatMatrix(col2, row2);

  RPCMatMulClient client(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  FloatMatrix *result = client.RPCMatMul(matrix1, matrix2);
  if (result == nullptr) {
    return 1;
  }
  LOG(INFO) << "Successfully RPCMatMul";
  // Do some check!
  if (Check(matrix1, matrix2, result)) {
    LOG(INFO) << "Successfully check";
  } else {
    LOG(ERROR) << "Failed check";
    return 1;
  }

  delete matrix1;
}
