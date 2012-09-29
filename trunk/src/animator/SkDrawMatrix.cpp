
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawMatrix.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkParse.h"
#include "SkMatrixParts.h"
#include "SkScript.h"
#include "SkTypedArray.h"

enum SkDrawMatrix_Properties {
    SK_PROPERTY(perspectX),
    SK_PROPERTY(perspectY),
    SK_PROPERTY(rotate),
    SK_PROPERTY(scale),
    SK_PROPERTY(scaleX),
    SK_PROPERTY(scaleY),
    SK_PROPERTY(skewX),
    SK_PROPERTY(skewY),
    SK_PROPERTY(translate),
    SK_PROPERTY(translateX),
    SK_PROPERTY(translateY)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawMatrix::fInfo[] = {
    SK_MEMBER_ARRAY(matrix, Float),
    SK_MEMBER_PROPERTY(perspectX, Float),
    SK_MEMBER_PROPERTY(perspectY, Float),
    SK_MEMBER_PROPERTY(rotate, Float),
    SK_MEMBER_PROPERTY(scale, Float),
    SK_MEMBER_PROPERTY(scaleX, Float),
    SK_MEMBER_PROPERTY(scaleY, Float),
    SK_MEMBER_PROPERTY(skewX, Float),
    SK_MEMBER_PROPERTY(skewY, Float),
    SK_MEMBER_PROPERTY(translate, Point),
    SK_MEMBER_PROPERTY(translateX, Float),
    SK_MEMBER_PROPERTY(translateY, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawMatrix);

SkDrawMatrix::SkDrawMatrix() : fChildHasID(false), fDirty(false) {
    fConcat.reset();
    fMatrix.reset();
}

SkDrawMatrix::~SkDrawMatrix() {
    for (SkMatrixPart** part = fParts.begin(); part < fParts.end();  part++)
        delete *part;
}

bool SkDrawMatrix::add(SkAnimateMaker& maker, SkDisplayable* child) {
    SkASSERT(child && child->isMatrixPart());
    SkMatrixPart* part = (SkMatrixPart*) child;
    *fParts.append() = part;
    if (part->add())
        maker.setErrorCode(SkDisplayXMLParserError::kErrorAddingToMatrix);
    return true;
}

bool SkDrawMatrix::childrenNeedDisposing() const {
    return false;
}

SkDisplayable* SkDrawMatrix::deepCopy(SkAnimateMaker* maker) {
    SkDrawMatrix* copy = (SkDrawMatrix*)
        SkDisplayType::CreateInstance(maker, SkType_Matrix);
    SkASSERT(fParts.count() == 0);
    copy->fMatrix = fMatrix;
    copy->fConcat = fConcat;
    return copy;
}

void SkDrawMatrix::dirty() {
    fDirty = true;
}

bool SkDrawMatrix::draw(SkAnimateMaker& maker) {
    SkMatrix& concat = getMatrix();
    maker.fCanvas->concat(concat);
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkDrawMatrix::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    if (fMatrix.isIdentity()) {
        SkDebugf("matrix=\"identity\"/>\n");
        return;
    }
    SkScalar result;
    result = fMatrix[SkMatrix::kMScaleX];
    if (result != SK_Scalar1)
        SkDebugf("sx=\"%g\" ", SkScalarToFloat(result));
    result = fMatrix.getScaleY();
    if (result != SK_Scalar1)
        SkDebugf("sy=\"%g\" ", SkScalarToFloat(result));
    result = fMatrix.getSkewX();
    if (result)
        SkDebugf("skew-x=\"%g\" ", SkScalarToFloat(result));
    result = fMatrix.getSkewY();
    if (result)
        SkDebugf("skew-y=\"%g\" ", SkScalarToFloat(result));
    result = fMatrix.getTranslateX();
    if (result)
        SkDebugf("tx=\"%g\" ", SkScalarToFloat(result));
    result = fMatrix.getTranslateY();
    if (result)
        SkDebugf("ty=\"%g\" ", SkScalarToFloat(result));
    result = SkPerspToScalar(fMatrix.getPerspX());
    if (result)
        SkDebugf("perspect-x=\"%g\" ", SkScalarToFloat(result));
    result = SkPerspToScalar(fMatrix.getPerspY());
    if (result)
        SkDebugf("perspect-y=\"%g\" ", SkScalarToFloat(result));
    SkDebugf("/>\n");
}
#endif

SkMatrix& SkDrawMatrix::getMatrix() {
    if (fDirty == false)
        return fConcat;
    fMatrix.reset();
    for (SkMatrixPart** part = fParts.begin(); part < fParts.end();  part++) {
        (*part)->add();
        fConcat = fMatrix;
    }
    fDirty = false;
    return fConcat;
}

bool SkDrawMatrix::getProperty(int index, SkScriptValue* value) const {
    value->fType = SkType_Float;
    SkScalar result;
    switch (index) {
        case SK_PROPERTY(perspectX):
            result = fMatrix.getPerspX();
            break;
        case SK_PROPERTY(perspectY):
            result = fMatrix.getPerspY();
            break;
        case SK_PROPERTY(scaleX):
            result = fMatrix.getScaleX();
            break;
        case SK_PROPERTY(scaleY):
            result = fMatrix.getScaleY();
            break;
        case SK_PROPERTY(skewX):
            result = fMatrix.getSkewX();
            break;
        case SK_PROPERTY(skewY):
            result = fMatrix.getSkewY();
            break;
        case SK_PROPERTY(translateX):
            result = fMatrix.getTranslateX();
            break;
        case SK_PROPERTY(translateY):
            result = fMatrix.getTranslateY();
            break;
        default:
//          SkASSERT(0);
            return false;
    }
    value->fOperand.fScalar = result;
    return true;
}

void SkDrawMatrix::initialize() {
    fConcat = fMatrix;
}

void SkDrawMatrix::onEndElement(SkAnimateMaker& ) {
    if (matrix.count() > 0) {
        SkScalar* vals = matrix.begin();
        fMatrix.setScaleX(vals[0]);
        fMatrix.setSkewX(vals[1]);
        fMatrix.setTranslateX(vals[2]);
        fMatrix.setSkewY(vals[3]);
        fMatrix.setScaleY(vals[4]);
        fMatrix.setTranslateY(vals[5]);
        fMatrix.setPerspX(SkScalarToPersp(vals[6]));
        fMatrix.setPerspY(SkScalarToPersp(vals[7]));
//      fMatrix.setPerspW(SkScalarToPersp(vals[8]));
        goto setConcat;
    }
    if (fChildHasID == false) {
        {
            for (SkMatrixPart** part = fParts.begin(); part < fParts.end();  part++)
                delete *part;
        }
        fParts.reset();
setConcat:
        fConcat = fMatrix;
        fDirty = false;
    }
}

void SkDrawMatrix::setChildHasID() {
    fChildHasID = true;
}

bool SkDrawMatrix::setProperty(int index, SkScriptValue& scriptValue) {
    SkScalar number = scriptValue.fOperand.fScalar;
    switch (index) {
        case SK_PROPERTY(translate):
    //      SkScalar xy[2];
            SkASSERT(scriptValue.fType == SkType_Array);
            SkASSERT(scriptValue.fOperand.fArray->getType() == SkType_Float);
            SkASSERT(scriptValue.fOperand.fArray->count() == 2);
    //      SkParse::FindScalars(scriptValue.fOperand.fString->c_str(), xy, 2);
            fMatrix.setTranslateX((*scriptValue.fOperand.fArray)[0].fScalar);
            fMatrix.setTranslateY((*scriptValue.fOperand.fArray)[1].fScalar);
            return true;
        case SK_PROPERTY(perspectX):
            fMatrix.setPerspX(SkScalarToPersp((number)));
            break;
        case SK_PROPERTY(perspectY):
            fMatrix.setPerspY(SkScalarToPersp((number)));
            break;
        case SK_PROPERTY(rotate): {
            SkMatrix temp;
            temp.setRotate(number, 0, 0);
            fMatrix.setScaleX(temp.getScaleX());
            fMatrix.setScaleY(temp.getScaleY());
            fMatrix.setSkewX(temp.getSkewX());
            fMatrix.setSkewY(temp.getSkewY());
            } break;
        case SK_PROPERTY(scale):
            fMatrix.setScaleX(number);
            fMatrix.setScaleY(number);
            break;
        case SK_PROPERTY(scaleX):
            fMatrix.setScaleX(number);
            break;
        case SK_PROPERTY(scaleY):
            fMatrix.setScaleY(number);
            break;
        case SK_PROPERTY(skewX):
            fMatrix.setSkewX(number);
            break;
        case SK_PROPERTY(skewY):
            fMatrix.setSkewY(number);
            break;
        case SK_PROPERTY(translateX):
            fMatrix.setTranslateX(number);
            break;
        case SK_PROPERTY(translateY):
            fMatrix.setTranslateY(number);
            break;
        default:
            SkASSERT(0);
            return false;
    }
    fConcat = fMatrix;
    return true;
}

