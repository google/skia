
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAnimatorScript.h"
#include "SkAnimateBase.h"
#include "SkAnimateMaker.h"
#include "SkDisplayTypes.h"
#include "SkExtras.h"
#include "SkMemberInfo.h"
#include "SkParse.h"

static const SkDisplayEnumMap gEnumMaps[] = {
    { SkType_AddMode, "indirect|immediate" },
    { SkType_Align, "left|center|right" },
    { SkType_ApplyMode, "create|immediate|once" },
    { SkType_ApplyTransition, "normal|reverse" },
    { SkType_BitmapEncoding, "jpeg|png" },
    { SkType_BitmapFormat, "none|A1|A8|Index8|RGB16|RGB32" },
    { SkType_Boolean, "false|true" },
    { SkType_Cap, "butt|round|square" },
    { SkType_EventCode, "none|leftSoftKey|rightSoftKey|home|back|send|end|key0|key1|key2|key3|key4|key5|key6|key7|key8|key9|star|hash|up|down|left|right|OK|volUp|volDown|camera" },
    { SkType_EventKind, "none|keyChar|keyPress|keyPressUp|mouseDown|mouseDrag|mouseMove|mouseUp|onEnd|onLoad|user" },
    { SkType_EventMode, "deferred|immediate" },
    { SkType_FillType, "winding|evenOdd" },
    { SkType_FilterType, "none|bilinear" },
    { SkType_FontStyle, "normal|bold|italic|boldItalic" },
    { SkType_FromPathMode, "normal|angle|position" },
    { SkType_Join, "miter|round|blunt" },
    { SkType_MaskFilterBlurStyle, "normal|solid|outer|inner" },
    { SkType_PathDirection, "cw|ccw" },
    { SkType_Style, "fill|stroke|strokeAndFill" },
    { SkType_TextBoxAlign, "start|center|end" },
    { SkType_TextBoxMode, "oneLine|lineBreak" },
    { SkType_TileMode, "clamp|repeat|mirror" },
    { SkType_Xfermode, "clear|src|dst|srcOver|dstOver|srcIn|dstIn|srcOut|dstOut|"
        "srcATop|dstATop|xor|darken|lighten" },
};

static int gEnumMapCount = SK_ARRAY_COUNT(gEnumMaps);

SkAnimatorScript::SkAnimatorScript(SkAnimateMaker& maker, SkDisplayable* working, SkDisplayTypes type)
    : SkScriptEngine(SkScriptEngine::ToOpType(type)), fMaker(maker), fParent(NULL), fWorking(working)
{
    memberCallBack(EvalMember, (void*) this);
    memberFunctionCallBack(EvalMemberFunction, (void*) this);
    boxCallBack(Box, (void*) this);
    unboxCallBack(Unbox, (void*) &maker);
    propertyCallBack(EvalID, (void*) this); // must be first (entries are prepended, will be last), since it never fails
    propertyCallBack(Infinity, (void*) this);
    propertyCallBack(NaN, (void*) this);
    functionCallBack(Eval, (void*) this);
    functionCallBack(IsFinite, (void*) this);
    functionCallBack(IsNaN, (void*) this);
    if (type == SkType_ARGB) {
        functionCallBack(EvalRGB, (void*) this);
        propertyCallBack(EvalNamedColor, (void*) &maker.fIDs);
    }
    if (SkDisplayType::IsEnum(&maker, type)) {
        // !!! for SpiderMonkey, iterate through the enum values, and map them to globals
        const SkDisplayEnumMap& map = GetEnumValues(type);
        propertyCallBack(EvalEnum, (void*) map.fValues);
    }
    for (SkExtras** extraPtr = maker.fExtras.begin(); extraPtr < maker.fExtras.end(); extraPtr++) {
        SkExtras* extra = *extraPtr;
        if (extra->fExtraCallBack)
            propertyCallBack(extra->fExtraCallBack, extra->fExtraStorage);
    }
}

SkAnimatorScript::~SkAnimatorScript() {
    for (SkDisplayable** dispPtr = fTrackDisplayable.begin(); dispPtr < fTrackDisplayable.end(); dispPtr++)
        delete *dispPtr;
}

bool SkAnimatorScript::evaluate(const char* original, SkScriptValue* result, SkDisplayTypes type) {
        const char* script = original;
        bool success = evaluateScript(&script, result);
        if (success == false || result->fType != type) {
            fMaker.setScriptError(*this);
            return false;
        }
        return true;
}

