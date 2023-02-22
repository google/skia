"""
This file is auto-generated from //bazel/deps_parser
DO NOT MODIFY BY HAND.
Instead, do:
    bazel run //bazel/deps_parser
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

def git_repos_from_deps():
    """A list of native Bazel git rules to download third party git repositories

       These are in the order they appear in //DEPS.
        https://bazel.build/rules/lib/repo/git
    """
    git_repository(
        name = "brotli",
        commit = "6d03dfbedda1615c4cba1211f8d81735575209c8",
        remote = "https://skia.googlesource.com/external/github.com/google/brotli.git",
    )

    new_git_repository(
        name = "dawn",
        build_file = "@//bazel/external/dawn:BUILD.bazel",
        commit = "f16b9bbd6e2ad12507956852297528acd164774f",
        remote = "https://dawn.googlesource.com/dawn.git",
    )

    git_repository(
        name = "abseil_cpp",
        commit = "cb436cf0142b4cbe47aae94223443df7f82e2920",
        remote = "https://skia.googlesource.com/external/github.com/abseil/abseil-cpp.git",
    )

    new_git_repository(
        name = "dng_sdk",
        build_file = "@//bazel/external/dng_sdk:BUILD.bazel",
        commit = "c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
        remote = "https://android.googlesource.com/platform/external/dng_sdk.git",
    )

    new_git_repository(
        name = "expat",
        build_file = "@//bazel/external/expat:BUILD.bazel",
        commit = "441f98d02deafd9b090aea568282b28f66a50e36",
        remote = "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git",
    )

    new_git_repository(
        name = "freetype",
        build_file = "@//bazel/external/freetype:BUILD.bazel",
        commit = "82ce172669f132839debe6e50a3a53fe88db9e31",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git",
    )

    new_git_repository(
        name = "harfbuzz",
        build_file = "@//bazel/external/harfbuzz:BUILD.bazel",
        commit = "f1f2be776bcd994fa9262622e1a7098a066e5cf7",
        remote = "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git",
    )

    git_repository(
        name = "highway",
        commit = "424360251cdcfc314cfc528f53c872ecd63af0f0",
        remote = "https://chromium.googlesource.com/external/github.com/google/highway.git",
    )

    new_git_repository(
        name = "icu",
        build_file = "@//bazel/external/icu:BUILD.bazel",
        commit = "a0718d4f121727e30b8d52c7a189ebf5ab52421f",
        remote = "https://chromium.googlesource.com/chromium/deps/icu.git",
    )

    new_git_repository(
        name = "libavif",
        build_file = "@//bazel/external/libavif:BUILD.bazel",
        commit = "f49462dc93784bf34148715eee36ab6697ca0b35",
        remote = "https://github.com/AOMediaCodec/libavif.git",
    )

    new_git_repository(
        name = "libgav1",
        build_file = "@//bazel/external/libgav1:BUILD.bazel",
        commit = "0fb779c1e169fe6c229cd1fa9cc6ea6feeb441da",
        remote = "https://chromium.googlesource.com/codecs/libgav1.git",
    )

    new_git_repository(
        name = "libjpeg_turbo",
        build_file = "@//bazel/external/libjpeg_turbo:BUILD.bazel",
        commit = "ed683925e4897a84b3bffc5c1414c85b97a129a3",
        remote = "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git",
    )

    new_git_repository(
        name = "libjxl",
        build_file = "@//bazel/external/libjxl:BUILD.bazel",
        commit = "a205468bc5d3a353fb15dae2398a101dff52f2d3",
        remote = "https://chromium.googlesource.com/external/gitlab.com/wg1/jpeg-xl.git",
    )

    new_git_repository(
        name = "libpng",
        build_file = "@//bazel/external/libpng:BUILD.bazel",
        commit = "386707c6d19b974ca2e3db7f5c61873813c6fe44",
        remote = "https://skia.googlesource.com/third_party/libpng.git",
    )

    new_git_repository(
        name = "libwebp",
        build_file = "@//bazel/external/libwebp:BUILD.bazel",
        commit = "603e8d7adb0ccc35237419c2938194623b60e9be",
        remote = "https://chromium.googlesource.com/webm/libwebp.git",
    )

    new_git_repository(
        name = "libyuv",
        build_file = "@//bazel/external/libyuv:BUILD.bazel",
        commit = "d248929c059ff7629a85333699717d7a677d8d96",
        remote = "https://chromium.googlesource.com/libyuv/libyuv.git",
    )

    new_git_repository(
        name = "perfetto",
        build_file = "@//bazel/external/perfetto:BUILD.bazel",
        commit = "93885509be1c9240bc55fa515ceb34811e54a394",
        remote = "https://android.googlesource.com/platform/external/perfetto",
    )

    new_git_repository(
        name = "piex",
        build_file = "@//bazel/external/piex:BUILD.bazel",
        commit = "bb217acdca1cc0c16b704669dd6f91a1b509c406",
        remote = "https://android.googlesource.com/platform/external/piex.git",
    )

    new_git_repository(
        name = "vulkanmemoryallocator",
        build_file = "@//bazel/external/vulkanmemoryallocator:BUILD.bazel",
        commit = "7de5cc00de50e71a3aab22dea52fbb7ff4efceb6",
        remote = "https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator",
    )

    new_git_repository(
        name = "spirv_cross",
        build_file = "@//bazel/external/spirv_cross:BUILD.bazel",
        commit = "3550a54ae01b295c40ce972d951b420b388b9401",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross",
    )

    git_repository(
        name = "spirv_headers",
        commit = "aa331ab0ffcb3a67021caa1a0c1c9017712f2f31",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git",
    )

    git_repository(
        name = "spirv_tools",
        commit = "4183faa2eca6e531aecc7fb4a3f8a1b12c63a252",
        remote = "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git",
    )

    new_git_repository(
        name = "vulkan_headers",
        build_file = "@//bazel/external/vulkan_headers:BUILD.bazel",
        commit = "115820a6e5ff35881046a8e3920c2514c73e4a63",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers",
    )

    new_git_repository(
        name = "vulkan_tools",
        build_file = "@//bazel/external/vulkan_tools:BUILD.bazel",
        commit = "1a66f59f23ac2e45af4e38ee49c711df022a8f22",
        remote = "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools",
    )

    new_git_repository(
        name = "wuffs",
        build_file = "@//bazel/external/wuffs:BUILD.bazel",
        commit = "a0041ac0310b3156b963e2f2bea09245f25ec073",
        remote = "https://skia.googlesource.com/external/github.com/google/wuffs-mirror-release-c.git",
    )

    new_git_repository(
        name = "zlib_skia",
        build_file = "@//bazel/external/zlib_skia:BUILD.bazel",
        commit = "c876c8f87101c5a75f6014b0f832499afeb65b73",
        remote = "https://chromium.googlesource.com/chromium/src/third_party/zlib",
    )
