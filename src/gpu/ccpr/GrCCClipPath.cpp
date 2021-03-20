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

GrCCClipPath::GrCCClipPath(const SkPath& deviceSpacePath, const SkIRect& accessRect,
                           const GrCaps& caps)
        : fDeviceSpacePath(deviceSpacePath)
        , fPathDevIBounds(fDeviceSpacePath.getBounds().roundOut())
        , fAccessRect(accessRect)
        , fAtlasLazyProxy(GrCCAtlas::MakeLazyAtlasProxy(
                [](GrResourceProvider*, const GrCCAtlas::LazyAtlasDesc&) {
                    // GrCCClipPaths get instantiated explicitly after the atlas is laid out. If
                    // this callback gets invoked, it means atlas proxy itself failed to
                    // instantiate.
                    return GrSurfaceProxy::LazyCallbackResult();
                }, caps, GrSurfaceProxy::UseAllocator::kYes)) {
    SkASSERT(!deviceSpacePath.isEmpty());
    SkASSERT(SkIRect::Intersects(fAccessRect, fPathDevIBounds));

}

void GrCCClipPath::accountForOwnPath(GrCCAtlas::Specs* specs) const {
    SkIRect ibounds;
    if (ibounds.intersect(fAccessRect, fPathDevIBounds)) {
        specs->accountForSpace(ibounds.width(), ibounds.height());
    }
}

std::unique_ptr<GrCCAtlas> GrCCClipPath::renderPathInAtlas(GrCCPerFlushResources* resources,
                                                           GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(!fHasAtlas);
    auto retiredAtlas = resources->renderDeviceSpacePathInAtlas(
            onFlushRP, fAccessRect, fDeviceSpacePath, fPathDevIBounds,
            GrFillRuleForSkPath(fDeviceSpacePath), &fDevToAtlasOffset);
    SkDEBUGCODE(fHasAtlas = true);
    return retiredAtlas;
}