bool SkAnimatorScript::Box(void* user, SkScriptValue* scriptValue) {
    SkAnimatorScript* engine = (SkAnimatorScript*) user;
    SkDisplayTypes type = scriptValue->fType;
    SkDisplayable* displayable;
    switch (type) {
        case SkType_Array: {
            SkDisplayArray* boxedValue = new SkDisplayArray(*scriptValue->fOperand.fArray);
            displayable = boxedValue;
            } break;
        case SkType_Boolean: {
            SkDisplayBoolean* boxedValue = new SkDisplayBoolean;
            displayable = boxedValue;
            boxedValue->value = !! scriptValue->fOperand.fS32;
            } break;
        case SkType_Int: {
            SkDisplayInt* boxedValue = new SkDisplayInt;
            displayable = boxedValue;
            boxedValue->value = scriptValue->fOperand.fS32;
            } break;
        case SkType_Float: {
            SkDisplayFloat* boxedValue = new SkDisplayFloat;
            displayable = boxedValue;
            boxedValue->value = scriptValue->fOperand.fScalar;
            } break;
        case SkType_String: {
            SkDisplayString* boxedValue = new SkDisplayString(*scriptValue->fOperand.fString);
            displayable = boxedValue;
            } break;
        case SkType_Displayable:
            scriptValue->fOperand.fObject = scriptValue->fOperand.fDisplayable;
            scriptValue->fType = SkType_Displayable;
            return true;
        default:
            SkASSERT(0);
            return false;
    }
    engine->track(displayable);
    scriptValue->fOperand.fObject = displayable;
    scriptValue->fType = SkType_Displayable;
    return true;
}

bool SkAnimatorScript::Eval(const char* function, size_t len, SkTDArray<SkScriptValue>& params,
        void* eng, SkScriptValue* value) {
    if (SK_LITERAL_STR_EQUAL("eval", function, len) == false)
        return false;
    if (params.count() != 1)
        return false;
    SkAnimatorScript* host = (SkAnimatorScript*) eng;
    SkAnimatorScript engine(host->fMaker, host->fWorking, SkScriptEngine::ToDisplayType(host->fReturnType));
    SkScriptValue* scriptValue = params.begin();
    bool success = true;
    if (scriptValue->fType == SkType_String) {
        const char* script = scriptValue->fOperand.fString->c_str();
        success = engine.evaluateScript(&script, value);
    } else
        *value = *scriptValue;
    return success;
}

bool SkAnimatorScript::EvalEnum(const char* token, size_t len, void* callBack, SkScriptValue* value) {
    const char* tokens = (const char*) callBack;
    value->fType = SkType_Int;
    if (MapEnums(tokens, token, len, (int*)&value->fOperand.fS32))
        return true;
    return false;
}

bool SkAnimatorScript::EvalID(const char* token, size_t len, void* user, SkScriptValue* value) {
    SkAnimatorScript* engine = (SkAnimatorScript*) user;
    SkTDict<SkDisplayable*>* ids = &engine->fMaker.fIDs;
    SkDisplayable* displayable;
    bool success = ids->find(token, len, &displayable);
    if (success == false) {
        displayable = engine->fWorking;
        if (SK_LITERAL_STR_EQUAL("parent", token, len)) {
            SkDisplayable* parent = displayable->getParent();
            if (parent == NULL)
                parent = engine->fParent;
            if (parent) {
                value->fOperand.fDisplayable = parent;
                value->fType = SkType_Displayable;
                return true;
            }
        }
        if (displayable && EvalMember(token, len, displayable, engine, value))
            return true;
        value->fOperand.fString = NULL;
        value->fType = SkType_String;
    } else {
        SkDisplayable* working = engine->fWorking;
        value->fOperand.fDisplayable = displayable;
        value->fType = SkType_Displayable;
        if (displayable->canContainDependents() && working && working->isAnimate()) {
            SkAnimateBase* animator = (SkAnimateBase*) working;
            if (animator->isDynamic()) {
                SkDisplayDepend* depend = (SkDisplayDepend* ) displayable;
                depend->addDependent(working);
            }
        }
    }
    return true;
}

bool SkAnimatorScript::EvalNamedColor(const char* token, size_t len, void* callback, SkScriptValue* value) {
        value->fType = SkType_Int;
    if (SkParse::FindNamedColor(token, len, (SkColor*) &value->fOperand.fS32) != NULL)
        return true;
    return false;
}

