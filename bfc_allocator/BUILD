cc_library(
    name = "bfc_allocator_lib",
    srcs = ["bfc_allocator.h",
            "bfc_allocator.cc"],
    deps = ["@org_chime//chime/core/platform:logging",
            "@org_chime//chime/core/platform:macros"]
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [":bfc_allocator_lib"],
)
