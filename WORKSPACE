load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "org_chime",
    remote = "https://github.com/yatorho/CHIME.git",
    commit = "afb4466ba1a1f1e187430d0842637d3f54969807",
    shallow_since = "1660981055 +0800",
)

load("@org_chime//chime:workspace0.bzl", "chime_deps")  # workspace0 must be loaded first

chime_deps()

load("@org_chime//chime:workspace1.bzl", "chime_extra_deps0")

chime_extra_deps0()

load("@org_chime//chime:workspace2.bzl", "chime_extra_deps1")  # workspace2 must before workspace3

chime_extra_deps1()

load("@org_chime//chime:workspace3.bzl", "chime_extra_deps2")

chime_extra_deps2()

# Pybind11 support here
###################################################################################################
http_archive(
    name = "pybind11_bazel",
    strip_prefix = "pybind11_bazel-72cbbf1fbc830e487e3012862b7b720001b70672",
    urls = ["https://github.com/pybind/pybind11_bazel/archive/72cbbf1fbc830e487e3012862b7b720001b70672.zip"],
)

http_archive(
    name = "pybind11",
    build_file = "@pybind11_bazel//:pybind11.BUILD",
    strip_prefix = "pybind11-2.6.2",
    urls = ["https://github.com/pybind/pybind11/archive/v2.6.2.tar.gz"],
)
load("@pybind11_bazel//:python_configure.bzl", "python_configure")
python_configure(name = "local_config_python")
###################################################################################################


###################################################################################################
# FP16 support
http_archive(
    name = "FP16",
    strip_prefix = "FP16-4dfe081cf6bcd15db339cf2680b9281b8451eeb3",
    sha256 = "d973501a40c55126b31accc2d9f08d931ec3cc190c0430309a5e341d3c0ce32a",
    urls = ["https://github.com/Maratyszcza/FP16/archive/4dfe081cf6bcd15db339cf2680b9281b8451eeb3.zip"],
    build_file = "//fp16_helper:FP16.BUILD",
)

###################################################################################################
http_archive(
    name = "half",
    urls = ["https://github.com/Oneflow-Inc/half/archive/refs/tags/v2.1.0-fix-cuda-raise.zip"],
    sha256 = "7e6e33c188e63e03b9e137dabec25f9de7460f8684d8dce4d4e5df3aff022c24",
    strip_prefix = "half-2.1.0-fix-cuda-raise",
    build_file = "//half_helper:half.BUILD",
)