bool SkAnimatorScript::EvalRGB(const char* function, size_t len, SkTDArray<SkScriptValue>& params,
        void* eng, SkScriptValue* value) {
    if (SK_LITERAL_STR_EQUAL("rgb", function, len) == false)
        return false;
    if (params.count() != 3)
        return false;
    SkScriptEngine* engine = (SkScriptEngine*) eng;
    unsigned result = 0xFF000000;
    int shift = 16;
    for (SkScriptValue* valuePtr = params.begin(); valuePtr < params.end(); valuePtr++) {
        engine->convertTo(SkType_Int, valuePtr);
        result |= SkClampMax(valuePtr->fOperand.fS32, 255) << shift;
        shift -= 8;
    }
    value->fOperand.fS32 = result;
    value->fType = SkType_Int;
    return true;
}

bool SkAnimatorScript::EvalMemberCommon(SkScriptEngine* engine, const SkMemberInfo* info,
        SkDisplayable* displayable, SkScriptValue* value) {
    SkDisplayTypes original;
    SkDisplayTypes type = original = (SkDisplayTypes) info->getType();
    if (info->fType == SkType_Array)
        type = SkType_Array;
    switch (type) {
        case SkType_ARGB:
            type = SkType_Int;
        case SkType_Boolean:
        case SkType_Int:
        case SkType_MSec:
        case SkType_Float:
            SkASSERT(info->getCount() == 1);
            if (info->fType != SkType_MemberProperty && info->fType != SkType_MemberFunction)
                value->fOperand.fS32 = *(int32_t*) info->memberData(displayable);   // OK for SkScalar too
            if (type == SkType_MSec) {
                value->fOperand.fScalar = SkScalarDiv((SkScalar) value->fOperand.fS32, 1000); // dividing two ints is the same as dividing two scalars
                type = SkType_Float;
            }
            break;
        case SkType_String: {
            SkString* displayableString;
            if (info->fType != SkType_MemberProperty && info->fType != SkType_MemberFunction) {
                info->getString(displayable, &displayableString);
                value->fOperand.fString = new SkString(*displayableString);
            }
            } break;
        case SkType_Array: {
            SkASSERT(info->fType != SkType_MemberProperty); // !!! incomplete
            SkTDOperandArray* displayableArray = (SkTDOperandArray*) info->memberData(displayable);
            if (displayable->getType() == SkType_Array) {
                SkDisplayArray* typedArray = (SkDisplayArray*) displayable;
                original = typedArray->values.getType();
            }
            SkASSERT(original != SkType_Unknown);
            SkTypedArray* array = value->fOperand.fArray = new SkTypedArray(original);
            engine->track(array);
            int count = displayableArray->count();
            if (count > 0) {
                array->setCount(count);
                memcpy(array->begin(), displayableArray->begin(), count * sizeof(SkOperand));
            }
            } break;
        default:
            SkASSERT(0); // unimplemented
    }
    value->fType = type;
    return true;
}

bool SkAnimatorScript::EvalMember(const char* member, size_t len, void* object, void* eng,
        SkScriptValue* value) {
    SkScriptEngine* engine = (SkScriptEngine*) eng;
    SkDisplayable* displayable = (SkDisplayable*) object;
    SkString name(member, len);
    SkDisplayable* named = displayable->contains(name);
    if (named) {
        value->fOperand.fDisplayable = named;
        value->fType = SkType_Displayable;
        return true;
    }
    const SkMemberInfo* info = displayable->getMember(name.c_str());
    if (info == NULL)
        return false;
    if (info->fType == SkType_MemberProperty) {
        if (displayable->getProperty(info->propertyIndex(), value) == false) {
            SkASSERT(0);
            return false;
        }
    }
    return EvalMemberCommon(engine, info, displayable, value);
}

bool SkAnimatorScript::EvalMemberFunction(const char* member, size_t len, void* object,
        SkTDArray<SkScriptValue>& params, void* eng, SkScriptValue* value) {
    SkScriptEngine* engine = (SkScriptEngine*) eng;
    SkDisplayable* displayable = (SkDisplayable*) object;
    SkString name(member, len);
    const SkMemberInfo* info = displayable->getMember(name.c_str());
    SkASSERT(info != NULL); /* !!! error handling unimplemented */
    if (info->fType != SkType_MemberFunction) {
        SkASSERT(0);
        return false;
    }
    displayable->executeFunction(displayable, info->functionIndex(), params, info->getType(),
        value);
    return EvalMemberCommon(engine, info, displayable, value);
}

