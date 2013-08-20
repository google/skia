/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathUtils_DEFINED
#define GrPathUtils_DEFINED

#include "GrPoint.h"
#include "SkRect.h"
#include "SkPath.h"
#include "SkTArray.h"

class SkMatrix;

/**
 *  Utilities for evaluating paths.
 */
namespace GrPathUtils {
    SkScalar scaleToleranceToSrc(SkScalar devTol,
                                 const SkMatrix& viewM,
                                 const SkRect& pathBounds);

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

    // Input is 3 control points and a weight for a bezier conic. Calculates the
    // three linear functionals (K,L,M) that represent the implicit equation of the
    // conic, K^2 - LM.
    //
    // Output:
    //  K = (klm[0], klm[1], klm[2])
    //  L = (klm[3], klm[4], klm[5])
    //  M = (klm[6], klm[7], klm[8])
    void getConicKLM(const SkPoint p[3], const SkScalar weight, SkScalar klm[9]);

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

    // Chops the cubic bezier passed in by src, at the double point (intersection point)
    // if the curve is a cubic loop. If it is a loop, there will be two parametric values for
    // the double point: ls and ms. We chop the cubic at these values if they are between 0 and 1.
    // Return value:
    // Value of 3: ls and ms are both between (0,1), and dst will contain the three cubics,
    //             dst[0..3], dst[3..6], and dst[6..9] if dst is not NULL
    // Value of 2: Only one of ls and ms are between (0,1), and dst will contain the two cubics,
    //             dst[0..3] and dst[3..6] if dst is not NULL
    // Value of 1: Neither ls or ms are between (0,1), and dst will contain the one original cubic,
    //             dst[0..3] if dst is not NULL
    //
    // Optional KLM Calculation:
    // The function can also return the KLM linear functionals for the chopped cubic implicit form
    // of K^3 - LM.
    // It will calculate a single set of KLM values that can be shared by all sub cubics, except
    // for the subsection that is "the loop" the K and L values need to be negated.
    // Output:
    // klm:     Holds the values for the linear functionals as:
    //          K = (klm[0], klm[1], klm[2])
    //          L = (klm[3], klm[4], klm[5])
    //          M = (klm[6], klm[7], klm[8])
    // klm_rev: These values are flags for the corresponding sub cubic saying whether or not
    //          the K and L values need to be flipped. A value of -1.f means flip K and L and
    //          a value of 1.f means do nothing.
    //          *****DO NOT FLIP M, JUST K AND L*****
    //
    // Notice that the klm lines are calculated in the same space as the input control points.
    // If you transform the points the lines will also need to be transformed. This can be done
    // by mapping the lines with the inverse-transpose of the matrix used to map the points.
    int chopCubicAtLoopIntersection(const SkPoint src[4], SkPoint dst[10] = NULL,
                                    SkScalar klm[9] = NULL, SkScalar klm_rev[3] = NULL);

    // Input is p which holds the 4 control points of a non-rational cubic Bezier curve.
    // Output is the coefficients of the three linear functionals K, L, & M which
    // represent the implicit form of the cubic as f(x,y,w) = K^3 - LM. The w term
    // will always be 1. The output is stored in the array klm, where the values are:
    // K = (klm[0], klm[1], klm[2])
    // L = (klm[3], klm[4], klm[5])
    // M = (klm[6], klm[7], klm[8])
    //
    // Notice that the klm lines are calculated in the same space as the input control points.
    // If you transform the points the lines will also need to be transformed. This can be done
    // by mapping the lines with the inverse-transpose of the matrix used to map the points.
    void getCubicKLM(const SkPoint p[4], SkScalar klm[9]);
};
#endif
