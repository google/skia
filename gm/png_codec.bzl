"""This module defines the png_codec_tests macro."""

load("//bazel:skia_rules.bzl", "skia_cc_test")

# These lists of lists are shaped as follows:
#
#     [images, decode_mode, dst_color_type, dst_alpha_type, surface_config]
#
# For each such list, we will define a test that decodes each image into an SkImage using
# SkPngDecoder. The decode mode, destination color type and alpha type are specified via the
# decode_mode, dst_color_type and dst_alpha_type fields, respectively. The resulting image is then
# drawn into an SkSurface specified via the surface_config field, and is saved as an undeclared
# test output which may be uploaded to Gold. See //tools/testrunners/gm/BazelGMTestRunner.cpp for
# more details.
#
# Some combinations of parameters are excluded because they are mutually incompatible or redundant.
_GRAYSCALE_8888_TESTS = [
    [
        "@skimage//:dm_pngs_gray8_opaque",
        decode_mode,
        dst_color_type,
        "premul",
        "8888",
    ]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
    for dst_color_type in ["force-grayscale", "force-nonnative-premul-color", "get-from-canvas"]
]
_GRAYSCALE_565_TESTS = [
    [
        "@skimage//:dm_pngs_gray8_opaque",
        decode_mode,
        "get-from-canvas",
        "premul",
        "565",
    ]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
]
_COLOR_TRANSLUCENT_TESTS = [
    [
        "@skimage//:dm_pngs_rgba8888_translucent",
        decode_mode,
        dst_color_type,
        dst_alpha_type,
        "8888",
    ]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
    for dst_color_type in ["force-nonnative-premul-color", "get-from-canvas"]
    for dst_alpha_type in ["premul", "unpremul"]
]
_COLOR_OPAQUE_8888_TESTS = [
    [
        "@skimage//:dm_pngs_rgba8888_opaque",
        decode_mode,
        dst_color_type,
        "premul",
        "8888",
    ]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
    for dst_color_type in ["force-nonnative-premul-color", "get-from-canvas"]
]
_COLOR_OPAQUE_565_TESTS = [
    [
        "@skimage//:dm_pngs_rgba8888_opaque",
        decode_mode,
        "get-from-canvas",
        "premul",
        "565",
    ]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
]
_TESTS = (
    _GRAYSCALE_8888_TESTS +
    _GRAYSCALE_565_TESTS +
    _COLOR_TRANSLUCENT_TESTS +
    _COLOR_OPAQUE_8888_TESTS +
    _COLOR_OPAQUE_565_TESTS
)

