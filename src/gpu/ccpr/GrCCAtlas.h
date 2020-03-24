/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCAtlas_DEFINED
#define GrCCAtlas_DEFINED

#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrTAllocator.h"
#include "src/gpu/ccpr/GrCCPathProcessor.h"

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

    static constexpr InternalMultisample CoverageTypeHasInternalMultisample(
            CoverageType coverageType) {
        switch (coverageType) {
            case CoverageType::kFP16_CoverageCount:
            case CoverageType::kA8_LiteralCoverage:
                return InternalMultisample::kNo;
            case CoverageType::kA8_Multisample:
                return InternalMultisample::kYes;
        }
        SkUNREACHABLE;
    }

    static constexpr GrCCPathProcessor::CoverageMode CoverageTypeToPathCoverageMode(
            CoverageType coverageType) {
        return (GrCCAtlas::CoverageType::kFP16_CoverageCount == coverageType)
                ? GrCCPathProcessor::CoverageMode::kCoverageCount
                : GrCCPathProcessor::CoverageMode::kLiteral;
    }


    static sk_sp<GrTextureProxy> MakeLazyAtlasProxy(const LazyInstantiateAtlasCallback& callback,
                                                    CoverageType coverageType, const GrCaps& caps,
                                                    GrSurfaceProxy::UseAllocator useAllocator) {
        return GrDynamicAtlas::MakeLazyAtlasProxy(callback, CoverageTypeToColorType(coverageType),
                                                  CoverageTypeHasInternalMultisample(coverageType),
                                                  caps, useAllocator);
    }

    GrCCAtlas(CoverageType, const Specs&, const GrCaps&);
    ~GrCCAtlas() override;

    // This is an optional space for the caller to jot down user-defined instance data to use when
    // rendering atlas content.
    void setFillBatchID(int id);
    int getFillBatchID() const { return fFillBatchID; }
    void setStrokeBatchID(int id);
    int getStrokeBatchID() const { return fStrokeBatchID; }
    void setEndStencilResolveInstance(int idx);
    int getEndStencilResolveInstance() const { return fEndStencilResolveInstance; }

    sk_sp<GrCCCachedAtlas> refOrMakeCachedAtlas(GrOnFlushResourceProvider*);

private:
    const CoverageType fCoverageType;
    int fFillBatchID;
    int fStrokeBatchID;
    int fEndStencilResolveInstance;
    sk_sp<GrCCCachedAtlas> fCachedAtlas;
};

/**
 * This class implements an unbounded stack of atlases. When the current atlas reaches the
 * implementation-dependent max texture size, a new one is pushed to the back and we continue on.
 */
class GrCCAtlasStack {
public:
    using CoverageType = GrCCAtlas::CoverageType;
    using CCAtlasAllocator = GrTAllocator<GrCCAtlas, 4>;

    GrCCAtlasStack(CoverageType coverageType, const GrCCAtlas::Specs& specs, const GrCaps* caps)
            : fCoverageType(coverageType), fSpecs(specs), fCaps(caps) {}

    CoverageType coverageType() const { return fCoverageType; }
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
    const CoverageType fCoverageType;
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
