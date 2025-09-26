/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkFontMgr_android_ndk.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFeatures.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkSharedMutex.h"
#include "src/base/SkTSort.h"
#include "src/base/SkUTF.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTHash.h"
#include "src/ports/SkFontMgr_android_parser.h"
#include "src/ports/SkTypeface_proxy.h"

#include <unicode/uchar.h>
#include <unicode/ustring.h>

#if defined(SK_BUILD_FOR_ANDROID)
#include <android/api-level.h>
#else
#define __ANDROID_API__ 0
#define __ANDROID_API_Q__ 29
#define __ANDROID_API_R__ 30
#define __ANDROID_API_S__ 31
int android_get_device_api_level() { return __ANDROID_API__; };
#endif

using namespace skia_private;

/**
 * Technically, the AFont API was introduced in Android 10 (Q, API 29). However...
 *
 * The AFontMatcher API implementation is broken from its introduction until at least API 33. What
 * is desired is to find a font for the given locale which contains the given character. However,
 * the implementation actually attempts to shape the string passed to it with the default font and
 * then returns the font chosen for the first run. However, this produces undesireable results, as
 * it will always prefer the default font over the locale, so any code points covered by the default
 * font will always come from the default font regardless of the requested locale. In addition, this
 * will claim coverage for code points "made up" by the shaper through normalization,
 * denormalization, whitespace synthesis, no-draw synthesis, etc, for the default font, when there
 * may be better choices later in fallback.
 *
 * On Android 10 (Q, API 29) AFont_getLocale always returns nullptr (if there is a locale set) or
 * whatever std::unique_ptr<std::string>()->c_str() returns, which happens to be 0x1. As a result,
 * AFont_getLocale cannot be used until Android 11 (R, API 30). This is b/139201432 and fixed with
 * "Make AFont_getLocale work" [0]. This change is in Android 11 (API 30) but does not appear to
 * have been cherry-picked into Android 10 (Q, API 29).
 * [0]  https://cs.android.com/android/_/android/platform/frameworks/base/+/01709c7469b59e451f064c266bbe442e9bef0ab4
 *
 * As a result, there is no correct way to use locale information from the Android 10 NDK. So this
 * font manager only works with Android 11 (R, API 30) and above.
 */
#define SK_ANDROID_NDK_FONT_API_EXISTS __ANDROID_API_Q__
#define SK_ANDROID_NDK_FONT_API_LOCALE_WORKS __ANDROID_API_R__

#if __ANDROID_API__ >= SK_ANDROID_NDK_FONT_API_EXISTS
#include <android/font.h>
#include <android/font_matcher.h>
#include <android/system_fonts.h>
#endif

#include <cinttypes>
#include <memory>
#include <vector>

#include <dlfcn.h>

struct ASystemFontIterator;
struct AFont;

namespace {

[[maybe_unused]] static inline const constexpr bool kSkFontMgrVerbose = false;

namespace variation {
using Coordinate = SkFontArguments::VariationPosition::Coordinate;
using Storage = AutoSTArray<4, Coordinate>;

static constexpr SkFourByteTag wghtTag = SkSetFourByteTag('w','g','h','t');
static constexpr SkFourByteTag wdthTag = SkSetFourByteTag('w','d','t','h');
static constexpr SkFourByteTag slntTag = SkSetFourByteTag('s','l','n','t');
static constexpr SkFourByteTag italTag = SkSetFourByteTag('i','t','a','l');

static bool coordinateLess(const Coordinate& a, const Coordinate& b) {
    return a.axis != b.axis ? a.axis < b.axis : a.value < b.value;
};

static bool coordinateEqual(const Coordinate& a, const Coordinate& b) {
    return a.axis == b.axis && a.value == b.value;
};

static SkSpan<Coordinate> Get(const SkTypeface& typeface, Storage& storage) {
    if (storage.size() < Storage::kCount) {
        storage.reset(Storage::kCount);
    }
    int numAxes = typeface.getVariationDesignPosition(SkSpan(storage));
    if (SkToInt(storage.size()) < numAxes) {
        storage.reset(numAxes);
        numAxes = typeface.getVariationDesignPosition(SkSpan(storage));
    }
    if (numAxes < 0) {
        numAxes = 0;
    }

    return SkSpan(storage.data(), numAxes);
}

/* Normalize the values. NaN and -0.0 => 0.
 * Should normalize before sorting or comparing.
 */
static SkSpan<Coordinate> Normalize(SkSpan<Coordinate> variation) {
    for (auto&& coord : variation) {
        if (coord.value == 0 || SkIsNaN(coord.value)) {
            coord.value = 0.0f;
        }
    }
    return variation;
}

static SkSpan<Coordinate> Sort(SkSpan<Coordinate> variation) {
    SkTQSort(variation.begin(), variation.end(), variation::coordinateLess);
    return variation;
}

}  // namespace variation

struct AndroidFontAPI {
    ASystemFontIterator* (*ASystemFontIterator_open)();
    void (*ASystemFontIterator_close)(ASystemFontIterator*);
    AFont* (*ASystemFontIterator_next)(ASystemFontIterator*);

    void (*AFont_close)(AFont*);
    const char* (*AFont_getFontFilePath)(const AFont*);
    uint16_t (*AFont_getWeight)(const AFont*);
    bool (*AFont_isItalic)(const AFont*);
    const char* (*AFont_getLocale)(const AFont*);
    size_t (*AFont_getCollectionIndex)(const AFont*);
    size_t (*AFont_getAxisCount)(const AFont*);
    uint32_t (*AFont_getAxisTag)(const AFont*, uint32_t axisIndex);
    float (*AFont_getAxisValue)(const AFont*, uint32_t axisIndex);

#if __ANDROID_API__ >= SK_ANDROID_NDK_FONT_API_EXISTS

    static std::optional<AndroidFontAPI> Make() {
        static AndroidFontAPI api {
            ::ASystemFontIterator_open,
            ::ASystemFontIterator_close,
            ::ASystemFontIterator_next,

            ::AFont_close,
            ::AFont_getFontFilePath,
            ::AFont_getWeight,
            ::AFont_isItalic,
            ::AFont_getLocale,
            ::AFont_getCollectionIndex,
            ::AFont_getAxisCount,
            ::AFont_getAxisTag,
            ::AFont_getAxisValue,
        };
        if (android_get_device_api_level() < SK_ANDROID_NDK_FONT_API_LOCALE_WORKS) {
            return std::nullopt;
        }
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidFontAPI direct\n"); }
        return api;
    }

#else

private:
    AndroidFontAPI() {}
    std::unique_ptr<void, SkFunctionObject<dlclose>> self;
public:
    AndroidFontAPI(const AndroidFontAPI&) = delete;
    AndroidFontAPI& operator=(const AndroidFontAPI&) = delete;
    AndroidFontAPI(AndroidFontAPI&&) = default;
    AndroidFontAPI& operator=(AndroidFontAPI&&) = default;

