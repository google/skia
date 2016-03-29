/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPatchUtils.h"

#include "SkColorPriv.h"
#include "SkGeometry.h"

/**
 * Evaluator to sample the values of a cubic bezier using forward differences.
 * Forward differences is a method for evaluating a nth degree polynomial at a uniform step by only
 * adding precalculated values.
 * For a linear example we have the function f(t) = m*t+b, then the value of that function at t+h
 * would be f(t+h) = m*(t+h)+b. If we want to know the uniform step that we must add to the first
 * evaluation f(t) then we need to substract f(t+h) - f(t) = m*t + m*h + b - m*t + b = mh. After
 * obtaining this value (mh) we could just add this constant step to our first sampled point
 * to compute the next one.
 *
 * For the cubic case the first difference gives as a result a quadratic polynomial to which we can
 * apply again forward differences and get linear function to which we can apply again forward
 * differences to get a constant difference. This is why we keep an array of size 4, the 0th
 * position keeps the sampled value while the next ones keep the quadratic, linear and constant
 * difference values.
 */

class FwDCubicEvaluator {

public:

    /**
     * Receives the 4 control points of the cubic bezier.
     */

    explicit FwDCubicEvaluator(const SkPoint points[4])
            : fCoefs(points) {
        memcpy(fPoints, points, 4 * sizeof(SkPoint));

        this->restart(1);
    }

    /**
     * Restarts the forward differences evaluator to the first value of t = 0.
     */
    void restart(int divisions)  {
        fDivisions = divisions;
        fCurrent    = 0;
        fMax        = fDivisions + 1;
        Sk2s h  = Sk2s(1.f / fDivisions);
        Sk2s h2 = h * h;
        Sk2s h3 = h2 * h;
        Sk2s fwDiff3 = Sk2s(6) * fCoefs.fA * h3;
        fFwDiff[3] = to_point(fwDiff3);
        fFwDiff[2] = to_point(fwDiff3 + times_2(fCoefs.fB) * h2);
        fFwDiff[1] = to_point(fCoefs.fA * h3 + fCoefs.fB * h2 + fCoefs.fC * h);
        fFwDiff[0] = to_point(fCoefs.fD);
    }

    /**
     * Check if the evaluator is still within the range of 0<=t<=1
     */
    bool done() const {
        return fCurrent > fMax;
    }

    /**
     * Call next to obtain the SkPoint sampled and move to the next one.
     */
    SkPoint next() {
        SkPoint point = fFwDiff[0];
        fFwDiff[0]    += fFwDiff[1];
        fFwDiff[1]    += fFwDiff[2];
        fFwDiff[2]    += fFwDiff[3];
        fCurrent++;
        return point;
    }

    const SkPoint* getCtrlPoints() const {
        return fPoints;
    }

private:
    SkCubicCoeff fCoefs;
    int fMax, fCurrent, fDivisions;
    SkPoint fFwDiff[4], fPoints[4];
};

////////////////////////////////////////////////////////////////////////////////

// size in pixels of each partition per axis, adjust this knob
static const int kPartitionSize = 10;

/**
 * Calculate the approximate arc length given a bezier curve's control points.
 */
static SkScalar approx_arc_length(SkPoint* points, int count) {
    if (count < 2) {
        return 0;
    }
    SkScalar arcLength = 0;
    for (int i = 0; i < count - 1; i++) {
        arcLength += SkPoint::Distance(points[i], points[i + 1]);
    }
    return arcLength;
}

static SkScalar bilerp(SkScalar tx, SkScalar ty, SkScalar c00, SkScalar c10, SkScalar c01,
                      SkScalar c11) {
    SkScalar a = c00 * (1.f - tx) + c10 * tx;
    SkScalar b = c01 * (1.f - tx) + c11 * tx;
    return a * (1.f - ty) + b * ty;
}

SkISize SkPatchUtils::GetLevelOfDetail(const SkPoint cubics[12], const SkMatrix* matrix) {

    // Approximate length of each cubic.
    SkPoint pts[kNumPtsCubic];
    SkPatchUtils::getTopCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar topLength = approx_arc_length(pts, kNumPtsCubic);

    SkPatchUtils::getBottomCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar bottomLength = approx_arc_length(pts, kNumPtsCubic);

    SkPatchUtils::getLeftCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar leftLength = approx_arc_length(pts, kNumPtsCubic);

    SkPatchUtils::getRightCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar rightLength = approx_arc_length(pts, kNumPtsCubic);

    // Level of detail per axis, based on the larger side between top and bottom or left and right
    int lodX = static_cast<int>(SkMaxScalar(topLength, bottomLength) / kPartitionSize);
    int lodY = static_cast<int>(SkMaxScalar(leftLength, rightLength) / kPartitionSize);

    return SkISize::Make(SkMax32(8, lodX), SkMax32(8, lodY));
}

