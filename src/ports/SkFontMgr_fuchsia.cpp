/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/ports/SkFontMgr_fuchsia.h"

#include <fuchsia/fonts/cpp/fidl.h>
#include <lib/zx/vmar.h>
#include <strings.h>
#include <memory>
#include <unordered_map>

#include "src/core/SkFontDescriptor.h"
#include "src/ports/SkFontMgr_custom.h"
#include "src/ports/SkFontScanner_FreeType_priv.h"
#include "src/ports/SkTypeface_FreeType.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "src/core/SkTypefaceCache.h"

using namespace skia_private;

// SkFuchsiaFontDataCache keep track of SkData created from `fuchsia::mem::Buffer` where each buffer
// is identified with a unique identifier. It allows to share the same SkData instances between all
// SkTypeface instances created from the same buffer.
class SkFuchsiaFontDataCache : public SkRefCnt {
public:
    SkFuchsiaFontDataCache() = default;
    ~SkFuchsiaFontDataCache() { SkASSERT(fBuffers.empty()); }

    sk_sp<SkData> GetOrCreateSkData(int bufferId, const fuchsia::mem::Buffer& buffer);

private:
    struct ReleaseSkDataContext {
        sk_sp<SkFuchsiaFontDataCache> fCache;
        int fBufferId;
    };

    static void ReleaseSkData(const void* buffer, void* context);
    void OnBufferDeleted(int bufferId);

    SkMutex fMutex;
    std::unordered_map<int, SkData*> fBuffers SK_GUARDED_BY(fMutex);
};

sk_sp<SkData> SkFuchsiaFontDataCache::GetOrCreateSkData(int bufferId,
                                                        const fuchsia::mem::Buffer& buffer) {
    SkAutoMutexExclusive mutexLock(fMutex);

    auto iter = fBuffers.find(bufferId);
    if (iter != fBuffers.end()) {
        return sk_ref_sp(iter->second);
    }
    auto font_mgr = sk_ref_sp(this);

    uint64_t size = buffer.size;
    uintptr_t mapped_addr = 0;
    zx_status_t status =
            zx::vmar::root_self()->map(ZX_VM_PERM_READ, 0, buffer.vmo, 0, size, &mapped_addr);
    if (status != ZX_OK) return nullptr;

    auto context = new ReleaseSkDataContext{sk_ref_sp(this), bufferId};
    auto data = SkData::MakeWithProc(
            reinterpret_cast<void*>(mapped_addr), size, ReleaseSkData, context);
    SkASSERT(data);

    fBuffers[bufferId] = data.get();
    return data;
}

void SkFuchsiaFontDataCache::OnBufferDeleted(int bufferId) {
    zx_vaddr_t unmap_addr;
    size_t unmap_size;
    {
        SkAutoMutexExclusive mutexLock(fMutex);
        auto it = fBuffers.find(bufferId);
        SkASSERT(it != fBuffers.end());
        unmap_addr = reinterpret_cast<zx_vaddr_t>(it->second->data());
        unmap_size = it->second->size();
        fBuffers.erase(it);
    }

    zx::vmar::root_self()->unmap(unmap_addr, unmap_size);
}

// static
void SkFuchsiaFontDataCache::ReleaseSkData(const void* buffer, void* context) {
    auto releaseSkDataContext = reinterpret_cast<ReleaseSkDataContext*>(context);
    releaseSkDataContext->fCache->OnBufferDeleted(releaseSkDataContext->fBufferId);
    delete releaseSkDataContext;
}

fuchsia::fonts::Slant SkToFuchsiaSlant(SkFontStyle::Slant slant) {
    switch (slant) {
        case SkFontStyle::kOblique_Slant:
            return fuchsia::fonts::Slant::OBLIQUE;
        case SkFontStyle::kItalic_Slant:
            return fuchsia::fonts::Slant::ITALIC;
        case SkFontStyle::kUpright_Slant:
        default:
            return fuchsia::fonts::Slant::UPRIGHT;
    }
}

SkFontStyle::Slant FuchsiaToSkSlant(fuchsia::fonts::Slant slant) {
    switch (slant) {
        case fuchsia::fonts::Slant::OBLIQUE:
            return SkFontStyle::kOblique_Slant;
        case fuchsia::fonts::Slant::ITALIC:
            return SkFontStyle::kItalic_Slant;
        case fuchsia::fonts::Slant::UPRIGHT:
        default:
            return SkFontStyle::kUpright_Slant;
    }
}

