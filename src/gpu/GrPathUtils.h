/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathUtils_DEFINED
#define GrPathUtils_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkTArray.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"

class SkMatrix;

/**
 *  Utilities for evaluating paths.
 */
namespace GrPathUtils {
    // Very small tolerances will be increased to a minimum threshold value, to avoid division
    // problems in subsequent math.
    SkScalar scaleToleranceToSrc(SkScalar devTol,
                                 const SkMatrix& viewM,
                                 const SkRect& pathBounds);

    int worstCasePointCount(const SkPath&,
                            int* subpaths,
                            SkScalar tol);

    uint32_t quadraticPointCount(const SkPoint points[], SkScalar tol);

    uint32_t generateQuadraticPoints(const SkPoint& p0,
                                     const SkPoint& p1,
                                     const SkPoint& p2,
                                     SkScalar tolSqd,
                                     SkPoint** points,
                                     uint32_t pointsLeft);

    uint32_t cubicPointCount(const SkPoint points[], SkScalar tol);

    uint32_t generateCubicPoints(const SkPoint& p0,
                                 const SkPoint& p1,
                                 const SkPoint& p2,
                                 const SkPoint& p3,
                                 SkScalar tolSqd,
                                 SkPoint** points,
                                 uint32_t pointsLeft);

    // A 2x3 matrix that goes from the 2d space coordinates to UV space where
    // u^2-v = 0 specifies the quad. The matrix is determined by the control
    // points of the quadratic.
    class QuadUVMatrix {
    public:
        QuadUVMatrix() {}
        // Initialize the matrix from the control pts
        QuadUVMatrix(const SkPoint controlPts[3]) { this->set(controlPts); }
        void set(const SkPoint controlPts[3]);

        /**
         * Applies the matrix to vertex positions to compute UV coords.
         *
         * vertices is a pointer to the first vertex.
         * vertexCount is the number of vertices.
         * stride is the size of each vertex.
         * uvOffset is the offset of the UV values within each vertex.
         */
        void apply(void* vertices, int vertexCount, size_t stride, size_t uvOffset) const {
            intptr_t xyPtr = reinterpret_cast<intptr_t>(vertices);
            intptr_t uvPtr = reinterpret_cast<intptr_t>(vertices) + uvOffset;
            float sx = fM[0];
            float kx = fM[1];
            float tx = fM[2];
            float ky = fM[3];
            float sy = fM[4];
            float ty = fM[5];
            for (int i = 0; i < vertexCount; ++i) {
                const SkPoint* xy = reinterpret_cast<const SkPoint*>(xyPtr);
                SkPoint* uv = reinterpret_cast<SkPoint*>(uvPtr);
                uv->fX = sx * xy->fX + kx * xy->fY + tx;
                uv->fY = ky * xy->fX + sy * xy->fY + ty;
                xyPtr += stride;
                uvPtr += stride;
            }
        }
    private:
        float fM[6];
    };

    // Input is 3 control points and a weight for a bezier conic. Calculates the
    // three linear functionals (K,L,M) that represent the implicit equation of the
    // conic, k^2 - lm.
    //
    // Output: klm holds the linear functionals K,L,M as row vectors:
    //
    //     | ..K.. |   | x |      | k |
    //     | ..L.. | * | y |  ==  | l |
    //     | ..M.. |   | 1 |      | m |
    //
    void getConicKLM(const SkPoint p[3], const SkScalar weight, SkMatrix* klm);

    // Converts a cubic into a sequence of quads. If working in device space
    // use tolScale = 1, otherwise set based on stretchiness of the matrix. The
    // result is sets of 3 points in quads. This will preserve the starting and
    // ending tangent vectors (modulo FP precision).
    void convertCubicToQuads(const SkPoint p[4],
                             SkScalar tolScale,
                             SkTArray<SkPoint, true>* quads);

    // When we approximate a cubic {a,b,c,d} with a quadratic we may have to
    // ensure that the new control point lies between the lines ab and cd. The
    // convex path renderer requires this. It starts with a path where all the
    // control points taken together form a convex polygon. It relies on this
    // property and the quadratic approximation of cubics step cannot alter it.
    // This variation enforces this constraint. The cubic must be simple and dir
    // must specify the orientation of the contour containing the cubic.
    void convertCubicToQuadsConstrainToTangents(const SkPoint p[4],
                                                SkScalar tolScale,
                                                SkPathPriv::FirstDirection dir,
                                                SkTArray<SkPoint, true>* quads);

