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
    const SkPixmap* src = &pixmap;
    SkAutoPixmapStorage temp;
    if (origin == kBottomLeft_GrSurfaceOrigin) {
        temp.alloc(src->info());
        auto s = static_cast<const char*>(src->addr(0, pixmap.height() - 1));
        auto d = static_cast<char*>(temp.writable_addr(0, 0));
        for (int y = 0; y < temp.height(); ++y, s -= pixmap.rowBytes(), d += temp.rowBytes()) {
            std::copy_n(s, temp.info().minRowBytes(), d);
        }
        src = &temp;
    }
    auto mbet = ManagedBackendTexture::MakeWithData(dContext, src, 1, renderable, GrProtected::kNo);
    if (!mbet) {
        return nullptr;
    }
    return SkImage::MakeFromTexture(dContext,
                                    mbet->texture(),
                                    origin,
                                    src->colorType(),
                                    src->alphaType(),
                                    src->refColorSpace(),
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