bool SkAnimatorScript::EvaluateDisplayable(SkAnimateMaker& maker, SkDisplayable* displayable, const char* script, SkDisplayable** result) {
    SkAnimatorScript engine(maker, displayable, SkType_Displayable);
    SkScriptValue value;
    bool success = engine.evaluate(script, &value, SkType_Displayable);
    if (success)
        *result = value.fOperand.fDisplayable;
    return success;
}

bool SkAnimatorScript::EvaluateInt(SkAnimateMaker& maker, SkDisplayable* displayable, const char* script, int32_t* result) {
    SkAnimatorScript engine(maker, displayable, SkType_Int);
    SkScriptValue value;
    bool success = engine.evaluate(script, &value, SkType_Int);
    if (success)
        *result = value.fOperand.fS32;
    return success;
}

bool SkAnimatorScript::EvaluateFloat(SkAnimateMaker& maker, SkDisplayable* displayable, const char* script, SkScalar* result) {
    SkAnimatorScript engine(maker, displayable, SkType_Float);
    SkScriptValue value;
    bool success = engine.evaluate(script, &value, SkType_Float);
    if (success)
        *result = value.fOperand.fScalar;
    return success;
}

bool SkAnimatorScript::EvaluateString(SkAnimateMaker& maker, SkDisplayable* displayable, const char* script, SkString* result) {
    SkAnimatorScript engine(maker, displayable, SkType_String);
    SkScriptValue value;
    bool success = engine.evaluate(script, &value, SkType_String);
    if (success)
        result->set(*(value.fOperand.fString));
  return success;
}

bool SkAnimatorScript::EvaluateString(SkAnimateMaker& maker, SkDisplayable* displayable, SkDisplayable* parent, const char* script, SkString* result) {
    SkAnimatorScript engine(maker, displayable, SkType_String);
    engine.fParent = parent;
    SkScriptValue value;
    bool success = engine.evaluate(script, &value, SkType_String);
    if (success)
        result->set(*(value.fOperand.fString));
  return success;
}

const SkDisplayEnumMap& SkAnimatorScript::GetEnumValues(SkDisplayTypes type) {
    int index = SkTSearch<SkDisplayTypes>(&gEnumMaps[0].fType, gEnumMapCount, type,
        sizeof(SkDisplayEnumMap));
    SkASSERT(index >= 0);
    return gEnumMaps[index];
}

bool SkAnimatorScript::Infinity(const char* token, size_t len, void* user, SkScriptValue* value) {
    if (SK_LITERAL_STR_EQUAL("Infinity", token, len) == false)
        return false;
    value->fType = SkType_Float;
    value->fOperand.fScalar = SK_ScalarInfinity;
    return true;
}

bool SkAnimatorScript::IsFinite(const char* function, size_t len, SkTDArray<SkScriptValue>& params,
        void* eng, SkScriptValue* value) {
    if (SK_LITERAL_STR_EQUAL(function, "isFinite", len) == false)
        return false;
    if (params.count() != 1)
        return false;
    SkScriptValue* scriptValue = params.begin();
    SkDisplayTypes type = scriptValue->fType;
    SkScalar scalar = scriptValue->fOperand.fScalar;
    value->fType = SkType_Int;
    value->fOperand.fS32 = type == SkType_Float ? SkScalarIsNaN(scalar) == false &&
        SkScalarAbs(scalar) != SK_ScalarInfinity    : type == SkType_Int;
    return true;
}

bool SkAnimatorScript::IsNaN(const char* function, size_t len, SkTDArray<SkScriptValue>& params,
        void* eng, SkScriptValue* value) {
    if (SK_LITERAL_STR_EQUAL("isNaN", function, len) == false)
        return false;
    if (params.count() != 1)
        return false;
    SkScriptValue* scriptValue = params.begin();
    value->fType = SkType_Int;
    value->fOperand.fS32 = scriptValue->fType == SkType_Float ? SkScalarIsNaN(scriptValue->fOperand.fScalar) : 0;
    return true;
}

