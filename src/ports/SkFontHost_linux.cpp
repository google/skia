/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontHost.h"
#include "SkFontHost_FreeType_common.h"
#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkDescriptor.h"
#include "SkOSFile.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTSearch.h"
#include "SkTypefaceCache.h"
#include "SkTArray.h"

#include <limits>

#ifndef SK_FONT_FILE_PREFIX
#    define SK_FONT_FILE_PREFIX "/usr/share/fonts/truetype/"
#endif

///////////////////////////////////////////////////////////////////////////////

/** The base SkTypeface implementation for the custom font manager. */
class SkTypeface_Custom : public SkTypeface_FreeType {
public:
    SkTypeface_Custom(Style style, bool isFixedPitch,
                      bool sysFont, const SkString familyName, int index)
        : INHERITED(style, SkTypefaceCache::NewFontID(), isFixedPitch)
        , fIsSysFont(sysFont), fFamilyName(familyName), fIndex(index)
    { }

    bool isSysFont() const { return fIsSysFont; }

    virtual const char* getUniqueString() const = 0;

protected:
    virtual void onGetFamilyName(SkString* familyName) const SK_OVERRIDE {
        *familyName = fFamilyName;
    }

    virtual void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const SK_OVERRIDE {
        desc->setFamilyName(fFamilyName.c_str());
        desc->setFontFileName(this->getUniqueString());
        desc->setFontIndex(fIndex);
        *isLocal = !this->isSysFont();
    }

    int getIndex() const { return fIndex; }

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
    SkTypeface_Empty() : INHERITED(SkTypeface::kNormal, false, true, SkString(), 0) {}

    virtual const char* getUniqueString() const SK_OVERRIDE { return NULL; }

protected:
    virtual SkStream* onOpenStream(int*) const SK_OVERRIDE { return NULL; }

private:
    typedef SkTypeface_Custom INHERITED;
};

/** The stream SkTypeface implementation for the custom font manager. */
class SkTypeface_Stream : public SkTypeface_Custom {
public:
    SkTypeface_Stream(Style style, bool isFixedPitch, bool sysFont, const SkString familyName,
                      SkStream* stream, int ttcIndex)
        : INHERITED(style, isFixedPitch, sysFont, familyName, ttcIndex)
        , fStream(SkRef(stream))
    { }

    virtual const char* getUniqueString() const SK_OVERRIDE { return NULL; }

protected:
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = this->getIndex();
        return fStream->duplicate();
    }

private:
    const SkAutoTUnref<const SkStream> fStream;

    typedef SkTypeface_Custom INHERITED;
};

/** The file SkTypeface implementation for the custom font manager. */
class SkTypeface_File : public SkTypeface_Custom {
public:
    SkTypeface_File(Style style, bool isFixedPitch, bool sysFont, const SkString familyName,
                    const char path[], int index)
        : INHERITED(style, isFixedPitch, sysFont, familyName, index)
        , fPath(path)
    { }

