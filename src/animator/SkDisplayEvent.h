
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayEvent_DEFINED
#define SkDisplayEvent_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"
#include "SkIntArray.h"
#include "SkKey.h"

class SkEvent;

class SkDisplayEvent : public SkDisplayable {
    DECLARE_DISPLAY_MEMBER_INFO(Event);
    enum Kind {
        kNo_kind,
        kKeyChar,
        kKeyPress,
        kKeyPressUp,    //i assume the order here is intended to match with skanimatorscript.cpp
        kMouseDown,
        kMouseDrag,
        kMouseMove,
        kMouseUp,
        kOnEnd,
        kOnload,
        kUser
    };
    SkDisplayEvent();
    virtual ~SkDisplayEvent();
    bool addChild(SkAnimateMaker& , SkDisplayable* child) override;
    bool contains(SkDisplayable*) override;
    SkDisplayable* contains(const SkString& ) override;
#ifdef SK_DEBUG
    void dumpEvent(SkAnimateMaker* );
#endif
    bool enableEvent(SkAnimateMaker& );
    bool getProperty(int index, SkScriptValue* ) const override;
    void onEndElement(SkAnimateMaker& maker) override;
    void populateInput(SkAnimateMaker& , const SkEvent& fEvent);
    bool setProperty(int index, SkScriptValue& ) override;
protected:
    SkKey code;
    SkBool disable;
    Kind kind;
    SkString target;
    SkScalar x;
    SkScalar y;
    SkTDDisplayableArray fChildren;
    mutable SkString fKeyString;
    SkKey fLastCode; // last key to trigger this event
    SkKey fMax; // if the code expresses a range
    SkDisplayable* fTarget; // used by onEnd
private:
    void deleteMembers();
    friend class SkEvents;
    typedef SkDisplayable INHERITED;
};

#endif // SkDisplayEvent_DEFINED
