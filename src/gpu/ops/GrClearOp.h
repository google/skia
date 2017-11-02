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

class GrClearOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrClearOp> Make(const GrFixedClip& clip, GrColor color,
                                           GrSurfaceProxy* dstProxy) {
        const SkIRect rect = SkIRect::MakeWH(dstProxy->width(), dstProxy->height());
        if (clip.scissorEnabled() && !SkIRect::Intersects(clip.scissorRect(), rect)) {
            return nullptr;
        }

        return std::unique_ptr<GrClearOp>(new GrClearOp(clip, color, dstProxy));
    }

    static std::unique_ptr<GrClearOp> Make(const SkIRect& rect, GrColor color,
                                           bool fullScreen) {
        SkASSERT(fullScreen || !rect.isEmpty());

        return std::unique_ptr<GrClearOp>(new GrClearOp(rect, color, fullScreen));
    }

    const char* name() const override { return "Clear"; }

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
        string.appendf("], Color: 0x%08x\n", fColor);
        return string;
    }

    GrColor color() const { return fColor; }
    void setColor(GrColor color) { fColor = color; }

private:
    GrClearOp(const GrFixedClip& clip, GrColor color, GrSurfaceProxy* proxy);

    GrClearOp(const SkIRect& rect, GrColor color, bool fullScreen)
        : INHERITED(ClassID())
        , fClip(GrFixedClip(rect))
        , fColor(color) {

        if (fullScreen) {
            fClip.disableScissor();
        }
        this->setBounds(SkRect::Make(rect), HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearOp* cb = t->cast<GrClearOp>();
        if (fClip.windowRectsState() != cb->fClip.windowRectsState()) {
            return false;
        }
        if (cb->contains(this)) {
            fClip = cb->fClip;
            this->replaceBounds(*t);
            fColor = cb->fColor;
            return true;
        } else if (cb->fColor == fColor && this->contains(cb)) {
            return true;
        }
        return false;
    }

    bool contains(const GrClearOp* that) const {
        // The constructor ensures that scissor gets disabled on any clip that fills the entire RT.
        return !fClip.scissorEnabled() ||
               (that->fClip.scissorEnabled() &&
                fClip.scissorRect().contains(that->fClip.scissorRect()));
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override;

    GrFixedClip fClip;
    GrColor     fColor;

    typedef GrOp INHERITED;
};

#endif
