/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrWrappedImageFactory.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"

static int num_planes_per_yuv_format(YUVAPlanarType format) {
    switch (format) {
        case YUVAPlanarType::kNV12: return 2;
    }
    SkUNREACHABLE;
}

std::unique_ptr<GrWrappedImageFactory> GrWrappedImageFactory::Make(GrContext* context,
                                                                   const GrBackendTexture& beTex,
                                                                   GrSurfaceOrigin origin,
                                                                   SkColorType colorType,
                                                                   SkAlphaType alphaType,
                                                                   sk_sp<SkColorSpace> cs,
                                                                   TextureReleaseProc releaseProc,
                                                                   ReleaseContext releaseContext) {
    SkImageInfo ii = SkImageInfo::Make({ beTex.width(), beTex.height() }, colorType, alphaType,
                                       std::move(cs));

    return std::unique_ptr<GrWrappedImageFactory>(new GrWrappedImageFactory(context, beTex, origin,
                                                                            ii, releaseProc,
                                                                            releaseContext));
}

std::unique_ptr<GrWrappedImageFactory> GrWrappedImageFactory::MakeYUVA(GrContext* context,
                                                                       YUVAPlanarType planarType,
                                                                       SkYUVColorSpace yuvCS,
                                                                       const GrBackendTexture beTexs[],
                                                                       GrSurfaceOrigin origin,
                                                                       sk_sp<SkColorSpace> cs,
                                                                       TextureReleaseProc relProc,
                                                                       ReleaseContext relContext) {
    switch (planarType) {
        case YUVAPlanarType::kNV12: {
            SkImageInfo ii = SkImageInfo::Make({ beTexs[0].width(), beTexs[0].height() },
                                               kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                               std::move(cs));

            return nullptr;
        }
    }
    SkUNREACHABLE;
}

bool GrWrappedImageFactory::canWritePixels() const {
    return false;
}

bool GrWrappedImageFactory::writePixels(int plane, sk_sp<SkData> pixelData) {
    if (fPlanarType == YUVAPlanarType::kInvalid) {
        SkASSERT(fSurfaces.size() == 1);

        if (!plane) {
            return false;
        }

        size_t requiredDataSize = fII.computeMinByteSize();
        if (pixelData->size() < requiredDataSize) {
            return false;
        }

        SkPixmap tmp(fII, pixelData->data(), fII.minRowBytes());

        fSurfaces[0]->writePixels(tmp, 0, 0);
        return true;
    } else {
        return false;
    }

}

sk_sp<SkImage> GrWrappedImageFactory::makeImageSnapshot() {
    if (fPlanarType == YUVAPlanarType::kInvalid) {
        SkASSERT(fSurfaces.size() == 1);
        return fSurfaces[0]->makeImageSnapshot();
    } else {
        switch (fPlanarType) {
            case YUVAPlanarType::kNV12: {
                return nullptr;
            }
            case YUVAPlanarType::kInvalid:
                return nullptr;
        }
        SkUNREACHABLE;
    }
}

GrWrappedImageFactory::GrWrappedImageFactory(GrContext* context,
                                             const GrBackendTexture& beTex,
                                             GrSurfaceOrigin origin,
                                             const SkImageInfo& ii,
                                             TextureReleaseProc releaseProc,
                                             ReleaseContext releaseContext)
        : fII(ii)
        , fPlanarType(YUVAPlanarType::kInvalid)
        , fYUVColorSpace(kIdentity_SkYUVColorSpace) {

    // for now, we cheat and use an SkSurface
    fSurfaces.push_back(SkSurface::MakeFromBackendTexture(context, beTex, origin, 1,
                                                          ii.colorType(), ii.refColorSpace(),
                                                          nullptr, releaseProc, releaseContext));
}

GrWrappedImageFactory::GrWrappedImageFactory(GrContext* context,
                                             YUVAPlanarType planarType,
                                             SkYUVColorSpace yuvColorSpace,
                                             const GrBackendTexture beTextures,
                                             GrSurfaceOrigin origin,
                                             const SkImageInfo& ii,
                                             TextureReleaseProc releaseProc,
                                             ReleaseContext releaseContext)
        : fII(ii)
        , fPlanarType(planarType)
        , fYUVColorSpace(yuvColorSpace) {

    // for now, we cheat and use an SkSurface
    fSurfaces.push_back(SkSurface::MakeFromBackendTexture(context, beTex, origin, 1,
                                                          colorType, std::move(cs),
                                                          nullptr, releaseProc, releaseContext));
}

GrWrappedImageFactory::~GrWrappedImageFactory() {}
