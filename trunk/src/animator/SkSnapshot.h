
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSnapShot_DEFINED
#define SkSnapShot_DEFINED

#include "SkDrawable.h"
#include "SkImageDecoder.h"
#include "SkMemberInfo.h"
#include "SkString.h"

class SkSnapshot: public SkDrawable {
    DECLARE_MEMBER_INFO(Snapshot);
    SkSnapshot();
    virtual bool draw(SkAnimateMaker& );
    private:
    SkString filename;
    SkScalar quality;
    SkBool sequence;
    int /*SkImageEncoder::Type*/    type;
    int fSeqVal;
};

#endif // SkSnapShot_DEFINED

