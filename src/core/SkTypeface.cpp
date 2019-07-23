/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkEndian.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkTypefaceCache.h"
#include "src/sfnt/SkOTTable_OS_2.h"

SkTypeface::SkTypeface(const SkFontStyle& style, bool isFixedPitch)
    : fUniqueID(SkTypefaceCache::NewFontID()), fStyle(style), fIsFixedPitch(isFixedPitch) { }

SkTypeface::~SkTypeface() { }

#ifdef SK_WHITELIST_SERIALIZED_TYPEFACES
extern void WhitelistSerializeTypeface(const SkTypeface*, SkWStream* );
#define SK_TYPEFACE_DELEGATE WhitelistSerializeTypeface
#else
#define SK_TYPEFACE_DELEGATE nullptr
#endif

void (*gSerializeTypefaceDelegate)(const SkTypeface*, SkWStream* ) = SK_TYPEFACE_DELEGATE;
sk_sp<SkTypeface> (*gDeserializeTypefaceDelegate)(SkStream* ) = nullptr;

///////////////////////////////////////////////////////////////////////////////

namespace {

class SkEmptyTypeface : public SkTypeface {
public:
    static sk_sp<SkTypeface> Make() { return sk_sp<SkTypeface>(new SkEmptyTypeface); }
protected:
    SkEmptyTypeface() : SkTypeface(SkFontStyle(), true) { }

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override { return nullptr; }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor*) const override {
        return nullptr;
    }
    void onFilterRec(SkScalerContextRec*) const override { }
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        return nullptr;
    }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override { }
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override {
        sk_bzero(glyphs, count * sizeof(glyphs[0]));
    }
    int onCountGlyphs() const override { return 0; }
    void getPostScriptGlyphNames(SkString*) const override {}
    void getGlyphToUnicodeMap(SkUnichar*) const override {}
    int onGetUPEM() const override { return 0; }
    class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
    public:
        bool next(SkTypeface::LocalizedString*) override { return false; }
    };
    void onGetFamilyName(SkString* familyName) const override {
        familyName->reset();
    }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        return new EmptyLocalizedStrings;
    }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override
    {
        return 0;
    }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override
    {
        return 0;
    }
    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override {
        return 0;
    }
};

}  // namespace

SkFontStyle SkTypeface::FromOldStyle(Style oldStyle) {
    return SkFontStyle((oldStyle & SkTypeface::kBold) ? SkFontStyle::kBold_Weight
                                                      : SkFontStyle::kNormal_Weight,
                       SkFontStyle::kNormal_Width,
                       (oldStyle & SkTypeface::kItalic) ? SkFontStyle::kItalic_Slant
                                                        : SkFontStyle::kUpright_Slant);
}

SkTypeface* SkTypeface::GetDefaultTypeface(Style style) {
    static SkOnce once[4];
    static sk_sp<SkTypeface> defaults[4];

    SkASSERT((int)style < 4);
    once[style]([style] {
        sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
        auto t = fm->legacyMakeTypeface(nullptr, FromOldStyle(style));
        defaults[style] = t ? t : SkEmptyTypeface::Make();
    });
    return defaults[style].get();
}

sk_sp<SkTypeface> SkTypeface::MakeDefault() {
    return sk_ref_sp(GetDefaultTypeface());
}

uint32_t SkTypeface::UniqueID(const SkTypeface* face) {
    if (nullptr == face) {
        face = GetDefaultTypeface();
    }
    return face->uniqueID();
}

