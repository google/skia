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
    SkTextMapStateProc(const SkMatrix& matrix, const SkPoint& offset, int scalarsPerPosition)
        : fMatrix(matrix)
        , fProc(matrix.getMapXYProc())
        , fOffset(offset)
        , fScaleX(fMatrix.getScaleX()) {
        SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);
        if (1 == scalarsPerPosition) {
            unsigned mtype = fMatrix.getType();
            if (mtype & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
                fMapCase = kX;
            } else {
                // Bake the matrix scale/translation components into fOffset,
                // to expedite proc computations.
                fOffset.set(SkScalarMul(offset.x(), fMatrix.getScaleX()) + fMatrix.getTranslateX(),
                            SkScalarMul(offset.y(), fMatrix.getScaleY()) + fMatrix.getTranslateY());

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
    SkPoint  fOffset; // In kOnly* mode, this includes the matrix translation component.
    SkScalar fScaleX; // This is only used by kOnly... cases.
};

inline void SkTextMapStateProc::operator()(const SkScalar pos[], SkPoint* loc) const {
    switch(fMapCase) {
    case kXY:
        fProc(fMatrix, pos[0] + fOffset.x(), pos[1] + fOffset.y(), loc);
        break;
    case kOnlyScaleX:
        loc->set(SkScalarMul(fScaleX, *pos) + fOffset.x(), fOffset.y());
        break;
    case kOnlyTransX:
        loc->set(*pos + fOffset.x(), fOffset.y());
        break;
    default:
        SkASSERT(false);
    case kX:
        fProc(fMatrix, *pos + fOffset.x(), fOffset.y(), loc);
        break;
    }
}

#endif

