/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeHardwareTessellator_DEFINED
#define GrStrokeHardwareTessellator_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/tessellate/GrStrokeTessellateOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

// Renders opaque, constant-color strokes by decomposing them into standalone tessellation patches.
// Each patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". Requires
// MSAA if antialiasing is desired.
class GrStrokeHardwareTessellator : public GrStrokeTessellator {
public:
    GrStrokeHardwareTessellator(ShaderFlags shaderFlags,
                                GrSTArenaList<PathStroke>&& pathStrokeList,
                                int totalCombinedVerbCnt, const GrShaderCaps& shaderCaps)
            : GrStrokeTessellator(shaderFlags, std::move(pathStrokeList))
            , fTotalCombinedVerbCnt(totalCombinedVerbCnt)
            , fPatchStride(GrStrokeTessellateShader::PatchStride(fShaderFlags))
            // Subtract 2 because the tessellation shader chops every cubic at two locations, and
            // each chop has the potential to introduce an extra segment.
            , fMaxTessellationSegments(shaderCaps.maxTessellationSegments() - 2) {
    }

    void prepare(GrMeshDrawOp::Target*, const SkMatrix&) override;

    void draw(GrOpFlushState*) const override;

private:
    using Tolerances = GrStrokeTessellateShader::Tolerances;

    enum class JoinType {
        kFromStroke,  // The shader will use the join type defined in our fStrokeRec.
        kBowtie,  // Double sided round join.
        kNone
    };

    // Is a cubic curve convex, and does it rotate no more than 180 degrees?
    enum class Convex180Status : bool {
        kUnknown,
        kYes
    };

    // Updates our internal tolerances for determining how much subdivision to do. We need to ensure
    // every curve we emit requires no more segments than fMaxTessellationSegments.
    void updateTolerances(Tolerances, SkPaint::Join strokeJoin);

    void moveTo(SkPoint);
    void moveTo(SkPoint, SkPoint lastControlPoint);
    void lineTo(SkPoint, JoinType prevJoinType = JoinType::kFromStroke);
    void conicTo(const SkPoint[3], float w, JoinType prevJoinType = JoinType::kFromStroke,
                 int maxDepth = -1);
    void cubicTo(const SkPoint[4], JoinType prevJoinType = JoinType::kFromStroke,
                 Convex180Status = Convex180Status::kUnknown, int maxDepth = -1);
    // Chops the curve into 1-3 convex sections that rotate no more than 180 degrees, then calls
    // cubicTo() for each section.
    void cubicConvex180SegmentsTo(const SkPoint[4], JoinType prevJoinType = JoinType::kFromStroke,
                                  int maxDepth = -1);
    void joinTo(JoinType joinType, const SkPoint nextCubic[]) {
        const SkPoint& nextCtrlPt = (nextCubic[1] == nextCubic[0]) ? nextCubic[2] : nextCubic[1];
        // The caller should have culled out curves where p0==p1==p2 by this point.
        SkASSERT(nextCtrlPt != nextCubic[0]);
        this->joinTo(joinType, nextCtrlPt);
    }
    void joinTo(JoinType, SkPoint nextControlPoint, int maxDepth = -1);
    void close();
    void cap();
    void emitPatch(JoinType prevJoinType, const SkPoint pts[4], SkPoint endPt);
    void emitJoinPatch(JoinType, SkPoint nextControlPoint);
    void emitDynamicAttribs();
    bool reservePatch();
    void allocPatchChunkAtLeast(int minPatchAllocCount);

    // The combined number of path verbs from all paths in fPathStrokeList.
    const int fTotalCombinedVerbCnt;

    // Size in bytes of a tessellation patch with our shader flags.
    const size_t fPatchStride;

    // The maximum number of tessellation segments the hardware can emit for a single patch.
    const int fMaxTessellationSegments;

    // These will only be valid during prepare() and its callees.
    GrMeshDrawOp::Target* fTarget = nullptr;
    const SkMatrix* fViewMatrix = nullptr;
    const SkStrokeRec* fStroke = nullptr;

    // These values contain worst-case numbers of parametric segments, raised to the 4th power, that
    // our hardware can support for the current stroke radius. They assume curve rotations of 180
    // and 360 degrees respectively. These are used for "quick accepts" that allow us to send almost
    // all curves directly to the hardware without having to chop. We raise to the 4th power because
    // the "pow4" variants of Wang's formula are the quickest to evaluate.
    GrStrokeTessellateShader::Tolerances fTolerances;
    float fMaxParametricSegments180_pow4;
    float fMaxParametricSegments360_pow4;
    float fMaxParametricSegments180_pow4_withJoin;
    float fMaxParametricSegments360_pow4_withJoin;
    float fMaxCombinedSegments_withJoin;
    bool fSoloRoundJoinAlwaysFitsInPatch;

    // We generate and store patch buffers in chunks. Normally there will only be one chunk, but in
    // rare cases the first can run out of space if too many cubics needed to be subdivided.
    struct PatchChunk {
        sk_sp<const GrBuffer> fPatchBuffer;
        int fPatchCount = 0;
        int fBasePatch;
    };
    SkSTArray<1, PatchChunk> fPatchChunks;

    // Variables related to the patch chunk that we are currently writing out during prepareBuffers.
    int fCurrChunkPatchCapacity;
    int fCurrChunkMinPatchAllocCount;
    GrVertexWriter fPatchWriter;

    // Variables related to the specific contour that we are currently iterating during
    // prepareBuffers().
    bool fHasLastControlPoint = false;
    SkDEBUGCODE(bool fHasCurrentPoint = false;)
    SkPoint fCurrContourStartPoint;
    SkPoint fCurrContourFirstControlPoint;
    SkPoint fLastControlPoint;
    SkPoint fCurrentPoint;

    // Stateful values for the dynamic state (if any) that will get written out with each patch.
    GrStrokeTessellateShader::DynamicStroke fDynamicStroke;
    GrVertexColor fDynamicColor;

    friend class GrOp;  // For ctor.

public:
    // This class is used to benchmark prepareBuffers().
    class TestingOnly_Benchmark;
};

#endif
