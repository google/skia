/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontConfigTypeface.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkMath.h"
#include "SkMutex.h"
#include "SkString.h"
#include "SkTDArray.h"

// for now we pull these in directly. eventually we will solely rely on the
// SkFontConfigInterface instance.
#include <fontconfig/fontconfig.h>
#include <unistd.h>

namespace {

// Fontconfig is not threadsafe before 2.10.91.  Before that, we lock with a global mutex.
// See skia:1497 for background.
SK_DECLARE_STATIC_MUTEX(gFCMutex);
static bool gFCSafeToUse;

struct FCLocker {
    FCLocker() {
        if (FcGetVersion() < 21091) {  // We assume FcGetVersion() has always been thread safe.
            gFCMutex.acquire();
            fUnlock = true;
        } else {
            fUnlock = false;
        }
        gFCSafeToUse = true;
    }

    ~FCLocker() {
        if (fUnlock) {
            gFCSafeToUse = false;
            gFCMutex.release();
        }
    }

private:
    bool fUnlock;
};

} // namespace

// borrow this global from SkFontHost_fontconfig. eventually that file should
// go away, and be replaced with this one.
extern SkFontConfigInterface* SkFontHost_fontconfig_ref_global();
static SkFontConfigInterface* RefFCI() {
    return SkFontHost_fontconfig_ref_global();
}

// look for the last substring after a '/' and return that, or return null.
static const char* find_just_name(const char* str) {
    const char* last = strrchr(str, '/');
    return last ? last + 1 : NULL;
}

static bool is_lower(char c) {
    return c >= 'a' && c <= 'z';
}

static int get_int(FcPattern* pattern, const char field[]) {
    SkASSERT(gFCSafeToUse);
    int value;
    if (FcPatternGetInteger(pattern, field, 0, &value) != FcResultMatch) {
        value = SK_MinS32;
    }
    return value;
}

static const char* get_name(FcPattern* pattern, const char field[]) {
    SkASSERT(gFCSafeToUse);
    const char* name;
    if (FcPatternGetString(pattern, field, 0, (FcChar8**)&name) != FcResultMatch) {
        name = "";
    }
    return name;
}

static bool valid_pattern(FcPattern* pattern) {
    SkASSERT(gFCSafeToUse);
    FcBool is_scalable;
    if (FcPatternGetBool(pattern, FC_SCALABLE, 0, &is_scalable) != FcResultMatch || !is_scalable) {
        return false;
    }

    // fontconfig can also return fonts which are unreadable
    const char* c_filename = get_name(pattern, FC_FILE);
    if (0 == *c_filename) {
        return false;
    }
    if (access(c_filename, R_OK) != 0) {
        return false;
    }
    return true;
}

static bool match_name(FcPattern* pattern, const char family_name[]) {
    return !strcasecmp(family_name, get_name(pattern, FC_FAMILY));
}

static FcPattern** MatchFont(FcFontSet* font_set,
                             const char post_config_family[],
                             int* count) {
  // Older versions of fontconfig have a bug where they cannot select
  // only scalable fonts so we have to manually filter the results.

    FcPattern** iter = font_set->fonts;
    FcPattern** stop = iter + font_set->nfont;
    // find the first good match
    for (; iter < stop; ++iter) {
        if (valid_pattern(*iter)) {
            break;
        }
    }

    if (iter == stop || !match_name(*iter, post_config_family)) {
        return NULL;
    }

    FcPattern** firstIter = iter++;
    for (; iter < stop; ++iter) {
        if (!valid_pattern(*iter) || !match_name(*iter, post_config_family)) {
            break;
        }
    }

    *count = iter - firstIter;
    return firstIter;
}

class SkFontStyleSet_FC : public SkFontStyleSet {
public:
    SkFontStyleSet_FC(FcPattern** matches, int count);
    virtual ~SkFontStyleSet_FC();

    int count() override { return fRecCount; }
    void getStyle(int index, SkFontStyle*, SkString* style) override;
    SkTypeface* createTypeface(int index) override;
    SkTypeface* matchStyle(const SkFontStyle& pattern) override;

private:
    struct Rec {
        SkString    fStyleName;
        SkString    fFileName;
        SkFontStyle fStyle;
    };
    Rec* fRecs;
    int  fRecCount;
};

static int map_range(int value,
                     int old_min, int old_max, int new_min, int new_max) {
    SkASSERT(old_min < old_max);
    SkASSERT(new_min < new_max);
    return new_min + SkMulDiv(value - old_min,
                              new_max - new_min, old_max - old_min);
}

static SkFontStyle make_fontconfig_style(FcPattern* match) {
    int weight = get_int(match, FC_WEIGHT);
    int width = get_int(match, FC_WIDTH);
    int slant = get_int(match, FC_SLANT);
//    SkDebugf("old weight %d new weight %d\n", weight, map_range(weight, 0, 80, 0, 400));

    // fontconfig weight seems to be 0..200 or so, so we remap it here
    weight = map_range(weight, 0, 80, 0, 400);
    width = map_range(width, 0, 200, 0, 9);
    return SkFontStyle(weight, width, slant > 0 ? SkFontStyle::kItalic_Slant
                                                : SkFontStyle::kUpright_Slant);
}

