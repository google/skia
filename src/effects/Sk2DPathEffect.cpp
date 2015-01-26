
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Sk2DPathEffect.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPath.h"
#include "SkRegion.h"

Sk2DPathEffect::Sk2DPathEffect(const SkMatrix& mat) : fMatrix(mat) {
    fMatrixIsInvertible = mat.invert(&fInverse);
}

bool Sk2DPathEffect::filterPath(SkPath* dst, const SkPath& src,
                                SkStrokeRec*, const SkRect*) const {
    if (!fMatrixIsInvertible) {
        return false;
    }

    SkPath  tmp;
    SkIRect ir;

    src.transform(fInverse, &tmp);
    tmp.getBounds().round(&ir);
    if (!ir.isEmpty()) {
        this->begin(ir, dst);

        SkRegion rgn;
        rgn.setPath(tmp, SkRegion(ir));
        SkRegion::Iterator iter(rgn);
        for (; !iter.done(); iter.next()) {
            const SkIRect& rect = iter.rect();
            for (int y = rect.fTop; y < rect.fBottom; ++y) {
                this->nextSpan(rect.fLeft, y, rect.width(), dst);
            }
        }

        this->end(dst);
    }
    return true;
}

void Sk2DPathEffect::nextSpan(int x, int y, int count, SkPath* path) const {
    if (!fMatrixIsInvertible) {
        return;
    }

    const SkMatrix& mat = this->getMatrix();
    SkPoint src, dst;

    src.set(SkIntToScalar(x) + SK_ScalarHalf, SkIntToScalar(y) + SK_ScalarHalf);
    do {
        mat.mapPoints(&dst, &src, 1);
        this->next(dst, x++, y, path);
        src.fX += SK_Scalar1;
    } while (--count > 0);
}

void Sk2DPathEffect::begin(const SkIRect& uvBounds, SkPath* dst) const {}
void Sk2DPathEffect::next(const SkPoint& loc, int u, int v, SkPath* dst) const {}
void Sk2DPathEffect::end(SkPath* dst) const {}

///////////////////////////////////////////////////////////////////////////////

void Sk2DPathEffect::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fMatrix);
}

#ifndef SK_IGNORE_TO_STRING
void Sk2DPathEffect::toString(SkString* str) const {
    str->appendf("(matrix: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f)",
            fMatrix[SkMatrix::kMScaleX], fMatrix[SkMatrix::kMSkewX],  fMatrix[SkMatrix::kMTransX],
            fMatrix[SkMatrix::kMSkewY],  fMatrix[SkMatrix::kMScaleY], fMatrix[SkMatrix::kMTransY],
            fMatrix[SkMatrix::kMPersp0], fMatrix[SkMatrix::kMPersp1], fMatrix[SkMatrix::kMPersp2]);
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool SkLine2DPathEffect::filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec* rec, const SkRect* cullRect) const {
    if (this->INHERITED::filterPath(dst, src, rec, cullRect)) {
        rec->setStrokeStyle(fWidth);
        return true;
    }
    return false;
}

void SkLine2DPathEffect::nextSpan(int u, int v, int ucount, SkPath* dst) const {
    if (ucount > 1) {
        SkPoint    src[2], dstP[2];

        src[0].set(SkIntToScalar(u) + SK_ScalarHalf, SkIntToScalar(v) + SK_ScalarHalf);
        src[1].set(SkIntToScalar(u+ucount) + SK_ScalarHalf, SkIntToScalar(v) + SK_ScalarHalf);
        this->getMatrix().mapPoints(dstP, src, 2);

        dst->moveTo(dstP[0]);
        dst->lineTo(dstP[1]);
    }
}

SkFlattenable* SkLine2DPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    SkScalar width = buffer.readScalar();
    return SkLine2DPathEffect::Create(width, matrix);
}

void SkLine2DPathEffect::flatten(SkWriteBuffer &buffer) const {
    buffer.writeMatrix(this->getMatrix());
    buffer.writeScalar(fWidth);
}


#ifndef SK_IGNORE_TO_STRING
void SkLine2DPathEffect::toString(SkString* str) const {
    str->appendf("SkLine2DPathEffect: (");
    this->INHERITED::toString(str);
    str->appendf("width: %f", fWidth);
    str->appendf(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkPath2DPathEffect::SkPath2DPathEffect(const SkMatrix& m, const SkPath& p)
    : INHERITED(m), fPath(p) {
}

SkFlattenable* SkPath2DPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    SkPath path;
    buffer.readPath(&path);
    return SkPath2DPathEffect::Create(matrix, path);
}

void SkPath2DPathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getMatrix());
    buffer.writePath(fPath);
}

void SkPath2DPathEffect::next(const SkPoint& loc, int u, int v,
                              SkPath* dst) const {
    dst->addPath(fPath, loc.fX, loc.fY);
}

#ifndef SK_IGNORE_TO_STRING
void SkPath2DPathEffect::toString(SkString* str) const {
    str->appendf("SkPath2DPathEffect: (");
    this->INHERITED::toString(str);
    // TODO: print out path information
    str->appendf(")");
}
#endif
