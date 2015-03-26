
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayApply_DEFINED
#define SkDisplayApply_DEFINED

#include "SkAnimateBase.h"
#include "SkADrawable.h"
#include "SkIntArray.h"

class SkActive;

class SkApply : public SkADrawable {
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
    bool contains(SkDisplayable*) override;
//  void createActive(SkAnimateMaker& );
    SkDisplayable* deepCopy(SkAnimateMaker* ) override;
    void disable();
    bool draw(SkAnimateMaker& ) override;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    bool enable(SkAnimateMaker& ) override;
    void enableCreate(SkAnimateMaker& );
    void enableDynamic(SkAnimateMaker& );
    void endSave(int index);
    Mode getMode() { return mode; }
    bool getProperty(int index, SkScriptValue* value) const override;
    SkADrawable* getScope() { return scope; }
    void getStep(SkScriptValue* );
    SkADrawable* getTarget(SkAnimateBase* );
    bool hasDelayedAnimator() const;
    bool hasEnable() const override;
    bool inactivate(SkAnimateMaker& maker);
    void initialize() override;
    bool interpolate(SkAnimateMaker& , SkMSec time);
    void onEndElement(SkAnimateMaker& ) override;
    const SkMemberInfo* preferredChild(SkDisplayTypes type) override;
    void refresh(SkAnimateMaker& );
    void reset();
    bool resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* ) override;
    bool resolveField(SkAnimateMaker& , SkDisplayable* parent, SkString* str);
    void save(int index);
    void setEmbedded() { fEmbedded = true; }
    bool setProperty(int index, SkScriptValue& ) override;
    void setSteps(int _steps) override;
//  virtual void setTime(SkMSec time);
#ifdef SK_DEBUG
    void validate() override;
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
    SkADrawable* scope;
    int32_t steps;
    Transition transition;
    SkActive* fActive;
    SkTDAnimateArray fAnimators;
//  SkADrawable* fCurrentScope;
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
    typedef SkADrawable INHERITED;
};

#endif // SkDisplayApply_DEFINED
