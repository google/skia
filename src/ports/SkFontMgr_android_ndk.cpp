/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkFontMgr_android_ndk.h"
#include "include/ports/SkFontScanner_FreeType.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTSearch.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkOSFile.h"
#include "src/ports/SkFontScanner_FreeType_priv.h"
#include "src/ports/SkTypeface_FreeType.h"

#include <android/api-level.h>

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
#define SK_FONTMGR_ANDROID_NDK_API_LEVEL __ANDROID_API_R__

#if __ANDROID_API__ >= SK_FONTMGR_ANDROID_NDK_API_LEVEL
#include <android/font.h>
#include <android/font_matcher.h>
#include <android/system_fonts.h>
#endif

#include <cinttypes>
#include <memory>

#include <dlfcn.h>

struct ASystemFontIterator;
struct AFont;

namespace {

[[maybe_unused]] static inline const constexpr bool kSkFontMgrVerbose = false;

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
};

#if __ANDROID_API__ >= SK_FONTMGR_ANDROID_NDK_API_LEVEL

static const AndroidFontAPI* GetAndroidFontAPI() {
    static AndroidFontAPI androidFontAPI {
        ASystemFontIterator_open,
        ASystemFontIterator_close,
        ASystemFontIterator_next,

        AFont_close,
        AFont_getFontFilePath,
        AFont_getWeight,
        AFont_isItalic,
        AFont_getLocale,
        AFont_getCollectionIndex,
        AFont_getAxisCount,
        AFont_getAxisTag,
        AFont_getAxisValue,
    };
    if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidFontAPI direct\n"); }
    return &androidFontAPI;
}

#else