bool SkTypeface::Equal(const SkTypeface* facea, const SkTypeface* faceb) {
    return facea == faceb || SkTypeface::UniqueID(facea) == SkTypeface::UniqueID(faceb);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkTypeface> SkTypeface::MakeFromName(const char name[],
                                           SkFontStyle fontStyle) {
    if (nullptr == name && (fontStyle.slant() == SkFontStyle::kItalic_Slant ||
                            fontStyle.slant() == SkFontStyle::kUpright_Slant) &&
                           (fontStyle.weight() == SkFontStyle::kBold_Weight ||
                            fontStyle.weight() == SkFontStyle::kNormal_Weight)) {
        return sk_ref_sp(GetDefaultTypeface(static_cast<SkTypeface::Style>(
            (fontStyle.slant() == SkFontStyle::kItalic_Slant ? SkTypeface::kItalic :
                                                               SkTypeface::kNormal) |
            (fontStyle.weight() == SkFontStyle::kBold_Weight ? SkTypeface::kBold :
                                                               SkTypeface::kNormal))));
    }
    return SkFontMgr::RefDefault()->legacyMakeTypeface(name, fontStyle);
}

sk_sp<SkTypeface> SkTypeface::MakeFromStream(std::unique_ptr<SkStreamAsset> stream, int index) {
    if (!stream) {
        return nullptr;
    }
    return SkFontMgr::RefDefault()->makeFromStream(std::move(stream), index);
}

sk_sp<SkTypeface> SkTypeface::MakeFromData(sk_sp<SkData> data, int index) {
    if (!data) {
        return nullptr;
    }
    return SkFontMgr::RefDefault()->makeFromData(std::move(data), index);
}

sk_sp<SkTypeface> SkTypeface::MakeFromFontData(std::unique_ptr<SkFontData> data) {
    return SkFontMgr::RefDefault()->makeFromFontData(std::move(data));
}

sk_sp<SkTypeface> SkTypeface::MakeFromFile(const char path[], int index) {
    return SkFontMgr::RefDefault()->makeFromFile(path, index);
}

sk_sp<SkTypeface> SkTypeface::makeClone(const SkFontArguments& args) const {
    return this->onMakeClone(args);
}

///////////////////////////////////////////////////////////////////////////////

void SkTypeface::serialize(SkWStream* wstream, SerializeBehavior behavior) const {
    if (gSerializeTypefaceDelegate) {
        (*gSerializeTypefaceDelegate)(this, wstream);
        return;
    }

    bool isLocalData = false;
    SkFontDescriptor desc;
    this->onGetFontDescriptor(&desc, &isLocalData);

    bool shouldSerializeData = false;
    switch (behavior) {
        case SerializeBehavior::kDoIncludeData:      shouldSerializeData = true;        break;
        case SerializeBehavior::kDontIncludeData:    shouldSerializeData = false;       break;
        case SerializeBehavior::kIncludeDataIfLocal: shouldSerializeData = isLocalData; break;
    }

    // TODO: why do we check hasFontData() and allow the data to pass through even if the caller
    //       has said they don't want the fontdata? Does this actually happen (getDescriptor returns
    //       fontdata as well?)
    if (shouldSerializeData && !desc.hasFontData()) {
        desc.setFontData(this->onMakeFontData());
    }
    desc.serialize(wstream);
}

sk_sp<SkData> SkTypeface::serialize(SerializeBehavior behavior) const {
    SkDynamicMemoryWStream stream;
    this->serialize(&stream, behavior);
    return stream.detachAsData();
}

sk_sp<SkTypeface> SkTypeface::MakeDeserialize(SkStream* stream) {
    if (gDeserializeTypefaceDelegate) {
        return (*gDeserializeTypefaceDelegate)(stream);
    }

    SkFontDescriptor desc;
    if (!SkFontDescriptor::Deserialize(stream, &desc)) {
        return nullptr;
    }

    std::unique_ptr<SkFontData> data = desc.detachFontData();
    if (data) {
        sk_sp<SkTypeface> typeface(SkTypeface::MakeFromFontData(std::move(data)));
        if (typeface) {
            return typeface;
        }
    }

    return SkTypeface::MakeFromName(desc.getFamilyName(), desc.getStyle());
}

///////////////////////////////////////////////////////////////////////////////

int SkTypeface::getVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const
{
    return this->onGetVariationDesignPosition(coordinates, coordinateCount);
}

int SkTypeface::getVariationDesignParameters(
        SkFontParameters::Variation::Axis parameters[], int parameterCount) const
{
    return this->onGetVariationDesignParameters(parameters, parameterCount);
}

int SkTypeface::countTables() const {
    return this->onGetTableTags(nullptr);
}

int SkTypeface::getTableTags(SkFontTableTag tags[]) const {
    return this->onGetTableTags(tags);
}

size_t SkTypeface::getTableSize(SkFontTableTag tag) const {
    return this->onGetTableData(tag, 0, ~0U, nullptr);
}

size_t SkTypeface::getTableData(SkFontTableTag tag, size_t offset, size_t length,
                                void* data) const {
    return this->onGetTableData(tag, offset, length, data);
}

sk_sp<SkData> SkTypeface::copyTableData(SkFontTableTag tag) const {
    return this->onCopyTableData(tag);
}

sk_sp<SkData> SkTypeface::onCopyTableData(SkFontTableTag tag) const {
    size_t size = this->getTableSize(tag);
    if (size) {
        sk_sp<SkData> data = SkData::MakeUninitialized(size);
        (void)this->getTableData(tag, 0, size, data->writable_data());
        return data;
    }
    return nullptr;
}

std::unique_ptr<SkStreamAsset> SkTypeface::openStream(int* ttcIndex) const {
    int ttcIndexStorage;
    if (nullptr == ttcIndex) {
        // So our subclasses don't need to check for null param
        ttcIndex = &ttcIndexStorage;
    }
    return this->onOpenStream(ttcIndex);
}

std::unique_ptr<SkFontData> SkTypeface::makeFontData() const {
    return this->onMakeFontData();
}

// This implementation is temporary until this method can be made pure virtual.
std::unique_ptr<SkFontData> SkTypeface::onMakeFontData() const {
    int index;
    std::unique_ptr<SkStreamAsset> stream(this->onOpenStream(&index));
    if (!stream) {
        return nullptr;
    }
    return skstd::make_unique<SkFontData>(std::move(stream), index, nullptr, 0);
};

void SkTypeface::unicharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    if (count > 0 && glyphs && uni) {
        this->onCharsToGlyphs(uni, count, glyphs);
    }
}

