/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef fiddle_main_DEFINED
#define fiddle_main_DEFINED

#ifdef FIDDLE_BUILD_TEST
    #include "include/core/SkCanvas.h"
    #include "include/core/SkDocument.h"
    #include "include/core/SkPictureRecorder.h"
    #include "include/core/SkStream.h"
    #include "include/core/SkSurface.h"
    #include "include/gpu/GrContext.h"
    #include "include/gpu/gl/GrGLAssembleInterface.h"
    #include "include/gpu/gl/GrGLInterface.h"
#else
    #include "skia.h"
#endif

#include <memory>
#include <sstream>

extern GrBackendTexture backEndTexture;
extern GrBackendRenderTarget backEndRenderTarget;
extern GrBackendTexture backEndTextureRenderTarget;
extern SkBitmap source;
extern sk_sp<SkImage> image;
extern double duration; // The total duration of the animation in seconds.
extern double frame;    // A value in [0, 1] of where we are in the animation.

namespace sk_gpu_test {
class GLTestContext;
}

struct DrawOptions {
    DrawOptions(int w, int h, bool r, bool g, bool p, bool k, bool srgb, bool f16,
                bool textOnly, const char* s,
                GrMipMapped mipMapping,
                int offScreenWidth,
                int offScreenHeight,
                int offScreenSampleCount,
                GrMipMapped offScreenMipMapping)
        : size(SkISize::Make(w, h))
        , raster(r)
        , gpu(g)
        , pdf(p)
        , skp(k)
        , srgb(srgb)
        , f16(f16)
        , textOnly(textOnly)
        , source(s)
        , fMipMapping(mipMapping)
        , fOffScreenWidth(offScreenWidth)
        , fOffScreenHeight(offScreenHeight)
        , fOffScreenSampleCount(offScreenSampleCount)
        , fOffScreenMipMapping(offScreenMipMapping) {
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

    // This flag is used when a GPU texture resource is created and exposed as a GrBackendTexture.
    // In this case the resource is created with extra room to accomodate mipmaps.
    // TODO: The SkImage::makeTextureImage API would need to be widened to allow this to be true
    // for the non-backend gpu SkImages.
    GrMipMapped fMipMapping;

    // Parameters for an GPU offscreen resource exposed as a GrBackendRenderTarget
    int         fOffScreenWidth;
    int         fOffScreenHeight;
    int         fOffScreenSampleCount;
    // TODO: should we also expose stencilBits here? How about the config?

    GrMipMapped fOffScreenMipMapping; // only applicable if the offscreen is also textureable
};

extern DrawOptions GetDrawOptions();
extern void SkDebugf(const char * format, ...);
extern void draw(SkCanvas*);

// There are different implementations of create_grcontext() for EGL, Mesa,
// and a fallback to a null context.
extern sk_sp<GrContext> create_grcontext(std::ostringstream& driverinfo,
                                         std::unique_ptr<sk_gpu_test::GLTestContext>* glContext);

#endif  // fiddle_main_DEFINED
