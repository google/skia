"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains flags for the C++ linker, referred to by Bazel as linkopts.

For similar reasons as ./copts.bzl, we define "global" flags we want to pass to the linker
here. We do allow subpackages to conditionally set linkopts because that is likely to be more
readable than trying to express with select statements whether a library should be linked against
because the relevant Skia source file was compiled in.

"""

CORE_LINKOPTS = select({
    "@platforms//os:android": [
        "-landroid",
        "-ldl",
        "-llog",  # Provides __android_log_vprint, needed by //src/ports/SkDebug_android.cpp.
    ],
    "//conditions:default": [],
})

OPT_LEVEL = select({
    "//bazel/common_config_settings:debug_build": [],
    "//bazel/common_config_settings:fast_build_linux": [
        "-Wl,--strip-debug",
    ],
    "//bazel/common_config_settings:fast_build_mac": [],
    "//bazel/common_config_settings:release_build_mac": [
        "-dead_strip",
    ],
    "//bazel/common_config_settings:release_build_linux": [
        "-Wl,--gc-sections",
        "-Wl,--strip-all",
    ],
    "//conditions:default": [],
})

DEFAULT_LINKOPTS = CORE_LINKOPTS + OPT_LEVEL