    virtual const char* getUniqueString() const SK_OVERRIDE {
        const char* str = strrchr(fPath.c_str(), '/');
        if (str) {
            str += 1;   // skip the '/'
        }
        return str;
    }

protected:
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = this->getIndex();
        return SkStream::NewFromFile(fPath.c_str());
    }

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
    explicit SkFontStyleSet_Custom(const SkString familyName) : fFamilyName(familyName) { }

    virtual int count() SK_OVERRIDE {
        return fStyles.count();
    }

    virtual void getStyle(int index, SkFontStyle* style, SkString* name) SK_OVERRIDE {
        SkASSERT(index < fStyles.count());
        bool bold = fStyles[index]->isBold();
        bool italic = fStyles[index]->isItalic();
        *style = SkFontStyle(bold ? SkFontStyle::kBold_Weight : SkFontStyle::kNormal_Weight,
                             SkFontStyle::kNormal_Width,
                             italic ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant);
        name->reset();
    }

    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE {
        SkASSERT(index < fStyles.count());
        return SkRef(fStyles[index].get());
    }

    static int match_score(const SkFontStyle& pattern, const SkFontStyle& candidate) {
        int score = 0;
        score += (pattern.width() - candidate.width()) * 100;
        score += (pattern.isItalic() == candidate.isItalic()) ? 0 : 1000;
        score += pattern.weight() - candidate.weight();
        return score;
    }

    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) SK_OVERRIDE {
        if (0 == fStyles.count()) {
            return NULL;
        }

        SkTypeface_Custom* closest = fStyles[0];
        int minScore = std::numeric_limits<int>::max();
        for (int i = 0; i < fStyles.count(); ++i) {
            bool bold = fStyles[i]->isBold();
            bool italic = fStyles[i]->isItalic();
            SkFontStyle style = SkFontStyle(bold ? SkFontStyle::kBold_Weight
                                                 : SkFontStyle::kNormal_Weight,
                                            SkFontStyle::kNormal_Width,
                                            italic ? SkFontStyle::kItalic_Slant
                                                   : SkFontStyle::kUpright_Slant);

            int score = match_score(pattern, style);
            if (score < minScore) {
                closest = fStyles[i];
                minScore = score;
            }
        }
        return SkRef(closest);
    }

private:
    SkTArray<SkAutoTUnref<SkTypeface_Custom>, true> fStyles;
    SkString fFamilyName;

    void appendTypeface(SkTypeface_Custom* typeface) {
        fStyles.push_back().reset(typeface);
    }

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
    explicit SkFontMgr_Custom(const char* dir) {
        this->load_system_fonts(dir);
    }

protected:
    virtual int onCountFamilies() const SK_OVERRIDE {
        return fFamilies.count();
    }

    virtual void onGetFamilyName(int index, SkString* familyName) const SK_OVERRIDE {
        SkASSERT(index < fFamilies.count());
        familyName->set(fFamilies[index]->fFamilyName);
    }

    virtual SkFontStyleSet_Custom* onCreateStyleSet(int index) const SK_OVERRIDE {
        SkASSERT(index < fFamilies.count());
        return SkRef(fFamilies[index].get());
    }

    virtual SkFontStyleSet_Custom* onMatchFamily(const char familyName[]) const SK_OVERRIDE {
        for (int i = 0; i < fFamilies.count(); ++i) {
            if (fFamilies[i]->fFamilyName.equals(familyName)) {
                return SkRef(fFamilies[i].get());
            }
        }
        return NULL;
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& fontStyle) const SK_OVERRIDE
    {
        SkAutoTUnref<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(fontStyle);
    }

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                         const SkFontStyle& fontStyle) const SK_OVERRIDE
    {
        for (int i = 0; i < fFamilies.count(); ++i) {
            for (int j = 0; j < fFamilies[i]->fStyles.count(); ++j) {
                if (fFamilies[i]->fStyles[j] == familyMember) {
                    return fFamilies[i]->matchStyle(fontStyle);
                }
            }
        }
        return NULL;
    }

    virtual SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const SK_OVERRIDE {
        SkAutoTUnref<SkStream> stream(new SkMemoryStream(data));
        return this->createFromStream(stream, ttcIndex);
    }

    virtual SkTypeface* onCreateFromStream(SkStream* stream, int ttcIndex) const SK_OVERRIDE {
        if (NULL == stream || stream->getLength() <= 0) {
            SkDELETE(stream);
            return NULL;
        }

        bool isFixedPitch;
        SkTypeface::Style style;
        SkString name;
        if (SkTypeface_FreeType::ScanFont(stream, ttcIndex, &name, &style, &isFixedPitch)) {
            return SkNEW_ARGS(SkTypeface_Stream, (style, isFixedPitch, false, name,
                                                  stream, ttcIndex));
        } else {
            return NULL;
        }
    }

    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const SK_OVERRIDE {
        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
        return stream.get() ? this->createFromStream(stream, ttcIndex) : NULL;
    }

    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                               unsigned styleBits) const SK_OVERRIDE
    {
        SkTypeface::Style oldStyle = (SkTypeface::Style)styleBits;
        SkFontStyle style = SkFontStyle(oldStyle & SkTypeface::kBold
                                                 ? SkFontStyle::kBold_Weight
                                                 : SkFontStyle::kNormal_Weight,
                                        SkFontStyle::kNormal_Width,
                                        oldStyle & SkTypeface::kItalic
                                                 ? SkFontStyle::kItalic_Slant
                                                 : SkFontStyle::kUpright_Slant);
        SkTypeface* tf = NULL;

        if (familyName) {
            tf = this->onMatchFamilyStyle(familyName, style);
        }

        if (NULL == tf) {
            tf = gDefaultFamily->matchStyle(style);
        }

        return SkSafeRef(tf);
    }

