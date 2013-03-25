/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_DEFINED
#define SkFontMgr_DEFINED

#include "SkRefCnt.h"
#include "SkFontStyle.h"

class SkData;
class SkStream;
class SkString;

class SkFontStyleSet : public SkRefCnt {
public:
    int count() const;
    void getStyle(int index, SkFontStyle*) const;
    SkTypeface* createTypeface(int index) const;
};

class SkFontFamilySet : public SkRefCnt {
public:
    int count() const;
    void getName(int index, SkString* familyName) const;
    SkFontStyleSet* refStyleSet(int index) const;
};

class SkFontMgr : public SkRefCnt {
public:
    /**
     *  Return a fontfamily set, which can iterate all of the font families
     *  available to this fontmgr. The caller is responsible for calling unref()
     *  on the returned object. Will never return NULL.
     */
    SkFontFamilySet* createFamilySet();

    /**
     *  Find the closest matching typeface to the specified familyName and style
     *  and return a ref to it. The caller must call unref() on the returned
     *  object. Will never return NULL, as it will return the default font if
     *  no matching font is found.
     */
    SkTypeface* matchFamilyStyle(const char familyName[], const SkFontStyle&);

    /**
     *  Create a typeface for the specified data and TTC index (pass 0 for none)
     *  or NULL if the data is not recognized. The caller must call unref() on
     *  the returned object if it is not null.
     */
    SkTypeface* createFromData(SkData*, int ttcIndex = 0);

    /**
     *  Create a typeface for the specified stream and TTC index
     *  (pass 0 for none) or NULL if the stream is not recognized. The caller
     *  must call unref() on the returned object if it is not null.
     */
    SkTypeface* createFromStream(SkStream*, int ttcIndex = 0);

    /**
     *  Create a typeface for the specified fileName and TTC index
     *  (pass 0 for none) or NULL if the file is not found, or its contents are
     *  not recognized. The caller must call unref() on the returned object
     *  if it is not null.
     */
    SkTypeface* createFromFile(const char path[], int ttcIndex = 0);

private:
    typedef SkRefCnt INHERITED;
};

#endif
