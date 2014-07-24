/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigParser_android.h"
#include "SkFontDescriptor.h"
#include "SkFontHost_FreeType_common.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTSearch.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"

#include <limits>
#include <stdlib.h>

#ifndef SK_FONT_FILE_PREFIX
#    define SK_FONT_FILE_PREFIX "/fonts/"
#endif

#ifndef SK_DEBUG_FONTS
    #define SK_DEBUG_FONTS 0
#endif

#if SK_DEBUG_FONTS
#    define DEBUG_FONT(args) SkDebugf args
#else
#    define DEBUG_FONT(args)
#endif

class SkTypeface_Android : public SkTypeface_FreeType {
public:
    SkTypeface_Android(int index,
                       Style style,
                       bool isFixedPitch,
                       const SkString familyName)
        : INHERITED(style, SkTypefaceCache::NewFontID(), isFixedPitch)
        , fIndex(index)
        , fFamilyName(familyName)
    { }

    const SkString& name() const { return fFamilyName; }

protected:
    int fIndex;
    SkString fFamilyName;

private:
    typedef SkTypeface_FreeType INHERITED;
};

class SkTypeface_AndroidSystem : public SkTypeface_Android {
public:
    SkTypeface_AndroidSystem(const SkString pathName,
                             int index,
                             Style style,
                             bool isFixedPitch,
                             const SkString familyName)
        : INHERITED(index, style, isFixedPitch, familyName)
        , fPathName(pathName)
    { }

    virtual void onGetFontDescriptor(SkFontDescriptor* desc,
                                     bool* serialize) const SK_OVERRIDE
    {
        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setFontFileName(fPathName.c_str());
        *serialize = false;
    }
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = fIndex;
        return SkStream::NewFromFile(fPathName.c_str());
    }

private:
    SkString fPathName;

    typedef SkTypeface_Android INHERITED;
};

class SkTypeface_AndroidStream : public SkTypeface_Android {
public:
    SkTypeface_AndroidStream(SkStream* stream,
                             int index,
                             Style style,
                             bool isFixedPitch,
                             const SkString familyName)
        : INHERITED(index, style, isFixedPitch, familyName)
        , fStream(stream)
    { }

    virtual void onGetFontDescriptor(SkFontDescriptor* desc,
                                     bool* serialize) const SK_OVERRIDE {
        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setFontFileName(NULL);
        *serialize = true;
    }

    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = fIndex;
        return fStream->duplicate();
    }

private:
    SkAutoTUnref<SkStream> fStream;

    typedef SkTypeface_Android INHERITED;
};

void get_path_for_sys_fonts(SkString* full, const SkString& name) {
    full->set(getenv("ANDROID_ROOT"));
    full->append(SK_FONT_FILE_PREFIX);
    full->append(name);
}

class SkFontStyleSet_Android : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_Android(FontFamily* family) : fFontFamily(family) {
        // TODO? make this lazy
        for (int i = 0; i < family->fFontFiles.count(); ++i) {
            const SkString& fileName = family->fFontFiles[i].fFileName;

            SkString pathName;
            get_path_for_sys_fonts(&pathName, fileName);

            SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(pathName.c_str()));
            if (!stream.get()) {
                DEBUG_FONT(("---- SystemFonts[%d] file=%s (NOT EXIST)", i, filename.c_str()));
                continue;
            }

            SkString fontName;
            SkTypeface::Style style;
            bool isFixedWidth;
            if (!SkTypeface_FreeType::ScanFont(stream.get(), family->fFontFiles[i].fIndex,
                                               &fontName, &style, &isFixedWidth))
            {
                DEBUG_FONT(("---- SystemFonts[%d] file=%s (INVALID)", i, filename.c_str()));
                continue;
            }

