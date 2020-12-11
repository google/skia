/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateOp_DEFINED
#define GrStrokeTessellateOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/tessellate/GrStrokeOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class GrStrokeTessellateOp : public GrStrokeOp {
public:
    DEFINE_OP_CLASS_ID

private:
    // Patches can overlap, so until a stencil technique is implemented, the provided paint must be
    // a constant blended color.
    GrStrokeTessellateOp(GrAAType aaType, const SkMatrix& viewMatrix, const SkStrokeRec& stroke,
                         const SkPath& path, GrPaint&& paint)
            : GrStrokeOp(ClassID(), aaType, viewMatrix, stroke, path, std::move(paint)) {
    }

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                      GrLoadOp colorLoadOp) override;

    enum class JoinType {
        kFromStroke,  // The shader will use the join type defined in our fStrokeRec.
        kCusp,  // Double sided round join.
        kNone
    };

    // Is a cubic curve convex, and does it rotate no more than 180 degrees?
    enum class Convex180Status : bool {
        kUnknown,
        kYes
    };

    void onPrepare(GrOpFlushState*) override;
    void prepareBuffers();
    void moveTo(SkPoint);
    void moveTo(SkPoint, SkPoint lastControlPoint);
    void lineTo(SkPoint, JoinType prevJoinType = JoinType::kFromStroke);
    void quadraticTo(const SkPoint[3], JoinType prevJoinType = JoinType::kFromStroke,
                     int maxDepth = -1);
    void cubicTo(const SkPoint[4], JoinType prevJoinType = JoinType::kFromStroke,
                 Convex180Status = Convex180Status::kUnknown, int maxDepth = -1);
    void joinTo(JoinType joinType, const SkPoint nextCubic[]) {
        const SkPoint& nextCtrlPt = (nextCubic[1] == nextCubic[0]) ? nextCubic[2] : nextCubic[1];
        // The caller should have culled out cubics where p0==p1==p2 by this point.
        SkASSERT(nextCtrlPt != nextCubic[0]);
        this->joinTo(joinType, nextCtrlPt);
    }
    void joinTo(JoinType, SkPoint nextControlPoint, int maxDepth = -1);
    void close();
    void cap();
    void cubicToRaw(JoinType prevJoinType, const SkPoint pts[4]);
    void joinToRaw(JoinType, SkPoint nextControlPoint);
    GrStrokeTessellateShader::Patch* reservePatch();
    void allocPatchChunkAtLeast(int minPatchAllocCount);

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    // We generate and store patch buffers in chunks. Normally there will only be one chunk, but in
    // rare cases the first can run out of space if too many cubics needed to be subdivided.
    struct PatchChunk {
        sk_sp<const GrBuffer> fPatchBuffer;
        int fPatchCount = 0;
        int fBasePatch;
    };
    SkSTArray<1, PatchChunk> fPatchChunks;

    // The target will be non-null during prepareBuffers. It is used to allocate vertex space for
    // the patch chunks.
    GrMeshDrawOp::Target* fTarget = nullptr;

    // The maximum number of tessellation segments the hardware can emit for a single patch.
    int fMaxTessellationSegments;

    // Tolerances the tessellation shader will use for determining how much subdivision to do. We
    // need to ensure every curve we emit doesn't require more than fMaxTessellationSegments.
    GrStrokeTessellateShader::Tolerances fTolerances;

    // These values contain worst-case numbers of parametric segments, raised to the 4th power, that
    // our hardware can support for the current stroke radius. They assume curve rotations of 180
    // and 360 degrees respectively. These are used for "quick accepts" that allow us to send almost
    // all curves directly to the hardware without having to chop. We raise to the 4th power because
    // the "pow4" variants of Wang's formula are the quickest to evaluate.
    float fMaxParametricSegments180_pow4;
    float fMaxParametricSegments360_pow4;
    float fMaxParametricSegments180_pow4_withJoin;
    float fMaxParametricSegments360_pow4_withJoin;
    float fMaxCombinedSegments_withJoin;
    bool fSoloRoundJoinAlwaysFitsInPatch;

    // Variables related to the patch chunk that we are currently writing out during prepareBuffers.
    int fCurrChunkPatchCapacity;
    int fCurrChunkMinPatchAllocCount;
    GrStrokeTessellateShader::Patch* fCurrChunkPatchData;

    // Variables related to the specific contour that we are currently iterating during
    // prepareBuffers.
    bool fHasLastControlPoint = false;
    SkDEBUGCODE(bool fHasCurrentPoint = false;)
    SkPoint fCurrContourStartPoint;
    SkPoint fCurrContourFirstControlPoint;
    SkPoint fLastControlPoint;
    SkPoint fCurrentPoint;

    friend class GrOp;  // For ctor.

public:
    // This class is used to benchmark prepareBuffers().
    class TestingOnly_Benchmark;
};

#endif
