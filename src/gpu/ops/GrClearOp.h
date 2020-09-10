/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearOp_DEFINED
#define GrClearOp_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/GrScissorState.h"
#include "src/gpu/ops/GrOp.h"

class GrOpFlushState;
class GrRecordingContext;

class GrClearOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    // A fullscreen or scissored clear, depending on the clip and proxy dimensions
    static std::unique_ptr<GrClearOp> MakeColor(GrRecordingContext* context,
                                                const GrScissorState& scissor,
                                                const SkPMColor4f& color);

    static std::unique_ptr<GrClearOp> MakeStencilClip(GrRecordingContext* context,
                                                      const GrScissorState& scissor,
                                                      bool insideMask);

    const char* name() const override { return "Clear"; }

private:
    friend class GrOpMemoryPool; // for ctors

    enum class Buffer {
        kColor       = 0b01,
        kStencilClip = 0b10,

        kBoth        = 0b11,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Buffer);

    GrClearOp(Buffer buffer, const GrScissorState& scissor, const SkPMColor4f& color, bool stencil);

    CombineResult onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*,
                                      const GrCaps& caps) override;

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView* writeView, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers) override {}

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override;
#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        SkString string("Scissor [ ");
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

    GrScissorState fScissor;
    SkPMColor4f    fColor;
    bool           fStencilInsideMask;
    Buffer         fBuffer;

    using INHERITED = GrOp;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrClearOp::Buffer)

#endif
