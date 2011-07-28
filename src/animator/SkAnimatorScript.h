
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAnimatorScript_DEFINED
#define SkAnimatorScript_DEFINED

#include "SkDisplayable.h"
#include "SkScript.h"
#include "SkTypedArray.h"

class SkAnimateMaker;
struct SkMemberInfo;

struct SkDisplayEnumMap {
    SkDisplayTypes fType;
    const char* fValues;
};

class SkAnimatorScript : public SkScriptEngine {
public:
    SkAnimatorScript(SkAnimateMaker& , SkDisplayable* , SkDisplayTypes type);
    ~SkAnimatorScript();
    bool evaluate(const char* script, SkScriptValue* , SkDisplayTypes type);
    void track(SkDisplayable* displayable) { 
        SkASSERT(fTrackDisplayable.find(displayable) < 0);  
        *fTrackDisplayable.append() = displayable; }
    static bool EvaluateDisplayable(SkAnimateMaker& , SkDisplayable* , const char* script, SkDisplayable** );
    static bool EvaluateFloat(SkAnimateMaker& , SkDisplayable* , const char* script, SkScalar* );
    static bool EvaluateInt(SkAnimateMaker& , SkDisplayable* , const char* script, int32_t* );
    static bool EvaluateString(SkAnimateMaker& , SkDisplayable* , const char* script, SkString* );
    static bool EvaluateString(SkAnimateMaker& , SkDisplayable* , SkDisplayable* parent, const char* script, SkString* );
    static bool MapEnums(const char* ptr, const char* match, size_t len, int* value);
protected:
    static bool Box(void* user, SkScriptValue* );
    static bool Eval(const char* function, size_t len, SkTDArray<SkScriptValue>& params, 
        void* callBack, SkScriptValue* );
    static bool EvalEnum(const char* token, size_t len, void* callBack, SkScriptValue* );
    static bool EvalID(const char* token, size_t len, void* callBack, SkScriptValue* );
    static bool EvalMember(const char* member, size_t len, void* object, void* eng, 
        SkScriptValue* value);
    static bool EvalMemberCommon(SkScriptEngine* , const SkMemberInfo* info, 
        SkDisplayable* displayable, SkScriptValue* value);
    static bool EvalMemberFunction(const char* member, size_t len, void* object, 
        SkTDArray<SkScriptValue>& params, void* user, SkScriptValue* value);
    static bool EvalNamedColor(const char* token, size_t len, void* callBack, SkScriptValue* );
    static bool EvalRGB(const char* function, size_t len, SkTDArray<SkScriptValue>& params, 
        void* callBack, SkScriptValue* );
    static const SkDisplayEnumMap& GetEnumValues(SkDisplayTypes type); 
    static bool Infinity(const char* token, size_t len, void* callBack, SkScriptValue* );
    static bool IsFinite(const char* function, size_t len, SkTDArray<SkScriptValue>& params, 
        void* callBack, SkScriptValue* );
    static bool IsNaN(const char* function, size_t len, SkTDArray<SkScriptValue>& params, 
        void* callBack, SkScriptValue* );
    static bool NaN(const char* token, size_t len, void* callBack, SkScriptValue* );
    static bool Unbox(void* , SkScriptValue* scriptValue);
    SkTDDisplayableArray fTrackDisplayable;
    SkAnimateMaker& fMaker;
    SkDisplayable* fParent;
    SkDisplayable* fWorking;
private:
    friend class SkDump;
    friend struct SkScriptNAnswer;
#ifdef SK_SUPPORT_UNITTEST
public:
    static void UnitTest();
#endif
};

#endif // SkAnimatorScript_DEFINED

