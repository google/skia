/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCAtlas_DEFINED
#define GrCCAtlas_DEFINED

#include "GrAllocator.h"
#include "GrNonAtomicRef.h"
#include "GrResourceKey.h"
#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "SkRefCnt.h"
#include "SkSize.h"

class GrOnFlushResourceProvider;
class GrRenderTargetContext;
class GrTextureProxy;
struct SkIPoint16;
struct SkIRect;

/**
 * This class implements a dynamic size GrRectanizer that grows until it reaches the implementation-
 * dependent max texture size. When finalized, it also creates and stores a GrTextureProxy for the
 * underlying atlas.
 */
class GrCCAtlas {
public:
    static constexpr GrSurfaceOrigin kOrigin = kTopLeft_GrSurfaceOrigin;
    static constexpr int kPadding = 1;  // Amount of padding below and to the right of each path.

    // This struct encapsulates the minimum and desired requirements for an atlas, as well as an
    // approximate number of pixels to help select a good initial size.
    struct Specs {
        int fMaxPreferredTextureSize = 0;
        int fMinTextureSize = 0;
        int fMinWidth = 0;  // If there are 100 20x10 paths, this should be 20.
        int fMinHeight = 0;  // If there are 100 20x10 paths, this should be 10.
        int fApproxNumPixels = 0;

        // Add space for a rect in the desired atlas specs.
        void accountForSpace(int width, int height);
    };

    GrCCAtlas(const Specs&);
    ~GrCCAtlas();

    // Attempts to add a rect to the atlas. If successful, returns the integer offset from
    // device-space pixels where the path will be drawn, to atlas pixels where its mask resides.
    bool addRect(const SkIRect& devIBounds, SkIVector* atlasOffset);
    const SkISize& drawBounds() { return fDrawBounds; }

    // This is a general purpose int that the client may use to store information about how to
    // render the atlas (e.g. an instance index, batch ID, etc.).
    void setRenderToken(int token) { fRenderToken = token; }
    int renderToken() const { return fRenderToken; }

    // Manages a unique resource cache key that gets assigned to the atlas texture.
    const GrUniqueKey& getOrAssignUniqueKey(GrOnFlushResourceProvider*);
    const GrUniqueKey& uniqueKey() const { return fUniqueKey; }

    // An object for simple bookkeeping on the atlas texture once it has a unique key. A client may
    // use this object to track when to purge the texture from the resource cache.
    struct CachedAtlasInfo : public GrNonAtomicRef<CachedAtlasInfo> {
        int fNumPathPixels = 0;
        int fNumInvalidatedPathPixels = 0;
        bool fIsPurgedFromResourceCache = false;
    };
    sk_sp<CachedAtlasInfo> refOrMakeCachedAtlasInfo();

    sk_sp<GrRenderTargetContext> makeClearedTextureProxy(GrOnFlushResourceProvider*, GrPixelConfig);
    GrTextureProxy* textureProxy() const { return fTextureProxy.get(); }

private:
    class Node;

    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const int fMaxTextureSize;
    int fWidth, fHeight;
    std::unique_ptr<Node> fTopNode;
    SkISize fDrawBounds = {0, 0};

    int fRenderToken;
    GrUniqueKey fUniqueKey;
    sk_sp<CachedAtlasInfo> fCachedAtlasInfo;
    sk_sp<GrTextureProxy> fTextureProxy;
};

/**
 * This class implements an unbounded stack of atlases. When the current atlas reaches the
 * implementation-dependent max texture size, a new one is pushed to the back and we continue on.
 */
class GrCCAtlasStack {
public:
    GrCCAtlasStack(const GrCCAtlas::Specs& specs) : fSpecs(specs) {}

    bool empty() const { return fAllocator.empty(); }
    const GrCCAtlas& front() const { SkASSERT(!this->empty()); return fAllocator.front(); }
    GrCCAtlas& front() { SkASSERT(!this->empty()); return fAllocator.front(); }
    GrCCAtlas& current() { SkASSERT(!this->empty()); return fAllocator.back(); }

    class Iter {
    public:
        Iter(GrCCAtlasStack& stack) : fImpl(&stack.fAllocator) {}
        bool next() { return fImpl.next(); }
        GrCCAtlas* operator->() const { return fImpl.get(); }
    private:
        typename GrTAllocator<GrCCAtlas>::Iter fImpl;
    };

    // Adds a rect to the current atlas and returns the offset from device space to atlas space.
    // Call current() to get the atlas it was added to.
    //
    // If the return value is non-null, it means the given rect did not fit in the then-current
    // atlas, so it was retired and a new one was added to the stack. The return value is the
    // newly-retired atlas. The caller should call setRenderToken() on the retired atlas before
    // moving on.
    GrCCAtlas* addRect(const SkIRect& devIBounds, SkIVector* offset);

private:
    const GrCCAtlas::Specs fSpecs;
    GrSTAllocator<4, GrCCAtlas> fAllocator;
};

inline void GrCCAtlas::Specs::accountForSpace(int width, int height) {
    fMinWidth = SkTMax(width, fMinWidth);
    fMinHeight = SkTMax(height, fMinHeight);
    fApproxNumPixels += (width + kPadding) * (height + kPadding);
}

#endif
