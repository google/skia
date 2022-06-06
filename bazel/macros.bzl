"""
This file contains general helper macros that make our BUILD.bazel files easier to read.
"""

# https://github.com/bazelbuild/bazel-skylib
load("@bazel_skylib//lib:selects.bzl", _selects = "selects")
load("@rules_python//python:defs.bzl", _py_binary = "py_binary")
load("@py_deps//:requirements.bzl", _requirement = "requirement")
load("@bazel_gazelle//:def.bzl", _gazelle = "gazelle")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", _wasm_cc_binary = "wasm_cc_binary")

# re-export symbols that are commonly used or that are not supported in G3
# (and thus we need to stub out)
selects = _selects
py_binary = _py_binary
requirement = _requirement
gazelle = _gazelle
wasm_cc_binary = _wasm_cc_binary

def select_multi(values_map, default):
    """select() but allowing multiple matches of the keys.

    select_multi works around a restriction in native select() that prevents multiple
    keys from being matched unless one is a strict subset of another. For some features,
    we allow multiple of that component to be active. For example, with codecs, we let
    the clients mix and match anywhere from 0 built in codecs to all of them.

    select_multi takes a given map and turns it into several distinct select statements
    that have the effect of using any values associated with any active keys.
    For example, if the following parameters are passed in:
        values_map = {
            ":alpha": ["apple", "apricot"],
            ":beta": ["banana"],
            ":gamma": ["grapefruit"],
        },
        default = []
    it will be unrolled into the following select statements
        [] + select({
            ":apple": ["apple", "apricot"],
            "//conditions:default": [],
        }) + select({
            ":beta": ["banana"],
            "//conditions:default": [],
        }) + select({
            ":gamma": ["grapefruit"],
            "//conditions:default": [],
        })

    Args:
        values_map: dictionary of labels to a list of labels, just like select()
        default: list of labels, the value that should be used if any of the options do not match.
            This is typically an empty list

    Returns:
        A list of values that is filled in by the generated select statements.
    """
    if len(values_map) == 0:
        return default
    rv = []
    for key, value in values_map.items():
        rv += select({
            key: value,
            "//conditions:default": default,
        })
    return rv

# buildifier: disable=unnamed-macro
def cc_library(**kwargs):
    """A shim around cc_library that lets us tweak settings for G3 if necessary."""
    native.cc_library(**kwargs)

# buildifier: disable=unnamed-macro
def exports_files_legacy(label_list = None, visibility = None):
    """A self-annotating macro to export all files in this package for legacy G3 rules.

    Args:
        label_list: If provided, this will act like a normal exports_files rule. If not
           provided, nothing happens.
        visibility: Should be provided if label_list is set
    """
    if label_list:
        native.exports_files(label_list, visibility = visibility)

def split_srcs_and_hdrs(name, files):
    """Take a list of files and creates filegroups for C++ sources and headers.

    The reason we make filegroups is that they are more friendly towards a file being
    listed twice than just returning a sorted list of files.

    For example, in //src/codecs, "SkEncodedInfo.cpp" is needed for some, but not all
    the codecs. It is easier for devs to list the file for the codecs that need it
    rather than making a complicated select statement to make sure it is only in the
    list of files once.

    Bazel is smart enough to not compile the same file twice, even if it shows up in
    multiple filegroups.

    The "_srcs" and "_hdrs" filegroups will only be created if there are a non-zero amount
    of files of both types. Otherwise, it will fail because we do not need the macro.

    Args:
        name: The prefix of the generated filegroups. One will have the suffix "_srcs" and
            the other "_hdrs".
        files: List of file names, e.g. ["SkAAClip.cpp", "SkAAClip.h"]
    """
    srcs = []
    hdrs = []
    for f in files:
        if f.endswith(".cpp"):
            srcs.append(f)
        elif f.endswith(".h"):
            hdrs.append(f)
        else:
            fail("Neither .cpp nor .h file " + f)

    if len(srcs) == 0 or len(hdrs) == 0:
        fail("The list consist of either only source or header files. No need to use this macro.")

    native.filegroup(
        name = name + "_srcs",
        srcs = srcs,
    )
    native.filegroup(
        name = name + "_hdrs",
        srcs = hdrs,
    )
