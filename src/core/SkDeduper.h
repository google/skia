/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeduper_DEFINED
#define SkDeduper_DEFINED

#include "SkFlattenable.h"

class SkImage;
class SkPicture;
class SkTypeface;

class SkDeduper {
public:
    virtual ~SkDeduper() {}

    // These return 0 on failure

    virtual int findOrDefineImage(SkImage*) = 0;
    virtual int findOrDefinePicture(SkPicture*) = 0;
    virtual int findOrDefineTypeface(SkTypeface*) = 0;
    virtual int findOrDefineFactory(SkFlattenable*) = 0;
};

class SkInflator {
public:
    virtual ~SkInflator() {}

    virtual SkImage* getImage(int) = 0;
    virtual SkPicture* getPicture(int) = 0;
    virtual SkTypeface* getTypeface(int) = 0;
    virtual SkFlattenable::Factory getFactory(int) = 0;
};

#endif
