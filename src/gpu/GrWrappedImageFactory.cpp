/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrWrappedImageFactory.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"

std::unique_ptr<GrWrappedImageFactory> GrWrappedImageFactory::Make(GrContext* context,
                                                                   const GrBackendTexture& beTex,
                                                                   SkColorType colorType,
                                                                   SkAlphaType alphaType,
                                                                   sk_sp<SkColorSpace> colorSpace,
                                                                   TextureReleaseProc releaseProc,
                                                                   ReleaseContext releaseContext) {
    return std::make_unique<GrWrappedImageFactory>(context, beTex, colorType, alphaType,
                                                   std::move(colorSpace), releaseProc,
                                                   releaseContext);
}

sk_sp<SkImage> GrWrappedImageFactory::makeImageSnapshot() {
    return fSurface->makeImageSnapshot();
}

GrWrappedImageFactory::GrWrappedImageFactory(GrContext* context,
                                             const GrBackendTexture& beTex,
                                             SkColorType colorType,
                                             SkAlphaType alphaType,
                                             sk_sp<SkColorSpace> colorSpace,
                                             TextureReleaseProc releaseProc,
                                             ReleaseContext releaseContext) {
    // for now, we cheat and use an SkSurface
    fSurface = SkSurface::MakeFromBackendTexture(context, beTex, kTopLeft_GrSurfaceOrigin, 1,
                                                 colorType, std::move(colorSpace),
                                                 nullptr, releaseProc, releaseContext);
}

GrWrappedImageFactory::~GrWrappedImageFactory() {}
