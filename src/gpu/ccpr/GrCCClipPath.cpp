/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCClipPath.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/ccpr/GrCCPerFlushResources.h"

void GrCCClipPath::init(const SkPath& deviceSpacePath, const SkIRect& accessRect, int rtWidth,
                        int rtHeight, const GrCaps& caps) {
    SkASSERT(!this->isInitialized());

    const GrBackendFormat format = caps.getBackendFormatFromColorType(GrColorType::kAlpha_F16,
                                                                      GrSRGBEncoded::kNo);

    fAtlasLazyProxy = GrProxyProvider::MakeFullyLazyProxy(
            [this](GrResourceProvider* resourceProvider)
                    -> GrSurfaceProxy::LazyInstantiationResult {
                SkASSERT(fHasAtlas);
                SkASSERT(!fHasAtlasTransform);

                GrTextureProxy* textureProxy = fAtlas ? fAtlas->textureProxy() : nullptr;

                if (!textureProxy || !textureProxy->instantiate(resourceProvider)) {
                    fAtlasScale = fAtlasTranslate = {0, 0};
                    SkDEBUGCODE(fHasAtlasTransform = true);
                    return {};
                }

                sk_sp<GrTexture> texture = sk_ref_sp(textureProxy->peekTexture());
                SkASSERT(texture);
                SkASSERT(kTopLeft_GrSurfaceOrigin == textureProxy->origin());

                fAtlasScale = {1.f / texture->width(), 1.f / texture->height()};
                fAtlasTranslate.set(fDevToAtlasOffset.fX * fAtlasScale.x(),
                                    fDevToAtlasOffset.fY * fAtlasScale.y());
                SkDEBUGCODE(fHasAtlasTransform = true);

                return std::move(texture);
            },
            format, GrProxyProvider::Renderable::kYes, kTopLeft_GrSurfaceOrigin,
            kAlpha_half_GrPixelConfig, caps);

    fDeviceSpacePath = deviceSpacePath;
    fDeviceSpacePath.getBounds().roundOut(&fPathDevIBounds);
    fAccessRect = accessRect;
}

void GrCCClipPath::accountForOwnPath(GrCCPerFlushResourceSpecs* specs) const {
    SkASSERT(this->isInitialized());

    ++specs->fNumClipPaths;
    specs->fRenderedPathStats[GrCCPerFlushResourceSpecs::kFillIdx].statPath(fDeviceSpacePath);

    SkIRect ibounds;
    if (ibounds.intersect(fAccessRect, fPathDevIBounds)) {
        specs->fRenderedAtlasSpecs.accountForSpace(ibounds.width(), ibounds.height());
    }
}

void GrCCClipPath::renderPathInAtlas(GrCCPerFlushResources* resources,
                                     GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(this->isInitialized());
    SkASSERT(!fHasAtlas);
    fAtlas = resources->renderDeviceSpacePathInAtlas(fAccessRect, fDeviceSpacePath, fPathDevIBounds,
                                                     &fDevToAtlasOffset);
    SkDEBUGCODE(fHasAtlas = true);
}
