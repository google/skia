/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCClipPath_DEFINED
#define GrCCClipPath_DEFINED

#include "include/core/SkPath.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/ccpr/GrCCAtlas.h"

struct GrCCPerFlushResourceSpecs;
class GrCCPerFlushResources;
class GrOnFlushResourceProvider;
class GrProxyProvider;

/**
 * These are keyed by SkPath generation ID, and store which device-space paths are accessed and
 * where by clip FPs in a given opsTask. A single GrCCClipPath can be referenced by multiple FPs. At
 * flush time their coverage count masks are packed into atlas(es) alongside normal DrawPathOps.
 */
class GrCCClipPath {
public:
    GrCCClipPath() = default;
    GrCCClipPath(const GrCCClipPath&) = delete;

    ~GrCCClipPath() {
        // Ensure no clip FP exists with a dangling pointer back into this class. This works because
        // a clip FP will have a ref on the proxy if it exists.
        //
        // This assert also guarantees there won't be a lazy proxy callback with a dangling pointer
        // back into this class, since no proxy will exist after we destruct, if the assert passes.
        SkASSERT(!fAtlasLazyProxy || fAtlasLazyProxy->unique());
    }

    bool isInitialized() const { return fAtlasLazyProxy != nullptr; }
    void init(const SkPath& deviceSpacePath,
              const SkIRect& desc,
              const GrCaps&);

    void addAccess(const SkIRect& accessRect) {
        SkASSERT(this->isInitialized());
        fAccessRect.join(accessRect);
    }
    GrTextureProxy* atlasLazyProxy() const {
        SkASSERT(this->isInitialized());
        return fAtlasLazyProxy.get();
    }
    const SkPath& deviceSpacePath() const {
        SkASSERT(this->isInitialized());
        return fDeviceSpacePath;
    }
    const SkIRect& pathDevIBounds() const {
        SkASSERT(this->isInitialized());
        return fPathDevIBounds;
    }

    void accountForOwnPath(GrCCAtlas::Specs*) const;
    void renderPathInAtlas(GrCCPerFlushResources*, GrOnFlushResourceProvider*);

    const SkIVector& atlasTranslate() const {
        SkASSERT(fHasAtlasTranslate);
        return fDevToAtlasOffset;
    }

private:
    sk_sp<GrTextureProxy> fAtlasLazyProxy;
    SkPath fDeviceSpacePath;
    SkIRect fPathDevIBounds;
    SkIRect fAccessRect;

    const GrCCAtlas* fAtlas = nullptr;
    SkIVector fDevToAtlasOffset;  // Translation from device space to location in atlas.
    SkDEBUGCODE(bool fHasAtlas = false;)
    SkDEBUGCODE(bool fHasAtlasTranslate = false;)
};

#endif
