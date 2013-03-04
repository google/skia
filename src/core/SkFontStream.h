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
    static int GetTableTags(SkStream*, SkFontTableTag tags[]);
    static size_t GetTableData(SkStream*, SkFontTableTag tag,
                               size_t offset, size_t length, void* data);
};

#endif