            fStyles.push_back().reset(SkNEW_ARGS(SkTypeface_AndroidSystem,
                                                 (pathName, 0,
                                                  style, isFixedWidth, fontName)));
        }
    }

    virtual int count() SK_OVERRIDE {
        return fStyles.count();
    }
    virtual void getStyle(int index, SkFontStyle* style, SkString* name) SK_OVERRIDE {
        if (index < 0 || fStyles.count() <= index) {
            return;
        }
        if (style) {
            *style = this->style(index);
        }
        if (name) {
            name->reset();
        }
    }
    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE {
        if (index < 0 || fStyles.count() <= index) {
            return NULL;
        }
        return SkRef(fStyles[index].get());
    }

    /** Find the typeface in this style set that most closely matches the given pattern.
     *  TODO: consider replacing with SkStyleSet_Indirect::matchStyle();
     *  this simpler version using match_score() passes all our tests.
     */
    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) SK_OVERRIDE {
        if (0 == fStyles.count()) {
            return NULL;
        }
        SkTypeface* closest = fStyles[0];
        int minScore = std::numeric_limits<int>::max();
        for (int i = 0; i < fStyles.count(); ++i) {
            SkFontStyle style = this->style(i);
            int score = match_score(pattern, style);
            if (score < minScore) {
                closest = fStyles[i];
                minScore = score;
            }
        }
        return SkRef(closest);
    }

private:
    SkFontStyle style(int index) {
        return SkFontStyle(this->weight(index), SkFontStyle::kNormal_Width,
                           this->slant(index));
    }
    SkFontStyle::Weight weight(int index) {
        if (fStyles[index]->isBold()) return SkFontStyle::kBold_Weight;
        return SkFontStyle::kNormal_Weight;
    }
    SkFontStyle::Slant slant(int index) {
        if (fStyles[index]->isItalic()) return SkFontStyle::kItalic_Slant;
        return SkFontStyle::kUpright_Slant;
    }
    static int match_score(const SkFontStyle& pattern, const SkFontStyle& candidate) {
        int score = 0;
        score += abs((pattern.width() - candidate.width()) * 100);
        score += abs((pattern.isItalic() == candidate.isItalic()) ? 0 : 1000);
        score += abs(pattern.weight() - candidate.weight());
        return score;
    }


    FontFamily* fFontFamily;
    SkTArray<SkAutoTUnref<SkTypeface>, true> fStyles;

    friend struct NameToFamily;
    friend class SkFontMgr_Android;

    typedef SkFontStyleSet INHERITED;
};

/** On Android a single family can have many names, but our API assumes unique names.
 *  Map names to the back end so that all names for a given family refer to the same
 *  (non-replicated) set of typefaces.
 *  SkTDict<> doesn't let us do index-based lookup, so we write our own mapping.
 */
struct NameToFamily {
    SkString name;
    SkFontStyleSet_Android* styleSet;
};

class SkFontMgr_Android : public SkFontMgr {
public:
    SkFontMgr_Android() {
        SkTDArray<FontFamily*> fontFamilies;
        SkFontConfigParser::GetFontFamilies(fontFamilies);
        this->buildNameToFamilyMap(fontFamilies);
        this->findDefaultFont();
    }

protected:
    /** Returns not how many families we have, but how many unique names
     *  exist among the families.
     */
    virtual int onCountFamilies() const SK_OVERRIDE {
        return fNameToFamilyMap.count();
    }

    virtual void onGetFamilyName(int index, SkString* familyName) const SK_OVERRIDE {
        if (index < 0 || fNameToFamilyMap.count() <= index) {
            familyName->reset();
            return;
        }
        familyName->set(fNameToFamilyMap[index].name);
    }

