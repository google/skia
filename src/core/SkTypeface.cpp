/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypeface.h"

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTemplates.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkEndian.h"
#include "src/base/SkNoDestructor.h"
#include "src/base/SkUTF.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkTypefaceCache.h"
#include "src/sfnt/SkOTTable_OS_2.h"

#ifdef SK_TYPEFACE_FACTORY_FREETYPE
#include "src/ports/SkTypeface_FreeType.h"
#endif

#ifdef SK_TYPEFACE_FACTORY_CORETEXT
#include "src/ports/SkTypeface_mac_ct.h"
#endif

#ifdef SK_TYPEFACE_FACTORY_DIRECTWRITE
#include "src/ports/SkTypeface_win_dw.h"
#endif

// TODO(skbug.com/40045343): This needs to be set by Bazel rules.
#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
#include "src/ports/SkTypeface_fontations_priv.h"
#endif

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <vector>

using namespace skia_private;

SkTypeface::SkTypeface(const SkFontStyle& style, bool isFixedPitch)
    : fUniqueID(SkTypefaceCache::NewTypefaceID()), fStyle(style), fIsFixedPitch(isFixedPitch) { }

SkTypeface::~SkTypeface() { }

///////////////////////////////////////////////////////////////////////////////

namespace {

class SkEmptyTypeface : public SkTypeface {
public:
    static sk_sp<SkTypeface> Make() {
        static SkNoDestructor<SkEmptyTypeface> instance;
        return sk_ref_sp(instance.get());
    }

