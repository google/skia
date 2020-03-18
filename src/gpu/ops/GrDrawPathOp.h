/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawPathOp_DEFINED
#define GrDrawPathOp_DEFINED

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPath.h"
#include "src/gpu/GrPathProcessor.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/ops/GrDrawOp.h"

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
    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        return this->doProcessorAnalysis(caps, clip, hasMixedSampledCoverage, clampType);
    }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessorSet.visitProxies(func);
    }

protected:
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkPMColor4f& color() const { return fInputColor; }
    GrPathRendering::FillType fillType() const { return fFillType; }
    bool doAA() const { return fDoAA; }
    const GrProcessorSet& processors() const { return fProcessorSet; }
    GrProcessorSet detachProcessorSet() { return std::move(fProcessorSet); }
    const GrProcessorSet::Analysis& doProcessorAnalysis(
            const GrCaps&, const GrAppliedClip*, bool hasMixedSampledCoverage, GrClampType);
    const GrProcessorSet::Analysis& processorAnalysis() const {
        SkASSERT(fAnalysis.isInitialized());
        return fAnalysis;
    }

private:
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView* outputView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) final {}

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
            GrRecordingContext*, const SkMatrix& viewMatrix, GrPaint&&, GrAA, sk_sp<const GrPath>);

    const char* name() const override { return "DrawPath"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override;
#endif

private:
    friend class GrOpMemoryPool; // for ctor

    GrDrawPathOp(const SkMatrix& viewMatrix, GrPaint&& paint, GrAA aa, sk_sp<const GrPath> path)
            : GrDrawPathOpBase(
                    ClassID(), viewMatrix, std::move(paint), path->getFillType(), aa)
            , fPath(std::move(path)) {
        this->setTransformedBounds(fPath->getBounds(), viewMatrix, HasAABloat::kNo,
                                   IsHairline::kNo);
    }

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    sk_sp<const GrPath> fPath;

    typedef GrDrawPathOpBase INHERITED;
};

#endif
