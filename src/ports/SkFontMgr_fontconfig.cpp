/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkDataTable.h"
#include "SkFixed.h"
#include "SkFontDescriptor.h"
#include "SkFontHost_FreeType_common.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkMakeUnique.h"
#include "SkMath.h"
#include "SkMutex.h"
#include "SkOSFile.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTemplates.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "SkTypes.h"

#include <fontconfig/fontconfig.h>
#include <string.h>

class SkData;

// FC_POSTSCRIPT_NAME was added with b561ff20 which ended up in 2.10.92
// Ubuntu 12.04 is on 2.8.0, 13.10 is on 2.10.93
// Debian 7 is on 2.9.0, 8 is on 2.11
// OpenSUSE 12.2 is on 2.9.0, 12.3 is on 2.10.2, 13.1 2.11.0
// Fedora 19 is on 2.10.93
#ifndef FC_POSTSCRIPT_NAME
#    define FC_POSTSCRIPT_NAME  "postscriptname"
#endif

#ifdef SK_DEBUG
#    include "SkTLS.h"
#endif

/** Since FontConfig is poorly documented, this gives a high level overview:
 *
 *  FcConfig is a handle to a FontConfig configuration instance. Each 'configuration' is independent
 *  from any others which may exist. There exists a default global configuration which is created
 *  and destroyed by FcInit and FcFini, but this default should not normally be used.
 *  Instead, one should use FcConfigCreate and FcInit* to have a named local state.
 *
 *  FcPatterns are {objectName -> [element]} (maps from object names to a list of elements).
 *  Each element is some internal data plus an FcValue which is a variant (a union with a type tag).
 *  Lists of elements are not typed, except by convention. Any collection of FcValues must be
 *  assumed to be heterogeneous by the code, but the code need not do anything particularly
 *  interesting if the values go against convention.
 *
 *  Somewhat like DirectWrite, FontConfig supports synthetics through FC_EMBOLDEN and FC_MATRIX.
 *  Like all synthetic information, such information must be passed with the font data.
 */

namespace {

// Fontconfig is not threadsafe before 2.10.91. Before that, we lock with a global mutex.
// See https://bug.skia.org/1497 for background.
SK_DECLARE_STATIC_MUTEX(gFCMutex);

#ifdef SK_DEBUG
void* CreateThreadFcLocked() { return new bool(false); }
void DeleteThreadFcLocked(void* v) { delete static_cast<bool*>(v); }
#   define THREAD_FC_LOCKED \
        static_cast<bool*>(SkTLS::Get(CreateThreadFcLocked, DeleteThreadFcLocked))
#endif

struct FCLocker {
    // Assume FcGetVersion() has always been thread safe.

    FCLocker() {
        if (FcGetVersion() < 21091) {
            gFCMutex.acquire();
        } else {
            SkDEBUGCODE(bool* threadLocked = THREAD_FC_LOCKED);
            SkASSERT(false == *threadLocked);
            SkDEBUGCODE(*threadLocked = true);
        }
    }

    ~FCLocker() {
        AssertHeld();
        if (FcGetVersion() < 21091) {
            gFCMutex.release();
        } else {
            SkDEBUGCODE(*THREAD_FC_LOCKED = false);
        }
    }

    static void AssertHeld() { SkDEBUGCODE(
        if (FcGetVersion() < 21091) {
            gFCMutex.assertHeld();
        } else {
            SkASSERT(true == *THREAD_FC_LOCKED);
        }
    ) }
};

} // namespace

template<typename T, void (*D)(T*)> void FcTDestroy(T* t) {
    FCLocker::AssertHeld();
    D(t);
}
template <typename T, T* (*C)(), void (*D)(T*)> class SkAutoFc
    : public SkAutoTCallVProc<T, FcTDestroy<T, D> > {
public:
    SkAutoFc() : SkAutoTCallVProc<T, FcTDestroy<T, D> >(C()) {
        T* obj = this->operator T*();
        SkASSERT_RELEASE(nullptr != obj);
    }
    explicit SkAutoFc(T* obj) : SkAutoTCallVProc<T, FcTDestroy<T, D> >(obj) {}
};

typedef SkAutoFc<FcCharSet, FcCharSetCreate, FcCharSetDestroy> SkAutoFcCharSet;
typedef SkAutoFc<FcConfig, FcConfigCreate, FcConfigDestroy> SkAutoFcConfig;
typedef SkAutoFc<FcFontSet, FcFontSetCreate, FcFontSetDestroy> SkAutoFcFontSet;
typedef SkAutoFc<FcLangSet, FcLangSetCreate, FcLangSetDestroy> SkAutoFcLangSet;
typedef SkAutoFc<FcObjectSet, FcObjectSetCreate, FcObjectSetDestroy> SkAutoFcObjectSet;
typedef SkAutoFc<FcPattern, FcPatternCreate, FcPatternDestroy> SkAutoFcPattern;

