/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontStream_DEFINED
#define SkFontStream_DEFINED

class SkStream;

#include "SkTypeface.h"

class SkFontStream {
public:
    /**
     *  Return the number of shared 'fonts' inside a TTC sfnt, or return 0
     *  if the stream is a normal sfnt (not a TTC).
     *
     *  Note: the stream is rewound initially, but is returned at an arbitrary
     *  read offset.
     */
    static int CountTTCEntries(SkStream*);

    /**
     *  @param ttcIndex 0 for normal sfnts, or the index within a TTC sfnt.
     *
     *  Note: the stream is rewound initially, but is returned at an arbitrary
     *  read offset.
     */
    static int GetTableTags(SkStream*, int ttcIndex, SkFontTableTag tags[]);

    /**
     *  @param ttcIndex 0 for normal sfnts, or the index within a TTC sfnt.
     *
     *  Note: the stream is rewound initially, but is returned at an arbitrary
     *  read offset.
     */
    static size_t GetTableData(SkStream*, int ttcIndex, SkFontTableTag tag,
                               size_t offset, size_t length, void* data);
};

#endif
