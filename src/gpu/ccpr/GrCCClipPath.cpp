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

void GrCCClipPath::init(const SkPath& deviceSpacePath, const SkIRect& accessRect,
                        const GrCaps& caps) {
    SkASSERT(!this->isInitialized());

    fAtlasLazyProxy = GrCCAtlas::MakeLazyAtlasProxy(
            [this](GrResourceProvider* resourceProvider, const GrCCAtlas::LazyAtlasDesc& desc) {
                SkASSERT(fHasAtlas);
                SkASSERT(!fHasAtlasTranslate);

                GrTextureProxy* textureProxy = fAtlas ? fAtlas->textureProxy() : nullptr;

                if (!textureProxy || !textureProxy->instantiate(resourceProvider)) {
                    SkDEBUGCODE(fHasAtlasTranslate = true);
                    return GrSurfaceProxy::LazyCallbackResult();
                }

                sk_sp<GrTexture> texture = sk_ref_sp(textureProxy->peekTexture());
                SkASSERT(texture);

                SkDEBUGCODE(fHasAtlasTranslate = true);

                // We use LazyInstantiationKeyMode::kUnsynced here because CCPR clip masks are never
                // cached, and the clip FP proxies need to ignore any unique keys that atlas
                // textures use for path mask caching.
                return GrSurfaceProxy::LazyCallbackResult(
                        std::move(texture), true,
                        GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced);
            }, caps, GrSurfaceProxy::UseAllocator::kYes);

    fDeviceSpacePath = deviceSpacePath;
    fDeviceSpacePath.getBounds().roundOut(&fPathDevIBounds);
    fAccessRect = accessRect;
}

void GrCCClipPath::accountForOwnPath(GrCCAtlas::Specs* specs) const {
    SkASSERT(this->isInitialized());

    SkIRect ibounds;
    if (ibounds.intersect(fAccessRect, fPathDevIBounds)) {
        specs->accountForSpace(ibounds.width(), ibounds.height());
    }
}

void GrCCClipPath::renderPathInAtlas(GrCCPerFlushResources* resources,
                                     GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(this->isInitialized());
    SkASSERT(!fHasAtlas);
    fAtlas = resources->renderDeviceSpacePathInAtlas(
            onFlushRP, fAccessRect, fDeviceSpacePath, fPathDevIBounds,
            GrFillRuleForSkPath(fDeviceSpacePath), &fDevToAtlasOffset);
    SkDEBUGCODE(fHasAtlas = true);
}
