"""
This file is auto-generated from //bazel/deps_parser
DO NOT MODIFY BY HAND.
Instead, do:
    bazel run //bazel/deps_parser
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")
load("//bazel:gcs_mirror.bzl", "gcs_mirror_url")

def c_plus_plus_deps(ws = "@skia"):
    """A list of native Bazel git rules to download third party git repositories

       These are in the order they appear in //DEPS.
        https://bazel.build/rules/lib/repo/git

    Args:
      ws: The name of the Skia Bazel workspace. The default, "@", may be when used from within the
          Skia workspace.
    """
    git_repository(
        name = "brotli",
        commit = "6d03dfbedda1615c4cba1211f8d81735575209c8",
        remote = "https://skia.googlesource.com/external/github.com/google/brotli.git",
    )

    new_git_repository(
        name = "dawn",
        build_file = ws + "//bazel/external/dawn:BUILD.bazel",
        commit = "a42cebae4e42103c82ca58e5abdc2ba9fcffae84",
        remote = "https://dawn.googlesource.com/dawn.git",
    )

    git_repository(
        name = "abseil_cpp",
        commit = "cb436cf0142b4cbe47aae94223443df7f82e2920",
        remote = "https://skia.googlesource.com/external/github.com/abseil/abseil-cpp.git",
    )

    new_git_repository(
        name = "dng_sdk",
        build_file = ws + "//bazel/external/dng_sdk:BUILD.bazel",
        commit = "c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
        remote = "https://android.googlesource.com/platform/external/dng_sdk.git",
    )

    new_git_repository(
        name = "expat",
        build_file = ws + "//bazel/external/expat:BUILD.bazel",
        commit = "441f98d02deafd9b090aea568282b28f66a50e36",
        remote = "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git",
    )

    new_git_repository(
        name = "freetype",
        build_file = ws + "//bazel/external/freetype:BUILD.bazel",
        commit = "45903920b984540bb629bc89f4c010159c23a89a",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git",
    )

    new_git_repository(
        name = "harfbuzz",
        build_file = ws + "//bazel/external/harfbuzz:BUILD.bazel",
        commit = "4cfc6d8e173e800df086d7be078da2e8c5cfca19",
        remote = "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git",
    )

    git_repository(
        name = "highway",
        commit = "424360251cdcfc314cfc528f53c872ecd63af0f0",
        remote = "https://chromium.googlesource.com/external/github.com/google/highway.git",
    )

    new_git_repository(
        name = "icu",
        build_file = ws + "//bazel/external/icu:BUILD.bazel",
        commit = "a0718d4f121727e30b8d52c7a189ebf5ab52421f",
        remote = "https://chromium.googlesource.com/chromium/deps/icu.git",
    )

    new_git_repository(
        name = "imgui",
        build_file = ws + "//bazel/external/imgui:BUILD.bazel",
        commit = "55d35d8387c15bf0cfd71861df67af8cfbda7456",
        remote = "https://skia.googlesource.com/external/github.com/ocornut/imgui.git",
    )

    new_git_repository(
        name = "libavif",
        build_file = ws + "//bazel/external/libavif:BUILD.bazel",
        commit = "f49462dc93784bf34148715eee36ab6697ca0b35",
        remote = "https://skia.googlesource.com/external/github.com/AOMediaCodec/libavif.git",
    )

    new_git_repository(
        name = "libgav1",
        build_file = ws + "//bazel/external/libgav1:BUILD.bazel",
        commit = "0fb779c1e169fe6c229cd1fa9cc6ea6feeb441da",
        remote = "https://chromium.googlesource.com/codecs/libgav1.git",
    )

    new_git_repository(
        name = "libjpeg_turbo",
        build_file = ws + "//bazel/external/libjpeg_turbo:BUILD.bazel",
        commit = "ed683925e4897a84b3bffc5c1414c85b97a129a3",
        remote = "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git",
    )

    new_git_repository(
        name = "libjxl",
        build_file = ws + "//bazel/external/libjxl:BUILD.bazel",
        commit = "a205468bc5d3a353fb15dae2398a101dff52f2d3",
        remote = "https://chromium.googlesource.com/external/gitlab.com/wg1/jpeg-xl.git",
    )

    new_git_repository(
        name = "libpng",
        build_file = ws + "//bazel/external/libpng:BUILD.bazel",
        commit = "386707c6d19b974ca2e3db7f5c61873813c6fe44",
        remote = "https://skia.googlesource.com/third_party/libpng.git",
    )

    new_git_repository(
        name = "libwebp",
        build_file = ws + "//bazel/external/libwebp:BUILD.bazel",
        commit = "2af26267cdfcb63a88e5c74a85927a12d6ca1d76",
        remote = "https://chromium.googlesource.com/webm/libwebp.git",
    )

    new_git_repository(
        name = "libyuv",
        build_file = ws + "//bazel/external/libyuv:BUILD.bazel",
        commit = "d248929c059ff7629a85333699717d7a677d8d96",
        remote = "https://chromium.googlesource.com/libyuv/libyuv.git",
    )

    new_git_repository(
        name = "perfetto",
        build_file = ws + "//bazel/external/perfetto:BUILD.bazel",
        commit = "93885509be1c9240bc55fa515ceb34811e54a394",
        remote = "https://android.googlesource.com/platform/external/perfetto",
    )

    new_git_repository(
        name = "piex",
        build_file = ws + "//bazel/external/piex:BUILD.bazel",
        commit = "bb217acdca1cc0c16b704669dd6f91a1b509c406",
        remote = "https://android.googlesource.com/platform/external/piex.git",
    )

    new_git_repository(
        name = "vulkanmemoryallocator",
        build_file = ws + "//bazel/external/vulkanmemoryallocator:BUILD.bazel",
        commit = "a6bfc237255a6bac1513f7c1ebde6d8aed6b5191",
        remote = "https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator",
    )

    new_git_repository(
        name = "spirv_cross",
        build_file = ws + "//bazel/external/spirv_cross:BUILD.bazel",
        commit = "6e1fb9b09efadee38748e0fd0e6210d329087e89",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross",
    )

    git_repository(
        name = "spirv_headers",
        commit = "79743b899fde5c954897b2694291002626358fac",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git",
    )

    git_repository(
        name = "spirv_tools",
        commit = "1bc0e6f59abc3c9cd75f93baef47e9612a448045",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git",
    )

    new_git_repository(
        name = "vello",
        build_file = ws + "//bazel/external/vello:BUILD.bazel",
        commit = "ee3a076b291d206c361431cc841407adf265c692",
        remote = "https://skia.googlesource.com/external/github.com/linebender/vello.git",
    )

    new_git_repository(
        name = "vulkan_headers",
        build_file = ws + "//bazel/external/vulkan_headers:BUILD.bazel",
        commit = "df60f0316899460eeaaefa06d2dd7e4e300c1604",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers",
    )

    new_git_repository(
        name = "vulkan_tools",
        build_file = ws + "//bazel/external/vulkan_tools:BUILD.bazel",
        commit = "576f3286e10ca0c98fc481787b27e229bd61ac8f",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools",
    )

    new_git_repository(
        name = "vulkan_utility_libraries",
        build_file = ws + "//bazel/external/vulkan_utility_libraries:BUILD.bazel",
        commit = "8486ee700e2b2ce47fbbf7394ce8afe7f023f338",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Utility-Libraries",
    )

    new_git_repository(
        name = "wuffs",
        build_file = ws + "//bazel/external/wuffs:BUILD.bazel",
        commit = "e3f919ccfe3ef542cfc983a82146070258fb57f8",
        remote = "https://skia.googlesource.com/external/github.com/google/wuffs-mirror-release-c.git",
    )

    new_git_repository(
        name = "zlib_skia",
        build_file = ws + "//bazel/external/zlib_skia:BUILD.bazel",
        commit = "c876c8f87101c5a75f6014b0f832499afeb65b73",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/zlib",
    )

def bazel_deps():
    maybe(
        http_archive,
        name = "bazel_skylib",
        sha256 = "c6966ec828da198c5d9adbaa94c05e3a1c7f21bd012a0b29ba8ddbccb2c93b0d",
        urls = gcs_mirror_url(
            sha256 = "c6966ec828da198c5d9adbaa94c05e3a1c7f21bd012a0b29ba8ddbccb2c93b0d",
            url = "https://github.com/bazelbuild/bazel-skylib/releases/download/1.1.1/bazel-skylib-1.1.1.tar.gz",
        ),
    )

    maybe(
        http_archive,
        name = "bazel_toolchains",
        sha256 = "e52789d4e89c3e2dc0e3446a9684626a626b6bec3fde787d70bae37c6ebcc47f",
        strip_prefix = "bazel-toolchains-5.1.1",
        urls = gcs_mirror_url(
            sha256 = "e52789d4e89c3e2dc0e3446a9684626a626b6bec3fde787d70bae37c6ebcc47f",
            url = "https://github.com/bazelbuild/bazel-toolchains/archive/refs/tags/v5.1.1.tar.gz",
        ),
    )
