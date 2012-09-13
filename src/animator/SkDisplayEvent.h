
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
    virtual bool add(SkAnimateMaker& , SkDisplayable* child) SK_OVERRIDE;
    virtual bool contains(SkDisplayable*);
    virtual SkDisplayable* contains(const SkString& );
#ifdef SK_DEBUG
    void dumpEvent(SkAnimateMaker* );
#endif
    bool enableEvent(SkAnimateMaker& );
    virtual bool getProperty(int index, SkScriptValue* ) const;
    virtual void onEndElement(SkAnimateMaker& maker);
    void populateInput(SkAnimateMaker& , const SkEvent& fEvent);
    virtual bool setProperty(int index, SkScriptValue& );
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

