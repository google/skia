/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_custom_DEFINED
#define SkFontMgr_custom_DEFINED

#include "SkFontHost_FreeType_common.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTypes.h"

class SkData;
class SkFontDescriptor;
class SkStreamAsset;
class SkTypeface;

/** The base SkTypeface implementation for the custom font manager. */
class SkTypeface_Custom : public SkTypeface_FreeType {
public:
    SkTypeface_Custom(const SkFontStyle& style, bool isFixedPitch,
                      bool sysFont, const SkString familyName, int index);
    bool isSysFont() const;

protected:
    void onGetFamilyName(SkString* familyName) const override;
    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;
    int getIndex() const;

private:
    const bool fIsSysFont;
    const SkString fFamilyName;
    const int fIndex;

    typedef SkTypeface_FreeType INHERITED;
};

/** The empty SkTypeface implementation for the custom font manager.
 *  Used as the last resort fallback typeface.
 */
class SkTypeface_Empty : public SkTypeface_Custom {
public:
    SkTypeface_Empty() ;

protected:
    SkStreamAsset* onOpenStream(int*) const override;

private:
    typedef SkTypeface_Custom INHERITED;
};

/** The stream SkTypeface implementation for the custom font manager. */
class SkTypeface_Stream : public SkTypeface_Custom {
public:
    SkTypeface_Stream(std::unique_ptr<SkFontData> fontData,
                      const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                      const SkString familyName);

protected:
    SkStreamAsset* onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
    const std::unique_ptr<const SkFontData> fData;

    typedef SkTypeface_Custom INHERITED;
};

/** The file SkTypeface implementation for the custom font manager. */
class SkTypeface_File : public SkTypeface_Custom {
public:
    SkTypeface_File(const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                    const SkString familyName, const char path[], int index);

protected:
    SkStreamAsset* onOpenStream(int* ttcIndex) const override;

private:
    SkString fPath;

    typedef SkTypeface_Custom INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

/**
 *  SkFontStyleSet_Custom
 *
 *  This class is used by SkFontMgr_Custom to hold SkTypeface_Custom families.
 */
class SkFontStyleSet_Custom : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_Custom(const SkString familyName);

    /** Should only be called during the inital build phase. */
    void appendTypeface(sk_sp<SkTypeface_Custom> typeface);
    int count() override;
    void getStyle(int index, SkFontStyle* style, SkString* name) override;
    SkTypeface* createTypeface(int index) override;
    SkTypeface* matchStyle(const SkFontStyle& pattern) override;
    SkString getFamilyName();

private:
    SkTArray<sk_sp<SkTypeface_Custom>> fStyles;
    SkString fFamilyName;

    friend class SkFontMgr_Custom;
};

/**
 *  SkFontMgr_Custom
 *
 *  This class is essentially a collection of SkFontStyleSet_Custom,
 *  one SkFontStyleSet_Custom for each family. This class may be modified
 *  to load fonts from any source by changing the initialization.
 */
class SkFontMgr_Custom : public SkFontMgr {
public:
    typedef SkTArray<sk_sp<SkFontStyleSet_Custom>> Families;
    class SystemFontLoader {
    public:
        virtual ~SystemFontLoader() { }
        virtual void loadSystemFonts(const SkTypeface_FreeType::Scanner&, Families*) const = 0;
    };
    explicit SkFontMgr_Custom(const SystemFontLoader& loader);

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    SkFontStyleSet_Custom* onCreateStyleSet(int index) const override;
    SkFontStyleSet_Custom* onMatchFamily(const char familyName[]) const override;
    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle& fontStyle) const override;
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override;
    SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                 const SkFontStyle& fontStyle) const override;
    SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const override;
    SkTypeface* onCreateFromStream(SkStreamAsset* bareStream, int ttcIndex) const override;
    SkTypeface* onCreateFromStream(SkStreamAsset* s, const SkFontArguments& args) const override;
    SkTypeface* onCreateFromFontData(std::unique_ptr<SkFontData> data) const override;
    SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const override;
    SkTypeface* onLegacyCreateTypeface(const char familyName[], SkFontStyle style) const override;

private:
    Families fFamilies;
    SkFontStyleSet_Custom* fDefaultFamily;
    SkTypeface_FreeType::Scanner fScanner;
};

#endif
