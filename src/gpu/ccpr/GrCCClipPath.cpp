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
    SkASSERT(!deviceSpacePath.isEmpty());
    SkASSERT(!this->isInitialized());

    fAtlasLazyProxy = GrCCAtlas::MakeLazyAtlasProxy(
            [](GrResourceProvider*, const GrCCAtlas::LazyAtlasDesc&) {
                // GrCCClipPaths get instantiated explicitly after the atlas is laid out. If this
                // callback gets invoked, it means atlas proxy itself failed to instantiate.
                return GrSurfaceProxy::LazyCallbackResult();
            }, caps, GrSurfaceProxy::UseAllocator::kYes);

    fDeviceSpacePath = deviceSpacePath;
    fDeviceSpacePath.getBounds().roundOut(&fPathDevIBounds);
    fAccessRect = accessRect;
    SkASSERT(SkIRect::Intersects(fAccessRect, fPathDevIBounds));
}

void GrCCClipPath::accountForOwnPath(GrCCAtlas::Specs* specs) const {
    SkASSERT(this->isInitialized());

    SkIRect ibounds;
    if (ibounds.intersect(fAccessRect, fPathDevIBounds)) {
        specs->accountForSpace(ibounds.width(), ibounds.height());
    }
}

const GrCCAtlas* GrCCClipPath::renderPathInAtlas(GrCCPerFlushResources* resources,
                                                 GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(this->isInitialized());
    SkASSERT(!fHasAtlas);
    const GrCCAtlas* retiredAtlas = resources->renderDeviceSpacePathInAtlas(
            onFlushRP, fAccessRect, fDeviceSpacePath, fPathDevIBounds,
            GrFillRuleForSkPath(fDeviceSpacePath), &fDevToAtlasOffset);
    SkDEBUGCODE(fHasAtlas = true);
    return retiredAtlas;
}
