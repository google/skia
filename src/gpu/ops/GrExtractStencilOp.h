/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrExtractStencilOp_DEFINED
#define GrExtractStencilOp_DEFINED

#include "GrGpuCommandBuffer.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetProxy.h"

class GrExtractStencilOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRenderTargetProxy* proxy) {
        return std::unique_ptr<GrOp>(new GrExtractStencilOp(proxy));
    }

    const char* name() const override { return "ExtractStencil"; }

    SkString dumpInfo() const override {
        SkString string;
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrExtractStencilOp(GrRenderTargetProxy* proxy) : INHERITED(ClassID()) {
        this->makeFullScreen(proxy);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        SkASSERT(state->drawOpArgs().renderTarget());

        state->commandBuffer()->discard(state->drawOpArgs().fProxy);
    }

    typedef GrOp INHERITED;
};

#endif