static bool get_bool(FcPattern* pattern, const char object[], bool missing = false) {
    FcBool value;
    if (FcPatternGetBool(pattern, object, 0, &value) != FcResultMatch) {
        return missing;
    }
    return value;
}

static int get_int(FcPattern* pattern, const char object[], int missing) {
    int value;
    if (FcPatternGetInteger(pattern, object, 0, &value) != FcResultMatch) {
        return missing;
    }
    return value;
}

static const char* get_string(FcPattern* pattern, const char object[], const char* missing = "") {
    FcChar8* value;
    if (FcPatternGetString(pattern, object, 0, &value) != FcResultMatch) {
        return missing;
    }
    return (const char*)value;
}

static const FcMatrix* get_matrix(FcPattern* pattern, const char object[]) {
    FcMatrix* matrix;
    if (FcPatternGetMatrix(pattern, object, 0, &matrix) != FcResultMatch) {
        return nullptr;
    }
    return matrix;
}

enum SkWeakReturn {
    kIsWeak_WeakReturn,
    kIsStrong_WeakReturn,
    kNoId_WeakReturn
};
/** Ideally there  would exist a call like
 *  FcResult FcPatternIsWeak(pattern, object, id, FcBool* isWeak);
 *
 *  However, there is no such call and as of Fc 2.11.0 even FcPatternEquals ignores the weak bit.
 *  Currently, the only reliable way of finding the weak bit is by its effect on matching.
 *  The weak bit only affects the matching of FC_FAMILY and FC_POSTSCRIPT_NAME object values.
 *  A element with the weak bit is scored after FC_LANG, without the weak bit is scored before.
 *  Note that the weak bit is stored on the element, not on the value it holds.
 */
static SkWeakReturn is_weak(FcPattern* pattern, const char object[], int id) {
    FCLocker::AssertHeld();

    FcResult result;

    // Create a copy of the pattern with only the value 'pattern'['object'['id']] in it.
    // Internally, FontConfig pattern objects are linked lists, so faster to remove from head.
    SkAutoFcObjectSet requestedObjectOnly(FcObjectSetBuild(object, nullptr));
    SkAutoFcPattern minimal(FcPatternFilter(pattern, requestedObjectOnly));
    FcBool hasId = true;
    for (int i = 0; hasId && i < id; ++i) {
        hasId = FcPatternRemove(minimal, object, 0);
    }
    if (!hasId) {
        return kNoId_WeakReturn;
    }
    FcValue value;
    result = FcPatternGet(minimal, object, 0, &value);
    if (result != FcResultMatch) {
        return kNoId_WeakReturn;
    }
    while (hasId) {
        hasId = FcPatternRemove(minimal, object, 1);
    }

    // Create a font set with two patterns.
    // 1. the same 'object' as minimal and a lang object with only 'nomatchlang'.
    // 2. a different 'object' from minimal and a lang object with only 'matchlang'.
    SkAutoFcFontSet fontSet;

    SkAutoFcLangSet strongLangSet;
    FcLangSetAdd(strongLangSet, (const FcChar8*)"nomatchlang");
    SkAutoFcPattern strong(FcPatternDuplicate(minimal));
    FcPatternAddLangSet(strong, FC_LANG, strongLangSet);

    SkAutoFcLangSet weakLangSet;
    FcLangSetAdd(weakLangSet, (const FcChar8*)"matchlang");
    SkAutoFcPattern weak;
    FcPatternAddString(weak, object, (const FcChar8*)"nomatchstring");
    FcPatternAddLangSet(weak, FC_LANG, weakLangSet);

    FcFontSetAdd(fontSet, strong.release());
    FcFontSetAdd(fontSet, weak.release());

    // Add 'matchlang' to the copy of the pattern.
    FcPatternAddLangSet(minimal, FC_LANG, weakLangSet);

    // Run a match against the copy of the pattern.
    // If the 'id' was weak, then we should match the pattern with 'matchlang'.
    // If the 'id' was strong, then we should match the pattern with 'nomatchlang'.

    // Note that this config is only used for FcFontRenderPrepare, which we don't even want.
    // However, there appears to be no way to match/sort without it.
    SkAutoFcConfig config;
    FcFontSet* fontSets[1] = { fontSet };
    SkAutoFcPattern match(FcFontSetMatch(config, fontSets, SK_ARRAY_COUNT(fontSets),
                                         minimal, &result));

    FcLangSet* matchLangSet;
    FcPatternGetLangSet(match, FC_LANG, 0, &matchLangSet);
    return FcLangEqual == FcLangSetHasLang(matchLangSet, (const FcChar8*)"matchlang")
                        ? kIsWeak_WeakReturn : kIsStrong_WeakReturn;
}

/** Removes weak elements from either FC_FAMILY or FC_POSTSCRIPT_NAME objects in the property.
 *  This can be quite expensive, and should not be used more than once per font lookup.
 *  This removes all of the weak elements after the last strong element.
 */
