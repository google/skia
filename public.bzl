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
## BASE_SRCS
################################################################################

# All platform-independent SRCS.
BASE_SRCS_ALL = struct(
    include = [
        "include/private/*.h",
        "src/**/*.h",
        "src/**/*.cpp",

        # Third Party
        "third_party/etc1/*.cpp",
        "third_party/etc1/*.h",
        "third_party/ktx/*.cpp",
        "third_party/ktx/*.h",
    ],
    exclude = [
        # Exclude platform-dependent files.
        "src/codec/*",
        "src/device/xps/*",  # Windows-only. Move to ports?
        "src/doc/*_XPS.cpp",  # Windows-only. Move to ports?
        "src/fonts/SkFontMgr_fontconfig.cpp",
        "src/gpu/gl/android/*",
        "src/gpu/gl/egl/*",
        "src/gpu/gl/glx/*",
        "src/gpu/gl/iOS/*",
        "src/gpu/gl/mac/*",
        "src/gpu/gl/win/*",
        "src/images/*",
        "src/opts/**/*",
        "src/ports/**/*",
        "src/utils/android/**/*",
        "src/utils/mac/**/*",
        "src/utils/SkThreadUtils_win.cpp",  # Windows-only. Move to ports?
        "src/utils/win/**/*",
        "src/views/sdl/*",
        "src/views/win/*",
        "src/views/unix/*",

        # Exclude multiple definitions.
        # TODO(mtklein): Move to opts?
        "src/doc/SkDocument_PDF_None.cpp",  # We use SkDocument_PDF.cpp.
        "src/gpu/gl/GrGLCreateNativeInterface_none.cpp",
        "src/gpu/gl/GrGLDefaultInterface_native.cpp",

        # Exclude files that don't compile with the current DEFINES.
        "src/gpu/gl/angle/*",  # Requires SK_ANGLE define.
        "src/gpu/gl/command_buffer/*",  # unknown type name 'HMODULE'
        "src/gpu/gl/mesa/*",  # Requires SK_MESA define.
        "src/svg/parser/*",  # Missing SkSVG.h.

        # Conflicting dependencies among Lua versions. See cl/107087297.
        "src/utils/SkLua*",

        # Not used.
        "src/animator/**/*",
        "src/views/**/*",
        "src/xml/SkBML_Verbs.h",
        "src/xml/SkBML_XMLParser.cpp",
        "src/xml/SkXMLPullParser.cpp",
    ],
)

