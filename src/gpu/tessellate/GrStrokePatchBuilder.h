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
//        GrStrokePatchBuilder builder(target, &fMyVertexChunks, scale, count);  // Locks target.
//        for (...) {
//            builder.addPath(path, stroke);
//        }
//   }
//   ... target can now be used normally again.
//   ... fMyVertexChunks now contains chunks that can be drawn during onExecute.
class GrStrokePatchBuilder {
public:
    // We generate vertex buffers in chunks. Normally there will only be one chunk, but in rare
    // cases the first can run out of space if too many cubics needed to be subdivided.
    struct VertexChunk {
        sk_sp<const GrBuffer> fVertexBuffer;
        int fVertexCount = 0;
        int fBaseVertex;
    };

    // Stores raw pointers to the provided target and vertexChunkArray, which this class will use
    // and push to as addPath is called. The caller is responsible to bind and draw each chunk that
    // gets pushed to the array. (See GrStrokeTessellateShader.)
    //
    // All points are multiplied by 'matrixScale' before being written to the GPU buffer.
    GrStrokePatchBuilder(GrMeshDrawOp::Target* target, SkTArray<VertexChunk>* vertexChunkArray,
                         float matrixScale, int totalCombinedVerbCnt)
            : fTarget(target)
            , fVertexChunkArray(vertexChunkArray)
            , fMaxTessellationSegments(target->caps().shaderCaps()->maxTessellationSegments())
            , fMatrixScale(matrixScale) {
        this->allocVertexChunk(
                (totalCombinedVerbCnt * 3) * GrStrokeTessellateShader::kNumVerticesPerPatch);
    }

    // "Releases" the target to be used externally again by putting back any unused pre-allocated
    // vertices.
    ~GrStrokePatchBuilder() {
        fTarget->putBackVertices(fCurrChunkVertexCapacity - fVertexChunkArray->back().fVertexCount,
                                 sizeof(SkPoint));
    }

    void addPath(const SkPath&, const SkStrokeRec&);

private:
    void allocVertexChunk(int minVertexAllocCount);
    SkPoint* reservePatch();

    // Join types are written as floats in P4.x. See GrStrokeTessellateShader for definitions.
    void writeCubicSegment(float leftJoinType, const SkPoint pts[4], float overrideNumSegments = 0);
    void writeCubicSegment(float leftJoinType, const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                           const Sk2f& p3, float overrideNumSegments = 0) {
        SkPoint pts[4];
        p0.store(&pts[0]);
        p1.store(&pts[1]);
        p2.store(&pts[2]);
        p3.store(&pts[3]);
        this->writeCubicSegment(leftJoinType, pts, overrideNumSegments);
    }
    void writeJoin(float joinType, const SkPoint& anchorPoint, const SkPoint& prevControlPoint,
                   const SkPoint& nextControlPoint);
    void writeSquareCap(const SkPoint& endPoint, const SkPoint& controlPoint);
    void writeCaps();

    void beginPath(const SkStrokeRec&);
    void moveTo(const SkPoint&);
    void lineTo(const SkPoint& p0, const SkPoint& p1);
    void quadraticTo(const SkPoint[3]);
    void cubicTo(const SkPoint[4]);
    void close();

    void lineTo(float leftJoinType, const SkPoint& p0, const SkPoint& p1);
    void quadraticTo(float leftJoinType, const SkPoint[3], float maxCurvatureT);

    static constexpr float kLeftMaxCurvatureNone = 1;
    static constexpr float kRightMaxCurvatureNone = 0;
    void cubicTo(float leftJoinType, const SkPoint[4], float maxCurvatureT, float leftMaxCurvatureT,
                 float rightMaxCurvatureT);

    // TEMPORARY: Rotates the current control point without changing the current position.
    // This is used when we convert a curve to a lineTo, and that behavior will soon go away.
    void rotateTo(float leftJoinType, const SkPoint& anchorPoint, const SkPoint& controlPoint);

    // These are raw pointers whose lifetimes are controlled outside this class.
    GrMeshDrawOp::Target* const fTarget;
    SkTArray<VertexChunk>* const fVertexChunkArray;

    const int fMaxTessellationSegments;
    const float fMatrixScale;

    // Variables related to the vertex chunk that we are currently filling.
    int fCurrChunkVertexCapacity;
    int fCurrChunkMinVertexAllocCount;
    SkPoint* fCurrChunkVertexData;

    // Variables related to the path that we are currently iterating.
    float fCurrStrokeRadius;
    float fCurrStrokeJoinType;  // See GrStrokeTessellateShader for join type definitions .
    SkPaint::Cap fCurrStrokeCapType;
    // Any curvature on the original curve gets magnified on the outer edge of the stroke,
    // proportional to how thick the stroke radius is. This field tells us the maximum curvature we
    // can tolerate using the current stroke radius, before linearization artifacts begin to appear
    // on the outer edge.
    //
    // (Curvature this strong is quite rare in practice, but when it does happen, we decompose the
    // section with strong curvature into lineTo's with round joins in between.)
    float fMaxCurvatureCosTheta;

    // Variables related to the specific contour that we are currently iterating.
    bool fHasPreviousSegment = false;
    SkPoint fCurrContourStartPoint;
    SkPoint fCurrContourFirstControlPoint;
    SkPoint fLastControlPoint;
    SkPoint fCurrentPoint;
};

#endif