    static std::optional<AndroidFontAPI> Make() {
        if (android_get_device_api_level() < SK_ANDROID_NDK_FONT_API_LOCALE_WORKS) {
            return std::nullopt;
        }

        AndroidFontAPI api;
        api.self.reset(dlopen("libandroid.so", RTLD_LAZY | RTLD_LOCAL));
        if (!api.self) {
            return std::nullopt;
        }

#define SK_DLSYM_ANDROID_FONT_API(NAME)                           \
        do {                                                      \
            *(void**)(&api.NAME) = dlsym(api.self.get(), #NAME);  \
            if (!api.NAME) {                                      \
                if constexpr (kSkFontMgrVerbose) {                \
                    SkDebugf("SKIA: Failed to load: " #NAME "\n");\
                }                                                 \
                return std::nullopt;                              \
            }                                                     \
        } while (0)

        SK_DLSYM_ANDROID_FONT_API(ASystemFontIterator_open);
        SK_DLSYM_ANDROID_FONT_API(ASystemFontIterator_close);
        SK_DLSYM_ANDROID_FONT_API(ASystemFontIterator_next);

        SK_DLSYM_ANDROID_FONT_API(AFont_close);
        SK_DLSYM_ANDROID_FONT_API(AFont_getFontFilePath);
        SK_DLSYM_ANDROID_FONT_API(AFont_getWeight);
        SK_DLSYM_ANDROID_FONT_API(AFont_isItalic);
        SK_DLSYM_ANDROID_FONT_API(AFont_getLocale);
        SK_DLSYM_ANDROID_FONT_API(AFont_getCollectionIndex);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisCount);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisTag);
        SK_DLSYM_ANDROID_FONT_API(AFont_getAxisValue);
#undef SK_DLSYM_ANDROID_FONT_API

        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidFontAPI dlsym\n"); }
        return api;
    }

#endif
};

struct AndroidIcuAPI {
    /* The result is case folded UTF-8. This is normalized case, it isn't upper or lower case. */
    SkString casefold(SkSpan<const char> s) const {
        return fHasICU ? this->icuCaseFold(s) : this->cToWLower(s);
    }

private:
    int32_t (*u_strFoldCase)(UChar *dest, int32_t destCapacity, const UChar *src, int32_t srcLength,
                             uint32_t options, UErrorCode *pErrorCode) = nullptr;
    UChar* (*u_strFromUTF8)(UChar *dest, int32_t destCapacity, int32_t *pDestLength,
                            const char *src, int32_t srcLength, UErrorCode *pErrorCode) = nullptr;
    char * (*u_strToUTF8)(char *dest, int32_t destCapacity, int32_t *pDestLength,
                          const UChar *src, int32_t srcLength, UErrorCode *pErrorCode) = nullptr;
    bool fHasICU = false;

    SkString cToWLower(SkSpan<const char> s) const {
        // Find length of result
        size_t retLen = 0;
        bool isDifferent = false;
        const char* src = s.begin();
        while (src != s.end()) {
            SkUnichar uni = SkUTF::NextUTF8(&src, s.end());
            if (uni < 0) {
                return SkString(s.data(), s.size());
            }
            // On Android 2.3 (API 9) and later wchar_t is 32 bit, UTF-32 like.
            // towlower only provides simple case folding, but this should be fine for Android 11.
            wint_t wlow = towlower(uni);
            char buffer[SkUTF::kMaxBytesInUTF8Sequence];
            size_t buflen = SkUTF::ToUTF8(wlow, buffer);
            if (buflen == 0) {
                return SkString(s.data(), s.size());
            }
            isDifferent |= wlow != SkToU32(uni);
            retLen += buflen;
        }

        // No change needed
        if (!isDifferent) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: TL \"%s\" unchanged\n", s.data()); }
            return SkString(s.data(), s.size());
        }

        // Create result
        SkString ret(retLen);
        src = s.begin();
        char* dst = ret.begin();
        while (src != s.end()) {
            SkUnichar uni = SkUTF::NextUTF8(&src, s.end());
            wint_t wlow = towlower(uni);
            char buffer[SkUTF::kMaxBytesInUTF8Sequence];
            size_t buflen = SkUTF::ToUTF8(wlow, buffer);
            memcpy(dst, buffer, buflen);
            dst += buflen;
        }

        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: TL \"%s\" to \"%s\"\n", s.data(), ret.c_str());
        }
        return ret;
    }

    SkString icuCaseFold(SkSpan<const char> s) const {
        if (!SkTFitsIn<int32_t>(s.size())) {
            return SkString(s.data(), s.size());
        }

        using Storage = AutoSTMalloc<32, UChar>;
        UErrorCode error = U_ZERO_ERROR;

        // Convert to UTF-16
        int32_t uStrSize = 0;
        Storage uStr(Storage::kCount);
        this->u_strFromUTF8(uStr, Storage::kCount, &uStrSize, s.data(), s.size(), &error);
        if (error == U_BUFFER_OVERFLOW_ERROR) {
            error = U_ZERO_ERROR;
        }
        if (U_FAILURE(error)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-16 length\n"); }
            return SkString(s.data(), s.size());
        }
        if (SkTo<int32_t>(Storage::kCount) < uStrSize) {
            uStr.reset(uStrSize);
            int32_t uStrSizePrev = uStrSize;
            this->u_strFromUTF8(uStr, uStrSize, &uStrSize, s.data(), s.size(), &error);
            if (U_FAILURE(error) || uStrSizePrev < uStrSize) {
                if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-16 wrong length\n"); }
                return SkString(s.data(), s.size());
            }
        }

        // Case fold
        Storage uStrFolded(Storage::kCount);
        int32_t uStrFoldedSize = this->u_strFoldCase(uStrFolded, Storage::kCount, uStr, uStrSize,
                                                     U_FOLD_CASE_EXCLUDE_SPECIAL_I, &error);
        if (error == U_BUFFER_OVERFLOW_ERROR) {
            error = U_ZERO_ERROR;
        }
        if (U_FAILURE(error)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF folded length\n"); }
            return SkString(s.data(), s.size());
        }
        if (SkTo<int32_t>(Storage::kCount) < uStrFoldedSize) {
            uStrFolded.reset(uStrFoldedSize);
            int32_t uStrFoldedSizePrev = uStrFoldedSize;
            uStrFoldedSize = this->u_strFoldCase(uStrFolded, uStrFoldedSize, uStr, uStrSize,
                                                 U_FOLD_CASE_EXCLUDE_SPECIAL_I, &error);
            if (U_FAILURE(error) || uStrFoldedSizePrev < uStrFoldedSize) {
                if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF folded wrong length\n"); }
                return SkString(s.data(), s.size());
            }
        }

        // Convert to UTF-8
        SkString ret;
        int32_t retSize = 0;
        this->u_strToUTF8(nullptr, 0, &retSize, uStrFolded, uStrFoldedSize, &error);
        if (error == U_BUFFER_OVERFLOW_ERROR) {
            error = U_ZERO_ERROR;
        }
        if (U_FAILURE(error)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-8 length\n"); }
            return SkString(s.data(), s.size());
        }
        ret.resize(retSize);
        int32_t retSizePrev = retSize;
        this->u_strToUTF8(ret.data(), retSize, &retSize, uStrFolded, uStrFoldedSize, &error);
        if (U_FAILURE(error) || retSizePrev != retSize) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: CF UTF-8 wrong length\n"); }
            return SkString(s.data(), s.size());
        }

        // Return result
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: CF \"%s\" to \"%s\"\n", s.data(), ret.c_str());
        }
        return ret;
    }

