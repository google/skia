################################################################################
# Skylark macros
################################################################################

is_bazel = not hasattr(native, "genmpm")

def portable_select(select_dict, bazel_condition, default_condition):
    """Replaces select() with a Bazel-friendly wrapper.

    Args:
      select_dict: Dictionary in the same format as select().
    Returns:
      If Blaze platform, returns select() using select_dict.
      If Bazel platform, returns dependencies for condition
          bazel_condition, or empty list if none specified.
    """
    if is_bazel:
        return select_dict.get(bazel_condition, select_dict[default_condition])
    else:
        return select(select_dict)

def skia_select(conditions, results):
    """Replaces select() for conditions [UNIX, ANDROID, IOS]

    Args:
      conditions: [CONDITION_UNIX, CONDITION_ANDROID, CONDITION_IOS]
      results: [RESULT_UNIX, RESULT_ANDROID, RESULT_IOS]
    Returns:
      The result matching the platform condition.
    """
    if len(conditions) != 3 or len(results) != 3:
        fail("Must provide exactly 3 conditions and 3 results")

    selector = {}
    for i in range(3):
        selector[conditions[i]] = results[i]
    return portable_select(selector, conditions[2], conditions[0])

def skia_glob(srcs):
    """Replaces glob() with a version that accepts a struct.

    Args:
      srcs: struct(include=[], exclude=[])
    Returns:
      Equivalent of glob(srcs.include, exclude=srcs.exclude)
    """
    if hasattr(srcs, "include"):
        if hasattr(srcs, "exclude"):
            return native.glob(srcs.include, exclude = srcs.exclude)
        else:
            return native.glob(srcs.include)
    return []

################################################################################
## skia_{all,public}_hdrs()
################################################################################
def skia_all_hdrs():
    return native.glob([
        "src/**/*.h",
        "include/**/*.h",
        "third_party/**/*.h",
    ])

def skia_public_hdrs():
    return native.glob(
        ["include/**/*.h"],
        exclude = [
            "include/private/**/*",
        ],
    )

################################################################################
## skia_opts_srcs()
################################################################################
# Intel
SKIA_OPTS_SSE2 = "SSE2"

SKIA_OPTS_SSSE3 = "SSSE3"

SKIA_OPTS_SSE41 = "SSE41"

SKIA_OPTS_SSE42 = "SSE42"

SKIA_OPTS_AVX = "AVX"

SKIA_OPTS_HSW = "HSW"

# Arm
SKIA_OPTS_NEON = "NEON"

SKIA_OPTS_CRC32 = "CRC32"  # arm64

def opts_srcs(opts):
    if opts == SKIA_OPTS_SSE2:
        return native.glob([
            "src/opts/*_SSE2.cpp",
            "src/opts/*_sse2.cpp",  # No matches currently.
        ])
    elif opts == SKIA_OPTS_SSSE3:
        return native.glob([
            "src/opts/*_SSSE3.cpp",
            "src/opts/*_ssse3.cpp",
        ])
    elif opts == SKIA_OPTS_SSE41:
        return native.glob([
            "src/opts/*_sse41.cpp",
        ])
    elif opts == SKIA_OPTS_SSE42:
        return native.glob([
            "src/opts/*_sse42.cpp",
        ])
    elif opts == SKIA_OPTS_AVX:
        return native.glob([
            "src/opts/*_avx.cpp",
        ])
    elif opts == SKIA_OPTS_HSW:
        return native.glob([
            "src/opts/*_hsw.cpp",
        ])
    elif opts == SKIA_OPTS_NEON:
        return native.glob([
            "src/opts/*_neon.cpp",
        ])
    elif opts == SKIA_OPTS_CRC32:
        return native.glob([
            "src/opts/*_crc32.cpp",
        ])
    else:
        fail("skia_opts_srcs parameter 'opts' must be one of SKIA_OPTS_*.")

