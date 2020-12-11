/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeIndirectOp_DEFINED
#define GrStrokeIndirectOp_DEFINED

#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrStrokeOp.h"

struct SkPoint;
namespace skiatest { class Reporter; }

// This class bins strokes into indirect draws for consumption by GrStrokeTessellateShader.
class GrStrokeIndirectOp : public GrStrokeOp {
public:
    DEFINE_OP_CLASS_ID

    // Don't allow more than 2^15 stroke edges in a triangle strip. GrTessellationPathRenderer
    // already crops paths that require more than 2^10 parametric segments, so this should only
    // become an issue if we try to draw a stroke with an astronomically wide width.
    constexpr static int8_t kMaxResolveLevel = 15;

private:
    GrStrokeIndirectOp(GrAAType aaType, const SkMatrix& viewMatrix, const SkPath& path,
                       const SkStrokeRec& stroke, GrPaint&& paint)
            : GrStrokeOp(ClassID(), aaType, viewMatrix, stroke, path, std::move(paint)) {
    }

    const char* name() const override { return "GrStrokeIndirectOp"; }

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                      GrLoadOp colorLoadOp) override;
    void prePrepareResolveLevels(SkArenaAlloc*);

    void onPrepare(GrOpFlushState*) override;
    void prepareBuffers(GrMeshDrawOp::Target*);

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    int fResolveLevelCounts[kMaxResolveLevel + 1] = {0};  // # of instances at each resolve level.
    int fTotalInstanceCount = 0;  // Total number of stroke instances we will draw.

    // This array holds a resolveLevel for each stroke in the path, stored in the iteration order of
    // GrStrokeIterator. If a stroke needs to be chopped, the array will contain a negative number
    // whose absolute value is the number of chops required, followed by a resolveLevel for each
    // resulting stroke after the chop(s).
    int8_t* fResolveLevels = nullptr;
    // fResolveLevelArrayCount != fTotalInstanceCount because we don't always need to write out
    // resolve levels for line instances. (If they don't have round caps then their resolve level is
    // just 0.)
    SkDEBUGCODE(int fResolveLevelArrayCount = 0;)

    // Stores the in-order chop locations for all chops indicated by fResolveLevels.
    float* fChopTs = nullptr;
    SkDEBUGCODE(int fChopTsArrayCount = 0;)

    // A "circle" is a stroke-width circle drawn as a 180-degree point stroke. We draw them at cusp
    // points on curves and for round caps.
    int8_t fResolveLevelForCircles;

    // GPU buffers for drawing.
    sk_sp<const GrBuffer> fDrawIndirectBuffer;
    sk_sp<const GrBuffer> fInstanceBuffer;
    size_t fDrawIndirectOffset;
    int fDrawIndirectCount = 0;

    friend class GrOp;  // For ctor.

#if GR_TEST_UTILS
public:
    void verifyPrePrepareResolveLevels(skiatest::Reporter*, GrMeshDrawOp::Target*);
    void verifyPrepareBuffers(skiatest::Reporter*, GrMeshDrawOp::Target*);
    class Benchmark;
#endif
};

#endif