bool SkAnimatorScript::MapEnums(const char* ptr, const char* match, size_t len, int* value) {
    int index = 0;
    bool more = true;
    do {
        const char* last = strchr(ptr, '|');
        if (last == NULL) {
            last = &ptr[strlen(ptr)];
            more = false;
        }
        size_t length = last - ptr;
        if (len == length && strncmp(ptr, match, length) == 0) {
            *value = index;
            return true;
        }
        index++;
        ptr = last + 1;
    } while (more);
    return false;
}

bool SkAnimatorScript::NaN(const char* token, size_t len, void* user, SkScriptValue* value) {
    if (SK_LITERAL_STR_EQUAL("NaN", token, len) == false)
        return false;
    value->fType = SkType_Float;
    value->fOperand.fScalar = SK_ScalarNaN;
    return true;
}

#if 0
bool SkAnimatorScript::ObjectToString(void* object, void* user, SkScriptValue* value) {
    SkTDict<SkDisplayable*>* ids = (SkTDict<SkDisplayable*>*) user;
    SkDisplayable* displayable = (SkDisplayable*) object;
    const char* key;
    bool success = ids->findKey(displayable, &key);
    if (success == false)
        return false;
    value->fOperand.fString =   new SkString(key);
    value->fType = SkType_String;
    return true;
}
#endif

bool SkAnimatorScript::Unbox(void* m, SkScriptValue* scriptValue) {
    SkAnimateMaker* maker = (SkAnimateMaker*) m;
    SkASSERT((unsigned) scriptValue->fType == (unsigned) SkType_Displayable);
    SkDisplayable* displayable = (SkDisplayable*) scriptValue->fOperand.fObject;
    SkDisplayTypes type = displayable->getType();
    switch (displayable->getType()) {
        case SkType_Array: {
            SkDisplayArray* boxedValue = (SkDisplayArray*) displayable;
            scriptValue->fOperand.fArray = &boxedValue->values;
            } break;
        case SkType_Boolean: {
            SkDisplayBoolean* boxedValue = (SkDisplayBoolean*) displayable;
            scriptValue->fOperand.fS32 = boxedValue->value;
            } break;
        case SkType_Int: {
            SkDisplayInt* boxedValue = (SkDisplayInt*) displayable;
            scriptValue->fOperand.fS32 = boxedValue->value;
            } break;
        case SkType_Float: {
            SkDisplayFloat* boxedValue = (SkDisplayFloat*) displayable;
            scriptValue->fOperand.fScalar = boxedValue->value;
            } break;
        case SkType_String: {
            SkDisplayString* boxedValue = (SkDisplayString*) displayable;
            scriptValue->fOperand.fString = SkNEW_ARGS(SkString, (boxedValue->value));
            } break;
        default: {
            const char* id = NULL;
            SkDEBUGCODE(bool success = ) maker->findKey(displayable, &id);
            SkASSERT(success);
            scriptValue->fOperand.fString = SkNEW_ARGS(SkString, (id));
            type = SkType_String;
        }
    }
    scriptValue->fType = type;
    return true;
}

#if defined SK_SUPPORT_UNITTEST

#include "SkAnimator.h"

static const char scriptTestSetup[]  =
"<screenplay>\n"
    "<text id='label' text='defg'/>\n"
    "<add id='addLabel' use='label'/>\n"
    "<text id='text1' text='test'/>\n"
    "<apply scope='addLabel'>\n"
        "<set target='label' field='text' to='#script:text1.text'/>\n"
    "</apply>\n"
    "<apply>\n"
        "<paint id='labelPaint'>\n"
            "<emboss id='emboss' direction='[1,1,1]'  />\n"
        "</paint>\n"
        "<animate id='animation' field='direction' target='emboss' from='[1,1,1]' to='[-1,1,1]' dur='1'/>\n"
        "<set lval='direction[0]' target='emboss' to='-1' />\n"
    "</apply>\n"
    "<color id='testColor' color='0 ? rgb(0,0,0) : rgb(255,255,255)' />\n"
    "<color id='xColor' color='rgb(12,34,56)' />\n"
    "<array id='emptyArray' />\n"
    "<array id='intArray' values='[1, 4, 6]' />\n"
    "<int id='idx' value='2' />\n"
    "<int id='idy' value='2' />\n"
    "<string id='alpha' value='abc' />\n"
    "<rect id='testRect' left='Math.cos(0)' top='2' right='12' bottom='5' />\n"
    "<event id='evt'>\n"
        "<input name='x' />\n"
        "<apply scope='idy'>\n"
            "<set field='value' to='evt.x.int' />\n"
        "</apply>\n"
    "</event>\n"
