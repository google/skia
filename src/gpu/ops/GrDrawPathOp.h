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
class GrRecordingContext;

class GrDrawPathOpBase : public GrDrawOp {
protected:
    GrDrawPathOpBase(uint32_t classID, const SkMatrix& viewMatrix, GrPaint&&,
                     GrPathRendering::FillType, GrAA);

    FixedFunctionFlags fixedFunctionFlags() const override {
        return (fDoAA)
                ? FixedFunctionFlags::kUsesHWAA | FixedFunctionFlags::kUsesStencil
                : FixedFunctionFlags::kUsesStencil;
    }
    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        return this->doProcessorAnalysis(caps, clip, fsaaType, clampType);
    }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        fProcessorSet.visitProxies(func);
    }

protected:
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkPMColor4f& color() const { return fInputColor; }
    GrPathRendering::FillType fillType() const { return fFillType; }
    const GrProcessorSet& processors() const { return fProcessorSet; }
    GrProcessorSet detachProcessors() { return std::move(fProcessorSet); }
    inline GrPipeline::InitArgs pipelineInitArgs(const GrOpFlushState&);
    const GrProcessorSet::Analysis& doProcessorAnalysis(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType);
    const GrProcessorSet::Analysis& processorAnalysis() const {
        SkASSERT(fAnalysis.isInitialized());
        return fAnalysis;
    }

private:
    void onPrepare(GrOpFlushState*) final {}

    SkMatrix fViewMatrix;
    SkPMColor4f fInputColor;
    GrProcessorSet::Analysis fAnalysis;
    GrPathRendering::FillType fFillType;
    bool fDoAA;
    GrProcessorSet fProcessorSet;

    typedef GrDrawOp INHERITED;
};

class GrDrawPathOp final : public GrDrawPathOpBase {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(
            GrRecordingContext*, const SkMatrix& viewMatrix, GrPaint&&, GrAA, GrPath*);

    const char* name() const override { return "DrawPath"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override;
#endif

private:
    friend class GrOpMemoryPool; // for ctor

    GrDrawPathOp(const SkMatrix& viewMatrix, GrPaint&& paint, GrAA aa, const GrPath* path)
            : GrDrawPathOpBase(
                    ClassID(), viewMatrix, std::move(paint), path->getFillType(), aa)
            , fPath(path) {
        this->setTransformedBounds(path->getBounds(), viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;

    typedef GrDrawPathOpBase INHERITED;
};

#endif
