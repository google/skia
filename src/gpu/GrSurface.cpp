/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurface.h"
#include "GrContext.h"
#include "GrOpList.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"

#include "SkGr.h"
#include "SkMathPriv.h"

size_t GrSurface::WorstCaseSize(const GrSurfaceDesc& desc, bool useNextPow2) {
    size_t size;

    int width = useNextPow2
                ? SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(desc.fWidth))
                : desc.fWidth;
    int height = useNextPow2
                ? SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(desc.fHeight))
                : desc.fHeight;

    bool isRenderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (isRenderTarget) {
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
                              bool useNextPow2) {
    size_t colorSize;

    width = useNextPow2
            ? SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(width))
            : width;
    height = useNextPow2
            ? SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(height))
            : height;

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

template<typename T> static bool adjust_params(int surfaceWidth,
                                               int surfaceHeight,
                                               size_t bpp,
                                               int* left, int* top, int* width, int* height,
                                               T** data,
                                               size_t* rowBytes) {
    if (!*rowBytes) {
        *rowBytes = *width * bpp;
    }

    SkIRect subRect = SkIRect::MakeXYWH(*left, *top, *width, *height);
    SkIRect bounds = SkIRect::MakeWH(surfaceWidth, surfaceHeight);

    if (!subRect.intersect(bounds)) {
        return false;
    }
    *data = reinterpret_cast<void*>(reinterpret_cast<intptr_t>(*data) +
            (subRect.fTop - *top) * *rowBytes + (subRect.fLeft - *left) * bpp);

    *left = subRect.fLeft;
    *top = subRect.fTop;
    *width = subRect.width();
    *height = subRect.height();
    return true;
}

bool GrSurfacePriv::AdjustReadPixelParams(int surfaceWidth,
                                          int surfaceHeight,
                                          size_t bpp,
                                          int* left, int* top, int* width, int* height,
                                          void** data,
                                          size_t* rowBytes) {
    return adjust_params<void>(surfaceWidth, surfaceHeight, bpp, left, top, width, height, data,
                               rowBytes);
}

bool GrSurfacePriv::AdjustWritePixelParams(int surfaceWidth,
                                           int surfaceHeight,
                                           size_t bpp,
                                           int* left, int* top, int* width, int* height,
                                           const void** data,
                                           size_t* rowBytes) {
    return adjust_params<const void>(surfaceWidth, surfaceHeight, bpp, left, top, width, height,
                                     data, rowBytes);
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
