/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/BackendTextureImageFactory.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "tools/gpu/ManagedBackendTexture.h"

#ifdef SK_GANESH
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#endif

#ifdef SK_GRAPHITE
#include "include/core/SkBitmap.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"
#endif

namespace sk_gpu_test {
#ifdef SK_GANESH
sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext* dContext,
                                       const SkPixmap& pixmap,
                                       Renderable renderable,
                                       GrSurfaceOrigin origin,
                                       Protected isProtected) {
    auto mbet = ManagedBackendTexture::MakeWithData(dContext,
                                                    pixmap,
                                                    origin,
                                                    renderable,
                                                    isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkImages::BorrowTextureFrom(dContext,
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
                                       Mipmapped mipmapped,
                                       Renderable renderable,
                                       GrSurfaceOrigin origin,
                                       Protected isProtected) {
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
                                                    isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkImages::BorrowTextureFrom(dContext,
                                       mbet->texture(),
                                       origin,
                                       info.colorType(),
                                       info.alphaType(),
                                       info.refColorSpace(),
                                       ManagedBackendTexture::ReleaseProc,
                                       mbet->releaseContext());
}
#endif  // SK_GANESH

#ifdef SK_GRAPHITE
using Recorder = skgpu::graphite::Recorder;
sk_sp<SkImage> MakeBackendTextureImage(Recorder* recorder,
                                       const SkPixmap& pixmap,
                                       skgpu::Mipmapped isMipmapped,
                                       Renderable isRenderable,
                                       Origin origin,
                                       Protected isProtected) {
    auto mbet = ManagedGraphiteTexture::MakeFromPixmap(recorder,
                                                       pixmap,
                                                       isMipmapped,
                                                       isRenderable,
                                                       isProtected);
    if (!mbet) {
        return nullptr;
    }

    // TODO(b/476410476): The new WrapTexture API that does not take a colortype currently has no
    // way to be interpreted as gray. This is handled internally in the old WrapTexture call by
    // choosing an RRR1 swizzle. If we allow wrapping backend textures with swizzles, that will
    // be sufficient and we can remove the old API. Otherwise we will either remove kGray_8 or
    // expand the new API to augment the alpha-type with a semantic (red-as-gray, red-as-alpha,
    // alpha-only, force-opaque, etc.)
    if (pixmap.colorType() == kGray_8_SkColorType) {
        return SkImages::WrapTexture(recorder,
                                     mbet->texture(),
                                     pixmap.colorType(),
                                     pixmap.alphaType(),
                                     pixmap.refColorSpace(),
                                     origin,
                                     sk_gpu_test::ManagedGraphiteTexture::ImageReleaseProc,
                                     mbet->releaseContext());
    } else {
        return SkImages::WrapTexture(recorder,
                                    mbet->texture(),
                                    pixmap.alphaType(),
                                    pixmap.refColorSpace(),
                                    origin,
                                    sk_gpu_test::ManagedGraphiteTexture::ImageReleaseProc,
                                    mbet->releaseContext());
    }
}

sk_sp<SkImage> MakeBackendTextureImage(Recorder* recorder,
                                       const SkImageInfo& ii,
                                       SkColor4f color,
                                       skgpu::Mipmapped isMipmapped,
                                       Renderable isRenderable,
                                       Origin origin,
                                       Protected isProtected) {
    if (ii.alphaType() == kOpaque_SkAlphaType) {
        color = color.makeOpaque();
    }

    SkBitmap bitmap;
    bitmap.allocPixels(ii);

    bitmap.eraseColor(color);

    return MakeBackendTextureImage(recorder, bitmap.pixmap(), isMipmapped, isRenderable,
                                   origin, isProtected);
}
#endif  // SK_GRAPHITE

}  // namespace sk_gpu_test
