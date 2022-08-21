load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")


proto_library(
    name = "data_proto",
    srcs = ["data.proto"],
)

cc_proto_library(
    name = "data_cc_proto",
    deps = [":data_proto"]
)

cc_grpc_library(
    name = "data_cc_grpc",
    srcs = [":data_proto"],
    grpc_only = True,
    deps = [":data_cc_proto"]
)

cc_binary(
    name = "matmul_server",
    srcs = ["matmul_server.cc"],
    deps = [":data_cc_grpc",
            "@org_chime//chime/core/platform:threadpool",
            "@com_github_grpc_grpc//:grpc++",
            "@org_chime//chime/core/platform:cpu_info",
            "@org_chime//chime/core/platform/default:port",
            "@org_chime//chime/core/platform/default:env"]
)

cc_binary(
    name = "matmul_client",
    srcs = ["matmul_client.cc"],
    deps = [":data_cc_grpc",
            "@com_github_grpc_grpc//:grpc++",
            ":matrix",
            "@org_chime//chime/core/platform:logging"]
)

cc_library(
    name = "matrix",
    srcs = ["matrix.h"]
)
