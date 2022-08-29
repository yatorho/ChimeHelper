load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

pybind_extension(
    name = "example",  # This name is not actually created!
    srcs = ["example.cc"],
)

py_library(
    name = "example",
    data = [":example.so"],
)

py_binary(
    name = "example_test",
    srcs = ["example_test.py"],
    deps = [
        ":example"
    ],
)
