
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkUserConfig_DEFINED
#define SkUserConfig_DEFINED

/*  SkTypes.h, the root of the public header files, does the following trick:

    #include "SkPreConfig.h"
    #include "SkUserConfig.h"
    #include "SkPostConfig.h"

    SkPreConfig.h runs first, and it is responsible for initializing certain
    skia defines.

    SkPostConfig.h runs last, and its job is to just check that the final
    defines are consistent (i.e. that we don't have mutually conflicting
    defines).

    SkUserConfig.h (this file) runs in the middle. It gets to change or augment
    the list of flags initially set in preconfig, and then postconfig checks
    that everything still makes sense.

    Below are optional defines that add, subtract, or change default behavior
    in Skia. Your port can locally edit this file to enable/disable flags as
    you choose, or these can be delared on your command line (i.e. -Dfoo).

    By default, this include file will always default to having all of the flags
    commented out, so including it will have no effect.
*/

///////////////////////////////////////////////////////////////////////////////

/*  Scalars (the fractional value type in skia) can be implemented either as
    floats or 16.16 integers (fixed). Exactly one of these two symbols must be
    defined.
*/
//#define SK_SCALAR_IS_FLOAT
//#define SK_SCALAR_IS_FIXED


/*  For some performance-critical scalar operations, skia will optionally work
    around the standard float operators if it knows that the CPU does not have
    native support for floats. If your environment uses software floating point,
    define this flag.
 */
//#define SK_SOFTWARE_FLOAT


/*  Skia has lots of debug-only code. Often this is just null checks or other
    parameter checking, but sometimes it can be quite intrusive (e.g. check that
    each 32bit pixel is in premultiplied form). This code can be very useful
    during development, but will slow things down in a shipping product.

    By default, these mutually exclusive flags are defined in SkPreConfig.h,
    based on the presence or absence of NDEBUG, but that decision can be changed
    here.
 */
//#define SK_DEBUG
//#define SK_RELEASE

/*  Skia has certain debug-only code that is extremely intensive even for debug
    builds.  This code is useful for diagnosing specific issues, but is not
    generally applicable, therefore it must be explicitly enabled to avoid
    the performance impact. By default these flags are undefined, but can be
    enabled by uncommenting them below.
 */
//#define SK_DEBUG_GLYPH_CACHE
//#define SK_DEBUG_PATH

/*  To assist debugging, Skia provides an instance counting utility in
    include/core/SkInstCount.h. This flag turns on and off that utility to
    allow instance count tracking in either debug or release builds. By
    default it is enabled in debug but disabled in release.
 */
//#define SK_ENABLE_INST_COUNT 1

/*  If, in debugging mode, Skia needs to stop (presumably to invoke a debugger)
    it will call SK_CRASH(). If this is not defined it, it is defined in
    SkPostConfig.h to write to an illegal address
 */
//#define SK_CRASH() *(int *)(uintptr_t)0 = 0


/*  preconfig will have attempted to determine the endianness of the system,
    but you can change these mutually exclusive flags here.
 */
//#define SK_CPU_BENDIAN
//#define SK_CPU_LENDIAN

/*  Most compilers use the same bit endianness for bit flags in a byte as the
    system byte endianness, and this is the default. If for some reason this
    needs to be overridden, specify which of the mutually exclusive flags to
    use. For example, some atom processors in certain configurations have big
    endian byte order but little endian bit orders.
*/
//#define SK_UINT8_BITFIELD_BENDIAN
//#define SK_UINT8_BITFIELD_LENDIAN


/*  Some compilers don't support long long for 64bit integers. If yours does
    not, define this to the appropriate type.
 */
//#define SkLONGLONG int64_t


/*  To write debug messages to a console, skia will call SkDebugf(...) following
    printf conventions (e.g. const char* format, ...). If you want to redirect
    this to something other than printf, define yours here
 */
//#define SkDebugf(...)  MyFunction(__VA_ARGS__)

/*
 *  To specify a different default font cache limit, define this. If this is
 *  undefined, skia will use a built-in value.
 */
//#define SK_DEFAULT_FONT_CACHE_LIMIT   (1024 * 1024)

/*
 *  To specify the default size of the image cache, undefine this and set it to
 *  the desired value (in bytes). SkGraphics.h as a runtime API to set this
 *  value as well. If this is undefined, a built-in value will be used.
 */
//#define SK_DEFAULT_IMAGE_CACHE_LIMIT (1024 * 1024)

/*  If zlib is available and you want to support the flate compression
    algorithm (used in PDF generation), define SK_ZLIB_INCLUDE to be the
    include path. Alternatively, define SK_SYSTEM_ZLIB to use the system zlib
    library specified as "#include <zlib.h>".
 */
//#define SK_ZLIB_INCLUDE <zlib.h>
//#define SK_SYSTEM_ZLIB

/*  Define this to allow PDF scalars above 32k.  The PDF/A spec doesn't allow
    them, but modern PDF interpreters should handle them just fine.
 */
//#define SK_ALLOW_LARGE_PDF_SCALARS

/*  Define this to provide font subsetter in PDF generation.
 */
//#define SK_SFNTLY_SUBSETTER "sfntly/subsetter/font_subsetter.h"

/*  Define this to set the upper limit for text to support LCD. Values that
    are very large increase the cost in the font cache and draw slower, without
    improving readability. If this is undefined, Skia will use its default
    value (e.g. 48)
 */
//#define SK_MAX_SIZE_FOR_LCDTEXT     48

/*  If SK_DEBUG is defined, then you can optionally define SK_SUPPORT_UNITTEST
    which will run additional self-tests at startup. These can take a long time,
    so this flag is optional.
 */
#ifdef SK_DEBUG
//#define SK_SUPPORT_UNITTEST
#endif

/* If your system embeds skia and has complex event logging, define this
   symbol to name a file that maps the following macros to your system's
   equivalents:
       SK_TRACE_EVENT0(event)
       SK_TRACE_EVENT1(event, name1, value1)
       SK_TRACE_EVENT2(event, name1, value1, name2, value2)
   src/utils/SkDebugTrace.h has a trivial implementation that writes to
   the debug output stream. If SK_USER_TRACE_INCLUDE_FILE is not defined,
   SkTrace.h will define the above three macros to do nothing.
*/
//#undef SK_USER_TRACE_INCLUDE_FILE

/*  Change the ordering to work in X windows.
 */
#ifdef SK_SAMPLES_FOR_X
        #define SK_R32_SHIFT    16
        #define SK_G32_SHIFT    8
        #define SK_B32_SHIFT    0
        #define SK_A32_SHIFT    24
#endif


/* Determines whether to build code that supports the GPU backend. Some classes
   that are not GPU-specific, such as SkShader subclasses, have optional code
   that is used allows them to interact with the GPU backend. If you'd like to
   omit this code set SK_SUPPORT_GPU to 0. This also allows you to omit the gpu
   directories from your include search path when you're not building the GPU
   backend. Defaults to 1 (build the GPU code).
 */
//#define SK_SUPPORT_GPU 1


/* The PDF generation code uses Path Ops to generate inverse fills and complex
 * clipping paths, but at this time, Path Ops is not release ready yet. So,
 * the code is hidden behind this #define guard. If you are feeling adventurous
 * and want the latest and greatest PDF generation code, uncomment the #define.
 * When Path Ops is release ready, the define guards and this user config
 * define should be removed entirely.
 */
//#define SK_PDF_USE_PATHOPS

#endif
