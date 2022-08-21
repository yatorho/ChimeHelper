load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

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

