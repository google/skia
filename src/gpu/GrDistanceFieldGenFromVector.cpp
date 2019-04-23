/*
 * Copyright 2017 ARM Ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDistanceFieldGen.h"
#include "src/gpu/GrDistanceFieldGenFromVector.h"

#include "include/core/SkMatrix.h"
#include "include/gpu/GrConfig.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrPathUtils.h"

/**
 * If a scanline (a row of texel) cross from the kRight_SegSide
 * of a segment to the kLeft_SegSide, the winding score should
 * add 1.
 * And winding score should subtract 1 if the scanline cross
 * from kLeft_SegSide to kRight_SegSide.
 * Always return kNA_SegSide if the scanline does not cross over
 * the segment. Winding score should be zero in this case.
 * You can get the winding number for each texel of the scanline
 * by adding the winding score from left to right.
 * Assuming we always start from outside, so the winding number
 * should always start from zero.
 *      ________         ________
 *     |        |       |        |
 * ...R|L......L|R.....L|R......R|L..... <= Scanline & side of segment
 *     |+1      |-1     |-1      |+1     <= Winding score
 *   0 |   1    ^   0   ^  -1    |0      <= Winding number
 *     |________|       |________|
 *
 * .......NA................NA..........
 *         0                 0
 */
enum SegSide {
    kLeft_SegSide  = -1,
    kOn_SegSide    =  0,
    kRight_SegSide =  1,
    kNA_SegSide    =  2,
};

struct DFData {
    float fDistSq;            // distance squared to nearest (so far) edge
    int   fDeltaWindingScore; // +1 or -1 whenever a scanline cross over a segment
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Type definition for double precision DPoint and DAffineMatrix
 */

// Point with double precision
struct DPoint {
    double fX, fY;

    static DPoint Make(double x, double y) {
        DPoint pt;
        pt.set(x, y);
        return pt;
    }

    double x() const { return fX; }
    double y() const { return fY; }

    void set(double x, double y) { fX = x; fY = y; }

    /** Returns the euclidian distance from (0,0) to (x,y)
    */
    static double Length(double x, double y) {
        return sqrt(x * x + y * y);
    }

    /** Returns the euclidian distance between a and b
    */
    static double Distance(const DPoint& a, const DPoint& b) {
        return Length(a.fX - b.fX, a.fY - b.fY);
    }

    double distanceToSqd(const DPoint& pt) const {
        double dx = fX - pt.fX;
        double dy = fY - pt.fY;
        return dx * dx + dy * dy;
    }
};

// Matrix with double precision for affine transformation.
// We don't store row 3 because its always (0, 0, 1).
class DAffineMatrix {
public:
    double operator[](int index) const {
        SkASSERT((unsigned)index < 6);
        return fMat[index];
    }

    double& operator[](int index) {
        SkASSERT((unsigned)index < 6);
        return fMat[index];
    }

    void setAffine(double m11, double m12, double m13,
                   double m21, double m22, double m23) {
        fMat[0] = m11;
        fMat[1] = m12;
        fMat[2] = m13;
        fMat[3] = m21;
        fMat[4] = m22;
        fMat[5] = m23;
    }

    /** Set the matrix to identity
    */
    void reset() {
        fMat[0] = fMat[4] = 1.0;
        fMat[1] = fMat[3] =
        fMat[2] = fMat[5] = 0.0;
    }

    // alias for reset()
    void setIdentity() { this->reset(); }

    DPoint mapPoint(const SkPoint& src) const {
        DPoint pt = DPoint::Make(src.x(), src.y());
        return this->mapPoint(pt);
    }

