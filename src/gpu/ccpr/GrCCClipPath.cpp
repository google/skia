/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCClipPath.h"

#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/ccpr/GrCCPerFlushResources.h"

void GrCCClipPath::init(
        const SkPath& deviceSpacePath, const SkIRect& accessRect,
        GrCCAtlas::CoverageType atlasCoverageType, const GrCaps& caps) {
    SkASSERT(!this->isInitialized());

    fAtlasLazyProxy = GrCCAtlas::MakeLazyAtlasProxy(
            [this](GrResourceProvider* resourceProvider, const GrBackendFormat& format,
                   int sampleCount) {
                SkASSERT(fHasAtlas);
                SkASSERT(!fHasAtlasTransform);

                GrTextureProxy* textureProxy = fAtlas ? fAtlas->textureProxy() : nullptr;

                if (!textureProxy || !textureProxy->instantiate(resourceProvider)) {
                    fAtlasScale = fAtlasTranslate = {0, 0};
                    SkDEBUGCODE(fHasAtlasTransform = true);
                    return GrSurfaceProxy::LazyCallbackResult();
                }

                sk_sp<GrTexture> texture = sk_ref_sp(textureProxy->peekTexture());
                SkASSERT(texture);
                SkASSERT(texture->backendFormat() == format);
                SkASSERT(texture->asRenderTarget()->numSamples() == sampleCount);

                fAtlasScale = {1.f / texture->width(), 1.f / texture->height()};
                fAtlasTranslate.set(fDevToAtlasOffset.fX * fAtlasScale.x(),
                                    fDevToAtlasOffset.fY * fAtlasScale.y());
                SkDEBUGCODE(fHasAtlasTransform = true);

                // We use LazyInstantiationKeyMode::kUnsynced here because CCPR clip masks are never
                // cached, and the clip FP proxies need to ignore any unique keys that atlas
                // textures use for path mask caching.
                return GrSurfaceProxy::LazyCallbackResult(
                        std::move(texture), true,
                        GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced);
            },
            atlasCoverageType, caps, GrSurfaceProxy::UseAllocator::kYes);

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
