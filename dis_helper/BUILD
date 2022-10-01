load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")


proto_library(
    name = "env_proto",
    srcs = ["env.proto"],
)

cc_proto_library(
    name = "env_cc_proto",
    deps = [":env_proto"]
)

cc_grpc_library(
    name = "env_cc_grpc",
    srcs = [":env_proto"],
    grpc_only = True,
    deps = [":env_cc_proto"]
)

cc_binary(
  name = "main",
  srcs = ["main.cc"],
  deps = [":lib"]
)

cc_binary(
    name = "test",
    srcs = ["test_case.cc"],
    deps = [":lib"]
)

cc_library(
    name = "lib",
    srcs = ["master.cc",
            "master.h",
            "matrix.h",
            "worker_loop.h",
            "dis_info.h",
            "worker_loop.cc",
            "graph.h",
            "matrix.cc",
            "graph.cc",
            "dis_info.cc"],
    deps = [":env_cc_grpc",
            "@com_github_grpc_grpc//:grpc++",
            "@org_chime//chime/core/platform:logging"]
)

