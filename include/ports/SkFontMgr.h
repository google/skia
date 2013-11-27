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
class SkTypeface;

class SK_API SkFontStyleSet : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkFontStyleSet)

    virtual int count() = 0;
    virtual void getStyle(int index, SkFontStyle*, SkString* style) = 0;
    virtual SkTypeface* createTypeface(int index) = 0;
    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) = 0;

    static SkFontStyleSet* CreateEmpty();

private:
    typedef SkRefCnt INHERITED;
};

class SkTypeface;

class SK_API SkFontMgr : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkFontMgr)

    int countFamilies();
    void getFamilyName(int index, SkString* familyName);
    SkFontStyleSet* createStyleSet(int index);

    /**
     *  The caller must call unref() on the returned object.
     *  Never returns NULL; will return an empty set if the name is not found.
     */
    SkFontStyleSet* matchFamily(const char familyName[]);

    /**
     *  Find the closest matching typeface to the specified familyName and style
     *  and return a ref to it. The caller must call unref() on the returned
     *  object. Will never return NULL, as it will return the default font if
     *  no matching font is found.
     */
    SkTypeface* matchFamilyStyle(const char familyName[], const SkFontStyle&);

    SkTypeface* matchFaceStyle(const SkTypeface*, const SkFontStyle&);

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

    SkTypeface* legacyCreateTypeface(const char familyName[],
                                     unsigned typefaceStyleBits);

    /**
     *  Return a ref to the default fontmgr. The caller must call unref() on
     *  the returned object.
     */
    static SkFontMgr* RefDefault();

protected:
    virtual int onCountFamilies() = 0;
    virtual void onGetFamilyName(int index, SkString* familyName) = 0;
    virtual SkFontStyleSet* onCreateStyleSet(int index) = 0;

    /** May return NULL if the name is not found. */
    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) = 0;

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle&) = 0;
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                         const SkFontStyle&) = 0;

    virtual SkTypeface* onCreateFromData(SkData*, int ttcIndex) = 0;
    virtual SkTypeface* onCreateFromStream(SkStream*, int ttcIndex) = 0;
    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) = 0;

    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                               unsigned styleBits) = 0;
private:
    static SkFontMgr* Factory();    // implemented by porting layer
    friend void set_up_default(SkFontMgr** singleton);

    typedef SkRefCnt INHERITED;
};

#endif
