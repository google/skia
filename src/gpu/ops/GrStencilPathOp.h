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

    // MDB TODO: replace the renderTargetContext with just the renderTargetProxy.
    // For now, we need the renderTargetContext for its accessRenderTarget powers.
    static std::unique_ptr<GrOp> Make(const SkMatrix& viewMatrix,
                                      bool useHWAA,
                                      GrPathRendering::FillType fillType,
                                      bool hasStencilClip,
                                      int numStencilBits,
                                      const GrScissorState& scissor,
                                      GrRenderTargetContext* renderTargetContext,
                                      const GrPath* path) {

        // MDB TODO: remove this. In this hybrid state we need to be sure the RT is instantiable
        // so it can carry the IO refs. In the future we will just get the proxy and
        // it will carry the IO refs.
        if (!renderTargetContext->accessRenderTarget()) {
            return nullptr;
        }

        return std::unique_ptr<GrOp>(new GrStencilPathOp(viewMatrix, useHWAA, fillType,
                                                         hasStencilClip, numStencilBits, scissor,
                                                         renderTargetContext, path));
    }

    const char* name() const override { return "StencilPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("Path: 0x%p, AA: %d", fPath.get(), fUseHWAA);
        string.appendf("rtID: %d proxyID: %d",
                       fRenderTarget.get()->uniqueID().asUInt(), fProxyUniqueID.asUInt());
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
                    GrRenderTargetContext* renderTargetContext,
                    const GrPath* path)
            : INHERITED(ClassID())
            , fViewMatrix(viewMatrix)
            , fUseHWAA(useHWAA)
            , fStencil(GrPathRendering::GetStencilPassSettings(fillType), hasStencilClip,
                       numStencilBits)
            , fScissor(scissor)
            , fProxyUniqueID(renderTargetContext->asSurfaceProxy()->uniqueID())
            , fPath(path) {
        this->setBounds(path->getBounds(), HasAABloat::kNo, IsZeroArea::kNo);

        fRenderTarget.reset(renderTargetContext->accessRenderTarget());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        // MDB TODO: instantiate the renderTarget from the proxy in here
        GrPathRendering::StencilPathArgs args(fUseHWAA, fRenderTarget.get(), &fViewMatrix,
                                              &fScissor, &fStencil);
        state->gpu()->pathRendering()->stencilPath(args, fPath.get());
    }

    SkMatrix                                             fViewMatrix;
    bool                                                 fUseHWAA;
    GrStencilSettings                                    fStencil;
    GrScissorState                                       fScissor;
    // MDB TODO: remove this. When the renderTargetProxy carries the refs this will be redundant.
    GrSurfaceProxy::UniqueID                             fProxyUniqueID;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    GrPendingIOResource<const GrPath, kRead_GrIOType>    fPath;

    typedef GrOp INHERITED;
};

#endif
