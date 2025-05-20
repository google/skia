"""
This file is auto-generated from //bazel/deps_parser
DO NOT MODIFY BY HAND.
Instead, do:
    bazel run //bazel/deps_parser
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

def _c_plus_plus_modules_impl(ctx):
    """A list of native Bazel git rules to download third party git repositories

       These are in the order they appear in //DEPS.
        https://bazel.build/rules/lib/repo/git

    Args:
      ctx: https://bazel.build/rules/lib/builtins/module_ctx
    """
    git_repository(
        name = "brotli",
        commit = "6d03dfbedda1615c4cba1211f8d81735575209c8",
        remote = "https://skia.googlesource.com/external/github.com/google/brotli.git",
    )

    new_git_repository(
        name = "dawn",
        build_file = "//bazel/external/dawn:BUILD.bazel",
        commit = "a3bfb33a777c75f77da36a7eb5ea4be24b86810f",
        remote = "https://dawn.googlesource.com/dawn.git",
    )

    git_repository(
        name = "abseil_cpp",
        commit = "04dc59d2c83238cb1fcb49083e5e416643a899ce",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/abseil-cpp",
    )

    new_git_repository(
        name = "delaunator",
        build_file = "//bazel/external/delaunator:BUILD.bazel",
        commit = "98305ef6c4e862f7d48df9cc647b690d796fec68",
        remote = "https://skia.googlesource.com/external/github.com/skia-dev/delaunator-cpp.git",
    )

    new_git_repository(
        name = "dng_sdk",
        build_file = "//bazel/external/dng_sdk:BUILD.bazel",
        commit = "dbe0a676450d9b8c71bf00688bb306409b779e90",
        remote = "https://android.googlesource.com/platform/external/dng_sdk.git",
    )

    new_git_repository(
        name = "expat",
        build_file = "//bazel/external/expat:BUILD.bazel",
        commit = "8e49998f003d693213b538ef765814c7d21abada",
        remote = "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git",
        patches = ["//bazel/external/expat:config_files.patch"],
    )

    new_git_repository(
        name = "freetype",
        build_file = "//bazel/external/freetype:BUILD.bazel",
        commit = "7172bd11badd468f6a86dba0b1769d624ead885c",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git",
        patches = ["//bazel/external/freetype:config_files.patch"],
    )

    new_git_repository(
        name = "harfbuzz",
        build_file = "//bazel/external/harfbuzz:BUILD.bazel",
        commit = "08b52ae2e44931eef163dbad71697f911fadc323",
        remote = "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git",
        patches = ["//bazel/external/harfbuzz:config_files.patch"],
    )

    git_repository(
        name = "highway",
        commit = "424360251cdcfc314cfc528f53c872ecd63af0f0",
        remote = "https://chromium.googlesource.com/external/github.com/google/highway.git",
    )

    new_git_repository(
        name = "icu",
        build_file = "//bazel/external/icu:BUILD.bazel",
        commit = "364118a1d9da24bb5b770ac3d762ac144d6da5a4",
        remote = "https://chromium.googlesource.com/chromium/deps/icu.git",
        patches = ["//bazel/external/icu:icu_utils.patch"],
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
        build_file = "//bazel/external/icu4x:BUILD.bazel",
        commit = "bcf4f7198d4dc5f3127e84a6ca657c88e7d07a13",
        remote = "https://chromium.googlesource.com/external/github.com/unicode-org/icu4x.git",
    )

    new_git_repository(
        name = "imgui",
        build_file = "//bazel/external/imgui:BUILD.bazel",
        commit = "55d35d8387c15bf0cfd71861df67af8cfbda7456",
        remote = "https://skia.googlesource.com/external/github.com/ocornut/imgui.git",
    )

    new_git_repository(
        name = "libavif",
        build_file = "//bazel/external/libavif:BUILD.bazel",
        commit = "55aab4ac0607ab651055d354d64c4615cf3d8000",
        remote = "https://skia.googlesource.com/external/github.com/AOMediaCodec/libavif.git",
    )

    new_git_repository(
        name = "libgav1",
        build_file = "//bazel/external/libgav1:BUILD.bazel",
        commit = "5cf722e659014ebaf2f573a6dd935116d36eadf1",
        remote = "https://chromium.googlesource.com/codecs/libgav1.git",
    )

    new_git_repository(
        name = "libjpeg_turbo",
        build_file = "//bazel/external/libjpeg_turbo:BUILD.bazel",
        commit = "e14cbfaa85529d47f9f55b0f104a579c1061f9ad",
        remote = "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git",
    )

    new_git_repository(
        name = "libjxl",
        build_file = "//bazel/external/libjxl:BUILD.bazel",
        commit = "a205468bc5d3a353fb15dae2398a101dff52f2d3",
        remote = "https://chromium.googlesource.com/external/gitlab.com/wg1/jpeg-xl.git",
    )

    new_git_repository(
        name = "libpng",
        build_file = "//bazel/external/libpng:BUILD.bazel",
        commit = "ed217e3e601d8e462f7fd1e04bed43ac42212429",
        remote = "https://skia.googlesource.com/third_party/libpng.git",
    )

    new_git_repository(
        name = "libwebp",
        build_file = "//bazel/external/libwebp:BUILD.bazel",
        commit = "845d5476a866141ba35ac133f856fa62f0b7445f",
        remote = "https://chromium.googlesource.com/webm/libwebp.git",
    )

    new_git_repository(
        name = "libyuv",
        build_file = "//bazel/external/libyuv:BUILD.bazel",
        commit = "d248929c059ff7629a85333699717d7a677d8d96",
        remote = "https://chromium.googlesource.com/libyuv/libyuv.git",
    )

    new_git_repository(
        name = "perfetto",
        build_file = "//bazel/external/perfetto:BUILD.bazel",
        commit = "93885509be1c9240bc55fa515ceb34811e54a394",
        remote = "https://android.googlesource.com/platform/external/perfetto",
    )

    new_git_repository(
        name = "piex",
        build_file = "//bazel/external/piex:BUILD.bazel",
        commit = "bb217acdca1cc0c16b704669dd6f91a1b509c406",
        remote = "https://android.googlesource.com/platform/external/piex.git",
    )

    new_git_repository(
        name = "vulkanmemoryallocator",
        build_file = "//bazel/external/vulkanmemoryallocator:BUILD.bazel",
        commit = "a6bfc237255a6bac1513f7c1ebde6d8aed6b5191",
        remote = "https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator",
    )

    new_git_repository(
        name = "spirv_cross",
        build_file = "//bazel/external/spirv_cross:BUILD.bazel",
        commit = "b8fcf307f1f347089e3c46eb4451d27f32ebc8d3",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross",
    )

    git_repository(
        name = "spirv_headers",
        commit = "c9aad99f9276817f18f72a4696239237c83cb775",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git",
    )

    git_repository(
        name = "spirv_tools",
        commit = "66fe610946a6d98169f8ebe9ca483f64c4009fa5",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git",
    )

    new_git_repository(
        name = "vello",
        build_file = "//bazel/external/vello:BUILD.bazel",
        commit = "3ee3bea02164c5a816fe6c16ef4e3a810edb7620",
        remote = "https://skia.googlesource.com/external/github.com/linebender/vello.git",
    )

    new_git_repository(
        name = "vulkan_headers",
        build_file = "//bazel/external/vulkan_headers:BUILD.bazel",
        commit = "75ad707a587e1469fb53a901b9b68fe9f6fbc11f",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers",
    )

    new_git_repository(
        name = "vulkan_tools",
        build_file = "//bazel/external/vulkan_tools:BUILD.bazel",
        commit = "60b640cb931814fcc6dabe4fc61f4738c56579f6",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools",
    )

    new_git_repository(
        name = "vulkan_utility_libraries",
        build_file = "//bazel/external/vulkan_utility_libraries:BUILD.bazel",
        commit = "4f628210460c4df62029959cc7fb237ac75f7189",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Utility-Libraries",
    )

    new_git_repository(
        name = "wuffs",
        build_file = "//bazel/external/wuffs:BUILD.bazel",
        commit = "e3f919ccfe3ef542cfc983a82146070258fb57f8",
        remote = "https://skia.googlesource.com/external/github.com/google/wuffs-mirror-release-c.git",
    )

    new_git_repository(
        name = "zlib",
        build_file = "//bazel/external/zlib:BUILD.bazel",
        commit = "646b7f569718921d7d4b5b8e22572ff6c76f2596",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/zlib",
    )

c_plus_plus_modules = module_extension(
    implementation = _c_plus_plus_modules_impl,
)
