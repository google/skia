/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDataTable.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "src/base/SkTSort.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkTypefaceCache.h"
#include "src/ports/SkTypeface_FreeType.h"

#include <fontconfig/fontconfig.h>

#include <string.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

class SkData;

using namespace skia_private;

// FC_POSTSCRIPT_NAME was added with b561ff20 which ended up in 2.10.92
// Ubuntu 14.04 is on 2.11.0
// Debian 8 and 9 are on 2.11
// OpenSUSE Leap 42.1 is on 2.11.0 (42.3 is on 2.11.1)
// Fedora 24 is on 2.11.94
#ifndef FC_POSTSCRIPT_NAME
#    define FC_POSTSCRIPT_NAME  "postscriptname"
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

// FontConfig was thread antagonistic until 2.10.91 with known thread safety issues until 2.13.93.
// Before that, lock with a global mutex.
// See https://bug.skia.org/1497 and cl/339089311 for background.
static SkMutex& f_c_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}

class FCLocker {
    inline static constexpr int FontConfigThreadSafeVersion = 21393;

    // Assume FcGetVersion() has always been thread safe.
    static void lock() SK_NO_THREAD_SAFETY_ANALYSIS {
        if (FcGetVersion() < FontConfigThreadSafeVersion) {
            f_c_mutex().acquire();
        }
    }
    static void unlock() SK_NO_THREAD_SAFETY_ANALYSIS {
        AssertHeld();
        if (FcGetVersion() < FontConfigThreadSafeVersion) {
            f_c_mutex().release();
        }
    }

public:
    FCLocker() { lock(); }
    ~FCLocker() { unlock(); }

    static void AssertHeld() { SkDEBUGCODE(
        if (FcGetVersion() < FontConfigThreadSafeVersion) {
            f_c_mutex().assertHeld();
        }
    ) }
};

} // namespace

