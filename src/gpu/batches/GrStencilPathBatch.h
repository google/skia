/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilPathBatch_DEFINED
#define GrStencilPathBatch_DEFINED

#include "GrBatch.h"
#include "GrBatchFlushState.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRendering.h"
#include "GrRenderTarget.h"

class GrStencilPathBatch final : public GrBatch {
public:
    DEFINE_BATCH_CLASS_ID

    static GrBatch* Create(const SkMatrix& viewMatrix,
                           bool useHWAA,
                           const GrStencilSettings& stencil,
                           const GrScissorState& scissor,
                           GrRenderTarget* renderTarget,
                           const GrPath* path) {
        return new GrStencilPathBatch(viewMatrix, useHWAA, stencil, scissor, renderTarget, path);
    }

    const char* name() const override { return "StencilPath"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->getUniqueID(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("PATH: 0x%p, AA:%d", fPath.get(), fUseHWAA);
        return string;
    }

private:
    GrStencilPathBatch(const SkMatrix& viewMatrix,
                       bool useHWAA,
                       const GrStencilSettings& stencil,
                       const GrScissorState& scissor,
                       GrRenderTarget* renderTarget,
                       const GrPath* path)
    : INHERITED(ClassID())
    , fViewMatrix(viewMatrix)
    , fUseHWAA(useHWAA)
    , fStencil(stencil)
    , fScissor(scissor)
    , fRenderTarget(renderTarget)
    , fPath(path) {
        fBounds = path->getBounds();
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        GrPathRendering::StencilPathArgs args(fUseHWAA, fRenderTarget.get(), &fViewMatrix,
                                              &fScissor, &fStencil);
        state->gpu()->pathRendering()->stencilPath(args, fPath.get());
    }

    SkMatrix                                                fViewMatrix;
    bool                                                    fUseHWAA;
    GrStencilSettings                                       fStencil;
    GrScissorState                                          fScissor;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;
    GrPendingIOResource<const GrPath, kRead_GrIOType>       fPath;

    typedef GrBatch INHERITED;
};

#endif