static const AndroidFontAPI* GetAndroidFontAPI() {
    struct OptionalAndroidFontAPI : AndroidFontAPI {
        bool valid = false;
    };
    static OptionalAndroidFontAPI androidFontAPI = [](){
        using DLHandle = std::unique_ptr<void, SkFunctionObject<dlclose>>;
        OptionalAndroidFontAPI api;

        if (android_get_device_api_level() < SK_FONTMGR_ANDROID_NDK_API_LEVEL) {
            return api;
        }

        DLHandle self(dlopen("libandroid.so", RTLD_LAZY | RTLD_LOCAL));
        if (!self) {
            return api;
        }

#define SK_DLSYM_ANDROID_FONT_API(NAME)                           \
        do {                                                      \
            *(void**)(&api.NAME) = dlsym(self.get(), #NAME);      \
            if (!api.NAME) {                                      \
                if constexpr (kSkFontMgrVerbose) {                \
                    SkDebugf("SKIA: Failed to load: " #NAME "\n");\
                }                                                 \
                return api;                                       \
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

        api.valid = true;
        return api;
    }();
    if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: GetAndroidFontAPI dlsym\n"); }
    return androidFontAPI.valid ? &androidFontAPI : nullptr;
};

#endif

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

class SkLanguage {
public:
    SkLanguage() { }
    SkLanguage(const SkString& tag) : fTag(tag) { }
    SkLanguage(const char* tag) : fTag(tag) { }
    SkLanguage(const char* tag, size_t len) : fTag(tag, len) { }
    SkLanguage(const SkLanguage&) = default;
    SkLanguage& operator=(const SkLanguage& b) = default;

    /** Gets a BCP 47 language identifier for this SkLanguage.
        @return a BCP 47 language identifier representing this language
    */
    const SkString& getTag() const { return fTag; }

    /** Performs BCP 47 fallback to return an SkLanguage one step more general.
        @return an SkLanguage one step more general
    */
    SkLanguage getParent() const {
        SkASSERT(!fTag.isEmpty());
        const char* tag = fTag.c_str();

        // strip off the rightmost "-.*"
        const char* parentTagEnd = strrchr(tag, '-');
        if (parentTagEnd == nullptr) {
            return SkLanguage();
        }
        size_t parentTagLen = parentTagEnd - tag;
        return SkLanguage(tag, parentTagLen);
    }

    bool operator==(const SkLanguage& b) const {
        return fTag == b.fTag;
    }
    bool operator!=(const SkLanguage& b) const {
        return fTag != b.fTag;
    }

    using sk_is_trivially_relocatable = std::true_type;
private:
    //! BCP 47 language identifier
    SkString fTag;
    static_assert(::sk_is_trivially_relocatable<decltype(fTag)>::value);
};

class SkTypeface_AndroidNDK : public SkTypeface_FreeType {
public:
    SkTypeface_AndroidNDK(std::unique_ptr<SkStreamAsset> file,
                          const SkString& pathName,
                          const bool cacheFontFiles,
                          int index,
                          const SkFixed* axes, int axesCount,
                          const SkFontStyle& style,
                          bool isFixedPitch,
                          const SkString& familyName,
                          TArray<SkLanguage>&& lang)
        : SkTypeface_FreeType(style, isFixedPitch)
        , fFamilyName(familyName)
        , fPathName(pathName)
        , fIndex(index)
        , fAxes(axes, axesCount)
        , fLang(std::move(lang))
        , fFile((cacheFontFiles &&  file) ? std::move(file)
               :(cacheFontFiles && !file) ? SkStream::MakeFromFile(fPathName.c_str())
               : nullptr)
        , fCacheFontFiles(cacheFontFiles)
    {
        if (cacheFontFiles) {
            SkASSERT(fFile);
        }
    }

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    std::unique_ptr<SkStreamAsset> makeStream() const {
        if (fFile) {
            return fFile->duplicate();
        }
        return SkStream::MakeFromFile(fPathName.c_str());
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkASSERT(desc);
        SkASSERT(serialize);
        desc->setFamilyName(fFamilyName.c_str());
        desc->setStyle(this->fontStyle());
        desc->setFactoryId(SkTypeface_FreeType::FactoryId);
        *serialize = false;
    }
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        *ttcIndex = fIndex;
        return this->makeStream();
    }
    std::unique_ptr<SkFontData> onMakeFontData() const override {
        return std::make_unique<SkFontData>(
                this->makeStream(), fIndex, 0, fAxes.begin(), fAxes.size(), nullptr, 0);
    }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        SkFontStyle newStyle = this->fontStyle();
        std::unique_ptr<SkFontData> data = this->cloneFontData(args, &newStyle);
        if (!data) {
            return nullptr;
        }
        return sk_sp(new SkTypeface_AndroidNDK(fFile ? fFile->duplicate() : nullptr,
                                               fPathName,
                                               fCacheFontFiles,
                                               fIndex,
                                               data->getAxis(),
                                               data->getAxisCount(),
                                               newStyle,
                                               this->isFixedPitch(),
                                               fFamilyName,
                                               TArray<SkLanguage>()));
    }

    sk_sp<SkTypeface_AndroidNDK> makeNamedClone(const SkString& name) const {
        return sk_sp(new SkTypeface_AndroidNDK(fFile ? fFile->duplicate() : nullptr,
                                               fPathName,
                                               fCacheFontFiles,
                                               fIndex,
                                               fAxes.data(), fAxes.size(),
                                               this->fontStyle(),
                                               this->isFixedPitch(),
                                               name,
                                               STArray<4, SkLanguage>(fLang)));
    }

    const SkString fFamilyName;
    const SkString fPathName;
    int fIndex;
    const STArray<4, SkFixed> fAxes;
    const STArray<4, SkLanguage> fLang;
    std::unique_ptr<SkStreamAsset> fFile;
    bool fCacheFontFiles;
};

class SkFontStyleSet_AndroidNDK : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_AndroidNDK() { }

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
    sk_sp<SkTypeface> createTypeface(int index) override {
        if (index < 0 || fStyles.size() <= index) {
            return nullptr;
        }
        return fStyles[index];
    }

    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        sk_sp<SkTypeface> match = this->matchStyleCSS3(pattern);
        if constexpr (kSkFontMgrVerbose) {
            SkTypeface_AndroidNDK* amatch = static_cast<SkTypeface_AndroidNDK*>(match.get());
            SkString name;
            amatch->getFamilyName(&name);
            SkFontStyle fontStyle = amatch->fontStyle();
            SkString axes;
            for (auto&& axis : amatch->fAxes) {
                axes.appendScalar(SkFixedToScalar(axis));
                axes.append(", ");
            }
            SkDebugf("SKIA: Search for [%d, %d, %d] matched %s [%d, %d, %d] %s#%d [%s]\n",
                     pattern.weight(), pattern.width(), pattern.slant(),
                     name.c_str(), fontStyle.weight(), fontStyle.width(), fontStyle.slant(),
                     amatch->fPathName.c_str(), amatch->fIndex, axes.c_str());
        }
        return match;
    }

private:
    TArray<sk_sp<SkTypeface_AndroidNDK>> fStyles;
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
        for (NameToFamily& current : fNameToFamilyMap) {
            if (current.name == name) {
                nameToFamily = &current;
                break;
            }
        }
        if (!nameToFamily) {
            sk_sp<SkFontStyleSet_AndroidNDK> newSet(new SkFontStyleSet_AndroidNDK());
            SkAutoAsciiToLC tolc(name.c_str());
            nameToFamily = &fNameToFamilyMap.emplace_back(
                NameToFamily{name, SkString(tolc.lc(), tolc.length()), newSet.get()});
            fStyleSets.push_back(std::move(newSet));
        }
        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Adding member to %s\n", name.c_str()); }
        nameToFamily->styleSet->fStyles.push_back(typeface);
    }