    static constexpr SkTypeface::FactoryId FactoryId = SkSetFourByteTag('e','m','t','y');
    static sk_sp<SkTypeface> MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                            const SkFontArguments&) {
        if (stream->getLength() == 0) {
            return SkEmptyTypeface::Make();
        }
        return nullptr;
    }
protected:
    friend SkNoDestructor<SkEmptyTypeface>;
    SkEmptyTypeface() : SkTypeface(SkFontStyle(), true) { }

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override { return nullptr; }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    std::unique_ptr<SkScalerContext> onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const override
    {
        return SkScalerContext::MakeEmpty(*const_cast<SkEmptyTypeface*>(this), effects, desc);
    }
    void onFilterRec(SkScalerContextRec*) const override { }
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        return nullptr;
    }
    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        desc->setFactoryId(FactoryId);
        *serialize = false;
    }
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override {
        sk_bzero(glyphs, count * sizeof(glyphs[0]));
    }
    int onCountGlyphs() const override { return 0; }
    void getPostScriptGlyphNames(SkString*) const override {}
    void getGlyphToUnicodeMap(SkUnichar*) const override {}
    int onGetUPEM() const override { return 0; }
    bool onComputeBounds(SkRect* bounds) const override { return false; }

    class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
    public:
        bool next(SkTypeface::LocalizedString*) override { return false; }
    };
    void onGetFamilyName(SkString* familyName) const override {
        familyName->reset();
    }
    bool onGetPostScriptName(SkString*) const override {
        return false;
    }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        return new EmptyLocalizedStrings;
    }
    bool onGlyphMaskNeedsCurrentColor() const override {
        return false;
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

sk_sp<SkTypeface> SkTypeface::MakeEmpty() {
    return SkEmptyTypeface::Make();
}

bool SkTypeface::Equal(const SkTypeface* facea, const SkTypeface* faceb) {
    if (facea == faceb) {
        return true;
    }
    if (!facea || !faceb) {
        return false;
    }
    return facea->uniqueID() == faceb->uniqueID();
}

///////////////////////////////////////////////////////////////////////////////

namespace {

    struct DecoderProc {
        SkFourByteTag id;
        sk_sp<SkTypeface> (*makeFromStream)(std::unique_ptr<SkStreamAsset>, const SkFontArguments&);
    };

    std::vector<DecoderProc>* decoders() {
        static SkNoDestructor<std::vector<DecoderProc>> decoders{{
            { SkEmptyTypeface::FactoryId, SkEmptyTypeface::MakeFromStream },
            { SkCustomTypefaceBuilder::FactoryId, SkCustomTypefaceBuilder::MakeFromStream },
#ifdef SK_TYPEFACE_FACTORY_CORETEXT
            { SkTypeface_Mac::FactoryId, SkTypeface_Mac::MakeFromStream },
#endif
#ifdef SK_TYPEFACE_FACTORY_DIRECTWRITE
            { DWriteFontTypeface::FactoryId, DWriteFontTypeface::MakeFromStream },
#endif
#ifdef SK_TYPEFACE_FACTORY_FREETYPE
            { SkTypeface_FreeType::FactoryId, SkTypeface_FreeType::MakeFromStream },
#endif
#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
            { SkTypeface_Fontations::FactoryId, SkTypeface_Fontations::MakeFromStream },
#endif
        }};
        return decoders.get();
    }

}  // namespace

sk_sp<SkTypeface> SkTypeface::makeClone(const SkFontArguments& args) const {
    return this->onMakeClone(args);
}

///////////////////////////////////////////////////////////////////////////////

void SkTypeface::Register(
            FactoryId id,
            sk_sp<SkTypeface> (*make)(std::unique_ptr<SkStreamAsset>, const SkFontArguments&)) {
    decoders()->push_back(DecoderProc{id, make});
}

void SkTypeface::serialize(SkWStream* wstream, SerializeBehavior behavior) const {
    bool isLocalData = false;
    SkFontDescriptor desc;
    this->onGetFontDescriptor(&desc, &isLocalData);
    if (desc.getFactoryId() == 0) {
        SkDEBUGF("Factory was not set for %s.\n", desc.getFamilyName());
    }

    bool shouldSerializeData = false;
    switch (behavior) {
        case SerializeBehavior::kDoIncludeData:      shouldSerializeData = true;        break;
        case SerializeBehavior::kDontIncludeData:    shouldSerializeData = false;       break;
        case SerializeBehavior::kIncludeDataIfLocal: shouldSerializeData = isLocalData; break;
    }

    if (shouldSerializeData) {
        int index;
        desc.setStream(this->openStream(&index));
        if (desc.hasStream()) {
            desc.setCollectionIndex(index);
        }

        int numAxes = this->getVariationDesignPosition({});
        if (0 < numAxes) {
            numAxes = this->getVariationDesignPosition({desc.setVariationCoordinates(numAxes), numAxes});
            if (numAxes <= 0) {
                desc.setVariationCoordinates(0);
            }
        }
    }
    desc.serialize(wstream);
}

sk_sp<SkData> SkTypeface::serialize(SerializeBehavior behavior) const {
    SkDynamicMemoryWStream stream;
    this->serialize(&stream, behavior);
    return stream.detachAsData();
}

sk_sp<SkTypeface> SkTypeface::MakeDeserialize(SkStream* stream, sk_sp<SkFontMgr> lastResortMgr) {
    SkFontDescriptor desc;
    if (!SkFontDescriptor::Deserialize(stream, &desc)) {
        return nullptr;
    }

    if (desc.hasStream()) {
        for (const DecoderProc& proc : *decoders()) {
            if (proc.id == desc.getFactoryId()) {
                return proc.makeFromStream(desc.detachStream(), desc.getFontArguments());
            }
        }

        [[maybe_unused]] FactoryId id = desc.getFactoryId();
        SkDEBUGF("Could not find factory %c%c%c%c for %s.\n",
                 (char)((id >> 24) & 0xFF),
                 (char)((id >> 16) & 0xFF),
                 (char)((id >> 8) & 0xFF),
                 (char)((id >> 0) & 0xFF),
                 desc.getFamilyName());

        if (lastResortMgr) {
            // If we've gotten to here, we will try desperately to find something that might match
            // as a kind of last ditch effort to make something work (and maybe this SkFontMgr knows
            // something about the serialization and can look up the right thing by name anyway if
            // the user provides it).
            // Any time it is used the user will probably get the wrong glyphs drawn (and if they're
            // right it is totally by accident). But sometimes drawing something or getting lucky
            // while debugging is better than drawing nothing at all.
            sk_sp<SkTypeface> typeface = lastResortMgr->makeFromStream(desc.detachStream(),
                                                                       desc.getFontArguments());
            if (typeface) {
                return typeface;
            }
        }
    }
    if (lastResortMgr) {
        return lastResortMgr->legacyMakeTypeface(desc.getFamilyName(), desc.getStyle());
    }
    return SkEmptyTypeface::Make();
}

///////////////////////////////////////////////////////////////////////////////

bool SkTypeface::glyphMaskNeedsCurrentColor() const {
    return this->onGlyphMaskNeedsCurrentColor();
}

int SkTypeface::getVariationDesignPosition(
        SkSpan<SkFontArguments::VariationPosition::Coordinate> coordinates) const
{
    return this->onGetVariationDesignPosition(coordinates.data(), coordinates.size());
}

int SkTypeface::getVariationDesignParameters(
        SkSpan<SkFontParameters::Variation::Axis> parameters) const
{
    return this->onGetVariationDesignParameters(parameters.data(), parameters.size());
}

int SkTypeface::countTables() const {
    return this->onGetTableTags(nullptr);
}

int SkTypeface::readTableTags(SkSpan<SkFontTableTag> tags) const {
    return this->onGetTableTags(tags.size() ? tags.data() : nullptr);
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

std::unique_ptr<SkStreamAsset> SkTypeface::openExistingStream(int* ttcIndex) const {
    int ttcIndexStorage;
    if (nullptr == ttcIndex) {
        // So our subclasses don't need to check for null param
        ttcIndex = &ttcIndexStorage;
    }
    return this->onOpenExistingStream(ttcIndex);
}

std::unique_ptr<SkScalerContext> SkTypeface::createScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    std::unique_ptr<SkScalerContext> scalerContext = this->onCreateScalerContext(effects, desc);
    SkASSERT(scalerContext);
    return scalerContext;
}

std::unique_ptr<SkScalerContext> SkTypeface::onCreateScalerContextAsProxyTypeface
        (const SkScalerContextEffects&,
         const SkDescriptor*,
         SkTypeface*) const {
    SK_ABORT("Not implemented.");
}

void SkTypeface::unicharsToGlyphs(SkSpan<const SkUnichar> uni, SkSpan<SkGlyphID> glyphs) const {
    if (const size_t n = std::min(uni.size(), glyphs.size())) {
        this->onCharsToGlyphs(uni.data(), n, glyphs.data());
    }
}

SkGlyphID SkTypeface::unicharToGlyph(SkUnichar uni) const {
    SkGlyphID glyphs[1] = { 0 };
    this->onCharsToGlyphs(&uni, 1, glyphs);
    return glyphs[0];
}

namespace {
class SkConvertToUTF32 {
public:
    SkConvertToUTF32() {}

    const SkUnichar* convert(const void* text, size_t byteLength, SkTextEncoding encoding) {
        const SkUnichar* uni;
        switch (encoding) {
            case SkTextEncoding::kUTF8: {
                uni = fStorage.reset(byteLength);
                const char* ptr = (const char*)text;
                const char* end = ptr + byteLength;
                for (int i = 0; ptr < end; ++i) {
                    fStorage[i] = SkUTF::NextUTF8(&ptr, end);
                }
            } break;
            case SkTextEncoding::kUTF16: {
                uni = fStorage.reset(byteLength);
                const uint16_t* ptr = (const uint16_t*)text;
                const uint16_t* end = ptr + (byteLength >> 1);
                for (int i = 0; ptr < end; ++i) {
                    fStorage[i] = SkUTF::NextUTF16(&ptr, end);
                }
            } break;
            case SkTextEncoding::kUTF32:
                uni = (const SkUnichar*)text;
                break;
            default:
                SK_ABORT("unexpected enum");
        }
        return uni;
    }

private:
    AutoSTMalloc<256, SkUnichar> fStorage;
};
}

size_t SkTypeface::textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding,
                                SkSpan<SkGlyphID> glyphs) const {
    if (0 == byteLength) {
        return 0;
    }

    SkASSERT(text);

    size_t count = SkFontPriv::CountTextElements(text, byteLength, encoding);
    if (count > glyphs.size()) {
        return count;
    }

    if (encoding == SkTextEncoding::kGlyphID) {
        memcpy(glyphs.data(), text, count << 1);
        return count;
    }

    SkConvertToUTF32 storage;
    const SkUnichar* uni = storage.convert(text, byteLength, encoding);

    this->unicharsToGlyphs({uni, count}, glyphs);
    return count;
}