fuchsia::fonts::Width SkToFuchsiaWidth(SkFontStyle::Width width) {
    switch (width) {
        case SkFontStyle::Width::kUltraCondensed_Width:
            return fuchsia::fonts::Width::ULTRA_CONDENSED;
        case SkFontStyle::Width::kExtraCondensed_Width:
            return fuchsia::fonts::Width::EXTRA_CONDENSED;
        case SkFontStyle::Width::kCondensed_Width:
            return fuchsia::fonts::Width::CONDENSED;
        case SkFontStyle::Width::kSemiCondensed_Width:
            return fuchsia::fonts::Width::SEMI_CONDENSED;
        case SkFontStyle::Width::kNormal_Width:
            return fuchsia::fonts::Width::NORMAL;
        case SkFontStyle::Width::kSemiExpanded_Width:
            return fuchsia::fonts::Width::SEMI_EXPANDED;
        case SkFontStyle::Width::kExpanded_Width:
            return fuchsia::fonts::Width::EXPANDED;
        case SkFontStyle::Width::kExtraExpanded_Width:
            return fuchsia::fonts::Width::EXTRA_EXPANDED;
        case SkFontStyle::Width::kUltraExpanded_Width:
            return fuchsia::fonts::Width::ULTRA_EXPANDED;
    }
}

// Tries to convert the given integer Skia style width value to the Fuchsia equivalent.
//
// On success, returns true. On failure, returns false, and `outFuchsiaWidth` is left untouched.
bool SkToFuchsiaWidth(int skWidth, fuchsia::fonts::Width* outFuchsiaWidth) {
    if (skWidth < SkFontStyle::Width::kUltraCondensed_Width ||
        skWidth > SkFontStyle::Width::kUltraExpanded_Width) {
        return false;
    }
    auto typedSkWidth = static_cast<SkFontStyle::Width>(skWidth);
    *outFuchsiaWidth = SkToFuchsiaWidth(typedSkWidth);
    return true;
}

SkFontStyle::Width FuchsiaToSkWidth(fuchsia::fonts::Width width) {
    switch (width) {
        case fuchsia::fonts::Width::ULTRA_CONDENSED:
            return SkFontStyle::Width::kUltraCondensed_Width;
        case fuchsia::fonts::Width::EXTRA_CONDENSED:
            return SkFontStyle::Width::kExtraCondensed_Width;
        case fuchsia::fonts::Width::CONDENSED:
            return SkFontStyle::Width::kCondensed_Width;
        case fuchsia::fonts::Width::SEMI_CONDENSED:
            return SkFontStyle::Width::kSemiCondensed_Width;
        case fuchsia::fonts::Width::NORMAL:
            return SkFontStyle::Width::kNormal_Width;
        case fuchsia::fonts::Width::SEMI_EXPANDED:
            return SkFontStyle::Width::kSemiExpanded_Width;
        case fuchsia::fonts::Width::EXPANDED:
            return SkFontStyle::Width::kExpanded_Width;
        case fuchsia::fonts::Width::EXTRA_EXPANDED:
            return SkFontStyle::Width::kExtraExpanded_Width;
        case fuchsia::fonts::Width::ULTRA_EXPANDED:
            return SkFontStyle::Width::kUltraExpanded_Width;
    }
}

fuchsia::fonts::Style2 SkToFuchsiaStyle(const SkFontStyle& style) {
    fuchsia::fonts::Style2 fuchsiaStyle;
    fuchsiaStyle.set_slant(SkToFuchsiaSlant(style.slant())).set_weight(style.weight());

    fuchsia::fonts::Width fuchsiaWidth = fuchsia::fonts::Width::NORMAL;
    if (SkToFuchsiaWidth(style.width(), &fuchsiaWidth)) {
        fuchsiaStyle.set_width(fuchsiaWidth);
    }

    return fuchsiaStyle;
}

