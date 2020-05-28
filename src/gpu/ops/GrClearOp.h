/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearOp_DEFINED
#define GrClearOp_DEFINED

#include "src/gpu/GrScissorState.h"
#include "src/gpu/ops/GrOp.h"

class GrOpFlushState;
class GrRecordingContext;

class GrClearOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    // A fullscreen or scissored clear, depending on the clip and proxy dimensions
    static std::unique_ptr<GrClearOp> Make(GrRecordingContext* context,
                                           const GrScissorState& scissor,
                                           const SkPMColor4f& color);

    const char* name() const override { return "Clear"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.append(INHERITED::dumpInfo());
        string.appendf("Scissor [ ");
        if (fScissor.enabled()) {
            const SkIRect& r = fScissor.rect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        } else {
            string.append("disabled");
        }
        string.appendf("], Color: 0x%08x\n", fColor.toBytes_RGBA());
        return string;
    }
#endif

private:
    friend class GrOpMemoryPool; // for ctors

    GrClearOp(const GrScissorState& scissor, const SkPMColor4f& color);

    CombineResult onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*,
                                      const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearOp* cb = t->cast<GrClearOp>();
        if (cb->contains(this)) {
            fScissor = cb->fScissor;
            fColor = cb->fColor;
            return CombineResult::kMerged;
        } else if (cb->fColor == fColor && this->contains(cb)) {
            return CombineResult::kMerged;
        }
        return CombineResult::kCannotCombine;
    }

    bool contains(const GrClearOp* that) const {
        // The constructor ensures that scissor gets disabled on any clip that fills the entire RT.
        return !fScissor.enabled() ||
               (that->fScissor.enabled() && fScissor.rect().contains(that->fScissor.rect()));
    }

    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView* writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) override {}

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override;

    GrScissorState fScissor;
    SkPMColor4f    fColor;

    typedef GrOp INHERITED;
};

#endif
