/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCClipPath_DEFINED
#define GrCCClipPath_DEFINED

#include "include/core/SkPath.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
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
class GrCCClipPath : public SkRefCnt {
public:
    GrCCClipPath(const SkPath& deviceSpacePath, const SkIRect&, const GrCaps&);
    GrCCClipPath(const GrCCClipPath&) = delete;

    void addAccess(const SkIRect& accessRect) { fAccessRect.join(accessRect); }
    GrTextureProxy* atlasLazyProxy() const { return fAtlasLazyProxy.get(); }
    const SkPath& deviceSpacePath() const { return fDeviceSpacePath; }
    const SkIRect& pathDevIBounds() const { return fPathDevIBounds; }

    void accountForOwnPath(GrCCAtlas::Specs*) const;

    // Allocates our clip path in an atlas and records the offset.
    //
    // If the return value is non-null, it means the given path did not fit in the then-current
    // atlas, so it was retired and a new one was added to the stack. The return value is the
    // newly-retired atlas. (*NOT* the atlas this path will reside in.) The caller must call
    // assignAtlasTexture on all prior GrCCClipPaths that will use the retired atlas.
    std::unique_ptr<GrCCAtlas> renderPathInAtlas(GrCCPerFlushResources*,
                                                 GrOnFlushResourceProvider*);

    const SkIVector& atlasTranslate() const {
        SkASSERT(fHasAtlas);
        return fDevToAtlasOffset;
    }

    void assignAtlasTexture(sk_sp<GrTexture> atlasTexture) {
        fAtlasLazyProxy->priv().assign(std::move(atlasTexture));
    }

private:
    SkPath fDeviceSpacePath;
    SkIRect fPathDevIBounds;
    SkIRect fAccessRect;
    sk_sp<GrTextureProxy> fAtlasLazyProxy;

    SkIVector fDevToAtlasOffset;  // Translation from device space to location in atlas.
    SkDEBUGCODE(bool fHasAtlas = false;)
};

#endif
