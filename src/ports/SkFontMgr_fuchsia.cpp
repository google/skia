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

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkTypefaceCache.h"

void UnmapMemory(const void* buffer, uint64_t size) {
    static_assert(sizeof(void*) == sizeof(uint64_t), "pointers aren't 64-bit");
    zx::vmar::root_self()->unmap(reinterpret_cast<uintptr_t>(buffer), size);
}

struct ReleaseSkDataContext {
    uint64_t fBufferSize;
    std::function<void()> releaseProc;

    ReleaseSkDataContext(uint64_t bufferSize, const std::function<void()>& releaseProc)
            : fBufferSize(bufferSize), releaseProc(releaseProc) {}
};

void ReleaseSkData(const void* buffer, void* context) {
    auto releaseSkDataContext = reinterpret_cast<ReleaseSkDataContext*>(context);
    SkASSERT(releaseSkDataContext);
    UnmapMemory(buffer, releaseSkDataContext->fBufferSize);
    releaseSkDataContext->releaseProc();
    delete releaseSkDataContext;
}

sk_sp<SkData> MakeSkDataFromBuffer(const fuchsia::mem::Buffer& data,
                                   std::function<void()> release_proc) {
    uint64_t size = data.size;
    uintptr_t buffer = 0;
    zx_status_t status = zx::vmar::root_self()->map(0, data.vmo, 0, size, ZX_VM_PERM_READ, &buffer);
    if (status != ZX_OK) return nullptr;
    auto context = new ReleaseSkDataContext(size, release_proc);
    return SkData::MakeWithProc(reinterpret_cast<void*>(buffer), size, ReleaseSkData, context);
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

constexpr struct {
    const char* fName;
    fuchsia::fonts::FallbackGroup fFallbackGroup;
} kFallbackGroupsByName[] = {
        {"serif", fuchsia::fonts::FallbackGroup::SERIF},
        {"sans", fuchsia::fonts::FallbackGroup::SANS_SERIF},
        {"sans-serif", fuchsia::fonts::FallbackGroup::SANS_SERIF},
        {"mono", fuchsia::fonts::FallbackGroup::MONOSPACE},
        {"monospace", fuchsia::fonts::FallbackGroup::MONOSPACE},
        {"cursive", fuchsia::fonts::FallbackGroup::CURSIVE},
        {"fantasy", fuchsia::fonts::FallbackGroup::FANTASY},
};

fuchsia::fonts::FallbackGroup GetFallbackGroupByName(const char* name) {
    if (!name) return fuchsia::fonts::FallbackGroup::NONE;
    for (auto& group : kFallbackGroupsByName) {
        if (strcasecmp(group.fName, name) == 0) {
            return group.fFallbackGroup;
        }
    }
    return fuchsia::fonts::FallbackGroup::NONE;
}

struct TypefaceId {
    uint32_t bufferId;
    uint32_t ttcIndex;

    bool operator==(TypefaceId& other) {
        return std::tie(bufferId, ttcIndex) == std::tie(other.bufferId, other.ttcIndex);
    }
}

constexpr kNullTypefaceId = {0xFFFFFFFF, 0xFFFFFFFF};

class SkTypeface_Fuchsia : public SkTypeface_Stream {
public:
    SkTypeface_Fuchsia(std::unique_ptr<SkFontData> fontData, const SkFontStyle& style,
                       bool isFixedPitch, const SkString familyName, TypefaceId id)
            : SkTypeface_Stream(std::move(fontData), style, isFixedPitch,
                                /*sys_font=*/true, familyName)
            , fId(id) {}

    TypefaceId id() { return fId; }

private:
    TypefaceId fId;
};

sk_sp<SkTypeface> CreateTypefaceFromSkStream(std::unique_ptr<SkStreamAsset> stream,
                                             const SkFontArguments& args, TypefaceId id) {
    using Scanner = SkTypeface_FreeType::Scanner;
    Scanner scanner;
    bool isFixedPitch;
    SkFontStyle style;
    SkString name;
    Scanner::AxisDefinitions axisDefinitions;
    if (!scanner.scanFont(stream.get(), args.getCollectionIndex(), &name, &style, &isFixedPitch,
                          &axisDefinitions)) {
        return nullptr;
    }

    const SkFontArguments::VariationPosition position = args.getVariationDesignPosition();
    SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.count());
    Scanner::computeAxisValues(axisDefinitions, position, axisValues, name);

    auto fontData = std::make_unique<SkFontData>(std::move(stream), args.getCollectionIndex(),
                                                 axisValues.get(), axisDefinitions.count());
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
    SkFontStyleSet* onMatchFamily(const char familyName[]) const override;
    SkFontStyleSet* onCreateStyleSet(int index) const override;
    SkTypeface* onMatchFamilyStyle(const char familyName[], const SkFontStyle&) const override;
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override;
    SkTypeface* onMatchFaceStyle(const SkTypeface*, const SkFontStyle&) const override;
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

    sk_sp<SkData> GetOrCreateSkData(int bufferId, const fuchsia::mem::Buffer& buffer) const;
    void OnSkDataDeleted(int bufferId) const;

    sk_sp<SkTypeface> GetOrCreateTypeface(TypefaceId id, const fuchsia::mem::Buffer& buffer) const;

    mutable fuchsia::fonts::ProviderSyncPtr fFontProvider;

    mutable SkMutex fCacheMutex;

    // Must be accessed only with fCacheMutex acquired.
    mutable std::unordered_map<int, SkData*> fBufferCache;
    mutable SkTypefaceCache fTypefaceCache;
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

    SkTypeface* createTypeface(int index) override {
        SkASSERT(index >= 0 && index < static_cast<int>(fStyles.size()));

        if (fTypefaces.empty()) fTypefaces.resize(fStyles.size());

        if (!fTypefaces[index]) {
            fTypefaces[index] = fFontManager->FetchTypeface(
                    fFamilyName.c_str(), fStyles[index], /*bcp47=*/nullptr,
                    /*bcp47Count=*/0, /*character=*/0,
                    /*allow_fallback=*/false, /*exact_style_match=*/true);
        }

        return SkSafeRef(fTypefaces[index].get());
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override { return matchStyleCSS3(pattern); }

private:
    sk_sp<SkFontMgr_Fuchsia> fFontManager;
    std::string fFamilyName;
    std::vector<SkFontStyle> fStyles;
    std::vector<sk_sp<SkTypeface>> fTypefaces;
};

SkFontMgr_Fuchsia::SkFontMgr_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider)
        : fFontProvider(std::move(provider)) {}

