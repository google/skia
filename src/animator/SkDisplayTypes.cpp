
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayTypes.h"
#include "SkAnimateBase.h"

bool SkDisplayDepend::canContainDependents() const {
    return true;
}

void SkDisplayDepend::dirty() {
    SkDisplayable** last = fDependents.end();
    for (SkDisplayable** depPtr = fDependents.begin(); depPtr < last; depPtr++) {
        SkAnimateBase* animate = (SkAnimateBase* ) *depPtr;
        animate->setChanged(true);
    }
}

// Boolean
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayBoolean::fInfo[] = {
    SK_MEMBER(value, Boolean)
};

#endif

DEFINE_GET_MEMBER(SkDisplayBoolean);

SkDisplayBoolean::SkDisplayBoolean() : value(false) {
}

#ifdef SK_DUMP_ENABLED
void SkDisplayBoolean::dump(SkAnimateMaker* maker){
    dumpBase(maker);
    SkDebugf("value=\"%s\" />\n", value ? "true" : "false");
}
#endif

// int32_t
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayInt::fInfo[] = {
    SK_MEMBER(value, Int)
};

#endif

DEFINE_GET_MEMBER(SkDisplayInt);

SkDisplayInt::SkDisplayInt() : value(0) {
}

#ifdef SK_DUMP_ENABLED
void SkDisplayInt::dump(SkAnimateMaker* maker){
    dumpBase(maker);
    SkDebugf("value=\"%d\" />\n", value);
}
#endif

// SkScalar
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayFloat::fInfo[] = {
    SK_MEMBER(value, Float)
};

#endif

DEFINE_GET_MEMBER(SkDisplayFloat);

SkDisplayFloat::SkDisplayFloat() : value(0) {
}

#ifdef SK_DUMP_ENABLED
void SkDisplayFloat::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("value=\"%g\" />\n", SkScalarToFloat(value));
}
#endif

// SkString
enum SkDisplayString_Functions {
    SK_FUNCTION(slice)
};

enum SkDisplayString_Properties {
    SK_PROPERTY(length)
};

const SkFunctionParamType SkDisplayString::fFunctionParameters[] = {
    (SkFunctionParamType) SkType_Int,   // slice
    (SkFunctionParamType) SkType_Int,
    (SkFunctionParamType) 0
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayString::fInfo[] = {
    SK_MEMBER_PROPERTY(length, Int),
    SK_MEMBER_FUNCTION(slice, String),
    SK_MEMBER(value, String)
};

#endif

DEFINE_GET_MEMBER(SkDisplayString);

SkDisplayString::SkDisplayString() {
}

SkDisplayString::SkDisplayString(SkString& copyFrom) : value(copyFrom) {
}

void SkDisplayString::executeFunction(SkDisplayable* target, int index,
        SkTDArray<SkScriptValue>& parameters, SkDisplayTypes type,
        SkScriptValue* scriptValue) {
    if (scriptValue == nullptr)
        return;
    SkASSERT(target == this);
    switch (index) {
        case SK_FUNCTION(slice):
            scriptValue->fType = SkType_String;
            SkASSERT(parameters[0].fType == SkType_Int);
            int start =  parameters[0].fOperand.fS32;
            if (start < 0)
                start = (int) (value.size() - start);
            int end = (int) value.size();
            if (parameters.count() > 1) {
                SkASSERT(parameters[1].fType == SkType_Int);
                end = parameters[1].fOperand.fS32;
            }
            //if (end >= 0 && end < (int) value.size())
            if (end >= 0 && end <= (int) value.size())
                scriptValue->fOperand.fString = new SkString(&value.c_str()[start], end - start);
            else
                scriptValue->fOperand.fString = new SkString(value);
        break;
    }
}

const SkFunctionParamType* SkDisplayString::getFunctionsParameters() {
    return fFunctionParameters;
}

bool SkDisplayString::getProperty(int index, SkScriptValue* scriptValue) const {
    switch (index) {
        case SK_PROPERTY(length):
            scriptValue->fType = SkType_Int;
            scriptValue->fOperand.fS32 = (int32_t) value.size();
            break;
        default:
            SkASSERT(0);
            return false;
    }
    return true;
}


// SkArray
#if 0   // !!! reason enough to qualify enum with class name or move typedArray into its own file
enum SkDisplayArray_Properties {
    SK_PROPERTY(length)
};
#endif

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayArray::fInfo[] = {
    SK_MEMBER_PROPERTY(length, Int),
    SK_MEMBER_ARRAY(values, Unknown)
};

#endif

DEFINE_GET_MEMBER(SkDisplayArray);

SkDisplayArray::SkDisplayArray() {
}

SkDisplayArray::SkDisplayArray(SkTypedArray& copyFrom) : values(copyFrom) {

}

SkDisplayArray::~SkDisplayArray() {
    if (values.getType() == SkType_String) {
        for (int index = 0; index < values.count(); index++)
            delete values[index].fString;
        return;
    }
    if (values.getType() == SkType_Array) {
        for (int index = 0; index < values.count(); index++)
            delete values[index].fArray;
    }
}

bool SkDisplayArray::getProperty(int index, SkScriptValue* value) const {
    switch (index) {
        case SK_PROPERTY(length):
            value->fType = SkType_Int;
            value->fOperand.fS32 = values.count();
            break;
        default:
            SkASSERT(0);
            return false;
    }
    return true;
}
