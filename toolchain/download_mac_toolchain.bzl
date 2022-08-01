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
    ctx.symlink(
        # from =
        res.stdout.rstrip() + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks",
        # to =
        "./symlinks/xcode/MacSDK/Frameworks",
    )

    # Create a BUILD.bazel file that makes the files necessary for compiling,
    # linking and creating archive files visible to Bazel.
    # The smaller the globs are, the more performant the sandboxed builds will be.
    # Additionally, globs that are too wide can pick up infinite symlink loops,
    # and be difficult to quash: https://github.com/bazelbuild/bazel/issues/13950
    # https://bazel.build/rules/lib/repository_ctx#file
    #
    ctx.file(
        "BUILD.bazel",
        content = """
# DO NOT EDIT THIS BAZEL FILE DIRECTLY
# Generated from ctx.file action in download_mac_toolchain.bzl
filegroup(
    name = "archive_files",
    srcs = [
        "bin/llvm-ar",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "compile_files",
    srcs = [
        "bin/clang",
    ] + glob(
        include = [
            "include/c++/v1/**",
            "lib/clang/13.0.0/**",
            "symlinks/xcode/MacSDK/Frameworks/AppKit.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/ApplicationServices.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/Carbon.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CFNetwork.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CloudKit.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/Cocoa.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/ColorSync.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreData.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreFoundation.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreGraphics.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreImage.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreLocation.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreServices.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreText.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/CoreVideo.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/DiskArbitration.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/Foundation.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/ImageIO.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/IOKit.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/IOSurface.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/Metal.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/OpenGL.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/QuartzCore.Framework/**",
            "symlinks/xcode/MacSDK/Frameworks/Security.Framework/**",
            "symlinks/xcode/MacSDK/usr/include/**",
        ],
        allow_empty = False,
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "link_files",
    srcs = [
        "bin/clang",
        "bin/ld.lld",
        "bin/lld",
        "lib/libc++.a",
        "lib/libc++abi.a",
        "lib/libunwind.a",
    ],
    visibility = ["//visibility:public"],
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
