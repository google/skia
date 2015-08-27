
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawGradient.h"
#include "SkAnimateMaker.h"
#include "SkAnimatorScript.h"
#include "SkGradientShader.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawGradient::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER_ARRAY(offsets, Float),
    SK_MEMBER(unitMapper, String)
};

#endif

DEFINE_GET_MEMBER(SkDrawGradient);

SkDrawGradient::SkDrawGradient() {
}

SkDrawGradient::~SkDrawGradient() {
    for (int index = 0; index < fDrawColors.count(); index++)
        delete fDrawColors[index];
}

bool SkDrawGradient::addChild(SkAnimateMaker& , SkDisplayable* child) {
    SkASSERT(child);
    if (child->isColor()) {
        SkDrawColor* color = (SkDrawColor*) child;
        *fDrawColors.append() = color;
        return true;
    }
    return false;
}

int SkDrawGradient::addPrelude() {
    int count = fDrawColors.count();
    fColors.setCount(count);
    for (int index = 0; index < count; index++)
        fColors[index] = fDrawColors[index]->color;
    return count;
}

#ifdef SK_DUMP_ENABLED
void SkDrawGradient::dumpRest(SkAnimateMaker* maker) {
    dumpAttrs(maker);
    //can a gradient have no colors?
    bool closedYet = false;
    SkDisplayList::fIndent += 4;
    for (SkDrawColor** ptr = fDrawColors.begin(); ptr < fDrawColors.end(); ptr++) {
        if (closedYet == false) {
            SkDebugf(">\n");
            closedYet = true;
        }
        SkDrawColor* color = *ptr;
        color->dump(maker);
    }
    SkDisplayList::fIndent -= 4;
    dumpChildren(maker, closedYet); //dumps the matrix if it has one
}
#endif

void SkDrawGradient::onEndElement(SkAnimateMaker& maker) {
    if (offsets.count() != 0) {
        if (offsets.count() != fDrawColors.count()) {
            maker.setErrorCode(SkDisplayXMLParserError::kGradientOffsetsDontMatchColors);
            return;
        }
        if (offsets[0] != 0) {
            maker.setErrorCode(SkDisplayXMLParserError::kGradientOffsetsMustStartWithZero);
            return;
        }
        if (offsets[offsets.count()-1] != SK_Scalar1) {
            maker.setErrorCode(SkDisplayXMLParserError::kGradientOffsetsMustEndWithOne);
            return;
        }
        for (int i = 1; i < offsets.count(); i++) {
            if (offsets[i] <= offsets[i-1]) {
                maker.setErrorCode(SkDisplayXMLParserError::kGradientOffsetsMustIncrease);
                return;
            }
            if (offsets[i] > SK_Scalar1) {
                maker.setErrorCode(SkDisplayXMLParserError::kGradientOffsetsMustBeNoMoreThanOne);
                return;
            }
        }
    }
    INHERITED::onEndElement(maker);
}

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawLinearGradient::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER_ARRAY(points, Float),
};

#endif

DEFINE_GET_MEMBER(SkDrawLinearGradient);

SkDrawLinearGradient::SkDrawLinearGradient() {
}

void SkDrawLinearGradient::onEndElement(SkAnimateMaker& maker)
{
    if (points.count() != 4)
        maker.setErrorCode(SkDisplayXMLParserError::kGradientPointsLengthMustBeFour);
    INHERITED::onEndElement(maker);
}

#ifdef SK_DUMP_ENABLED
void SkDrawLinearGradient::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    dumpRest(maker);
    }
#endif

SkShader* SkDrawLinearGradient::getShader() {
    if (addPrelude() == 0 || points.count() != 4)
        return nullptr;
    SkShader* shader = SkGradientShader::CreateLinear((SkPoint*)points.begin(),
        fColors.begin(), offsets.begin(), fColors.count(), (SkShader::TileMode) tileMode,
        0, getMatrix());
    SkAutoTDelete<SkShader> autoDel(shader);
    (void)autoDel.detach();
    return shader;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawRadialGradient::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(center, Point),
    SK_MEMBER(radius, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawRadialGradient);

SkDrawRadialGradient::SkDrawRadialGradient() : radius(0) {
    center.set(0, 0);
}

#ifdef SK_DUMP_ENABLED
void SkDrawRadialGradient::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    dumpRest(maker);
}
#endif

SkShader* SkDrawRadialGradient::getShader() {
    if (addPrelude() == 0)
        return nullptr;
    SkShader* shader = SkGradientShader::CreateRadial(center,
        radius, fColors.begin(), offsets.begin(), fColors.count(), (SkShader::TileMode) tileMode,
        0, getMatrix());
    SkAutoTDelete<SkShader> autoDel(shader);
    (void)autoDel.detach();
    return shader;
}
