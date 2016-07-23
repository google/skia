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

extern SkBitmap source;
extern sk_sp<SkImage> image;

struct DrawOptions {
    DrawOptions(int w, int h, bool r, bool g, bool p, bool k, const char* s)
        : size(SkISize::Make(w, h))
        , raster(r)
        , gpu(g)
        , pdf(p)
        , skp(k)
        , source(s) {}
    SkISize size;
    bool raster;
    bool gpu;
    bool pdf;
    bool skp;
    const char* source;
};

extern DrawOptions GetDrawOptions();
extern void draw(SkCanvas*);

#endif  // fiddle_main_DEFINED