    enum class ExcludedTerm {
        kNonInvertible,
        kQuadraticTerm,
        kLinearTerm
    };

    // Computes the inverse-transpose of the cubic's power basis matrix, after removing a specific
    // row of coefficients.
    //
    // E.g. if the cubic is defined in power basis form as follows:
    //
    //                                         | x3   y3   0 |
    //     C(t,s) = [t^3  t^2*s  t*s^2  s^3] * | x2   y2   0 |
    //                                         | x1   y1   0 |
    //                                         | x0   y0   1 |
    //
    // And the excluded term is "kQuadraticTerm", then the resulting inverse-transpose will be:
    //
    //     | x3   y3   0 | -1 T
    //     | x1   y1   0 |
    //     | x0   y0   1 |
    //
    // (The term to exclude is chosen based on maximizing the resulting matrix determinant.)
    //
    // This can be used to find the KLM linear functionals:
    //
    //     | ..K.. |   | ..kcoeffs.. |
    //     | ..L.. | = | ..lcoeffs.. | * inverse_transpose_power_basis_matrix
    //     | ..M.. |   | ..mcoeffs.. |
    //
    // NOTE: the same term that was excluded here must also be removed from the corresponding column
    // of the klmcoeffs matrix.
    //
    // Returns which row of coefficients was removed, or kNonInvertible if the cubic was degenerate.
    ExcludedTerm calcCubicInverseTransposePowerBasisMatrix(const SkPoint p[4], SkMatrix* out);

    // Computes the KLM linear functionals for the cubic implicit form. The "right" side of the
    // curve (when facing in the direction of increasing parameter values) will be the area that
    // satisfies:
    //
    //     k^3 < l*m
    //
    // Output:
    //
    // klm: Holds the linear functionals K,L,M as row vectors:
    //
    //          | ..K.. |   | x |      | k |
    //          | ..L.. | * | y |  ==  | l |
    //          | ..M.. |   | 1 |      | m |
    //
    // NOTE: the KLM lines are calculated in the same space as the input control points. If you
    // transform the points the lines will also need to be transformed. This can be done by mapping
    // the lines with the inverse-transpose of the matrix used to map the points.
    //
    // t[],s[]: These are set to the two homogeneous parameter values at which points the lines L&M
    // intersect with K (See SkClassifyCubic).
    //
    // Returns the cubic's classification.
    SkCubicType getCubicKLM(const SkPoint src[4], SkMatrix* klm, double t[2], double s[2]);

    // Chops the cubic bezier passed in by src, at the double point (intersection point)
    // if the curve is a cubic loop. If it is a loop, there will be two parametric values for
    // the double point: t1 and t2. We chop the cubic at these values if they are between 0 and 1.
    // Return value:
    // Value of 3: t1 and t2 are both between (0,1), and dst will contain the three cubics,
    //             dst[0..3], dst[3..6], and dst[6..9] if dst is not nullptr
    // Value of 2: Only one of t1 and t2 are between (0,1), and dst will contain the two cubics,
    //             dst[0..3] and dst[3..6] if dst is not nullptr
    // Value of 1: Neither t1 nor t2 are between (0,1), and dst will contain the one original cubic,
    //             src[0..3]
    //
    // Output:
    //
    // klm: Holds the linear functionals K,L,M as row vectors. (See getCubicKLM().)
    //
    // loopIndex: This value will tell the caller which of the chopped sections (if any) are the
    //            actual loop. A value of -1 means there is no loop section. The caller can then use
    //            this value to decide how/if they want to flip the orientation of this section.
    //            The flip should be done by negating the k and l values as follows:
    //
    //            KLM.postScale(-1, -1)
    int chopCubicAtLoopIntersection(const SkPoint src[4], SkPoint dst[10], SkMatrix* klm,
                                    int* loopIndex);

    // When tessellating curved paths into linear segments, this defines the maximum distance
    // in screen space which a segment may deviate from the mathmatically correct value.
    // Above this value, the segment will be subdivided.
    // This value was chosen to approximate the supersampling accuracy of the raster path (16
    // samples, or one quarter pixel).
    static const SkScalar kDefaultTolerance = SkDoubleToScalar(0.25);

    // We guarantee that no quad or cubic will ever produce more than this many points
    static const int kMaxPointsPerCurve = 1 << 10;
};
#endif