SkGlyphID SkTypeface::unicharToGlyph(SkUnichar uni) const {
    SkGlyphID glyphs[1] = { 0 };
    this->onCharsToGlyphs(&uni, 1, glyphs);
    return glyphs[0];
}

int SkTypeface::countGlyphs() const {
    return this->onCountGlyphs();
}

int SkTypeface::getUnitsPerEm() const {
    // should we try to cache this in the base-class?
    return this->onGetUPEM();
}

bool SkTypeface::getKerningPairAdjustments(const uint16_t glyphs[], int count,
                                           int32_t adjustments[]) const {
    SkASSERT(count >= 0);
    // check for the only legal way to pass a nullptr.. everything is 0
    // in which case they just want to know if this face can possibly support
    // kerning (true) or never (false).
    if (nullptr == glyphs || nullptr == adjustments) {
        SkASSERT(nullptr == glyphs);
        SkASSERT(0 == count);
        SkASSERT(nullptr == adjustments);
    }
    return this->onGetKerningPairAdjustments(glyphs, count, adjustments);
}

SkTypeface::LocalizedStrings* SkTypeface::createFamilyNameIterator() const {
    return this->onCreateFamilyNameIterator();
}

void SkTypeface::getFamilyName(SkString* name) const {
    SkASSERT(name);
    this->onGetFamilyName(name);
}

void SkTypeface::getGlyphToUnicodeMap(SkUnichar* dst) const {
    sk_bzero(dst, sizeof(SkUnichar) * this->countGlyphs());
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface::getAdvancedMetrics() const {
    std::unique_ptr<SkAdvancedTypefaceMetrics> result = this->onGetAdvancedMetrics();
    if (result && result->fPostScriptName.isEmpty()) {
        result->fPostScriptName = result->fFontName;
    }
    if (result && result->fType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        SkOTTableOS2::Version::V2::Type::Field fsType;
        constexpr SkFontTableTag os2Tag = SkTEndian_SwapBE32(SkOTTableOS2::TAG);
        constexpr size_t fsTypeOffset = offsetof(SkOTTableOS2::Version::V2, fsType);
        if (this->getTableData(os2Tag, fsTypeOffset, sizeof(fsType), &fsType) == sizeof(fsType)) {
            if (fsType.Bitmap || (fsType.Restricted && !(fsType.PreviewPrint || fsType.Editable))) {
                result->fFlags |= SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag;
            }
            if (fsType.NoSubsetting) {
                result->fFlags |= SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag;
            }
        }
    }
    return result;
}

bool SkTypeface::onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                             int32_t adjustments[]) const {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

#include "include/core/SkPaint.h"
#include "src/core/SkDescriptor.h"

SkRect SkTypeface::getBounds() const {
    fBoundsOnce([this] {
        if (!this->onComputeBounds(&fBounds)) {
            fBounds.setEmpty();
        }
    });
    return fBounds;
}

bool SkTypeface::onComputeBounds(SkRect* bounds) const {
    // we use a big size to ensure lots of significant bits from the scalercontext.
    // then we scale back down to return our final answer (at 1-pt)
    const SkScalar textSize = 2048;
    const SkScalar invTextSize = 1 / textSize;

    SkFont font;
    font.setTypeface(sk_ref_sp(const_cast<SkTypeface*>(this)));
    font.setSize(textSize);
    font.setLinearMetrics(true);

    SkScalerContextRec rec;
    SkScalerContextEffects effects;

    SkScalerContext::MakeRecAndEffectsFromFont(font, &rec, &effects);

    SkAutoDescriptor ad;
    SkScalerContextEffects noeffects;
    SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, noeffects, &ad);

    std::unique_ptr<SkScalerContext> ctx = this->createScalerContext(noeffects, ad.getDesc(), true);
    if (!ctx) {
        return false;
    }

    SkFontMetrics fm;
    ctx->getFontMetrics(&fm);
    bounds->set(fm.fXMin * invTextSize, fm.fTop * invTextSize,
                fm.fXMax * invTextSize, fm.fBottom * invTextSize);
    return true;
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface::onGetAdvancedMetrics() const {
    SkDEBUGFAIL("Typefaces that need to work with PDF backend must override this.");
    return nullptr;
}
