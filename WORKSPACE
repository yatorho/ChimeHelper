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