/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DrawableOp_DEFINED
#define DrawableOp_DEFINED

#include "include/core/SkDrawable.h"
#include "src/gpu/ops/GrOp.h"

class GrRecordingContext;

namespace skgpu::v1 {

class DrawableOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext*,
                            std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                            const SkRect& bounds);

    const char* name() const override { return "Drawable"; }

private:
    friend class GrOp; // for ctor

    DrawableOp(std::unique_ptr<SkDrawable::GpuDrawHandler>, const SkRect& bounds);

    CombineResult onCombineIfPossible(GrOp* that, SkArenaAlloc*, const GrCaps& caps) override {
        return CombineResult::kCannotCombine;
    }

    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrDstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    std::unique_ptr<SkDrawable::GpuDrawHandler> fDrawable;
};

} // namespace skgpu::v1

#endif // DrawableOp_DEFINED
