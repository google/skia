/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearOp_DEFINED
#define GrClearOp_DEFINED

#include "GrFixedClip.h"
#include "GrOp.h"

class GrOpFlushState;
class GrRecordingContext;

class GrClearOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrClearOp> Make(GrRecordingContext* context,
                                           const GrFixedClip& clip,
                                           const SkPMColor4f& color,
                                           GrSurfaceProxy* dstProxy);

    static std::unique_ptr<GrClearOp> Make(GrRecordingContext* context,
                                           const SkIRect& rect,
                                           const SkPMColor4f& color,
                                           bool fullScreen);

    const char* name() const override { return "Clear"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.append(INHERITED::dumpInfo());
        string.appendf("Scissor [ ");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        } else {
            string.append("disabled");
        }
        string.appendf("], Color: 0x%08x\n", fColor.toBytes_RGBA());
        return string;
    }
#endif

    const SkPMColor4f& color() const { return fColor; }
    void setColor(const SkPMColor4f& color) { fColor = color; }

private:
    friend class GrOpMemoryPool; // for ctors

    GrClearOp(const GrFixedClip& clip, const SkPMColor4f& color, GrSurfaceProxy* proxy);

    GrClearOp(const SkIRect& rect, const SkPMColor4f& color, bool fullScreen)
        : INHERITED(ClassID())
        , fClip(GrFixedClip(rect))
        , fColor(color) {

        if (fullScreen) {
            fClip.disableScissor();
        }
        this->setBounds(SkRect::Make(rect), HasAABloat::kNo, IsZeroArea::kNo);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearOp* cb = t->cast<GrClearOp>();
        if (fClip.windowRectsState() != cb->fClip.windowRectsState()) {
            return CombineResult::kCannotCombine;
        }
        if (cb->contains(this)) {
            fClip = cb->fClip;
            fColor = cb->fColor;
            return CombineResult::kMerged;
        } else if (cb->fColor == fColor && this->contains(cb)) {
            return CombineResult::kMerged;
        }
        return CombineResult::kCannotCombine;
    }

    bool contains(const GrClearOp* that) const {
        // The constructor ensures that scissor gets disabled on any clip that fills the entire RT.
        return !fClip.scissorEnabled() ||
               (that->fClip.scissorEnabled() &&
                fClip.scissorRect().contains(that->fClip.scissorRect()));
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override;

    GrFixedClip fClip;
    SkPMColor4f fColor;

    typedef GrOp INHERITED;
};

#endif
