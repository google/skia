/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawPathOp_DEFINED
#define GrDrawPathOp_DEFINED

#include "GrDrawOp.h"
#include "GrGpu.h"
#include "GrOpFlushState.h"
#include "GrPath.h"
#include "GrPathProcessor.h"
#include "GrPathRendering.h"
#include "GrProcessorSet.h"
#include "GrStencilSettings.h"

#include "SkTLList.h"

class GrPaint;

class GrDrawPathOpBase : public GrDrawOp {
protected:
    GrDrawPathOpBase(uint32_t classID, const SkMatrix& viewMatrix, GrPaint&&,
                     GrPathRendering::FillType, GrAAType);
    FixedFunctionFlags fixedFunctionFlags() const override {
        if (GrAATypeIsHW(fAAType)) {
            return FixedFunctionFlags::kUsesHWAA | FixedFunctionFlags::kUsesStencil;
        }
        return FixedFunctionFlags::kUsesStencil;
    }
    bool xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip) override {
        return this->doProcessorAnalysis(caps, clip).requiresDstTexture();
    }

protected:
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    GrColor color() const { return fInputColor; }
    GrPathRendering::FillType fillType() const { return fFillType; }
    const GrProcessorSet& processors() const { return fProcessorSet; }
    void initPipeline(const GrOpFlushState&, GrPipeline*);
    const GrProcessorSet::Analysis& doProcessorAnalysis(const GrCaps& caps,
                                                        const GrAppliedClip* clip) {
        bool isMixedSamples = GrAAType::kMixedSamples == fAAType;
        fAnalysis = fProcessorSet.finalize(fInputColor, GrProcessorAnalysisCoverage::kNone, clip,
                                           isMixedSamples, caps, &fInputColor);
        return fAnalysis;
    }
    const GrProcessorSet::Analysis& processorAnalysis() const {
        SkASSERT(fAnalysis.isInitialized());
        return fAnalysis;
    }

private:
    void onPrepare(GrOpFlushState*) final {}

    SkMatrix fViewMatrix;
    GrColor fInputColor;
    GrProcessorSet fProcessorSet;
    GrProcessorSet::Analysis fAnalysis;
    GrPathRendering::FillType fFillType;
    GrAAType fAAType;

    typedef GrDrawOp INHERITED;
};

class GrDrawPathOp final : public GrDrawPathOpBase {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(const SkMatrix& viewMatrix, GrPaint&& paint,
                                          GrAAType aaType, GrPath* path) {
        return std::unique_ptr<GrDrawOp>(
                new GrDrawPathOp(viewMatrix, std::move(paint), aaType, path));
    }

    const char* name() const override { return "DrawPath"; }

    SkString dumpInfo() const override;

private:
    GrDrawPathOp(const SkMatrix& viewMatrix, GrPaint&& paint, GrAAType aaType, const GrPath* path)
            : GrDrawPathOpBase(ClassID(), viewMatrix, std::move(paint), path->getFillType(), aaType)
            , fPath(path) {
        this->setTransformedBounds(path->getBounds(), viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    void onExecute(GrOpFlushState* state) override;

    GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;

    typedef GrDrawPathOpBase INHERITED;
};

// Template this if we decide to support index types other than 16bit
class GrDrawPathRangeOp final : public GrDrawPathOpBase {
public:
    typedef GrPathRendering::PathTransformType TransformType;

    DEFINE_OP_CLASS_ID

    struct InstanceData : private ::SkNoncopyable {
    public:
        static InstanceData* Alloc(TransformType transformType, int reserveCnt) {
            int transformSize = GrPathRendering::PathTransformSize(transformType);
            uint8_t* ptr = (uint8_t*)sk_malloc_throw(Align32(sizeof(InstanceData)) +
                                                     Align32(reserveCnt * sizeof(uint16_t)) +
                                                     reserveCnt * transformSize * sizeof(float));
            InstanceData* instanceData = (InstanceData*)ptr;
            instanceData->fIndices = (uint16_t*)&ptr[Align32(sizeof(InstanceData))];
            instanceData->fTransformValues = (float*)&ptr[Align32(sizeof(InstanceData)) +
                                                          Align32(reserveCnt * sizeof(uint16_t))];
            instanceData->fTransformType = transformType;
            instanceData->fInstanceCount = 0;
            instanceData->fRefCnt = 1;
            SkDEBUGCODE(instanceData->fReserveCnt = reserveCnt);
            return instanceData;
        }

        // Overload this method if we start using other transform types.
        void append(uint16_t index, float x, float y) {
            SkASSERT(GrPathRendering::kTranslate_PathTransformType == fTransformType);
            SkASSERT(fInstanceCount < fReserveCnt);
            fIndices[fInstanceCount] = index;
            fTransformValues[2 * fInstanceCount] = x;
            fTransformValues[2 * fInstanceCount + 1] = y;
            ++fInstanceCount;
        }

        TransformType transformType() const { return fTransformType; }
        int count() const { return fInstanceCount; }

        const uint16_t* indices() const { return fIndices; }
        uint16_t* indices() { return fIndices; }

        const float* transformValues() const { return fTransformValues; }
        float* transformValues() { return fTransformValues; }

        void ref() const { ++fRefCnt; }

        void unref() const {
            if (0 == --fRefCnt) {
                sk_free(const_cast<InstanceData*>(this));
            }
        }

    private:
        static int Align32(int sizeInBytes) { return (sizeInBytes + 3) & ~3; }

        InstanceData() {}
        ~InstanceData() {}

        uint16_t* fIndices;
        float* fTransformValues;
        TransformType fTransformType;
        int fInstanceCount;
        mutable int fRefCnt;
        SkDEBUGCODE(int fReserveCnt;)
    };

    static std::unique_ptr<GrDrawOp> Make(const SkMatrix& viewMatrix, SkScalar scale, SkScalar x,
                                          SkScalar y, GrPaint&& paint,
                                          GrPathRendering::FillType fill, GrAAType aaType,
                                          GrPathRange* range, const InstanceData* instanceData,
                                          const SkRect& bounds) {
        return std::unique_ptr<GrDrawOp>(new GrDrawPathRangeOp(viewMatrix, scale, x, y,
                                                               std::move(paint), fill, aaType,
                                                               range, instanceData, bounds));
    }

    const char* name() const override { return "DrawPathRange"; }

    SkString dumpInfo() const override;

private:
    GrDrawPathRangeOp(const SkMatrix& viewMatrix, SkScalar scale, SkScalar x, SkScalar y,
                      GrPaint&& paint, GrPathRendering::FillType fill, GrAAType aaType,
                      GrPathRange* range, const InstanceData* instanceData, const SkRect& bounds);

    TransformType transformType() const { return fDraws.head()->fInstanceData->transformType(); }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override;

    void onExecute(GrOpFlushState* state) override;

    struct Draw {
        void set(const InstanceData* instanceData, SkScalar x, SkScalar y) {
            fInstanceData.reset(SkRef(instanceData));
            fX = x;
            fY = y;
        }

        sk_sp<const InstanceData> fInstanceData;
        SkScalar fX, fY;
    };

    typedef GrPendingIOResource<const GrPathRange, kRead_GrIOType> PendingPathRange;
    typedef SkTLList<Draw, 4> DrawList;

    PendingPathRange fPathRange;
    DrawList fDraws;
    int fTotalPathCount;
    SkScalar fScale;

    typedef GrDrawPathOpBase INHERITED;
};

#endif
