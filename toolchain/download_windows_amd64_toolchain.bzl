"""
This file assembles a toolchain for a Windows host using the Clang Compiler.

It downloads the necessary executables and creates symlinks in the external subfolder of the Bazel
cache (the same place third party deps are downloaded with http_archive or similar functions in
WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/windows_toolchain_config.bzl)

The destination folder for these files and symlinks are:
  [outputRoot (aka Bazel cache)]/[outputUserRoot]/[outputBase]/external/clang_windows_amd64
  (See https://bazel.build/docs/output_directories#layout-diagram)
"""

load(":utils.bzl", "gcs_mirror_url")

# From https://github.com/llvm/llvm-project/releases/tag/llvmorg-18.1.8
# When updating this, don't forget to use //bazel/gcs_mirror to upload a new version.
# go run bazel/gcs_mirror/gcs_mirror.go --url [clang_url] --sha256 [clang_sha256]
# We're using a newer version on Windows than Linux or Mac because major version 18 is the earliest
# version to include pre-builts for Windows.
clang_prefix = "clang+llvm-18.1.8-x86_64-pc-windows-msvc"
clang_sha256 = "22c5907db053026cc2a8ff96d21c0f642a90d24d66c23c6d28ee7b1d572b82e8"
clang_url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-x86_64-pc-windows-msvc.tar.xz"

def _download_windows_amd64_toolchain_impl(ctx):
    # Download the clang toolchain (the extraction can take a while)
    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    ctx.download_and_extract(
        url = gcs_mirror_url(clang_url, clang_sha256),
        output = "",
        stripPrefix = clang_prefix,
        sha256 = clang_sha256,
    )

    # Create a BUILD.bazel file that makes the files necessary for compiling,
    # linking and creating archive files visible to Bazel.
    # The smaller the globs are, the more performant the sandboxed builds will be.
    # Additionally, globs that are too wide can pick up infinite symlink loops,
    # and be difficult to quash: https://github.com/bazelbuild/bazel/issues/13950
    # https://bazel.build/rules/lib/repository_ctx#file
    ctx.file(
        "BUILD.bazel",
        content = """
# DO NOT EDIT THIS BAZEL FILE DIRECTLY
# Generated from ctx.file action in download_windows_amd64_toolchain.bzl
filegroup(
    name = "archive_files",
    srcs = [
        "bin/llvm-ar.exe",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "compile_files",
    srcs = [
        "bin/clang.exe",
    ] + glob(
        include = [
            "lib/clang/18/include/**",
        ],
        allow_empty = False,
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "link_files",
    srcs = [
        "bin/clang.exe",
        "bin/ld.lld.exe",
        "bin/lld.exe",
    ],
    visibility = ["//visibility:public"],
)
""",
        executable = False,
    )

# https://bazel.build/rules/repository_rules
download_windows_amd64_toolchain = repository_rule(
    implementation = _download_windows_amd64_toolchain_impl,
    attrs = {},
    doc = "Downloads clang to build Skia with." +
          " Assumes you have msvc located on your device and have" +
          " VCToolsInstallDir, WindowsSdkDir, and WindowsSDKVersion in your" +
          " environment, or have MSVC and Windows SDK installed in a" +
          " predictable location.",
)
