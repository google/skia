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
                                      GrRenderTargetProxy* renderTargetProxy,
                                      const GrPath* path) {

        return std::unique_ptr<GrOp>(new GrStencilPathOp(viewMatrix, useHWAA, fillType,
                                                         hasStencilClip, numStencilBits, scissor,
                                                         renderTargetProxy, path));
    }

    const char* name() const override { return "StencilPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("Path: 0x%p, AA: %d", fPath.get(), fUseHWAA);
        string.appendf("proxyID: %d",
                       fRenderTargetProxy.get()->uniqueID().asUInt());
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
                    GrRenderTargetProxy* renderTargetProxy,
                    const GrPath* path)
            : INHERITED(ClassID())
            , fViewMatrix(viewMatrix)
            , fUseHWAA(useHWAA)
            , fStencil(GrPathRendering::GetStencilPassSettings(fillType), hasStencilClip,
                       numStencilBits)
            , fScissor(scissor)
            , fPath(path) {
        this->setBounds(path->getBounds(), HasAABloat::kNo, IsZeroArea::kNo);

        fRenderTargetProxy.reset(renderTargetProxy);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        // MDB TODO: instantiate the renderTarget from the proxy in here
        GrRenderTarget* rt = fRenderTargetProxy.get()->instantiate(nullptr);
        if (!rt) {
            return;
        }

        GrPathRendering::StencilPathArgs args(fUseHWAA, rt, &fViewMatrix,
                                              &fScissor, &fStencil);
        state->gpu()->pathRendering()->stencilPath(args, fPath.get());
    }

    SkMatrix                                             fViewMatrix;
    bool                                                 fUseHWAA;
    GrStencilSettings                                    fStencil;
    GrScissorState                                       fScissor;
    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;
    GrPendingIOResource<const GrPath, kRead_GrIOType>    fPath;

    typedef GrOp INHERITED;
};

#endif
