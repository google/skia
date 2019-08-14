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
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/ccpr/GrCCPerFlushResources.h"

void GrCCClipPath::init(
        const SkPath& deviceSpacePath, const SkIRect& accessRect,
        GrCCAtlas::CoverageType atlasCoverageType, const GrCaps& caps) {
    SkASSERT(!this->isInitialized());

    fAtlasLazyProxy = GrCCAtlas::MakeLazyAtlasProxy(
            [this](GrResourceProvider* resourceProvider, GrPixelConfig,
                   const GrBackendFormat& format, int sampleCount) {
                SkASSERT(fHasAtlas);
                SkASSERT(!fHasAtlasTransform);

                GrTextureProxy* textureProxy = fAtlas ? fAtlas->textureProxy() : nullptr;

                if (!textureProxy || !textureProxy->instantiate(resourceProvider)) {
                    fAtlasScale = fAtlasTranslate = {0, 0};
                    SkDEBUGCODE(fHasAtlasTransform = true);
                    return sk_sp<GrTexture>();
                }

                sk_sp<GrTexture> texture = sk_ref_sp(textureProxy->peekTexture());
                SkASSERT(texture);
                SkASSERT(texture->backendFormat() == format);
                SkASSERT(texture->asRenderTarget()->numSamples() == sampleCount);
                SkASSERT(textureProxy->origin() == kTopLeft_GrSurfaceOrigin);

                fAtlasScale = {1.f / texture->width(), 1.f / texture->height()};
                fAtlasTranslate.set(fDevToAtlasOffset.fX * fAtlasScale.x(),
                                    fDevToAtlasOffset.fY * fAtlasScale.y());
                SkDEBUGCODE(fHasAtlasTransform = true);

                return texture;
            },
            atlasCoverageType, caps);

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
    fAtlas = resources->renderDeviceSpacePathInAtlas(
            fAccessRect, fDeviceSpacePath, fPathDevIBounds, GrFillRuleForSkPath(fDeviceSpacePath),
            &fDevToAtlasOffset);
    SkDEBUGCODE(fHasAtlas = true);
}
