/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDebugMarkerOp_DEFINED
#define GrDebugmarkerOp_DEFINED

#include "GrGpuCommandBuffer.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetProxy.h"

class GrDebugMarkerOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRenderTargetProxy* proxy,
                                      const char* str) {
        return std::unique_ptr<GrOp>(new GrDebugMarkerOp(proxy, str));
    }

    const char* name() const override { return "DebugMarker"; }

    SkString dumpInfo() const override {
        SkString string;
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrDebugMarkerOp(GrRenderTargetProxy* proxy, const char* str)
            : INHERITED(ClassID())
            , fStr(str) {
        // Make this cover the whole screen so it can't be reordered around
        this->setBounds(SkRect::MakeIWH(proxy->width(), proxy->height()),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        SkASSERT(state->drawOpArgs().renderTarget());

        //SkDebugf("%s\n", fStr);
        if (state->caps().gpuTracingSupport()) {
            state->commandBuffer()->insertEventMarker(state->drawOpArgs().fProxy, fStr);
        }
    }

    const char* fStr;

    typedef GrOp INHERITED;
};

#endif
