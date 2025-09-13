"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains macros which require no third-party dependencies.
Using these where possible makes it easier for clients to use Skia
without needing to download a bunch of unnecessary dependencies
in their WORKSPACE.bazel file.
"""

load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("@rules_cc//cc:objc_library.bzl", "objc_library")
load("@skia_user_config//:copts.bzl", "DEFAULT_COPTS", "DEFAULT_OBJC_COPTS")
load("@skia_user_config//:linkopts.bzl", "DEFAULT_LINKOPTS")
load(
    "//bazel:generate_cpp_files_for_headers.bzl",
    _generate_cpp_files_for_headers = "generate_cpp_files_for_headers",
)

generate_cpp_files_for_headers = _generate_cpp_files_for_headers

def select_multi(values_map, default_cases = None):
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
        default_cases: dictionary of labels to a list of labels to be used in the default case.
                       If not provided or a key is not mentioned, an empty list will be used.

    Returns:
        A list of values that is filled in by the generated select statements.
    """
    if len(values_map) == 0:
        return []
    rv = []
    if not default_cases:
        default_cases = {}
    for key, value in values_map.items():
        rv += select({
            key: value,
            "//conditions:default": default_cases.get(key, []),
        })
    return rv

def supports_platforms(*platforms):
    """Convenience macro to set the "target_compatible_with" argument of binary and test targets.

    The example below shows a binary that is compatible with all desktop OSes:

        skia_cc_binary(
            name = "my_binary",
            target_compatible_with = supports_platforms(
                "@platforms//os:linux",
                "@platforms//os:windows",
                "@platforms//os:macos",
            ),
            ...
        )

    Args:
        *platforms: One or more supported platforms, e.g. "@platforms//os:linux".
    Returns:
        A select() statement with an empty list for each provided platform, and
            ["@platforms//:incompatible"] as the default condition.
    """
    if len(platforms) == 0:
        fail("Please provide at least one platform.")

    platforms_map = {
        "//conditions:default": ["@platforms//:incompatible"],
    }
    for platform in platforms:
        platforms_map[platform] = []
    return select(platforms_map)

def skia_cc_binary(name, copts = DEFAULT_COPTS, linkopts = DEFAULT_LINKOPTS, **kwargs):
    """A wrapper around cc_library for Skia C++ executables (e.g. tools).

    This lets us provide compiler flags (copts) and global linker flags (linkopts) consistently
    to Skia built executables. These executables are almost always things like dev tools.

    Args:
        name: the name of the underlying executable.
        copts: Flags which should be passed to the C++ compiler. By default, we use DEFAULT_COPTS
            from @skia_user_config//:copts.bzl.
        linkopts: Global flags which should be passed to the C++ linker. By default, we use
            DEFAULT_LINKOPTS from  @skia_user_config//:linkopts.bzl. Other linker flags will be
            passed in via deps (see deps_and_linkopts below).
        **kwargs: All the normal arguments that cc_binary takes.
    """
    cc_binary(name = name, copts = copts, linkopts = linkopts, **kwargs)

def skia_cc_test(name, copts = DEFAULT_COPTS, linkopts = DEFAULT_LINKOPTS, **kwargs):
    """A wrapper around cc_test for Skia C++ executables (e.g. tests).

    This lets us provide compiler flags (copts) and global linker flags (linkopts) consistently
    to Skia built executables, that is, tests.

    Args:
        name: the name of the underlying executable.
        copts: Flags which should be passed to the C++ compiler. By default, we use DEFAULT_COPTS
            from @skia_user_config//:copts.bzl.
        linkopts: Global flags which should be passed to the C++ linker. By default, we use
            DEFAULT_LINKOPTS from  @skia_user_config//:linkopts.bzl. Other linker flags will be
            passed in via deps (see deps_and_linkopts below).
        **kwargs: All the normal arguments that cc_binary takes.
    """
    cc_test(name = name, copts = copts, linkopts = linkopts, **kwargs)

