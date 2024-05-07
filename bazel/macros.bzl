"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains general helper macros that make our BUILD.bazel files easier to read.
"""

load("@bazel_gazelle//:def.bzl", _gazelle = "gazelle")

# https://github.com/bazelbuild/bazel-skylib
load("@bazel_skylib//lib:selects.bzl", _selects = "selects")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", _wasm_cc_binary = "wasm_cc_binary")
load("@io_bazel_rules_go//go:def.bzl", _go_binary = "go_binary", _go_library = "go_library")
load("@py_deps//:requirements.bzl", _requirement = "requirement")
load("@rules_python//python:defs.bzl", _py_binary = "py_binary")
load("//bazel:flags.bzl", _bool_flag = "bool_flag", _string_flag_with_values = "string_flag_with_values")
load(
    "//bazel:skia_rules.bzl",
    _exports_files_legacy = "exports_files_legacy",
    _generate_cpp_files_for_headers = "generate_cpp_files_for_headers",
    _select_multi = "select_multi",
    _skia_cc_binary = "skia_cc_binary",
    _skia_cc_binary_with_flags = "skia_cc_binary_with_flags",
    _skia_cc_deps = "skia_cc_deps",
    _skia_cc_library = "skia_cc_library",
    _skia_filegroup = "skia_filegroup",
    _skia_objc_library = "skia_objc_library",
    _split_srcs_and_hdrs = "split_srcs_and_hdrs",
)

# re-export symbols that are commonly used or that are not supported in G3
# (and thus we need to stub out)
bool_flag = _bool_flag
gazelle = _gazelle
go_binary = _go_binary
go_library = _go_library
py_binary = _py_binary
requirement = _requirement
selects = _selects
string_flag_with_values = _string_flag_with_values
wasm_cc_binary = _wasm_cc_binary
generate_cpp_files_for_headers = _generate_cpp_files_for_headers

exports_files_legacy = _exports_files_legacy
select_multi = _select_multi
skia_cc_binary = _skia_cc_binary
skia_cc_binary_with_flags = _skia_cc_binary_with_flags
skia_cc_deps = _skia_cc_deps
skia_cc_library = _skia_cc_library
skia_filegroup = _skia_filegroup
skia_objc_library = _skia_objc_library
split_srcs_and_hdrs = _split_srcs_and_hdrs