#if __ANDROID_API__ >= __ANDROID_API_S__
public:
    AndroidIcuAPI()
        : u_strFoldCase(::u_strFoldCase)
        , u_strFromUTF8(::u_strFromUTF8)
        , u_strToUTF8(::u_strToUTF8)
        , fHasICU(true)
    {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidIcuAPI direct\n"); }
    }

#else

private:
    std::unique_ptr<void, SkFunctionObject<dlclose>> fSelf;
public:
    AndroidIcuAPI(const AndroidIcuAPI&) = delete;
    AndroidIcuAPI& operator=(const AndroidIcuAPI&) = delete;
    AndroidIcuAPI(AndroidIcuAPI&&) = default;
    AndroidIcuAPI& operator=(AndroidIcuAPI&&) = default;

    AndroidIcuAPI() {
        fSelf.reset(dlopen("libicu.so", RTLD_LAZY | RTLD_LOCAL));
        if (!fSelf) {
            return;
        }

#define SK_DLSYM_ANDROID_ICU_API(NAME)                            \
        do {                                                      \
            *(void**)(&NAME) = dlsym(fSelf.get(), #NAME);         \
            if (!NAME) {                                          \
                if constexpr (kSkFontMgrVerbose) {                \
                    SkDebugf("SKIA: Failed to load: " #NAME "\n");\
                }                                                 \
                return;                                           \
            }                                                     \
        } while (0)

        SK_DLSYM_ANDROID_ICU_API(u_strFoldCase);
        SK_DLSYM_ANDROID_ICU_API(u_strFromUTF8);
        SK_DLSYM_ANDROID_ICU_API(u_strToUTF8);
#undef SK_DLSYM_ANDROID_ICU_API

        fHasICU = true;
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidIcuAPI dlsym\n"); }
    }
#endif
};

// bcp47 is ascii only and only does 1:1 replacements for case folding.
static void normalizeAsciiCase(SkSpan<char> s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](char c){ return (c < 'A' || 'Z' < c) ? c : c + 'a' - 'A'; });
}

struct SkAFont {
    SkAFont(const AndroidFontAPI& api, AFont* font) : fAPI(api), fFont(font) {}
    SkAFont(SkAFont&& that) : fAPI(that.fAPI), fFont(that.fFont) {
        that.fFont = nullptr;
    }
    ~SkAFont() {
        if (fFont) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: AFont_close\n"); }
            fAPI.AFont_close(fFont);
        }
    }
    explicit operator bool() { return fFont; }

    const char* getFontFilePath() const { return fAPI.AFont_getFontFilePath(fFont); }
    uint16_t getWeight() const { return fAPI.AFont_getWeight(fFont); }
    bool isItalic() const { return fAPI.AFont_isItalic(fFont); }
    const char* getLocale() const { return fAPI.AFont_getLocale(fFont); }
    size_t getCollectionIndex() const { return fAPI.AFont_getCollectionIndex(fFont); }
    size_t getAxisCount() const { return fAPI.AFont_getAxisCount(fFont); }
    uint32_t getAxisTag(uint32_t index) const { return fAPI.AFont_getAxisTag(fFont, index); }
    float getAxisValue(uint32_t index) const { return fAPI.AFont_getAxisValue(fFont, index); }

private:
    const AndroidFontAPI& fAPI;
    AFont* fFont;
};

struct SkASystemFontIterator {
    SkASystemFontIterator(const AndroidFontAPI& api)
        : fAPI(api)
        , fIterator(fAPI.ASystemFontIterator_open())
    {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: ASystemFontIterator_open\n"); }
    }
    SkASystemFontIterator(SkASystemFontIterator&&) = default;
    ~SkASystemFontIterator() {
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: ASystemFontIterator_close\n"); }
        fAPI.ASystemFontIterator_close(fIterator);
    }
    explicit operator bool() { return fIterator; }

    SkAFont next() { return SkAFont(fAPI, fAPI.ASystemFontIterator_next(fIterator)); }

private:
    const AndroidFontAPI& fAPI;
    ASystemFontIterator* const fIterator;
};

class SkALanguage {
public:
    SkALanguage() {}
    SkALanguage(const char* tag) : SkALanguage(SkSpan<const char>(tag, tag ? strlen(tag) : 0)) {}
    SkALanguage(SkSpan<const char> tag) {
        fLanguage = consumeSubtag(tag);
        if (fLanguage.equals("und", 3)) {
            fLanguage = SkString();
        }
        fScript = consumeSubtag(tag);
        fRegion = consumeSubtag(tag);
    }
    SkALanguage(const SkALanguage&) = default;
    SkALanguage& operator=(const SkALanguage& b) = default;

    SkALanguage lessSpecific() const {
        SkALanguage lessSpecific(*this);
        if (!lessSpecific.fRegion.isEmpty()) {
            lessSpecific.fRegion = SkString();
            return lessSpecific;
        }
        if (!lessSpecific.fScript.isEmpty()) {
            lessSpecific.fScript = SkString();
            return lessSpecific;
          }

        if (!lessSpecific.fLanguage.isEmpty()) {
            lessSpecific.fLanguage = SkString();
            return lessSpecific;
        }
        return lessSpecific;
    }

    bool isEmpty() const {
        return fLanguage.isEmpty() && fScript.isEmpty() && fRegion.isEmpty();
    }

    bool startsWith(const SkALanguage& that) {
        return (that.fLanguage.isEmpty() || fLanguage == that.fLanguage) &&
               (that.fScript.isEmpty() || fScript == that.fScript) &&
               (that.fRegion.isEmpty() || fRegion == that.fRegion);
    }

    const SkString& getLanguage() const { return fLanguage; }
    const SkString& getScript() const { return fScript; }
    const SkString& getRegion() const { return fRegion; }


