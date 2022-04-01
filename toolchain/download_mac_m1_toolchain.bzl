"""
This file assembles a toolchain for a Mac M1 host using the Clang Compiler and glibc.

It downloads the necessary headers, executables, and pre-compiled static/shared libraries to
the external subfolder of the Bazel cache (the same place third party deps are downloaded with
http_archive or similar functions in WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/clang_toolchain_config.bzl)
"""

def _download_mac_m1_toolchain(ctx):
    # TODO(jmbetancourt)
    pass

# https://bazel.build/rules/repository_rules
download_mac_m1_toolchain = repository_rule(
    implementation = _download_mac_m1_toolchain,
    attrs = {},
    doc = "Downloads clang, and all supporting headers, executables, " +
          "and shared libraries required to build Skia on a Mac M1 host",
)
