
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDump_DEFINED
#define SkDump_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"

class SkAnimateMaker;
class SkString;

class SkDump : public SkDisplayable {
    DECLARE_MEMBER_INFO(Dump);
#ifdef SK_DUMP_ENABLED
    SkDump();
    virtual bool enable(SkAnimateMaker & );
    bool evaluate(SkAnimateMaker &);
    virtual bool hasEnable() const;
    static void GetEnumString(SkDisplayTypes , int index, SkString* result);
    SkBool displayList;
    SkBool eventList;
    SkBool events;
    SkString name;
    SkBool groups;
    SkBool posts;
    SkString script;
#else
    virtual bool enable(SkAnimateMaker & );
    virtual bool hasEnable() const;
    virtual bool setProperty(int index, SkScriptValue& );
#endif
};


#endif // SkDump_DEFINED