int SkTypeface::countGlyphs() const {
    return this->onCountGlyphs();
}

int SkTypeface::getUnitsPerEm() const {
    // should we try to cache this in the base-class?
    return this->onGetUPEM();
}

bool SkTypeface::getKerningPairAdjustments(SkSpan<const SkGlyphID> glyphs,
                                           SkSpan<int32_t> adjustments) const {
    // check for the only legal way to pass a nullptr.. everything is 0
    // in which case they just want to know if this face can possibly support
    // kerning (true) or never (false).
    if (glyphs.empty() || adjustments.empty()) {
        SkASSERT(glyphs.empty() && adjustments.empty());
    }

    // If we update/the on-virtual to take spans, we won't need this
    // But for now, we need to limit the # of glyphs to the number of adjustments - 1
    size_t count = glyphs.size();
    if (count > 0) {
        count = std::min(count - 1, adjustments.size());
    }

    return this->onGetKerningPairAdjustments(glyphs.data(), count, adjustments.data());
}

SkTypeface::LocalizedStrings* SkTypeface::createFamilyNameIterator() const {
    return this->onCreateFamilyNameIterator();
}

void SkTypeface::getFamilyName(SkString* name) const {
    SkASSERT(name);
    this->onGetFamilyName(name);
}

bool SkTypeface::getPostScriptName(SkString* name) const {
    return this->onGetPostScriptName(name);
}

