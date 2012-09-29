
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTrace_DEFINED
#define SkTrace_DEFINED

#ifdef SK_USER_TRACE_INCLUDE_FILE

/* If your system embeds skia and has complex event logging, in
   src/config/SkUserConfig.h:
     - define the three SK_TRACE_EVENT macros to map to your system's
       equivalents,
     - define the name of the include file in SK_USER_TRACE_INCLUDE_FILE
   A trivial example is given in src/utils/SkDebugTrace.h.

   All arguments are const char*. Skia typically passes the name of
   the object and function (and sometimes region of interest within
   the function) separated by double colons for 'event'.

   SK_TRACE_EVENT1 and SK_TRACE_EVENT2 take one or two arbitrary
   name-value pairs that you also want to log. SkStringPrintf() is useful
   for formatting these values.

   For example:
    SK_TRACE_EVENT0("GrContext::createAndLockTexture");
    SK_TRACE_EVENT1("GrDefaultPathRenderer::onDrawPath::renderPasses",
                    "verts", SkStringPrintf("%i", vert - base).c_str());
*/

    #include SK_USER_TRACE_INCLUDE_FILE

#else

    #define SK_TRACE_EVENT0(event)
    #define SK_TRACE_EVENT1(event, name1, value1)
    #define SK_TRACE_EVENT2(event, name1, value1, name2, value2)

#endif

#endif


