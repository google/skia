/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_DEFINED
#define SkFontMgr_DEFINED

#include "SkFontArguments.h"
#include "SkFontStyle.h"
#include "SkRefCnt.h"
#include "SkTypes.h"

class SkData;
class SkFontData;
class SkStreamAsset;
class SkString;
class SkTypeface;

class SK_API SkFontStyleSet : public SkRefCnt {
public:
    virtual int count() = 0;
    virtual void getStyle(int index, SkFontStyle*, SkString* style) = 0;
    virtual SkTypeface* createTypeface(int index) = 0;
    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) = 0;

    static SkFontStyleSet* CreateEmpty();

protected:
    SkTypeface* matchStyleCSS3(const SkFontStyle& pattern);

private:
    typedef SkRefCnt INHERITED;
};

class SK_API SkFontMgr : public SkRefCnt {
public:
    int countFamilies() const;
    void getFamilyName(int index, SkString* familyName) const;
    SkFontStyleSet* createStyleSet(int index) const;

    /**
     *  The caller must call unref() on the returned object.
     *  Never returns NULL; will return an empty set if the name is not found.
     *
     *  Passing nullptr as the parameter will return the default system family.
     *  Note that most systems don't have a default system family, so passing nullptr will often
     *  result in the empty set.
     *
     *  It is possible that this will return a style set not accessible from
     *  createStyleSet(int) due to hidden or auto-activated fonts.
     */
    SkFontStyleSet* matchFamily(const char familyName[]) const;

    /**
     *  Find the closest matching typeface to the specified familyName and style
     *  and return a ref to it. The caller must call unref() on the returned
     *  object. Will never return NULL, as it will return the default font if
     *  no matching font is found.
     *
     *  Passing |nullptr| as the parameter for |familyName| will return the
     *  default system font.
     *
     *  It is possible that this will return a style set not accessible from
     *  createStyleSet(int) or matchFamily(const char[]) due to hidden or
     *  auto-activated fonts.
     */
    SkTypeface* matchFamilyStyle(const char familyName[], const SkFontStyle&) const;

    /**
     *  Use the system fallback to find a typeface for the given character.
     *  Note that bcp47 is a combination of ISO 639, 15924, and 3166-1 codes,
     *  so it is fine to just pass a ISO 639 here.
     *
     *  Will return NULL if no family can be found for the character
     *  in the system fallback.
     *
     *  Passing |nullptr| as the parameter for |familyName| will return the
     *  default system font.
     *
     *  bcp47[0] is the least significant fallback, bcp47[bcp47Count-1] is the
     *  most significant. If no specified bcp47 codes match, any font with the
     *  requested character will be matched.
     */
    SkTypeface* matchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                          const char* bcp47[], int bcp47Count,
                                          SkUnichar character) const;

    SkTypeface* matchFaceStyle(const SkTypeface*, const SkFontStyle&) const;

    /**
     *  Create a typeface for the specified data and TTC index (pass 0 for none)
     *  or NULL if the data is not recognized. The caller must call unref() on
     *  the returned object if it is not null.
     */
    SkTypeface* createFromData(SkData*, int ttcIndex = 0) const;

    /**
     *  Create a typeface for the specified stream and TTC index
     *  (pass 0 for none) or NULL if the stream is not recognized. The caller
     *  must call unref() on the returned object if it is not null.
     */
    SkTypeface* createFromStream(SkStreamAsset*, int ttcIndex = 0) const;

    // deprecated, use SkFontArguments instead.
    using FontParameters = SkFontArguments;
    /* Experimental, API subject to change. */
    SkTypeface* createFromStream(SkStreamAsset*, const SkFontArguments&) const;

    /**
     *  Create a typeface from the specified font data.
     *  Will return NULL if the typeface could not be created.
     *  The caller must call unref() on the returned object if it is not null.
     */
    SkTypeface* createFromFontData(std::unique_ptr<SkFontData>) const;

    /**
     *  Create a typeface for the specified fileName and TTC index
     *  (pass 0 for none) or NULL if the file is not found, or its contents are
     *  not recognized. The caller must call unref() on the returned object
     *  if it is not null.
     */
    SkTypeface* createFromFile(const char path[], int ttcIndex = 0) const;

    SkTypeface* legacyCreateTypeface(const char familyName[], SkFontStyle style) const;

    /** Return the default fontmgr. */
    static sk_sp<SkFontMgr> RefDefault();

protected:
    virtual int onCountFamilies() const = 0;
    virtual void onGetFamilyName(int index, SkString* familyName) const = 0;
    virtual SkFontStyleSet* onCreateStyleSet(int index)const  = 0;

    /** May return NULL if the name is not found. */
    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) const = 0;

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle&) const = 0;
    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                    const char* bcp47[], int bcp47Count,
                                                    SkUnichar character) const = 0;
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                         const SkFontStyle&) const = 0;

    virtual SkTypeface* onCreateFromData(SkData*, int ttcIndex) const = 0;
    virtual SkTypeface* onCreateFromStream(SkStreamAsset*, int ttcIndex) const = 0;
    // TODO: make pure virtual.
    virtual SkTypeface* onCreateFromStream(SkStreamAsset*, const SkFontArguments&) const;
    virtual SkTypeface* onCreateFromFontData(std::unique_ptr<SkFontData>) const;
    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const = 0;

    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[], SkFontStyle) const = 0;

private:

    /** Implemented by porting layer to return the default factory. */
    static sk_sp<SkFontMgr> Factory();

    typedef SkRefCnt INHERITED;
};

#endif
