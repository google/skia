/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCAtlas_DEFINED
#define GrCCAtlas_DEFINED

#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrTBlockList.h"

class GrCCCachedAtlas;

/**
 * GrDynamicAtlas with CCPR caching capabilities.
 */
class GrCCAtlas : public GrDynamicAtlas {
public:
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

    static sk_sp<GrTextureProxy> MakeLazyAtlasProxy(LazyInstantiateAtlasCallback&& callback,
                                                    const GrCaps& caps,
                                                    GrSurfaceProxy::UseAllocator useAllocator) {
        return GrDynamicAtlas::MakeLazyAtlasProxy(std::move(callback),
                                                  GrColorType::kAlpha_8,
                                                  InternalMultisample::kYes,
                                                  caps,
                                                  useAllocator);
    }

    GrCCAtlas(const Specs&, const GrCaps&);
    ~GrCCAtlas() override;

    // This is an optional space for the caller to jot down user-defined instance data to use when
    // rendering atlas content.
    void setFillBatchID(int id);
    int getFillBatchID() const { return fFillBatchID; }
    void setEndStencilResolveInstance(int idx);
    int getEndStencilResolveInstance() const { return fEndStencilResolveInstance; }

private:
    int fFillBatchID;
    int fEndStencilResolveInstance;
};

/**
 * This class implements an unbounded stack of atlases. When the current atlas reaches the
 * implementation-dependent max texture size, a new one is pushed to the back and we continue on.
 */
class GrCCAtlasStack {
public:
    using CCAtlasAllocator = GrTBlockList<GrCCAtlas, 4>;

    GrCCAtlasStack(const GrCCAtlas::Specs& specs, const GrCaps* caps)
            : fSpecs(specs), fCaps(caps) {}

    bool empty() const { return fAtlases.empty(); }
    const GrCCAtlas& front() const { SkASSERT(!this->empty()); return fAtlases.front(); }
    GrCCAtlas& front() { SkASSERT(!this->empty()); return fAtlases.front(); }
    GrCCAtlas& current() { SkASSERT(!this->empty()); return fAtlases.back(); }

    CCAtlasAllocator::Iter atlases() { return fAtlases.items(); }
    CCAtlasAllocator::CIter atlases() const { return fAtlases.items(); }

    // Adds a rect to the current atlas and returns the offset from device space to atlas space.
    // Call current() to get the atlas it was added to.
    //
    // If the return value is non-null, it means the given rect did not fit in the then-current
    // atlas, so it was retired and a new one was added to the stack. The return value is the
    // newly-retired atlas. The caller should call setUserBatchID() on the retired atlas before
    // moving on.
    GrCCAtlas* addRect(const SkIRect& devIBounds, SkIVector* devToAtlasOffset);

private:
    const GrCCAtlas::Specs fSpecs;
    const GrCaps* const fCaps;
    CCAtlasAllocator fAtlases;
};

inline void GrCCAtlas::Specs::accountForSpace(int width, int height) {
    fMinWidth = std::max(width, fMinWidth);
    fMinHeight = std::max(height, fMinHeight);
    fApproxNumPixels += (width + kPadding) * (height + kPadding);
}

#endif