    DPoint mapPoint(const DPoint& src) const {
        return DPoint::Make(fMat[0] * src.x() + fMat[1] * src.y() + fMat[2],
                            fMat[3] * src.x() + fMat[4] * src.y() + fMat[5]);
    }
private:
    double fMat[6];
};

///////////////////////////////////////////////////////////////////////////////

static const double kClose = (SK_Scalar1 / 16.0);
static const double kCloseSqd = kClose * kClose;
static const double kNearlyZero = (SK_Scalar1 / (1 << 18));
static const double kTangentTolerance = (SK_Scalar1 / (1 << 11));
static const float  kConicTolerance = 0.25f;

static inline bool between_closed_open(double a, double b, double c,
                                       double tolerance = 0.0,
                                       bool xformToleranceToX = false) {
    SkASSERT(tolerance >= 0.0);
    double tolB = tolerance;
    double tolC = tolerance;

    if (xformToleranceToX) {
        // Canonical space is y = x^2 and the derivative of x^2 is 2x.
        // So the slope of the tangent line at point (x, x^2) is 2x.
        //
        //                          /|
        //  sqrt(2x * 2x + 1 * 1)  / | 2x
        //                        /__|
        //                         1
        tolB = tolerance / sqrt(4.0 * b * b + 1.0);
        tolC = tolerance / sqrt(4.0 * c * c + 1.0);
    }
    return b < c ? (a >= b - tolB && a < c - tolC) :
                   (a >= c - tolC && a < b - tolB);
}

static inline bool between_closed(double a, double b, double c,
                                  double tolerance = 0.0,
                                  bool xformToleranceToX = false) {
    SkASSERT(tolerance >= 0.0);
    double tolB = tolerance;
    double tolC = tolerance;

    if (xformToleranceToX) {
        tolB = tolerance / sqrt(4.0 * b * b + 1.0);
        tolC = tolerance / sqrt(4.0 * c * c + 1.0);
    }
    return b < c ? (a >= b - tolB && a <= c + tolC) :
                   (a >= c - tolC && a <= b + tolB);
}

static inline bool nearly_zero(double x, double tolerance = kNearlyZero) {
    SkASSERT(tolerance >= 0.0);
    return fabs(x) <= tolerance;
}

static inline bool nearly_equal(double x, double y,
                                double tolerance = kNearlyZero,
                                bool xformToleranceToX = false) {
    SkASSERT(tolerance >= 0.0);
    if (xformToleranceToX) {
        tolerance = tolerance / sqrt(4.0 * y * y + 1.0);
    }
    return fabs(x - y) <= tolerance;
}

static inline double sign_of(const double &val) {
    return (val < 0.0) ? -1.0 : 1.0;
}

static bool is_colinear(const SkPoint pts[3]) {
    return nearly_zero((pts[1].y() - pts[0].y()) * (pts[1].x() - pts[2].x()) -
                       (pts[1].y() - pts[2].y()) * (pts[1].x() - pts[0].x()), kCloseSqd);
}

class PathSegment {
public:
    enum {
        // These enum values are assumed in member functions below.
        kLine = 0,
        kQuad = 1,
    } fType;

    // line uses 2 pts, quad uses 3 pts
    SkPoint fPts[3];

    DPoint  fP0T, fP2T;
    DAffineMatrix fXformMatrix;
    double fScalingFactor;
    double fScalingFactorSqd;
    double fNearlyZeroScaled;
    double fTangentTolScaledSqd;
    SkRect  fBoundingBox;

    void init();

    int countPoints() {
        GR_STATIC_ASSERT(0 == kLine && 1 == kQuad);
        return fType + 2;
    }

