/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearStencilClipOp_DEFINED
#define GrClearStencilClipOp_DEFINED

#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/ops/GrOp.h"

class GrOpFlushState;
class GrRecordingContext;

class GrClearStencilClipOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRecordingContext* context,
                                      const GrFixedClip& clip,
                                      bool insideStencilMask,
                                      GrRenderTargetProxy* proxy);

    const char* name() const override { return "ClearStencilClip"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string("Scissor [");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        } else {
            string.append("disabled");
        }
        string.appendf("], insideMask: %s\n", fInsideStencilMask ? "true" : "false");
        string.append(INHERITED::dumpInfo());
        return string;
    }
#endif

private:
    friend class GrOpMemoryPool; // for ctor

    GrClearStencilClipOp(const GrFixedClip& clip, bool insideStencilMask,
                         GrRenderTargetProxy* proxy)
            : INHERITED(ClassID())
            , fClip(clip)
            , fInsideStencilMask(insideStencilMask) {
        const SkRect& bounds = fClip.scissorEnabled()
                                            ? SkRect::Make(fClip.scissorRect())
                                            : SkRect::MakeIWH(proxy->width(), proxy->height());
        this->setBounds(bounds, HasAABloat::kNo, IsHairline::kNo);
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const GrFixedClip fClip;
    const bool        fInsideStencilMask;

    typedef GrOp INHERITED;
};

#endif
