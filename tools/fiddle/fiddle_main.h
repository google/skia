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
    #include "include/gpu/GrDirectContext.h"
    #include "include/gpu/gl/GrGLAssembleInterface.h"
    #include "include/gpu/gl/GrGLInterface.h"
#else
    #include "skia.h"
#endif

#include <memory>
#include <sstream>

namespace sk_gpu_test {
class GLTestContext;
class ManagedBackendTexture;
}  // namespace sk_gpu_test

extern sk_sp<sk_gpu_test::ManagedBackendTexture> backEndTexture;
extern GrBackendRenderTarget backEndRenderTarget;
extern sk_sp<sk_gpu_test::ManagedBackendTexture> backEndTextureRenderTarget;
extern SkBitmap source;
extern sk_sp<SkImage> image;
extern double duration; // The total duration of the animation in seconds.
extern double frame;    // A value in [0, 1] of where we are in the animation.

struct DrawOptions {
    DrawOptions(int w, int h, bool r, bool g, bool p, bool k, bool srgb, bool f16,
                bool textOnly, const char* s,
                GrMipmapped mipMapping,
                int offScreenWidth,
                int offScreenHeight,
                int deprecated, // TODO(jcgregorio): remove
                GrMipmapped offScreenMipMapping)
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
    // In this case the resource is created with extra room to accommodate mipmaps.
    // TODO: The SkImage::makeTextureImage API would need to be widened to allow this to be true
    // for the non-backend gpu SkImages.
    GrMipmapped fMipMapping;

    // Parameters for an GPU offscreen resource exposed as a GrBackendRenderTarget
    int         fOffScreenWidth;
    int         fOffScreenHeight;
    // TODO: should we also expose stencilBits here? How about the config?

    GrMipmapped fOffScreenMipMapping; // only applicable if the offscreen is also textureable
};

extern DrawOptions GetDrawOptions();
extern void SkDebugf(const char * format, ...);
extern void draw(SkCanvas*);

// There are different implementations of create_direct_context() for EGL, Mesa,
// and a fallback to a null context.
extern sk_sp<GrDirectContext> create_direct_context(std::ostringstream& driverinfo,
                                                    std::unique_ptr<sk_gpu_test::GLTestContext>*);

#endif  // fiddle_main_DEFINED