static void remove_weak(FcPattern* pattern, const char object[]) {
    FCLocker::AssertHeld();

    SkAutoFcObjectSet requestedObjectOnly(FcObjectSetBuild(object, nullptr));
    SkAutoFcPattern minimal(FcPatternFilter(pattern, requestedObjectOnly));

    int lastStrongId = -1;
    int numIds;
    SkWeakReturn result;
    for (int id = 0; ; ++id) {
        result = is_weak(minimal, object, 0);
        if (kNoId_WeakReturn == result) {
            numIds = id;
            break;
        }
        if (kIsStrong_WeakReturn == result) {
            lastStrongId = id;
        }
        SkAssertResult(FcPatternRemove(minimal, object, 0));
    }

    // If they were all weak, then leave the pattern alone.
    if (lastStrongId < 0) {
        return;
    }

    // Remove everything after the last strong.
    for (int id = lastStrongId + 1; id < numIds; ++id) {
        SkAssertResult(FcPatternRemove(pattern, object, lastStrongId + 1));
    }
}

static int map_range(SkFixed value,
                     SkFixed old_min, SkFixed old_max,
                     SkFixed new_min, SkFixed new_max)
{
    SkASSERT(old_min < old_max);
    SkASSERT(new_min <= new_max);
    return new_min + SkMulDiv(value - old_min, new_max - new_min, old_max - old_min);
}

struct MapRanges {
    SkFixed old_val;
    SkFixed new_val;
};

static SkFixed map_ranges_fixed(SkFixed val, MapRanges const ranges[], int rangesCount) {
    // -Inf to [0]
    if (val < ranges[0].old_val) {
        return ranges[0].new_val;
    }

    // Linear from [i] to [i+1]
    for (int i = 0; i < rangesCount - 1; ++i) {
        if (val < ranges[i+1].old_val) {
            return map_range(val, ranges[i].old_val, ranges[i+1].old_val,
                                  ranges[i].new_val, ranges[i+1].new_val);
        }
    }

    // From [n] to +Inf
    // if (fcweight < Inf)
    return ranges[rangesCount-1].new_val;
}

static int map_ranges(int val, MapRanges const ranges[], int rangesCount) {
    return SkFixedRoundToInt(map_ranges_fixed(SkIntToFixed(val), ranges, rangesCount));
}

template<int n> struct SkTFixed {
    static_assert(-32768 <= n && n <= 32767, "SkTFixed_n_not_in_range");
    static const SkFixed value = static_cast<SkFixed>(n << 16);
};

static SkFontStyle skfontstyle_from_fcpattern(FcPattern* pattern) {
    typedef SkFontStyle SkFS;

    static const MapRanges weightRanges[] = {
        { SkTFixed<FC_WEIGHT_THIN>::value,       SkTFixed<SkFS::kThin_Weight>::value },
        { SkTFixed<FC_WEIGHT_EXTRALIGHT>::value, SkTFixed<SkFS::kExtraLight_Weight>::value },
        { SkTFixed<FC_WEIGHT_LIGHT>::value,      SkTFixed<SkFS::kLight_Weight>::value },
        { SkTFixed<FC_WEIGHT_REGULAR>::value,    SkTFixed<SkFS::kNormal_Weight>::value },
        { SkTFixed<FC_WEIGHT_MEDIUM>::value,     SkTFixed<SkFS::kMedium_Weight>::value },
        { SkTFixed<FC_WEIGHT_DEMIBOLD>::value,   SkTFixed<SkFS::kSemiBold_Weight>::value },
        { SkTFixed<FC_WEIGHT_BOLD>::value,       SkTFixed<SkFS::kBold_Weight>::value },
        { SkTFixed<FC_WEIGHT_EXTRABOLD>::value,  SkTFixed<SkFS::kExtraBold_Weight>::value },
        { SkTFixed<FC_WEIGHT_BLACK>::value,      SkTFixed<SkFS::kBlack_Weight>::value },
        { SkTFixed<FC_WEIGHT_EXTRABLACK>::value, SkTFixed<SkFS::kExtraBlack_Weight>::value },
    };
    int weight = map_ranges(get_int(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR),
                            weightRanges, SK_ARRAY_COUNT(weightRanges));

    static const MapRanges widthRanges[] = {
        { SkTFixed<FC_WIDTH_ULTRACONDENSED>::value, SkTFixed<SkFS::kUltraCondensed_Width>::value },
        { SkTFixed<FC_WIDTH_EXTRACONDENSED>::value, SkTFixed<SkFS::kExtraCondensed_Width>::value },
        { SkTFixed<FC_WIDTH_CONDENSED>::value,      SkTFixed<SkFS::kCondensed_Width>::value },
        { SkTFixed<FC_WIDTH_SEMICONDENSED>::value,  SkTFixed<SkFS::kSemiCondensed_Width>::value },
        { SkTFixed<FC_WIDTH_NORMAL>::value,         SkTFixed<SkFS::kNormal_Width>::value },
        { SkTFixed<FC_WIDTH_SEMIEXPANDED>::value,   SkTFixed<SkFS::kSemiExpanded_Width>::value },
        { SkTFixed<FC_WIDTH_EXPANDED>::value,       SkTFixed<SkFS::kExpanded_Width>::value },
        { SkTFixed<FC_WIDTH_EXTRAEXPANDED>::value,  SkTFixed<SkFS::kExtraExpanded_Width>::value },
        { SkTFixed<FC_WIDTH_ULTRAEXPANDED>::value,  SkTFixed<SkFS::kUltraExpanded_Width>::value },
    };
    int width = map_ranges(get_int(pattern, FC_WIDTH, FC_WIDTH_NORMAL),
                           widthRanges, SK_ARRAY_COUNT(widthRanges));

    SkFS::Slant slant = SkFS::kUpright_Slant;
    switch (get_int(pattern, FC_SLANT, FC_SLANT_ROMAN)) {
        case FC_SLANT_ROMAN:   slant = SkFS::kUpright_Slant; break;
        case FC_SLANT_ITALIC : slant = SkFS::kItalic_Slant ; break;
        case FC_SLANT_OBLIQUE: slant = SkFS::kOblique_Slant; break;
        default: SkASSERT(false); break;
    }

    return SkFontStyle(weight, width, slant);
}

