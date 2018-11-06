/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPatchUtils.h"

#include "SkArenaAlloc.h"
#include "SkColorData.h"
#include "SkColorSpacePriv.h"
#include "SkConvertPixels.h"
#include "SkGeometry.h"
#include "SkTo.h"

namespace {
    enum CubicCtrlPts {
        kTopP0_CubicCtrlPts = 0,
        kTopP1_CubicCtrlPts = 1,
        kTopP2_CubicCtrlPts = 2,
        kTopP3_CubicCtrlPts = 3,

        kRightP0_CubicCtrlPts = 3,
        kRightP1_CubicCtrlPts = 4,
        kRightP2_CubicCtrlPts = 5,
        kRightP3_CubicCtrlPts = 6,

        kBottomP0_CubicCtrlPts = 9,
        kBottomP1_CubicCtrlPts = 8,
        kBottomP2_CubicCtrlPts = 7,
        kBottomP3_CubicCtrlPts = 6,

        kLeftP0_CubicCtrlPts = 0,
        kLeftP1_CubicCtrlPts = 11,
        kLeftP2_CubicCtrlPts = 10,
        kLeftP3_CubicCtrlPts = 9,
    };

    // Enum for corner also clockwise.
    enum Corner {
        kTopLeft_Corner = 0,
        kTopRight_Corner,
        kBottomRight_Corner,
        kBottomLeft_Corner
    };
}

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
 *  Calculate the approximate arc length given a bezier curve's control points.
 *  Returns -1 if bad calc (i.e. non-finite)
 */
static SkScalar approx_arc_length(const SkPoint points[], int count) {
    if (count < 2) {
        return 0;
    }
    SkScalar arcLength = 0;
    for (int i = 0; i < count - 1; i++) {
        arcLength += SkPoint::Distance(points[i], points[i + 1]);
    }
    return SkScalarIsFinite(arcLength) ? arcLength : -1;
}

static SkScalar bilerp(SkScalar tx, SkScalar ty, SkScalar c00, SkScalar c10, SkScalar c01,
                       SkScalar c11) {
    SkScalar a = c00 * (1.f - tx) + c10 * tx;
    SkScalar b = c01 * (1.f - tx) + c11 * tx;
    return a * (1.f - ty) + b * ty;
}

static Sk4f bilerp(SkScalar tx, SkScalar ty,
                   const Sk4f& c00, const Sk4f& c10, const Sk4f& c01, const Sk4f& c11) {
    Sk4f a = c00 * (1.f - tx) + c10 * tx;
    Sk4f b = c01 * (1.f - tx) + c11 * tx;
    return a * (1.f - ty) + b * ty;
}

SkISize SkPatchUtils::GetLevelOfDetail(const SkPoint cubics[12], const SkMatrix* matrix) {
    // Approximate length of each cubic.
    SkPoint pts[kNumPtsCubic];
    SkPatchUtils::GetTopCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar topLength = approx_arc_length(pts, kNumPtsCubic);

    SkPatchUtils::GetBottomCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar bottomLength = approx_arc_length(pts, kNumPtsCubic);

    SkPatchUtils::GetLeftCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar leftLength = approx_arc_length(pts, kNumPtsCubic);

    SkPatchUtils::GetRightCubic(cubics, pts);
    matrix->mapPoints(pts, kNumPtsCubic);
    SkScalar rightLength = approx_arc_length(pts, kNumPtsCubic);

    if (topLength < 0 || bottomLength < 0 || leftLength < 0 || rightLength < 0) {
        return {0, 0};  // negative length is a sentinel for bad length (i.e. non-finite)
    }

    // Level of detail per axis, based on the larger side between top and bottom or left and right
    int lodX = static_cast<int>(SkMaxScalar(topLength, bottomLength) / kPartitionSize);
    int lodY = static_cast<int>(SkMaxScalar(leftLength, rightLength) / kPartitionSize);

    return SkISize::Make(SkMax32(8, lodX), SkMax32(8, lodY));
}

void SkPatchUtils::GetTopCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kTopP0_CubicCtrlPts];
    points[1] = cubics[kTopP1_CubicCtrlPts];
    points[2] = cubics[kTopP2_CubicCtrlPts];
    points[3] = cubics[kTopP3_CubicCtrlPts];
}

void SkPatchUtils::GetBottomCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kBottomP0_CubicCtrlPts];
    points[1] = cubics[kBottomP1_CubicCtrlPts];
    points[2] = cubics[kBottomP2_CubicCtrlPts];
    points[3] = cubics[kBottomP3_CubicCtrlPts];
}

void SkPatchUtils::GetLeftCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kLeftP0_CubicCtrlPts];
    points[1] = cubics[kLeftP1_CubicCtrlPts];
    points[2] = cubics[kLeftP2_CubicCtrlPts];
    points[3] = cubics[kLeftP3_CubicCtrlPts];
}

void SkPatchUtils::GetRightCubic(const SkPoint cubics[12], SkPoint points[4]) {
    points[0] = cubics[kRightP0_CubicCtrlPts];
    points[1] = cubics[kRightP1_CubicCtrlPts];
    points[2] = cubics[kRightP2_CubicCtrlPts];
    points[3] = cubics[kRightP3_CubicCtrlPts];
}