"</screenplay>";

#define DEFAULT_ANSWER   , 0

static const SkScriptNAnswer scriptTests[]  = {
    { "label.text.length == 4", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
//  { "labelPaint.measureText(label.text) > 0 ? labelPaint.measureText(label.text)+10 : 40", SkType_Float, 0, SkIntToScalar(0x23)  },
    {   "Number.POSITIVE_INFINITY >= Number.MAX_VALUE ? 1 : 0", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "Infinity >= Number.MAX_VALUE ? 1 : 0", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "Number.NEGATIVE_INFINITY <= -Number.MAX_VALUE ? 1 : 0", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "Number.MIN_VALUE > 0 ? 1 : 0", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "isNaN(Number.NaN)", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "isNaN(NaN)", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "Math.sin(0)", SkType_Float, 0, SkIntToScalar(0) DEFAULT_ANSWER },
    {   "alpha+alpha", SkType_String, 0, 0, "abcabc" },
    {   "intArray[4]", SkType_Unknown DEFAULT_ANSWER DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "emptyArray[4]", SkType_Unknown DEFAULT_ANSWER DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "idx", SkType_Int, 2 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "intArray.length", SkType_Int, 3 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "intArray.values[0]", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "intArray[0]", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "idx.value", SkType_Int, 2 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "alpha.value", SkType_String, 0, 0, "abc" },
    {   "alpha", SkType_String, 0, 0, "abc" },
    {   "alpha.value+alpha.value", SkType_String, 0, 0, "abcabc" },
    {   "alpha+idx", SkType_String, 0, 0, "abc2" },
    {   "idx+alpha", SkType_String, 0, 0, "2abc" },
    {   "intArray[idx]", SkType_Int, 6 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "alpha.slice(1,2)", SkType_String, 0, 0, "b" },
    {   "alpha.value.slice(1,2)", SkType_String, 0, 0, "b" },
    {   "testRect.left+2", SkType_Float, 0, SkIntToScalar(3) DEFAULT_ANSWER },
    {   "0 ? Math.sin(0) : 1", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "0 ? intArray[0] : 1", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "0 ? intArray.values[0] : 1", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "0 ? idx : 1", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "0 ? idx.value : 1", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "0 ? alpha.slice(1,2) : 1", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    {   "0 ? alpha.value.slice(1,2) : 1", SkType_Int, 1 DEFAULT_ANSWER DEFAULT_ANSWER },
    { "idy", SkType_Int, 3 DEFAULT_ANSWER DEFAULT_ANSWER }
};

#define SkScriptNAnswer_testCount   SK_ARRAY_COUNT(scriptTests)

void SkAnimatorScript::UnitTest() {
#if defined(SK_SUPPORT_UNITTEST)
    SkAnimator animator;
    SkASSERT(animator.decodeMemory(scriptTestSetup, sizeof(scriptTestSetup)-1));
    SkEvent evt;
    evt.setString("id", "evt");
    evt.setS32("x", 3);
    animator.doUserEvent(evt);
    // set up animator with memory script above, then run value tests
    for (unsigned index = 0; index < SkScriptNAnswer_testCount; index++) {
        SkAnimatorScript engine(*animator.fMaker, NULL, scriptTests[index].fType);
        SkScriptValue value;
        const char* script = scriptTests[index].fScript;
        bool success = engine.evaluateScript(&script, &value);
        if (success == false) {
            SkDebugf("script failed: %s\n", scriptTests[index].fScript);
            SkASSERT(scriptTests[index].fType == SkType_Unknown);
            continue;
        }
        SkASSERT(value.fType == scriptTests[index].fType);
        SkScalar error;
        switch (value.fType) {
            case SkType_Int:
                SkASSERT(value.fOperand.fS32 == scriptTests[index].fIntAnswer);
                break;
            case SkType_Float:
                error = SkScalarAbs(value.fOperand.fScalar - scriptTests[index].fScalarAnswer);
                SkASSERT(error < SK_Scalar1 / 10000);
                break;
            case SkType_String:
                SkASSERT(strcmp(value.fOperand.fString->c_str(), scriptTests[index].fStringAnswer) == 0);
                break;
            default:
                SkASSERT(0);
        }
    }
#endif
}

#endif
