
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Sk2DPathEffect.h"
#include "SkBlitter.h"
#include "SkPath.h"
#include "SkScan.h"

class Sk2DPathEffectBlitter : public SkBlitter {
public:
    Sk2DPathEffectBlitter(Sk2DPathEffect* pe, SkPath* dst)
        : fPE(pe), fDst(dst) {}

    virtual void blitH(int x, int y, int count) {
        fPE->nextSpan(x, y, count, fDst);
    }
private:
    Sk2DPathEffect* fPE;
    SkPath*         fDst;
};

///////////////////////////////////////////////////////////////////////////////

Sk2DPathEffect::Sk2DPathEffect(const SkMatrix& mat) : fMatrix(mat) {
    mat.invert(&fInverse);
}

bool Sk2DPathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width) {
    Sk2DPathEffectBlitter   blitter(this, dst);
    SkPath                  tmp;
    SkIRect                 ir;

    src.transform(fInverse, &tmp);
    tmp.getBounds().round(&ir);
    if (!ir.isEmpty()) {
        this->begin(ir, dst);
        SkScan::FillPath(tmp, ir, &blitter);
        this->end(dst);
    }
    return true;
}

void Sk2DPathEffect::nextSpan(int x, int y, int count, SkPath* path) {
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

void Sk2DPathEffect::flatten(SkFlattenableWriteBuffer& buffer) {
    char storage[SkMatrix::kMaxFlattenSize];
    uint32_t size = fMatrix.flatten(storage);
    buffer.write32(size);
    buffer.write(storage, size);
}

Sk2DPathEffect::Sk2DPathEffect(SkFlattenableReadBuffer& buffer) {
    char storage[SkMatrix::kMaxFlattenSize];
    uint32_t size = buffer.readS32();
    SkASSERT(size <= sizeof(storage));
    buffer.read(storage, size);
    fMatrix.unflatten(storage);
    fMatrix.invert(&fInverse);
}

SkFlattenable::Factory Sk2DPathEffect::getFactory() {
    return CreateProc;
}

SkFlattenable* Sk2DPathEffect::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(Sk2DPathEffect, (buffer));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkPath2DPathEffect::SkPath2DPathEffect(const SkMatrix& m, const SkPath& p)
    : INHERITED(m), fPath(p) {
}

SkPath2DPathEffect::SkPath2DPathEffect(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
    fPath.unflatten(buffer);
}

SkFlattenable* SkPath2DPathEffect::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkPath2DPathEffect, (buffer));
}

void SkPath2DPathEffect::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);
    fPath.flatten(buffer);
}

SkFlattenable::Factory SkPath2DPathEffect::getFactory() {
    return CreateProc;
}

void SkPath2DPathEffect::next(const SkPoint& loc, int u, int v, SkPath* dst) {
    dst->addPath(fPath, loc.fX, loc.fY);
}

static SkFlattenable::Registrar gReg("SkPath2DPathEffect",
                                     SkPath2DPathEffect::CreateProc);

