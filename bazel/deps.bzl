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
        commit = "68d8508758f256704de0ea735c5ec81f46dd2c32",
        remote = "https://dawn.googlesource.com/dawn.git",
    )

    git_repository(
        name = "abseil_cpp",
        commit = "65a55c2ba891f6d2492477707f4a2e327a0b40dc",
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
        commit = "624da0f593bb8d7e146b9f42b06d8e6c80d032a3",
        remote = "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git",
    )

    new_git_repository(
        name = "freetype",
        build_file = ws + "//bazel/external/freetype:BUILD.bazel",
        commit = "83af801b552111e37d9466a887e1783a0fb5f196",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git",
    )

    new_git_repository(
        name = "harfbuzz",
        build_file = ws + "//bazel/external/harfbuzz:BUILD.bazel",
        commit = "a070f9ebbe88dc71b248af9731dd49ec93f4e6e6",
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
        commit = "364118a1d9da24bb5b770ac3d762ac144d6da5a4",
        remote = "https://chromium.googlesource.com/chromium/deps/icu.git",
        patch_cmds = [
            "rm source/i18n/BUILD.bazel",
            "rm source/common/BUILD.bazel",
            "rm source/stubdata/BUILD.bazel",
        ],
        patch_cmds_win = [
            "del source/i18n/BUILD.bazel",
            "del source/common/BUILD.bazel",
            "del source/stubdata/BUILD.bazel",
        ],
    )

    new_git_repository(
        name = "icu4x",
        build_file = ws + "//bazel/external/icu4x:BUILD.bazel",
        commit = "bcf4f7198d4dc5f3127e84a6ca657c88e7d07a13",
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
        commit = "ccfbe1c82a3b6dbe8647ceb36a3f9ee711fba3cf",
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
        commit = "ed217e3e601d8e462f7fd1e04bed43ac42212429",
        remote = "https://skia.googlesource.com/third_party/libpng.git",
    )

    new_git_repository(
        name = "libwebp",
        build_file = ws + "//bazel/external/libwebp:BUILD.bazel",
        commit = "845d5476a866141ba35ac133f856fa62f0b7445f",
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
        commit = "b8fcf307f1f347089e3c46eb4451d27f32ebc8d3",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross",
    )

    git_repository(
        name = "spirv_headers",
        commit = "07ddb1c0f1ffa929262d4568481a692bb0fb1535",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git",
    )

    git_repository(
        name = "spirv_tools",
        commit = "fcf994a619608c2bdb505189f6e325b1a6b4f294",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git",
    )

    new_git_repository(
        name = "vello",
        build_file = ws + "//bazel/external/vello:BUILD.bazel",
        commit = "3ee3bea02164c5a816fe6c16ef4e3a810edb7620",
        remote = "https://skia.googlesource.com/external/github.com/linebender/vello.git",
    )

    new_git_repository(
        name = "vulkan_headers",
        build_file = ws + "//bazel/external/vulkan_headers:BUILD.bazel",
        commit = "14345dab231912ee9601136e96ca67a6e1f632e7",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers",
    )

    new_git_repository(
        name = "vulkan_tools",
        build_file = ws + "//bazel/external/vulkan_tools:BUILD.bazel",
        commit = "c9a5acda16dc2759457dc856b5d7df00ac5bf4a2",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools",
    )

    new_git_repository(
        name = "vulkan_utility_libraries",
        build_file = ws + "//bazel/external/vulkan_utility_libraries:BUILD.bazel",
        commit = "8c907ea21fe0147f791d79051b18e21bc8c4ede0",
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
        commit = "646b7f569718921d7d4b5b8e22572ff6c76f2596",
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
    skia_revision = "d211141c45c9171437fa8e6e07989edb5bffa17a"
    maybe(
        download_config_files,
        name = "expat_config",
        skia_revision = skia_revision,
        files = {
            "BUILD.bazel": "third_party/expat/include/BUILD.bazel",
            "expat_config/expat_config.h": "third_party/expat/include/expat_config/expat_config.h",
        },
    )
    maybe(
        download_config_files,
        name = "freetype_config",
        skia_revision = skia_revision,
        files = {
            "BUILD.bazel": "third_party/freetype2/include/BUILD.bazel",
            "freetype-android/freetype/config/ftmodule.h": "third_party/freetype2/include/freetype-android/freetype/config/ftmodule.h",
            "freetype-android/freetype/config/ftoption.h": "third_party/freetype2/include/freetype-android/freetype/config/ftoption.h",
            "freetype-no-type1/freetype/config/ftmodule.h": "third_party/freetype2/include/freetype-no-type1/freetype/config/ftmodule.h",
            "freetype-no-type1/freetype/config/ftoption.h": "third_party/freetype2/include/freetype-no-type1/freetype/config/ftoption.h",
        },
    )
    maybe(
        download_config_files,
        name = "harfbuzz_config",
        skia_revision = skia_revision,
        files = {
            "BUILD.bazel": "third_party/harfbuzz/BUILD.bazel",
            "config-override.h": "third_party/harfbuzz/config-override.h",
        },
    )
    maybe(
        download_config_files,
        name = "icu_utils",
        skia_revision = skia_revision,
        files = {
            "BUILD.bazel": "third_party/icu/BUILD.bazel",
            "SkLoadICU.cpp": "third_party/icu/SkLoadICU.cpp",
            "SkLoadICU.h": "third_party/icu/SkLoadICU.h",
            "make_data_cpp.py": "third_party/icu/make_data_cpp.py",
        },
    )
