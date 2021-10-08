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
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/GrVx.h"

class SkMatrix;

/**
 *  Utilities for evaluating paths.
 */
namespace GrPathUtils {

// When tessellating curved paths into linear segments, this defines the maximum distance in screen
// space which a segment may deviate from the mathematically correct value. Above this value, the
// segment will be subdivided.
// This value was chosen to approximate the supersampling accuracy of the raster path (16 samples,
// or one quarter pixel).
static const SkScalar kDefaultTolerance = SkDoubleToScalar(0.25);

// We guarantee that no quad or cubic will ever produce more than this many points
static const int kMaxPointsPerCurve = 1 << 10;

// Very small tolerances will be increased to a minimum threshold value, to avoid division problems
// in subsequent math.
SkScalar scaleToleranceToSrc(SkScalar devTol,
                             const SkMatrix& viewM,
                             const SkRect& pathBounds);

// Returns the maximum number of vertices required when using a recursive chopping algorithm to
// linearize the quadratic Bezier (e.g. generateQuadraticPoints below) to the given error tolerance.
// This is a power of two and will not exceed kMaxPointsPerCurve.
uint32_t quadraticPointCount(const SkPoint points[], SkScalar tol);

// Returns the number of points actually written to 'points', will be <= to 'pointsLeft'
uint32_t generateQuadraticPoints(const SkPoint& p0,
                                 const SkPoint& p1,
                                 const SkPoint& p2,
                                 SkScalar tolSqd,
                                 SkPoint** points,
                                 uint32_t pointsLeft);

// Returns the maximum number of vertices required when using a recursive chopping algorithm to
// linearize the cubic Bezier (e.g. generateQuadraticPoints below) to the given error tolerance.
// This is a power of two and will not exceed kMaxPointsPerCurve.
uint32_t cubicPointCount(const SkPoint points[], SkScalar tol);

// Returns the number of points actually written to 'points', will be <= to 'pointsLeft'
uint32_t generateCubicPoints(const SkPoint& p0,
                             const SkPoint& p1,
                             const SkPoint& p2,
                             const SkPoint& p3,
                             SkScalar tolSqd,
                             SkPoint** points,
                             uint32_t pointsLeft);

// A 2x3 matrix that goes from the 2d space coordinates to UV space where u^2-v = 0 specifies the
// quad. The matrix is determined by the control points of the quadratic.
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

// Input is 3 control points and a weight for a bezier conic. Calculates the three linear
// functionals (K,L,M) that represent the implicit equation of the conic, k^2 - lm.
//
// Output: klm holds the linear functionals K,L,M as row vectors:
//
//     | ..K.. |   | x |      | k |
//     | ..L.. | * | y |  ==  | l |
//     | ..M.. |   | 1 |      | m |
//
void getConicKLM(const SkPoint p[3], const SkScalar weight, SkMatrix* klm);

// Converts a cubic into a sequence of quads. If working in device space use tolScale = 1, otherwise
// set based on stretchiness of the matrix. The result is sets of 3 points in quads. This will
// preserve the starting and ending tangent vectors (modulo FP precision).
void convertCubicToQuads(const SkPoint p[4],
                         SkScalar tolScale,
                         SkTArray<SkPoint, true>* quads);

// When we approximate a cubic {a,b,c,d} with a quadratic we may have to ensure that the new control
// point lies between the lines ab and cd. The convex path renderer requires this. It starts with a
// path where all the control points taken together form a convex polygon. It relies on this
// property and the quadratic approximation of cubics step cannot alter it. This variation enforces
// this constraint. The cubic must be simple and dir must specify the orientation of the contour
// containing the cubic.
void convertCubicToQuadsConstrainToTangents(const SkPoint p[4],
                                            SkScalar tolScale,
                                            SkPathFirstDirection dir,
                                            SkTArray<SkPoint, true>* quads);

// Converts the given line to a cubic bezier.
// NOTE: This method interpolates at 1/3 and 2/3, but if suitable in context, the cubic
// {p0, p0, p1, p1} may also work.
inline void writeLineAsCubic(SkPoint startPt, SkPoint endPt, GrVertexWriter* writer) {
    using grvx::float2, skvx::bit_pun;
    float2 p0 = bit_pun<float2>(startPt);
    float2 p1 = bit_pun<float2>(endPt);
    float2 v = (p1 - p0) * (1/3.f);
    writer->write(p0, p0 + v, p1 - v, p1);
}

// Converts the given quadratic bezier to a cubic.
inline void writeQuadAsCubic(const SkPoint p[3], GrVertexWriter* writer) {
    using grvx::float2, skvx::bit_pun;
    float2 p0 = bit_pun<float2>(p[0]);
    float2 p1 = bit_pun<float2>(p[1]);
    float2 p2 = bit_pun<float2>(p[2]);
    float2 c = p1 * (2/3.f);
    writer->write(p0, p0*(1/3.f) + c, p2 * (1/3.f) + c, p2);
}
inline void convertQuadToCubic(const SkPoint p[3], SkPoint out[4]) {
    GrVertexWriter writer(out);
    writeQuadAsCubic(p, &writer);
}

// Finds 0, 1, or 2 T values at which to chop the given curve in order to guarantee the resulting
// cubics are convex and rotate no more than 180 degrees.
//
//   - If the cubic is "serpentine", then the T values are any inflection points in [0 < T < 1].
//   - If the cubic is linear, then the T values are any 180-degree cusp points in [0 < T < 1].
//   - Otherwise the T value is the point at which rotation reaches 180 degrees, iff in [0 < T < 1].
//
// 'areCusps' is set to true if the chop point occurred at a cusp (within tolerance), or if the chop
// point(s) occurred at 180-degree turnaround points on a degenerate flat line.
int findCubicConvex180Chops(const SkPoint[], float T[2], bool* areCusps);

// Returns true if the given conic (or quadratic) has a cusp point. The w value is not necessary in
// determining this. If there is a cusp, it can be found at the midtangent.
inline bool conicHasCusp(const SkPoint p[3]) {
    SkVector a = p[1] - p[0];
    SkVector b = p[2] - p[1];
    // A conic of any class can only have a cusp if it is a degenerate flat line with a 180 degree
    // turnarund. To detect this, the beginning and ending tangents must be parallel
    // (a.cross(b) == 0) and pointing in opposite directions (a.dot(b) < 0).
    return a.cross(b) == 0 && a.dot(b) < 0;
}

}  // namespace GrPathUtils

#endif