static void fcpattern_from_skfontstyle(SkFontStyle style, FcPattern* pattern) {
    FCLocker::AssertHeld();

    typedef SkFontStyle SkFS;

    static const MapRanges weightRanges[] = {
        { SkTFixed<SkFS::kThin_Weight>::value,       SkTFixed<FC_WEIGHT_THIN>::value },
        { SkTFixed<SkFS::kExtraLight_Weight>::value, SkTFixed<FC_WEIGHT_EXTRALIGHT>::value },
        { SkTFixed<SkFS::kLight_Weight>::value,      SkTFixed<FC_WEIGHT_LIGHT>::value },
        { SkTFixed<SkFS::kNormal_Weight>::value,     SkTFixed<FC_WEIGHT_REGULAR>::value },
        { SkTFixed<SkFS::kMedium_Weight>::value,     SkTFixed<FC_WEIGHT_MEDIUM>::value },
        { SkTFixed<SkFS::kSemiBold_Weight>::value,   SkTFixed<FC_WEIGHT_DEMIBOLD>::value },
        { SkTFixed<SkFS::kBold_Weight>::value,       SkTFixed<FC_WEIGHT_BOLD>::value },
        { SkTFixed<SkFS::kExtraBold_Weight>::value,  SkTFixed<FC_WEIGHT_EXTRABOLD>::value },
        { SkTFixed<SkFS::kBlack_Weight>::value,      SkTFixed<FC_WEIGHT_BLACK>::value },
        { SkTFixed<SkFS::kExtraBlack_Weight>::value, SkTFixed<FC_WEIGHT_EXTRABLACK>::value },
    };
    int weight = map_ranges(style.weight(), weightRanges, SK_ARRAY_COUNT(weightRanges));

    static const MapRanges widthRanges[] = {
        { SkTFixed<SkFS::kUltraCondensed_Width>::value, SkTFixed<FC_WIDTH_ULTRACONDENSED>::value },
        { SkTFixed<SkFS::kExtraCondensed_Width>::value, SkTFixed<FC_WIDTH_EXTRACONDENSED>::value },
        { SkTFixed<SkFS::kCondensed_Width>::value,      SkTFixed<FC_WIDTH_CONDENSED>::value },
        { SkTFixed<SkFS::kSemiCondensed_Width>::value,  SkTFixed<FC_WIDTH_SEMICONDENSED>::value },
        { SkTFixed<SkFS::kNormal_Width>::value,         SkTFixed<FC_WIDTH_NORMAL>::value },
        { SkTFixed<SkFS::kSemiExpanded_Width>::value,   SkTFixed<FC_WIDTH_SEMIEXPANDED>::value },
        { SkTFixed<SkFS::kExpanded_Width>::value,       SkTFixed<FC_WIDTH_EXPANDED>::value },
        { SkTFixed<SkFS::kExtraExpanded_Width>::value,  SkTFixed<FC_WIDTH_EXTRAEXPANDED>::value },
        { SkTFixed<SkFS::kUltraExpanded_Width>::value,  SkTFixed<FC_WIDTH_ULTRAEXPANDED>::value },
    };
    int width = map_ranges(style.width(), widthRanges, SK_ARRAY_COUNT(widthRanges));

    int slant = FC_SLANT_ROMAN;
    switch (style.slant()) {
        case SkFS::kUpright_Slant: slant = FC_SLANT_ROMAN  ; break;
        case SkFS::kItalic_Slant : slant = FC_SLANT_ITALIC ; break;
        case SkFS::kOblique_Slant: slant = FC_SLANT_OBLIQUE; break;
        default: SkASSERT(false); break;
    }

    FcPatternAddInteger(pattern, FC_WEIGHT, weight);
    FcPatternAddInteger(pattern, FC_WIDTH , width);
    FcPatternAddInteger(pattern, FC_SLANT , slant);
}

