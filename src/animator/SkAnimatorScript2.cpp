
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkAnimatorScript2.h"
#include "SkAnimateBase.h"
#include "SkAnimateMaker.h"
#include "SkDisplayTypes.h"
#include "SkExtras.h"
#include "SkMemberInfo.h"
#include "SkOpArray.h"
#include "SkParse.h"
#include "SkScript2.h"
#include "SkScriptCallBack.h"

static const SkDisplayEnumMap gEnumMaps[] = {
    { SkType_AddMode, "indirect|immediate" },
    { SkType_Align, "left|center|right" },
    { SkType_ApplyMode, "immediate|once" },
    { SkType_ApplyTransition, "reverse" },
    { SkType_BitmapEncoding, "jpeg|png" },
    { SkType_BitmapFormat, "none|A1|A8|Index8|RGB16|RGB32" },
    { SkType_Boolean, "false|true" },
    { SkType_Cap, "butt|round|square" },
    { SkType_EventCode, "none|up|down|left|right|back|end|OK|send|leftSoftKey|rightSoftKey|key0|key1|key2|key3|key4|key5|key6|key7|key8|key9|star|hash" },
    { SkType_EventKind, "none|keyChar|keyPress|mouseDown|mouseDrag|mouseMove|mouseUp|onEnd|onLoad|user" },
    { SkType_EventMode, "deferred|immediate" },
    { SkType_FillType, "winding|evenOdd" },
    { SkType_FilterType, "none|bilinear" },
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


class SkAnimatorScript_Box : public SkScriptCallBackConvert {
public:
    SkAnimatorScript_Box() {}

    ~SkAnimatorScript_Box() {
        for (SkDisplayable** dispPtr = fTrackDisplayable.begin(); dispPtr < fTrackDisplayable.end(); dispPtr++)
            delete *dispPtr;
    }

    virtual bool convert(SkOperand2::OpType type, SkOperand2* operand) {
        SkDisplayable* displayable;
        switch (type) {
            case SkOperand2::kArray: {
                SkDisplayArray* boxedValue = new SkDisplayArray(*operand->fArray);
                displayable = boxedValue;
                } break;
            case SkOperand2::kS32: {
                SkDisplayInt* boxedValue = new SkDisplayInt;
                displayable = boxedValue;
                boxedValue->value = operand->fS32;
                } break;
            case SkOperand2::kScalar: {
                SkDisplayFloat* boxedValue = new SkDisplayFloat;
                displayable = boxedValue;
                boxedValue->value = operand->fScalar;
                } break;
            case SkOperand2::kString: {
                SkDisplayString* boxedValue = new SkDisplayString(*operand->fString);
                displayable = boxedValue;
                } break;
            case SkOperand2::kObject:
                return true;
            default:
                SkASSERT(0);
                return false;
        }
        track(displayable);
        operand->fObject = (void*) displayable;
        return true;
    }

    virtual SkOperand2::OpType getReturnType(int index) {
        return SkOperand2::kObject;
    }

    virtual Type getType() const {
        return kBox;
    }

    void track(SkDisplayable* displayable) {
        SkASSERT(fTrackDisplayable.find(displayable) < 0);
        *fTrackDisplayable.append() = displayable;
    }

    SkTDDisplayableArray fTrackDisplayable;
};


class SkAnimatorScript_Enum : public SkScriptCallBackProperty {
public:
    SkAnimatorScript_Enum(const char* tokens) : fTokens(tokens) {}

    virtual bool getConstValue(const char* name, int len, SkOperand2* value) {
        return SkAnimatorScript2::MapEnums(fTokens, name, len, &value->fS32);
    }

private:
    const char* fTokens;
};

    // !!! if type is string, call invoke
    // if any other type, return original value
        // distinction is undone: could do this by returning index == 0 only if param is string
        // still, caller of getParamTypes will attempt to convert param to string (I guess)
class SkAnimatorScript_Eval : public SkScriptCallBackFunction {
public:
    SkAnimatorScript_Eval(SkAnimatorScript2* engine) : fEngine(engine) {}

    virtual bool getIndex(const char* name, int len, size_t* result) {
        if (SK_LITERAL_STR_EQUAL("eval", name, len) != 0)
            return false;
        *result = 0;
        return true;
    }

    virtual void getParamTypes(SkIntArray(SkOperand2::OpType)* types) {
        types->setCount(1);
        SkOperand2::OpType* type = types->begin();
        type[0] = SkOperand2::kString;
    }

    virtual bool invoke(size_t index, SkOpArray* params, SkOperand2* answer) {
        SkAnimatorScript2 engine(fEngine->getMaker(), fEngine->getWorking(),
            SkAnimatorScript2::ToDisplayType(fEngine->getReturnType()));
        SkOperand2* op = params->begin();
        const char* script = op->fString->c_str();
        SkScriptValue2 value;
        return engine.evaluateScript(&script, &value);
        SkASSERT(value.fType == fEngine->getReturnType());
        *answer = value.fOperand;
        // !!! incomplete ?
        return true;
    }

private:
    SkAnimatorScript2* fEngine;
};

class SkAnimatorScript_ID : public SkScriptCallBackProperty {
public:
    SkAnimatorScript_ID(SkAnimatorScript2* engine) : fEngine(engine) {}

    virtual bool getIndex(const char* token, int len, size_t* result) {
        SkDisplayable* displayable;
        bool success = fEngine->getMaker().find(token, len, &displayable);
        if (success == false) {
            *result = 0;
        } else {
            *result = (size_t) displayable;
            SkDisplayable* working = fEngine->getWorking();
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

    virtual bool getResult(size_t ref, SkOperand2* answer) {
        answer->fObject = (void*) ref;
        return true;
    }

    virtual SkOperand2::OpType getReturnType(size_t index) {
        return index == 0 ? SkOperand2::kString : SkOperand2::kObject;
    }

private:
    SkAnimatorScript2* fEngine;
};


class SkAnimatorScript_Member : public SkScriptCallBackMember {
public:

    SkAnimatorScript_Member(SkAnimatorScript2* engine) : fEngine(engine) {}

    bool getMemberReference(const char* member, size_t len, void* object, SkScriptValue2* ref) {
        SkDisplayable* displayable = (SkDisplayable*) object;
        SkString name(member, len);
        SkDisplayable* named = displayable->contains(name);
        if (named) {
            ref->fType = SkOperand2::kObject;
            ref->fOperand.fObject = named;
            return true;
        }
        const SkMemberInfo* info = displayable->getMember(name.c_str());
        if (info == NULL)
            return false;    // !!! add additional error info?
        ref->fType = SkAnimatorScript2::ToOpType(info->getType());
        ref->fOperand.fObject = (void*) info;
        return true;
    }

    bool invoke(size_t ref, void* object, SkOperand2* value) {
        const SkMemberInfo* info = (const SkMemberInfo* ) ref;
        SkDisplayable* displayable = (SkDisplayable*) object;
        if (info->fType == SkType_MemberProperty) {
            if (displayable->getProperty2(info->propertyIndex(), value) == false) {
                return false;
            }
        }
        return fEngine->evalMemberCommon(info, displayable, value);
    }

    SkAnimatorScript2* fEngine;
};


class SkAnimatorScript_MemberFunction : public SkScriptCallBackMemberFunction {
public:
    SkAnimatorScript_MemberFunction(SkAnimatorScript2* engine) : fEngine(engine) {}

    bool getMemberReference(const char* member, size_t len, void* object, SkScriptValue2* ref) {
        SkDisplayable* displayable = (SkDisplayable*) object;
        SkString name(member, len);
        const SkMemberInfo* info = displayable->getMember(name.c_str());
        if (info == NULL || info->fType != SkType_MemberFunction)
            return false;    // !!! add additional error info?
        ref->fType = SkAnimatorScript2::ToOpType(info->getType());
        ref->fOperand.fObject = (void*) info;
        return true;
    }

    virtual void getParamTypes(SkIntArray(SkOperand2::OpType)* types) {
        types->setCount(3);
        SkOperand2::OpType* type = types->begin();
        type[0] = type[1] = type[2] = SkOperand2::kS32;
    }

    bool invoke(size_t ref, void* object, SkOpArray* params, SkOperand2* value)
    {
        const SkMemberInfo* info = (const SkMemberInfo* ) ref;
        SkDisplayable* displayable = (SkDisplayable*) object;
        displayable->executeFunction2(displayable, info->functionIndex(), params, info->getType(),
            value);
        return fEngine->evalMemberCommon(info, displayable, value);
    }

    SkAnimatorScript2* fEngine;
};


class SkAnimatorScript_NamedColor : public SkScriptCallBackProperty {
public:
    virtual bool getConstValue(const char* name, int len, SkOperand2* value) {
        return SkParse::FindNamedColor(name, len, (SkColor*) &value->fS32) != NULL;
    }
};


class SkAnimatorScript_RGB : public SkScriptCallBackFunction {
public:
    virtual bool getIndex(const char* name, int len, size_t* result) {
        if (SK_LITERAL_STR_EQUAL("rgb", name, len) != 0)
            return false;
        *result = 0;
        return true;
    }

    virtual void getParamTypes(SkIntArray(SkOperand2::OpType)* types) {
        types->setCount(3);
        SkOperand2::OpType* type = types->begin();
        type[0] = type[1] = type[2] = SkOperand2::kS32;
    }

    virtual bool invoke(size_t index, SkOpArray* params, SkOperand2* answer) {
        SkASSERT(index == 0);
        unsigned result = 0xFF000000;
        int shift = 16;
        for (int index = 0; index < 3; index++) {
            result |= SkClampMax(params->begin()[index].fS32, 255) << shift;
            shift -= 8;
        }
        answer->fS32 = result;
        return true;
    }

};


class SkAnimatorScript_Unbox : public SkScriptCallBackConvert {
public:
    SkAnimatorScript_Unbox(SkAnimatorScript2* engine) : fEngine(engine) {}

    virtual bool convert(SkOperand2::OpType type, SkOperand2* operand) {
        SkASSERT(type == SkOperand2::kObject);
        SkDisplayable* displayable = (SkDisplayable*) operand->fObject;
        switch (displayable->getType()) {
            case SkType_Array: {
                SkDisplayArray* boxedValue = (SkDisplayArray*) displayable;
                operand->fArray = new SkOpArray(SkAnimatorScript2::ToOpType(boxedValue->values.getType()));
                int count = boxedValue->values.count();
                operand->fArray->setCount(count);
                memcpy(operand->fArray->begin(), boxedValue->values.begin(), count * sizeof(SkOperand2));
                fEngine->track(operand->fArray);
                } break;
            case SkType_Boolean: {
                SkDisplayBoolean* boxedValue = (SkDisplayBoolean*) displayable;
                operand->fS32 = boxedValue->value;
                } break;
            case SkType_Int: {
                SkDisplayInt* boxedValue = (SkDisplayInt*) displayable;
                operand->fS32 = boxedValue->value;
                } break;
            case SkType_Float: {
                SkDisplayFloat* boxedValue = (SkDisplayFloat*) displayable;
                operand->fScalar = boxedValue->value;
                } break;
            case SkType_String: {
                SkDisplayString* boxedValue = (SkDisplayString*) displayable;
                operand->fString = SkNEW_ARGS(SkString, (boxedValue->value));
                } break;
            default: {
                const char* id;
                bool success = fEngine->getMaker().findKey(displayable, &id);
                SkASSERT(success);
                operand->fString = SkNEW_ARGS(SkString, (id));
            }
        }
        return true;
    }

    virtual SkOperand2::OpType getReturnType(int /*index*/, SkOperand2* operand) {
        SkDisplayable* displayable = (SkDisplayable*) operand->fObject;
        switch (displayable->getType()) {
            case SkType_Array:
                return SkOperand2::kArray;
            case SkType_Int:
                return SkOperand2::kS32;
            case SkType_Float:
                return SkOperand2::kScalar;
            case SkType_String:
            default:
                return SkOperand2::kString;
        }
    }

    virtual Type getType() const {
        return kUnbox;
    }

    SkAnimatorScript2* fEngine;
};

SkAnimatorScript2::SkAnimatorScript2(SkAnimateMaker& maker, SkDisplayable* working, SkDisplayTypes type) :
        SkScriptEngine2(ToOpType(type)), fMaker(maker), fWorking(working) {
    *fCallBackArray.append() = new SkAnimatorScript_Member(this);
    *fCallBackArray.append() = new SkAnimatorScript_MemberFunction(this);
    *fCallBackArray.append() = new SkAnimatorScript_Box();
    *fCallBackArray.append() = new SkAnimatorScript_Unbox(this);
    *fCallBackArray.append() = new SkAnimatorScript_ID(this);
    if (type == SkType_ARGB) {
        *fCallBackArray.append() = new SkAnimatorScript_RGB();
        *fCallBackArray.append() = new SkAnimatorScript_NamedColor();
    }
    if (SkDisplayType::IsEnum(&maker, type)) {
        // !!! for SpiderMonkey, iterate through the enum values, and map them to globals
        const SkDisplayEnumMap& map = GetEnumValues(type);
        *fCallBackArray.append() = new SkAnimatorScript_Enum(map.fValues);
    }
    *fCallBackArray.append() = new SkAnimatorScript_Eval(this);
#if 0        // !!! no extra support for now
    for (SkExtras** extraPtr = maker.fExtras.begin(); extraPtr < maker.fExtras.end(); extraPtr++) {
        SkExtras* extra = *extraPtr;
        if (extra->fExtraCallBack)
            *fCallBackArray.append() = new propertyCallBack(extra->fExtraCallBack, extra->fExtraStorage);
    }
#endif
}

SkAnimatorScript2::~SkAnimatorScript2() {
    SkScriptCallBack** end = fCallBackArray.end();
    for (SkScriptCallBack** ptr = fCallBackArray.begin(); ptr < end; ptr++)
        delete *ptr;
}

bool SkAnimatorScript2::evalMemberCommon(const SkMemberInfo* info,
        SkDisplayable* displayable, SkOperand2* value) {
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
                value->fS32 = *(int32_t*) info->memberData(displayable);    // OK for SkScalar too
            if (type == SkType_MSec) {
                value->fScalar = value->fS32 * 0.001f;
                type = SkType_Float;
            }
            break;
        case SkType_String: {
            SkString* displayableString;
            if (info->fType != SkType_MemberProperty && info->fType != SkType_MemberFunction) {
                info->getString(displayable, &displayableString);
                value->fString = new SkString(*displayableString);
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
            SkOpArray* array = value->fArray = new SkOpArray(ToOpType(original));
            track(array);
            int count = displayableArray->count();
            if (count > 0) {
                array->setCount(count);
                memcpy(array->begin(), displayableArray->begin(), count * sizeof(SkOperand2));
            }
            } break;
        default:
            SkASSERT(0); // unimplemented
    }
    return true;
}

const SkDisplayEnumMap& SkAnimatorScript2::GetEnumValues(SkDisplayTypes type) {
    int index = SkTSearch<SkDisplayTypes>(&gEnumMaps[0].fType, gEnumMapCount, type,
        sizeof(SkDisplayEnumMap));
    SkASSERT(index >= 0);
    return gEnumMaps[index];
}

SkDisplayTypes SkAnimatorScript2::ToDisplayType(SkOperand2::OpType type) {
    int val = type;
    switch (val) {
        case SkOperand2::kNoType:
            return SkType_Unknown;
        case SkOperand2::kS32:
            return SkType_Int;
        case SkOperand2::kScalar:
            return SkType_Float;
        case SkOperand2::kString:
            return SkType_String;
        case SkOperand2::kArray:
            return SkType_Array;
        case SkOperand2::kObject:
            return SkType_Displayable;
        default:
            SkASSERT(0);
            return SkType_Unknown;
    }
}

SkOperand2::OpType SkAnimatorScript2::ToOpType(SkDisplayTypes type) {
    if (SkDisplayType::IsDisplayable(NULL /* fMaker */, type))
        return SkOperand2::kObject;
    if (SkDisplayType::IsEnum(NULL /* fMaker */, type))
        return SkOperand2::kS32;
    switch (type) {
        case SkType_ARGB:
        case SkType_MSec:
        case SkType_Int:
            return SkOperand2::kS32;
        case SkType_Float:
        case SkType_Point:
        case SkType_3D_Point:
            return SkOperand2::kScalar;
        case SkType_Base64:
        case SkType_DynamicString:
        case SkType_String:
            return SkOperand2::kString;
        case SkType_Array:
            return SkOperand2::kArray;
        case SkType_Unknown:
            return SkOperand2::kNoType;
        default:
            SkASSERT(0);
            return SkOperand2::kNoType;
    }
}

bool SkAnimatorScript2::MapEnums(const char* ptr, const char* match, size_t len, int* value) {
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

#if defined SK_DEBUG

#include "SkAnimator.h"

static const char scriptTestSetup[]  =
"<screenplay>"
    "<apply>"
        "<paint>"
            "<emboss id='emboss' direction='[1,1,1]'  />"
        "</paint>"
        "<animateField id='animation' field='direction' target='emboss' from='[1,1,1]' to='[-1,1,1]' dur='1'/>"
        "<set lval='direction[0]' target='emboss' to='-1' />"
    "</apply>"
    "<color id='testColor' color='0 ? rgb(0,0,0) : rgb(255,255,255)' />"
    "<color id='xColor' color='rgb(12,34,56)' />"
    "<typedArray id='emptyArray' />"
    "<typedArray id='intArray' values='[1, 4, 6]' />"
    "<s32 id='idx' value='2' />"
    "<s32 id='idy' value='2' />"
    "<string id='alpha' value='abc' />"
    "<rectangle id='testRect' left='Math.cos(0)' top='2' right='12' bottom='5' />"
    "<event id='evt'>"
        "<input name='x' />"
        "<apply scope='idy'>"
            "<set field='value' to='evt.x.s32' />"
        "</apply>"
    "</event>"
"</screenplay>";

static const SkScriptNAnswer scriptTests[]  = {
    {    "alpha+alpha", SkType_String, 0, 0, "abcabc" },
    {    "0 ? Math.sin(0) : 1", SkType_Int, 1 },
    {    "intArray[4]", SkType_Unknown },
    {    "emptyArray[4]", SkType_Unknown },
    {    "idx", SkType_Int, 2 },
    {    "intArray.length", SkType_Int, 3 },
    {    "intArray.values[0]", SkType_Int, 1 },
    {    "intArray[0]", SkType_Int, 1 },
    {    "idx.value", SkType_Int, 2 },
    {    "alpha.value", SkType_String, 0, 0, "abc" },
    {    "alpha", SkType_String, 0, 0, "abc" },
    {    "alpha.value+alpha.value", SkType_String, 0, 0, "abcabc" },
    {    "alpha+idx", SkType_String, 0, 0, "abc2" },
    {    "idx+alpha", SkType_String, 0, 0, "2abc" },
    {    "intArray[idx]", SkType_Int, 6 },
    {    "alpha.slice(1,2)", SkType_String, 0, 0, "b" },
    {    "alpha.value.slice(1,2)", SkType_String, 0, 0, "b" },
    {    "Math.sin(0)", SkType_Float, 0, SkIntToScalar(0) },
    {    "testRect.left+2", SkType_Float, 0, SkIntToScalar(3) },
    {    "0 ? intArray[0] : 1", SkType_Int, 1 },
    {    "0 ? intArray.values[0] : 1", SkType_Int, 1 },
    {    "0 ? idx : 1", SkType_Int, 1 },
    {    "0 ? idx.value : 1", SkType_Int, 1 },
    {    "0 ? alpha.slice(1,2) : 1", SkType_Int, 1 },
    {    "0 ? alpha.value.slice(1,2) : 1", SkType_Int, 1 },
    { "idy", SkType_Int, 3 }
};

#define SkScriptNAnswer_testCount    SK_ARRAY_COUNT(scriptTests)

void SkAnimatorScript2::UnitTest() {
#if defined(SK_SUPPORT_UNITTEST)
    SkAnimator animator;
    SkASSERT(animator.decodeMemory(scriptTestSetup, sizeof(scriptTestSetup)-1));
    SkEvent evt;
    evt.setString("id", "evt");
    evt.setS32("x", 3);
    animator.doUserEvent(evt);
    // set up animator with memory script above, then run value tests
    for (int index = 0; index < SkScriptNAnswer_testCount; index++) {
        SkAnimatorScript2 engine(*animator.fMaker, NULL, scriptTests[index].fType);
        SkScriptValue2 value;
        const char* script = scriptTests[index].fScript;
        bool success = engine.evaluateScript(&script, &value);
        if (success == false) {
            SkASSERT(scriptTests[index].fType == SkType_Unknown);
            continue;
        }
        SkASSERT(value.fType == ToOpType(scriptTests[index].fType));
        SkScalar error;
        switch (value.fType) {
            case SkOperand2::kS32:
                SkASSERT(value.fOperand.fS32 == scriptTests[index].fIntAnswer);
                break;
            case SkOperand2::kScalar:
                error = SkScalarAbs(value.fOperand.fScalar - scriptTests[index].fScalarAnswer);
                SkASSERT(error < SK_Scalar1 / 10000);
                break;
            case SkOperand2::kString:
                SkASSERT(value.fOperand.fString->equals(scriptTests[index].fStringAnswer));
                break;
            default:
                SkASSERT(0);
        }
    }
#endif
}

#endif
