
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawRectangle.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkMatrixParts.h"
#include "SkPaint.h"
#include "SkScript.h"

enum SkRectangle_Properties {
    SK_PROPERTY(height),
    SK_PROPERTY(needsRedraw),
    SK_PROPERTY(width)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawRect::fInfo[] = {
    SK_MEMBER_ALIAS(bottom, fRect.fBottom, Float),
    SK_MEMBER_PROPERTY(height, Float),
    SK_MEMBER_ALIAS(left, fRect.fLeft, Float),
    SK_MEMBER_PROPERTY(needsRedraw, Boolean),
    SK_MEMBER_ALIAS(right, fRect.fRight, Float),
    SK_MEMBER_ALIAS(top, fRect.fTop, Float),
    SK_MEMBER_PROPERTY(width, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawRect);

SkDrawRect::SkDrawRect() : fParent(NULL) { 
    fRect.setEmpty(); 
}

void SkDrawRect::dirty() {
    if (fParent)
        fParent->dirty();
}

bool SkDrawRect::draw(SkAnimateMaker& maker) {
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawRect(fRect, *maker.fPaint);
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkDrawRect::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("left=\"%g\" top=\"%g\" right=\"%g\" bottom=\"%g\" />\n",
        SkScalarToFloat(fRect.fLeft), SkScalarToFloat(fRect.fTop), SkScalarToFloat(fRect.fRight),
        SkScalarToFloat(fRect.fBottom));
}
#endif

SkDisplayable* SkDrawRect::getParent() const {
    return fParent;
}

bool SkDrawRect::getProperty(int index, SkScriptValue* value) const {
    SkScalar result;
    switch (index) {
        case SK_PROPERTY(height):
            result = fRect.height();
            break;
        case SK_PROPERTY(needsRedraw):
            value->fType = SkType_Boolean;
            value->fOperand.fS32 = fBounds.isEmpty() == false;
            return true;
        case SK_PROPERTY(width):
            result = fRect.width();
            break;
        default:
            SkASSERT(0);
            return false;
    }
    value->fType = SkType_Float;
    value->fOperand.fScalar = result;
    return true;
}


bool SkDrawRect::setParent(SkDisplayable* parent) {
    fParent = parent;
    return false;
}

bool SkDrawRect::setProperty(int index, SkScriptValue& value) {
    SkScalar scalar = value.fOperand.fScalar;
    switch (index) {
        case SK_PROPERTY(height):
            SkASSERT(value.fType == SkType_Float);
            fRect.fBottom = scalar + fRect.fTop;
            return true;
        case SK_PROPERTY(needsRedraw):
            return false;
        case SK_PROPERTY(width):
            SkASSERT(value.fType == SkType_Float);
            fRect.fRight = scalar + fRect.fLeft;
            return true;
        default:
            SkASSERT(0);
    }
    return false;
}

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRoundRect::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(rx, Float),
    SK_MEMBER(ry, Float),
};

#endif

DEFINE_GET_MEMBER(SkRoundRect);

SkRoundRect::SkRoundRect() : rx(0), ry(0) {
}

bool SkRoundRect::draw(SkAnimateMaker& maker) {
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawRoundRect(fRect, rx, ry, *maker.fPaint);
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkRoundRect::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("left=\"%g\" top=\"%g\" right=\"%g\" bottom=\"%g\" rx=\"%g\" ry=\"%g\" />\n",
            SkScalarToFloat(fRect.fLeft), SkScalarToFloat(fRect.fTop), SkScalarToFloat(fRect.fRight),
            SkScalarToFloat(fRect.fBottom), SkScalarToFloat(rx), SkScalarToFloat(ry));
}
#endif



