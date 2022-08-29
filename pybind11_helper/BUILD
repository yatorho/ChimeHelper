load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")
load("@pybind11_bazel//:build_defs.bzl", "pybind_library")

pybind_extension(
    name = "example",
    srcs = ["example.cpp"],
)

py_library(
    name = "example_py",
    data = [":example.so"],
)

py_binary(
    name = "example",
    srcs = ["example.py"],
    deps = [":example_py"],
)