SkFontMgr_Fuchsia::~SkFontMgr_Fuchsia() = default;

int SkFontMgr_Fuchsia::onCountFamilies() const {
    // Family enumeration is not supported.
    return 0;
}

void SkFontMgr_Fuchsia::onGetFamilyName(int index, SkString* familyName) const {
    // Family enumeration is not supported.
    familyName->reset();
}

SkFontStyleSet* SkFontMgr_Fuchsia::onCreateStyleSet(int index) const {
    // Family enumeration is not supported.
    return nullptr;
}

SkFontStyleSet* SkFontMgr_Fuchsia::onMatchFamily(const char familyName[]) const {
    fuchsia::fonts::FamilyInfoPtr familyInfo;
    int result = fFontProvider->GetFamilyInfo(familyName, &familyInfo);
    if (result != ZX_OK || !familyInfo) return nullptr;

    std::vector<SkFontStyle> styles;
    for (auto& style : familyInfo->styles) {
        styles.push_back(SkFontStyle(style.weight, style.width, FuchsiaToSkSlant(style.slant)));
    }

    return new SkFontStyleSet_Fuchsia(sk_ref_sp(this), familyInfo->name, std::move(styles));
}

SkTypeface* SkFontMgr_Fuchsia::onMatchFamilyStyle(const char familyName[],
                                                  const SkFontStyle& style) const {
    sk_sp<SkTypeface> typeface =
            FetchTypeface(familyName, style, /*bcp47=*/nullptr,
                          /*bcp47Count=*/0, /*character=*/0,
                          /*allow_fallback=*/false, /*exact_style_match=*/false);
    return typeface.release();
}

