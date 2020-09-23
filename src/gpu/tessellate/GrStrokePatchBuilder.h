/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrStrokePatchBuilder_DEFINED
#define GrGrStrokePatchBuilder_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkTArray.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

class SkStrokeRec;

// This is an RAII class that expands strokes into tessellation patches for consumption by
// GrStrokeTessellateShader. The provided GrMeshDrawOp::Target must not be used externally for the
// entire lifetime of this class. e.g.:
//
//   void onPrepare(GrOpFlushState* target)  {
//        GrStrokePatchBuilder builder(target, &fMyPatchChunks, scale, count);  // Locks target.
//        for (...) {
//            builder.addPath(path, stroke);
//        }
//   }
//   ... target can now be used normally again.
//   ... fMyPatchChunks now contains chunks that can be drawn during onExecute.
class GrStrokePatchBuilder {
public:
    // We generate patch buffers in chunks. Normally there will only be one chunk, but in rare cases
    // the first can run out of space if too many cubics needed to be subdivided.
    struct PatchChunk {
        sk_sp<const GrBuffer> fPatchBuffer;
        int fPatchCount = 0;
        int fBasePatch;
    };

    // Stores raw pointers to the provided target and patchChunkArray, which this class will use and
    // push to as addPath is called. The caller is responsible to bind and draw each chunk that gets
    // pushed to the array. (See GrStrokeTessellateShader.)
    //
    // All points are multiplied by 'matrixScale' before being written to the GPU buffer.
    GrStrokePatchBuilder(GrMeshDrawOp::Target*, SkTArray<PatchChunk>*, float matrixScale,
                         const SkStrokeRec&, int totalCombinedVerbCnt);

    // "Releases" the target to be used externally again by putting back any unused pre-allocated
    // vertices.
    ~GrStrokePatchBuilder() {
        fTarget->putBackVertices(fCurrChunkPatchCapacity - fPatchChunkArray->back().fPatchCount,
                                 sizeof(GrStrokeTessellateShader::Patch));
    }

    void addPath(const SkPath&);

private:
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

    // These are raw pointers whose lifetimes are controlled outside this class.
    GrMeshDrawOp::Target* const fTarget;
    SkTArray<PatchChunk>* const fPatchChunkArray;

    const int fMaxTessellationSegments;
    // GrTessellationPathRenderer::kIntolerance adjusted for the matrix scale.
    const float fLinearizationIntolerance;

    // Variables related to the stroke parameters.
    const SkStrokeRec fStroke;
    float fNumRadialSegmentsPerRadian;
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

    // Variables related to the vertex chunk that we are currently filling.
    int fCurrChunkPatchCapacity;
    int fCurrChunkMinPatchAllocCount;
    GrStrokeTessellateShader::Patch* fCurrChunkPatchData;

    // Variables related to the specific contour that we are currently iterating.
    bool fHasLastControlPoint = false;
    SkDEBUGCODE(bool fHasCurrentPoint = false;)
    SkPoint fCurrContourStartPoint;
    SkPoint fCurrContourFirstControlPoint;
    SkPoint fLastControlPoint;
    SkPoint fCurrentPoint;
};

#endif
