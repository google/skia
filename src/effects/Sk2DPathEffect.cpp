
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Sk2DPathEffect.h"
#include "SkFlattenableBuffers.h"
#include "SkPath.h"
#include "SkRegion.h"

Sk2DPathEffect::Sk2DPathEffect(const SkMatrix& mat) : fMatrix(mat) {
    fMatrixIsInvertible = mat.invert(&fInverse);
}

bool Sk2DPathEffect::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*) {
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

void Sk2DPathEffect::nextSpan(int x, int y, int count, SkPath* path) {
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

void Sk2DPathEffect::begin(const SkIRect& uvBounds, SkPath* dst) {}
void Sk2DPathEffect::next(const SkPoint& loc, int u, int v, SkPath* dst) {}
void Sk2DPathEffect::end(SkPath* dst) {}

///////////////////////////////////////////////////////////////////////////////

void Sk2DPathEffect::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fMatrix);
}

Sk2DPathEffect::Sk2DPathEffect(SkFlattenableReadBuffer& buffer) {
    buffer.readMatrix(&fMatrix);
    fMatrixIsInvertible = fMatrix.invert(&fInverse);
}

SK_DEFINE_FLATTENABLE_REGISTRAR(Sk2DPathEffect)

///////////////////////////////////////////////////////////////////////////////

bool SkLine2DPathEffect::filterPath(SkPath *dst, const SkPath &src, SkStrokeRec *rec) {
    if (this->INHERITED::filterPath(dst, src, rec)) {
        rec->setStrokeStyle(fWidth);
        return true;
    }
    return false;
}

void SkLine2DPathEffect::nextSpan(int u, int v, int ucount, SkPath *dst) {
    if (ucount > 1) {
        SkPoint    src[2], dstP[2];

        src[0].set(SkIntToScalar(u) + SK_ScalarHalf, SkIntToScalar(v) + SK_ScalarHalf);
        src[1].set(SkIntToScalar(u+ucount) + SK_ScalarHalf, SkIntToScalar(v) + SK_ScalarHalf);
        this->getMatrix().mapPoints(dstP, src, 2);

        dst->moveTo(dstP[0]);
        dst->lineTo(dstP[1]);
    }
}

SkLine2DPathEffect::SkLine2DPathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fWidth = buffer.readScalar();
}

void SkLine2DPathEffect::flatten(SkFlattenableWriteBuffer &buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fWidth);
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkLine2DPathEffect)

///////////////////////////////////////////////////////////////////////////////

SkPath2DPathEffect::SkPath2DPathEffect(const SkMatrix& m, const SkPath& p)
    : INHERITED(m), fPath(p) {
}

SkPath2DPathEffect::SkPath2DPathEffect(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
    buffer.readPath(&fPath);
}

void SkPath2DPathEffect::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePath(fPath);
}

void SkPath2DPathEffect::next(const SkPoint& loc, int u, int v, SkPath* dst) {
    dst->addPath(fPath, loc.fX, loc.fY);
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR(SkPath2DPathEffect)