def skia_cc_library(name, copts = DEFAULT_COPTS, local_defines = [], **kwargs):
    """A wrapper around cc_library for Skia C++ libraries.

    This lets us provide compiler flags (copts) consistently to the Skia build. By default,
    copts do not flow up the dependency stack. Additionally, in G3, this allows us to set
    some options universally.

    It also lets us easily tweak these settings when being built in G3.

    Third party libraries should *not* use this directly, as there are likely some flags used
    by Skia (e.g. warnings) that we do not want to have to fix for third party code.

    Args:
        name: the name of the underlying library.
        copts: Flags which should be passed to the C++ compiler. By default, we use DEFAULT_COPTS
            from @skia_user_config//:copts.bzl.
        local_defines: Defines set when compiling this library, but not dependents. We
            add a define to all our libraries to correctly export/import symbols.
        **kwargs: All the normal arguments that cc_library takes.
    """

    # This allows us to mark APIs as exported when building this
    # as a library, but the APIs will be marked as an import
    # (the default) when clients try to use our headers. See SkAPI.h for more.
    # We have to create a new (mutable) list since if the client passes in a list
    # it will be immutable ("frozen").
    ld = []
    ld.extend(local_defines)
    ld.append("SKIA_IMPLEMENTATION=1")
    cc_library(name = name, copts = copts, local_defines = ld, **kwargs)

def skia_filegroup(**kwargs):
    """A wrapper around filegroup allowing us to customize visibility in G3."""
    native.filegroup(**kwargs)

_COMPILE_HEADERS = (".h", ".hh", ".hpp", "hxx")

def _headers(files):
    return [f for f in files if f.endswith(_COMPILE_HEADERS)]

def skia_objc_library(
        name,
        copts = DEFAULT_OBJC_COPTS,
        hdrs = [],
        deps = [],
        srcs = [],
        non_arc_srcs = [],
        ios_frameworks = [],
        mac_frameworks = [],
        sdk_frameworks = [],
        **kwargs):
    """A wrapper around objc_library for Skia Objective C libraries.

    This lets us provide compiler flags (copts) consistently to the Skia build (e.g. //:core)
    and builds which depend on those targets (e.g. things in //tools or //modules).

    It also lets us easily tweak these settings when being built in G3.
    Args:
        name: the name of the underlying target.
        copts: Flags which should be passed to the C++ compiler. By default, we use
            DEFAULT_OBJC_COPTS from @skia_user_config//:copts.bzl.
        deps: https://bazel.build/reference/be/objective-c#objc_library.deps
        ios_frameworks: A list (not select) of iOS-specific Frameworks.
        mac_frameworks: A list (not select) of Mac-specific Frameworks.
        sdk_frameworks: https://bazel.build/reference/be/objective-c#objc_library.sdk_frameworks
                        except this should only be a list, not a select.
        **kwargs: Normal arguments to objc_library

    We re-implement the non_arc_srcs attribute because at least on a Mac with Bazel 8.2.1 it
    crashes if we pass non_arc_srcs straight through to the objc_library (explicitly or in kwargs).
    """
    if len(ios_frameworks) > 0 or len(mac_frameworks) > 0:
        sdk_frameworks += select({
            "@platforms//os:ios": ios_frameworks,
            "@platforms//os:macos": mac_frameworks,
            "//conditions:default": [],
        })
    if len(non_arc_srcs) > 0:
        objc_library(
            name = name + "_non_arc",
            copts = copts + ["-fno-objc-arc"],
            deps = deps,
            # Make sure header files in hdrs and srcs that are required by non_arc_srcs are visible.
            srcs = non_arc_srcs + _headers(srcs) + hdrs,
            sdk_frameworks = sdk_frameworks,
            **kwargs
        )
        deps = deps + [name + "_non_arc"]
    objc_library(
        name = name,
        copts = copts,
        deps = deps,
        hdrs = hdrs,
        # Make sure any headers in non_arc_srcs are visible here
        srcs = srcs + _headers(non_arc_srcs),
        sdk_frameworks = sdk_frameworks,
        **kwargs
    )

def split_srcs_and_hdrs(name, files, visibility = None):
    """Take a list of files and creates filegroups for C++ sources and headers.

    The "_srcs" and "_hdrs" filegroups will only be created if there are a non-zero amount
    of files of both types. Otherwise, it will fail because we do not need the macro.

    Args:
        name: The prefix of the generated filegroups. One will have the suffix "_srcs" and
            the other "_hdrs".
        files: List of file names, e.g. ["SkAAClip.cpp", "SkAAClip.h"]
        visibility: Optional list of visibility rules
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
        visibility = visibility,
    )
    skia_filegroup(
        name = name + "_hdrs",
        srcs = hdrs,
        visibility = visibility,
    )