private:

    static bool get_name_and_style(const char path[], SkString* name,
                                   SkTypeface::Style* style, bool* isFixedPitch) {
        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
        if (stream.get()) {
            return SkTypeface_FreeType::ScanFont(stream, 0, name, style, isFixedPitch);
        } else {
            SkDebugf("---- failed to open <%s> as a font\n", path);
            return false;
        }
    }

    void load_directory_fonts(const SkString& directory) {
        SkOSFile::Iter iter(directory.c_str(), ".ttf");
        SkString name;

        while (iter.next(&name, false)) {
            SkString filename(
                SkOSPath::Join(directory.c_str(), name.c_str()));

            bool isFixedPitch;
            SkString realname;
            SkTypeface::Style style = SkTypeface::kNormal; // avoid uninitialized warning

            if (!get_name_and_style(filename.c_str(), &realname, &style, &isFixedPitch)) {
                SkDebugf("------ can't load <%s> as a font\n", filename.c_str());
                continue;
            }

            SkTypeface_Custom* tf = SkNEW_ARGS(SkTypeface_File, (
                                                style,
                                                isFixedPitch,
                                                true,  // system-font (cannot delete)
                                                realname,
                                                filename.c_str(), 0));

            SkFontStyleSet_Custom* addTo = this->onMatchFamily(realname.c_str());
            if (NULL == addTo) {
                addTo = new SkFontStyleSet_Custom(realname);
                fFamilies.push_back().reset(addTo);
            }
            addTo->appendTypeface(tf);
        }

        SkOSFile::Iter dirIter(directory.c_str());
        while (dirIter.next(&name, true)) {
            if (name.startsWith(".")) {
                continue;
            }
            SkString dirname(SkOSPath::Join(directory.c_str(), name.c_str()));
            load_directory_fonts(dirname);
        }
    }

    void load_system_fonts(const char* dir) {
        SkString baseDirectory(dir);
        load_directory_fonts(baseDirectory);

        if (fFamilies.empty()) {
            SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
            fFamilies.push_back().reset(family);
            family->appendTypeface(SkNEW(SkTypeface_Empty));
        }

        // Try to pick a default font.
        static const char* gDefaultNames[] = {
            "Arial", "Verdana", "Times New Roman", "Droid Sans", NULL
        };
        for (size_t i = 0; i < SK_ARRAY_COUNT(gDefaultNames); ++i) {
            SkFontStyleSet_Custom* set = this->onMatchFamily(gDefaultNames[i]);
            if (NULL == set) {
                continue;
            }

            SkTypeface* tf = set->matchStyle(SkFontStyle(SkFontStyle::kNormal_Weight,
                                                         SkFontStyle::kNormal_Width,
                                                         SkFontStyle::kUpright_Slant));
            if (NULL == tf) {
                continue;
            }

            gDefaultFamily = set;
            gDefaultNormal = tf;
            break;
        }
        if (NULL == gDefaultNormal) {
            gDefaultFamily = fFamilies[0];
            gDefaultNormal = gDefaultFamily->fStyles[0];
        }
    }

    SkTArray<SkAutoTUnref<SkFontStyleSet_Custom>, true> fFamilies;
    SkFontStyleSet_Custom* gDefaultFamily;
    SkTypeface* gDefaultNormal;
};

SkFontMgr* SkFontMgr::Factory() {
    return new SkFontMgr_Custom(SK_FONT_FILE_PREFIX);
}
