#include "async_helper/dynamic_async/virtual_machine.h"
#include "async_helper/dynamic_async/virtual_machine_scope.h"
#include "async_helper/tensor.h"
#include <cmath>
#include <iostream>

namespace ah = async_helper;

bool Check(ah::Tensor &t1, ah::Tensor &t2, ah::Tensor &t3, ah::Tensor &sum) {
  DCHECK(t1.rows == t2.rows);
  DCHECK(t1.rows == t3.rows);
  DCHECK(t1.rows == sum.rows);
  DCHECK(t1.cols == t2.cols);
  DCHECK(t1.cols == t3.cols);
  DCHECK(t1.cols == sum.cols);

  const float err = 1e-6;

  for (int64_t i = 0; i < t1.rows * t1.cols; i++) {
    if (std::fabs(t1.data.get()[i] + t2.data.get()[i] + t3.data.get()[i] -
                  sum.data.get()[i]) > err) {
      return false;
    }
  }
  return true;
}

int main() {
  ah::VirtualMachineScope vm_scope;
  /// This should be included in the scope of the main thread, to make sure the
  /// vm has been initialized and started its `ScheduleLoop`.

  LOG(INFO) << "main: start";

  ah::Tensor t1(2, 3);
  ah::Tensor t2(2, 3);
  ah::Tensor t3(2, 3);

  ah::SetValueOp op1("t1", &t1, 1);
  ah::SetValueOp op2("t2", &t2, 2);
  ah::SetValueOp op3("t3", &t3, 3);

  ah::AddOp add_op1("add1", &op1, &op2);
  ah::AddOp add_op2("add2", &op3, &add_op1);

  /// Sleep 1000ms to make sure vm has sheduled, and computed all ops
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  /// Of course, we can also provide a callback function to wait ops to be
  /// computed.
  /// Such as: `add_op1.Wait()` these means that you give a hint to the vm that
  /// you want to wait for `add_op1` to be done at least.

  if (!Check(t1, t2, t3, *add_op2._outputs[0])) {
    std::cout << "Check failed!" << std::endl;
    return -1;
  }
  std::cout << "Check passed!" << std::endl;
  return 0;
}