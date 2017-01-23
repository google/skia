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
  if hasattr(srcs, 'include'):
    if hasattr(srcs, 'exclude'):
      return native.glob(srcs.include, exclude=srcs.exclude)
    else:
      return native.glob(srcs.include)
  return []

################################################################################
## PRIVATE_HDRS
################################################################################

PRIVATE_HDRS_INCLUDE_LIST = [
    "include/private/**/*.h",
    "src/**/*.inc",
]

PRIVATE_HDRS = struct(
    include = PRIVATE_HDRS_INCLUDE_LIST,
)

ALL_HDRS = struct(
    include = [
        "src/**/*.h",
        "include/**/*.h",
    ],
)

################################################################################
## BASE_SRCS
################################################################################

# All platform-independent SRCS.
BASE_SRCS_ALL = struct(
    include = [
        "src/**/*.h",
        "src/**/*.cpp",

        # Third Party
        "third_party/etc1/*.cpp",
        "third_party/etc1/*.h",
        "third_party/gif/*.cpp",
        "third_party/gif/*.h",
        "third_party/ktx/*.cpp",
        "third_party/ktx/*.h",
    ],
    # Note: PRIVATE_HDRS_INCLUDE_LIST is excluded from BASE_SRCS_ALL here
    # because they are required to appear in srcs for some rules but hdrs for
    # other rules. See internal cl/119566959.
    exclude = PRIVATE_HDRS_INCLUDE_LIST + [
        # Exclude platform-dependent files.
        "src/android/*",
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
        "src/images/*",
        "src/opts/**/*",
        "src/ports/**/*",
        "src/splicer/*",
        "src/utils/android/**/*",
        "src/utils/mac/**/*",
        "src/utils/SkThreadUtils_win.cpp",  # Windows-only. Move to ports?
        "src/utils/win/**/*",
        "src/views/sdl/*",
        "src/views/win/*",
        "src/views/unix/*",

        # Exclude multiple definitions.
        # TODO(mtklein): Move to opts?
        "src/pdf/SkDocument_PDF_None.cpp",  # We use src/pdf/SkPDFDocument.cpp.
        "src/gpu/gl/GrGLCreateNativeInterface_none.cpp",
        "src/gpu/gl/GrGLDefaultInterface_native.cpp",
        "src/gpu/gl/GrGLDefaultInterface_none.cpp",

        # Exclude files that don't compile with the current DEFINES.
        "src/gpu/gl/mesa/*",  # Requires SK_MESA define.
        "src/svg/**/*",  # Depends on XML.
        "src/xml/**/*",

        # Conflicting dependencies among Lua versions. See cl/107087297.
        "src/utils/SkLua*",

        # Not used.
        "src/views/**/*",

        # Currently exclude all vulkan specific files
        "src/gpu/vk/*",

        # Defines main.
        "src/sksl/SkSLMain.cpp",
    ],
)

