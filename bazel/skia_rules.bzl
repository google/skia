"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains macros which require no third-party dependencies.
Using these where possible makes it easier for clients to use Skia
without needing to download a bunch of unnecessary dependencies
in their WORKSPACE.bazel file.
"""

load("//bazel:cc_binary_with_flags.bzl", "cc_binary_with_flags")
load("//bazel:copts.bzl", "DEFAULT_COPTS", "DEFAULT_OBJC_COPTS")
load(
    "//bazel:generate_cpp_files_for_headers.bzl",
    _generate_cpp_files_for_header_list = "generate_cpp_files_for_header_list",
    _generate_cpp_files_for_headers = "generate_cpp_files_for_headers",
)
load("//bazel:linkopts.bzl", "DEFAULT_LINKOPTS")

generate_cpp_files_for_headers = _generate_cpp_files_for_headers
generate_cpp_files_for_header_list = _generate_cpp_files_for_header_list

def select_multi(values_map):
    """select() but allowing multiple matches of the keys.

    select_multi works around a restriction in native select() that prevents multiple
    keys from being matched unless one is a strict subset of another. For some features,
    we allow multiple of that component to be active. For example, with codecs, we let
    the clients mix and match anywhere from 0 built in codecs to all of them.

    select_multi takes a given map and turns it into several distinct select statements
    that have the effect of using any values associated with any active keys.
    For example, if the following parameter is passed in:
        values_map = {
            ":alpha": ["apple", "apricot"],
            ":beta": ["banana"],
            ":gamma": ["grapefruit"],
        }
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

    Returns:
        A list of values that is filled in by the generated select statements.
    """
    if len(values_map) == 0:
        return []
    rv = []
    for key, value in values_map.items():
        rv += select({
            key: value,
            "//conditions:default": [],
        })
    return rv

def skia_cc_binary(name, copts = DEFAULT_COPTS, linkopts = DEFAULT_LINKOPTS, **kwargs):
    """A wrapper around cc_library for Skia C++ executables (e.g. tests).

    This lets us provide compiler flags (copts) and global linker flags (linkopts) consistently
    to Skia built executables. These executables are almost always things like unit tests and
    dev tools.

    Args:
        name: the name of the underlying executable.
        copts: Flags which should be passed to the C++ compiler. By default, we use DEFAULT_COPTS
            from //bazel/copts.bzl.
        linkopts: Global flags which should be passed to the C++ linker. By default, we use
            DEFAULT_LINKOPTS from //bazel/linkopts.bzl. Other linker flags will be passed in
            via deps (see deps_and_linkopts below).
        **kwargs: All the normal arguments that cc_binary takes.
    """
    native.cc_binary(name = name, copts = copts, linkopts = linkopts, **kwargs)

def skia_cc_binary_with_flags(
        name,
        copts = DEFAULT_COPTS,
        linkopts = DEFAULT_LINKOPTS,
        set_flags = None,
        **kwargs):
    cc_binary_with_flags(
        name = name,
        copts = copts,
        linkopts = linkopts,
        set_flags = set_flags,
        **kwargs
    )

def skia_cc_library(name, copts = DEFAULT_COPTS, **kwargs):
    """A wrapper around cc_library for Skia C++ libraries.

    This lets us provide compiler flags (copts) consistently to the Skia build (e.g. //:skia_public)
    and builds which depend on those targets (e.g. things in //tools or //modules).

    It also lets us easily tweak these settings when being built in G3.

    Third party libraries should *not* use this directly, as there are likely some flags used
    by Skia (e.g. warnings) that we do not want to have to fix for third party code.

    Args:
        name: the name of the underlying library.
        copts: Flags which should be passed to the C++ compiler. By default, we use DEFAULT_COPTS
            from //bazel/copts.bzl.
        **kwargs: All the normal arguments that cc_library takes.
    """
    native.cc_library(name = name, copts = copts, **kwargs)

def skia_cc_deps(name, visibility, deps = [], linkopts = [], textual_hdrs = [], testonly = False):
    """A self-documenting wrapper around cc_library for things to pass to the top skia_cc_library.

    It lets us have third_party deps, linkopts, etc be set close to where they impact,
    and trickle up the file hierarchy to //:skia_public and //:skia_internal

    Args:
        name: the name of the underlying target. By convention, this is usually called "deps".
        visibility: To prevent this rule from being used where it should not, we have the
            convention of setting the visibility to just the parent package.
        deps: A list of labels or select statements to collect third_party dependencies.
        linkopts: A list of strings or select statements to collect linker flags.
        textual_hdrs: A list of labels or select statements to collect files which are included, but
            do not have a suffix of .h, like a typical C++ header does.
        testonly: A boolean that, if true, will enforce all targets who depend on this are also
            marked as testonly.
    """
    native.cc_library(
        name = name,
        visibility = visibility,
        deps = deps,
        linkopts = linkopts,
        textual_hdrs = textual_hdrs,
        testonly = testonly,
    )

def skia_defines(name, visibility, defines):
    """A self-documenting wrapper around cc_library for defines"""
    native.cc_library(name = name, visibility = visibility, defines = defines)

def skia_filegroup(**kwargs):
    """A wrapper around filegroup allowing us to customize visibility in G3."""
    native.filegroup(**kwargs)

def skia_objc_library(name, copts = DEFAULT_OBJC_COPTS, **kwargs):
    """A wrapper around cc_library for Skia Objective C libraries.

    This lets us provide compiler flags (copts) consistently to the Skia build (e.g. //:skia_public)
    and builds which depend on those targets (e.g. things in //tools or //modules).

    It also lets us easily tweak these settings when being built in G3.
    Args:
        name: the name of the underlying target.
        copts: Flags which should be passed to the C++ compiler. By default, we use
            DEFAULT_OBJC_COPTS from //bazel/copts.bzl.
        **kwargs: Normal arguments to objc_library
    """

    # Internally, we need to combine sdk_frameworks and deps, but we can only
    # do that if both are lists
    # https://github.com/bazelbuild/bazel/issues/14157
    sdks = kwargs.get("sdk_frameworks", None)
    deps = kwargs.get("deps", [])
    if type(sdks) != "NoneType":
        if type(sdks) != "list" or type(deps) != "list":
            fail("skd_frameworks and deps must both be normal lists, not selects")
    native.objc_library(name = name, copts = copts, **kwargs)

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
        elif f.endswith(".mm"):
            srcs.append(f)
        elif f.endswith(".h"):
            hdrs.append(f)
        else:
            fail("Neither .cpp, .mm, nor .h file " + f)

    if len(srcs) == 0 or len(hdrs) == 0:
        fail("The list consist of either only source or header files. No need to use this macro.")

    skia_filegroup(
        name = name + "_srcs",
        srcs = srcs,
    )
    skia_filegroup(
        name = name + "_hdrs",
        srcs = hdrs,
    )
