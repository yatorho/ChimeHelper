#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int i, int j) { return i + j; }

struct Pet {
  Pet(const std::string &name) : name(name) {}
  std::string name;
};

struct Dog : Pet {
  Dog(const std::string &name) : Pet(name) {}
  std::string bark() const { return "woof!"; }
};

struct PolymorphicPet {
  virtual ~PolymorphicPet() = default;
};

struct PolymorphicDog : PolymorphicPet {
  std::string bark() const { return "woof!"; }
};

PYBIND11_MODULE(example, m) {
  m.doc() = "pybind11 example plugin"; // optional module docstring
  // Return a base pointer to a derived instance
  py::class_<Pet>(m, "Pet")
      .def(py::init<const std::string &>())
      .def_readwrite("name", &Pet::name);

  py::class_<Dog, Pet /* <- specify C++ parent type */>(m, "Dog")
      .def(py::init<const std::string &>())
      .def("bark", &Dog::bark);

  m.def("pet_store", []() { return std::unique_ptr<Pet>(new Dog("Molly")); });

  py::class_<PolymorphicPet>(m, "PolymorphicPet");
  py::class_<PolymorphicDog, PolymorphicPet>(m, "PolymorphicDog")
      .def(py::init<>())
      .def("bark", &PolymorphicDog::bark);

  m.def("pet_store2", []() { return std::unique_ptr<PolymorphicPet>(new PolymorphicDog); });
}
