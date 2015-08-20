/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTTCFHeader_DEFINED
#define SkTTCFHeader_DEFINED

#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkTTCFHeader {
    SK_SFNT_ULONG ttcTag;
    static const SK_OT_CHAR TAG0 = 't';
    static const SK_OT_CHAR TAG1 = 't';
    static const SK_OT_CHAR TAG2 = 'c';
    static const SK_OT_CHAR TAG3 = 'f';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkTTCFHeader>::value;

    SK_OT_Fixed version;
    static const SK_OT_Fixed version_1 = SkTEndian_SwapBE32(1 << 16);
    static const SK_OT_Fixed version_2 = SkTEndian_SwapBE32(2 << 16);

    SK_OT_ULONG numOffsets;
    //SK_OT_ULONG offset[numOffsets]

    struct Version2Ext {
        SK_OT_ULONG dsigType;
        struct dsigType_None {
            static const SK_OT_CHAR TAG0 = 0;
            static const SK_OT_CHAR TAG1 = 0;
            static const SK_OT_CHAR TAG2 = 0;
            static const SK_OT_CHAR TAG3 = 0;
            static const SK_OT_ULONG TAG = SkOTTableTAG<dsigType_None>::value;
        };
        struct dsigType_Format1 {
            static const SK_OT_CHAR TAG0 = 'D';
            static const SK_OT_CHAR TAG1 = 'S';
            static const SK_OT_CHAR TAG2 = 'I';
            static const SK_OT_CHAR TAG3 = 'G';
            static const SK_OT_ULONG TAG = SkOTTableTAG<dsigType_Format1>::value;
        };
        SK_OT_ULONG dsigLength; //Length of DSIG table (in bytes).
        SK_OT_ULONG dsigOffset; //Offset of DSIG table from the beginning of file (in bytes).
    };// version2ext (if version == version_2)
};

#pragma pack(pop)


static_assert(sizeof(SkTTCFHeader) == 12, "sizeof_SkTTCFHeader_not_12");

#endif
