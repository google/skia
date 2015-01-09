
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
    bool contains(SkDisplayable*) SK_OVERRIDE;
//  void createActive(SkAnimateMaker& );
    SkDisplayable* deepCopy(SkAnimateMaker* ) SK_OVERRIDE;
    void disable();
    bool draw(SkAnimateMaker& ) SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    bool enable(SkAnimateMaker& ) SK_OVERRIDE;
    void enableCreate(SkAnimateMaker& );
    void enableDynamic(SkAnimateMaker& );
    void endSave(int index);
    Mode getMode() { return mode; }
    bool getProperty(int index, SkScriptValue* value) const SK_OVERRIDE;
    SkADrawable* getScope() { return scope; }
    void getStep(SkScriptValue* );
    SkADrawable* getTarget(SkAnimateBase* );
    bool hasDelayedAnimator() const;
    bool hasEnable() const SK_OVERRIDE;
    bool inactivate(SkAnimateMaker& maker);
    void initialize() SK_OVERRIDE;
    bool interpolate(SkAnimateMaker& , SkMSec time);
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
    const SkMemberInfo* preferredChild(SkDisplayTypes type) SK_OVERRIDE;
    void refresh(SkAnimateMaker& );
    void reset();
    bool resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* ) SK_OVERRIDE;
    bool resolveField(SkAnimateMaker& , SkDisplayable* parent, SkString* str);
    void save(int index);
    void setEmbedded() { fEmbedded = true; }
    bool setProperty(int index, SkScriptValue& ) SK_OVERRIDE;
    void setSteps(int _steps) SK_OVERRIDE;
//  virtual void setTime(SkMSec time);
#ifdef SK_DEBUG
    void validate() SK_OVERRIDE;
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