void SkPatchUtils::getTopCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kTopP0_CubicCtrlPts];
    points[1] = cubics[kTopP1_CubicCtrlPts];
    points[2] = cubics[kTopP2_CubicCtrlPts];
    points[3] = cubics[kTopP3_CubicCtrlPts];
}

void SkPatchUtils::getBottomCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kBottomP0_CubicCtrlPts];
    points[1] = cubics[kBottomP1_CubicCtrlPts];
    points[2] = cubics[kBottomP2_CubicCtrlPts];
    points[3] = cubics[kBottomP3_CubicCtrlPts];
}

void SkPatchUtils::getLeftCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kLeftP0_CubicCtrlPts];
    points[1] = cubics[kLeftP1_CubicCtrlPts];
    points[2] = cubics[kLeftP2_CubicCtrlPts];
    points[3] = cubics[kLeftP3_CubicCtrlPts];
}

void SkPatchUtils::getRightCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kRightP0_CubicCtrlPts];
    points[1] = cubics[kRightP1_CubicCtrlPts];
    points[2] = cubics[kRightP2_CubicCtrlPts];
    points[3] = cubics[kRightP3_CubicCtrlPts];
}

bool SkPatchUtils::getVertexData(SkPatchUtils::VertexData* data, const SkPoint cubics[12],
                   const SkColor colors[4], const SkPoint texCoords[4], int lodX, int lodY) {
    if (lodX < 1 || lodY < 1 || nullptr == cubics || nullptr == data) {
        return false;
    }

    // check for overflow in multiplication
    const int64_t lodX64 = (lodX + 1),
                   lodY64 = (lodY + 1),
                   mult64 = lodX64 * lodY64;
    if (mult64 > SK_MaxS32) {
        return false;
    }
    data->fVertexCount = SkToS32(mult64);

    // it is recommended to generate draw calls of no more than 65536 indices, so we never generate
    // more than 60000 indices. To accomplish that we resize the LOD and vertex count
    if (data->fVertexCount > 10000 || lodX > 200 || lodY > 200) {
        SkScalar weightX = static_cast<SkScalar>(lodX) / (lodX + lodY);
        SkScalar weightY = static_cast<SkScalar>(lodY) / (lodX + lodY);

        // 200 comes from the 100 * 2 which is the max value of vertices because of the limit of
        // 60000 indices ( sqrt(60000 / 6) that comes from data->fIndexCount = lodX * lodY * 6)
        lodX = static_cast<int>(weightX * 200);
        lodY = static_cast<int>(weightY * 200);
        data->fVertexCount = (lodX + 1) * (lodY + 1);
    }
    data->fIndexCount = lodX * lodY * 6;

    data->fPoints = new SkPoint[data->fVertexCount];
    data->fIndices = new uint16_t[data->fIndexCount];

    // if colors is not null then create array for colors
    SkPMColor colorsPM[kNumCorners];
    if (colors) {
        // premultiply colors to avoid color bleeding.
        for (int i = 0; i < kNumCorners; i++) {
            colorsPM[i] = SkPreMultiplyColor(colors[i]);
        }
        data->fColors = new uint32_t[data->fVertexCount];
    }

    // if texture coordinates are not null then create array for them
    if (texCoords) {
        data->fTexCoords = new SkPoint[data->fVertexCount];
    }

    SkPoint pts[kNumPtsCubic];
    SkPatchUtils::getBottomCubic(cubics, pts);
    FwDCubicEvaluator fBottom(pts);
    SkPatchUtils::getTopCubic(cubics, pts);
    FwDCubicEvaluator fTop(pts);
    SkPatchUtils::getLeftCubic(cubics, pts);
    FwDCubicEvaluator fLeft(pts);
    SkPatchUtils::getRightCubic(cubics, pts);
    FwDCubicEvaluator fRight(pts);

    fBottom.restart(lodX);
    fTop.restart(lodX);

    SkScalar u = 0.0f;
    int stride = lodY + 1;
    for (int x = 0; x <= lodX; x++) {
        SkPoint bottom = fBottom.next(), top = fTop.next();
        fLeft.restart(lodY);
        fRight.restart(lodY);
        SkScalar v = 0.f;
        for (int y = 0; y <= lodY; y++) {
            int dataIndex = x * (lodY + 1) + y;

            SkPoint left = fLeft.next(), right = fRight.next();

            SkPoint s0 = SkPoint::Make((1.0f - v) * top.x() + v * bottom.x(),
                                       (1.0f - v) * top.y() + v * bottom.y());
            SkPoint s1 = SkPoint::Make((1.0f - u) * left.x() + u * right.x(),
                                       (1.0f - u) * left.y() + u * right.y());
            SkPoint s2 = SkPoint::Make(
                                       (1.0f - v) * ((1.0f - u) * fTop.getCtrlPoints()[0].x()
                                                     + u * fTop.getCtrlPoints()[3].x())
                                       + v * ((1.0f - u) * fBottom.getCtrlPoints()[0].x()
                                              + u * fBottom.getCtrlPoints()[3].x()),
                                       (1.0f - v) * ((1.0f - u) * fTop.getCtrlPoints()[0].y()
                                                     + u * fTop.getCtrlPoints()[3].y())
                                       + v * ((1.0f - u) * fBottom.getCtrlPoints()[0].y()
                                              + u * fBottom.getCtrlPoints()[3].y()));
            data->fPoints[dataIndex] = s0 + s1 - s2;

            if (colors) {
                uint8_t a = uint8_t(bilerp(u, v,
                                   SkScalar(SkColorGetA(colorsPM[kTopLeft_Corner])),
                                   SkScalar(SkColorGetA(colorsPM[kTopRight_Corner])),
                                   SkScalar(SkColorGetA(colorsPM[kBottomLeft_Corner])),
                                   SkScalar(SkColorGetA(colorsPM[kBottomRight_Corner]))));
                uint8_t r = uint8_t(bilerp(u, v,
                                   SkScalar(SkColorGetR(colorsPM[kTopLeft_Corner])),
                                   SkScalar(SkColorGetR(colorsPM[kTopRight_Corner])),
                                   SkScalar(SkColorGetR(colorsPM[kBottomLeft_Corner])),
                                   SkScalar(SkColorGetR(colorsPM[kBottomRight_Corner]))));
                uint8_t g = uint8_t(bilerp(u, v,
                                   SkScalar(SkColorGetG(colorsPM[kTopLeft_Corner])),
                                   SkScalar(SkColorGetG(colorsPM[kTopRight_Corner])),
                                   SkScalar(SkColorGetG(colorsPM[kBottomLeft_Corner])),
                                   SkScalar(SkColorGetG(colorsPM[kBottomRight_Corner]))));
                uint8_t b = uint8_t(bilerp(u, v,
                                   SkScalar(SkColorGetB(colorsPM[kTopLeft_Corner])),
                                   SkScalar(SkColorGetB(colorsPM[kTopRight_Corner])),
                                   SkScalar(SkColorGetB(colorsPM[kBottomLeft_Corner])),
                                   SkScalar(SkColorGetB(colorsPM[kBottomRight_Corner]))));
                data->fColors[dataIndex] = SkPackARGB32(a,r,g,b);
            }

            if (texCoords) {
                data->fTexCoords[dataIndex] = SkPoint::Make(
                                            bilerp(u, v, texCoords[kTopLeft_Corner].x(),
                                                   texCoords[kTopRight_Corner].x(),
                                                   texCoords[kBottomLeft_Corner].x(),
                                                   texCoords[kBottomRight_Corner].x()),
                                            bilerp(u, v, texCoords[kTopLeft_Corner].y(),
                                                   texCoords[kTopRight_Corner].y(),
                                                   texCoords[kBottomLeft_Corner].y(),
                                                   texCoords[kBottomRight_Corner].y()));

            }

            if(x < lodX && y < lodY) {
                int i = 6 * (x * lodY + y);
                data->fIndices[i] = x * stride + y;
                data->fIndices[i + 1] = x * stride + 1 + y;
                data->fIndices[i + 2] = (x + 1) * stride + 1 + y;
                data->fIndices[i + 3] = data->fIndices[i];
                data->fIndices[i + 4] = data->fIndices[i + 2];
                data->fIndices[i + 5] = (x + 1) * stride + y;
            }
            v = SkScalarClampMax(v + 1.f / lodY, 1);
        }
        u = SkScalarClampMax(u + 1.f / lodX, 1);
    }
    return true;

}