    using sk_is_trivially_relocatable = std::true_type;
private:
    SkString consumeSubtag(SkSpan<const char>& tag) {
        if (tag.size() == 0) {
            return SkString();
        }
        const char* subtagEnd = static_cast<const char*>(memchr(tag.data(), '-', tag.size()));
        if (!subtagEnd) {
            SkString ret(tag.data(), tag.size());
            tag = SkSpan<const char>();
            return ret;
        }
        size_t subtagLength = subtagEnd - tag.data();
        SkString ret(tag.data(), subtagLength);
        normalizeAsciiCase({ret.begin(), ret.size()});
        tag = tag.subspan(subtagLength + 1);
        return ret;
    }
    SkString fLanguage;
    SkString fScript;
    SkString fRegion;
    // variants, extensions, and privateuses are ignored as no known font configuration uses them.
    static_assert(::sk_is_trivially_relocatable<decltype(fLanguage)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fScript)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fRegion)>::value);
};

class SkTypeface_AndroidNDK : public SkTypeface_proxy {
public:
    struct AutoAxis {
        static constexpr struct KnownAxis {
            SkFourByteTag tag;
            int flag;
        } kKnownAxis[] = {
            { variation::wghtTag, 1 << 0 },
            { variation::wdthTag, 1 << 1 },
            { variation::slntTag, 1 << 2 },
            { variation::italTag, 1 << 3 },
        };

        AutoAxis() = default;
        AutoAxis(const AutoAxis&) = default;
        AutoAxis& operator=(const AutoAxis&) = default;
        AutoAxis(SkSpan<const SkFontArguments::VariationPosition::Coordinate> pos) {
            for (auto&& coord : pos) {
                for (auto&& axis : kKnownAxis) {
                    if (coord.axis == axis.tag) {
                        fFlags |= axis.flag;
                    }
                }
            }
        }

        void remove(const AutoAxis& that) {
            fFlags &= ~that.fFlags;
        }

        bool weight() const { return SkToBool(fFlags & kKnownAxis[0].flag); }
        bool width() const { return SkToBool(fFlags & kKnownAxis[1].flag); }
        bool slant() const { return SkToBool(fFlags & kKnownAxis[2].flag); }
        bool italic() const { return SkToBool(fFlags & kKnownAxis[3].flag); }
        bool none() const { return fFlags == 0; }
        uint8_t fFlags = 0;
    };
    static sk_sp<SkTypeface_AndroidNDK> Make(sk_sp<SkTypeface> realTypeface,
                                             const SkFontStyle& style,
                                             bool isFixedPitch,
                                             const SkString& familyName,
                                             TArray<SkString>&& extraFamilyNames,
                                             TArray<SkALanguage>&& lang,
                                             const AutoAxis& autoAxis) {
        SkASSERT(realTypeface);
        return sk_sp<SkTypeface_AndroidNDK>(new SkTypeface_AndroidNDK(std::move(realTypeface),
                                                                      style,
                                                                      isFixedPitch,
                                                                      familyName,
                                                                      std::move(extraFamilyNames),
                                                                      std::move(lang),
                                                                      autoAxis));
    }

private:
    SkTypeface_AndroidNDK(sk_sp<SkTypeface> realTypeface,
                          const SkFontStyle& style,
                          bool isFixedPitch,
                          const SkString& familyName,
                          TArray<SkString>&& extraFamilyNames,
                          TArray<SkALanguage>&& lang,
                          const AutoAxis& autoAxis)
        : SkTypeface_proxy(std::move(realTypeface), style, isFixedPitch)
        , fFamilyName(familyName)
        , fExtraFamilyNames(std::move(extraFamilyNames))
        , fLang(std::move(lang))
        , fAutoAxis(autoAxis)
    { }

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkASSERT(desc);
        SkASSERT(serialize);
        SkTypeface_proxy::onGetFontDescriptor(desc, serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setStyle(this->fontStyle());
        *serialize = false;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        auto proxy = SkTypeface_proxy::onMakeClone(args);
        if (proxy == nullptr) {
            return nullptr;
        }
        SkFontStyle style = proxy->fontStyle();
        bool fixedPitch = proxy->isFixedPitch();
        return SkTypeface_AndroidNDK::Make(
                std::move(proxy),
                style,
                fixedPitch,
                fFamilyName,
                TArray<SkString>(fExtraFamilyNames),
                TArray<SkALanguage>(),
                AutoAxis());
    }

    SkFontStyle onGetFontStyle() const override {
        return SkTypeface::onGetFontStyle();
    }

    bool onGetFixedPitch() const override {
        return SkTypeface::onGetFixedPitch();
    }

    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        class ALocalizedStrings : public SkTypeface::LocalizedStrings {
        public:
            ALocalizedStrings(sk_sp<SkTypeface_AndroidNDK> typeface,
                              sk_sp<SkTypeface::LocalizedStrings> base)
                : fTypeface(std::move(typeface))
                , fBase(std::move(base))
                , fExtraFamilyName(nullptr) {}
        private:
            sk_sp<SkTypeface_AndroidNDK> fTypeface;
            sk_sp<SkTypeface::LocalizedStrings> fBase;
            const SkString* fExtraFamilyName;

            bool next(LocalizedString* localizedString) override {
                if (!fExtraFamilyName) {
                    if (fBase->next(localizedString)) {
                        return true;
                    }
                    fExtraFamilyName = fTypeface->fExtraFamilyNames.begin();
                }
                if (fExtraFamilyName == fTypeface->fExtraFamilyNames.end()) {
                    return false;
                }
                *localizedString = {*fExtraFamilyName, SkString()};
                ++fExtraFamilyName;
                return true;
            }
        };
        sk_sp<SkTypeface::LocalizedStrings> base(SkTypeface_proxy::onCreateFamilyNameIterator());
        return new ALocalizedStrings(sk_ref_sp(this), std::move(base));
    }

public:
    const SkString fFamilyName;
    const TArray<SkString> fExtraFamilyNames;
    const STArray<4, SkALanguage> fLang;
    const AutoAxis fAutoAxis;
};

class TypefaceCache : public SkRefCnt {
public:
    TypefaceCache() : fRequests(64), fMatches(64) {}
    ~TypefaceCache() override {}

    class Request {
    public:
        Request(SkTypefaceID id, SkFontStyle style) : fId(id), fStyle(style) {
            SkGoodHash hasher;
            fHash = hasher(fId);
            fHash ^= hasher(fStyle);
        }
        bool operator==(const Request& that) const {
            return fId == that.fId && fStyle == that.fStyle;
        }
        struct Hash { uint32_t operator()(const Request& a) { return a.fHash; } };
    private:
        const SkTypefaceID fId;
        const SkFontStyle fStyle;
        uint32_t fHash;
    };
    sk_sp<SkTypeface> find(const Request& request) {
        SkAutoSharedMutexShared lock(fMutex);
        sk_sp<SkTypeface>* typeface = fRequests.find(request);
        if (typeface) {
            return *typeface;
        }
        return nullptr;
    }
    void add(const Request& request, sk_sp<SkTypeface> typeface) {
        SkAutoSharedMutexExclusive lock(fMutex);
        fRequests.insert_or_update(request, std::move(typeface));
    }

