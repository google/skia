
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayRandom_DEFINED
#define SkDisplayRandom_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"
#include "SkRandom.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

class SkDisplayRandom : public SkDisplayable {
    DECLARE_DISPLAY_MEMBER_INFO(Random);
    SkDisplayRandom();
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    virtual bool getProperty(int index, SkScriptValue* value) const SK_OVERRIDE;
    virtual bool setProperty(int index, SkScriptValue& ) SK_OVERRIDE;
private:
    SkScalar blend;
    SkScalar min;
    SkScalar max;
    mutable SkRandom fRandom;
};

#endif // SkDisplayRandom_DEFINED
