
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTagList_DEFINED
#define SkTagList_DEFINED

#include "SkTypes.h"

enum SkTagListEnum {
    kListeners_SkTagList,
    kViewLayout_SkTagList,
    kViewArtist_SkTagList,

    kSkTagListCount
};

struct SkTagList {
    SkTagList*  fNext;
    uint16_t    fExtra16;
    uint8_t     fExtra8;
    uint8_t     fTag;

    SkTagList(U8CPU tag) : fTag(SkToU8(tag))
    {
        SkASSERT(tag < kSkTagListCount);
        fNext       = nullptr;
        fExtra16    = 0;
        fExtra8     = 0;
    }
    virtual ~SkTagList();

    static SkTagList*   Find(SkTagList* head, U8CPU tag);
    static void         DeleteTag(SkTagList** headptr, U8CPU tag);
    static void         DeleteAll(SkTagList* head);
};

#endif
