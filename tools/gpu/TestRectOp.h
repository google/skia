/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestRectOp_DEFINED
#define TestRectOp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

class GrPaint;

namespace sk_gpu_test {

/**
 * Test Op that draw a rectangle with local coords and a local matrix with a single color fragment
 * processor. It is important to test effects in the presence of GP local matrices. Our standard
 * rect drawing code doesn't exercise this.
 */
class TestRectOp final : public GrMeshDrawOp {
public:
    /** Full specified device space rect op. */
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext*, GrPaint&&, const SkRect& drawRect, const SkRect& localRect, const SkMatrix& localM = SkMatrix::I());

    /** Takes a single color FP instead of a full paint. Uses SkBlendMode::kSrcOver. */
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext*, std::unique_ptr<GrFragmentProcessor>, const SkRect& drawRect, const SkRect& localRect, const SkMatrix& localM = SkMatrix::I());

    /**
     * Uses the rect as the device space rect to draw as well as the local rect. The local matrix
     * is identity.
     */
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext*, GrPaint&&, const SkRect& rect);

    const char* name() const override { return "TestRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, bool hasMixedSampledCoverage,
            GrClampType) override;

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessorSet.visitProxies(func);
    }

private:
    DEFINE_OP_CLASS_ID

    TestRectOp(const GrCaps*, GrPaint&&, const SkRect& drawRect, const SkRect& localRect, const SkMatrix& localMatrix);
    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    SkRect fDrawRect;
    SkRect fLocalRect;
    SkMatrix fLocalMatrix;
    SkPMColor4f fColor;
    bool fWideColor;
    sk_sp<const GrGeometryProcessor> fGeometryProcessor;
    GrProcessorSet fProcessorSet;

    friend class ::GrOpMemoryPool;
};

}  // namespace sk_gpu_test

#endif