SkFontStyleSet_FC::SkFontStyleSet_FC(FcPattern** matches, int count) {
    fRecCount = count;
    fRecs = SkNEW_ARRAY(Rec, count);
    for (int i = 0; i < count; ++i) {
        fRecs[i].fStyleName.set(get_name(matches[i], FC_STYLE));
        fRecs[i].fFileName.set(get_name(matches[i], FC_FILE));
        fRecs[i].fStyle = make_fontconfig_style(matches[i]);
    }
}

SkFontStyleSet_FC::~SkFontStyleSet_FC() {
    SkDELETE_ARRAY(fRecs);
}

void SkFontStyleSet_FC::getStyle(int index, SkFontStyle* style,
                                 SkString* styleName) {
    SkASSERT((unsigned)index < (unsigned)fRecCount);
    if (style) {
        *style = fRecs[index].fStyle;
    }
    if (styleName) {
        *styleName = fRecs[index].fStyleName;
    }
}

SkTypeface* SkFontStyleSet_FC::createTypeface(int index) {
    return NULL;
}

SkTypeface* SkFontStyleSet_FC::matchStyle(const SkFontStyle& pattern) {
    return NULL;
}

class SkFontMgr_fontconfig : public SkFontMgr {
    SkAutoTUnref<SkFontConfigInterface> fFCI;
    SkDataTable* fFamilyNames;
    SkTypeface_FreeType::Scanner fScanner;

public:
    SkFontMgr_fontconfig(SkFontConfigInterface* fci)
        : fFCI(fci)
        , fFamilyNames(fFCI->getFamilyNames()) {}

    virtual ~SkFontMgr_fontconfig() {
        SkSafeUnref(fFamilyNames);
    }

protected:
    int onCountFamilies() const override {
        return fFamilyNames->count();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        familyName->set(fFamilyNames->atStr(index));
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        return this->onMatchFamily(fFamilyNames->atStr(index));
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        FCLocker lock;

        FcPattern* pattern = FcPatternCreate();

        FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)familyName);
#if 0
        FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
#endif
        FcConfigSubstitute(NULL, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        const char* post_config_family = get_name(pattern, FC_FAMILY);

        FcResult result;
        FcFontSet* font_set = FcFontSort(0, pattern, 0, 0, &result);
        if (!font_set) {
            FcPatternDestroy(pattern);
            return NULL;
        }

        int count;
        FcPattern** match = MatchFont(font_set, post_config_family, &count);
        if (!match) {
            FcPatternDestroy(pattern);
            FcFontSetDestroy(font_set);
            return NULL;
        }

        FcPatternDestroy(pattern);

        SkTDArray<FcPattern*> trimmedMatches;
        for (int i = 0; i < count; ++i) {
            const char* justName = find_just_name(get_name(match[i], FC_FILE));
            if (!is_lower(*justName)) {
                *trimmedMatches.append() = match[i];
            }
        }

        SkFontStyleSet_FC* sset = SkNEW_ARGS(SkFontStyleSet_FC,
                                             (trimmedMatches.begin(),
                                              trimmedMatches.count()));
        return sset;
    }

    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle&) const override { return NULL; }
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override {
        return NULL;
    }
    SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                 const SkFontStyle&) const override { return NULL; }

    SkTypeface* onCreateFromData(SkData*, int ttcIndex) const override { return NULL; }

    SkTypeface* onCreateFromStream(SkStreamAsset* bareStream, int ttcIndex) const override {
        SkAutoTDelete<SkStreamAsset> stream(bareStream);
        const size_t length = stream->getLength();
        if (!length) {
            return NULL;
        }
        if (length >= 1024 * 1024 * 1024) {
            return NULL;  // don't accept too large fonts (>= 1GB) for safety.
        }

        // TODO should the caller give us the style or should we get it from freetype?
        SkFontStyle style;
        bool isFixedWidth = false;
        if (!fScanner.scanFont(stream, 0, NULL, &style, &isFixedWidth, NULL)) {
            return NULL;
        }

        SkTypeface* face = FontConfigTypeface::Create(style, isFixedWidth, stream.detach());
        return face;
    }

    SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const override {
        SkAutoTDelete<SkStreamAsset> stream(SkStream::NewFromFile(path));
        return stream.get() ? this->createFromStream(stream.detach(), ttcIndex) : NULL;
    }

    SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                       unsigned styleBits) const override {
        FCLocker lock;
        return FontConfigTypeface::LegacyCreateTypeface(familyName, (SkTypeface::Style)styleBits);
    }
};

SkFontMgr* SkFontMgr::Factory() {
    SkFontConfigInterface* fci = RefFCI();
    return fci ? SkNEW_ARGS(SkFontMgr_fontconfig, (fci)) : NULL;
}
