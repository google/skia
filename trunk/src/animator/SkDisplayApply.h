
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayApply_DEFINED
#define SkDisplayApply_DEFINED

#include "SkAnimateBase.h"
#include "SkDrawable.h"
#include "SkIntArray.h"

class SkActive;

class SkApply : public SkDrawable {
    DECLARE_MEMBER_INFO(Apply);
public:

    SkApply();
    virtual ~SkApply();

    enum Transition {
        kTransition_normal,
        kTransition_reverse
    };

    enum Mode {
        kMode_create,
        kMode_immediate,
        //kMode_once
    };
    void activate(SkAnimateMaker& );
    void append(SkApply* apply);
    void appendActive(SkActive* );
    void applyValues(int animatorIndex, SkOperand* values, int count,
        SkDisplayTypes , SkMSec time);
    virtual bool contains(SkDisplayable*);
//  void createActive(SkAnimateMaker& );
    virtual SkDisplayable* deepCopy(SkAnimateMaker* );
    void disable();
    virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
    virtual bool enable(SkAnimateMaker& );
    void enableCreate(SkAnimateMaker& );
    void enableDynamic(SkAnimateMaker& );
    void endSave(int index);
    Mode getMode() { return mode; }
    virtual bool getProperty(int index, SkScriptValue* value) const;
    SkDrawable* getScope() { return scope; }
    void getStep(SkScriptValue* );
    SkDrawable* getTarget(SkAnimateBase* );
    bool hasDelayedAnimator() const;
    virtual bool hasEnable() const;
    bool inactivate(SkAnimateMaker& maker);
    virtual void initialize();
    bool interpolate(SkAnimateMaker& , SkMSec time);
    virtual void onEndElement(SkAnimateMaker& );
    virtual const SkMemberInfo* preferredChild(SkDisplayTypes type);
    void refresh(SkAnimateMaker& );
    void reset();
    virtual bool resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* );
    bool resolveField(SkAnimateMaker& , SkDisplayable* parent, SkString* str);
    void save(int index);
    void setEmbedded() { fEmbedded = true; }
    virtual bool setProperty(int index, SkScriptValue& );
    virtual void setSteps(int _steps);
//  virtual void setTime(SkMSec time);
#ifdef SK_DEBUG
    virtual void validate();
#endif
private:
    SkMSec begin;
    SkBool dontDraw;
    SkString dynamicScope;
    SkMSec interval;
    Mode mode;
#if 0
    SkBool pickup;
#endif
    SkBool restore;
    SkDrawable* scope;
    int32_t steps;
    Transition transition;
    SkActive* fActive;
    SkTDAnimateArray fAnimators;
//  SkDrawable* fCurrentScope;
    SkMSec fLastTime;   // used only to return script property time
    SkTDDrawableArray fScopes;
    SkBool fAppended : 1;
    SkBool fContainsScope : 1;
    SkBool fDeleteScope : 1;
    SkBool fEmbedded : 1;
    SkBool fEnabled : 1;
    SkBool fEnabling : 1; // set if calling interpolate from enable
    friend class SkActive;
    friend class SkDisplayList;
    typedef SkDrawable INHERITED;
};

#endif // SkDisplayApply_DEFINED