class SkTypeface_stream : public SkTypeface_FreeType {
public:
    /** @param data takes ownership of the font data.*/
    SkTypeface_stream(std::unique_ptr<SkFontData> data,
                      SkString familyName, const SkFontStyle& style, bool fixedWidth)
        : INHERITED(style, fixedWidth)
        , fFamilyName(std::move(familyName))
        , fData(std::move(data))
    { }

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        *serialize = true;
    }

    SkStreamAsset* onOpenStream(int* ttcIndex) const override {
        *ttcIndex = fData->getIndex();
        return fData->getStream()->duplicate();
    }

    std::unique_ptr<SkFontData> onMakeFontData() const override {
        return skstd::make_unique<SkFontData>(*fData);
    }

private:
    SkString fFamilyName;
    const std::unique_ptr<const SkFontData> fData;

    typedef SkTypeface_FreeType INHERITED;
};

class SkTypeface_fontconfig : public SkTypeface_FreeType {
public:
    /** @param pattern takes ownership of the reference. */
    static SkTypeface_fontconfig* Create(FcPattern* pattern) {
        return new SkTypeface_fontconfig(pattern);
    }
    mutable SkAutoFcPattern fPattern;

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = get_string(fPattern, FC_FAMILY);
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        FCLocker lock;
        desc->setFamilyName(get_string(fPattern, FC_FAMILY));
        desc->setFullName(get_string(fPattern, FC_FULLNAME));
        desc->setPostscriptName(get_string(fPattern, FC_POSTSCRIPT_NAME));
        desc->setStyle(this->fontStyle());
        *serialize = false;
    }

    SkStreamAsset* onOpenStream(int* ttcIndex) const override {
        FCLocker lock;
        *ttcIndex = get_int(fPattern, FC_INDEX, 0);
        return SkStream::MakeFromFile(get_string(fPattern, FC_FILE)).release();
    }

    void onFilterRec(SkScalerContextRec* rec) const override {
        const FcMatrix* fcMatrix = get_matrix(fPattern, FC_MATRIX);
        if (fcMatrix) {
            // fPost2x2 is column-major, left handed (y down).
            // FcMatrix is column-major, right handed (y up).
            SkMatrix fm;
            fm.setAll(fcMatrix->xx,-fcMatrix->xy, 0,
                     -fcMatrix->yx, fcMatrix->yy, 0,
                      0           , 0           , 1);

            SkMatrix sm;
            rec->getMatrixFrom2x2(&sm);

            sm.preConcat(fm);
            rec->fPost2x2[0][0] = sm.getScaleX();
            rec->fPost2x2[0][1] = sm.getSkewX();
            rec->fPost2x2[1][0] = sm.getSkewY();
            rec->fPost2x2[1][1] = sm.getScaleY();
        }
        if (get_bool(fPattern, FC_EMBOLDEN)) {
            rec->fFlags |= SkScalerContext::kEmbolden_Flag;
        }
        this->INHERITED::onFilterRec(rec);
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        std::unique_ptr<SkAdvancedTypefaceMetrics> info =
            this->INHERITED::onGetAdvancedMetrics();

        // Simulated fonts shouldn't be considered to be of the type of their data.
        if (get_matrix(fPattern, FC_MATRIX) || get_bool(fPattern, FC_EMBOLDEN)) {
            info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
        }
        return info;
    }

    ~SkTypeface_fontconfig() override {
        // Hold the lock while unrefing the pattern.
        FCLocker lock;
        fPattern.reset();
    }

private:
    /** @param pattern takes ownership of the reference. */
    SkTypeface_fontconfig(FcPattern* pattern)
        : INHERITED(skfontstyle_from_fcpattern(pattern),
                    FC_PROPORTIONAL != get_int(pattern, FC_SPACING, FC_PROPORTIONAL))
        , fPattern(pattern)
    { }

    typedef SkTypeface_FreeType INHERITED;
};

class SkFontMgr_fontconfig : public SkFontMgr {
    mutable SkAutoFcConfig fFC;
    sk_sp<SkDataTable> fFamilyNames;
    SkTypeface_FreeType::Scanner fScanner;

    class StyleSet : public SkFontStyleSet {
    public:
        /** @param parent does not take ownership of the reference.
         *  @param fontSet takes ownership of the reference.
         */
        StyleSet(const SkFontMgr_fontconfig* parent, FcFontSet* fontSet)
            : fFontMgr(SkRef(parent)), fFontSet(fontSet)
        { }