    virtual SkFontStyleSet* onCreateStyleSet(int index) const SK_OVERRIDE {
        if (index < 0 || fNameToFamilyMap.count() <= index) {
            return NULL;
        }
        return SkRef(fNameToFamilyMap[index].styleSet);
    }

    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) const SK_OVERRIDE {
        if (!familyName) {
            return NULL;
        }
        SkAutoAsciiToLC tolc(familyName);
        for (int i = 0; i < fNameToFamilyMap.count(); ++i) {
            if (fNameToFamilyMap[i].name.equals(tolc.lc())) {
                return SkRef(fNameToFamilyMap[i].styleSet);
            }
        }
        return NULL;
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& style) const SK_OVERRIDE {
        SkAutoTUnref<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(style);
    }

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* typeface,
                                         const SkFontStyle& style) const SK_OVERRIDE {
        for (int i = 0; i < fFontStyleSets.count(); ++i) {
            for (int j = 0; j < fFontStyleSets[i]->fStyles.count(); ++j) {
                if (fFontStyleSets[i]->fStyles[j] == typeface) {
                    return fFontStyleSets[i]->matchStyle(style);
                }
            }
        }
        return NULL;
    }

    virtual SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const SK_OVERRIDE {
        SkAutoTUnref<SkStream> stream(new SkMemoryStream(data));
        return this->createFromStream(stream, ttcIndex);
    }

    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const SK_OVERRIDE {
        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
        return stream.get() ? this->createFromStream(stream, ttcIndex) : NULL;
    }

    virtual SkTypeface* onCreateFromStream(SkStream* s, int ttcIndex) const SK_OVERRIDE {
        SkAutoTUnref<SkStream> stream(s);

        bool isFixedPitch;
        SkTypeface::Style style;
        SkString name;
        if (!SkTypeface_FreeType::ScanFont(stream, ttcIndex, &name, &style, &isFixedPitch)) {
            return NULL;
        }
        return SkNEW_ARGS(SkTypeface_AndroidStream, (stream.detach(), ttcIndex,
                                                     style, isFixedPitch, name));
    }


    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                               unsigned styleBits) const SK_OVERRIDE {
        SkTypeface::Style oldStyle = (SkTypeface::Style)styleBits;
        SkFontStyle style = SkFontStyle(oldStyle & SkTypeface::kBold
                                                 ? SkFontStyle::kBold_Weight
                                                 : SkFontStyle::kNormal_Weight,
                                        SkFontStyle::kNormal_Width,
                                        oldStyle & SkTypeface::kItalic
                                                 ? SkFontStyle::kItalic_Slant
                                                 : SkFontStyle::kUpright_Slant);
        SkTypeface* tf = NULL;

        if (NULL != familyName) {
            // On Android, we must return NULL when we can't find the requested
            // named typeface so that the system/app can provide their own recovery
            // mechanism. On other platforms we'd provide a typeface from the
            // default family instead.
            tf = this->onMatchFamilyStyle(familyName, style);
        } else {
            tf = fDefaultFamily->matchStyle(style);
        }

        // TODO: double ref? qv matchStyle()
        return SkSafeRef(tf);
    }


private:

    SkTArray<SkAutoTUnref<SkFontStyleSet_Android>, true> fFontStyleSets;
    SkFontStyleSet* fDefaultFamily;
    SkTypeface* fDefaultTypeface;

    SkTDArray<NameToFamily> fNameToFamilyMap;

    void buildNameToFamilyMap(SkTDArray<FontFamily*> families) {
        for (int i = 0; i < families.count(); i++) {
            fFontStyleSets.push_back().reset(
                SkNEW_ARGS(SkFontStyleSet_Android, (families[i])));
            for (int j = 0; j < families[i]->fNames.count(); j++) {
                NameToFamily* nextEntry = fNameToFamilyMap.append();
                SkNEW_PLACEMENT_ARGS(&nextEntry->name, SkString, (families[i]->fNames[j]));
                nextEntry->styleSet = fFontStyleSets.back().get();
            }
        }
    }

    void findDefaultFont() {
        SkASSERT(!fFontStyleSets.empty());

        static const char* gDefaultNames[] = { "sans-serif" };
        for (size_t i = 0; i < SK_ARRAY_COUNT(gDefaultNames); ++i) {
            SkFontStyleSet* set = this->onMatchFamily(gDefaultNames[i]);
            if (NULL == set) {
                continue;
            }
            SkTypeface* tf = set->matchStyle(SkFontStyle());
            if (NULL == tf) {
                continue;
            }
            fDefaultFamily = set;
            fDefaultTypeface = tf;
            break;
        }
        if (NULL == fDefaultTypeface) {
            fDefaultFamily = fFontStyleSets[0];
            fDefaultTypeface = fDefaultFamily->createTypeface(0);
        }
        SkASSERT(fDefaultFamily);
        SkASSERT(fDefaultTypeface);
    }

    typedef SkFontMgr INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkFontMgr* SkFontMgr::Factory() {
    return SkNEW(SkFontMgr_Android);
}
