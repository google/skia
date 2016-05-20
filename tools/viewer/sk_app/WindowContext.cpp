
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "SkSurface.h"
#include "WindowContext.h"

#include "gl/GrGLDefines.h"

#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"
#include "GrContext.h"

#include "SkCanvas.h"
#include "SkImage_Base.h"

namespace sk_app {

sk_sp<SkSurface> WindowContext::createRenderSurface(sk_sp<GrRenderTarget> rt, int colorBits) {
    auto flags = (fSurfaceProps.flags() & ~SkSurfaceProps::kGammaCorrect_Flag) |
                 (GrPixelConfigIsSRGB(fPixelConfig) ? SkSurfaceProps::kGammaCorrect_Flag : 0);
    SkSurfaceProps props(flags, fSurfaceProps.pixelGeometry());

    if (!this->isGpuContext() || colorBits > 24 ||
        kRGBA_F16_SkColorType == fDisplayParams.fColorType) {
        // If we're rendering to F16, we need an off-screen surface - the current render
        // target is most likely the wrong format.
        //
        // If we're rendering raster data or using a deep (10-bit or higher) surface, we probably
        // need an off-screen surface. 10-bit, in particular, has strange gamma behavior.
        SkImageInfo info = SkImageInfo::Make(fWidth, fHeight,
                                             fDisplayParams.fColorType,
                                             kUnknown_SkAlphaType,
                                             fDisplayParams.fProfileType);
        return SkSurface::MakeRenderTarget(fContext, SkBudgeted::kNo, info,
                                           fDisplayParams.fMSAASampleCount, &props);
    } else {
        return SkSurface::MakeRenderTargetDirect(rt.get(), &props);
    }
}

void WindowContext::presentRenderSurface(sk_sp<SkSurface> renderSurface, sk_sp<GrRenderTarget> rt,
                                         int colorBits) {
    if (!this->isGpuContext() || colorBits > 24 ||
        kRGBA_F16_SkColorType == fDisplayParams.fColorType) {
        // We made/have an off-screen surface. Get the contents as an SkImage:
        SkImageInfo info = SkImageInfo::Make(fWidth, fHeight,
                                             fDisplayParams.fColorType,
                                             kUnknown_SkAlphaType,
                                             fDisplayParams.fProfileType);
        SkBitmap bm;
        bm.allocPixels(info);
        renderSurface->getCanvas()->readPixels(&bm, 0, 0);
        SkPixmap pm;
        bm.peekPixels(&pm);
        sk_sp<SkImage> image(SkImage::MakeTextureFromPixmap(fContext, pm,
                             SkBudgeted::kNo));
        GrTexture* texture = as_IB(image)->peekTexture();
        SkASSERT(texture);

        // With ten-bit output, we need to manually apply the gamma of the output device
        // (unless we're in non-gamma correct mode, in which case our data is already
        // fake-sRGB, like we're expected to put in the 10-bit buffer):
        bool doGamma = (colorBits == 30) && SkImageInfoIsGammaCorrect(info);
        fContext->applyGamma(rt.get(), texture, doGamma ? 1.0f / 2.2f : 1.0f);
    }
}

}   //namespace sk_app
