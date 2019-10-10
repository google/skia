/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "include/gpu/GrSurface.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfacePriv.h"

#include "src/core/SkMathPriv.h"
#include "src/gpu/SkGr.h"

size_t GrSurface::ComputeSize(GrPixelConfig config,
                              const GrCaps& caps,
                              const GrBackendFormat& format,
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
        colorSize = (size_t)width * height * caps.bytesPerPixel(format);
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

void GrSurface::onRelease() {
    this->invokeReleaseProc();
    this->INHERITED::onRelease();
}

void GrSurface::onAbandon() {
    this->invokeReleaseProc();
    this->INHERITED::onAbandon();
}
