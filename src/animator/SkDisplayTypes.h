
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayTypes_DEFINED
#define SkDisplayTypes_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"
#include "SkTypedArray.h"

class SkOpArray; // compiled script experiment


class SkDisplayDepend : public SkDisplayable {
public:
    virtual bool canContainDependents() const;
    void addDependent(SkDisplayable* displayable) {
        if (fDependents.find(displayable) < 0)
            *fDependents.append() = displayable;
    }
    virtual void dirty();
private:
    SkTDDisplayableArray fDependents;
    typedef SkDisplayable INHERITED;
};

class SkDisplayBoolean : public SkDisplayDepend {
    DECLARE_DISPLAY_MEMBER_INFO(Boolean);
    SkDisplayBoolean();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    SkBool value;
    friend class SkAnimatorScript;
    friend class SkAnimatorScript_Box;
    friend class SkAnimatorScript_Unbox;
    typedef SkDisplayDepend INHERITED;
};

class SkDisplayInt : public SkDisplayDepend {
    DECLARE_DISPLAY_MEMBER_INFO(Int);
    SkDisplayInt();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
private:
    int32_t value;
    friend class SkAnimatorScript;
    friend class SkAnimatorScript_Box;
    friend class SkAnimatorScript_Unbox;
    typedef SkDisplayDepend INHERITED;
};

class SkDisplayFloat : public SkDisplayDepend {
    DECLARE_DISPLAY_MEMBER_INFO(Float);
    SkDisplayFloat();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
private:
    SkScalar value;
    friend class SkAnimatorScript;
    friend class SkAnimatorScript_Box;
    friend class SkAnimatorScript_Unbox;
    typedef SkDisplayDepend INHERITED;
};

class SkDisplayString : public SkDisplayDepend {
    DECLARE_DISPLAY_MEMBER_INFO(String);
    SkDisplayString();
    SkDisplayString(SkString& );
    void executeFunction(SkDisplayable* , int index,
        SkTDArray<SkScriptValue>& parameters, SkDisplayTypes type,
        SkScriptValue* ) override;
    const SkFunctionParamType* getFunctionsParameters() override;
    bool getProperty(int index, SkScriptValue* ) const override;
    SkString value;
private:
    static const SkFunctionParamType fFunctionParameters[];
};

class SkDisplayArray : public SkDisplayDepend {
    DECLARE_DISPLAY_MEMBER_INFO(Array);
    SkDisplayArray();
    SkDisplayArray(SkTypedArray& );
    SkDisplayArray(SkOpArray& ); // compiled script experiment
    virtual ~SkDisplayArray();
    bool getProperty(int index, SkScriptValue* ) const override;
private:
    SkTypedArray values;
    friend class SkAnimator;
    friend class SkAnimatorScript;
    friend class SkAnimatorScript2;
    friend class SkAnimatorScript_Unbox;
    friend class SkDisplayable;
    friend struct SkMemberInfo;
    friend class SkScriptEngine;
};

#endif // SkDisplayTypes_DEFINED
