/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeIndirectOp_DEFINED
#define GrStrokeIndirectOp_DEFINED

#include "include/core/SkPoint.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrStrokeOp.h"

namespace skiatest { class Reporter; }

// This class expands strokes into tessellation patches for consumption by GrStrokeTessellateShader.
class GrStrokeIndirectOp : public GrStrokeOp {
public:
    DEFINE_OP_CLASS_ID
    constexpr static int8_t kMaxResolveLevel = 15;

private:
    GrStrokeIndirectOp(GrAAType, const SkMatrix&, const SkStrokeRec&, const SkPath&, GrPaint&&);
    const char* name() const override { return "GrStrokeIndirectOp"; }

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                      GrLoadOp colorLoadOp) override;
    void prePrepareResolveLevels(SkArenaAlloc*);

    void onPrepare(GrOpFlushState*) override;
    void prepareBuffers(GrMeshDrawOp::Target*);

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const int8_t fResolveLevelForCusps;

    int fResolveLevelCounts[kMaxResolveLevel + 1] = {0};
    int fTotalInstanceCount = 0;
    int8_t* fResolveLevels = nullptr;
    SkDEBUGCODE(int fNumResolveLevels = 0;)

    sk_sp<const GrBuffer> fInstanceBuffer;
    sk_sp<const GrBuffer> fDrawIndirectBuffer;
    size_t fDrawIndirectOffset;
    int fDrawIndirectCount = 0;

    friend class GrOp;  // For ctor.

public:
    void testingOnly_verifyPrePrepareResolveLevels(skiatest::Reporter*, GrMeshDrawOp::Target*);
    void testingOnly_verifyPrepareBuffers(skiatest::Reporter*, GrMeshDrawOp::Target*);
    // This class is used to benchmark prepareBuffers().
    class TestingOnly_Benchmark;
};

#endif
