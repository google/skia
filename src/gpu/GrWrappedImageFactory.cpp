/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrWrappedImageFactory.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"

std::unique_ptr<GrWrappedImageFactory> GrWrappedImageFactory::Make(GrContext* context,
                                                                   const GrBackendTexture& beTex,
                                                                   GrSurfaceOrigin origin,
                                                                   SkColorType colorType,
                                                                   SkAlphaType alphaType,
                                                                   sk_sp<SkColorSpace> cs,
                                                                   TextureReleaseProc releaseProc,
                                                                   ReleaseContext releaseContext) {
    return std::unique_ptr<GrWrappedImageFactory>(new GrWrappedImageFactory(context, beTex, origin,
                                                                            colorType, alphaType,
                                                                            std::move(cs),
                                                                            releaseProc,
                                                                            releaseContext));
}

std::unique_ptr<GrWrappedImageFactory> GrWrappedImageFactory::MakeYUVA(GrContext* context,
                                                                       YUVAPlanarType planarType,
                                                                       SkYUVColorSpace yuvCS,
                                                                       GrBackendTexture beTexs[],
                                                                       GrSurfaceOrigin origin,
                                                                       sk_sp<SkColorSpace> cs,
                                                                       TextureReleaseProc releaseProc,
                                                                       ReleaseContext releaseContext) {
    return nullptr;
}

bool GrWrappedImageFactory::canWritePixels() const {
    return false;
}

bool GrWrappedImageFactory::writePixels(int plane, sk_sp<SkData> pixelData) {
    if (!fIsYUV) {
        if (!plane) {
            return false;
        }

        size_t requiredDataSize = fII.computeMinByteSize();
        if (pixelData->size() < requiredDataSize) {
            return false;
        }

        SkPixmap tmp(fII, pixelData->data(), fII.minRowBytes());

        fSurface->writePixels(tmp, 0, 0);
        return true;
    } else {
        return false;
    }

}

sk_sp<SkImage> GrWrappedImageFactory::makeImageSnapshot() {
    return fSurface->makeImageSnapshot();
}

GrWrappedImageFactory::GrWrappedImageFactory(GrContext* context,
                                             const GrBackendTexture& beTex,
                                             GrSurfaceOrigin origin,
                                             SkColorType colorType,
                                             SkAlphaType alphaType,
                                             sk_sp<SkColorSpace> cs,
                                             TextureReleaseProc releaseProc,
                                             ReleaseContext releaseContext)
        : fII(SkImageInfo::Make({ beTex.width(), beTex.height() }, colorType, alphaType, cs))
        , fIsYUV(false) {

    // for now, we cheat and use an SkSurface
    fSurface = SkSurface::MakeFromBackendTexture(context, beTex, origin, 1,
                                                 colorType, std::move(cs),
                                                 nullptr, releaseProc, releaseContext);
}

GrWrappedImageFactory::~GrWrappedImageFactory() {}
