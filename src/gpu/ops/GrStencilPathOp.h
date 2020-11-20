/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilPathOp_DEFINED
#define GrStencilPathOp_DEFINED

#include "src/gpu/GrPath.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrScissorState.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/ops/GrOp.h"

class GrOpFlushState;
class GrRecordingContext;

class GrStencilPathOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            const SkMatrix& viewMatrix,
                            bool useHWAA,
                            bool hasStencilClip,
                            const GrScissorState& scissor,
                            sk_sp<const GrPath> path);

    const char* name() const override { return "StencilPathOp"; }

private:
    friend class GrOp; // for ctor

    GrStencilPathOp(const SkMatrix& viewMatrix,
                    bool useHWAA,
                    bool hasStencilClip,
                    const GrScissorState& scissor,
                    sk_sp<const GrPath> path)
            : INHERITED(ClassID())
            , fViewMatrix(viewMatrix)
            , fUseHWAA(useHWAA)
            , fHasStencilClip(hasStencilClip)
            , fScissor(scissor)
            , fPath(std::move(path)) {
        this->setBounds(fPath->getBounds(), HasAABloat::kNo, IsHairline::kNo);
    }

    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        return SkStringPrintf("Path: 0x%p, AA: %d", fPath.get(), fUseHWAA);
    }
#endif

    SkMatrix                  fViewMatrix;
    bool                      fUseHWAA;
    bool                      fHasStencilClip;
    GrScissorState            fScissor;
    sk_sp<const GrPath>       fPath;

    using INHERITED = GrOp;
};

#endif
