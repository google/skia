
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPathUtils_DEFINED
#define GrPathUtils_DEFINED

#include "GrRect.h"
#include "SkPath.h"
#include "SkTArray.h"

class SkMatrix;

/**
 *  Utilities for evaluating paths.
 */
namespace GrPathUtils {
    SkScalar scaleToleranceToSrc(SkScalar devTol,
                                 const SkMatrix& viewM,
                                 const GrRect& pathBounds);

    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    int worstCasePointCount(const SkPath&,
                            int* subpaths,
                            SkScalar tol);

    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    uint32_t quadraticPointCount(const GrPoint points[], SkScalar tol);

    uint32_t generateQuadraticPoints(const GrPoint& p0,
                                     const GrPoint& p1,
                                     const GrPoint& p2,
                                     SkScalar tolSqd,
                                     GrPoint** points,
                                     uint32_t pointsLeft);

    /// Since we divide by tol if we're computing exact worst-case bounds,
    /// very small tolerances will be increased to gMinCurveTol.
    uint32_t cubicPointCount(const GrPoint points[], SkScalar tol);

    uint32_t generateCubicPoints(const GrPoint& p0,
                                 const GrPoint& p1,
                                 const GrPoint& p2,
                                 const GrPoint& p3,
                                 SkScalar tolSqd,
                                 GrPoint** points,
                                 uint32_t pointsLeft);

    // A 2x3 matrix that goes from the 2d space coordinates to UV space where
    // u^2-v = 0 specifies the quad. The matrix is determined by the control
    // points of the quadratic.
    class QuadUVMatrix {
    public:
        QuadUVMatrix() {};
        // Initialize the matrix from the control pts
        QuadUVMatrix(const GrPoint controlPts[3]) { this->set(controlPts); }
        void set(const GrPoint controlPts[3]);

        /**
         * Applies the matrix to vertex positions to compute UV coords. This
         * has been templated so that the compiler can easliy unroll the loop
         * and reorder to avoid stalling for loads. The assumption is that a
         * path renderer will have a small fixed number of vertices that it
         * uploads for each quad.
         *
         * N is the number of vertices.
         * STRIDE is the size of each vertex.
         * UV_OFFSET is the offset of the UV values within each vertex.
         * vertices is a pointer to the first vertex.
         */
        template <int N, size_t STRIDE, size_t UV_OFFSET>
        void apply(const void* vertices) {
            intptr_t xyPtr = reinterpret_cast<intptr_t>(vertices);
            intptr_t uvPtr = reinterpret_cast<intptr_t>(vertices) + UV_OFFSET;
            float sx = fM[0];
            float kx = fM[1];
            float tx = fM[2];
            float ky = fM[3];
            float sy = fM[4];
            float ty = fM[5];
            for (int i = 0; i < N; ++i) {
                const GrPoint* xy = reinterpret_cast<const GrPoint*>(xyPtr);
                GrPoint* uv = reinterpret_cast<GrPoint*>(uvPtr);
                uv->fX = sx * xy->fX + kx * xy->fY + tx;
                uv->fY = ky * xy->fX + sy * xy->fY + ty;
                xyPtr += STRIDE;
                uvPtr += STRIDE;
            }
        }
    private:
        float fM[6];
    };


    // Converts a cubic into a sequence of quads. If working in device space
    // use tolScale = 1, otherwise set based on stretchiness of the matrix. The
    // result is sets of 3 points in quads (TODO: share endpoints in returned
    // array)
    // When we approximate a cubic {a,b,c,d} with a quadratic we may have to
    // ensure that the new control point lies between the lines ab and cd. The
    // convex path renderer requires this. It starts with a path where all the
    // control points taken together form a convex polygon. It relies on this
    // property and the quadratic approximation of cubics step cannot alter it.
    // Setting constrainWithinTangents to true enforces this property. When this
    // is true the cubic must be simple and dir must specify the orientation of
    // the cubic. Otherwise, dir is ignored.
    void convertCubicToQuads(const GrPoint p[4],
                             SkScalar tolScale,
                             bool constrainWithinTangents,
                             SkPath::Direction dir,
                             SkTArray<SkPoint, true>* quads);
};
#endif
