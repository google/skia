/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextMapStateProc_DEFINED
#define SkTextMapStateProc_DEFINED

#include "SkPoint.h"
#include "SkMatrix.h"

class SkTextMapStateProc {
public:
    SkTextMapStateProc(const SkMatrix& matrix, SkScalar y, int scalarsPerPosition)
        : fMatrix(matrix)
        , fProc(matrix.getMapXYProc())
        , fY(y)
        , fScaleX(fMatrix.getScaleX())
        , fTransX(fMatrix.getTranslateX()) {
        SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);
        if (1 == scalarsPerPosition) {
            unsigned mtype = fMatrix.getType();
            if (mtype & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
                fMapCase = kX;
            } else {
                fY = SkScalarMul(y, fMatrix.getScaleY()) +
                    fMatrix.getTranslateY();
                if (mtype & SkMatrix::kScale_Mask) {
                    fMapCase = kOnlyScaleX;
                } else {
                    fMapCase = kOnlyTransX;
                }
            }
        } else {
            fMapCase = kXY;
        }
    }

    void operator()(const SkScalar pos[], SkPoint* loc) const;

private:
    const SkMatrix& fMatrix;
    enum {
        kXY,
        kOnlyScaleX,
        kOnlyTransX,
        kX
    } fMapCase;
    const SkMatrix::MapXYProc fProc;
    SkScalar fY; // Ignored by kXY case.
    SkScalar fScaleX, fTransX; // These are only used by Only... cases.
};

inline void SkTextMapStateProc::operator()(const SkScalar pos[], SkPoint* loc) const {
    switch(fMapCase) {
    case kXY:
        fProc(fMatrix, pos[0], pos[1], loc);
        break;
    case kOnlyScaleX:
        loc->set(SkScalarMul(fScaleX, *pos) + fTransX, fY);
        break;
    case kOnlyTransX:
        loc->set(*pos + fTransX, fY);
        break;
    default:
        SkASSERT(false);
    case kX:
        fProc(fMatrix, *pos, fY, loc);
        break;
    }
}

#endif

