/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/GrSurface.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrOpList.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfacePriv.h"

#include "src/core/SkMathPriv.h"
#include "src/gpu/SkGr.h"

size_t GrSurface::WorstCaseSize(const GrSurfaceDesc& desc, GrRenderable renderable, bool binSize) {
    size_t size;

    int width  = binSize ? GrResourceProvider::MakeApprox(desc.fWidth)  : desc.fWidth;
    int height = binSize ? GrResourceProvider::MakeApprox(desc.fHeight) : desc.fHeight;

    if (renderable == GrRenderable::kYes) {
        // We own one color value for each MSAA sample.
        SkASSERT(desc.fSampleCnt >= 1);
        int colorValuesPerPixel = desc.fSampleCnt;
        if (desc.fSampleCnt > 1) {
            // Worse case, we own the resolve buffer so that is one more sample per pixel.
            colorValuesPerPixel += 1;
        }
        SkASSERT(kUnknown_GrPixelConfig != desc.fConfig);
        SkASSERT(!GrPixelConfigIsCompressed(desc.fConfig));
        size_t colorBytes = (size_t) width * height * GrBytesPerPixel(desc.fConfig);

        // This would be a nice assert to have (i.e., we aren't creating 0 width/height surfaces).
        // Unfortunately Chromium seems to want to do this.
        //SkASSERT(colorBytes > 0);

        size = colorValuesPerPixel * colorBytes;
        size += colorBytes/3; // in case we have to mipmap
    } else {
        if (GrPixelConfigIsCompressed(desc.fConfig)) {
            size = GrCompressedFormatDataSize(desc.fConfig, width, height);
        } else {
            size = (size_t)width * height * GrBytesPerPixel(desc.fConfig);
        }

        size += size/3;  // in case we have to mipmap
    }

    return size;
}

size_t GrSurface::ComputeSize(GrPixelConfig config,
                              int width,
                              int height,
                              int colorSamplesPerPixel,
                              GrMipMapped mipMapped,
                              bool binSize) {
    size_t colorSize;

    width  = binSize ? GrResourceProvider::MakeApprox(width)  : width;
    height = binSize ? GrResourceProvider::MakeApprox(height) : height;

    SkASSERT(kUnknown_GrPixelConfig != config);
    if (GrPixelConfigIsCompressed(config)) {
        colorSize = GrCompressedFormatDataSize(config, width, height);
    } else {
        colorSize = (size_t)width * height * GrBytesPerPixel(config);
    }
    SkASSERT(colorSize > 0);

    size_t finalSize = colorSamplesPerPixel * colorSize;

    if (GrMipMapped::kYes == mipMapped) {
        // We don't have to worry about the mipmaps being a different size than
        // we'd expect because we never change fDesc.fWidth/fHeight.
        finalSize += colorSize/3;
    }
    return finalSize;
}

//////////////////////////////////////////////////////////////////////////////

bool GrSurface::hasPendingRead() const {
    const GrTexture* thisTex = this->asTexture();
    if (thisTex && thisTex->internalHasPendingRead()) {
        return true;
    }
    const GrRenderTarget* thisRT = this->asRenderTarget();
    if (thisRT && thisRT->internalHasPendingRead()) {
        return true;
    }
    return false;
}

bool GrSurface::hasPendingWrite() const {
    const GrTexture* thisTex = this->asTexture();
    if (thisTex && thisTex->internalHasPendingWrite()) {
        return true;
    }
    const GrRenderTarget* thisRT = this->asRenderTarget();
    if (thisRT && thisRT->internalHasPendingWrite()) {
        return true;
    }
    return false;
}

bool GrSurface::hasPendingIO() const {
    const GrTexture* thisTex = this->asTexture();
    if (thisTex && thisTex->internalHasPendingIO()) {
        return true;
    }
    const GrRenderTarget* thisRT = this->asRenderTarget();
    if (thisRT && thisRT->internalHasPendingIO()) {
        return true;
    }
    return false;
}

void GrSurface::onRelease() {
    this->invokeReleaseProc();
    this->INHERITED::onRelease();
}

void GrSurface::onAbandon() {
    this->invokeReleaseProc();
    this->INHERITED::onAbandon();
}