        ~StyleSet() override {
            // Hold the lock while unrefing the font set.
            FCLocker lock;
            fFontSet.reset();
        }

        int count() override { return fFontSet->nfont; }

        void getStyle(int index, SkFontStyle* style, SkString* styleName) override {
            if (index < 0 || fFontSet->nfont <= index) {
                return;
            }

            FCLocker lock;
            if (style) {
                *style = skfontstyle_from_fcpattern(fFontSet->fonts[index]);
            }
            if (styleName) {
                *styleName = get_string(fFontSet->fonts[index], FC_STYLE);
            }
        }

        SkTypeface* createTypeface(int index) override {
            FCLocker lock;

            FcPattern* match = fFontSet->fonts[index];
            return fFontMgr->createTypefaceFromFcPattern(match);
        }

        SkTypeface* matchStyle(const SkFontStyle& style) override {
            FCLocker lock;

            SkAutoFcPattern pattern;
            fcpattern_from_skfontstyle(style, pattern);
            FcConfigSubstitute(fFontMgr->fFC, pattern, FcMatchPattern);
            FcDefaultSubstitute(pattern);

            FcResult result;
            FcFontSet* fontSets[1] = { fFontSet };
            SkAutoFcPattern match(FcFontSetMatch(fFontMgr->fFC,
                                                 fontSets, SK_ARRAY_COUNT(fontSets),
                                                 pattern, &result));
            if (nullptr == match) {
                return nullptr;
            }

            return fFontMgr->createTypefaceFromFcPattern(match);
        }

    private:
        sk_sp<const SkFontMgr_fontconfig> fFontMgr;
        SkAutoFcFontSet fFontSet;
    };

    static bool FindName(const SkTDArray<const char*>& list, const char* str) {
        int count = list.count();
        for (int i = 0; i < count; ++i) {
            if (!strcmp(list[i], str)) {
                return true;
            }
        }
        return false;
    }

    static sk_sp<SkDataTable> GetFamilyNames(FcConfig* fcconfig) {
        FCLocker lock;

        SkTDArray<const char*> names;
        SkTDArray<size_t> sizes;

        static const FcSetName fcNameSet[] = { FcSetSystem, FcSetApplication };
        for (int setIndex = 0; setIndex < (int)SK_ARRAY_COUNT(fcNameSet); ++setIndex) {
            // Return value of FcConfigGetFonts must not be destroyed.
            FcFontSet* allFonts(FcConfigGetFonts(fcconfig, fcNameSet[setIndex]));
            if (nullptr == allFonts) {
                continue;
            }

            for (int fontIndex = 0; fontIndex < allFonts->nfont; ++fontIndex) {
                FcPattern* current = allFonts->fonts[fontIndex];
                for (int id = 0; ; ++id) {
                    FcChar8* fcFamilyName;
                    FcResult result = FcPatternGetString(current, FC_FAMILY, id, &fcFamilyName);
                    if (FcResultNoId == result) {
                        break;
                    }
                    if (FcResultMatch != result) {
                        continue;
                    }
                    const char* familyName = reinterpret_cast<const char*>(fcFamilyName);
                    if (familyName && !FindName(names, familyName)) {
                        *names.append() = familyName;
                        *sizes.append() = strlen(familyName) + 1;
                    }
                }
            }
        }

        return SkDataTable::MakeCopyArrays((void const *const *)names.begin(),
                                           sizes.begin(), names.count());
    }

    static bool FindByFcPattern(SkTypeface* cached, void* ctx) {
        SkTypeface_fontconfig* cshFace = static_cast<SkTypeface_fontconfig*>(cached);
        FcPattern* ctxPattern = static_cast<FcPattern*>(ctx);
        return FcTrue == FcPatternEqual(cshFace->fPattern, ctxPattern);
    }

    mutable SkMutex fTFCacheMutex;
    mutable SkTypefaceCache fTFCache;
    /** Creates a typeface using a typeface cache.
     *  @param pattern a complete pattern from FcFontRenderPrepare.
     */
    SkTypeface* createTypefaceFromFcPattern(FcPattern* pattern) const {
        FCLocker::AssertHeld();
        SkAutoMutexAcquire ama(fTFCacheMutex);
        SkTypeface* face = fTFCache.findByProcAndRef(FindByFcPattern, pattern);
        if (nullptr == face) {
            FcPatternReference(pattern);
            face = SkTypeface_fontconfig::Create(pattern);
            if (face) {
                fTFCache.add(face);
            }
        }
        return face;
    }

public:
    /** Takes control of the reference to 'config'. */
    explicit SkFontMgr_fontconfig(FcConfig* config)
        : fFC(config ? config : FcInitLoadConfigAndFonts())
        , fFamilyNames(GetFamilyNames(fFC)) { }

