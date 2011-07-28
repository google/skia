
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
#include "SkUnitMapper.h"

SkScalar SkUnitToScalar(U16CPU x) {
#ifdef SK_SCALAR_IS_FLOAT
    return x / 65535.0f;
#else
    return x + (x >> 8);
#endif
}

U16CPU SkScalarToUnit(SkScalar x) {
    SkScalar pin =  SkScalarPin(x, 0, SK_Scalar1);
#ifdef SK_SCALAR_IS_FLOAT
    return (int) (pin * 65535.0f);
#else
    return pin - (pin >= 32768);
#endif
}

class SkGradientUnitMapper : public SkUnitMapper {
public:
    SkGradientUnitMapper(SkAnimateMaker* maker, const char* script) : fMaker(maker), fScript(script) {
    }
    
    // overrides for SkFlattenable
    virtual Factory getFactory() { return NULL; }
    
protected:
    virtual uint16_t mapUnit16(uint16_t x) {
        fUnit = SkUnitToScalar(x);
        SkScriptValue value;
        SkAnimatorScript engine(*fMaker, NULL, SkType_Float);
        engine.propertyCallBack(GetUnitValue, &fUnit);
        if (engine.evaluate(fScript, &value, SkType_Float)) 
            x = SkScalarToUnit(value.fOperand.fScalar);
        return x;
    }

    static bool GetUnitValue(const char* token, size_t len, void* unitPtr, SkScriptValue* value) {
        if (SK_LITERAL_STR_EQUAL("unit", token, len)) {
            value->fOperand.fScalar = *(SkScalar*) unitPtr;
            value->fType = SkType_Float;
            return true;
        }
        return false;
    }

    SkAnimateMaker* fMaker;
    const char* fScript;
    SkScalar fUnit;
};


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkGradient::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER_ARRAY(offsets, Float),
    SK_MEMBER(unitMapper, String)
};

#endif

DEFINE_GET_MEMBER(SkGradient);

SkGradient::SkGradient() : fUnitMapper(NULL) {
}

SkGradient::~SkGradient() {
    for (int index = 0; index < fDrawColors.count(); index++) 
        delete fDrawColors[index];
    delete fUnitMapper;
}

bool SkGradient::add(SkAnimateMaker& , SkDisplayable* child) {
    SkASSERT(child);
    if (child->isColor()) {
        SkDrawColor* color = (SkDrawColor*) child;
        *fDrawColors.append() = color;
        return true;
    }
    return false;
}

int SkGradient::addPrelude() {
    int count = fDrawColors.count();
    fColors.setCount(count);
    for (int index = 0; index < count; index++) 
        fColors[index] = fDrawColors[index]->color;
    return count;
}

#ifdef SK_DUMP_ENABLED
void SkGradient::dumpRest(SkAnimateMaker* maker) {
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

void SkGradient::onEndElement(SkAnimateMaker& maker) {
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
    if (unitMapper.size() > 0) 
        fUnitMapper = new SkGradientUnitMapper(&maker, unitMapper.c_str());
    INHERITED::onEndElement(maker);
}

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkLinearGradient::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER_ARRAY(points, Float),
};

#endif

DEFINE_GET_MEMBER(SkLinearGradient);

SkLinearGradient::SkLinearGradient() { 
}

void SkLinearGradient::onEndElement(SkAnimateMaker& maker)
{
    if (points.count() != 4)
        maker.setErrorCode(SkDisplayXMLParserError::kGradientPointsLengthMustBeFour);
    INHERITED::onEndElement(maker);
}

#ifdef SK_DUMP_ENABLED
void SkLinearGradient::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    dumpRest(maker);
    }
#endif

SkShader* SkLinearGradient::getShader() {
    if (addPrelude() == 0 || points.count() != 4)
        return NULL;
    SkShader* shader = SkGradientShader::CreateLinear((SkPoint*)points.begin(),
        fColors.begin(), offsets.begin(), fColors.count(), (SkShader::TileMode) tileMode, fUnitMapper);
    SkAutoTDelete<SkShader> autoDel(shader);
    addPostlude(shader);
    (void)autoDel.detach();
    return shader;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRadialGradient::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(center, Point),
    SK_MEMBER(radius, Float)
};

#endif

DEFINE_GET_MEMBER(SkRadialGradient);

SkRadialGradient::SkRadialGradient() : radius(0) { 
    center.set(0, 0); 
}

#ifdef SK_DUMP_ENABLED
void SkRadialGradient::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    dumpRest(maker);
}
#endif

SkShader* SkRadialGradient::getShader() {
    if (addPrelude() == 0)
        return NULL;
    SkShader* shader = SkGradientShader::CreateRadial(center,
        radius, fColors.begin(), offsets.begin(), fColors.count(), (SkShader::TileMode) tileMode, fUnitMapper);
    SkAutoTDelete<SkShader> autoDel(shader);
    addPostlude(shader);
    (void)autoDel.detach();
    return shader;
}
