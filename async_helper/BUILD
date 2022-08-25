package(default_visibility = [
        "//visibility:public",
    ])

cc_library(
    name = "tensor",
    srcs = ["tensor.h",
            "tensor.cc"],
    deps = ["@org_chime//chime/core/platform:logging"]
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [":tensor",
            ":op",
            ":graph",
            ":op_lib"],
)

cc_library(
    name = "op",
    srcs = ["op.h"],
    deps = [":tensor"],
)

cc_library(
    name = "op_lib",
    srcs = ["op_impl.cc"],
    deps = [":op",
            ":graph"],
)

cc_library(
    name = "graph",
    srcs = ["graph.h",
            "graph.cc"],
    deps = [":tensor",
            ":op",
            "@org_chime//chime/core/platform:threadpool",
            "@org_chime//chime/core/platform/default:env"],
)
