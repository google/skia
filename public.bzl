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
## skia_{all,public}_hdrs()
################################################################################
def skia_all_hdrs():
  return native.glob([
      "src/**/*.h",
      "include/**/*.h",
      "third_party/**/*.h",
  ])

def skia_public_hdrs():
  return native.glob(["include/**/*.h"],
                     exclude=[
                         "include/private/**/*",
                         "include/views/**/*",  # Not used.
                     ])

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
        "src/jumper/SkJumper_generated.S",

        # Third Party
        "third_party/gif/*.cpp",
        "third_party/gif/*.h",
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
        "src/views/sdl/*",
        "src/views/win/*",
        "src/views/unix/*",

        # Exclude multiple definitions.
        # TODO(mtklein): Move to opts?
        "src/pdf/SkDocument_PDF_None.cpp",  # We use src/pdf/SkPDFDocument.cpp.
        "src/gpu/gl/GrGLMakeNativeInterface_none.cpp",

        # Exclude files that don't compile with the current DEFINES.
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

        # Only used to regenerate the lexer
        "src/sksl/lex/*",

        # Atlas text
        "src/atlastext/*",
    ],
)

def codec_srcs(limited):
  """Sources for the codecs. Excludes Ico, Webp, Png, and Raw if limited."""
  exclude = []
  if limited:
    exclude += [
        "src/codec/*Ico*.cpp",
        "src/codec/*Webp*.cpp",
        "src/codec/*Png*",
        "src/codec/*Raw*.cpp",
    ]
  return native.glob(["src/codec/*.cpp"], exclude = exclude)

# Platform-dependent SRCS for google3-default platform.
BASE_SRCS_UNIX = struct(
    include = [
        "src/gpu/gl/GrGLMakeNativeInterface_none.cpp",
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
        "src/ports/SkFontMgr_fontconfig.cpp",
        "src/ports/SkFontMgr_fontconfig_factory.cpp",
        "src/ports/SkGlobalInitialization_none.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

# Platform-dependent SRCS for google3-default Android.
BASE_SRCS_ANDROID = struct(
    include = [
        "src/gpu/gl/GrGLMakeNativeInterface_none.cpp",
        # TODO(benjaminwagner): Figure out how to compile with EGL.
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
        "src/ports/SkGlobalInitialization_none.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

# Platform-dependent SRCS for google3-default iOS.
BASE_SRCS_IOS = struct(
    include = [
        "src/gpu/gl/iOS/GrGLMakeNativeInterface_iOS.cpp",
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
        "src/ports/SkGlobalInitialization_none.cpp",
        "src/ports/SkImageGenerator_none.cpp",
        "src/ports/SkTLS_none.cpp",
    ],
)

################################################################################
## skia_srcs()
################################################################################
def skia_srcs(os_conditions):
  """Sources to be compiled into the skia library."""
  return skia_glob(BASE_SRCS_ALL) + skia_select(
      os_conditions,
      [
          skia_glob(BASE_SRCS_UNIX),
          skia_glob(BASE_SRCS_ANDROID),
          skia_glob(BASE_SRCS_IOS),
      ],
  )

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
    "include/encode",
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
    "src/images",
    "src/lazy",
    "src/opts",
    "src/ports",
    "src/pdf",
    "src/sfnt",
    "src/shaders",
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
        "tools/fonts/SkRandomScalerContext.cpp",
        "tools/fonts/SkRandomScalerContext.h",
        "tools/fonts/SkTestFontMgr.cpp",
        "tools/fonts/SkTestFontMgr.h",
        "tools/fonts/SkTestScalerContext.cpp",
        "tools/fonts/SkTestScalerContext.h",
        "tools/fonts/sk_tool_utils_font.cpp",
        "tools/fonts/test_font_monospace.inc",
        "tools/fonts/test_font_sans_serif.inc",
        "tools/fonts/test_font_serif.inc",
        "tools/fonts/test_font_index.inc",
        "tools/gpu/**/*.cpp",
        "tools/gpu/**/*.h",
        "tools/picture_utils.cpp",
        "tools/picture_utils.h",
        "tools/random_parse_path.cpp",
        "tools/random_parse_path.h",
        "tools/sk_tool_utils.cpp",
        "tools/sk_tool_utils.h",
        "tools/timer/*.cpp",
        "tools/timer/*.h",
        "tools/trace/*.cpp",
        "tools/trace/*.h",
    ],
    exclude = [
        "tests/FontMgrAndroidParserTest.cpp",  # Android-only.
        "tests/skia_test.cpp",  # Old main.
        "tests/SVGDeviceTest.cpp",
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
          [],
          ["tests/FontMgrAndroidParserTest.cpp"],
          [],
      ],
  )

################################################################################
## DM_INCLUDES
################################################################################

DM_INCLUDES = [
    "dm",
    "gm",
    "experimental/svg/model",
    "src/codec",
    "src/core",
    "src/effects",
    "src/fonts",
    "src/images",
    "src/pathops",
    "src/pipe/utils",
    "src/ports",
    "src/shaders",
    "src/shaders/gradients",
    "src/xml",
    "tests",
    "tools",
    "tools/debugger",
    "tools/flags",
    "tools/fonts",
    "tools/gpu",
    "tools/timer",
    "tools/trace",
]

################################################################################
## DM_ARGS
################################################################################

def DM_ARGS(asan):
  source = ["tests", "gm", "image"]
  # TODO(benjaminwagner): f16, pic-8888, serialize-8888, and tiles_rt-8888 fail.
  config = ["565", "8888", "pdf", "srgb"]
  # TODO(mtklein): maybe investigate why these fail?
  match = [
      "~^FontHostStream$$",
      "~^FontMgr$$",
      "~^PaintBreakText$$",
      "~^RecordDraw_TextBounds$$",
  ]
  return ["--src"] + source + ["--config"] + config + ["--match"] + match

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
          ],
          # ANDROID
          [
              # 'GrResourceCache' declared with greater visibility than the
              # type of its field 'GrResourceCache::fPurgeableQueue'... bogus.
              "-Wno-error=attributes",
          ],
          # IOS
          [],
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
      # Staging flags for API changes
      # Should remove after we update golden images
      "SK_WEBP_ENCODER_USE_DEFAULT_METHOD",
      # Experiment to diagnose image diffs in Google3
      "SK_JUMPER_DISABLE_8BIT",
      # JPEG is in codec_limited
      "SK_HAS_JPEG_LIBRARY",
  ] + skia_select(
      os_conditions,
      [
          # UNIX
          [
              "PNG_SKIP_SETJMP_CHECK",
              "SK_BUILD_FOR_UNIX",
              "SK_SAMPLES_FOR_X",
              "SK_PDF_USE_SFNTLY",
              "SK_CODEC_DECODES_RAW",
              "SK_HAS_PNG_LIBRARY",
              "SK_HAS_WEBP_LIBRARY",
          ],
          # ANDROID
          [
              "SK_BUILD_FOR_ANDROID",
              "SK_CODEC_DECODES_RAW",
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
          ],
          # IOS
          [
              "-framework CoreFoundation",
              "-framework CoreGraphics",
              "-framework CoreText",
              "-framework ImageIO",
              "-framework MobileCoreServices",
          ],
      ]
  )