static void skcolor_to_float(SkPMColor4f* dst, const SkColor* src, int count, SkColorSpace* dstCS) {
    SkImageInfo srcInfo = SkImageInfo::Make(count, 1, kBGRA_8888_SkColorType,
                                            kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkImageInfo dstInfo = SkImageInfo::Make(count, 1, kRGBA_F32_SkColorType,
                                            kPremul_SkAlphaType, sk_ref_sp(dstCS));
    SkConvertPixels(dstInfo, dst, 0, srcInfo, src, 0);
}

static void float_to_skcolor(SkColor* dst, const SkPMColor4f* src, int count, SkColorSpace* srcCS) {
    SkImageInfo srcInfo = SkImageInfo::Make(count, 1, kRGBA_F32_SkColorType,
                                            kPremul_SkAlphaType, sk_ref_sp(srcCS));
    SkImageInfo dstInfo = SkImageInfo::Make(count, 1, kBGRA_8888_SkColorType,
                                            kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkConvertPixels(dstInfo, dst, 0, srcInfo, src, 0);
}

sk_sp<SkVertices> SkPatchUtils::MakeVertices(const SkPoint cubics[12], const SkColor srcColors[4],
                                             const SkPoint srcTexCoords[4], int lodX, int lodY,
                                             SkColorSpace* colorSpace) {
    if (lodX < 1 || lodY < 1 || nullptr == cubics) {
        return nullptr;
    }

    // check for overflow in multiplication
    const int64_t lodX64 = (lodX + 1),
    lodY64 = (lodY + 1),
    mult64 = lodX64 * lodY64;
    if (mult64 > SK_MaxS32) {
        return nullptr;
    }

    // Treat null interpolation space as sRGB.
    if (!colorSpace) {
        colorSpace = sk_srgb_singleton();
    }

    int vertexCount = SkToS32(mult64);
    // it is recommended to generate draw calls of no more than 65536 indices, so we never generate
    // more than 60000 indices. To accomplish that we resize the LOD and vertex count
    if (vertexCount > 10000 || lodX > 200 || lodY > 200) {
        float weightX = static_cast<float>(lodX) / (lodX + lodY);
        float weightY = static_cast<float>(lodY) / (lodX + lodY);

        // 200 comes from the 100 * 2 which is the max value of vertices because of the limit of
        // 60000 indices ( sqrt(60000 / 6) that comes from data->fIndexCount = lodX * lodY * 6)
        // Need a min of 1 since we later divide by lod
        lodX = std::max(1, sk_float_floor2int_no_saturate(weightX * 200));
        lodY = std::max(1, sk_float_floor2int_no_saturate(weightY * 200));
        vertexCount = (lodX + 1) * (lodY + 1);
    }
    const int indexCount = lodX * lodY * 6;
    uint32_t flags = 0;
    if (srcTexCoords) {
        flags |= SkVertices::kHasTexCoords_BuilderFlag;
    }
    if (srcColors) {
        flags |= SkVertices::kHasColors_BuilderFlag;
    }

    SkSTArenaAlloc<2048> alloc;
    SkPMColor4f* cornerColors = srcColors ? alloc.makeArray<SkPMColor4f>(4) : nullptr;
    SkPMColor4f* tmpColors = srcColors ? alloc.makeArray<SkPMColor4f>(vertexCount) : nullptr;

    SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, vertexCount, indexCount, flags);
    SkPoint* pos = builder.positions();
    SkPoint* texs = builder.texCoords();
    uint16_t* indices = builder.indices();

    if (cornerColors) {
        skcolor_to_float(cornerColors, srcColors, kNumCorners, colorSpace);
    }

    SkPoint pts[kNumPtsCubic];
    SkPatchUtils::GetBottomCubic(cubics, pts);
    FwDCubicEvaluator fBottom(pts);
    SkPatchUtils::GetTopCubic(cubics, pts);
    FwDCubicEvaluator fTop(pts);
    SkPatchUtils::GetLeftCubic(cubics, pts);
    FwDCubicEvaluator fLeft(pts);
    SkPatchUtils::GetRightCubic(cubics, pts);
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
            pos[dataIndex] = s0 + s1 - s2;

            if (cornerColors) {
                bilerp(u, v, Sk4f::Load(cornerColors[kTopLeft_Corner].vec()),
                             Sk4f::Load(cornerColors[kTopRight_Corner].vec()),
                             Sk4f::Load(cornerColors[kBottomLeft_Corner].vec()),
                             Sk4f::Load(cornerColors[kBottomRight_Corner].vec()))
                    .store(tmpColors[dataIndex].vec());
            }

            if (texs) {
                texs[dataIndex] = SkPoint::Make(bilerp(u, v, srcTexCoords[kTopLeft_Corner].x(),
                                                       srcTexCoords[kTopRight_Corner].x(),
                                                       srcTexCoords[kBottomLeft_Corner].x(),
                                                       srcTexCoords[kBottomRight_Corner].x()),
                                                bilerp(u, v, srcTexCoords[kTopLeft_Corner].y(),
                                                       srcTexCoords[kTopRight_Corner].y(),
                                                       srcTexCoords[kBottomLeft_Corner].y(),
                                                       srcTexCoords[kBottomRight_Corner].y()));

            }

            if(x < lodX && y < lodY) {
                int i = 6 * (x * lodY + y);
                indices[i] = x * stride + y;
                indices[i + 1] = x * stride + 1 + y;
                indices[i + 2] = (x + 1) * stride + 1 + y;
                indices[i + 3] = indices[i];
                indices[i + 4] = indices[i + 2];
                indices[i + 5] = (x + 1) * stride + y;
            }
            v = SkScalarClampMax(v + 1.f / lodY, 1);
        }
        u = SkScalarClampMax(u + 1.f / lodX, 1);
    }

    if (tmpColors) {
        float_to_skcolor(builder.colors(), tmpColors, vertexCount, colorSpace);
    }
    return builder.detach();
}
