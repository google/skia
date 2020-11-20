/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawableOp_DEFINED
#define GrDrawableOp_DEFINED

#include "src/gpu/ops/GrOp.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkMatrix.h"
#include "src/gpu/GrSemaphore.h"

class GrRecordingContext;

class GrDrawableOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext*,
                            std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                            const SkRect& bounds);

    const char* name() const override { return "Drawable"; }

private:
    friend class GrOp; // for ctor

    GrDrawableOp(std::unique_ptr<SkDrawable::GpuDrawHandler>, const SkRect& bounds);

    CombineResult onCombineIfPossible(GrOp* that, SkArenaAlloc*, const GrCaps& caps) override {
        return CombineResult::kCannotCombine;
    }

    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    std::unique_ptr<SkDrawable::GpuDrawHandler> fDrawable;

    using INHERITED = GrOp;
};

#endif

