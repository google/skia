
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMMapStream_DEFINED
#define SkMMapStream_DEFINED

#include "SkStream.h"

class SkMMAPStream : public SkMemoryStream {
public:
    SkMMAPStream(const char filename[]);
    virtual ~SkMMAPStream();

    virtual void setMemory(const void* data, size_t length, bool);
private:
    void*   fAddr;
    size_t  fSize;

    void closeMMap();

    typedef SkMemoryStream INHERITED;
};

#endif