    const SkPoint& endPt() const {
        GR_STATIC_ASSERT(0 == kLine && 1 == kQuad);
        return fPts[fType + 1];
    }
};

typedef SkTArray<PathSegment, true> PathSegmentArray;

void PathSegment::init() {
    const DPoint p0 = DPoint::Make(fPts[0].x(), fPts[0].y());
    const DPoint p2 = DPoint::Make(this->endPt().x(), this->endPt().y());
    const double p0x = p0.x();
    const double p0y = p0.y();
    const double p2x = p2.x();
    const double p2y = p2.y();

    fBoundingBox.set(fPts[0], this->endPt());

    if (fType == PathSegment::kLine) {
        fScalingFactorSqd = fScalingFactor = 1.0;
        double hypotenuse = DPoint::Distance(p0, p2);

        const double cosTheta = (p2x - p0x) / hypotenuse;
        const double sinTheta = (p2y - p0y) / hypotenuse;

        fXformMatrix.setAffine(
            cosTheta, sinTheta, -(cosTheta * p0x) - (sinTheta * p0y),
            -sinTheta, cosTheta, (sinTheta * p0x) - (cosTheta * p0y)
        );
    } else {
        SkASSERT(fType == PathSegment::kQuad);

        // Calculate bounding box
        const SkPoint _P1mP0 = fPts[1] - fPts[0];
        SkPoint t = _P1mP0 - fPts[2] + fPts[1];
        t.fX = _P1mP0.x() / t.x();
        t.fY = _P1mP0.y() / t.y();
        t.fX = SkScalarClampMax(t.x(), 1.0);
        t.fY = SkScalarClampMax(t.y(), 1.0);
        t.fX = _P1mP0.x() * t.x();
        t.fY = _P1mP0.y() * t.y();
        const SkPoint m = fPts[0] + t;
        SkRectPriv::GrowToInclude(&fBoundingBox, m);

        const double p1x = fPts[1].x();
        const double p1y = fPts[1].y();

        const double p0xSqd = p0x * p0x;
        const double p0ySqd = p0y * p0y;
        const double p2xSqd = p2x * p2x;
        const double p2ySqd = p2y * p2y;
        const double p1xSqd = p1x * p1x;
        const double p1ySqd = p1y * p1y;

        const double p01xProd = p0x * p1x;
        const double p02xProd = p0x * p2x;
        const double b12xProd = p1x * p2x;
        const double p01yProd = p0y * p1y;
        const double p02yProd = p0y * p2y;
        const double b12yProd = p1y * p2y;

        const double sqrtA = p0y - (2.0 * p1y) + p2y;
        const double a = sqrtA * sqrtA;
        const double h = -1.0 * (p0y - (2.0 * p1y) + p2y) * (p0x - (2.0 * p1x) + p2x);
        const double sqrtB = p0x - (2.0 * p1x) + p2x;
        const double b = sqrtB * sqrtB;
        const double c = (p0xSqd * p2ySqd) - (4.0 * p01xProd * b12yProd)
                - (2.0 * p02xProd * p02yProd) + (4.0 * p02xProd * p1ySqd)
                + (4.0 * p1xSqd * p02yProd) - (4.0 * b12xProd * p01yProd)
                + (p2xSqd * p0ySqd);
        const double g = (p0x * p02yProd) - (2.0 * p0x * p1ySqd)
                + (2.0 * p0x * b12yProd) - (p0x * p2ySqd)
                + (2.0 * p1x * p01yProd) - (4.0 * p1x * p02yProd)
                + (2.0 * p1x * b12yProd) - (p2x * p0ySqd)
                + (2.0 * p2x * p01yProd) + (p2x * p02yProd)
                - (2.0 * p2x * p1ySqd);
        const double f = -((p0xSqd * p2y) - (2.0 * p01xProd * p1y)
                - (2.0 * p01xProd * p2y) - (p02xProd * p0y)
                + (4.0 * p02xProd * p1y) - (p02xProd * p2y)
                + (2.0 * p1xSqd * p0y) + (2.0 * p1xSqd * p2y)
                - (2.0 * b12xProd * p0y) - (2.0 * b12xProd * p1y)
                + (p2xSqd * p0y));

        const double cosTheta = sqrt(a / (a + b));
        const double sinTheta = -1.0 * sign_of((a + b) * h) * sqrt(b / (a + b));

        const double gDef = cosTheta * g - sinTheta * f;
        const double fDef = sinTheta * g + cosTheta * f;


        const double x0 = gDef / (a + b);
        const double y0 = (1.0 / (2.0 * fDef)) * (c - (gDef * gDef / (a + b)));


        const double lambda = -1.0 * ((a + b) / (2.0 * fDef));
        fScalingFactor = fabs(1.0 / lambda);
        fScalingFactorSqd = fScalingFactor * fScalingFactor;

        const double lambda_cosTheta = lambda * cosTheta;
        const double lambda_sinTheta = lambda * sinTheta;

        fXformMatrix.setAffine(
            lambda_cosTheta, -lambda_sinTheta, lambda * x0,
            lambda_sinTheta, lambda_cosTheta, lambda * y0
        );
    }

    fNearlyZeroScaled = kNearlyZero / fScalingFactor;
    fTangentTolScaledSqd = kTangentTolerance * kTangentTolerance / fScalingFactorSqd;

    fP0T = fXformMatrix.mapPoint(p0);
    fP2T = fXformMatrix.mapPoint(p2);
}

static void init_distances(DFData* data, int size) {
    DFData* currData = data;

    for (int i = 0; i < size; ++i) {
        // init distance to "far away"
        currData->fDistSq = SK_DistanceFieldMagnitude * SK_DistanceFieldMagnitude;
        currData->fDeltaWindingScore = 0;
        ++currData;
    }
}

static inline void add_line_to_segment(const SkPoint pts[2],
                                       PathSegmentArray* segments) {
    segments->push_back();
    segments->back().fType = PathSegment::kLine;
    segments->back().fPts[0] = pts[0];
    segments->back().fPts[1] = pts[1];

    segments->back().init();
}

static inline void add_quad_segment(const SkPoint pts[3],
                                    PathSegmentArray* segments) {
    if (SkPointPriv::DistanceToSqd(pts[0], pts[1]) < kCloseSqd ||
        SkPointPriv::DistanceToSqd(pts[1], pts[2]) < kCloseSqd ||
        is_colinear(pts)) {
        if (pts[0] != pts[2]) {
            SkPoint line_pts[2];
            line_pts[0] = pts[0];
            line_pts[1] = pts[2];
            add_line_to_segment(line_pts, segments);
        }
    } else {
        segments->push_back();
        segments->back().fType = PathSegment::kQuad;
        segments->back().fPts[0] = pts[0];
        segments->back().fPts[1] = pts[1];
        segments->back().fPts[2] = pts[2];

        segments->back().init();
    }
}

static inline void add_cubic_segments(const SkPoint pts[4],
                                      PathSegmentArray* segments) {
    SkSTArray<15, SkPoint, true> quads;
    GrPathUtils::convertCubicToQuads(pts, SK_Scalar1, &quads);
    int count = quads.count();
    for (int q = 0; q < count; q += 3) {
        add_quad_segment(&quads[q], segments);
    }
}

static float calculate_nearest_point_for_quad(
                const PathSegment& segment,
                const DPoint &xFormPt) {
    static const float kThird = 0.33333333333f;
    static const float kTwentySeventh = 0.037037037f;

    const float a = 0.5f - (float)xFormPt.y();
    const float b = -0.5f * (float)xFormPt.x();

    const float a3 = a * a * a;
    const float b2 = b * b;

    const float c = (b2 * 0.25f) + (a3 * kTwentySeventh);

    if (c >= 0.f) {
        const float sqrtC = sqrt(c);
        const float result = (float)cbrt((-b * 0.5f) + sqrtC) + (float)cbrt((-b * 0.5f) - sqrtC);
        return result;
    } else {
        const float cosPhi = (float)sqrt((b2 * 0.25f) * (-27.f / a3)) * ((b > 0) ? -1.f : 1.f);
        const float phi = (float)acos(cosPhi);
        float result;
        if (xFormPt.x() > 0.f) {
            result = 2.f * (float)sqrt(-a * kThird) * (float)cos(phi * kThird);
            if (!between_closed(result, segment.fP0T.x(), segment.fP2T.x())) {
                result = 2.f * (float)sqrt(-a * kThird) * (float)cos((phi * kThird) + (SK_ScalarPI * 2.f * kThird));
            }
        } else {
            result = 2.f * (float)sqrt(-a * kThird) * (float)cos((phi * kThird) + (SK_ScalarPI * 2.f * kThird));
            if (!between_closed(result, segment.fP0T.x(), segment.fP2T.x())) {
                result = 2.f * (float)sqrt(-a * kThird) * (float)cos(phi * kThird);
            }
        }
        return result;
    }
}

// This structure contains some intermediate values shared by the same row.
// It is used to calculate segment side of a quadratic bezier.
struct RowData {
    // The intersection type of a scanline and y = x * x parabola in canonical space.
    enum IntersectionType {
        kNoIntersection,
        kVerticalLine,
        kTangentLine,
        kTwoPointsIntersect
    } fIntersectionType;