constexpr struct {
    const char* fName;
    fuchsia::fonts::GenericFontFamily fGenericFontFamily;
} kGenericFontFamiliesByName[] = {{"serif", fuchsia::fonts::GenericFontFamily::SERIF},
                                  {"sans", fuchsia::fonts::GenericFontFamily::SANS_SERIF},
                                  {"sans-serif", fuchsia::fonts::GenericFontFamily::SANS_SERIF},
                                  {"mono", fuchsia::fonts::GenericFontFamily::MONOSPACE},
                                  {"monospace", fuchsia::fonts::GenericFontFamily::MONOSPACE},
                                  {"cursive", fuchsia::fonts::GenericFontFamily::CURSIVE},
                                  {"fantasy", fuchsia::fonts::GenericFontFamily::FANTASY},
                                  {"system-ui", fuchsia::fonts::GenericFontFamily::SYSTEM_UI},
                                  {"emoji", fuchsia::fonts::GenericFontFamily::EMOJI},
                                  {"math", fuchsia::fonts::GenericFontFamily::MATH},
                                  {"fangsong", fuchsia::fonts::GenericFontFamily::FANGSONG}};

// Tries to find a generic font family with the given name. If none is found, returns false.
bool GetGenericFontFamilyByName(const char* name,
                                fuchsia::fonts::GenericFontFamily* outGenericFamily) {
    if (!name) return false;
    for (auto& genericFamily : kGenericFontFamiliesByName) {
        if (strcasecmp(genericFamily.fName, name) == 0) {
            *outGenericFamily = genericFamily.fGenericFontFamily;
            return true;
        }
    }
    return false;
}

struct TypefaceId {
    uint32_t bufferId;
    uint32_t ttcIndex;

    bool operator==(TypefaceId& other) const {
        return std::tie(bufferId, ttcIndex) == std::tie(other.bufferId, other.ttcIndex);
    }
}

constexpr kNullTypefaceId = {0xFFFFFFFF, 0xFFFFFFFF};

class SkTypeface_Fuchsia : public SkTypeface_FreeTypeStream {
public:
    SkTypeface_Fuchsia(std::unique_ptr<SkFontData> fontData, const SkFontStyle& style,
                       bool isFixedPitch, const SkString familyName, TypefaceId id)
            : SkTypeface_FreeTypeStream(std::move(fontData), familyName, style, isFixedPitch)
            , fId(id) {}

    TypefaceId id() { return fId; }

private:
    TypefaceId fId;
};

sk_sp<SkTypeface> CreateTypefaceFromSkStream(std::unique_ptr<SkStreamAsset> stream,
                                             const SkFontArguments& args, TypefaceId id) {
    SkFontScanner_FreeType fontScanner;
    int numInstances;
    if (!fontScanner.scanFace(stream.get(), args.getCollectionIndex(), &numInstances)) {
        return nullptr;
    }
    bool isFixedPitch;
    SkFontStyle style;
    SkString name;
    SkFontScanner::AxisDefinitions axisDefinitions;
    if (!fontScanner.scanInstance(stream.get(),
                                  args.getCollectionIndex(),
                                  0,
                                  &name,
                                  &style,
                                  &isFixedPitch,
                                  &axisDefinitions)) {
        return nullptr;
    }

    const SkFontArguments::VariationPosition position = args.getVariationDesignPosition();
    AutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.size());
    SkFontScanner_FreeType::computeAxisValues(axisDefinitions, position, axisValues, name, &style);

    auto fontData = std::make_unique<SkFontData>(
        std::move(stream), args.getCollectionIndex(), args.getPalette().index,
        axisValues.get(), axisDefinitions.size(),
        args.getPalette().overrides, args.getPalette().overrideCount);
    return sk_make_sp<SkTypeface_Fuchsia>(std::move(fontData), style, isFixedPitch, name, id);
}

sk_sp<SkTypeface> CreateTypefaceFromSkData(sk_sp<SkData> data, TypefaceId id) {
    return CreateTypefaceFromSkStream(std::make_unique<SkMemoryStream>(std::move(data)),
                                      SkFontArguments().setCollectionIndex(id.ttcIndex), id);
}

class SkFontMgr_Fuchsia final : public SkFontMgr {
public:
    SkFontMgr_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider);
    ~SkFontMgr_Fuchsia() override;

protected:
    // SkFontMgr overrides.
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override;
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[], const SkFontStyle&) const override;
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override;
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                            int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                           const SkFontArguments&) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle) const override;