# Platform-dependent SRCS for google3-default platform.
BASE_SRCS_UNIX = struct(
    include = [
        "src/codec/*",
        "src/fonts/SkFontMgr_fontconfig.cpp",
        "src/images/*",
        "src/opts/**/*.cpp",
        "src/opts/**/*.h",
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
        "src/codec/SkJpegCodec.cpp",  # libjpeg_turbo version mismatch.
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
        # Included in :opts_avx or :opts_avx2
        "src/opts/*avx*",
        "src/opts/SkBitmapProcState_opts_none.cpp",
        "src/opts/SkBlitMask_opts_none.cpp",
        "src/opts/SkBlitRow_opts_none.cpp",
        "src/ports/*android*",
        "src/ports/*chromium*",
        "src/ports/*mac*",
        "src/ports/*mozalloc*",
        "src/ports/*nacl*",
        "src/ports/*win*",
        "src/ports/SkFontConfigInterface_direct_factory.cpp",
        "src/ports/SkFontMgr_custom_directory_factory.cpp",
        "src/ports/SkFontMgr_custom_embedded_factory.cpp",
        "src/ports/SkFontMgr_empty_factory.cpp",
        "src/ports/SkFontMgr_fontconfig.cpp",
        "src/ports/SkImageDecoder_CG.cpp",
        "src/ports/SkFontMgr_fontconfig_factory.cpp",
        "src/ports/SkImageDecoder_WIC.cpp",
        "src/ports/SkImageDecoder_empty.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

# Platform-dependent SRCS for google3-default Android.
BASE_SRCS_ANDROID = struct(
    include = [
        "src/codec/*",
        "src/images/*",
        # TODO(benjaminwagner): Figure out how to compile with EGL.
        "src/opts/**/*.cpp",
        "src/opts/**/*.h",
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
        "src/codec/SkJpegCodec.cpp",  # libjpeg_turbo version mismatch.
        "src/opts/*mips*",
        "src/opts/*SSE2*",
        "src/opts/*SSSE3*",
        "src/opts/*ssse3*",
        "src/opts/*SSE4*",
        "src/opts/*sse4*",
        "src/opts/*avx*",
        "src/opts/*x86*",
        "src/opts/SkBitmapProcState_opts_none.cpp",
        "src/opts/SkBlitMask_opts_none.cpp",
        "src/opts/SkBlitRow_opts_none.cpp",
        "src/ports/*chromium*",
        "src/ports/*fontconfig*",
        "src/ports/*FontConfig*",
        "src/ports/*mac*",
        "src/ports/*mozalloc*",
        "src/ports/*nacl*",
        "src/ports/*win*",
        "src/ports/SkDebug_stdio.cpp",
        "src/ports/SkFontConfigInterface_direct_factory.cpp",
        "src/ports/SkFontConfigInterface_direct_google3_factory.cpp",
        "src/ports/SkFontMgr_custom_directory_factory.cpp",
        "src/ports/SkFontMgr_custom_embedded_factory.cpp",
        "src/ports/SkFontMgr_empty_factory.cpp",
        "src/ports/SkImageDecoder_CG.cpp",
        "src/ports/SkImageDecoder_WIC.cpp",
        "src/ports/SkImageDecoder_empty.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

# Platform-dependent SRCS for google3-default iOS.
BASE_SRCS_IOS = struct(
    include = [
        "src/opts/**/*.cpp",
        "src/opts/**/*.h",
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
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
        "src/opts/SkBitmapProcState_opts_none.cpp",
        "src/opts/SkBlitMask_opts_none.cpp",
        "src/opts/SkBlitRow_opts_none.cpp",
        "src/ports/*android*",
        "src/ports/*chromium*",
        "src/ports/*fontconfig*",
        "src/ports/*FontConfig*",
        "src/ports/*FreeType*",
        "src/ports/*mozalloc*",
        "src/ports/*nacl*",
        "src/ports/*win*",
        "src/ports/SkDebug_stdio.cpp",
        "src/ports/SkFontMgr_custom.cpp",
        "src/ports/SkFontConfigInterface_direct_factory.cpp",
        "src/ports/SkFontConfigInterface_direct_google3_factory.cpp",
        "src/ports/SkFontMgr_custom_directory_factory.cpp",
        "src/ports/SkFontMgr_custom_embedded_factory.cpp",
        "src/ports/SkFontMgr_empty_factory.cpp",
        "src/ports/SkImageDecoder_CG.cpp",
        "src/ports/SkImageDecoder_WIC.cpp",
        "src/ports/SkImageDecoder_empty.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

################################################################################
## SSSE3/SSE4/AVX/AVX2 SRCS
################################################################################

SSSE3_SRCS = struct(
    include = [
        "src/opts/*SSSE3*.cpp",
        "src/opts/*ssse3*.cpp",
    ])

SSE4_SRCS = struct(
    include = [
        "src/opts/*SSE4*.cpp",
        "src/opts/*sse4*.cpp",
    ])

AVX_SRCS = struct(
    include = [
        "src/opts/*_avx.cpp",
    ])

AVX2_SRCS = struct(
    include = [
        "src/opts/*_avx2.cpp",
    ])

################################################################################
## BASE_HDRS
################################################################################

BASE_HDRS = struct(
    include = [
        "include/**/*.h",
    ],
    exclude = [
        "include/private/**/*",

        # Not used.
        "include/animator/**/*",
        "include/views/**/*",
        "include/xml/SkBML_WXMLParser.h",
        "include/xml/SkBML_XMLParser.h",
    ])

################################################################################
## BASE_DEPS
################################################################################

BASE_DEPS_ALL = []

BASE_DEPS_UNIX = [
    ":opts_ssse3",
    ":opts_sse4",
    ":opts_avx",
    ":opts_avx2",
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
    "src/utils",
    "third_party/etc1",
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
        "tools/CrashHandler.cpp",
        "tools/CrashHandler.h",
        "tools/LazyDecodeBitmap.cpp",
        "tools/LazyDecodeBitmap.h",
        "tools/ProcStats.cpp",
        "tools/ProcStats.h",
        "tools/Resources.cpp",
        "tools/Resources.h",
        "tools/SkBitmapRegionCanvas.cpp",
        "tools/SkBitmapRegionCanvas.h",
        "tools/SkBitmapRegionCodec.cpp",
        "tools/SkBitmapRegionCodec.h",
        "tools/SkBitmapRegionDecoder.cpp",
        "tools/SkBitmapRegionDecoder.h",
        "tools/SkBitmapRegionSampler.cpp",
        "tools/SkBitmapRegionSampler.h",
        "tools/flags/*.cpp",
        "tools/flags/*.h",
        "tools/sk_tool_utils.cpp",
        "tools/sk_tool_utils.h",
        "tools/sk_tool_utils_font.cpp",
        "tools/timer/*.cpp",
        "tools/timer/*.h",
    ],
    exclude = [
        "dm/DMSrcSinkAndroid.cpp",  # Android-only.
        "tests/FontMgrAndroidParserTest.cpp",  # Android-only.
        "tests/PathOpsSkpClipTest.cpp",  # Alternate main.
        "tests/skia_test.cpp",  # Old main.
        "tests/SkpSkGrTest.cpp",  # Alternate main.
        "tools/timer/SysTimer_mach.cpp",
        "tools/timer/SysTimer_windows.cpp",
    ],
)

DM_SRCS_UNIX = struct()

DM_SRCS_ANDROID = struct(
    include = [
        # Depends on Android HWUI library that is not available in google3.
        #"dm/DMSrcSinkAndroid.cpp",
        "tests/FontMgrAndroidParserTest.cpp",
    ],
)

DM_SRCS_IOS = struct()

################################################################################
## DM_INCLUDES
################################################################################

DM_INCLUDES = [
    "gm",
    "src/codec",
    "src/effects",
    "src/fonts",
    "src/pathops",
    "src/pipe/utils",
    "src/ports",
    "src/utils/debugger",
    "tests",
    "tools",
    "tools/flags",
    "tools/timer",
]

################################################################################
## DM_ARGS
################################################################################

def DM_ARGS(base_dir):
    return [
        "--nogpu",
        "--verbose",
        # TODO(mtklein): maybe investigate why these fail?
        "--match ~FontMgr ~Scalar ~Canvas ~Codec_stripes ~Codec_Dimensions ~Codec ~Stream ~skps ~RecordDraw_TextBounds",
        "--resourcePath %s/resources" % base_dir,
        "--images %s/resources" % base_dir,
    ]

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
    "SK_BUILD_FOR_UNIX",
    "SK_SAMPLES_FOR_X",
    "SK_SFNTLY_SUBSETTER",
]

DEFINES_ANDROID = [
    "SK_BUILD_FOR_ANDROID",
    # TODO(benjaminwagner): Try to get png library updated?
    "SK_PNG_NO_INDEX_SUPPORTED",
]

DEFINES_IOS = [
    "SK_BUILD_FOR_IOS",
    "SK_IGNORE_ETC1_SUPPORT",
]

DEFINES_ALL = [
    # Chrome DEFINES.
    "SK_USE_FLOATBITS",
    "SK_USE_FREETYPE_EMBOLDEN",
    # Turn on a few Google3-specific build fixes.
    "GOOGLE3",
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

