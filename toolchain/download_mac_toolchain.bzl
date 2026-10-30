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

# Prebuilt Clang 24 from Chromium.
# Once infra/bots/assets/clang_mac_{intel,arm} CIPD assets are re-uploaded, these URLs can point
# back to https://chrome-infra-packages.appspot.com/dl/skia/bots/clang_mac_{intel,arm}/+/version:1
clang_tar_file = "clang-llvmorg-24-init-14453-ge063be79-1.tar.xz"
clang_intel_url = (
    "https://commondatastorage.googleapis.com/chromium-browser-clang/Mac/" +
    clang_tar_file
)
clang_intel_sha256 = "ae29ec360eda6e80e96f8638d368ecdeea93d19c18c527c525dc0932bd5243a4"

clang_arm_url = (
    "https://commondatastorage.googleapis.com/chromium-browser-clang/Mac_arm64/" +
    clang_tar_file
)
clang_arm_sha256 = "0a8d887534b3153c7f6860265dc7e075d420174521c60ccd52c2c243949ef468"

# This should be the same across both arm and intel, and must stay in sync with
# CLANG_VER in //toolchain/mac_toolchain_config.bzl.
clang_ver = "24"

def _get_system_sdk_path(ctx):
    res = ctx.execute(["xcrun", "--sdk", "macosx", "--show-sdk-path"])
    if res.return_code != 0:
        fail("Error Getting SDK path: " + res.stderr)
    return res.stdout.rstrip()

def _delete_macos_sdk_symlinks(ctx):
    ctx.delete("./symlinks/xcode/MacSDK/usr")
    ctx.delete("./symlinks/xcode/MacSDK/System/Library/Frameworks")

def _create_macos_sdk_symlinks(ctx):
    system_sdk_path = _get_system_sdk_path(ctx)

    # Watch SDKSettings.plist so that Bazel invalidates and re-runs this
    # repository rule whenever the active Xcode / macOS SDK is updated.
    ctx.watch(system_sdk_path + "/SDKSettings.plist")

    # https://bazel.build/rules/lib/actions#symlink
    ctx.symlink(
        # from =
        system_sdk_path + "/usr",
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
        system_sdk_path + "/System/Library/Frameworks",
        # to =
        "./symlinks/xcode/MacSDK/System/Library/Frameworks",
    )

def _download_mac_toolchain_impl(ctx):
    # https://bazel.build/rules/lib/repository_ctx

    # Download the clang toolchain (the extraction can take a while)
    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    clang_url = clang_intel_url
    clang_sha256 = clang_intel_sha256
    if "arm" in ctx.os.arch or "aarch64" in ctx.os.arch:
        clang_url = clang_arm_url
        clang_sha256 = clang_arm_sha256
    ctx.download_and_extract(
        url = clang_url,
        type = "tar.xz",
        output = "",
        sha256 = clang_sha256,
    )

    # Some std library headers use #include_next to include system specific headers, and
    # some skia source files require Xcode headers when compiling, (see SkTypes.h and look
    # for TargetedConditionals.h) All of these are located in Xcode, stopping the Mac
    # builds from being purely hermetic.
    # For now, we can grab the user's Xcode path by calling xcode-select and create a symlink in
    # our toolchain directory to refer to during compilation.

    _delete_macos_sdk_symlinks(ctx)
    _create_macos_sdk_symlinks(ctx)

    # This list of files lines up with _make_default_flags() in mac_toolchain_config.bzl
    # It is all locations that our toolchain could find a system header.
    builtin_include_directories = [
        "lib/clang/" + clang_ver + "/include",
        # Frameworks is a symlink, and the trailing slash is intentional
        # (to ensure traversal in generate_system_module_map.sh's find).
        "symlinks/xcode/MacSDK/System/Library/Frameworks/",
        "symlinks/xcode/MacSDK/usr/include",
        # The C++ standard library headers are here but that's accounted for above.
        # "symlinks/xcode/MacSDK/usr/include/c++/v1",
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
    "symlinks/xcode/MacSDK/System/Library/Frameworks/_LocationEssentials.framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/AppKit.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/ApplicationServices.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/AVFAudio.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/AVFoundation.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/AVRouting.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Carbon.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CFNetwork.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CloudKit.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Cocoa.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/ColorSync.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreData.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreAudio.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreAudioTypes.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreFoundation.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreGraphics.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreImage.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreLocation.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreMedia.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/CoreMIDI.Framework/**",
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
    "symlinks/xcode/MacSDK/System/Library/Frameworks/ModelIO.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/OpenGL.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/QuartzCore.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Security.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/Symbols.Framework/**",
    "symlinks/xcode/MacSDK/System/Library/Frameworks/UniformTypeIdentifiers.framework/**",
]

filegroup(
    name = "compile_files",
    srcs = [
        "bin/clang",
    ] + glob(
        include = [
            "lib/clang/*/include/**",
            "symlinks/xcode/MacSDK/usr/include/**",
        ],
        allow_empty = False,
    ) + glob(
        # Frameworks are SDK-dependent, and can vary between releases.
        # We attempt to capture a super-set.
        include = FRAMEWORK_GLOB,
        allow_empty = True,
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "link_files",
    srcs = [
        "bin/clang",
        "bin/ld.lld",
        "bin/lld",
    ] + glob(
        include = [
            # libc++.tbd and libSystem.tbd live here.
            "symlinks/xcode/MacSDK/usr/lib/*",
        ],
        allow_empty = False,
    ) + glob(
        # Frameworks are SDK-dependent, and can vary between releases.
        # We attempt to capture a super-set.
        include = FRAMEWORK_GLOB,
        allow_empty = True,
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
