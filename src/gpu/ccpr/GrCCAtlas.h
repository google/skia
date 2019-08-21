/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCAtlas_DEFINED
#define GrCCAtlas_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrResourceKey.h"
#include "src/gpu/GrAllocator.h"
#include "src/gpu/GrNonAtomicRef.h"

class GrCCCachedAtlas;
class GrOnFlushResourceProvider;
class GrRenderTargetContext;
class GrResourceProvider;
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
    // As long as GrSurfaceOrigin exists, we just have to decide on one for the atlas texture.
    static constexpr GrSurfaceOrigin kTextureOrigin = kTopLeft_GrSurfaceOrigin;
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

    enum class CoverageType {
        kFP16_CoverageCount,
        kA8_Multisample,
        kA8_LiteralCoverage
    };

    static constexpr GrColorType CoverageTypeToColorType(CoverageType coverageType) {
        switch (coverageType) {
            case CoverageType::kFP16_CoverageCount:
                return GrColorType::kAlpha_F16;
            case CoverageType::kA8_Multisample:
            case CoverageType::kA8_LiteralCoverage:
                return GrColorType::kAlpha_8;
        }
        SkUNREACHABLE;
    }

    using LazyInstantiateAtlasCallback = std::function<sk_sp<GrTexture>(
            GrResourceProvider*, GrPixelConfig, const GrBackendFormat&, int sampleCount)>;

    static sk_sp<GrTextureProxy> MakeLazyAtlasProxy(
            const LazyInstantiateAtlasCallback&, CoverageType, const GrCaps&);

    GrCCAtlas(CoverageType, const Specs&, const GrCaps&);
    ~GrCCAtlas();

    GrTextureProxy* textureProxy() const { return fTextureProxy.get(); }
    int currentWidth() const { return fWidth; }
    int currentHeight() const { return fHeight; }

    // Attempts to add a rect to the atlas. If successful, returns the integer offset from
    // device-space pixels where the path will be drawn, to atlas pixels where its mask resides.
    bool addRect(const SkIRect& devIBounds, SkIVector* atlasOffset);
    const SkISize& drawBounds() { return fDrawBounds; }

    // This is an optional space for the caller to jot down user-defined instance data to use when
    // rendering atlas content.
    void setFillBatchID(int id);
    int getFillBatchID() const { return fFillBatchID; }
    void setStrokeBatchID(int id);
    int getStrokeBatchID() const { return fStrokeBatchID; }
    void setEndStencilResolveInstance(int idx);
    int getEndStencilResolveInstance() const { return fEndStencilResolveInstance; }

    sk_sp<GrCCCachedAtlas> refOrMakeCachedAtlas(GrOnFlushResourceProvider*);

    // Instantiates our texture proxy for the atlas and returns a pre-cleared GrRenderTargetContext
    // that the caller may use to render the content. After this call, it is no longer valid to call
    // addRect(), setUserBatchID(), or this method again.
    //
    // 'backingTexture', if provided, is a renderable texture with which to instantiate our proxy.
    // If null then we will create a texture using the resource provider. The purpose of this param
    // is to provide a guaranteed way to recycle a stashed atlas texture from a previous flush.
    std::unique_ptr<GrRenderTargetContext> makeRenderTargetContext(
            GrOnFlushResourceProvider*, sk_sp<GrTexture> backingTexture = nullptr);

private:
    class Node;

    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const CoverageType fCoverageType;
    const int fMaxTextureSize;
    int fWidth, fHeight;
    std::unique_ptr<Node> fTopNode;
    SkISize fDrawBounds = {0, 0};

    int fFillBatchID;
    int fStrokeBatchID;
    int fEndStencilResolveInstance;

    sk_sp<GrCCCachedAtlas> fCachedAtlas;
    sk_sp<GrTextureProxy> fTextureProxy;
    sk_sp<GrTexture> fBackingTexture;
};

/**
 * This class implements an unbounded stack of atlases. When the current atlas reaches the
 * implementation-dependent max texture size, a new one is pushed to the back and we continue on.
 */
class GrCCAtlasStack {
public:
    using CoverageType = GrCCAtlas::CoverageType;

    GrCCAtlasStack(CoverageType coverageType, const GrCCAtlas::Specs& specs, const GrCaps* caps)
            : fCoverageType(coverageType), fSpecs(specs), fCaps(caps) {}

    CoverageType coverageType() const { return fCoverageType; }
    bool empty() const { return fAtlases.empty(); }
    const GrCCAtlas& front() const { SkASSERT(!this->empty()); return fAtlases.front(); }
    GrCCAtlas& front() { SkASSERT(!this->empty()); return fAtlases.front(); }
    GrCCAtlas& current() { SkASSERT(!this->empty()); return fAtlases.back(); }

    class Iter {
    public:
        Iter(GrCCAtlasStack& stack) : fImpl(&stack.fAtlases) {}
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
    // newly-retired atlas. The caller should call setUserBatchID() on the retired atlas before
    // moving on.
    GrCCAtlas* addRect(const SkIRect& devIBounds, SkIVector* devToAtlasOffset);

private:
    const CoverageType fCoverageType;
    const GrCCAtlas::Specs fSpecs;
    const GrCaps* const fCaps;
    GrSTAllocator<4, GrCCAtlas> fAtlases;
};

inline void GrCCAtlas::Specs::accountForSpace(int width, int height) {
    fMinWidth = SkTMax(width, fMinWidth);
    fMinHeight = SkTMax(height, fMinHeight);
    fApproxNumPixels += (width + kPadding) * (height + kPadding);
}

#endif
