/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/BackendTextureImageFactory.h"

#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "tools/gpu/ManagedBackendTexture.h"

namespace sk_gpu_test {
sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext* dContext,
                                       const SkPixmap& pixmap,
                                       GrRenderable renderable,
                                       GrSurfaceOrigin origin) {
    auto mbet = ManagedBackendTexture::MakeWithData(dContext,
                                                    pixmap,
                                                    origin,
                                                    renderable,
                                                    GrProtected::kNo);
    if (!mbet) {
        return nullptr;
    }
    return SkImage::MakeFromTexture(dContext,
                                    mbet->texture(),
                                    origin,
                                    pixmap.colorType(),
                                    pixmap.alphaType(),
                                    pixmap.refColorSpace(),
                                    ManagedBackendTexture::ReleaseProc,
                                    mbet->releaseContext());
}

sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext* dContext,
                                       const SkImageInfo& info,
                                       SkColor4f color,
                                       GrMipmapped mipmapped,
                                       GrRenderable renderable,
                                       GrSurfaceOrigin origin) {
    if (info.alphaType() == kOpaque_SkAlphaType) {
        color = color.makeOpaque();
    } else if (info.alphaType() == kPremul_SkAlphaType) {
        auto pmColor = color.premul();
        color = {pmColor.fR, pmColor.fG, pmColor.fB, pmColor.fA};
    }
    auto mbet = ManagedBackendTexture::MakeWithData(dContext,
                                                    info.width(),
                                                    info.height(),
                                                    info.colorType(),
                                                    color,
                                                    mipmapped,
                                                    renderable,
                                                    GrProtected::kNo);
    if (!mbet) {
        return nullptr;
    }
    return SkImage::MakeFromTexture(dContext,
                                    mbet->texture(),
                                    origin,
                                    info.colorType(),
                                    info.alphaType(),
                                    info.refColorSpace(),
                                    ManagedBackendTexture::ReleaseProc,
                                    mbet->releaseContext());
}

}  // namespace sk_gpu_test
