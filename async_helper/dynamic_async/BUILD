cc_library(
    name = "virtual_machine",
    srcs = ["virtual_machine.h", "virtual_machine.cc", "virtual_machine_engine.h", "virtual_machine_scope.h"],
    deps = ["//async_helper/static_sync:graph",
            "//async_helper/static_sync:op",
            "//async_helper/static_sync:lib",
            "@org_chime//chime/core/platform:threadpool",
            "@org_chime//chime/core/platform/default:env",
            "@org_chime//chime/core/platform/default:port",
            "@org_chime//chime/core/platform:cpu_info"]
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [":virtual_machine"]
)
