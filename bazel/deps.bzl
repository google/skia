"""
This file is auto-generated from //bazel/deps_parser
DO NOT MODIFY BY HAND.
Instead, do:
    bazel run //bazel/deps_parser
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")
load("//bazel:download_config_files.bzl", "download_config_files")
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
        commit = "e2be33f0b421ba63985f5fc0ecf060614782993e",
        remote = "https://dawn.googlesource.com/dawn.git",
    )

    git_repository(
        name = "abseil_cpp",
        commit = "334aca32051ef6ede2711487acf45d959e9bdffc",
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
        name = "icu4x",
        build_file = ws + "//bazel/external/icu4x:BUILD.bazel",
        commit = "4f81635489681ecf7707623177123cb78d6a66a0",
        remote = "https://chromium.googlesource.com/external/github.com/unicode-org/icu4x.git",
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
        commit = "55aab4ac0607ab651055d354d64c4615cf3d8000",
        remote = "https://skia.googlesource.com/external/github.com/AOMediaCodec/libavif.git",
    )

    new_git_repository(
        name = "libgav1",
        build_file = ws + "//bazel/external/libgav1:BUILD.bazel",
        commit = "5cf722e659014ebaf2f573a6dd935116d36eadf1",
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
        commit = "03b485dc47c6e84a15936601e8b121d84d8ddadf",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross",
    )

    git_repository(
        name = "spirv_headers",
        commit = "1c9115b562bab79ee2160fbd845f41b815b9f21f",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git",
    )

    git_repository(
        name = "spirv_tools",
        commit = "ad11927e6cb251624f884327ede64a642be11f06",
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
        commit = "ea45703effcb01df0856628286f8a890dd313ecd",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers",
    )

    new_git_repository(
        name = "vulkan_tools",
        build_file = ws + "//bazel/external/vulkan_tools:BUILD.bazel",
        commit = "5a5e17fbcc09d189f8b730d042af4a2c79920cc7",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools",
    )

    new_git_repository(
        name = "vulkan_utility_libraries",
        build_file = ws + "//bazel/external/vulkan_utility_libraries:BUILD.bazel",
        commit = "b6f52799ec558808c7cad9522cf3b4fe75ee97d5",
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

def header_based_configs():
    maybe(
        download_config_files,
        name = "freetype_config",
        skia_revision = "7b730016006e6b66d24a6f94eefe8bec00ac1674",
        files = {
            "BUILD.bazel": "bazel/external/freetype/config/BUILD.bazel",
            "android/freetype/config/ftmodule.h": "third_party/freetype2/include/freetype-android/freetype/config/ftmodule.h",
            "android/freetype/config/ftoption.h": "third_party/freetype2/include/freetype-android/freetype/config/ftoption.h",
            "no-type1/freetype/config/ftmodule.h": "third_party/freetype2/include/freetype-no-type1/freetype/config/ftmodule.h",
            "no-type1/freetype/config/ftoption.h": "third_party/freetype2/include/freetype-no-type1/freetype/config/ftoption.h",
        },
    )
    maybe(
        download_config_files,
        name = "harfbuzz_config",
        skia_revision = "7b730016006e6b66d24a6f94eefe8bec00ac1674",
        files = {
            "BUILD.bazel": "bazel/external/harfbuzz/config/BUILD.bazel",
            "config-override.h": "third_party/harfbuzz/config-override.h",
        },
    )
    maybe(
        download_config_files,
        name = "icu_utils",
        skia_revision = "7b730016006e6b66d24a6f94eefe8bec00ac1674",
        files = {
            "BUILD.bazel": "bazel/external/icu/utils/BUILD.bazel",
            "icu/SkLoadICU.cpp": "third_party/icu/SkLoadICU.cpp",
            "icu/SkLoadICU.h": "third_party/icu/SkLoadICU.h",
            "icu/make_data_cpp.py": "third_party/icu/make_data_cpp.py",
        },
    )
