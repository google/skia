
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawMatrix_DEFINED
#define SkDrawMatrix_DEFINED

#include "SkADrawable.h"
#include "SkMatrix.h"
#include "SkMemberInfo.h"
#include "SkIntArray.h"

class SkMatrixPart;

class SkDrawMatrix : public SkADrawable {
    DECLARE_DRAW_MEMBER_INFO(Matrix);
    SkDrawMatrix();
    virtual ~SkDrawMatrix();
    bool addChild(SkAnimateMaker& , SkDisplayable* child) SK_OVERRIDE;
    bool childrenNeedDisposing() const SK_OVERRIDE;
    void dirty() SK_OVERRIDE;
    bool draw(SkAnimateMaker& ) SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    SkMatrix& getMatrix();
    bool getProperty(int index, SkScriptValue* value) const SK_OVERRIDE;
    void initialize() SK_OVERRIDE;
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
    void setChildHasID() SK_OVERRIDE;
    bool setProperty(int index, SkScriptValue& ) SK_OVERRIDE;

    void concat(SkMatrix& inMatrix) {
        fConcat.preConcat(inMatrix);
    }

    SkDisplayable* deepCopy(SkAnimateMaker* ) SK_OVERRIDE;


    void rotate(SkScalar degrees, SkPoint& center) {
        fMatrix.preRotate(degrees, center.fX, center.fY);
    }

    void set(SkMatrix& src) {
        fMatrix.preConcat(src);
    }

    void scale(SkScalar scaleX, SkScalar scaleY, SkPoint& center) {
        fMatrix.preScale(scaleX, scaleY, center.fX, center.fY);
    }

    void skew(SkScalar skewX, SkScalar skewY, SkPoint& center) {
        fMatrix.preSkew(skewX, skewY, center.fX, center.fY);
    }

    void translate(SkScalar x, SkScalar y) {
        fMatrix.preTranslate(x, y);
    }
private:
    SkTDScalarArray matrix;
    SkMatrix fConcat;
    SkMatrix fMatrix;
    SkTDMatrixPartArray fParts;
    SkBool8 fChildHasID;
    SkBool8 fDirty;
    typedef SkADrawable INHERITED;
};

#endif // SkDrawMatrix_DEFINED
