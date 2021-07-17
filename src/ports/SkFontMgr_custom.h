/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_custom_DEFINED
#define SkFontMgr_custom_DEFINED

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/ports/SkFontHost_FreeType_common.h"

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

    using INHERITED = SkTypeface_FreeType;
};

/** The empty SkTypeface implementation for the custom font manager.
 *  Used as the last resort fallback typeface.
 */
class SkTypeface_Empty : public SkTypeface_Custom {
public:
    SkTypeface_Empty() ;

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
    using INHERITED = SkTypeface_Custom;
};

/** The stream SkTypeface implementation for the custom font manager. */
class SkTypeface_Stream : public SkTypeface_Custom {
public:
    SkTypeface_Stream(std::unique_ptr<SkFontData> fontData,
                      const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                      const SkString familyName);

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;

private:
    const std::unique_ptr<const SkFontData> fData;

    using INHERITED = SkTypeface_Custom;
};

/** The file SkTypeface implementation for the custom font manager. */
class SkTypeface_File : public SkTypeface_Custom {
public:
    SkTypeface_File(const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                    const SkString familyName, const char path[], int index);

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
    SkString fPath;

    using INHERITED = SkTypeface_Custom;
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

    /** Should only be called during the initial build phase. */
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
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>, const SkFontArguments&) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override;

private:
    Families fFamilies;
    SkFontStyleSet_Custom* fDefaultFamily;
    SkTypeface_FreeType::Scanner fScanner;
};

#endif
