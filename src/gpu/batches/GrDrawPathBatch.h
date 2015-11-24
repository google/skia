/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawPathBatch_DEFINED
#define GrDrawPathBatch_DEFINED

#include "GrBatchFlushState.h"
#include "GrDrawBatch.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRendering.h"
#include "GrPathProcessor.h"

#include "SkTLList.h"

class GrDrawPathBatchBase : public GrDrawBatch {
public:
    void computePipelineOptimizations(GrInitInvariantOutput* color, 
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setKnownFourComponents(fColor);
        coverage->setKnownSingleComponent(0xff);
        overrides->fUsePLSDstRead = false;
    }

    void setStencilSettings(const GrStencilSettings& stencil) { fStencilSettings = stencil; }

protected:
    GrDrawPathBatchBase(uint32_t classID, const SkMatrix& viewMatrix, GrColor initialColor)
        : INHERITED(classID)
        , fViewMatrix(viewMatrix)
        , fColor(initialColor) {}

    const GrStencilSettings& stencilSettings() const { return fStencilSettings; }
    const GrXPOverridesForBatch& overrides() const { return fOverrides; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    GrColor color() const { return fColor; }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fColor);
        fOverrides = overrides;
    }

    SkMatrix                                                fViewMatrix;
    GrColor                                                 fColor;
    GrStencilSettings                                       fStencilSettings;
    GrXPOverridesForBatch                                   fOverrides;

    typedef GrDrawBatch INHERITED;
};

class GrDrawPathBatch final : public GrDrawPathBatchBase {
public:
    DEFINE_BATCH_CLASS_ID

    // This can't return a more abstract type because we install the stencil settings late :(
    static GrDrawPathBatchBase* Create(const SkMatrix& viewMatrix, GrColor color,
                                       const GrPath* path) {
        return new GrDrawPathBatch(viewMatrix, color, path);
    }

    const char* name() const override { return "DrawPath"; }

    SkString dumpInfo() const override;

private:
    GrDrawPathBatch(const SkMatrix& viewMatrix, GrColor color, const GrPath* path)
        : INHERITED(ClassID(), viewMatrix, color)
        , fPath(path) {
        fBounds = path->getBounds();
        viewMatrix.mapRect(&fBounds);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override;

    GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;

    typedef GrDrawPathBatchBase INHERITED;
};

/**
 * This could be nested inside the batch class, but for now it must be declarable in a public
 * header (GrDrawContext)
 */
class GrPathRangeDraw : public GrNonAtomicRef {
public:
    typedef GrPathRendering::PathTransformType TransformType;

    static GrPathRangeDraw* Create(TransformType transformType, int reserveCnt) {
        return new GrPathRangeDraw(transformType, reserveCnt);
    }

    void append(uint16_t index, float transform[]) {
        fTransforms.push_back_n(GrPathRendering::PathTransformSize(fTransformType), transform);
        fIndices.push_back(index);
    }

    int count() const { return fIndices.count(); }

    TransformType transformType() const { return fTransformType; }

    const float* transforms() const { return fTransforms.begin(); }

    const uint16_t* indices() const { return fIndices.begin(); }

    static bool CanMerge(const GrPathRangeDraw& a, const GrPathRangeDraw& b) {
        return a.transformType() == b.transformType();
    }

private:
    GrPathRangeDraw(TransformType transformType, int reserveCnt)
        : fTransformType(transformType)
        , fIndices(reserveCnt)
        , fTransforms(reserveCnt * GrPathRendering::PathTransformSize(transformType)) {
        SkDEBUGCODE(fUsedInBatch = false;)
    }

    // Reserve space for 64 paths where indices are 16 bit and transforms are translations.
    static const int kIndexReserveCnt = 64;
    static const int kTransformBufferReserveCnt = 2 * 64;

    GrPathRendering::PathTransformType                     fTransformType;
    SkSTArray<kIndexReserveCnt, uint16_t, true>            fIndices;
    SkSTArray<kTransformBufferReserveCnt, float, true>     fTransforms;

    // To ensure we don't reuse these across batches.
#ifdef SK_DEBUG
    bool fUsedInBatch;
    friend class GrDrawPathRangeBatch;
#endif

    typedef GrNonAtomicRef INHERITED;
};

// Template this if we decide to support index types other than 16bit
class GrDrawPathRangeBatch final : public GrDrawPathBatchBase {
public:
    DEFINE_BATCH_CLASS_ID

    // This can't return a more abstract type because we install the stencil settings late :(
    static GrDrawPathBatchBase* Create(const SkMatrix& viewMatrix, const SkMatrix& localMatrix,
                                       GrColor color, GrPathRange* range, GrPathRangeDraw* draw,
                                       const SkRect& bounds) {
        return new GrDrawPathRangeBatch(viewMatrix, localMatrix, color, range, draw, bounds);
    }

    ~GrDrawPathRangeBatch() override;

    const char* name() const override { return "DrawPathRange"; }

    SkString dumpInfo() const override;

private:
    inline bool isWinding() const;

    GrDrawPathRangeBatch(const SkMatrix& viewMatrix, const SkMatrix& localMatrix, GrColor color,
                         GrPathRange* range, GrPathRangeDraw* draw, const SkRect& bounds);

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override;

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override;

    typedef GrPendingIOResource<const GrPathRange, kRead_GrIOType> PendingPathRange;
    typedef SkTLList<GrPathRangeDraw*, 4> DrawList;
    PendingPathRange    fPathRange;
    DrawList            fDraws;
    int                 fTotalPathCount;
    SkMatrix            fLocalMatrix;

    typedef GrDrawPathBatchBase INHERITED;
};

#endif