# Platform-dependent SRCS for google3-default platform.
BASE_SRCS_UNIX = struct(
    include = [
        "src/android/*",
        "src/codec/*",
        "src/gpu/gl/GrGLDefaultInterface_none.cpp",
        "src/images/*",
        "src/opts/**/*.cpp",
        "src/opts/**/*.h",
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
        "src/opts/*arm*",
        "src/opts/*mips*",
        "src/opts/*NEON*",
        "src/opts/*neon*",
        # Included in :opts_ssse3 library.
        "src/opts/*SSSE3*",
        "src/opts/*ssse3*",
        # Included in :opts_sse4 library.
        "src/opts/*SSE4*",
        "src/opts/*sse4*",
        # Included in :opts_avx or :opts_hsw
        "src/opts/*avx*",
        "src/opts/*hsw*",
        "src/opts/SkBitmapProcState_opts_none.cpp",
        "src/opts/SkBlitMask_opts_none.cpp",
        "src/opts/SkBlitRow_opts_none.cpp",
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
        "src/ports/SkFontMgr_fontconfig.cpp",
        "src/ports/SkFontMgr_fontconfig_factory.cpp",
        "src/ports/SkImageEncoder_none.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

# Platform-dependent SRCS for google3-default Android.
BASE_SRCS_ANDROID = struct(
    include = [
        "src/android/*",
        "src/codec/*",
        "src/gpu/gl/GrGLDefaultInterface_none.cpp",
        "src/images/*",
        # TODO(benjaminwagner): Figure out how to compile with EGL.
        "src/opts/**/*.cpp",
        "src/opts/**/*.h",
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
        "src/opts/*mips*",
        "src/opts/*SSE2*",
        "src/opts/*SSSE3*",
        "src/opts/*ssse3*",
        "src/opts/*SSE4*",
        "src/opts/*sse4*",
        "src/opts/*avx*",
        "src/opts/*x86*",
        "src/opts/SkBlitMask_opts_none.cpp",
        "src/opts/SkBlitRow_opts_none.cpp",
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
        "src/ports/SkImageEncoder_none.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

# Platform-dependent SRCS for google3-default iOS.
BASE_SRCS_IOS = struct(
    include = [
        "src/android/*",
        "src/codec/*",
        "src/gpu/gl/GrGLDefaultInterface_native.cpp",
        "src/gpu/gl/iOS/GrGLCreateNativeInterface_iOS.cpp",
        "src/opts/**/*.cpp",
        "src/opts/**/*.h",
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
        "src/utils/mac/*.cpp",
    ],
    exclude = [
        "src/codec/*Ico*.cpp",
        "src/codec/*Jpeg*.cpp",
        "src/codec/*Webp*.cpp",
        "src/codec/*Png*",
        "src/codec/*Raw*.cpp",
        "src/opts/*mips*",
        "src/opts/*NEON*",
        "src/opts/*neon*",
        "src/opts/*SSE2*",
        "src/opts/*SSSE3*",
        "src/opts/*ssse3*",
        "src/opts/*SSE4*",
        "src/opts/*sse4*",
        "src/opts/*avx*",
        "src/opts/*x86*",
        "src/opts/SkBlitMask_opts_arm*.cpp",
        "src/opts/SkBlitRow_opts_arm*.cpp",
        "src/ports/*CG*",
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
        "src/ports/SkFontMgr_custom_directory_factory.cpp",
        "src/ports/SkFontMgr_custom_embedded_factory.cpp",
        "src/ports/SkFontMgr_custom_empty_factory.cpp",
        "src/ports/SkFontMgr_empty_factory.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

################################################################################
## SSSE3/SSE4/AVX/HSW SRCS
################################################################################

SSSE3_SRCS = struct(
    include = [
        "src/opts/*SSSE3*.cpp",
        "src/opts/*ssse3*.cpp",
    ],
)

SSE4_SRCS = struct(
    include = [
        "src/opts/*SSE4*.cpp",
        "src/opts/*sse4*.cpp",
    ],
)

AVX_SRCS = struct(
    include = [
        "src/opts/*_avx.cpp",
    ],
)

HSW_SRCS = struct(
    include = [
        "src/opts/*_hsw.cpp",
    ],
)

################################################################################
## BASE_HDRS
################################################################################

BASE_HDRS = struct(
    include = [
        "include/**/*.h",
    ],
    exclude = PRIVATE_HDRS_INCLUDE_LIST + [
        # Not used.
        "include/views/**/*",
    ],
)

################################################################################
## BASE_DEPS
################################################################################

BASE_DEPS_ALL = []

BASE_DEPS_UNIX = [
    ":opts_ssse3",
    ":opts_sse4",
    ":opts_avx",
    ":opts_hsw",
]

BASE_DEPS_ANDROID = []

BASE_DEPS_IOS = []

################################################################################
## INCLUDES
################################################################################

# Includes needed by Skia implementation.  Not public includes.
INCLUDES = [
    "include/android",
    "include/c",
    "include/client/android",
    "include/codec",
    "include/config",
    "include/core",
    "include/effects",
    "include/gpu",
    "include/images",
    "include/pathops",
    "include/pipe",
    "include/ports",
    "include/private",
    "include/utils",
    "include/utils/mac",
    "include/utils/win",
    "include/svg",
    "include/xml",
    "src/codec",
    "src/core",
    "src/gpu",
    "src/image",
    "src/lazy",
    "src/opts",
    "src/ports",
    "src/pdf",
    "src/sfnt",
    "src/sksl",
    "src/utils",
    "third_party/etc1",
    "third_party/gif",
    "third_party/ktx",
]

################################################################################
## DM_SRCS
################################################################################

DM_SRCS_ALL = struct(
    include = [
        "dm/*.cpp",
        "dm/*.h",
        "gm/*.c",
        "gm/*.cpp",
        "gm/*.h",
        "tests/*.cpp",
        "tests/*.h",
        "tools/BigPathBench.inc",
        "tools/CrashHandler.cpp",
        "tools/CrashHandler.h",
        "tools/ProcStats.cpp",
        "tools/ProcStats.h",
        "tools/Resources.cpp",
        "tools/Resources.h",
        "tools/SkJSONCPP.h",
        "tools/UrlDataManager.cpp",
        "tools/UrlDataManager.h",
        "tools/debugger/*.cpp",
        "tools/debugger/*.h",
        "tools/flags/*.cpp",
        "tools/flags/*.h",
        "tools/gpu/**/*.cpp",
        "tools/gpu/**/*.h",
        "tools/picture_utils.cpp",
        "tools/picture_utils.h",
        "tools/random_parse_path.cpp",
        "tools/random_parse_path.h",
        "tools/sk_tool_utils.cpp",
        "tools/sk_tool_utils.h",
        "tools/sk_tool_utils_flags.h",
        "tools/sk_tool_utils_font.cpp",
        "tools/test_font_monospace.inc",
        "tools/test_font_sans_serif.inc",
        "tools/test_font_serif.inc",
        "tools/test_font_index.inc",
        "tools/timer/*.cpp",
        "tools/timer/*.h",
    ],
    exclude = [
        "tests/FontMgrAndroidParserTest.cpp",  # Android-only.
        "tests/skia_test.cpp",  # Old main.
        "tests/SkpSkGrTest.cpp",  # Alternate main.
        "tests/SVGDeviceTest.cpp",
        "tools/gpu/gl/angle/*",
        "tools/gpu/gl/egl/*",
        "tools/gpu/gl/glx/*",
        "tools/gpu/gl/iOS/*",
        "tools/gpu/gl/mac/*",
        "tools/gpu/gl/mesa/*",
        "tools/gpu/gl/win/*",
        "tools/timer/SysTimer_mach.cpp",
        "tools/timer/SysTimer_windows.cpp",
    ],
)

DM_SRCS_UNIX = struct(
    include = [
        "tools/gpu/gl/CreatePlatformGLContext_none.cpp",
    ],
)

DM_SRCS_ANDROID = struct(
    include = [
        "tests/FontMgrAndroidParserTest.cpp",
        # TODO(benjaminwagner): Figure out how to compile with EGL.
        "tools/gpu/gl/CreatePlatformGLContext_none.cpp",
    ],
)

DM_SRCS_IOS = struct(
    include = [
        "tools/gpu/iOS/CreatePlatformGLContext_iOS.cpp",
    ],
)

################################################################################
## DM_INCLUDES
################################################################################

DM_INCLUDES = [
    "dm",
    "gm",
    "src/codec",
    "src/effects",
    "src/effects/gradients",
    "src/fonts",
    "src/images",
    "src/pathops",
    "src/pipe/utils",
    "src/ports",
    "tests",
    "tools",
    "tools/debugger",
    "tools/flags",
    "tools/gpu",
    "tools/timer",
]

################################################################################
## DM_ARGS
################################################################################

def DM_ARGS(asan):
  source = ["tests", "gm", "image"]
  # TODO(benjaminwagner): f16 and serialize-8888 fail.
  config = ["565", "8888", "pdf", "srgb", "tiles_rt", "pic"]
  # TODO(mtklein): maybe investigate why these fail?
  match = [
      "~Canvas",
      "~Codec",
      "~Codec_Dimensions",
      "~Codec_stripes",
      "~FontMgr",
      "~PaintBreakText",
      "~RecordDraw_TextBounds",
      "~Scalar",
      "~skps",
      "~Stream",
  ]
  if asan:
    # The ASAN we use with Bazel has some strict checks, so omit tests that
    # trigger them.
    match += [
        "~clippedcubic2",
        "~conicpaths",
        "~^gradients",
        "~Math",
        "~Matrix",
        "~PathOpsCubic",
        "~PathOpsFailOp",
        "~PathOpsOpCubicsThreaded",
        "~PathOpsOpLoopsThreaded",
        "~PathOpsSimplify",
        "~PathOpsTightBoundsQuads",
        "~Point",
        "~sk_linear_to_srgb",
        "~small_color_stop",
    ]
  return ["--src"] + source + ["--config"] + config + ["--match"] + match

################################################################################
## COPTS
################################################################################

COPTS_UNIX = [
    "-Wno-implicit-fallthrough",  # Some intentional fallthrough.
    "-Wno-deprecated-declarations",  # Internal use of deprecated methods. :(
]

COPTS_ANDROID = ["-mfpu=neon"]

COPTS_IOS = []

COPTS_ALL = []

################################################################################
## DEFINES
################################################################################

DEFINES_UNIX = [
    "PNG_SKIP_SETJMP_CHECK",
    "SK_BUILD_FOR_UNIX",
    "SK_SAMPLES_FOR_X",
    "SK_PDF_USE_SFNTLY",
    "SK_CODEC_DECODES_RAW",
    "SK_HAS_JPEG_LIBRARY",
    "SK_HAS_PNG_LIBRARY",
    "SK_HAS_WEBP_LIBRARY",
]

DEFINES_ANDROID = [
    "SK_BUILD_FOR_ANDROID",
    "SK_CODEC_DECODES_RAW",
    "SK_HAS_JPEG_LIBRARY",
    "SK_HAS_PNG_LIBRARY",
    "SK_HAS_WEBP_LIBRARY",
]

DEFINES_IOS = [
    "SK_BUILD_FOR_IOS",
    "SK_BUILD_NO_OPTS",
    "SK_IGNORE_ETC1_SUPPORT",
    "SKNX_NO_SIMD",
]

DEFINES_ALL = [
    # Chrome DEFINES.
    "SK_USE_FLOATBITS",
    "SK_USE_FREETYPE_EMBOLDEN",
    # Turn on a few Google3-specific build fixes.
    "GOOGLE3",
    # Staging flags for API changes
    # Temporarily Disable analytic AA for Google3
    "SK_NO_ANALYTIC_AA",
    "SK_SUPPORT_LEGACY_BITMAP_SETPIXELREF",
    "SK_SUPPORT_LEGACY_CLIPOP_EXOTIC_NAMES",
    "SK_SUPPORT_LEGACY_GETCLIPBOUNDS",
]

################################################################################
## LINKOPTS
################################################################################

LINKOPTS_UNIX = []

LINKOPTS_ANDROID = [
    "-lEGL",
]

LINKOPTS_IOS = []

LINKOPTS_ALL = [
    "-ldl",
]
