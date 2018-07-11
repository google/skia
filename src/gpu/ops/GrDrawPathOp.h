/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawPathOp_DEFINED
#define GrDrawPathOp_DEFINED

#include "GrDrawOp.h"
#include "GrOpFlushState.h"
#include "GrPath.h"
#include "GrPathProcessor.h"
#include "GrPathRendering.h"
#include "GrProcessorSet.h"
#include "GrStencilSettings.h"

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
    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        return this->doProcessorAnalysis(caps, clip).requiresDstTexture()
                ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
    }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessorSet.visitProxies(func);
    }

protected:
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    GrColor color() const { return fInputColor; }
    GrPathRendering::FillType fillType() const { return fFillType; }
    const GrProcessorSet& processors() const { return fProcessorSet; }
    GrProcessorSet detachProcessors() { return std::move(fProcessorSet); }
    inline GrPipeline::InitArgs pipelineInitArgs(const GrOpFlushState&);
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
    GrProcessorSet::Analysis fAnalysis;
    GrPathRendering::FillType fFillType;
    GrAAType fAAType;
    GrProcessorSet fProcessorSet;

    typedef GrDrawOp INHERITED;
};

class GrDrawPathOp final : public GrDrawPathOpBase {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext*,
                                          const SkMatrix& viewMatrix,
                                          GrPaint&&,
                                          GrAAType,
                                          GrPath*);

    const char* name() const override { return "DrawPath"; }

    SkString dumpInfo() const override;

private:
    friend class GrOpMemoryPool; // for ctor

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

#endif
