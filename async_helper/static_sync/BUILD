package(default_visibility = [
        "//visibility:public",
    ])

cc_library(
    name = "op",
    srcs = ["op.h"],
    deps = ["//async_helper:tensor"]
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [":op",
            ":lib",
            ":session"]
)

cc_library(
    name = "lib",
    srcs = ["op.cc", "graph.cc"],
    deps = [":op",
            ":graph",
            "@org_chime//chime/core/platform:logging"]
)

cc_library(
    name = "session",
    srcs = ["session.h"],
    deps = [":graph",
            "@org_chime//chime/core/platform:threadpool",
            "@org_chime//chime/core/platform/default:env"]
)


cc_library(
    name = "graph",
    srcs = ["graph.h"],
)