    class Match {
    public:
        /* variation is expected to be normalized and sorted. */
        Match(SkTypefaceID id, variation::Storage&& variation)
            : fId(id)
            , fVariation(std::move(variation))
        {
            SkGoodHash hasher;
            fHash = hasher(id);
            for (auto&& coord : fVariation) {
                fHash ^= hasher(coord.axis);
                fHash ^= hasher(FloatBits(coord.value));
            }
        }
        Match(const Match&) = delete;
        Match& operator=(const Match&) = delete;
        Match(Match&& that) : fId(std::move(that.fId))
                            , fVariation(std::move(that.fVariation))
                            , fHash(std::move(that.fHash)) {}
        Match& operator=(Match&&) = delete;
        bool operator==(const Match& that) const {
            return fId == that.fId &&
                   fVariation.size() == that.fVariation.size() &&
                   std::equal(fVariation.begin(), fVariation.end(),
                              that.fVariation.begin(), variation::coordinateEqual);
        }
        struct Hash { uint32_t operator()(const Match& a) { return a.fHash; } };
    private:
        static uint32_t FloatBits(float f) {
            static_assert(sizeof(uint32_t) == sizeof(float));
            uint32_t bits;
            std::memcpy(&bits, &f, sizeof(uint32_t));
            return bits;
        }
        const SkTypefaceID fId;
        variation::Storage fVariation;
        uint32_t fHash;
    };
    sk_sp<SkTypeface> find(const Match& match) {
        SkAutoSharedMutexShared lock(fMutex);
        sk_sp<SkTypeface>* typeface = fMatches.find(match);
        if (typeface) {
            return *typeface;
        }
        return nullptr;
    }
    void add(Match&& match, sk_sp<SkTypeface> typeface) {
        SkAutoSharedMutexExclusive lock(fMutex);
        fMatches.insert_or_update(std::move(match), typeface);
    }
private:
    SkLRUCache<Request, sk_sp<SkTypeface>, Request::Hash> fRequests;
    SkLRUCache<Match, sk_sp<SkTypeface>, Match::Hash> fMatches;
    SkSharedMutex fMutex;
};

sk_sp<SkTypeface> adjustForStyle(sk_sp<SkTypeface_AndroidNDK>&& typeface, SkFontStyle style,
                                 TypefaceCache& cache) {
    if (!typeface) {
        return std::move(typeface);
    }

    SkFontStyle typefaceStyle = typeface->fontStyle();
    if (typefaceStyle == style || typeface->fAutoAxis.none()) {
        return std::move(typeface);
    }

    SkFontArguments::VariationPosition::Coordinate coord[4];
    int numCoords = 0;
    if (typefaceStyle.weight() != style.weight() && typeface->fAutoAxis.weight()) {
        coord[numCoords++] = {variation::wghtTag, static_cast<float>(style.weight())};
    }
    if (typefaceStyle.width() != style.width() && typeface->fAutoAxis.width()) {
        coord[numCoords++] = {variation::wdthTag,
                              SkFontDescriptor::SkFontWidthAxisValueForStyleWidth(style.width())};
    }
    if (typefaceStyle.slant() != style.slant()) {
        switch (style.slant()) {
        case SkFontStyle::Slant::kItalic_Slant:
            if (typeface->fAutoAxis.italic()) {
                coord[numCoords++] = {variation::italTag, 1.0};
            }
            break;
        case SkFontStyle::Slant::kOblique_Slant:
            if (typeface->fAutoAxis.slant()) {
                coord[numCoords++] = {variation::slntTag, -7.0};
            }
            break;
        case SkFontStyle::Slant::kUpright_Slant:
            if (typeface->fAutoAxis.italic()) {
                coord[numCoords++] = {variation::italTag, 0.0};
            }
            if (typeface->fAutoAxis.slant()) {
                coord[numCoords++] = {variation::slntTag, 0.0};
            }
            break;
        }
    }
    if (numCoords == 0) {
        return std::move(typeface);
    }

    TypefaceCache::Request request(typeface->uniqueID(), style);
    if (sk_sp<SkTypeface> cachedTypeface = cache.find(request)) {
        if constexpr (kSkFontMgrVerbose) {
            SkString familyName;
            typeface->getFamilyName(&familyName);
            SkFontStyle s = cachedTypeface->fontStyle();
            SkDebugf("Cached request of \"%s\" weight:%d width: %d slant %d\n",
                     familyName.c_str(), s.weight(), s.width(), s.slant());
        }
        return cachedTypeface;
    }

    sk_sp<SkTypeface> newTypeface = typeface->makeClone(
        SkFontArguments().setVariationDesignPosition({coord, numCoords}));
    if (!newTypeface) {
        if constexpr (kSkFontMgrVerbose) {
            SkString familyName;
            typeface->getFamilyName(&familyName);
            SkDebugf("Failed to clone \"%s\"\n", familyName.c_str());
        }
        return std::move(typeface);
    }

    variation::Storage variationStorage;
    SkSpan<variation::Coordinate> newVariation = variation::Get(*newTypeface, variationStorage);
    variation::Sort(variation::Normalize(newVariation));
    variationStorage.trimTo(newVariation.size());
    TypefaceCache::Match match(typeface->uniqueID(), std::move(variationStorage));
    if (sk_sp<SkTypeface> cachedTypeface = cache.find(match)) {
        if constexpr (kSkFontMgrVerbose) {
            SkString familyName;
            typeface->getFamilyName(&familyName);
            SkFontStyle s = cachedTypeface->fontStyle();
            SkDebugf("Cached match of \"%s\" weight:%d width: %d slant %d\n",
                     familyName.c_str(), s.weight(), s.width(), s.slant());
        }
        cache.add(std::move(request), cachedTypeface);
        return cachedTypeface;
    }

    if constexpr (kSkFontMgrVerbose) {
        SkString familyName;
        typeface->getFamilyName(&familyName);
        SkFontStyle s = newTypeface->fontStyle();
        SkDebugf("New variant of \"%s\" weight:%d width: %d slant %d\n",
                 familyName.c_str(), s.weight(), s.width(), s.slant());
    }
    cache.add(std::move(match), newTypeface);
    cache.add(std::move(request), newTypeface);
    return newTypeface;
}