public:
    SkFontMgr_AndroidNDK(const AndroidFontAPI& androidFontAPI, bool const cacheFontFiles,
                          std::unique_ptr<SkFontScanner> scanner)
        : fAPI(androidFontAPI)
        , fScanner(std::move(scanner))
    {
        SkASystemFontIterator fontIter(fAPI);
        if (!fontIter) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: No ASystemFontIterator"); }
            return;
        }

        if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Iterating over AFonts\n"); }
        while (SkAFont font = fontIter.next()) {
            sk_sp<SkTypeface_AndroidNDK> typeface = this->make(std::move(font), cacheFontFiles);
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

            // There nothing in the NDK to indicate how to handle generic font names like 'serif',
            // 'sans-serif`, 'monospace', etc.
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

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        SkAutoAsciiToLC tolc(familyName);
        for (int i = 0; i < fNameToFamilyMap.size(); ++i) {
            if (fNameToFamilyMap[i].normalizedName.equals(tolc.lc())) {
                return sk_ref_sp(fNameToFamilyMap[i].styleSet);
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override
    {
        sk_sp<SkFontStyleSet> sset(this->onMatchFamily(familyName));
        if (!sset) {
            return nullptr;
        }
        return sset->matchStyle(style);
    }

    sk_sp<SkTypeface_AndroidNDK> make(SkAFont font, bool cacheFontFiles) const {
        const char* filePath = font.getFontFilePath();

        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(filePath);
        if (!stream) {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Font file %s does not exist or cannot be opened.\n", filePath);
            }
            return nullptr;
        }

        size_t collectionIndex = font.getCollectionIndex();
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: Making font from %s#%zu\n", filePath, collectionIndex);
        }
        if (!SkTFitsIn<int>(collectionIndex)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Collection index invalid!"); }
            return nullptr;
        }
        const int ttcIndex = SkTo<int>(collectionIndex);

        SkString familyName;
        SkFontStyle style;
        bool isFixedWidth;
        SkFontScanner::AxisDefinitions axisDefinitions;
        if (!fScanner->scanInstance(stream.get(), ttcIndex, 0,
                                    &familyName, &style, &isFixedWidth, &axisDefinitions))
        {
            if constexpr (kSkFontMgrVerbose) {
                SkDebugf("SKIA: Font file %s exists, but is not a valid font.\n", filePath);
            }
            return nullptr;
        }

        int weight = SkTo<int>(font.getWeight());
        SkFontStyle::Slant slant = style.slant();
        if (font.isItalic()) {
            slant = SkFontStyle::kItalic_Slant;
        }
        int width = style.width();
        constexpr SkFourByteTag wdth = SkSetFourByteTag('w','d','t','h');

        // The family name(s) are not reported.
        // This would be very helpful for aliases, like "sans-serif", "Arial", etc.

        size_t requestAxisCount = font.getAxisCount();
        if (!SkTFitsIn<int>(requestAxisCount)) {
            if constexpr (kSkFontMgrVerbose) { SkDebugf("SKIA: Axis count unreasonable!"); }
            return nullptr;
        }
        using Coordinate = SkFontArguments::VariationPosition::Coordinate;
        AutoSTMalloc<4, Coordinate> requestAxisValues(requestAxisCount);
        for (size_t i = 0; i < requestAxisCount; ++i) {
            uint32_t tag = font.getAxisTag(i);
            float value = font.getAxisValue(i);
            requestAxisValues[i] = { tag, value };
            if (tag == wdth) {
                // Set the width based on the requested `wdth` axis value.
                width = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(value);
            }
        }

        AutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.size());
        SkFontArguments::VariationPosition position = {
            requestAxisValues.get(), SkTo<int>(requestAxisCount)
        };
        SkFontScanner_FreeType::computeAxisValues(axisDefinitions, position,
                                                  axisValues, familyName, nullptr);

        STArray<4, SkLanguage> skLangs;
        const char* aLangs = font.getLocale();
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
                    skLangs.emplace_back(begin, size);
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
                SkDebugf("SKIA: %s Lang %s\n", familyName.c_str(), lang.getTag().c_str());
            }
        }

        style = SkFontStyle(weight, width, slant);
        if constexpr (kSkFontMgrVerbose) {
            SkDebugf("SKIA: New typeface %s [%d %d %d]\n", familyName.c_str(), weight,width,slant);
        }
        return sk_sp<SkTypeface_AndroidNDK>(new SkTypeface_AndroidNDK(
                std::move(stream), SkString(filePath), cacheFontFiles, ttcIndex,
                axisValues.get(), axisDefinitions.size(),
                style, isFixedWidth, familyName, std::move(skLangs)));
    }


    static bool has_locale_and_character(SkTypeface_AndroidNDK* face,
                                         const SkString& langTag,
                                         SkUnichar character,
                                         const char* scope, size_t* step) {
        ++*step;
        if (!langTag.isEmpty() &&
            std::none_of(face->fLang.begin(), face->fLang.end(), [&](SkLanguage lang) {
                return lang.getTag().startsWith(langTag.c_str());
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
            SkDebugf("SKIA: Found U+%" PRIx32 " in \"%s\" lang \"%s\" scope %s step %zu.\n",
                     character, foundName.c_str(), langTag.c_str(), scope, *step);
        }
        return true;
    }

    sk_sp<SkTypeface> findByCharacterLocaleFamily(
        SkTypeface_AndroidNDK* familyFace,
        const SkFontStyle& style,
        const SkString& langTag,
        SkUnichar character) const
    {
        size_t step = 0;
        // First look at the familyFace
        if (familyFace && has_locale_and_character(familyFace, langTag, character, "face", &step)) {
            return sk_ref_sp(familyFace);
        }

        // Look through the styles that match in each family.
        for (int i = 0; i < fNameToFamilyMap.size(); ++i) {
            SkFontStyleSet_AndroidNDK* family = fNameToFamilyMap[i].styleSet;
            sk_sp<SkTypeface> face(family->matchStyle(style));
            auto aface = static_cast<SkTypeface_AndroidNDK*>(face.get());
            if (has_locale_and_character(aface, langTag, character, "style", &step)) {
                return face;
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

        for (int i = 0; i < fNameToFamilyMap.size(); ++i) {
            SkFontStyleSet_AndroidNDK* family = fNameToFamilyMap[i].styleSet;
            for (int j = 0; j < family->count(); ++j) {
                sk_sp<SkTypeface> face(family->createTypeface(j));
                auto aface = static_cast<SkTypeface_AndroidNDK*>(face.get());
                if (has_locale_and_character(aface, langTag, character, "anything", &step)) {
                    return face;
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

        for (int bcp47Index = bcp47Count; bcp47Index --> 0;) {
            SkLanguage lang(bcp47[bcp47Index]);
            while (!lang.getTag().isEmpty()) {
                sk_sp<SkTypeface> typeface =
                    findByCharacterLocaleFamily(afamilyFace, style, lang.getTag(), character);
                if (typeface) {
                    return typeface;
                }
                lang = lang.getParent();
            }
        }

        sk_sp<SkTypeface> typeface =
            findByCharacterLocaleFamily(afamilyFace, style, SkString(), character);
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
        return SkTypeface_FreeType::MakeFromStream(std::move(stream), args);
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
    const AndroidFontAPI& fAPI;
    std::unique_ptr<SkFontScanner> fScanner;

    TArray<sk_sp<SkFontStyleSet_AndroidNDK>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    TArray<NameToFamily> fNameToFamilyMap;

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

sk_sp<SkFontMgr> SkFontMgr_New_AndroidNDK(bool cacheFontFiles) {
    return SkFontMgr_New_AndroidNDK(cacheFontFiles, SkFontScanner_Make_FreeType());
}

sk_sp<SkFontMgr> SkFontMgr_New_AndroidNDK(bool cacheFontFiles,
                                           std::unique_ptr<SkFontScanner> scanner)
{
    AndroidFontAPI const * const androidFontAPI = GetAndroidFontAPI();
    if (!androidFontAPI) {
        return nullptr;
    }
    return sk_sp(new SkFontMgr_AndroidNDK(*androidFontAPI, cacheFontFiles, std::move(scanner)));
}