def opts_cflags(opts):
    if opts == SKIA_OPTS_SSE2:
        return ["-msse2"]
    elif opts == SKIA_OPTS_SSSE3:
        return ["-mssse3"]
    elif opts == SKIA_OPTS_SSE41:
        return ["-msse4.1"]
    elif opts == SKIA_OPTS_SSE42:
        return ["-msse4.2"]
    elif opts == SKIA_OPTS_AVX:
        return ["-mavx"]
    elif opts == SKIA_OPTS_HSW:
        return ["-mavx2", "-mf16c", "-mfma"]
    elif opts == SKIA_OPTS_NEON:
        return ["-mfpu=neon"]
    elif opts == SKIA_OPTS_CRC32:
        # NDK r11's Clang (3.8) doesn't pass along this -march setting correctly to an external
        # assembler, so we do it manually with -Wa.  This is just a bug, fixed in later Clangs.
        return ["-march=armv8-a+crc", "-Wa,-march=armv8-a+crc"]
    else:
        return []

SKIA_CPU_ARM = "ARM"

SKIA_CPU_ARM64 = "ARM64"

SKIA_CPU_X86 = "X86"

SKIA_CPU_OTHER = "OTHER"

def opts_rest_srcs(cpu):
    srcs = []
    if cpu == SKIA_CPU_ARM or cpu == SKIA_CPU_ARM64:
        srcs += native.glob([
            "src/opts/*_arm.cpp",
            "src/opts/SkBitmapProcState_opts_none.cpp",
        ])
        if cpu == SKIA_CPU_ARM64:
            # NEON doesn't need special flags to compile on ARM64.
            srcs += native.glob([
                "src/opts/*_neon.cpp",
            ])
    elif cpu == SKIA_CPU_X86:
        srcs += native.glob([
            "src/opts/*_x86.cpp",
        ])
    elif cpu == SKIA_CPU_OTHER:
        srcs += native.glob([
            "src/opts/*_none.cpp",
        ])
    else:
        fail("opts_rest_srcs parameter 'cpu' must be one of " +
             "SKIA_CPU_{ARM,ARM64,X86,OTHER}.")
    return srcs

def skia_opts_deps(cpu):
    res = [":opts_rest"]

    if cpu == SKIA_CPU_ARM:
        res += [":opts_neon"]

    if cpu == SKIA_CPU_ARM64:
        res += [":opts_crc32"]

    if cpu == SKIA_CPU_X86:
        res += [
            ":opts_sse2",
            ":opts_ssse3",
            ":opts_sse41",
            ":opts_sse42",
            ":opts_avx",
            ":opts_hsw",
        ]

    return res

################################################################################
## BASE_SRCS
################################################################################

# All platform-independent SRCS.
BASE_SRCS_ALL = struct(
    include = [
        "include/private/**/*.h",
        "src/**/*.h",
        "src/**/*.cpp",
        "src/**/*.inc",
    ],
    exclude = [
        # Exclude platform-dependent files.
        "src/codec/*",
        "src/device/xps/*",  # Windows-only. Move to ports?
        "src/doc/*_XPS.cpp",  # Windows-only. Move to ports?
        "src/gpu/gl/android/*",
        "src/gpu/gl/egl/*",
        "src/gpu/gl/glfw/*",
        "src/gpu/gl/glx/*",
        "src/gpu/gl/iOS/*",
        "src/gpu/gl/mac/*",
        "src/gpu/gl/win/*",
        "src/opts/**/*",
        "src/ports/**/*",
        "src/utils/android/**/*",
        "src/utils/mac/**/*",
        "src/utils/win/**/*",

        # Exclude multiple definitions.
        "src/core/SkPicture_none.cpp",
        "src/gpu/GrPathRendering_none.cpp",
        "src/gpu/ccpr/GrCoverageCountingPathRenderer_none.cpp",
        "src/gpu/gl/GrGLMakeNativeInterface_none.cpp",
        "src/pdf/SkDocument_PDF_None.cpp",  # We use src/pdf/SkPDFDocument.cpp.

        # Exclude files that don't compile everywhere.
        "src/svg/**/*",  # Depends on xml, SkJpegCodec, and SkPngCodec.
        "src/xml/**/*",  # Avoid dragging in expat when not needed.

        # Conflicting dependencies among Lua versions. See cl/107087297.
        "src/utils/SkLua*",

        # Currently exclude all vulkan specific files
        "src/gpu/vk/*",

        # Defines main.
        "src/sksl/SkSLMain.cpp",

        # Only used to regenerate the lexer
        "src/sksl/lex/*",

        # Atlas text
        "src/atlastext/*",
    ],
)