class SkFontStyleSet_AndroidNDK : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_AndroidNDK(sk_sp<TypefaceCache> cache) : fCache(std::move(cache)) {}

    int count() override {
        return fStyles.size();
    }
    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (index < 0 || fStyles.size() <= index) {
            return;
        }
        if (style) {
            *style = fStyles[index]->fontStyle();
        }
        if (name) {
            name->reset();
        }
    }
    sk_sp<SkTypeface_AndroidNDK> createATypeface(int index) {
        if (index < 0 || fStyles.size() <= index) {
            return nullptr;
        }
        return fStyles[index];
    }
    sk_sp<SkTypeface> createTypeface(int index) override {
        return createATypeface(index);
    }

    sk_sp<SkTypeface_AndroidNDK> matchAStyle(const SkFontStyle& pattern) {
        sk_sp<SkTypeface> match = this->matchStyleCSS3(pattern);
        sk_sp<SkTypeface_AndroidNDK> amatch(static_cast<SkTypeface_AndroidNDK*>(match.release()));
        if constexpr (kSkFontMgrVerbose) {
            SkString name;
            amatch->getFamilyName(&name);
            SkString resourceName;
            amatch->getResourceName(&resourceName);
            SkFontStyle fontStyle = amatch->fontStyle();
            SkDebugf("SKIA: Search for [%d, %d, %d] matched %s [%d, %d, %d] %s\n",
                     pattern.weight(), pattern.width(), pattern.slant(),
                     name.c_str(), fontStyle.weight(), fontStyle.width(), fontStyle.slant(),
                     resourceName.c_str());
        }
        return amatch;
    }
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        return adjustForStyle(this->matchAStyle(pattern), pattern, *fCache);
    }

private:
    TArray<sk_sp<SkTypeface_AndroidNDK>> fStyles;
    sk_sp<TypefaceCache> fCache;
    friend class SkFontMgr_AndroidNDK;
};

struct NameToFamily {
    SkString name;
    SkString normalizedName;
    SkFontStyleSet_AndroidNDK* styleSet;

    using sk_is_trivially_relocatable = std::true_type;
    static_assert(::sk_is_trivially_relocatable<decltype(name)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(normalizedName)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(styleSet)>::value);
};

class SkFontMgr_AndroidNDK : public SkFontMgr {
    void addSystemTypeface(sk_sp<SkTypeface_AndroidNDK> typeface, const SkString& name) {
        NameToFamily* nameToFamily = nullptr;
        SkString normalizedName(fICU.casefold({name.data(), name.size()}));
        for (NameToFamily& current : fNameToFamilyMap) {
            if (current.normalizedName == normalizedName) {
                nameToFamily = &current;
                break;
            }
        }
        if (!nameToFamily) {
            sk_sp<SkFontStyleSet_AndroidNDK> newSet(new SkFontStyleSet_AndroidNDK(fCache));
            nameToFamily = &fNameToFamilyMap.emplace_back(
                NameToFamily{name, normalizedName, newSet.get()});
            fStyleSets.push_back(std::move(newSet));
        }
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Adding member to %s\n", name.c_str()); }
        nameToFamily->styleSet->fStyles.push_back(typeface);
    }

public:
    SkFontMgr_AndroidNDK(AndroidFontAPI&& fontAPI, bool const cacheFontFiles,
                         std::unique_ptr<SkFontScanner> scanner)
        : fAPI(std::move(fontAPI))
        , fScanner(std::move(scanner))
        , fCache(new TypefaceCache())
    {
        SkASystemFontIterator fontIter(fAPI);
        if (!fontIter) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: No ASystemFontIterator"); }
            return;
        }

        std::vector<std::unique_ptr<FontFamily>> xmlFamilies;
        SkFontMgr_Android_Parser::GetSystemFontFamilies(xmlFamilies);

        skia_private::THashMap<SkString, std::unique_ptr<SkStreamAsset>> streamForPath;

        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Iterating over AFonts\n"); }
        while (SkAFont font = fontIter.next()) {
            sk_sp<SkTypeface_AndroidNDK> typeface = this->make(font, xmlFamilies, streamForPath);
            if (!typeface) {
                continue;
            }

            SkString name;
            typeface->getFamilyName(&name);
            this->addSystemTypeface(typeface, name);

            // A font may have many localized family names.
            sk_sp<SkTypeface::LocalizedStrings> names(typeface->createFamilyNameIterator());
            SkTypeface::LocalizedString localeName;
            while (names->next(&localeName)) {
                if (localeName.fString != name) {
                    this->addSystemTypeface(typeface, localeName.fString);
                }
            }
        }

        if (fStyleSets.empty()) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: No fonts!"); }
        } else {
            this->findDefaultStyleSet();
        }
    }

