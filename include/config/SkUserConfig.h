
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


/*  Somewhat independent of how SkScalar is implemented, Skia also wants to know
    if it can use floats at all. Naturally, if SK_SCALAR_IS_FLOAT is defined,
    SK_CAN_USE_FLOAT must be too; but if scalars are fixed, SK_CAN_USE_FLOAT
    can go either way.
 */
//#define SK_CAN_USE_FLOAT

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


/*  Some compilers don't support long long for 64bit integers. If yours does
    not, define this to the appropriate type.
 */
//#define SkLONGLONG int64_t


/*  Some envorinments do not suport writable globals (eek!). If yours does not,
    define this flag.
 */
//#define SK_USE_RUNTIME_GLOBALS


/*  To write debug messages to a console, skia will call SkDebugf(...) following
    printf conventions (e.g. const char* format, ...). If you want to redirect
    this to something other than printf, define yours here
 */
//#define SkDebugf(...)  MyFunction(__VA_ARGS__)

/* If defined, use CoreText instead of ATSUI on OS X.
*/
//#define SK_USE_MAC_CORE_TEXT


/*  If zlib is available and you want to support the flate compression
    algorithm (used in PDF generation), define SK_ZLIB_INCLUDE to be the
    include path.
 */
//#define SK_ZLIB_INCLUDE <zlib.h>

/*  Define this to allow PDF scalars above 32k.  The PDF/A spec doesn't allow
    them, but modern PDF interpreters should handle them just fine.
 */
//#define SK_ALLOW_LARGE_PDF_SCALARS

/*  Define this to provide font subsetter in PDF generation.
 */
//#define SK_SFNTLY_SUBSETTER "sfntly/subsetter/font_subsetter.h"

/*  Define this to remove dimension checks on bitmaps. Not all blits will be
    correct yet, so this is mostly for debugging the implementation.
 */
//#define SK_ALLOW_OVER_32K_BITMAPS

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

#endif