private:
    friend class SkFontStyleSet_Fuchsia;

    sk_sp<SkTypeface> FetchTypeface(const char familyName[], const SkFontStyle& style,
                                    const char* bcp47[], int bcp47Count, SkUnichar character,
                                    bool allow_fallback, bool exact_style_match) const;

    sk_sp<SkTypeface> GetOrCreateTypeface(TypefaceId id, const fuchsia::mem::Buffer& buffer) const;

    mutable fuchsia::fonts::ProviderSyncPtr fFontProvider;

    sk_sp<SkFuchsiaFontDataCache> fBufferCache;

    mutable SkMutex fCacheMutex;
    mutable SkTypefaceCache fTypefaceCache SK_GUARDED_BY(fCacheMutex);
};

class SkFontStyleSet_Fuchsia : public SkFontStyleSet {
public:
    SkFontStyleSet_Fuchsia(sk_sp<SkFontMgr_Fuchsia> font_manager, std::string familyName,
                 std::vector<SkFontStyle> styles)
            : fFontManager(font_manager), fFamilyName(familyName), fStyles(styles) {}

    ~SkFontStyleSet_Fuchsia() override = default;

    int count() override { return fStyles.size(); }

    void getStyle(int index, SkFontStyle* style, SkString* styleName) override {
        SkASSERT(index >= 0 && index < static_cast<int>(fStyles.size()));
        if (style) *style = fStyles[index];

        // We don't have style names. Return an empty name.
        if (styleName) styleName->reset();
    }

    sk_sp<SkTypeface> createTypeface(int index) override {
        SkASSERT(index >= 0 && index < static_cast<int>(fStyles.size()));

        if (fTypefaces.empty()) fTypefaces.resize(fStyles.size());

        if (!fTypefaces[index]) {
            fTypefaces[index] = fFontManager->FetchTypeface(
                    fFamilyName.c_str(), fStyles[index], /*bcp47=*/nullptr,
                    /*bcp47Count=*/0, /*character=*/0,
                    /*allow_fallback=*/false, /*exact_style_match=*/true);
        }

        return fTypefaces[index];
    }

    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        return matchStyleCSS3(pattern);
    }

private:
    sk_sp<SkFontMgr_Fuchsia> fFontManager;
    std::string fFamilyName;
    std::vector<SkFontStyle> fStyles;
    std::vector<sk_sp<SkTypeface>> fTypefaces;
};

SkFontMgr_Fuchsia::SkFontMgr_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider)
        : fFontProvider(std::move(provider)), fBufferCache(sk_make_sp<SkFuchsiaFontDataCache>()) {}

SkFontMgr_Fuchsia::~SkFontMgr_Fuchsia() = default;

int SkFontMgr_Fuchsia::onCountFamilies() const {
    // Family enumeration is not supported.
    return 0;
}

void SkFontMgr_Fuchsia::onGetFamilyName(int index, SkString* familyName) const {
    // Family enumeration is not supported.
    familyName->reset();
}

sk_sp<SkFontStyleSet> SkFontMgr_Fuchsia::onCreateStyleSet(int index) const {
    // Family enumeration is not supported.
    return nullptr;
}