    // The direction of the quadratic segment/scanline in the canonical space.
    //  1: The quadratic segment/scanline going from negative x-axis to positive x-axis.
    //  0: The scanline is a vertical line in the canonical space.
    // -1: The quadratic segment/scanline going from positive x-axis to negative x-axis.
    int fQuadXDirection;
    int fScanlineXDirection;

    // The y-value(equal to x*x) of intersection point for the kVerticalLine intersection type.
    double fYAtIntersection;

    // The x-value for two intersection points.
    double fXAtIntersection1;
    double fXAtIntersection2;
};

void precomputation_for_row(
            RowData *rowData,
            const PathSegment& segment,
            const SkPoint& pointLeft,
            const SkPoint& pointRight
            ) {
    if (segment.fType != PathSegment::kQuad) {
        return;
    }

    const DPoint& xFormPtLeft = segment.fXformMatrix.mapPoint(pointLeft);
    const DPoint& xFormPtRight = segment.fXformMatrix.mapPoint(pointRight);

    rowData->fQuadXDirection = (int)sign_of(segment.fP2T.x() - segment.fP0T.x());
    rowData->fScanlineXDirection = (int)sign_of(xFormPtRight.x() - xFormPtLeft.x());

    const double x1 = xFormPtLeft.x();
    const double y1 = xFormPtLeft.y();
    const double x2 = xFormPtRight.x();
    const double y2 = xFormPtRight.y();

    if (nearly_equal(x1, x2, segment.fNearlyZeroScaled, true)) {
        rowData->fIntersectionType = RowData::kVerticalLine;
        rowData->fYAtIntersection = x1 * x1;
        rowData->fScanlineXDirection = 0;
        return;
    }

    // Line y = mx + b
    const double m = (y2 - y1) / (x2 - x1);
    const double b = -m * x1 + y1;

    const double m2 = m * m;
    const double c = m2 + 4.0 * b;

    const double tol = 4.0 * segment.fTangentTolScaledSqd / (m2 + 1.0);

    // Check if the scanline is the tangent line of the curve,
    // and the curve start or end at the same y-coordinate of the scanline
    if ((rowData->fScanlineXDirection == 1 &&
         (segment.fPts[0].y() == pointLeft.y() ||
         segment.fPts[2].y() == pointLeft.y())) &&
         nearly_zero(c, tol)) {
        rowData->fIntersectionType = RowData::kTangentLine;
        rowData->fXAtIntersection1 = m / 2.0;
        rowData->fXAtIntersection2 = m / 2.0;
    } else if (c <= 0.0) {
        rowData->fIntersectionType = RowData::kNoIntersection;
        return;
    } else {
        rowData->fIntersectionType = RowData::kTwoPointsIntersect;
        const double d = sqrt(c);
        rowData->fXAtIntersection1 = (m + d) / 2.0;
        rowData->fXAtIntersection2 = (m - d) / 2.0;
    }
}

SegSide calculate_side_of_quad(
            const PathSegment& segment,
            const SkPoint& point,
            const DPoint& xFormPt,
            const RowData& rowData) {
    SegSide side = kNA_SegSide;

    if (RowData::kVerticalLine == rowData.fIntersectionType) {
        side = (SegSide)(int)(sign_of(xFormPt.y() - rowData.fYAtIntersection) * rowData.fQuadXDirection);
    }
    else if (RowData::kTwoPointsIntersect == rowData.fIntersectionType) {
        const double p1 = rowData.fXAtIntersection1;
        const double p2 = rowData.fXAtIntersection2;

        int signP1 = (int)sign_of(p1 - xFormPt.x());
        bool includeP1 = true;
        bool includeP2 = true;

        if (rowData.fScanlineXDirection == 1) {
            if ((rowData.fQuadXDirection == -1 && segment.fPts[0].y() <= point.y() &&
                 nearly_equal(segment.fP0T.x(), p1, segment.fNearlyZeroScaled, true)) ||
                 (rowData.fQuadXDirection == 1 && segment.fPts[2].y() <= point.y() &&
                 nearly_equal(segment.fP2T.x(), p1, segment.fNearlyZeroScaled, true))) {
                includeP1 = false;
            }
            if ((rowData.fQuadXDirection == -1 && segment.fPts[2].y() <= point.y() &&
                 nearly_equal(segment.fP2T.x(), p2, segment.fNearlyZeroScaled, true)) ||
                 (rowData.fQuadXDirection == 1 && segment.fPts[0].y() <= point.y() &&
                 nearly_equal(segment.fP0T.x(), p2, segment.fNearlyZeroScaled, true))) {
                includeP2 = false;
            }
        }

        if (includeP1 && between_closed(p1, segment.fP0T.x(), segment.fP2T.x(),
                                        segment.fNearlyZeroScaled, true)) {
            side = (SegSide)(signP1 * rowData.fQuadXDirection);
        }
        if (includeP2 && between_closed(p2, segment.fP0T.x(), segment.fP2T.x(),
                                        segment.fNearlyZeroScaled, true)) {
            int signP2 = (int)sign_of(p2 - xFormPt.x());
            if (side == kNA_SegSide || signP2 == 1) {
                side = (SegSide)(-signP2 * rowData.fQuadXDirection);
            }
        }
    } else if (RowData::kTangentLine == rowData.fIntersectionType) {
        // The scanline is the tangent line of current quadratic segment.

        const double p = rowData.fXAtIntersection1;
        int signP = (int)sign_of(p - xFormPt.x());
        if (rowData.fScanlineXDirection == 1) {
            // The path start or end at the tangent point.
            if (segment.fPts[0].y() == point.y()) {
                side = (SegSide)(signP);
            } else if (segment.fPts[2].y() == point.y()) {
                side = (SegSide)(-signP);
            }
        }
    }

    return side;
}

static float distance_to_segment(const SkPoint& point,
                                 const PathSegment& segment,
                                 const RowData& rowData,
                                 SegSide* side) {
    SkASSERT(side);

    const DPoint xformPt = segment.fXformMatrix.mapPoint(point);

    if (segment.fType == PathSegment::kLine) {
        float result = SK_DistanceFieldPad * SK_DistanceFieldPad;

        if (between_closed(xformPt.x(), segment.fP0T.x(), segment.fP2T.x())) {
            result = (float)(xformPt.y() * xformPt.y());
        } else if (xformPt.x() < segment.fP0T.x()) {
            result = (float)(xformPt.x() * xformPt.x() + xformPt.y() * xformPt.y());
        } else {
            result = (float)((xformPt.x() - segment.fP2T.x()) * (xformPt.x() - segment.fP2T.x())
                     + xformPt.y() * xformPt.y());
        }

        if (between_closed_open(point.y(), segment.fBoundingBox.top(),
                                segment.fBoundingBox.bottom())) {
            *side = (SegSide)(int)sign_of(xformPt.y());
        } else {
            *side = kNA_SegSide;
        }
        return result;
    } else {
        SkASSERT(segment.fType == PathSegment::kQuad);

        const float nearestPoint = calculate_nearest_point_for_quad(segment, xformPt);

        float dist;

        if (between_closed(nearestPoint, segment.fP0T.x(), segment.fP2T.x())) {
            DPoint x = DPoint::Make(nearestPoint, nearestPoint * nearestPoint);
            dist = (float)xformPt.distanceToSqd(x);
        } else {
            const float distToB0T = (float)xformPt.distanceToSqd(segment.fP0T);
            const float distToB2T = (float)xformPt.distanceToSqd(segment.fP2T);

            if (distToB0T < distToB2T) {
                dist = distToB0T;
            } else {
                dist = distToB2T;
            }
        }

        if (between_closed_open(point.y(), segment.fBoundingBox.top(),
                                segment.fBoundingBox.bottom())) {
            *side = calculate_side_of_quad(segment, point, xformPt, rowData);
        } else {
            *side = kNA_SegSide;
        }

        return (float)(dist * segment.fScalingFactorSqd);
    }
}

static void calculate_distance_field_data(PathSegmentArray* segments,
                                          DFData* dataPtr,
                                          int width, int height) {
    int count = segments->count();
    for (int a = 0; a < count; ++a) {
        PathSegment& segment = (*segments)[a];
        const SkRect& segBB = segment.fBoundingBox.makeOutset(
                                SK_DistanceFieldPad, SK_DistanceFieldPad);
        int startColumn = (int)segBB.left();
        int endColumn = SkScalarCeilToInt(segBB.right());

        int startRow = (int)segBB.top();
        int endRow = SkScalarCeilToInt(segBB.bottom());

        SkASSERT((startColumn >= 0) && "StartColumn < 0!");
        SkASSERT((endColumn <= width) && "endColumn > width!");
        SkASSERT((startRow >= 0) && "StartRow < 0!");
        SkASSERT((endRow <= height) && "EndRow > height!");

        // Clip inside the distance field to avoid overflow
        startColumn = SkTMax(startColumn, 0);
        endColumn   = SkTMin(endColumn,   width);
        startRow    = SkTMax(startRow,    0);
        endRow      = SkTMin(endRow,      height);

        for (int row = startRow; row < endRow; ++row) {
            SegSide prevSide = kNA_SegSide;
            const float pY = row + 0.5f;
            RowData rowData;

            const SkPoint pointLeft = SkPoint::Make((SkScalar)startColumn, pY);
            const SkPoint pointRight = SkPoint::Make((SkScalar)endColumn, pY);

            if (between_closed_open(pY, segment.fBoundingBox.top(),
                                    segment.fBoundingBox.bottom())) {
                precomputation_for_row(&rowData, segment, pointLeft, pointRight);
            }

            for (int col = startColumn; col < endColumn; ++col) {
                int idx = (row * width) + col;

                const float pX = col + 0.5f;
                const SkPoint point = SkPoint::Make(pX, pY);

                const float distSq = dataPtr[idx].fDistSq;
                int dilation = distSq < 1.5 * 1.5 ? 1 :
                               distSq < 2.5 * 2.5 ? 2 :
                               distSq < 3.5 * 3.5 ? 3 : SK_DistanceFieldPad;
                if (dilation > SK_DistanceFieldPad) {
                    dilation = SK_DistanceFieldPad;
                }

                // Optimisation for not calculating some points.
                if (dilation != SK_DistanceFieldPad && !segment.fBoundingBox.roundOut()
                    .makeOutset(dilation, dilation).contains(col, row)) {
                    continue;
                }

                SegSide side = kNA_SegSide;
                int     deltaWindingScore = 0;
                float   currDistSq = distance_to_segment(point, segment, rowData, &side);
                if (prevSide == kLeft_SegSide && side == kRight_SegSide) {
                    deltaWindingScore = -1;
                } else if (prevSide == kRight_SegSide && side == kLeft_SegSide) {
                    deltaWindingScore = 1;
                }

                prevSide = side;

                if (currDistSq < distSq) {
                    dataPtr[idx].fDistSq = currDistSq;
                }

                dataPtr[idx].fDeltaWindingScore += deltaWindingScore;
            }
        }
    }
}

template <int distanceMagnitude>
static unsigned char pack_distance_field_val(float dist) {
    // The distance field is constructed as unsigned char values, so that the zero value is at 128,
    // Beside 128, we have 128 values in range [0, 128), but only 127 values in range (128, 255].
    // So we multiply distanceMagnitude by 127/128 at the latter range to avoid overflow.
    dist = SkScalarPin(-dist, -distanceMagnitude, distanceMagnitude * 127.0f / 128.0f);

    // Scale into the positive range for unsigned distance.
    dist += distanceMagnitude;

    // Scale into unsigned char range.
    // Round to place negative and positive values as equally as possible around 128
    // (which represents zero).
    return (unsigned char)SkScalarRoundToInt(dist / (2 * distanceMagnitude) * 256.0f);
}

bool GrGenerateDistanceFieldFromPath(unsigned char* distanceField,
                                     const SkPath& path, const SkMatrix& drawMatrix,
                                     int width, int height, size_t rowBytes) {
    SkASSERT(distanceField);

#ifdef SK_DEBUG
    SkPath xformPath;
    path.transform(drawMatrix, &xformPath);
    SkIRect pathBounds = xformPath.getBounds().roundOut();
    SkIRect expectPathBounds =
            SkIRect::MakeWH(width - 2 * SK_DistanceFieldPad, height - 2 * SK_DistanceFieldPad);
#endif

    SkASSERT(expectPathBounds.isEmpty() ||
             expectPathBounds.contains(pathBounds.x(), pathBounds.y()));
    SkASSERT(expectPathBounds.isEmpty() || pathBounds.isEmpty() ||
             expectPathBounds.contains(pathBounds));

    SkPath simplifiedPath;
    SkPath workingPath;
    if (Simplify(path, &simplifiedPath)) {
        workingPath = simplifiedPath;
    } else {
        workingPath = path;
    }

    if (!IsDistanceFieldSupportedFillType(workingPath.getFillType())) {
        return false;
    }

    workingPath.transform(drawMatrix);

    SkDEBUGCODE(pathBounds = workingPath.getBounds().roundOut());
    SkASSERT(expectPathBounds.isEmpty() ||
             expectPathBounds.contains(pathBounds.x(), pathBounds.y()));
    SkASSERT(expectPathBounds.isEmpty() || pathBounds.isEmpty() ||
             expectPathBounds.contains(pathBounds));

    // translate path to offset (SK_DistanceFieldPad, SK_DistanceFieldPad)
    SkMatrix dfMatrix;
    dfMatrix.setTranslate(SK_DistanceFieldPad, SK_DistanceFieldPad);
    workingPath.transform(dfMatrix);

    // create temp data
    size_t dataSize = width * height * sizeof(DFData);
    SkAutoSMalloc<1024> dfStorage(dataSize);
    DFData* dataPtr = (DFData*) dfStorage.get();

    // create initial distance data
    init_distances(dataPtr, width * height);

    SkPath::Iter iter(workingPath, true);
    SkSTArray<15, PathSegment, true> segments;

    for (;;) {
        SkPoint pts[4];
        SkPath::Verb verb = iter.next(pts);
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb: {
                add_line_to_segment(pts, &segments);
                break;
            }
            case SkPath::kQuad_Verb:
                add_quad_segment(pts, &segments);
                break;
            case SkPath::kConic_Verb: {
                SkScalar weight = iter.conicWeight();
                SkAutoConicToQuads converter;
                const SkPoint* quadPts = converter.computeQuads(pts, weight, kConicTolerance);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    add_quad_segment(quadPts + 2*i, &segments);
                }
                break;
            }
            case SkPath::kCubic_Verb: {
                add_cubic_segments(pts, &segments);
                break;
            }
            default:
                break;
        }
        if (verb == SkPath::kDone_Verb) {
            break;
        }
    }

    calculate_distance_field_data(&segments, dataPtr, width, height);

    for (int row = 0; row < height; ++row) {
        int windingNumber = 0; // Winding number start from zero for each scanline
        for (int col = 0; col < width; ++col) {
            int idx = (row * width) + col;
            windingNumber += dataPtr[idx].fDeltaWindingScore;

            enum DFSign {
                kInside = -1,
                kOutside = 1
            } dfSign;

            if (workingPath.getFillType() == SkPath::kWinding_FillType) {
                dfSign = windingNumber ? kInside : kOutside;
            } else if (workingPath.getFillType() == SkPath::kInverseWinding_FillType) {
                dfSign = windingNumber ? kOutside : kInside;
            } else if (workingPath.getFillType() == SkPath::kEvenOdd_FillType) {
                dfSign = (windingNumber % 2) ? kInside : kOutside;
            } else {
                SkASSERT(workingPath.getFillType() == SkPath::kInverseEvenOdd_FillType);
                dfSign = (windingNumber % 2) ? kOutside : kInside;
            }

            // The winding number at the end of a scanline should be zero.
            SkASSERT(((col != width - 1) || (windingNumber == 0)) &&
                    "Winding number should be zero at the end of a scan line.");
            // Fallback to use SkPath::contains to determine the sign of pixel in release build.
            if (col == width - 1 && windingNumber != 0) {
                for (int col = 0; col < width; ++col) {
                    int idx = (row * width) + col;
                    dfSign = workingPath.contains(col + 0.5, row + 0.5) ? kInside : kOutside;
                    const float miniDist = sqrt(dataPtr[idx].fDistSq);
                    const float dist = dfSign * miniDist;

                    unsigned char pixelVal = pack_distance_field_val<SK_DistanceFieldMagnitude>(dist);

                    distanceField[(row * rowBytes) + col] = pixelVal;
                }
                continue;
            }

            const float miniDist = sqrt(dataPtr[idx].fDistSq);
            const float dist = dfSign * miniDist;

            unsigned char pixelVal = pack_distance_field_val<SK_DistanceFieldMagnitude>(dist);

            distanceField[(row * rowBytes) + col] = pixelVal;
        }
    }
    return true;
}
