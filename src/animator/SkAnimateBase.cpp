
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAnimateBase.h"
#include "SkAnimateMaker.h"
#include "SkAnimateProperties.h"
#include "SkAnimatorScript.h"
#include "SkDisplayApply.h"
#include "SkADrawable.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAnimateBase::fInfo[] = {
    SK_MEMBER(begin, MSec),
    SK_MEMBER_ARRAY(blend, Float),
    SK_MEMBER(dur, MSec),
    SK_MEMBER_PROPERTY(dynamic, Boolean),
    SK_MEMBER(field, String),   // name of member info in target
    SK_MEMBER(formula, DynamicString),
    SK_MEMBER(from, DynamicString),
    SK_MEMBER(lval, DynamicString),
    SK_MEMBER_PROPERTY(mirror, Boolean),
    SK_MEMBER(repeat, Float),
    SK_MEMBER_PROPERTY(reset, Boolean),
    SK_MEMBER_PROPERTY(step, Int),
    SK_MEMBER(target, DynamicString),
    SK_MEMBER(to, DynamicString),
    SK_MEMBER_PROPERTY(values, DynamicString)
};

#endif

DEFINE_GET_MEMBER(SkAnimateBase);

SkAnimateBase::SkAnimateBase() : begin(0), dur(1), repeat(SK_Scalar1),
        fApply(NULL), fFieldInfo(NULL), fFieldOffset(0), fStart((SkMSec) -1), fTarget(NULL),
        fChanged(0), fDelayed(0), fDynamic(0), fHasEndEvent(0), fHasValues(0),
        fMirror(0), fReset(0), fResetPending(0), fTargetIsScope(0) {
    blend.setCount(1);
    blend[0] = SK_Scalar1;
}

SkAnimateBase::~SkAnimateBase() {
    SkDisplayTypes type = fValues.getType();
    if (type == SkType_String || type == SkType_DynamicString) {
        SkASSERT(fValues.count() == 1);
        delete fValues[0].fString;
    }
}

int SkAnimateBase::components() {
    return 1;
}

SkDisplayable* SkAnimateBase::deepCopy(SkAnimateMaker* maker) {
    SkAnimateBase* result = (SkAnimateBase*) INHERITED::deepCopy(maker);
    result->fApply = fApply;
    result->fFieldInfo =fFieldInfo;
    result->fHasValues = false;
    return result;
}

void SkAnimateBase::dirty() {
    fChanged = true;
}

#ifdef SK_DUMP_ENABLED
void SkAnimateBase::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    if (target.size() > 0)
        SkDebugf("target=\"%s\" ", target.c_str());
    else if (fTarget && strcmp(fTarget->id, ""))
        SkDebugf("target=\"%s\" ", fTarget->id);
    if (lval.size() > 0)
        SkDebugf("lval=\"%s\" ", lval.c_str());
    if (field.size() > 0)
        SkDebugf("field=\"%s\" ", field.c_str());
    else if (fFieldInfo)
        SkDebugf("field=\"%s\" ", fFieldInfo->fName);
    if (formula.size() > 0)
        SkDebugf("formula=\"%s\" ", formula.c_str());
    else {
        if (from.size() > 0)
            SkDebugf("from=\"%s\" ", from.c_str());
        SkDebugf("to=\"%s\" ", to.c_str());
    }
    if (begin != 0) {
        SkDebugf("begin=\"%g\" ", begin * 0.001);
    }
}
#endif

SkDisplayable* SkAnimateBase::getParent() const {
    return (SkDisplayable*) fApply;
}

bool SkAnimateBase::getProperty(int index, SkScriptValue* value) const {
    int boolResult;
    switch (index) {
        case SK_PROPERTY(dynamic):
            boolResult = fDynamic;
            goto returnBool;
        case SK_PROPERTY(mirror):
            boolResult = fMirror;
            goto returnBool;
        case SK_PROPERTY(reset):
            boolResult = fReset;
returnBool:
            value->fOperand.fS32 = SkToBool(boolResult);
            value->fType = SkType_Boolean;
            break;
        case SK_PROPERTY(step):
            if (fApply == NULL)
                return false;    // !!! notify there's an error?
            fApply->getStep(value);
            break;
        case SK_PROPERTY(values):
            value->fOperand.fString = (SkString*) &to;
            value->fType = SkType_String;
            break;
        default:
            SkASSERT(0);
            return false;
    }
    return true;
}