int SkTypeface::getResourceName(SkString* resourceName) const {
    return this->onGetResourceName(resourceName);
}

int SkTypeface::onGetResourceName(SkString* resourceName) const {
    return 0;
}

SkFontStyle SkTypeface::fontStyle() const {
    return this->onGetFontStyle();
}

SkFontStyle SkTypeface::onGetFontStyle() const {
    return fStyle;
}

bool SkTypeface::isBold() const {
    return this->onGetFontStyle().weight() >= SkFontStyle::kSemiBold_Weight;
}

bool SkTypeface::isItalic() const {
    return this->onGetFontStyle().slant() != SkFontStyle::kUpright_Slant;
}

bool SkTypeface::isFixedPitch() const {
    return this->onGetFixedPitch();
}

bool SkTypeface::onGetFixedPitch() const {
    return fIsFixedPitch;
}

void SkTypeface::getGlyphToUnicodeMap(SkUnichar* dst) const {
    sk_bzero(dst, sizeof(SkUnichar) * this->countGlyphs());
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface::getAdvancedMetrics() const {
    std::unique_ptr<SkAdvancedTypefaceMetrics> result = this->onGetAdvancedMetrics();
    if (result && result->fPostScriptName.isEmpty()) {
        if (!this->getPostScriptName(&result->fPostScriptName)) {
            this->getFamilyName(&result->fPostScriptName);
        }
    }
    if (result && (result->fType == SkAdvancedTypefaceMetrics::kTrueType_Font ||
                   result->fType == SkAdvancedTypefaceMetrics::kCFF_Font)) {
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

bool SkTypeface::onGetKerningPairAdjustments(const SkGlyphID glyphs[], int count,
                                             int32_t adjustments[]) const {
    return false;
}

std::unique_ptr<SkStreamAsset> SkTypeface::onOpenExistingStream(int* ttcIndex) const {
    return this->onOpenStream(ttcIndex);
}

///////////////////////////////////////////////////////////////////////////////

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

    std::unique_ptr<SkScalerContext> ctx = this->createScalerContext(noeffects, ad.getDesc());

    SkFontMetrics fm;
    ctx->getFontMetrics(&fm);
    if (!fm.hasBounds()) {
        return false;
    }
    bounds->setLTRB(fm.fXMin * invTextSize, fm.fTop * invTextSize,
                    fm.fXMax * invTextSize, fm.fBottom * invTextSize);
    return true;
}
