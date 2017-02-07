/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilPathOp_DEFINED
#define GrStencilPathOp_DEFINED

#include "GrGpu.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrPath.h"
#include "GrPathRendering.h"
#include "GrRenderTarget.h"
#include "GrStencilSettings.h"

class GrStencilPathOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(const SkMatrix& viewMatrix,
                                      bool useHWAA,
                                      GrPathRendering::FillType fillType,
                                      bool hasStencilClip,
                                      int numStencilBits,
                                      const GrScissorState& scissor,
                                      GrRenderTarget* renderTarget,
                                      const GrPath* path) {
        return std::unique_ptr<GrOp>(new GrStencilPathOp(viewMatrix, useHWAA, fillType,
                                                         hasStencilClip, numStencilBits, scissor,
                                                         renderTarget, path));
    }

    const char* name() const override { return "StencilPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("PATH: 0x%p, AA:%d", fPath.get(), fUseHWAA);
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrStencilPathOp(const SkMatrix& viewMatrix,
                    bool useHWAA,
                    GrPathRendering::FillType fillType,
                    bool hasStencilClip,
                    int numStencilBits,
                    const GrScissorState& scissor,
                    GrRenderTarget* renderTarget,
                    const GrPath* path)
            : INHERITED(ClassID())
            , fViewMatrix(viewMatrix)
            , fUseHWAA(useHWAA)
            , fStencil(GrPathRendering::GetStencilPassSettings(fillType), hasStencilClip,
                       numStencilBits)
            , fScissor(scissor)
            , fRenderTarget(renderTarget)
            , fPath(path) {
        this->setBounds(path->getBounds(), HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& bounds) override {
        GrPathRendering::StencilPathArgs args(fUseHWAA, fRenderTarget.get(), &fViewMatrix,
                                              &fScissor, &fStencil);
        state->gpu()->pathRendering()->stencilPath(args, fPath.get());
    }

    SkMatrix fViewMatrix;
    bool fUseHWAA;
    GrStencilSettings fStencil;
    GrScissorState fScissor;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;

    typedef GrOp INHERITED;
};

#endif
