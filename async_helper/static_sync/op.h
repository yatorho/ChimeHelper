#ifndef ASYNC_HELPER_STATIC_SYNC_OP_H_
#define ASYNC_HELPER_STATIC_SYNC_OP_H_

#include "async_helper/tensor.h"
#include <memory>

namespace async_helper {

class Operator {
public:
  virtual ~Operator() {}

  virtual bool IsHeaderOp() { return false; }

  virtual void Compute() = 0;

  virtual const std::string &Name() { return _name; }

  virtual bool IsReadyToBeScheduled() {
    if (IsHeaderOp())
      return true;
    for (auto op : _froms) {
      if (!op->_computed) {
        return false;
      }
    }
    return true;
  }

  std::vector<Operator *> _froms;
  std::vector<Tensor *> _outputs;

  bool _computed = false;
  std::string _name;
};

class RandomInitOp : public Operator {
public:
  RandomInitOp(const std::string &name, Tensor *t);

  bool IsHeaderOp() override { return true; }

  void Compute() override;
};

class SetValueOp : public Operator {
public:
  SetValueOp(const std::string &name, Tensor *t, float vavlue);

  void Compute() override;

  bool IsHeaderOp() override { return true; }

private:
  float _value;
};

class AddOp : public Operator {
public:
  AddOp(const std::string &name, Operator *add1, Operator *add2);

  ~AddOp() override;

  bool IsHeaderOp() override { return false; }

  void Compute() override;
};

} // namespace async_helper

#endif // ASYNC_HELPER_STATIC_SYNC_OP_H_
