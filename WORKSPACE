load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "org_chime",
    remote = "https://github.com/yatorho/CHIME.git",
    commit = "c71f5308c5520ef9395dd039ae15e306edc5947e",
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

