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

load(":clang_layering_check.bzl", "generate_system_module_map")
load(":utils.bzl", "gcs_mirror_url")

# From https://github.com/llvm/llvm-project/releases/tag/llvmorg-15.0.1
# When updating this, don't forget to use //bazel/gcs_mirror to upload a new version.
# go run bazel/gcs_mirror/gcs_mirror.go --url [clang_url] --sha256 [clang_sha256]
clang_prefix_arm64 = "clang+llvm-15.0.1-arm64-apple-darwin21.0"
clang_sha256_arm64 = "858f86d96b5e4880f69f7a583daddbf97ee94e7cffce0d53aa05cba6967f13b8"
clang_url_arm64 = "https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.1/clang+llvm-15.0.1-arm64-apple-darwin21.0.tar.xz"

clang_prefix_amd64 = "clang+llvm-15.0.1-x86_64-apple-darwin"
clang_sha256_amd64 = "0b2f1a811e68d011344103274733b7670c15bbe08b2a3a5140ccad8e19d9311e"
clang_url_amd64 = "https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.1/clang+llvm-15.0.1-x86_64-apple-darwin.tar.xz"

def _get_system_xcode_path(ctx):
    # https://developer.apple.com/library/archive/technotes/tn2339/_index.html
    res = ctx.execute(["xcode-select", "-p"])
    if res.return_code != 0:
        fail("Error Getting XCode path: " + res.stderr)
    return res.stdout.rstrip()

def _delete_macos_sdk_symlinks(ctx):
    ctx.delete("./symlinks/xcode/MacSDK/usr")
    ctx.delete("./symlinks/xcode/MacSDK/System/Library/Frameworks")

def _create_macos_sdk_symlinks(ctx):
    system_xcode_path = _get_system_xcode_path(ctx)

    # https://bazel.build/rules/lib/actions#symlink
    ctx.symlink(
        # from =
        system_xcode_path + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr",
        # to =
        "./symlinks/xcode/MacSDK/usr",
    )

    # It is very important to symlink the frameworks directory to [sysroot]/System/Library/Frameworks
    # because some Frameworks "re-export" other frameworks. These framework paths are relative to
    # the sysroot (which on a typical machine is /), and it is difficult to change these paths.
    # By making the symlinks emulate the original path structure, we can keep those re-exports
    # from breaking.
    ctx.symlink(
        # from =
        system_xcode_path + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks",
        # to =
        "./symlinks/xcode/MacSDK/System/Library/Frameworks",
    )

def _download_mac_toolchain_impl(ctx):
    # https://bazel.build/rules/lib/repository_ctx#os
    # https://bazel.build/rules/lib/repository_os
    if ctx.os.arch == "aarch64":
        clang_url = clang_url_arm64
        clang_sha256 = clang_sha256_arm64
        clang_prefix = clang_prefix_arm64
    else:
        clang_url = clang_url_amd64
        clang_sha256 = clang_sha256_amd64
        clang_prefix = clang_prefix_amd64

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

    _delete_macos_sdk_symlinks(ctx)
    _create_macos_sdk_symlinks(ctx)

    # This list of files lines up with _make_default_flags() in mac_toolchain_config.bzl
    # It is all locations that our toolchain could find a system header.
    builtin_include_directories = [
        "include/c++/v1",
        "lib/clang/15.0.1/include",
        "symlinks/xcode/MacSDK/System/Library/Frameworks",
        "symlinks/xcode/MacSDK/usr/include",
    ]

    generate_system_module_map(
        ctx,
        module_file = "toolchain_system_headers.modulemap",
        folders = builtin_include_directories,
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
# Generated from ctx.file action in download_mac_toolchain.bzl
filegroup(
    name = "generated_module_map",
    srcs = ["toolchain_system_headers.modulemap"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "archive_files",
    srcs = [
        "bin/llvm-ar",
    ],
    visibility = ["//visibility:public"],
)

# Any framework that Skia depends on directly or indirectly needs to be listed here.
FRAMEWORK_GLOB = [
    "symlinks/xcode/MacSDK/System/Library/Frameworks/AppKit.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/ApplicationServices.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Carbon.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CFNetwork.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CloudKit.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Cocoa.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/ColorSync.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreData.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreFoundation.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreGraphics.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreImage.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreLocation.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreServices.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreText.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreVideo.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/DiskArbitration.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Foundation.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/ImageIO.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/IOKit.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/IOSurface.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Metal.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/MetalKit.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/OpenGL.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/QuartzCore.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Security.Framework/**",
]

filegroup(
    name = "compile_files",
    srcs = [
        "bin/clang",
    ] + glob(
        include = [
            "include/c++/v1/**",
            "lib/clang/15.0.1/include/**",
            "symlinks/xcode/MacSDK/usr/include/**",
        ] + FRAMEWORK_GLOB,
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
    ] + glob(
        include = [
            # libc++.tbd and libSystem.tbd live here.
            "symlinks/xcode/MacSDK/usr/lib/*",
        ] + FRAMEWORK_GLOB,
        allow_empty = False,
    ),
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
