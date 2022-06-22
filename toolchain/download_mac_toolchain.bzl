"""
This file assembles a toolchain for a Mac host (either M1 or Intel) using the Clang Compiler
and a locally-installed XCode.

It downloads the necessary executables and creates symlinks in the external subfolder of the Bazel
cache (the same place third party deps are downloaded with http_archive or similar functions in
WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/mac_toolchain_config.bzl)

The destination folder for these files and symlinks are:
  [outputRoot (aka Bazel cache)]/[outputUserRoot]/[outputBase]/external/clang_mac
  (See https://bazel.build/docs/output_directories#layout-diagram)
"""

load("//toolchain:utils.bzl", "gcs_mirror_url")

# From https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-apple-darwin.tar.xz
# When updating this, don't forget to use //bazel/gcs_mirror to upload a new version.
# go run bazel/gcs_mirror/gcs_mirror.go --url [clang_url] --sha256 [clang_sha256]
clang_prefix = "clang+llvm-13.0.0-x86_64-apple-darwin"
clang_sha256 = "d051234eca1db1f5e4bc08c64937c879c7098900f7a0370f3ceb7544816a8b09"
clang_url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-apple-darwin.tar.xz"

def _download_mac_toolchain_impl(ctx):
    # Download the clang toolchain (the extraction can take a while)
    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    ctx.download_and_extract(
        url = gcs_mirror_url(clang_url, clang_sha256),
        output = "",
        stripPrefix = clang_prefix,
        sha256 = clang_sha256,
    )

    # Some std library headers use #include_next to include system specific headers, and
    # some skia source files require Xcode headers when compiling, (see SkTypes.h and look
    # for TargetedConditionals.h)) All of these are located in Xcode, stopping the Mac
    # builds from being purely hermetic.
    # For now, we can grab the user's Xcode path by calling xcode-select and create a symlink in
    # our toolchain directory to refer to during compilation.

    # https://developer.apple.com/library/archive/technotes/tn2339/_index.html
    res = ctx.execute(["xcode-select", "-p"])

    # https://bazel.build/rules/lib/actions#symlink
    ctx.symlink(
        # from =
        res.stdout.rstrip() + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr",
        # to =
        "./symlinks/xcode/MacSDK/usr",
    )

    # Create a BUILD.bazel file that makes all the files in this subfolder
    # except for those in the share directory. They are not necessary for building
    # and create a symlink looping error when resolving the filegroup.
    # available for use in rules, i.e. in the toolchain declaration.
    # https://bazel.build/rules/lib/repository_ctx#file
    # TODO(kjlubick, jmbetancourt): Let's have compile-specific and link-specific rules, similar
    #  to https://skia-review.googlesource.com/c/skia/+/547822 and
    #  https://github.com/emscripten-core/emsdk/commit/93f21c9ef30bab52de24f9d4ea3f2f377cf6326a
    ctx.file(
        "BUILD.bazel",
        content = """
# DO NOT EDIT THIS BAZEL FILE DIRECTLY
# Generated from ctx.file action in download_mac_toolchain.bzl
filegroup(
    name = "all_files",
    srcs = glob(
                ["**"],
                exclude = [
                    "symlinks/xcode/MacSDK/usr/share/**"
                ]),
    visibility = ["//visibility:public"]
)
""",
        executable = False,
    )

# https://bazel.build/rules/repository_rules
download_mac_toolchain = repository_rule(
    implementation = _download_mac_toolchain_impl,
    attrs = {},
    doc = "Downloads clang to build Skia with." +
          "Assumes you have xcode located on your device and have" +
          "xcode-select in your $PATH.",
)
