/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellateStrokeOp_DEFINED
#define GrTessellateStrokeOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/ops/GrDrawOp.h"

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class GrTessellateStrokeOp : public GrDrawOp {
    DEFINE_OP_CLASS_ID

    // The provided matrix must be a similarity matrix for the time being. This is so we can
    // bootstrap this Op on top of GrStrokeGeometry with minimal modifications.
    //
    // Patches can overlap, so until a stencil technique is implemented, the provided paint must be
    // a constant blended color.
    GrTessellateStrokeOp(const SkMatrix&, const SkPath&, const SkStrokeRec&, GrPaint&&, GrAAType);

    const char* name() const override { return "GrTessellateStrokeOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fProcessors.visitProxies(fn); }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView*, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) override;
    void onPrepare(GrOpFlushState* state) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const SkPath fDevPath;
    const SkStrokeRec fDevStroke;
    const GrAAType fAAType;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    sk_sp<const GrBuffer> fVertexBuffer;
    int fVertexCount = 0;
    int fBaseVertex;

    friend class GrOpMemoryPool;  // For ctor.
};

#endif
