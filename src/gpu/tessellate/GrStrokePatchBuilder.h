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
    GrStrokePatchBuilder(GrMeshDrawOp::Target* target, SkTArray<PatchChunk>* patchChunkArray,
                         float matrixScale, int totalCombinedVerbCnt)
            : fTarget(target)
            , fPatchChunkArray(patchChunkArray)
            , fMaxTessellationSegments(target->caps().shaderCaps()->maxTessellationSegments())
            , fLinearizationIntolerance(matrixScale *
                                        GrTessellationPathRenderer::kLinearizationIntolerance) {
        // Pre-allocate at least enough vertex space for one stroke in 3 to split, and for 8 caps.
        int strokePreallocCount = totalCombinedVerbCnt * 4/3;
        int capPreallocCount = 8;
        int joinPreallocCount = strokePreallocCount + capPreallocCount;
        this->allocPatchChunkAtLeast(strokePreallocCount + capPreallocCount + joinPreallocCount);
    }

    // "Releases" the target to be used externally again by putting back any unused pre-allocated
    // vertices.
    ~GrStrokePatchBuilder() {
        fTarget->putBackVertices(fCurrChunkPatchCapacity - fPatchChunkArray->back().fPatchCount,
                                 sizeof(GrStrokeTessellateShader::Patch));
    }

    void addPath(const SkPath&, const SkStrokeRec&);

private:
    void allocPatchChunkAtLeast(int minPatchAllocCount);
    GrStrokeTessellateShader::Patch* reservePatch();

    void writeCubicSegment(float prevJoinType, const SkPoint pts[4], float cubicType);
    void writeJoin(float joinType, const SkPoint& prevControlPoint, const SkPoint& anchorPoint,
                   const SkPoint& nextControlPoint);
    void writeSquareCap(const SkPoint& endPoint, const SkPoint& controlPoint);
    void writeCaps(SkPaint::Cap);

    void moveTo(const SkPoint&);
    void lineTo(float prevJoinType, const SkPoint&, const SkPoint&);
    void quadraticTo(float prevJoinType, const SkPoint[3], int maxDepth = -1);
    void cubicTo(float prevJoinType, const SkPoint[4], int maxDepth = -1, bool mightInflect = true);
    void close(SkPaint::Cap);

    // These are raw pointers whose lifetimes are controlled outside this class.
    GrMeshDrawOp::Target* const fTarget;
    SkTArray<PatchChunk>* const fPatchChunkArray;

    const int fMaxTessellationSegments;
    // GrTessellationPathRenderer::kIntolerance adjusted for the matrix scale.
    const float fLinearizationIntolerance;

    // Variables related to the patch chunk that we are currently filling.
    int fCurrChunkPatchCapacity;
    int fCurrChunkMinPatchAllocCount;
    GrStrokeTessellateShader::Patch* fCurrChunkPatchData;

    // Variables related to the path that we are currently iterating.
    float fCurrStrokeRadius;
    float fCurrStrokeJoinType;  // See GrStrokeTessellateShader for join type definitions.
    float fNumRadialSegmentsPerRad;
    // These values contain worst-case numbers of parametric segments our hardware can support for
    // the current stroke radius, in the event that there are also enough radial segments to rotate
    // 180 and 360 degrees respectively. These are used for "quick accepts" that allow us to send
    // almost all curves directly to the hardware without having to chop or think any further.
    float fMaxParametricSegments180;
    float fMaxParametricSegments360;

    // Variables related to the specific contour that we are currently iterating.
    bool fHasPreviousSegment = false;
    SkPoint fCurrContourStartPoint;
    SkPoint fCurrContourFirstControlPoint;
    SkPoint fLastControlPoint;
    SkPoint fCurrentPoint;
};

#endif
