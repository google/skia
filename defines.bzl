"""
This file contains defines used by all builds of Skia.
"""

load("//bazel:extra_defines.bzl", "EXTRA_DEFINES")
load("//bazel:skia_rules.bzl", "select_multi")

GENERAL_DEFINES = [
    "SK_GAMMA_APPLY_TO_A8",
] + select({
    "//bazel/common_config_settings:debug_build": [
        "SK_DEBUG",
    ],
    "//bazel/common_config_settings:release_build": [
        "SK_RELEASE",
        "NDEBUG",
    ],
    "//bazel/common_config_settings:fast_build": [
        "SK_DEBUG",
    ],
}) + select({
    "//bazel/common_config_settings:enable_tracing_false": ["SK_DISABLE_TRACING"],
    "//conditions:default": [],
}) + select({
    "//bazel/common_config_settings:enable_effect_serialization_false": ["SK_DISABLE_EFFECT_DESERIALIZATION"],
    "//conditions:default": [],
}) + select({
    "//src/gpu:enable_gpu_test_utils_true": [
        "GR_TEST_UTILS=1",
        "SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1",
    ],
    "//conditions:default": [],
}) + select({
    "//src/sksl:enable_skslc_true": [
        "SKSL_STANDALONE",
        "SK_DISABLE_TRACING",
        "SK_ENABLE_SPIRV_CROSS",
        "SK_ENABLE_SPIRV_VALIDATION",
        "SK_ENABLE_WGSL_VALIDATION",
    ],
    "//conditions:default": [],
}) + select({
    "//src/pdf:enable_pdf_backend_true": ["SK_SUPPORT_PDF"],
    "//conditions:default": [],
}) + select({
    "//src/shaders:legacy_shader_context_true": [],  # This is the default in SkTypes.h
    "//src/shaders:legacy_shader_context_false": ["SK_DISABLE_LEGACY_SHADERCONTEXT"],
}) + select({
    "//src/lazy:enable_discardable_memory_true": ["SK_USE_DISCARDABLE_SCALEDIMAGECACHE"],
    "//src/lazy:enable_discardable_memory_false": [],
}) + select({
    "//bazel/common_config_settings:build_for_debugger_true": ["SK_BUILD_FOR_DEBUGGER"],
    "//conditions:default": [],
})

GPU_DEFINES = select_multi({
    "//src/gpu:gl_ganesh": [
        "SK_GL",
        "SK_GANESH",
    ],
    "//src/gpu:vulkan_ganesh": [
        "SK_VULKAN",
        "SK_GANESH",
    ],
    "//src/gpu:metal_ganesh": [
        "SK_METAL",
        "SK_GANESH",
    ],
}) + select({
    "//src/gpu:gl_standard": [
        "SK_ASSUME_GL=1",
    ],
    "//src/gpu:gles_standard": [
        "SK_ASSUME_GL_ES=1",
    ],
    "//src/gpu:webgl_standard": [
        "SK_ASSUME_WEBGL=1",
    ],
    "//conditions:default": [],
}) + select({
    "//src/gpu:vulkan_with_vma": [
        "SK_USE_VMA",
    ],
    "//conditions:default": [],
})

CODEC_DEFINES = select_multi({
    "//src/codec:avif_decode_codec": ["SK_CODEC_DECODES_AVIF"],
    "//src/codec:gif_decode_codec": ["SK_HAS_WUFFS_LIBRARY", "SK_CODEC_DECODES_GIF"],
    "//src/codec:jpeg_decode_codec": ["SK_CODEC_DECODES_JPEG"],
    "//src/codec:png_decode_codec": ["SK_CODEC_DECODES_PNG"],
    "//src/codec:raw_decode_codec": [
        "SK_CODEC_DECODES_RAW",
        "SK_CODEC_DECODES_JPEG",
    ],
    "//src/codec:webp_decode_codec": ["SK_CODEC_DECODES_WEBP"],
})

TYPEFACE_DEFINES = select_multi(
    {
        "//src/ports:uses_freetype": ["SK_TYPEFACE_FACTORY_FREETYPE"],
        "//src/ports:uses_fontations": ["SK_TYPEFACE_FACTORY_FONTATIONS"],
        #TODO: others when they become available
    },
)

PLATFORM_DEFINES = select({
    "//bazel/common_config_settings:cpu_wasm": [
        # working around https://github.com/emscripten-core/emscripten/issues/10072
        "SK_FORCE_8_BYTE_ALIGNMENT",
    ],
    "//conditions:default": [],
}) + select({
    "//bazel/platform:trivial_abi": ["SK_TRIVIAL_ABI=[[clang::trivial_abi]]"],
    "//bazel/common_config_settings:cpu_wasm": ["SK_TRIVIAL_ABI=[[clang::trivial_abi]]"],
    "//conditions:default": [],
})

# Skia's public headers can work with any version of a Vulkan header. When compiling Skia internals,
# we want to use the Vulkan headers in //include/third_party/vulkan, so we set this define only when
# building the library but not exporting it for clients, so they can use whatever vulkan they want.
GENERAL_LOCAL_DEFINES = ["SKIA_IMPLEMENTATION=1"]

DEFAULT_DEFINES = GENERAL_DEFINES + GPU_DEFINES + CODEC_DEFINES + TYPEFACE_DEFINES + PLATFORM_DEFINES + EXTRA_DEFINES

DEFAULT_LOCAL_DEFINES = GENERAL_LOCAL_DEFINES