protected:
    /** Returns not how many families we have, but how many unique names
     *  exist among the families.
     */
    int onCountFamilies() const override {
        return fNameToFamilyMap.size();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if (index < 0 || fNameToFamilyMap.size() <= index) {
            familyName->reset();
            return;
        }
        familyName->set(fNameToFamilyMap[index].name);
    }

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        if (index < 0 || fNameToFamilyMap.size() <= index) {
            return nullptr;
        }
        return sk_ref_sp(fNameToFamilyMap[index].styleSet);
    }

    sk_sp<SkFontStyleSet_AndroidNDK> onMatchAFamily(const char familyName[]) const {
        if (!familyName) {
            return nullptr;
        }
        SkString normalizedFamilyName(fICU.casefold({familyName, strlen(familyName)}));
        for (const NameToFamily& nameToFamily : fNameToFamilyMap) {
            if (nameToFamily.normalizedName == normalizedFamilyName) {
                return sk_ref_sp(nameToFamily.styleSet);
            }
        }
        return nullptr;
    }
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        return this->onMatchAFamily(familyName);
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override
    {
        sk_sp<SkFontStyleSet_AndroidNDK> sset(this->onMatchAFamily(familyName));
        if (!sset) {
            return nullptr;
        }
        return sset->matchStyle(style);
    }

    static TArray<SkString> GetExtraFamilyNames(
        const SkAFont& font, const SkTypeface& typeface,
        const SkSpan<const std::unique_ptr<FontFamily>> xmlFamilies)
    {
        // The NDK does not report aliases like 'serif', 'sans-serif`, 'monospace', etc.
        // If a font matches an entry in fonts.xml, add the fonts.xml family name as well.

        // In Android <= 14 AFont reports the variation as specified in fonts.xml.
        // In Android >= 15 AFont only reports fixed axes.
        variation::Storage variationStorage;
        SkSpan<variation::Coordinate> variation = variation::Get(typeface, variationStorage);
        variation::Sort(variation::Normalize(variation));

        variation::Storage xmlVariationStorage;

        TArray<SkString> extraFamilyNames;
        for (const std::unique_ptr<FontFamily>& xmlFamily : xmlFamilies) {
            if (xmlFamily->fNames.empty()) {
                continue;
            }

            for (const FontFileInfo& xmlFont : xmlFamily->fFonts) {
                SkString pathName(xmlFamily->fBasePath);
                pathName.append(xmlFont.fFileName);
                if (!pathName.equals(font.getFontFilePath())) {
                    continue;
                }

                if (font.getCollectionIndex() != static_cast<size_t>(xmlFont.fIndex)) {
                    continue;
                }

                if (!xmlFont.fTypeface) {
                    xmlFont.fTypeface = typeface.makeClone(SkFontArguments()
                        .setCollectionIndex(xmlFont.fIndex)
                        .setVariationDesignPosition(SkFontArguments::VariationPosition{
                            xmlFont.fVariationDesignPosition.data(),
                            xmlFont.fVariationDesignPosition.size()
                        })
                    );
                }
                if (!xmlFont.fTypeface) {
                    SkDEBUGFAIL("Cannot create clone.");
                    continue;
                }

                SkSpan<variation::Coordinate> xmlVariation =
                        variation::Get(*xmlFont.fTypeface, xmlVariationStorage);
                if (variation.size() != xmlVariation.size()) {
                    SkDEBUGFAIL("Clone does not have same number of axes.");
                    continue;
                }

                variation::Sort(variation::Normalize(xmlVariation));
                if (!std::equal(variation.begin(), variation.end(),
                                xmlVariation.begin(), variation::coordinateEqual))
                {
                    continue;
                }

                if (xmlFont.fWeight != 0 && xmlFont.fWeight != font.getWeight()) {
                    continue;
                }

                for (auto&& xmlName : xmlFamily->fNames) {
                    extraFamilyNames.push_back(xmlName);
                }
            }
        }
        return extraFamilyNames;
    }

    sk_sp<SkTypeface_AndroidNDK> make(
        const SkAFont& font,
        const SkSpan<const std::unique_ptr<FontFamily>> xmlFamilies,
        skia_private::THashMap<SkString, std::unique_ptr<SkStreamAsset>>& streamForPath) const
    {
        SkString filePath(font.getFontFilePath());

        std::unique_ptr<SkStreamAsset>* streamPtr = streamForPath.find(filePath);
        if (!streamPtr) {
            streamPtr = streamForPath.set(filePath, SkStream::MakeFromFile(filePath.c_str()));
        }
        if (!*streamPtr) {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Font file %s cannot be opened.\n", filePath.c_str());
            }
            return nullptr;
        }
        std::unique_ptr<SkStreamAsset> stream = (*streamPtr)->duplicate();
        if (!stream) {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Font file %s could not be duplicated.\n", filePath.c_str());
            }
            return nullptr;
        }

        size_t collectionIndex = font.getCollectionIndex();
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: Making font from %s#%zu\n", filePath.c_str(), collectionIndex);
        }
        if (!SkTFitsIn<int>(collectionIndex)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Collection index invalid!"); }
            return nullptr;
        }

        size_t requestAxisCount = font.getAxisCount();
        if (!SkTFitsIn<int>(requestAxisCount)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Axis count unreasonable!"); }
            return nullptr;
        }
        variation::Storage requestAxisValues(requestAxisCount);
        std::optional<int> requestedWidth;
        for (size_t i = 0; i < requestAxisCount; ++i) {
            uint32_t tag = font.getAxisTag(i);
            float value = font.getAxisValue(i);
            requestAxisValues[i] = { tag, value };
            if (tag == variation::wdthTag) {
                // Set the width based on the requested `wdth` axis value.
                requestedWidth = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(value);
            }
        }
        SkFontArguments::VariationPosition requestedPosition = {
                requestAxisValues.get(), SkTo<int>(requestAxisCount)
        };
        // TODO: this creates the proxy with the given stream, so always cacheFontFiles.
        auto proxy = fScanner->MakeFromStream(
                std::move(stream),
                SkFontArguments()
                        .setCollectionIndex(collectionIndex)
                        .setVariationDesignPosition(requestedPosition));
        if (!proxy) {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Font file %s exists, but is not a valid font.\n", filePath.c_str());
            }
            return nullptr;
        }
        variation::Storage variationStorage;
        SkSpan<variation::Coordinate> variation = variation::Get(*proxy, variationStorage);
        SkTypeface_AndroidNDK::AutoAxis autoAxis(variation);
        autoAxis.remove(SkTypeface_AndroidNDK::AutoAxis(SkSpan(requestedPosition.coordinates,
                                                               requestedPosition.coordinateCount)));

        SkFontStyle style = proxy->fontStyle();
        int weight = SkTo<int>(font.getWeight());
        SkFontStyle::Slant slant = style.slant();
        if (font.isItalic()) {
            slant = SkFontStyle::kItalic_Slant;
        }
        int width = requestedWidth.value_or(style.width());
        style = SkFontStyle(weight, width, slant);

        // The family name(s) are not reported.
        // This would be very helpful for aliases, like "sans-serif", "Arial", etc.
        TArray<SkString> extraFamilyNames = GetExtraFamilyNames(font, *proxy, xmlFamilies);

        SkString familyName;
        proxy->getFamilyName(&familyName);

        STArray<4, SkALanguage> skLangs;
        const char* aLangs = font.getLocale();
        {
            SkString postscriptName;
            proxy->getPostScriptName(&postscriptName);

            // HACK: For backwards compatibility NotoSansSymbols-Regular-Subsetted needs "und-Zsym".
            // Base Android appears to hack this into its fallback list for similar reasons.
            static constexpr char kNotoSansSymbols[] = "NotoSansSymbols-Regular-Subsetted";
            if (postscriptName.equals(kNotoSansSymbols, std::size(kNotoSansSymbols)-1) &&
                (!aLangs || aLangs[0] == '\0') &&
                proxy->unicharToGlyph(0x2603) != 0)
            {
                if constexpr (kSkFontMgrVerbose) {
                    SkDebugf("SKIA: Hacking in und-Zsym for NotoSansSymbols-Regular-Subsetted\n");
                }
                aLangs = "und-Zsym";
            }

            // HACK: Some Android versions have a variable Roboto font named Roboto but also use
            // a font named RobotoStatic (which does not claim to be Roboto) for the 400 weight.
            // If RobotoStatic is found but does not have the name "Roboto", add it.
            // Fixed in U "[2nd attempt] Revive use of VF font for regular style of roboto font"
            // https://android.googlesource.com/platform/frameworks/base/+/89abe560d722a6f4136b7a05d80f23b269413aad
            static constexpr char kRobotoStatic[] = "RobotoStatic-Regular";
            static constexpr char kRoboto[] = "Roboto";
            if (postscriptName.equals(kRobotoStatic, std::size(kRobotoStatic)-1) &&
                std::none_of(extraFamilyNames.begin(), extraFamilyNames.end(),
                             [](SkString& n){return n.equals(kRoboto, std::size(kRoboto)-1);}))
            {
                if constexpr (kSkFontMgrVerbose) {
                    SkDebugf("SKIA: Hacking in Roboto for RobotoStatic-Regular\n");
                }
                extraFamilyNames.push_back(SkString(kRoboto, std::size(kRoboto)-1));
            }
        }
        if (aLangs) {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: %s ALangs %s\n", familyName.c_str(), aLangs);
            }
            // Format: ',' or '\0' are terminators, '\0' is the final terminator.
            const char* begin = aLangs;
            const char* end = aLangs;
            while (true) {
                while (*end != '\0' && *end != ',') {
                    ++end;
                }
                const size_t size = end - begin;
                if (size) {
                    skLangs.emplace_back(SkSpan(begin, size));
                }
                if (*end == '\0') {
                    break;
                }
                ++end;
                begin = end;
            }
        }
        if constexpr (kSkFontMgrVerbose) {
            for (auto&& lang : skLangs) {
                SkDebugf("SKIA: %s Lang %s Script %s Region %s\n",
                         familyName.c_str(),
                         lang.getLanguage().c_str(),
                         lang.getScript().c_str(),
                         lang.getRegion().c_str());
            }
        }

        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: New typeface %s [%d %d %d]\n", familyName.c_str(), style.weight(),
                     style.width(), style.slant());
        }

        return SkTypeface_AndroidNDK::Make(
            proxy, style, proxy->isFixedPitch(),
            familyName, std::move(extraFamilyNames), std::move(skLangs), autoAxis);
    }


    static bool has_locale_and_character(SkTypeface_AndroidNDK* face,
                                         const SkALanguage& langTag,
                                         SkUnichar character,
                                         const char* scope, size_t* step) {
        ++*step;
        if (!langTag.isEmpty() &&
            std::none_of(face->fLang.begin(), face->fLang.end(), [&](SkALanguage lang) {
                return lang.startsWith(langTag);
            }))
        {
            return false;
        }

        if (face->unicharToGlyph(character) == 0) {
            return false;
        }

        if constexpr (kSkFontMgrVerbose) {
            SkString foundName;
            face->getFamilyName(&foundName);
            SkDebugf("SKIA: Found U+%" PRIx32 " in \"%s\" lang \"%s\" "
                     "script \"%s\" region \"%s\" scope %s step %zu.\n",
                     character, foundName.c_str(), langTag.getLanguage().c_str(),
                     langTag.getScript().c_str(), langTag.getRegion().c_str(), scope, *step);

        }
        return true;
    }

    sk_sp<SkTypeface> findByCharacterLocaleFamily(
        SkTypeface_AndroidNDK* familyFace,
        const SkFontStyle& style,
        const SkALanguage& langTag,
        SkUnichar character) const
    {
        size_t step = 0;
        // First look at the familyFace
        if (familyFace && has_locale_and_character(familyFace, langTag, character, "face", &step)) {
            return sk_ref_sp(familyFace);
        }

        // Look through the styles that match in each family.
        for (const NameToFamily& nameToFamily : fNameToFamilyMap) {
            sk_sp<SkTypeface_AndroidNDK> face(nameToFamily.styleSet->matchAStyle(style));
            if (has_locale_and_character(face.get(), langTag, character, "style", &step)) {
                return adjustForStyle(std::move(face), style, *fCache);
            }
        }

        // Look through everything.

        // Android by default has a setup like
        // /system/fonts/NotoSansSymbols-Regular-Subsetted.ttf#0
        // /system/fonts/NotoSansSymbols-Regular-Subsetted2.ttf#0
        // Which are both "Noto Sans Symbols" so end up in a "family" together. However, these
        // are not in the same family, these are two different fonts in different families and
        // should have been given different names. Internally this works because these are
        // in separate <family> tags, but the NDK API doesn't provide that information.
        // While Android internally depends on all fonts in a family having the same characters
        // mapped, this cannot be relied upon when guessing at the families by name.

        for (const NameToFamily& nameToFamily : fNameToFamilyMap) {
            for (const sk_sp<SkTypeface_AndroidNDK>& face : nameToFamily.styleSet->fStyles) {
                if (has_locale_and_character(face.get(), langTag, character, "anything", &step)) {
                    return adjustForStyle(sk_sp(face), style, *fCache);
                }
            }
        }

        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        // If at some point AFontMatcher becomes usable, the code for using it is at
        // https://skia-review.googlesource.com/c/skia/+/585970/13/src/ports/SkFontMgr_android_ndk.cpp#766

        sk_sp<SkTypeface> familyFace;
        SkTypeface_AndroidNDK* afamilyFace = nullptr;
        if (familyName) {
            familyFace = this->onMatchFamilyStyle(familyName, style);
            afamilyFace = static_cast<SkTypeface_AndroidNDK*>(familyFace.get());
        }

        SkSpan langtags(bcp47, bcp47Count);
        for (auto&& langtag = langtags.rbegin(); langtag != langtags.rend(); ++langtag) {
            SkALanguage lang(*langtag);
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Matching against %s Lang %s Script %s Region %s\n",
                         familyName ? familyName : "",
                         lang.getLanguage().c_str(),
                         lang.getScript().c_str(),
                         lang.getRegion().c_str());
            }
            while (!lang.isEmpty()) {
                sk_sp<SkTypeface> typeface =
                    findByCharacterLocaleFamily(afamilyFace, style, lang, character);
                if (typeface) {
                    return typeface;
                }
                lang = lang.lessSpecific();
            }
        }

        sk_sp<SkTypeface> typeface =
            findByCharacterLocaleFamily(afamilyFace, style, SkALanguage(), character);
        if (typeface) {
            return typeface;
        }

        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: No font had U+%" PRIx32 "\n", character);
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(
            std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))), ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
        return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        return fScanner->MakeFromStream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char name[], SkFontStyle style) const override {
        if (name) {
            // On Android, we must return nullptr when we can't find the requested
            // named typeface so that the system/app can provide their own recovery
            // mechanism. On other platforms we'd provide a typeface from the
            // default family instead.
            return sk_sp<SkTypeface>(this->onMatchFamilyStyle(name, style));
        }
        if (fDefaultStyleSet) {
            return sk_sp<SkTypeface>(fDefaultStyleSet->matchStyle(style));
        }
        return nullptr;
    }