def png_codec_tests(name):
    """Generates various skia_cc_test targets for png_codec.cpp.

    Args:
        name: The name of the test_suite to generate.
    """

    all_tests = []

    for images, decode_mode, dst_color_type, dst_alpha_type, surface_config in _TESTS:
        test_name = "png_codec_%s_%s_%s_%s_%s_test" % (
            images.replace("@skimage//:dm_pngs_", "").replace("_", "-"),
            decode_mode,
            dst_color_type,
            dst_alpha_type,
            surface_config,
        )
        all_tests.append(test_name)

        skia_cc_test(
            name = test_name,
            size = "large",
            srcs = [
                "png_codec.cpp",
                "//tools/flags:common_flags",
                "//tools/testrunners/gm:BazelGMTestRunner.cpp",
            ],
            target_compatible_with = ["@platforms//os:linux"],
            args = [
                "--surfaceConfig",
                surface_config,
                "--pngCodecGMImages",
                "external/skimage/dm",
                "--pngCodecDecodeMode",
                decode_mode,
                "--pngCodecDstColorType",
                dst_color_type,
                "--pngCodecDstAlphaType",
                dst_alpha_type,
            ],
            data = [images],
            deps = [
                "//:core",
                "//:png_decode_codec",
                "//gm",
                "//src/core:core_priv",
                "//tools:codec_utils",
                "//tools:hash_and_encode",
                "//tools:tool_utils",
                "//tools/testrunners/common:testrunner",
                "//tools/testrunners/common/compilation_mode_keys",
                "//tools/testrunners/common/surface_manager:raster",
                "//tools/testrunners/gm/vias:simple_vias",
            ],
        )

    native.test_suite(
        name = name,
        tests = all_tests,
    )

    # List all generated target names for greppability.
    #
    # Editing this list does not by itself affect which targets are generated. Instead, edit the
    # list comprehensions at the top of this file, try to run a target with Bazel, and update this
    # list as instructed in the "out of sync" error message produced by the below fail() statement.
    greppable_test_list = [
        "png_codec_gray8-opaque_get-all-pixels_force-grayscale_premul_8888_test",
        "png_codec_gray8-opaque_get-all-pixels_force-nonnative-premul-color_premul_8888_test",
        "png_codec_gray8-opaque_get-all-pixels_get-from-canvas_premul_8888_test",
        "png_codec_gray8-opaque_incremental_force-grayscale_premul_8888_test",
        "png_codec_gray8-opaque_incremental_force-nonnative-premul-color_premul_8888_test",
        "png_codec_gray8-opaque_incremental_get-from-canvas_premul_8888_test",
        "png_codec_gray8-opaque_zero-init_force-grayscale_premul_8888_test",
        "png_codec_gray8-opaque_zero-init_force-nonnative-premul-color_premul_8888_test",
        "png_codec_gray8-opaque_zero-init_get-from-canvas_premul_8888_test",
        "png_codec_gray8-opaque_get-all-pixels_get-from-canvas_premul_565_test",
        "png_codec_gray8-opaque_incremental_get-from-canvas_premul_565_test",
        "png_codec_gray8-opaque_zero-init_get-from-canvas_premul_565_test",
        "png_codec_rgba8888-translucent_get-all-pixels_force-nonnative-premul-color_premul_8888_test",
        "png_codec_rgba8888-translucent_get-all-pixels_force-nonnative-premul-color_unpremul_8888_test",
        "png_codec_rgba8888-translucent_get-all-pixels_get-from-canvas_premul_8888_test",
        "png_codec_rgba8888-translucent_get-all-pixels_get-from-canvas_unpremul_8888_test",
        "png_codec_rgba8888-translucent_incremental_force-nonnative-premul-color_premul_8888_test",
        "png_codec_rgba8888-translucent_incremental_force-nonnative-premul-color_unpremul_8888_test",
        "png_codec_rgba8888-translucent_incremental_get-from-canvas_premul_8888_test",
        "png_codec_rgba8888-translucent_incremental_get-from-canvas_unpremul_8888_test",
        "png_codec_rgba8888-translucent_zero-init_force-nonnative-premul-color_premul_8888_test",
        "png_codec_rgba8888-translucent_zero-init_force-nonnative-premul-color_unpremul_8888_test",
        "png_codec_rgba8888-translucent_zero-init_get-from-canvas_premul_8888_test",
        "png_codec_rgba8888-translucent_zero-init_get-from-canvas_unpremul_8888_test",
        "png_codec_rgba8888-opaque_get-all-pixels_force-nonnative-premul-color_premul_8888_test",
        "png_codec_rgba8888-opaque_get-all-pixels_get-from-canvas_premul_8888_test",
        "png_codec_rgba8888-opaque_incremental_force-nonnative-premul-color_premul_8888_test",
        "png_codec_rgba8888-opaque_incremental_get-from-canvas_premul_8888_test",
        "png_codec_rgba8888-opaque_zero-init_force-nonnative-premul-color_premul_8888_test",
        "png_codec_rgba8888-opaque_zero-init_get-from-canvas_premul_8888_test",
        "png_codec_rgba8888-opaque_get-all-pixels_get-from-canvas_premul_565_test",
        "png_codec_rgba8888-opaque_incremental_get-from-canvas_premul_565_test",
        "png_codec_rgba8888-opaque_zero-init_get-from-canvas_premul_565_test",
    ]
    if greppable_test_list != all_tests:
        msg = [
            "Variable greppable_test_list is out of sync. Please update it as follows:",
            "",
            "    greppable_test_list = [",
        ] + [
            "        \"" + test + "\","
            for test in all_tests
        ] + [
            "    ]",
        ]
        fail("\n".join(msg))
