
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawText_DEFINED
#define SkDrawText_DEFINED

#include "SkBoundable.h"
#include "SkMemberInfo.h"

class SkText : public SkBoundable {
    DECLARE_MEMBER_INFO(Text);
    SkText();
    virtual ~SkText();
    virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
    virtual bool getProperty(int index, SkScriptValue* value) const ;
    const char* getText() { return text.c_str(); }
    size_t getSize() { return text.size(); }
protected:
    SkString text;
    SkScalar x;
    SkScalar y;
private:
    friend class SkTextToPath;
    typedef SkBoundable INHERITED;
};

#endif // SkDrawText_DEFINED