private:
    AndroidFontAPI fAPI;
    AndroidIcuAPI fICU;
    std::unique_ptr<SkFontScanner> fScanner;

    TArray<NameToFamily> fNameToFamilyMap;
    TArray<sk_sp<SkFontStyleSet_AndroidNDK>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    sk_sp<TypefaceCache> fCache;

    void findDefaultStyleSet() {
        SkASSERT(!fStyleSets.empty());

        static constexpr const char* kDefaultNames[] = { "sans-serif", "Roboto" };
        for (const char* defaultName : kDefaultNames) {
            fDefaultStyleSet = this->onMatchFamily(defaultName);
            if (fDefaultStyleSet) {
                break;
            }
        }
        if (nullptr == fDefaultStyleSet) {
            fDefaultStyleSet = fStyleSets[0];
        }
        SkASSERT(fDefaultStyleSet);
    }
};

}  // namespace

sk_sp<SkFontMgr> SkFontMgr_New_AndroidNDK(bool cacheFontFiles,
                                          std::unique_ptr<SkFontScanner> scanner)
{
    std::optional<AndroidFontAPI> fontAPI = AndroidFontAPI::Make();
    if (!fontAPI) {
        return nullptr;
    }
    return sk_sp(new SkFontMgr_AndroidNDK(*std::move(fontAPI), cacheFontFiles, std::move(scanner)));
}
