#include <iostream>

#include "chime/core/platform/threadpool.h"

#include <memory>


namespace async_helper {

using TensorShape = std::vector<int64_t>;

template <typename T> struct is_regular_type {
  static const bool value = false;
};

template <> struct is_regular_type<float> { static const bool value = true; };

template <> struct is_regular_type<double> { static const bool value = true; };

template <typename T> class Tensor {
  static_assert(is_regular_type<T>::value, "T must be a regular type");

public:
  Tensor() : _data(nullptr) {}

  Tensor(TensorShape &shape) : _shape(shape) {
    _data.reset(new T[shape.size()]);
  }

  Tensor(TensorShape &shape, T value) : _shape(shape) {
    _data.reset(new T[shape.size()]);
    for (size_t i = 0; i < shape.size(); i++) {
      _data.get()[i] = value;
    }
  }

  Tensor(const Tensor &other) : _shape(other._shape) {
    _data.reset(new T[other._shape.size()]);
    ::memcpy(_data.get(), other._data.get(),
             other._shape.size() * sizeof(T));
  }

  Tensor &operator=(const Tensor &other) {
    if (this == &other) {
      return *this;
    }
    _shape = other._shape;
    _data.reset(new T[other._shape.size()]);
    ::memcpy(_data.get(), other._data.get(),
             other._shape.size() * sizeof(T));
    return *this;
  }

  Tensor(Tensor &&other) :_data(std::move(other._data)),  _shape(other._shape) {
    std::cout << "Moved!\n";
  }

private:
  std::unique_ptr<T> _data;

  TensorShape _shape;
};

} // namespace async_helper

using namespace async_helper;

int main() {
  TensorShape shape({2, 3});
  Tensor<float> a(shape, 1.f);
  Tensor<float> b(std::move(a));

  std::cout << "Hello, Chime!" << std::endl;
}