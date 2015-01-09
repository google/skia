
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayPost_DEFINED
#define SkDisplayPost_DEFINED

#include "SkDisplayable.h"
#include "SkEvent.h"
#include "SkEventSink.h"
#include "SkMemberInfo.h"
#include "SkIntArray.h"

class SkDataInput;
class SkAnimateMaker;

class SkPost : public SkDisplayable {
    DECLARE_MEMBER_INFO(Post);
    enum Mode {
        kDeferred,
        kImmediate
    };
    SkPost();
    virtual ~SkPost();
    bool addChild(SkAnimateMaker& , SkDisplayable* child) SK_OVERRIDE;
    bool childrenNeedDisposing() const SK_OVERRIDE;
    void dirty() SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    bool enable(SkAnimateMaker& ) SK_OVERRIDE;
    bool hasEnable() const SK_OVERRIDE;
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
    void setChildHasID() SK_OVERRIDE;
    bool setProperty(int index, SkScriptValue& ) SK_OVERRIDE;
protected:
    SkMSec delay;
    SkString sink;
//  SkBool initialized;
    Mode mode;
    SkEvent fEvent;
    SkAnimateMaker* fMaker;
    SkTDDataArray fParts;
    SkEventSinkID fSinkID;
    SkAnimateMaker* fTargetMaker;
    SkBool8 fChildHasID;
    SkBool8 fDirty;
private:
    void findSinkID();
    friend class SkDataInput;
    typedef SkDisplayable INHERITED;
};

#endif //SkDisplayPost_DEFINED
