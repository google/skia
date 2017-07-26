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
        "include/private/**/*.h",
        "src/**/*.h",
        "src/**/*.cpp",
        "src/**/*.inc",
        "src/jumper/SkJumper_generated.S",

        # Third Party
        "third_party/etc1/*.cpp",
        "third_party/etc1/*.h",
        "third_party/gif/*.cpp",
        "third_party/gif/*.h",
    ],
    exclude = [
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

        # Only pre-compiled into SkJumper_generated.S.
        "src/jumper/SkJumper_stages_lowp.cpp",
    ],
)

# Platform-dependent SRCS for google3-default platform.
BASE_SRCS_UNIX = struct(
    include = [
        "src/android/*",
        "src/codec/*",
        "src/gpu/gl/GrGLDefaultInterface_none.cpp",
        "src/opts/**/*.cpp",
        "src/opts/**/*.h",
        "src/ports/**/*.cpp",
        "src/ports/**/*.h",
    ],
    exclude = [
        "src/opts/opts_check_x86.cpp",
        "src/opts/*arm*",
        "src/opts/*mips*",
        "src/opts/*NEON*",
        "src/opts/*neon*",
        # Included in :opts_sse2 library.
        "src/opts/*SSE2*",
        "src/opts/*sse2*",
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
        "src/ports/SkGlobalInitialization_none.cpp",
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
        "src/opts/*hsw*",
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
        "src/ports/SkGlobalInitialization_none.cpp",
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
        "src/opts/*hsw*",
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
## skia_{all,public}_hdrs()
################################################################################
def skia_all_hdrs():
  return native.glob(["src/**/*.h", "include/**/*.h"])

def skia_public_hdrs():
  return native.glob(["include/**/*.h"],
                     exclude=[
                         "include/private/**/*.h",
                         "include/views/**/*",  # Not used.
                     ])

################################################################################
## skia_opts_srcs()
################################################################################
# Intel
SKIA_OPTS_SSE2 = "SSE2"

SKIA_OPTS_SSSE3 = "SSSE3"

SKIA_OPTS_SSE4 = "SSE4"

SKIA_OPTS_AVX = "AVX"

SKIA_OPTS_HSW = "HSW"

# Arm
SKIA_OPTS_ARMV7 = "ARMV7"

SKIA_OPTS_NEON = "NEON"

SKIA_OPTS_ARM64 = "ARM64"

SKIA_OPTS_CRC32 = "CRC32"  # arm64

# Other
SKIA_OPTS_NONE = "NONE"  # not x86, arm, or arm64

def skia_opts_srcs(opts):
  if opts == SKIA_OPTS_SSE2:
    return native.glob([
        "src/opts/*SSE2*.cpp",
        "src/opts/*sse2*.cpp",
    ])
  elif opts == SKIA_OPTS_SSSE3:
    return native.glob([
        "src/opts/*SSSE3*.cpp",
        "src/opts/*ssse3*.cpp",
    ])
  elif opts == SKIA_OPTS_SSE4:
    return native.glob([
        "src/opts/*SSE4*.cpp",
        "src/opts/*sse4*.cpp",
    ])
  elif opts == SKIA_OPTS_AVX:
    return native.glob([
        "src/opts/*_avx.cpp",
    ])
  elif opts == SKIA_OPTS_HSW:
    return native.glob([
        "src/opts/*_hsw.cpp",
    ])
  elif opts == SKIA_OPTS_ARMV7:
    return native.glob([
        "src/opts/*_arm.cpp",
    ])
  elif opts == SKIA_OPTS_NEON:
    return native.glob([
        "src/opts/*_neon.cpp",
    ])
  elif opts == SKIA_OPTS_CRC32:
    return native.glob([
        "src/opts/*_crc32.cpp",
    ])
  elif opts == SKIA_OPTS_NONE:
    return native.glob([
        "src/opts/*_none.cpp",
    ])
  else:
    fail("skia_opts_srcs parameter 'opts' must be one of SKIA_OPTS_*.")

def skia_opts_cflags(opts):
  if opts == SKIA_OPTS_SSE2:
    return ["-msse2"]
  elif opts == SKIA_OPTS_SSSE3:
    return ["-mssse3"]
  elif opts == SKIA_OPTS_SSE4:
    return ["-msse4"]
  elif opts == SKIA_OPTS_AVX:
    return ["-mavx"]
  elif opts == SKIA_OPTS_HSW:
    return ["-mavx2", "-mbmi", "-mbmi2", "-mf16c", "-mfma"]
  elif opts == SKIA_OPTS_CRC32:
    return ["-march=armv8-a+crc"]
  else:
    return []

################################################################################
## skia_srcs()
################################################################################
SKIA_OS_UNIX = "UNIX"

SKIA_OS_ANDROID = "ANDROID"

SKIA_OS_IOS = "IOS"

SKIA_CPU_UNSPECIFIED = "UNSPECIFIED"

SKIA_CPU_ARM = "ARM"

SKIA_CPU_PPC = "PPC"

def skia_srcs(os=SKIA_OS_UNIX, cpu=SKIA_CPU_UNSPECIFIED):
  """Sources to be compiled into the skia library."""
  srcs = skia_glob(BASE_SRCS_ALL)
  if os == SKIA_OS_IOS:
    if cpu != SKIA_CPU_UNSPECIFIED:
      fail("Do not specify IOS and a cpu.")
    srcs = srcs + skia_glob(BASE_SRCS_IOS)
  elif os == SKIA_OS_ANDROID:
    if cpu != SKIA_CPU_UNSPECIFIED:
      fail("Do not specify ANDROID and a cpu.")
    srcs = srcs + skia_glob(BASE_SRCS_ANDROID)
  elif os == SKIA_OS_UNIX:
    if cpu == SKIA_CPU_UNSPECIFIED:
      srcs = srcs + ["src/opts/opts_check_x86.cpp"] + skia_glob(BASE_SRCS_UNIX)
    elif cpu == SKIA_CPU_PPC or cpu == SKIA_CPU_ARM:
      srcs = srcs + skia_glob(BASE_SRCS_UNIX)
    else:
      fail("cpu must be one of SKIA_CPU_*")
  else:
    fail("skia_srcs parameter 'os' must be one of SKIA_OS_{UNIX,ANDROID,IOS}.")
  return srcs

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
    "src/lazy",
    "src/opts",
    "src/ports",
    "src/pdf",
    "src/sfnt",
    "src/shaders",
    "src/sksl",
    "src/utils",
    "third_party/etc1",
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
        "tools/trace/*.cpp",
        "tools/trace/*.h",
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

################################################################################
## dm_srcs()
################################################################################
# SKIA_OS_* definitions can be found above.

def dm_srcs(os):
  """Sources for the dm binary for the specified os."""
  srcs = skia_glob(DM_SRCS_ALL)
  # TODO(benjaminwagner): None of the CreatePlatformGLContext_*.cpp files exist.
  # TODO(jwg): Remove the globs if possible, they only select single files.
  if os == SKIA_OS_IOS:
    srcs = srcs + native.glob(["tools/gpu/iOS/CreatePlatformGLContext_iOS.cpp"])
  elif os == SKIA_OS_ANDROID:
    srcs = srcs + native.glob([
        "tests/FontMgrAndroidParserTest.cpp",
        # TODO(benjaminwagner): Figure out how to compile with EGL.
        "tools/gpu/gl/CreatePlatformGLContext_none.cpp",
    ])
  elif os == SKIA_OS_UNIX:
    srcs = srcs + native.glob(["tools/gpu/gl/CreatePlatformGLContext_none.cpp"])
  else:
    fail("dm_srcs parameter 'os' must be one of SKIA_OS_{UNIX,ANDROID,IOS}.")
  return srcs

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
    "tools/gpu",
    "tools/timer",
    "tools/trace",
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
        "~bigrect",
        "~clippedcubic2",
        "~conicpaths",
        "~^gradients",
        "~Math",
        "~Matrix",
        "~PathBigCubic",
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

COPTS_ANDROID = [
    "-mfpu=neon",
    "-Wno-error=attributes",  # 'GrResourceCache' declared with greater visibility than the
                              # type of its field 'GrResourceCache::fPurgeableQueue'... bogus.
]

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
    "SK_HAS_JPEG_LIBRARY",
    "SKNX_NO_SIMD",
]

DEFINES_ALL = [
    # Chrome DEFINES.
    "SK_USE_FLOATBITS",
    "SK_USE_FREETYPE_EMBOLDEN",
    # Turn on a few Google3-specific build fixes.
    "GOOGLE3",
    # Required for building dm.
    "GR_TEST_UTILS",
    # Staging flags for API changes
    # Should remove after we update golden images
    "SK_WEBP_ENCODER_USE_DEFAULT_METHOD",
    # Temporarily Disable analytic AA for Google3
    "SK_NO_ANALYTIC_AA",
    # Experiment to diagnose image diffs in Google3
    "SK_DISABLE_SSSE3_RUNTIME_CHECK_FOR_LOWP_STAGES",
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
