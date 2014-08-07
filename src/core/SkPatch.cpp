/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPatch.h"

#include "SkGeometry.h"
#include "SkColorPriv.h"
#include "SkBuffer.h"

////////////////////////////////////////////////////////////////////////////////

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
    FwDCubicEvaluator() { }
    
    /**
     * Receives the 4 control points of the cubic bezier.
     */
    FwDCubicEvaluator(SkPoint a, SkPoint b, SkPoint c, SkPoint d) {
        fPoints[0] = a;
        fPoints[1] = b;
        fPoints[2] = c;
        fPoints[3] = d;
        
        SkScalar cx[4], cy[4];
        SkGetCubicCoeff(fPoints, cx, cy);
        fCoefs[0].set(cx[0], cy[0]);
        fCoefs[1].set(cx[1], cy[1]);
        fCoefs[2].set(cx[2], cy[2]);
        fCoefs[3].set(cx[3], cy[3]);
        
        this->restart(1);
    }
    
    explicit FwDCubicEvaluator(SkPoint points[4]) {
        for (int i = 0; i< 4; i++) {
            fPoints[i] = points[i];
        }
        
        SkScalar cx[4], cy[4];
        SkGetCubicCoeff(fPoints, cx, cy);
        fCoefs[0].set(cx[0], cy[0]);
        fCoefs[1].set(cx[1], cy[1]);
        fCoefs[2].set(cx[2], cy[2]);
        fCoefs[3].set(cx[3], cy[3]);
        
        this->restart(1);
    }
    
    /**
     * Restarts the forward differences evaluator to the first value of t = 0.
     */
    void restart(int divisions) {
        fDivisions = divisions;
        SkScalar h  = 1.f / fDivisions;
        fCurrent    = 0;
        fMax        = fDivisions + 1;
        fFwDiff[0]  = fCoefs[3];
        SkScalar h2 = h * h;
        SkScalar h3 = h2 * h;
        
        fFwDiff[3].set(6.f * fCoefs[0].x() * h3, 6.f * fCoefs[0].y() * h3); //6ah^3
        fFwDiff[2].set(fFwDiff[3].x() + 2.f * fCoefs[1].x() * h2, //6ah^3 + 2bh^2
                       fFwDiff[3].y() + 2.f * fCoefs[1].y() * h2);
        fFwDiff[1].set(fCoefs[0].x() * h3 + fCoefs[1].x() * h2 + fCoefs[2].x() * h,//ah^3 + bh^2 +ch
                       fCoefs[0].y() * h3 + fCoefs[1].y() * h2 + fCoefs[2].y() * h);
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
    int fMax, fCurrent, fDivisions;
    SkPoint fFwDiff[4], fCoefs[4], fPoints[4];
};

////////////////////////////////////////////////////////////////////////////////

SkPatch::SkPatch(const SkPoint points[12], const SkColor colors[4]) {
    this->reset(points, colors);
}

static uint8_t bilerp(SkScalar tx, SkScalar ty, SkScalar c00, SkScalar c10, SkScalar c01,
                      SkScalar c11) {
    SkScalar a = c00 * (1.f - tx) + c10 * tx;
    SkScalar b = c01 * (1.f - tx) + c11 * tx;
    return uint8_t(a * (1.f - ty) + b * ty);
}

bool SkPatch::getVertexData(SkPatch::VertexData* data, int lodX, int lodY) const {
    
    if (lodX < 1 || lodY < 1) {
        return false;
    }
    
    // premultiply colors to avoid color bleeding. 
    SkPMColor colors[SkPatch::kNumColors];
    for (int i = 0; i < SkPatch::kNumColors; i++) {
        colors[i] = SkPreMultiplyColor(fCornerColors[i]);
    }
    
    // number of indices is limited by size of uint16_t, so we clamp it to avoid overflow
    data->fVertexCount = SkMin32((lodX + 1) * (lodY + 1), 65536);
    lodX  = SkMin32(lodX, 255);
    lodY  = SkMin32(lodY, 255);
    data->fIndexCount = lodX * lodY * 6;

    data->fPoints = SkNEW_ARRAY(SkPoint, data->fVertexCount);
    data->fColors = SkNEW_ARRAY(uint32_t, data->fVertexCount);
    data->fTexCoords = SkNEW_ARRAY(SkPoint, data->fVertexCount);
    data->fIndices = SkNEW_ARRAY(uint16_t, data->fIndexCount);
    
    SkPoint pts[SkPatch::kNumPtsCubic];
    this->getBottomPoints(pts);
    FwDCubicEvaluator fBottom(pts);
    this->getTopPoints(pts);
    FwDCubicEvaluator fTop(pts);
    this->getLeftPoints(pts);
    FwDCubicEvaluator fLeft(pts);
    this->getRightPoints(pts);
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

            uint8_t a = bilerp(u, v,
                            SkScalar(SkColorGetA(colors[kTopLeft_CornerColors])),
                            SkScalar(SkColorGetA(colors[kTopRight_CornerColors])),
                            SkScalar(SkColorGetA(colors[kBottomLeft_CornerColors])),
                            SkScalar(SkColorGetA(colors[kBottomRight_CornerColors])));
            uint8_t r = bilerp(u, v,
                            SkScalar(SkColorGetR(colors[kTopLeft_CornerColors])),
                            SkScalar(SkColorGetR(colors[kTopRight_CornerColors])),
                            SkScalar(SkColorGetR(colors[kBottomLeft_CornerColors])),
                            SkScalar(SkColorGetR(colors[kBottomRight_CornerColors])));
            uint8_t g = bilerp(u, v,
                            SkScalar(SkColorGetG(colors[kTopLeft_CornerColors])),
                            SkScalar(SkColorGetG(colors[kTopRight_CornerColors])),
                            SkScalar(SkColorGetG(colors[kBottomLeft_CornerColors])),
                            SkScalar(SkColorGetG(colors[kBottomRight_CornerColors])));
            uint8_t b = bilerp(u, v, 
                            SkScalar(SkColorGetB(colors[kTopLeft_CornerColors])),
                            SkScalar(SkColorGetB(colors[kTopRight_CornerColors])),
                            SkScalar(SkColorGetB(colors[kBottomLeft_CornerColors])),
                            SkScalar(SkColorGetB(colors[kBottomRight_CornerColors])));
            data->fColors[dataIndex] = SkPackARGB32(a,r,g,b);
            
            data->fTexCoords[dataIndex] = SkPoint::Make(u, v);

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

size_t SkPatch::writeToMemory(void* storage) const {
    int byteCount =  kNumCtrlPts * sizeof(SkPoint) + kNumColors * sizeof(SkColor);
    
    if (NULL == storage) {
        return SkAlign4(byteCount);
    }
    
    SkWBuffer buffer(storage);
    
    buffer.write(fCtrlPoints, kNumCtrlPts * sizeof(SkPoint));
    buffer.write(fCornerColors, kNumColors * sizeof(SkColor));
    
    buffer.padToAlign4();
    return buffer.pos();
}

size_t SkPatch::readFromMemory(const void* storage, size_t length) {
    SkRBufferWithSizeCheck buffer(storage, length);
    
    if (!buffer.read(fCtrlPoints, kNumCtrlPts * sizeof(SkPoint))) {
        return 0;
    }
    
    if (!buffer.read(fCornerColors, kNumColors * sizeof(SkColor))) {
        return 0;
    }
    return kNumCtrlPts * sizeof(SkPoint) + kNumColors * sizeof(SkColor);
}