    ~SkFontMgr_fontconfig() override {
        // Hold the lock while unrefing the config.
        FCLocker lock;
        fFC.reset();
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

    /** True if any string object value in the font is the same
     *         as a string object value in the pattern.
     */
    static bool AnyMatching(FcPattern* font, FcPattern* pattern, const char* object) {
        FcChar8* fontString;
        FcChar8* patternString;
        FcResult result;
        // Set an arbitrary limit on the number of pattern object values to consider.
        // TODO: re-write this to avoid N*M
        static const int maxId = 16;
        for (int patternId = 0; patternId < maxId; ++patternId) {
            result = FcPatternGetString(pattern, object, patternId, &patternString);
            if (FcResultNoId == result) {
                break;
            }
            if (FcResultMatch != result) {
                continue;
            }
            for (int fontId = 0; fontId < maxId; ++fontId) {
                result = FcPatternGetString(font, object, fontId, &fontString);
                if (FcResultNoId == result) {
                    break;
                }
                if (FcResultMatch != result) {
                    continue;
                }
                if (0 == FcStrCmpIgnoreCase(patternString, fontString)) {
                    return true;
                }
            }
        }
        return false;
    }

    static bool FontAccessible(FcPattern* font) {
        // FontConfig can return fonts which are unreadable.
        const char* filename = get_string(font, FC_FILE, nullptr);
        if (nullptr == filename) {
            return false;
        }
        return sk_exists(filename, kRead_SkFILE_Flag);
    }

    static bool FontFamilyNameMatches(FcPattern* font, FcPattern* pattern) {
        return AnyMatching(font, pattern, FC_FAMILY);
    }

    static bool FontContainsCharacter(FcPattern* font, uint32_t character) {
        FcResult result;
        FcCharSet* matchCharSet;
        for (int charSetId = 0; ; ++charSetId) {
            result = FcPatternGetCharSet(font, FC_CHARSET, charSetId, &matchCharSet);
            if (FcResultNoId == result) {
                break;
            }
            if (FcResultMatch != result) {
                continue;
            }
            if (FcCharSetHasChar(matchCharSet, character)) {
                return true;
            }
        }
        return false;
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        FCLocker lock;

        SkAutoFcPattern pattern;
        FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)familyName);
        FcConfigSubstitute(fFC, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        FcPattern* matchPattern;
        SkAutoFcPattern strongPattern(nullptr);
        if (familyName) {
            strongPattern.reset(FcPatternDuplicate(pattern));
            remove_weak(strongPattern, FC_FAMILY);
            matchPattern = strongPattern;
        } else {
            matchPattern = pattern;
        }

        SkAutoFcFontSet matches;
        // TODO: Some families have 'duplicates' due to symbolic links.
        // The patterns are exactly the same except for the FC_FILE.
        // It should be possible to collapse these patterns by normalizing.
        static const FcSetName fcNameSet[] = { FcSetSystem, FcSetApplication };
        for (int setIndex = 0; setIndex < (int)SK_ARRAY_COUNT(fcNameSet); ++setIndex) {
            // Return value of FcConfigGetFonts must not be destroyed.
            FcFontSet* allFonts(FcConfigGetFonts(fFC, fcNameSet[setIndex]));
            if (nullptr == allFonts) {
                continue;
            }

            for (int fontIndex = 0; fontIndex < allFonts->nfont; ++fontIndex) {
                FcPattern* font = allFonts->fonts[fontIndex];
                if (FontAccessible(font) && FontFamilyNameMatches(font, matchPattern)) {
                    FcFontSetAdd(matches, FcFontRenderPrepare(fFC, pattern, font));
                }
            }
        }

        return new StyleSet(this, matches.release());
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& style) const override
    {
        FCLocker lock;

        SkAutoFcPattern pattern;
        FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)familyName);
        fcpattern_from_skfontstyle(style, pattern);
        FcConfigSubstitute(fFC, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        // We really want to match strong (prefered) and same (acceptable) only here.
        // If a family name was specified, assume that any weak matches after the last strong match
        // are weak (default) and ignore them.
        // The reason for is that after substitution the pattern for 'sans-serif' looks like
        // "wwwwwwwwwwwwwwswww" where there are many weak but preferred names, followed by defaults.
        // So it is possible to have weakly matching but preferred names.
        // In aliases, bindings are weak by default, so this is easy and common.
        // If no family name was specified, we'll probably only get weak matches, but that's ok.
        FcPattern* matchPattern;
        SkAutoFcPattern strongPattern(nullptr);
        if (familyName) {
            strongPattern.reset(FcPatternDuplicate(pattern));
            remove_weak(strongPattern, FC_FAMILY);
            matchPattern = strongPattern;
        } else {
            matchPattern = pattern;
        }

        FcResult result;
        SkAutoFcPattern font(FcFontMatch(fFC, pattern, &result));
        if (nullptr == font || !FontAccessible(font) || !FontFamilyNameMatches(font, matchPattern)) {
            return nullptr;
        }

        return createTypefaceFromFcPattern(font);
    }

    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char* bcp47[],
                                                    int bcp47Count,
                                                    SkUnichar character) const override
    {
        FCLocker lock;

        SkAutoFcPattern pattern;
        if (familyName) {
            FcValue familyNameValue;
            familyNameValue.type = FcTypeString;
            familyNameValue.u.s = reinterpret_cast<const FcChar8*>(familyName);
            FcPatternAddWeak(pattern, FC_FAMILY, familyNameValue, FcFalse);
        }
        fcpattern_from_skfontstyle(style, pattern);

        SkAutoFcCharSet charSet;
        FcCharSetAddChar(charSet, character);
        FcPatternAddCharSet(pattern, FC_CHARSET, charSet);

        if (bcp47Count > 0) {
            SkASSERT(bcp47);
            SkAutoFcLangSet langSet;
            for (int i = bcp47Count; i --> 0;) {
                FcLangSetAdd(langSet, (const FcChar8*)bcp47[i]);
            }
            FcPatternAddLangSet(pattern, FC_LANG, langSet);
        }

        FcConfigSubstitute(fFC, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        FcResult result;
        SkAutoFcPattern font(FcFontMatch(fFC, pattern, &result));
        if (nullptr == font || !FontAccessible(font) || !FontContainsCharacter(font, character)) {
            return nullptr;
        }

        return createTypefaceFromFcPattern(font);
    }

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* typeface,
                                         const SkFontStyle& style) const override
    {
        //TODO: should the SkTypeface_fontconfig know its family?
        const SkTypeface_fontconfig* fcTypeface =
                static_cast<const SkTypeface_fontconfig*>(typeface);
        return this->matchFamilyStyle(get_string(fcTypeface->fPattern, FC_FAMILY), style);
    }

    SkTypeface* onCreateFromStream(SkStreamAsset* bareStream, int ttcIndex) const override {
        std::unique_ptr<SkStreamAsset> stream(bareStream);
        const size_t length = stream->getLength();
        if (length <= 0 || (1u << 30) < length) {
            return nullptr;
        }

        SkString name;
        SkFontStyle style;
        bool isFixedWidth = false;
        if (!fScanner.scanFont(stream.get(), ttcIndex, &name, &style, &isFixedWidth, nullptr)) {
            return nullptr;
        }

        auto data = skstd::make_unique<SkFontData>(std::move(stream), ttcIndex, nullptr, 0);
        return new SkTypeface_stream(std::move(data), std::move(name), style, isFixedWidth);
    }

    SkTypeface* onCreateFromStream(SkStreamAsset* s, const SkFontArguments& args) const override {
        using Scanner = SkTypeface_FreeType::Scanner;
        std::unique_ptr<SkStreamAsset> stream(s);
        bool isFixedPitch;
        SkFontStyle style;
        SkString name;
        Scanner::AxisDefinitions axisDefinitions;
        if (!fScanner.scanFont(stream.get(), args.getCollectionIndex(),
                               &name, &style, &isFixedPitch, &axisDefinitions))
        {
            return nullptr;
        }

        SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.count());
        Scanner::computeAxisValues(axisDefinitions, args.getVariationDesignPosition(),
                                   axisValues, name);

        auto data = skstd::make_unique<SkFontData>(std::move(stream), args.getCollectionIndex(),
                                                   axisValues.get(), axisDefinitions.count());
        return new SkTypeface_stream(std::move(data), std::move(name), style, isFixedPitch);
    }

    SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const override {
        return this->createFromStream(new SkMemoryStream(sk_ref_sp(data)), ttcIndex);
    }

    SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const override {
        return this->createFromStream(SkStream::MakeFromFile(path).release(), ttcIndex);
    }

    SkTypeface* onCreateFromFontData(std::unique_ptr<SkFontData> fontData) const override {
        SkStreamAsset* stream(fontData->getStream());
        const size_t length = stream->getLength();
        if (length <= 0 || (1u << 30) < length) {
            return nullptr;
        }

        const int ttcIndex = fontData->getIndex();
        SkString name;
        SkFontStyle style;
        bool isFixedWidth = false;
        if (!fScanner.scanFont(stream, ttcIndex, &name, &style, &isFixedWidth, nullptr)) {
            return nullptr;
        }

        return new SkTypeface_stream(std::move(fontData), std::move(name), style, isFixedWidth);
    }

    SkTypeface* onLegacyCreateTypeface(const char familyName[], SkFontStyle style) const override {
        sk_sp<SkTypeface> typeface(this->matchFamilyStyle(familyName, style));
        if (typeface.get()) {
            return typeface.release();
        }

        return this->matchFamilyStyle(nullptr, style);
    }
};

SK_API sk_sp<SkFontMgr> SkFontMgr_New_FontConfig(FcConfig* fc) {
    return sk_make_sp<SkFontMgr_fontconfig>(fc);
}
