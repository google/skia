/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCClipPath.h"

#include "GrOnFlushResourceProvider.h"
#include "GrProxyProvider.h"
#include "GrTexture.h"
#include "ccpr/GrCCPerFlushResources.h"

void GrCCClipPath::init(GrProxyProvider* proxyProvider,
                        const SkPath& deviceSpacePath, const SkIRect& accessRect,
                        int rtWidth, int rtHeight) {
    SkASSERT(!this->isInitialized());

    fAtlasLazyProxy = proxyProvider->createFullyLazyProxy(
            [this](GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrTexture>();
                }
                SkASSERT(fHasAtlas);
                SkASSERT(!fHasAtlasTransform);

                GrTextureProxy* textureProxy = fAtlas ? fAtlas->textureProxy() : nullptr;
                if (!textureProxy || !textureProxy->instantiate(resourceProvider)) {
                    fAtlasScale = fAtlasTranslate = {0, 0};
                    SkDEBUGCODE(fHasAtlasTransform = true);
                    return sk_sp<GrTexture>();
                }

                SkASSERT(kTopLeft_GrSurfaceOrigin == textureProxy->origin());

                fAtlasScale = {1.f / textureProxy->width(), 1.f / textureProxy->height()};
                fAtlasTranslate = {fAtlasOffsetX * fAtlasScale.x(),
                                   fAtlasOffsetY * fAtlasScale.y()};
                SkDEBUGCODE(fHasAtlasTransform = true);

                return sk_ref_sp(textureProxy->priv().peekTexture());
            },
            GrProxyProvider::Renderable::kYes, kTopLeft_GrSurfaceOrigin, kAlpha_half_GrPixelConfig);

    fDeviceSpacePath = deviceSpacePath;
    fDeviceSpacePath.getBounds().roundOut(&fPathDevIBounds);
    fAccessRect = accessRect;
}

void GrCCClipPath::placePathInAtlas(GrCCPerFlushResources* resources,
                                    GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(this->isInitialized());
    SkASSERT(!fHasAtlas);
    fAtlas = resources->addDeviceSpacePathToAtlas(*onFlushRP->caps(), fAccessRect, fDeviceSpacePath,
                                                  fPathDevIBounds, &fAtlasOffsetX, &fAtlasOffsetY);
    SkDEBUGCODE(fHasAtlas = true);
}
