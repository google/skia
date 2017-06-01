/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef fiddle_main_DEFINED
#define fiddle_main_DEFINED

#ifdef FIDDLE_BUILD_TEST
    #include "GrContext.h"
    #include "SkCanvas.h"
    #include "SkDocument.h"
    #include "SkPictureRecorder.h"
    #include "SkStream.h"
    #include "SkSurface.h"
    #include "gl/GrGLAssembleInterface.h"
    #include "gl/GrGLInterface.h"
#else
    #include "skia.h"
#endif

#include <sstream>

extern SkBitmap source;
extern sk_sp<SkImage> image;
extern double duration; // The total duration of the animation in seconds.
extern double frame;    // A value in [0, 1] of where we are in the animation.

struct DrawOptions {
    DrawOptions(int w, int h, bool r, bool g, bool p, bool k, bool srgb, bool f16, bool textOnly, const char* s)
        : size(SkISize::Make(w, h))
        , raster(r)
        , gpu(g)
        , pdf(p)
        , skp(k)
        , srgb(srgb)
        , f16(f16)
        , textOnly(textOnly)
        , source(s)
    {
        // F16 mode is only valid for color correct backends.
        SkASSERT(srgb || !f16);
    }
    SkISize size;
    bool raster;
    bool gpu;
    bool pdf;
    bool skp;
    bool srgb;
    bool f16;
    bool textOnly;
    const char* source;
};

extern DrawOptions GetDrawOptions();
extern void SkDebugf(const char * format, ...);
extern void draw(SkCanvas*);

// There are different implementations of create_grcontext() for EGL, Mesa,
// and a fallback to a null context.
extern sk_sp<GrContext> create_grcontext(std::ostringstream &driverinfo);

#endif  // fiddle_main_DEFINED
