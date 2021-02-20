/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeIndirectTessellator_DEFINED
#define GrStrokeIndirectTessellator_DEFINED

#include "src/gpu/tessellate/GrStrokeTessellateOp.h"

struct GrVertexWriter;
struct SkPoint;
namespace skiatest { class Reporter; }

// This class bins strokes into indirect draws for consumption by GrStrokeTessellateShader.
class GrStrokeIndirectTessellator : public GrStrokeTessellator {
public:
    // Don't allow more than 2^15 stroke edges in a triangle strip. GrTessellationPathRenderer
    // already crops paths that require more than 2^10 parametric segments, so this should only
    // become an issue if we try to draw a stroke with an astronomically wide width.
    constexpr static int8_t kMaxResolveLevel = 15;

    GrStrokeIndirectTessellator(ShaderFlags, const SkMatrix&, PathStrokeList*,
                                int totalCombinedVerbCnt, SkArenaAlloc*);

    // Adds the given tessellator to our chain. The chained tessellators all append to a shared
    // indirect draw list during prepare().
    void addToChain(GrStrokeIndirectTessellator*);

    void prepare(GrMeshDrawOp::Target*, const SkMatrix&) override;

    void draw(GrOpFlushState*) const override;

private:
    // Called during prepare(). Appends our indirect-draw commands and instance data onto the
    // provided writers.
    void writeBuffers(GrDrawIndirectWriter*, GrVertexWriter*, const SkMatrix&,
                      size_t instanceStride, int baseInstance, int numExtraEdgesInJoin);

    int fResolveLevelCounts[kMaxResolveLevel + 1] = {0};  // # of instances at each resolve level.
    int fTotalInstanceCount = 0;  // Total number of stroke instances we will draw.
    // Total number of indirect draw commands in the chain, or zero if we are not the chain head.
    int fChainedDrawIndirectCount = 0;
    // Total number of stroke instances in the entire chain, or zero if we are not the chain head.
    int fChainedInstanceCount = 0;

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

    // Bevel, miter, and round joins require us to add different numbers of additional edges onto
    // their triangle strips. When using dynamic stroke, we just append the maximum required number
    // of additional edges to every instance.
    int fMaxNumExtraEdgesInJoin = 0;

    // Chained tessellators. These all append to our shared indirect draw list during prepare().
    GrStrokeIndirectTessellator* fNextInChain = nullptr;
    GrStrokeIndirectTessellator** fChainTail = &fNextInChain;  // Null if we are not the chain head.

    // GPU buffers for drawing.
    sk_sp<const GrBuffer> fDrawIndirectBuffer;
    sk_sp<const GrBuffer> fInstanceBuffer;
    size_t fDrawIndirectOffset;

    friend class GrOp;  // For ctor.

#if GR_TEST_UTILS
public:
    void verifyResolveLevels(skiatest::Reporter*, class GrMockOpTarget*, const SkMatrix&,
                             const SkPath&, const SkStrokeRec&);
    void verifyBuffers(skiatest::Reporter*, class GrMockOpTarget*, const SkMatrix&,
                       const SkStrokeRec&);
    class Benchmark;
#endif
};

#endif