def codec_srcs(limited):
    """Sources for the codecs. Excludes Raw, and Ico, Webp, and Png if limited."""

    # TODO: Enable wuffs in Google3
    exclude = ["src/codec/SkWuffsCodec.cpp", "src/codec/*Raw*.cpp"]
    if limited:
        exclude += [
            "src/codec/*Ico*.cpp",
            "src/codec/*Webp*.cpp",
            "src/codec/*Png*",
        ]
    return native.glob(["src/codec/*.cpp", "third_party/gif/*.cpp"], exclude = exclude)

GL_SRCS_UNIX = struct(
    include = [
        "src/gpu/gl/GrGLMakeNativeInterface_none.cpp",
    ],
    exclude = [],
)
PORTS_SRCS_UNIX = struct(
    include = [
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
        "src/ports/*CG*",
        "src/ports/*WIC*",
        "src/ports/*android*",
        "src/ports/*chromium*",
        "src/ports/*mac*",
        "src/ports/*mozalloc*",
        "src/ports/*nacl*",
        "src/ports/*win*",
        "src/ports/SkFontMgr_custom_directory_factory.cpp",
        "src/ports/SkFontMgr_custom_embedded_factory.cpp",
        "src/ports/SkFontMgr_custom_empty_factory.cpp",
        "src/ports/SkFontMgr_empty_factory.cpp",
        "src/ports/SkFontMgr_fontconfig_factory.cpp",
        "src/ports/SkFontMgr_fuchsia.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

GL_SRCS_ANDROID = struct(
    include = [
        "src/gpu/gl/android/*.cpp",
    ],
    exclude = [],
)
PORTS_SRCS_ANDROID = struct(
    include = [
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
        "src/ports/*CG*",
        "src/ports/*FontConfig*",
        "src/ports/*WIC*",
        "src/ports/*chromium*",
        "src/ports/*fontconfig*",
        "src/ports/*mac*",
        "src/ports/*mozalloc*",
        "src/ports/*nacl*",
        "src/ports/*win*",
        "src/ports/SkDebug_stdio.cpp",
        "src/ports/SkFontMgr_custom_directory_factory.cpp",
        "src/ports/SkFontMgr_custom_embedded_factory.cpp",
        "src/ports/SkFontMgr_custom_empty_factory.cpp",
        "src/ports/SkFontMgr_empty_factory.cpp",
        "src/ports/SkFontMgr_fuchsia.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

GL_SRCS_IOS = struct(
    include = [
        "src/gpu/gl/iOS/GrGLMakeNativeInterface_iOS.cpp",
    ],
    exclude = [],
)
PORTS_SRCS_IOS = struct(
    include = [
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
        "src/utils/mac/*.cpp",
    ],
    exclude = [
        "src/ports/*FontConfig*",
        "src/ports/*FreeType*",
        "src/ports/*WIC*",
        "src/ports/*android*",
        "src/ports/*chromium*",
        "src/ports/*fontconfig*",
        "src/ports/*mozalloc*",
        "src/ports/*nacl*",
        "src/ports/*win*",
        "src/ports/SkFontMgr_custom.cpp",
        "src/ports/SkFontMgr_custom_directory.cpp",
        "src/ports/SkFontMgr_custom_embedded.cpp",
        "src/ports/SkFontMgr_custom_empty.cpp",
        "src/ports/SkFontMgr_custom_directory_factory.cpp",
        "src/ports/SkFontMgr_custom_embedded_factory.cpp",
        "src/ports/SkFontMgr_custom_empty_factory.cpp",
        "src/ports/SkFontMgr_empty_factory.cpp",
        "src/ports/SkFontMgr_fuchsia.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

def base_srcs():
    return skia_glob(BASE_SRCS_ALL)

def ports_srcs(os_conditions):
    return skia_select(
        os_conditions,
        [
            skia_glob(PORTS_SRCS_UNIX),
            skia_glob(PORTS_SRCS_ANDROID),
            skia_glob(PORTS_SRCS_IOS),
        ],
    )

def gl_srcs(os_conditions):
    return skia_select(
        os_conditions,
        [
            skia_glob(GL_SRCS_UNIX),
            skia_glob(GL_SRCS_ANDROID),
            skia_glob(GL_SRCS_IOS),
        ],
    )

def skia_srcs(os_conditions):
    return base_srcs() + ports_srcs(os_conditions) + gl_srcs(os_conditions)

################################################################################
## INCLUDES
################################################################################

# Includes needed by Skia implementation.  Not public includes.
INCLUDES = [
    ".",
    "include/android",
    "include/c",
    "include/codec",
    "include/config",
    "include/core",
    "include/docs",
    "include/effects",
    "include/encode",
    "include/gpu",
    "include/pathops",
    "include/ports",
    "include/private",
    "include/third_party/skcms",
    "include/utils",
    "include/utils/mac",
    "src/codec",
    "src/core",
    "src/gpu",
    "src/image",
    "src/images",
    "src/lazy",
    "src/opts",
    "src/pdf",
    "src/ports",
    "src/sfnt",
    "src/shaders",
    "src/shaders/gradients",
    "src/sksl",
    "src/utils",
    "third_party/gif",
]

################################################################################
## DM_SRCS
################################################################################

DM_SRCS_ALL = struct(
    include = [
        "dm/*.cpp",
        "dm/*.h",
        "experimental/pipe/*.cpp",
        "experimental/pipe/*.h",
        "experimental/svg/model/*.cpp",
        "experimental/svg/model/*.h",
        "gm/*.cpp",
        "gm/*.h",
        "src/utils/SkMultiPictureDocument.cpp",
        "src/xml/*.cpp",
        "tests/*.cpp",
        "tests/*.h",
        "tools/ios_utils.h",
        "tools/AutoreleasePool.h",
        "tools/BinaryAsset.h",
        "tools/BigPathBench.inc",
        "tools/CrashHandler.cpp",
        "tools/CrashHandler.h",
        "tools/DDLPromiseImageHelper.cpp",
        "tools/DDLPromiseImageHelper.h",
        "tools/DDLTileHelper.cpp",
        "tools/DDLTileHelper.h",
        "tools/HashAndEncode.cpp",
        "tools/HashAndEncode.h",
        "tools/ProcStats.cpp",
        "tools/ProcStats.h",
        "tools/Registry.h",
        "tools/ResourceFactory.h",
        "tools/Resources.cpp",
        "tools/Resources.h",
        "tools/SkVMBuilders.cpp",
        "tools/SkVMBuilders.h",
        "tools/UrlDataManager.cpp",
        "tools/UrlDataManager.h",
        "tools/debugger/*.cpp",
        "tools/debugger/*.h",
        "tools/flags/*.cpp",
        "tools/flags/*.h",
        "tools/fonts/RandomScalerContext.cpp",
        "tools/fonts/RandomScalerContext.h",
        "tools/fonts/TestFontMgr.cpp",
        "tools/fonts/TestFontMgr.h",
        "tools/fonts/TestSVGTypeface.cpp",
        "tools/fonts/TestSVGTypeface.h",
        "tools/fonts/TestTypeface.cpp",
        "tools/fonts/TestTypeface.h",
        "tools/fonts/ToolUtilsFont.cpp",
        "tools/fonts/test_font_monospace.inc",
        "tools/fonts/test_font_sans_serif.inc",
        "tools/fonts/test_font_serif.inc",
        "tools/fonts/test_font_index.inc",
        "tools/gpu/**/*.cpp",
        "tools/gpu/**/*.h",
        "tools/random_parse_path.cpp",
        "tools/random_parse_path.h",
        "tools/SkSharingProc.cpp",
        "tools/ToolUtils.cpp",
        "tools/ToolUtils.h",
        "tools/timer/*.cpp",
        "tools/timer/*.h",
        "tools/trace/*.cpp",
        "tools/trace/*.h",
    ],
    exclude = [
        "gm/cgms.cpp",
        "gm/video_decoder.cpp",
        "tests/FontMgrAndroidParserTest.cpp",  # Android-only.
        "tests/FontMgrFontConfigTest.cpp",  # FontConfig-only.
        "tests/skia_test.cpp",  # Old main.
        "tools/gpu/atlastext/*",
        "tools/gpu/gl/angle/*",
        "tools/gpu/gl/egl/*",
        "tools/gpu/gl/glx/*",
        "tools/gpu/gl/iOS/*",
        "tools/gpu/gl/mac/*",
        "tools/gpu/gl/win/*",
        "tools/timer/SysTimer_mach.cpp",
        "tools/timer/SysTimer_windows.cpp",
    ],
)

################################################################################
## dm_srcs()
################################################################################

def dm_srcs(os_conditions):
    """Sources for the dm binary for the specified os."""
    return skia_glob(DM_SRCS_ALL) + skia_select(
        os_conditions,
        [
            ["tests/FontMgrFontConfigTest.cpp"],
            ["tests/FontMgrAndroidParserTest.cpp"],
            [],
        ],
    )

################################################################################
## DM_ARGS
################################################################################

def DM_ARGS(asan):
    source = ["gm", "image", "lottie"]

    # TODO(benjaminwagner): f16, pic-8888, serialize-8888, and tiles_rt-8888 fail.
    config = ["565", "8888", "pdf"]
    match = ["~Codec_78329453"]
    return (["--src"] + source + ["--config"] + config + ["--nonativeFonts"] +
            ["--match"] + match)

################################################################################
## COPTS
################################################################################

def base_copts(os_conditions):
    return skia_select(
        os_conditions,
        [
            # UNIX
            [
                "-Wno-implicit-fallthrough",  # Some intentional fallthrough.
                # Internal use of deprecated methods. :(
                "-Wno-deprecated-declarations",
                # TODO(kjlubick)
                "-Wno-self-assign",  # Spurious warning in tests/PathOpsDVectorTest.cpp?
            ],
            # ANDROID
            [
                "-Wno-implicit-fallthrough",  # Some intentional fallthrough.
                # 'GrResourceCache' declared with greater visibility than the
                # type of its field 'GrResourceCache::fPurgeableQueue'... bogus.
                "-Wno-error=attributes",
            ],
            # IOS
            [
                "-Wno-implicit-fallthrough",  # Some intentional fallthrough.
            ],
        ],
    )

################################################################################
## DEFINES
################################################################################

def base_defines(os_conditions):
    return [
        # Chrome DEFINES.
        "SK_USE_FREETYPE_EMBOLDEN",
        # Turn on a few Google3-specific build fixes.
        "SK_BUILD_FOR_GOOGLE3",
        # Required for building dm.
        "GR_TEST_UTILS",
        # Google3 probably doesn't want this feature yet
        "SK_DISABLE_REDUCE_OPLIST_SPLITTING",
        # Staging flags for API changes
        # Should remove after we update golden images
        "SK_WEBP_ENCODER_USE_DEFAULT_METHOD",
        # Experiment to diagnose image diffs in Google3
        "SK_DISABLE_LOWP_RASTER_PIPELINE",
        # JPEG is in codec_limited
        "SK_HAS_JPEG_LIBRARY",
    ] + skia_select(
        os_conditions,
        [
            # UNIX
            [
                "PNG_SKIP_SETJMP_CHECK",
                "SK_BUILD_FOR_UNIX",
                "SK_R32_SHIFT=16",
                "SK_PDF_USE_SFNTLY",
                "SK_HAS_PNG_LIBRARY",
                "SK_HAS_WEBP_LIBRARY",
            ],
            # ANDROID
            [
                "SK_BUILD_FOR_ANDROID",
                "SK_HAS_PNG_LIBRARY",
                "SK_HAS_WEBP_LIBRARY",
            ],
            # IOS
            [
                "SK_BUILD_FOR_IOS",
                "SK_BUILD_NO_OPTS",
                "SKNX_NO_SIMD",
            ],
        ],
    )

################################################################################
## LINKOPTS
################################################################################

def base_linkopts(os_conditions):
    return [
        "-ldl",
    ] + skia_select(
        os_conditions,
        [
            # UNIX
            [],
            # ANDROID
            [
                "-lEGL",
                "-lGLESv2",
            ],
            # IOS
            [
                "-framework CoreFoundation",
                "-framework CoreGraphics",
                "-framework CoreText",
                "-framework ImageIO",
                "-framework MobileCoreServices",
            ],
        ],
    )

################################################################################
## sksg_lib
################################################################################

def sksg_lib_hdrs():
    return native.glob(["modules/sksg/include/*.h"])

def sksg_lib_srcs():
    return native.glob(["modules/sksg/src/*.cpp"])

################################################################################
## skparagraph_lib
################################################################################

def skparagraph_lib_hdrs():
    return native.glob(["modules/skparagraph/include/*.h"])

def skparagraph_lib_srcs():
    return native.glob(["modules/skparagraph/src/*.cpp"])

################################################################################
## experimental xform
################################################################################

def exp_xform_lib_hdrs():
    return native.glob(["experimental/xform/*.h"])

def exp_xform_lib_srcs():
    return native.glob(["experimental/xform/*.cpp"])

################################################################################
## skottie_lib
################################################################################

def skottie_lib_hdrs():
    return native.glob(["modules/skottie/include/*.h"])

def skottie_lib_srcs():
    return native.glob(
        [
            "modules/skottie/src/*.cpp",
            "modules/skottie/src/*.h",
            "modules/skottie/src/effects/*.cpp",
            "modules/skottie/src/effects/*.h",
            "modules/skottie/src/text/*.cpp",
            "modules/skottie/src/text/*.h",
        ],
        exclude = [
            "modules/skottie/src/SkottieTest.cpp",
            "modules/skottie/src/SkottieTool.cpp",
        ],
    )

################################################################################
## skottie_shaper
################################################################################

SKOTTIE_SHAPER_HDRS = [
    "modules/skottie/src/text/SkottieShaper.h",
]

SKOTTIE_SHAPER_SRCS = [
    "modules/skottie/src/text/SkottieShaper.cpp",
]

################################################################################
## skottie_tool
################################################################################

SKOTTIE_TOOL_SRCS = [
    "modules/skottie/src/SkottieTool.cpp",
    "modules/skottie/utils/SkottieUtils.cpp",
    "modules/skottie/utils/SkottieUtils.h",
    # TODO(benjaminwagner): Add "flags" target.
    "tools/flags/CommandLineFlags.cpp",
    "tools/flags/CommandLineFlags.h",
]

################################################################################
## SkShaper
################################################################################

SKSHAPER_HARFBUZZ_SRCS = [
    "modules/skshaper/include/SkShaper.h",
    "modules/skshaper/src/SkShaper.cpp",
    "modules/skshaper/src/SkShaper_harfbuzz.cpp",
    "modules/skshaper/src/SkShaper_primitive.cpp",
]

SKSHAPER_PRIMITIVE_SRCS = [
    "modules/skshaper/include/SkShaper.h",
    "modules/skshaper/src/SkShaper.cpp",
    "modules/skshaper/src/SkShaper_primitive.cpp",
]