sk_sp<SkFontStyleSet> SkFontMgr_Fuchsia::onMatchFamily(const char familyName[]) const {
    fuchsia::fonts::FamilyName typedFamilyName;
    typedFamilyName.name = familyName;

    fuchsia::fonts::FontFamilyInfo familyInfo;
    int result = fFontProvider->GetFontFamilyInfo(typedFamilyName, &familyInfo);
    if (result != ZX_OK || !familyInfo.has_styles() || familyInfo.styles().empty()) return nullptr;

    std::vector<SkFontStyle> styles;
    for (auto& style : familyInfo.styles()) {
        styles.push_back(SkFontStyle(style.weight(), FuchsiaToSkWidth(style.width()),
                                     FuchsiaToSkSlant(style.slant())));
    }

    return sk_sp<SkFontStyleSet>(
        new SkFontStyleSet_Fuchsia(sk_ref_sp(this), familyInfo.name().name, std::move(styles)));
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onMatchFamilyStyle(const char familyName[],
                                                        const SkFontStyle& style) const {
    return FetchTypeface(familyName, style, /*bcp47=*/nullptr, /*bcp47Count=*/0, /*character=*/0,
                         /*allow_fallback=*/false, /*exact_style_match=*/false);
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onMatchFamilyStyleCharacter(
    const char familyName[], const SkFontStyle& style,
    const char* bcp47[], int bcp47Count,
    SkUnichar character) const
{
    return FetchTypeface(familyName, style, bcp47, bcp47Count, character,
                         /*allow_fallback=*/true, /*exact_style_match=*/false);
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onMakeFromData(sk_sp<SkData> data, int ttcIndex) const {
    return makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> asset,
                                                           int ttcIndex) const {
    return makeFromStream(std::move(asset), SkFontArguments().setCollectionIndex(ttcIndex));
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> asset,
                                                          const SkFontArguments& args) const {
    return CreateTypefaceFromSkStream(std::move(asset), args, kNullTypefaceId);
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onMakeFromFile(const char path[], int ttcIndex) const {
    return makeFromStream(std::make_unique<SkFILEStream>(path), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onLegacyMakeTypeface(const char familyName[],
                                                          SkFontStyle style) const {
    return sk_sp<SkTypeface>(matchFamilyStyle(familyName, style));
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::FetchTypeface(const char familyName[],
                                                   const SkFontStyle& style, const char* bcp47[],
                                                   int bcp47Count, SkUnichar character,
                                                   bool allow_fallback,
                                                   bool exact_style_match) const {
    fuchsia::fonts::TypefaceQuery query;
    query.set_style(SkToFuchsiaStyle(style));

    if (bcp47Count > 0) {
        std::vector<fuchsia::intl::LocaleId> languages{};
        for (int i = 0; i < bcp47Count; i++) {
            fuchsia::intl::LocaleId localeId;
            localeId.id = bcp47[i];
            languages.push_back(localeId);
        }
        query.set_languages(std::move(languages));
    }

    if (character) {
        query.set_code_points({static_cast<uint32_t>(character)});
    }

    // If family name is not specified or is a generic family name (e.g. "serif"), then enable
    // fallback; otherwise, pass the family name as is.
    fuchsia::fonts::GenericFontFamily genericFontFamily =
            fuchsia::fonts::GenericFontFamily::SANS_SERIF;
    bool isGenericFontFamily = GetGenericFontFamilyByName(familyName, &genericFontFamily);
    if (!familyName || *familyName == '\0' || isGenericFontFamily) {
        if (isGenericFontFamily) {
            query.set_fallback_family(genericFontFamily);
        }
        allow_fallback = true;
    } else {
        fuchsia::fonts::FamilyName typedFamilyName{};
        typedFamilyName.name = familyName;
        query.set_family(typedFamilyName);
    }

    fuchsia::fonts::TypefaceRequestFlags flags{};
    if (!allow_fallback) flags |= fuchsia::fonts::TypefaceRequestFlags::EXACT_FAMILY;
    if (exact_style_match) flags |= fuchsia::fonts::TypefaceRequestFlags::EXACT_STYLE;

    fuchsia::fonts::TypefaceRequest request;
    request.set_query(std::move(query));
    request.set_flags(flags);

    fuchsia::fonts::TypefaceResponse response;
    int result = fFontProvider->GetTypeface(std::move(request), &response);
    if (result != ZX_OK) return nullptr;

    // The service may return an empty response if there is no font matching the request.
    if (response.IsEmpty()) return nullptr;

    return GetOrCreateTypeface(TypefaceId{response.buffer_id(), response.font_index()},
                               response.buffer());
}

static bool FindByTypefaceId(SkTypeface* cachedTypeface, void* ctx) {
    SkTypeface_Fuchsia* cachedFuchsiaTypeface = static_cast<SkTypeface_Fuchsia*>(cachedTypeface);
    TypefaceId* id = static_cast<TypefaceId*>(ctx);

    return cachedFuchsiaTypeface->id() == *id;
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::GetOrCreateTypeface(TypefaceId id,
                                                         const fuchsia::mem::Buffer& buffer) const {
    SkAutoMutexExclusive mutexLock(fCacheMutex);

    sk_sp<SkTypeface> cached = fTypefaceCache.findByProcAndRef(FindByTypefaceId, &id);
    if (cached) return cached;

    sk_sp<SkData> data = fBufferCache->GetOrCreateSkData(id.bufferId, buffer);
    if (!data) return nullptr;

    auto result = CreateTypefaceFromSkData(std::move(data), id);
    fTypefaceCache.add(result);
    return result;
}

sk_sp<SkFontMgr> SkFontMgr_New_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider) {
    return sk_make_sp<SkFontMgr_Fuchsia>(std::move(provider));
}
