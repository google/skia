/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkStrokeRec.h"
#include "include/effects/Sk2DPathEffect.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

class Sk2DPathEffect : public SkPathEffectBase {
public:
    Sk2DPathEffect(const SkMatrix& mat) : fMatrix(mat) {
        // Calling invert will set the type mask on both matrices, making them thread safe.
        fMatrixIsInvertible = fMatrix.invert(&fInverse);
    }

protected:
    /** New virtual, to be overridden by subclasses.
        This is called once from filterPath, and provides the
        uv parameter bounds for the path. Subsequent calls to
        next() will receive u and v values within these bounds,
        and then a call to end() will signal the end of processing.
    */
    virtual void begin(const SkIRect& uvBounds, SkPath* dst) const {}
    virtual void next(const SkPoint& loc, int u, int v, SkPath* dst) const {}
    virtual void end(SkPath* dst) const {}

    /** Low-level virtual called per span of locations in the u-direction.
        The default implementation calls next() repeatedly with each
        location.
    */
    virtual void nextSpan(int x, int y, int ucount, SkPath* path) const {
        if (!fMatrixIsInvertible) {
            return;
        }
    #if defined(SK_BUILD_FOR_FUZZER)
        if (ucount > 100) {
            return;
        }
    #endif

        const SkMatrix& mat = this->getMatrix();
        SkPoint src, dst;

        src.set(SkIntToScalar(x) + SK_ScalarHalf, SkIntToScalar(y) + SK_ScalarHalf);
        do {
            mat.mapPoints(&dst, &src, 1);
            this->next(dst, x++, y, path);
            src.fX += SK_Scalar1;
        } while (--ucount > 0);
    }

    const SkMatrix& getMatrix() const { return fMatrix; }

    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        buffer.writeMatrix(fMatrix);
    }

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                      const SkRect* cullRect, const SkMatrix&) const override {
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
#if defined(SK_BUILD_FOR_FUZZER)
                if (rect.height() > 100) {
                    continue;
                }
#endif
                for (int y = rect.fTop; y < rect.fBottom; ++y) {
                    this->nextSpan(rect.fLeft, y, rect.width(), dst);
                }
            }

            this->end(dst);
        }
        return true;
    }

private:
    SkMatrix    fMatrix, fInverse;
    bool        fMatrixIsInvertible;

    // For simplicity, assume fast bounds cannot be computed
    bool computeFastBounds(SkRect*) const override { return false; }

    friend class Sk2DPathEffectBlitter;
    using INHERITED = SkPathEffect;
};

///////////////////////////////////////////////////////////////////////////////

class SkLine2DPathEffectImpl : public Sk2DPathEffect {
public:
    SkLine2DPathEffectImpl(SkScalar width, const SkMatrix& matrix)
        : Sk2DPathEffect(matrix)
        , fWidth(width)
    {
        SkASSERT(width >= 0);
    }

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                      const SkRect* cullRect, const SkMatrix& ctm) const override {
        if (this->INHERITED::onFilterPath(dst, src, rec, cullRect, ctm)) {
            rec->setStrokeStyle(fWidth);
            return true;
        }
        return false;
    }

    void nextSpan(int u, int v, int ucount, SkPath* dst) const override {
        if (ucount > 1) {
            SkPoint    src[2], dstP[2];

            src[0].set(SkIntToScalar(u) + SK_ScalarHalf, SkIntToScalar(v) + SK_ScalarHalf);
            src[1].set(SkIntToScalar(u+ucount) + SK_ScalarHalf, SkIntToScalar(v) + SK_ScalarHalf);
            this->getMatrix().mapPoints(dstP, src, 2);

            dst->moveTo(dstP[0]);
            dst->lineTo(dstP[1]);
        }
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        SkMatrix matrix;
        buffer.readMatrix(&matrix);
        SkScalar width = buffer.readScalar();
        return SkLine2DPathEffect::Make(width, matrix);
    }

    void flatten(SkWriteBuffer &buffer) const override {
        buffer.writeMatrix(this->getMatrix());
        buffer.writeScalar(fWidth);
    }

    Factory getFactory() const override { return CreateProc; }
    const char* getTypeName() const override { return "SkLine2DPathEffect"; }

private:
    SkScalar fWidth;

    using INHERITED = Sk2DPathEffect;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class SK_API SkPath2DPathEffectImpl : public Sk2DPathEffect {
public:
    SkPath2DPathEffectImpl(const SkMatrix& m, const SkPath& p) : INHERITED(m), fPath(p) {}

    void next(const SkPoint& loc, int u, int v, SkPath* dst) const override {
        dst->addPath(fPath, loc.fX, loc.fY);
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        SkMatrix matrix;
        buffer.readMatrix(&matrix);
        SkPath path;
        buffer.readPath(&path);
        return SkPath2DPathEffect::Make(matrix, path);
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeMatrix(this->getMatrix());
        buffer.writePath(fPath);
    }

    Factory getFactory() const override { return CreateProc; }
    const char* getTypeName() const override { return "SkPath2DPathEffect"; }

private:
    SkPath  fPath;

    using INHERITED = Sk2DPathEffect;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkLine2DPathEffect::Make(SkScalar width, const SkMatrix& matrix) {
    if (!(width >= 0)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkLine2DPathEffectImpl(width, matrix));
}

sk_sp<SkPathEffect> SkPath2DPathEffect::Make(const SkMatrix& matrix, const SkPath& path) {
    return sk_sp<SkPathEffect>(new SkPath2DPathEffectImpl(matrix, path));
}

void SkLine2DPathEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkLine2DPathEffectImpl);
}

void SkPath2DPathEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkPath2DPathEffectImpl);
}
