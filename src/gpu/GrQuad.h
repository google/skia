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
 * points make a trip strip with CCW triangles (top-left, bottom-left, top-right, bottom-right).
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

    void set(const SkRect& rect) {
        fPoints[0].set(rect.fLeft, rect.fTop);
        fPoints[1].set(rect.fLeft, rect.fBottom);
        fPoints[2].set(rect.fRight, rect.fTop);
        fPoints[3].set(rect.fRight, rect.fBottom);
    }

    void map(const SkMatrix& matrix) {
        matrix.mapPoints(fPoints, kNumPoints);
    }

    void setFromMappedRect(const SkRect& rect, const SkMatrix& matrix) {
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

#endif
