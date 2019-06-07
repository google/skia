/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXform_DEFINED
#define SkXform_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"

class XformResolver {
public:
    virtual ~XformResolver() {}

    virtual void concat(const SkMatrix&) = 0;
    virtual void clipRect(const SkRect&, SkClipOp) = 0;
    virtual void clipRRect(const SkRRect&, SkClipOp) = 0;
    virtual void clipPath(const SkPath&, SkClipOp) = 0;
};

class Xform : public SkRefCnt {
public:
    Xform* parent() const { return fParent.get(); }
    void setParent(sk_sp<Xform> p) {
        fParent = std::move(p);
    }

    void visit(XformResolver* resolver);

protected:
    Xform(sk_sp<Xform> parent = nullptr) : fParent(std::move(parent)) {}

    virtual void onVisit(XformResolver*) {}

private:
    sk_sp<Xform> fParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class MatrixXF : public Xform {
public:
    static sk_sp<MatrixXF> Make(sk_sp<Xform> parent = nullptr) {
        return sk_sp<MatrixXF>(new MatrixXF(std::move(parent)));
    }

    MatrixXF(sk_sp<Xform> parent) : Xform(std::move(parent)) {
        fLocalMatrix.reset();
    }

    void setLocalMatrix(const SkMatrix& m) {
        fLocalMatrix = m;
    }
    void setTranslate(SkScalar sx, SkScalar sy) {
        fLocalMatrix.setTranslate(sx, sy);
    }
    void setScale(SkScalar sx, SkScalar sy) {
        fLocalMatrix.setScale(sx, sy);
    }
    void setRotate(SkScalar degrees) {
        fLocalMatrix.setRotate(degrees);
    }

protected:
    void onVisit(XformResolver* resolver) override;

private:
    SkMatrix fLocalMatrix;
};

class ClipXF : public Xform {
public:
    ClipXF(sk_sp<Xform> parent = nullptr) : Xform(std::move(parent)) {}
    ClipXF(sk_sp<Xform> parent, const SkRect& r, SkClipOp op = SkClipOp::kIntersect)
        : Xform(std::move(parent))
        , fRect(r)
        , fOp(op)
    {}

    void setRect(const SkRect& r, SkClipOp op = SkClipOp::kIntersect) {
        fRect = r;
        fOp = op;
    }

protected:
    void onVisit(XformResolver* resolver) override;

private:
    SkRect      fRect;
    SkClipOp    fOp;
};

#endif