SkTypeface* SkFontMgr_Fuchsia::onMatchFamilyStyleCharacter(const char familyName[],
                                                           const SkFontStyle& style,
                                                           const char* bcp47[], int bcp47Count,
                                                           SkUnichar character) const {
    sk_sp<SkTypeface> typeface =
            FetchTypeface(familyName, style, bcp47, bcp47Count, character, /*allow_fallback=*/true,
                          /*exact_style_match=*/false);
    return typeface.release();
}

SkTypeface* SkFontMgr_Fuchsia::onMatchFaceStyle(const SkTypeface*, const SkFontStyle&) const {
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::onMakeFromData(sk_sp<SkData>, int ttcIndex) const {
    SkASSERT(false);
    return nullptr;
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
    fuchsia::fonts::Request request;
    request.weight = style.weight();
    request.width = style.width();
    request.slant = SkToFuchsiaSlant(style.slant());
    request.language.reset(std::vector<std::string>(bcp47, bcp47 + bcp47Count));
    request.character = character;
    request.fallback_group = GetFallbackGroupByName(familyName);

    // If family name is not specified or it is a generic fallback group name (e.g. "serif") then
    // enable fallback, otherwise pass the family name as is.
    if (!familyName || *familyName == '\0' ||
        request.fallback_group != fuchsia::fonts::FallbackGroup::NONE) {
        request.family = "";
        allow_fallback = true;
    } else {
        request.family = familyName;
    }

    request.flags = 0;
    if (!allow_fallback) request.flags |= fuchsia::fonts::REQUEST_FLAG_NO_FALLBACK;
    if (exact_style_match) request.flags |= fuchsia::fonts::REQUEST_FLAG_EXACT_MATCH;

    fuchsia::fonts::ResponsePtr response;
    int result = fFontProvider->GetFont(std::move(request), &response);
    if (result != ZX_OK) return nullptr;

    // The service may return null response if there is no font matching the request.
    if (!response) return nullptr;

    return GetOrCreateTypeface(TypefaceId{response->buffer_id, response->font_index},
                               response->buffer);
}

sk_sp<SkData> SkFontMgr_Fuchsia::GetOrCreateSkData(int bufferId,
                                                   const fuchsia::mem::Buffer& buffer) const {
    fCacheMutex.assertHeld();

    auto iter = fBufferCache.find(bufferId);
    if (iter != fBufferCache.end()) {
        return sk_ref_sp(iter->second);
    }
    auto font_mgr = sk_ref_sp(this);
    auto data = MakeSkDataFromBuffer(buffer,
                                     [font_mgr, bufferId]() { font_mgr->OnSkDataDeleted(bufferId); });
    if (!data) {
        return nullptr;
    }
    fBufferCache[bufferId] = data.get();
    return data;
}

void SkFontMgr_Fuchsia::OnSkDataDeleted(int bufferId) const {
    SK_UNUSED bool wasFound = fBufferCache.erase(bufferId) != 0;
    SkASSERT(wasFound);
}

static bool FindByTypefaceId(SkTypeface* cachedTypeface, void* ctx) {
    SkTypeface_Fuchsia* cachedFuchsiaTypeface = static_cast<SkTypeface_Fuchsia*>(cachedTypeface);
    TypefaceId* id = static_cast<TypefaceId*>(ctx);

    return cachedFuchsiaTypeface->id() == *id;
}

sk_sp<SkTypeface> SkFontMgr_Fuchsia::GetOrCreateTypeface(TypefaceId id,
                                                         const fuchsia::mem::Buffer& buffer) const {
    SkAutoMutexAcquire mutexLock(fCacheMutex);

    sk_sp<SkTypeface> cached = fTypefaceCache.findByProcAndRef(FindByTypefaceId, &id);
    if (cached) return cached;

    sk_sp<SkData> data = GetOrCreateSkData(id.bufferId, buffer);
    if (!data) return nullptr;

    auto result = CreateTypefaceFromSkData(std::move(data), id);
    fTypefaceCache.add(result);
    return result;
}

SK_API sk_sp<SkFontMgr> SkFontMgr_New_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider) {
    return sk_make_sp<SkFontMgr_Fuchsia>(std::move(provider));
}
