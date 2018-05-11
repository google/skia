/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuad_DEFINED
#define GrQuad_DEFINED

#include "SkPoint.h"
#include "SkMatrix.h"
#include "SkMatrixPriv.h"

/**
 * GrQuad is a collection of 4 points which can be used to represent an arbitrary quadrilateral. The
 * points make a triangle strip with CCW triangles (top-left, bottom-left, top-right, bottom-right).
 */
class GrQuad {
public:
    GrQuad() {}

    GrQuad(const GrQuad& that) {
        *this = that;
    }

    explicit GrQuad(const SkRect& rect) {
        this->set(rect);
    }

    GrQuad(const SkRect& rect, const SkMatrix& matrix) {
        this->setFromMappedRect(rect, matrix);
    }

    void set(const SkRect& rect) {
        SkPointPriv::SetRectTriStrip(fPoints, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom,
                sizeof(SkPoint));
    }

    void map(const SkMatrix& matrix) {
        matrix.mapPoints(fPoints, kNumPoints);
    }

    void setFromMappedRect(const SkRect& rect, const SkMatrix& matrix) {
        SkASSERT(!matrix.hasPerspective());
        SkMatrixPriv::SetMappedRectTriStrip(matrix, rect, fPoints);
    }

    const GrQuad& operator=(const GrQuad& that) {
        memcpy(fPoints, that.fPoints, sizeof(SkPoint) * kNumPoints);
        return *this;
    }

    SkPoint* points() {
        return fPoints;
    }

    const SkPoint* points() const {
        return fPoints;
    }

    const SkPoint& point(int i) const {
        SkASSERT(i < kNumPoints);
        return fPoints[i];
    }

private:
    static const int kNumPoints = 4;
    SkPoint fPoints[kNumPoints];
};

class GrPerspQuad {
public:
    GrPerspQuad() = default;

    GrPerspQuad(const SkRect& rect, const SkMatrix& matrix) {
        fPoints[0].set(rect.fLeft, rect.fTop, 1.f);
        fPoints[1].set(rect.fLeft, rect.fBottom, 1.f);
        fPoints[2].set(rect.fRight, rect.fTop, 1.f);
        fPoints[3].set(rect.fRight, rect.fBottom, 1.f);
        matrix.mapHomogeneousPoints(fPoints, fPoints, kNumPoints);
    }

    const GrPerspQuad& operator=(const GrPerspQuad& that) {
        memcpy(fPoints, that.fPoints, sizeof(SkPoint3) * kNumPoints);
        return *this;
    }

    SkPoint3* points() { return fPoints; }

    const SkPoint3* points() const { return fPoints; }

    const SkPoint3& point(int i) const {
        SkASSERT(i < kNumPoints);
        return fPoints[i];
    }

private:
    static const int kNumPoints = 4;
    SkPoint3 fPoints[kNumPoints];
};

#endif