bool SkAnimateBase::hasExecute() const
{
    return false;
}

void SkAnimateBase::onEndElement(SkAnimateMaker& maker) {
    fChanged = false;
    setTarget(maker);
    if (field.size()) {
        SkASSERT(fTarget);
        fFieldInfo = fTarget->getMember(field.c_str());
        field.reset();
    }
    if (lval.size()) {
        // lval must be of the form x[y]
        const char* lvalStr = lval.c_str();
        const char* arrayEnd = strchr(lvalStr, '[');
        if (arrayEnd == NULL)
            return; //should this return an error?
        size_t arrayNameLen = arrayEnd - lvalStr;
        SkString arrayStr(lvalStr, arrayNameLen);
        SkASSERT(fTarget);  //this return an error?
        fFieldInfo = fTarget->getMember(arrayStr.c_str());
        SkString scriptStr(arrayEnd + 1, lval.size() - arrayNameLen - 2);
        SkAnimatorScript::EvaluateInt(maker, this, scriptStr.c_str(), &fFieldOffset);
    }
}

void SkAnimateBase::packARGB(SkScalar array[], int count, SkTDOperandArray* converted)
{
    SkASSERT(count == 4);
    converted->setCount(1);
    SkColor color = SkColorSetARGB(SkScalarRoundToInt(array[0]),
                                   SkScalarRoundToInt(array[1]),
                                   SkScalarRoundToInt(array[2]),
                                   SkScalarRoundToInt(array[3]));
    (*converted)[0].fS32 = color;
}



void SkAnimateBase::refresh(SkAnimateMaker& ) {
}

bool SkAnimateBase::setParent(SkDisplayable* apply) {
    SkASSERT(apply->isApply());
    fApply = (SkApply*) apply;
    return false;
}

bool SkAnimateBase::setProperty(int index, SkScriptValue& value) {
    bool boolValue = SkToBool(value.fOperand.fS32);
    switch (index) {
        case SK_PROPERTY(dynamic):
            fDynamic = boolValue;
            goto checkForBool;
        case SK_PROPERTY(values):
            fHasValues = true;
            SkASSERT(value.fType == SkType_String);
            to = *value.fOperand.fString;
            break;
        case SK_PROPERTY(mirror):
            fMirror = boolValue;
            goto checkForBool;
        case SK_PROPERTY(reset):
            fReset = boolValue;
checkForBool:
            SkASSERT(value.fType == SkType_Boolean);
            break;
        default:
            return false;
    }
    return true;
}

void SkAnimateBase::setTarget(SkAnimateMaker& maker) {
    if (target.size()) {
        SkAnimatorScript engine(maker, this, SkType_Displayable);
        const char* script = target.c_str();
        SkScriptValue scriptValue;
        bool success = engine.evaluateScript(&script, &scriptValue);
        if (success && scriptValue.fType == SkType_Displayable)
            fTarget = scriptValue.fOperand.fDrawable;
        else if (maker.find(target.c_str(), (SkDisplayable**) &fTarget) == false) {
            if (fApply->getMode() == SkApply::kMode_create)
                return; // may not be an error
            if (engine.getError() != SkScriptEngine::kNoError)
                maker.setScriptError(engine);
            else {
                maker.setErrorNoun(target);
                maker.setErrorCode(SkDisplayXMLParserError::kTargetIDNotFound);
            }
            return;
        }
        if (fApply && fApply->getMode() != SkApply::kMode_create)
            target.reset();
    }
}

bool SkAnimateBase::targetNeedsInitialization() const {
    return false;
}