template<typename T, void (*D)(T*)> void FcTDestroy(T* t) {
    FCLocker::AssertHeld();
    D(t);
}
template <typename T, T* (*C)(), void (*D)(T*)> class SkAutoFc
    : public SkAutoTCallVProc<T, FcTDestroy<T, D>> {
    using inherited = SkAutoTCallVProc<T, FcTDestroy<T, D>>;
public:
    SkAutoFc() : SkAutoTCallVProc<T, FcTDestroy<T, D>>( C() ) {
        T* obj = this->operator T*();
        SkASSERT_RELEASE(nullptr != obj);
    }
    explicit SkAutoFc(T* obj) : inherited(obj) {}
    SkAutoFc(const SkAutoFc&) = delete;
    SkAutoFc(SkAutoFc&& that) : inherited(std::move(that)) {}
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
 *  Sometime after 2.12.4 FcPatternGetWithBinding was added which can retrieve the binding.
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
    SkAutoFcPattern match(FcFontSetMatch(config, fontSets, std::size(fontSets),
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

static int map_range(SkScalar value,
                     SkScalar old_min, SkScalar old_max,
                     SkScalar new_min, SkScalar new_max)
{
    SkASSERT(old_min < old_max);
    SkASSERT(new_min <= new_max);
    return new_min + ((value - old_min) * (new_max - new_min) / (old_max - old_min));
}

struct MapRanges {
    SkScalar old_val;
    SkScalar new_val;
};

static SkScalar map_ranges(SkScalar val, MapRanges const ranges[], int rangesCount) {
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

#ifndef FC_WEIGHT_DEMILIGHT
#define FC_WEIGHT_DEMILIGHT        65
#endif

static SkFontStyle skfontstyle_from_fcpattern(FcPattern* pattern) {
    typedef SkFontStyle SkFS;

    // FcWeightToOpenType was buggy until 2.12.4
    static constexpr MapRanges weightRanges[] = {
        { FC_WEIGHT_THIN,       SkFS::kThin_Weight },
        { FC_WEIGHT_EXTRALIGHT, SkFS::kExtraLight_Weight },
        { FC_WEIGHT_LIGHT,      SkFS::kLight_Weight },
        { FC_WEIGHT_DEMILIGHT,  350 },
        { FC_WEIGHT_BOOK,       380 },
        { FC_WEIGHT_REGULAR,    SkFS::kNormal_Weight },
        { FC_WEIGHT_MEDIUM,     SkFS::kMedium_Weight },
        { FC_WEIGHT_DEMIBOLD,   SkFS::kSemiBold_Weight },
        { FC_WEIGHT_BOLD,       SkFS::kBold_Weight },
        { FC_WEIGHT_EXTRABOLD,  SkFS::kExtraBold_Weight },
        { FC_WEIGHT_BLACK,      SkFS::kBlack_Weight },
        { FC_WEIGHT_EXTRABLACK, SkFS::kExtraBlack_Weight },
    };
    SkScalar weight = map_ranges(get_int(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR),
                                 weightRanges, std::size(weightRanges));

    static constexpr MapRanges widthRanges[] = {
        { FC_WIDTH_ULTRACONDENSED, SkFS::kUltraCondensed_Width },
        { FC_WIDTH_EXTRACONDENSED, SkFS::kExtraCondensed_Width },
        { FC_WIDTH_CONDENSED,      SkFS::kCondensed_Width },
        { FC_WIDTH_SEMICONDENSED,  SkFS::kSemiCondensed_Width },
        { FC_WIDTH_NORMAL,         SkFS::kNormal_Width },
        { FC_WIDTH_SEMIEXPANDED,   SkFS::kSemiExpanded_Width },
        { FC_WIDTH_EXPANDED,       SkFS::kExpanded_Width },
        { FC_WIDTH_EXTRAEXPANDED,  SkFS::kExtraExpanded_Width },
        { FC_WIDTH_ULTRAEXPANDED,  SkFS::kUltraExpanded_Width },
    };
    SkScalar width = map_ranges(get_int(pattern, FC_WIDTH, FC_WIDTH_NORMAL),
                                widthRanges, std::size(widthRanges));

    SkFS::Slant slant = SkFS::kUpright_Slant;
    switch (get_int(pattern, FC_SLANT, FC_SLANT_ROMAN)) {
        case FC_SLANT_ROMAN:   slant = SkFS::kUpright_Slant; break;
        case FC_SLANT_ITALIC : slant = SkFS::kItalic_Slant ; break;
        case FC_SLANT_OBLIQUE: slant = SkFS::kOblique_Slant; break;
        default: SkASSERT(false); break;
    }

    return SkFontStyle(SkScalarRoundToInt(weight), SkScalarRoundToInt(width), slant);
}

static void fcpattern_from_skfontstyle(SkFontStyle style, FcPattern* pattern) {
    FCLocker::AssertHeld();

    typedef SkFontStyle SkFS;

    // FcWeightFromOpenType was buggy until 2.12.4
    static constexpr MapRanges weightRanges[] = {
        { SkFS::kThin_Weight,       FC_WEIGHT_THIN },
        { SkFS::kExtraLight_Weight, FC_WEIGHT_EXTRALIGHT },
        { SkFS::kLight_Weight,      FC_WEIGHT_LIGHT },
        { 350,                      FC_WEIGHT_DEMILIGHT },
        { 380,                      FC_WEIGHT_BOOK },
        { SkFS::kNormal_Weight,     FC_WEIGHT_REGULAR },
        { SkFS::kMedium_Weight,     FC_WEIGHT_MEDIUM },
        { SkFS::kSemiBold_Weight,   FC_WEIGHT_DEMIBOLD },
        { SkFS::kBold_Weight,       FC_WEIGHT_BOLD },
        { SkFS::kExtraBold_Weight,  FC_WEIGHT_EXTRABOLD },
        { SkFS::kBlack_Weight,      FC_WEIGHT_BLACK },
        { SkFS::kExtraBlack_Weight, FC_WEIGHT_EXTRABLACK },
    };
    int weight = map_ranges(style.weight(), weightRanges, std::size(weightRanges));

    static constexpr MapRanges widthRanges[] = {
        { SkFS::kUltraCondensed_Width, FC_WIDTH_ULTRACONDENSED },
        { SkFS::kExtraCondensed_Width, FC_WIDTH_EXTRACONDENSED },
        { SkFS::kCondensed_Width,      FC_WIDTH_CONDENSED },
        { SkFS::kSemiCondensed_Width,  FC_WIDTH_SEMICONDENSED },
        { SkFS::kNormal_Width,         FC_WIDTH_NORMAL },
        { SkFS::kSemiExpanded_Width,   FC_WIDTH_SEMIEXPANDED },
        { SkFS::kExpanded_Width,       FC_WIDTH_EXPANDED },
        { SkFS::kExtraExpanded_Width,  FC_WIDTH_EXTRAEXPANDED },
        { SkFS::kUltraExpanded_Width,  FC_WIDTH_ULTRAEXPANDED },
    };
    int width = map_ranges(style.width(), widthRanges, std::size(widthRanges));

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

class SkTypeface_fontconfig : public SkTypeface_FreeType {
public:
    static sk_sp<SkTypeface_fontconfig> Make(SkAutoFcPattern pattern, SkString sysroot) {
        return sk_sp<SkTypeface_fontconfig>(new SkTypeface_fontconfig(std::move(pattern),
                                                                      std::move(sysroot)));
    }
    mutable SkAutoFcPattern fPattern;  // Mutable for passing to FontConfig API.
    const SkString fSysroot;

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = get_string(fPattern, FC_FAMILY);
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        // TODO: need to serialize FC_MATRIX and FC_EMBOLDEN
        FCLocker lock;
        desc->setFamilyName(get_string(fPattern, FC_FAMILY));
        desc->setFullName(get_string(fPattern, FC_FULLNAME));
        desc->setPostscriptName(get_string(fPattern, FC_POSTSCRIPT_NAME));
        desc->setStyle(this->fontStyle());
        desc->setFactoryId(SkTypeface_FreeType::FactoryId);
        *serialize = false;
    }

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        FCLocker lock;
        *ttcIndex = get_int(fPattern, FC_INDEX, 0);
        const char* filename = get_string(fPattern, FC_FILE);
        // See FontAccessible for note on searching sysroot then non-sysroot path.
        SkString resolvedFilename;
        if (!fSysroot.isEmpty()) {
            resolvedFilename = fSysroot;
            resolvedFilename += filename;
            if (sk_exists(resolvedFilename.c_str(), kRead_SkFILE_Flag)) {
                filename = resolvedFilename.c_str();
            }
        }
        return SkStream::MakeFromFile(filename);
    }

    void onFilterRec(SkScalerContextRec* rec) const override {
        // FontConfig provides 10-scale-bitmap-fonts.conf which applies an inverse "pixelsize"
        // matrix. It is not known if this .conf is active or not, so it is not clear if
        // "pixelsize" should be applied before this matrix. Since using a matrix with a bitmap
        // font isn't a great idea, only apply the matrix to outline fonts.
        const FcMatrix* fcMatrix = get_matrix(fPattern, FC_MATRIX);
        bool fcOutline = get_bool(fPattern, FC_OUTLINE, true);
        if (fcOutline && fcMatrix) {
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

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        SkFontStyle style = this->fontStyle();
        std::unique_ptr<SkFontData> data = this->cloneFontData(args, &style);
        if (!data) {
            return nullptr;
        }

        // TODO: need to clone FC_MATRIX and FC_EMBOLDEN
        SkString familyName;
        this->getFamilyName(&familyName);
        return sk_make_sp<SkTypeface_FreeTypeStream>(
            std::move(data), familyName, style, this->isFixedPitch());
    }

    std::unique_ptr<SkFontData> onMakeFontData() const override {
        int index;
        std::unique_ptr<SkStreamAsset> stream(this->onOpenStream(&index));
        if (!stream) {
            return nullptr;
        }
        // TODO: FC_VARIABLE and FC_FONT_VARIATIONS
        return std::make_unique<SkFontData>(std::move(stream), index, 0, nullptr, 0, nullptr, 0);
    }

    ~SkTypeface_fontconfig() override {
        // Hold the lock while unrefing the pattern.
        FCLocker lock;
        fPattern.reset();
    }

private:
    SkTypeface_fontconfig(SkAutoFcPattern pattern, SkString sysroot)
        : INHERITED(skfontstyle_from_fcpattern(pattern),
                    FC_PROPORTIONAL != get_int(pattern, FC_SPACING, FC_PROPORTIONAL))
        , fPattern(std::move(pattern))
        , fSysroot(std::move(sysroot))
    { }

    using INHERITED = SkTypeface_FreeType;
};

class SkFontMgr_fontconfig : public SkFontMgr {
    mutable SkAutoFcConfig fFC;  // Only mutable to avoid const cast when passed to FontConfig API.
    const SkString fSysroot;
    const sk_sp<SkDataTable> fFamilyNames;

    class StyleSet : public SkFontStyleSet {
    public:
        StyleSet(sk_sp<SkFontMgr_fontconfig> parent, SkAutoFcFontSet fontSet)
            : fFontMgr(std::move(parent))
            , fFontSet(std::move(fontSet))
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

        sk_sp<SkTypeface> createTypeface(int index) override {
            if (index < 0 || fFontSet->nfont <= index) {
                return nullptr;
            }
            SkAutoFcPattern match([this, &index]() {
                FCLocker lock;
                FcPatternReference(fFontSet->fonts[index]);
                return fFontSet->fonts[index];
            }());
            return fFontMgr->createTypefaceFromFcPattern(std::move(match));
        }

        sk_sp<SkTypeface> matchStyle(const SkFontStyle& style) override {
            SkAutoFcPattern match([this, &style]() {
                FCLocker lock;

                SkAutoFcPattern pattern;
                fcpattern_from_skfontstyle(style, pattern);
                FcConfigSubstitute(fFontMgr->fFC, pattern, FcMatchPattern);
                FcDefaultSubstitute(pattern);

                FcResult result;
                FcFontSet* fontSets[1] = { fFontSet };
                return FcFontSetMatch(fFontMgr->fFC,
                                      fontSets, std::size(fontSets),
                                      pattern, &result);

            }());
            return fFontMgr->createTypefaceFromFcPattern(std::move(match));
        }

    private:
        sk_sp<SkFontMgr_fontconfig> fFontMgr;
        SkAutoFcFontSet fFontSet;
    };

    static bool FindName(const SkTDArray<const char*>& list, const char* str) {
        int count = list.size();
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
        for (int setIndex = 0; setIndex < (int)std::size(fcNameSet); ++setIndex) {
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
                                           sizes.begin(), names.size());
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
    sk_sp<SkTypeface> createTypefaceFromFcPattern(SkAutoFcPattern pattern) const {
        if (!pattern) {
            return nullptr;
        }
        // Cannot hold FCLocker when calling fTFCache.add; an evicted typeface may need to lock.
        // Must hold fTFCacheMutex when interacting with fTFCache.
        SkAutoMutexExclusive ama(fTFCacheMutex);
        sk_sp<SkTypeface> face = [&]() {
            FCLocker lock;
            sk_sp<SkTypeface> face = fTFCache.findByProcAndRef(FindByFcPattern, pattern);
            if (face) {
                pattern.reset();
            }
            return face;
        }();
        if (!face) {
            face = SkTypeface_fontconfig::Make(std::move(pattern), fSysroot);
            if (face) {
                // Cannot hold FCLocker in fTFCache.add; evicted typefaces may need to lock.
                fTFCache.add(face);
            }
        }
        return face;
    }

public:
    /** Takes control of the reference to 'config'. */
    explicit SkFontMgr_fontconfig(FcConfig* config)
        : fFC(config ? config : FcInitLoadConfigAndFonts())
        , fSysroot(reinterpret_cast<const char*>(FcConfigGetSysRoot(fFC)))
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

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        return this->onMatchFamily(fFamilyNames->atStr(index));
    }

    /** True if any string object value in the font is the same
     *         as a string object value in the pattern.
     */
    static bool AnyStringMatching(FcPattern* font, FcPattern* pattern, const char* object) {
        auto getStrings = [](FcPattern* p, const char* object, STArray<32, FcChar8*>& strings) {
            // Set an arbitrary (but high) limit on the number of pattern object values to consider.
            static constexpr const int maxId = 65536;
            for (int patternId = 0; patternId < maxId; ++patternId) {
                FcChar8* patternString;
                FcResult result = FcPatternGetString(p, object, patternId, &patternString);
                if (result == FcResultNoId) {
                    break;
                }
                if (result == FcResultMatch) {
                    strings.push_back(patternString);
                }
            }
        };
        auto compareStrings = [](FcChar8* a, FcChar8* b) -> bool {
            return FcStrCmpIgnoreCase(a, b) < 0;
        };

        STArray<32, FcChar8*> fontStrings;
        STArray<32, FcChar8*> patternStrings;
        getStrings(font, object, fontStrings);
        getStrings(pattern, object, patternStrings);

        SkTQSort(fontStrings.begin(), fontStrings.end(), compareStrings);
        SkTQSort(patternStrings.begin(), patternStrings.end(), compareStrings);

        FcChar8** fontString = fontStrings.begin();
        FcChar8** patternString = patternStrings.begin();
        while (fontString != fontStrings.end() && patternString != patternStrings.end()) {
            int cmp = FcStrCmpIgnoreCase(*fontString, *patternString);
            if (cmp < 0) {
                ++fontString;
            } else if (cmp > 0) {
                ++patternString;
            } else {
                return true;
            }
        }
        return false;
    }

    bool FontAccessible(FcPattern* font) const {
        // FontConfig can return fonts which are unreadable.
        const char* filename = get_string(font, FC_FILE, nullptr);
        if (nullptr == filename) {
            return false;
        }

        // When sysroot was implemented in e96d7760886a3781a46b3271c76af99e15cb0146 (before 2.11.0)
        // it was broken;  mostly fixed in d17f556153fbaf8fe57fdb4fc1f0efa4313f0ecf (after 2.11.1).
        // This leaves Debian 8 and 9 with broken support for this feature.
        // As a result, this feature should not be used until at least 2.11.91.
        // The broken support is mostly around not making all paths relative to the sysroot.
        // However, even at 2.13.1 it is possible to get a mix of sysroot and non-sysroot paths,
        // as any added file path not lexically starting with the sysroot will be unchanged.
        // To allow users to add local app files outside the sysroot,
        // prefer the sysroot but also look without the sysroot.
        if (!fSysroot.isEmpty()) {
            SkString resolvedFilename;
            resolvedFilename = fSysroot;
            resolvedFilename += filename;
            if (sk_exists(resolvedFilename.c_str(), kRead_SkFILE_Flag)) {
                return true;
            }
        }
        return sk_exists(filename, kRead_SkFILE_Flag);
    }

    static bool FontFamilyNameMatches(FcPattern* font, FcPattern* pattern) {
        return AnyStringMatching(font, pattern, FC_FAMILY);
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

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        FCLocker lock;

        SkAutoFcPattern pattern;
        FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)familyName);
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
        for (int setIndex = 0; setIndex < (int)std::size(fcNameSet); ++setIndex) {
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

        return sk_sp<SkFontStyleSet>(new StyleSet(sk_ref_sp(this), std::move(matches)));
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override
    {
        SkAutoFcPattern font([this, &familyName, &style]() {
            FCLocker lock;

            SkAutoFcPattern pattern;
            FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)familyName);
            fcpattern_from_skfontstyle(style, pattern);
            FcConfigSubstitute(fFC, pattern, FcMatchPattern);
            FcDefaultSubstitute(pattern);

            // We really want to match strong (preferred) and same (acceptable) only here.
            // If a family name was specified, assume that any weak matches after the last strong
            // match are weak (default) and ignore them.
            // After substitution the pattern for 'sans-serif' looks like "wwwwwwwwwwwwwwswww" where
            // there are many weak but preferred names, followed by defaults.
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
            if (!font || !FontAccessible(font) || !FontFamilyNameMatches(font, matchPattern)) {
                font.reset();
            }
            return font;
        }());
        return createTypefaceFromFcPattern(std::move(font));
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override
    {
        SkAutoFcPattern font([&](){
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
            if (!font || !FontAccessible(font) || !FontContainsCharacter(font, character)) {
                font.reset();
            }
            return font;
        }());
        return createTypefaceFromFcPattern(std::move(font));
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        const size_t length = stream->getLength();
        if (length <= 0 || (1u << 30) < length) {
            return nullptr;
        }
        return SkTypeface_FreeType::MakeFromStream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        return this->makeFromStream(SkStream::MakeFromFile(path), ttcIndex);
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override {
        sk_sp<SkTypeface> typeface(this->matchFamilyStyle(familyName, style));
        if (typeface) {
            return typeface;
        }

        return sk_sp<SkTypeface>(this->matchFamilyStyle(nullptr, style));
    }
};

sk_sp<SkFontMgr> SkFontMgr_New_FontConfig(FcConfig* fc) {
    return sk_make_sp<SkFontMgr_fontconfig>(fc);
}
