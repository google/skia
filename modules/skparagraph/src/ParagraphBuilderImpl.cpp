// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkStringUtils.h"

#if !defined(SK_DISABLE_LEGACY_PARAGRAPH_UNICODE)
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_client.h"
#endif

#endif  // !defined(SK_DISABLE_LEGACY_PARAGRAPH_UNICODE)

#include <memory>
#include <utility>

namespace skia {
namespace textlayout {

#if !defined(SK_DISABLE_LEGACY_PARAGRAPH_UNICODE)

namespace {
// TODO(kjlubick,jlavrova) Remove these defines by having clients register something or somehow
// plumbing this all into the animation builder factories.
sk_sp<SkUnicode> get_unicode() {
#ifdef SK_UNICODE_ICU_IMPLEMENTATION
    if (auto unicode = SkUnicodes::ICU::Make()) {
        return unicode;
    }
#endif
#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
    if (auto unicode = SkUnicodes::ICU4X::Make()) {
        return unicode;
    }
#endif
#ifdef SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION
    if (auto unicode = SkUnicodes::Libgrapheme::Make()) {
        return unicode;
    }
#endif
    return nullptr;
}
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilder::make(const ParagraphStyle& style,
                                                         sk_sp<FontCollection> fontCollection) {
    return ParagraphBuilderImpl::make(style, std::move(fontCollection), get_unicode());
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(const ParagraphStyle& style,
                                                             sk_sp<FontCollection> fontCollection) {
    return std::make_unique<ParagraphBuilderImpl>(style, std::move(fontCollection), get_unicode());
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection)
        : ParagraphBuilderImpl(style, std::move(fontCollection), get_unicode())
{ }

#endif  // !defined(SK_DISABLE_LEGACY_PARAGRAPH_UNICODE)

std::unique_ptr<ParagraphBuilder> ParagraphBuilder::make(const ParagraphStyle& style,
                                                         sk_sp<FontCollection> fontCollection,
                                                         sk_sp<SkUnicode> unicode) {
    return ParagraphBuilderImpl::make(style, std::move(fontCollection), std::move(unicode));
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(const ParagraphStyle& style,
                                                             sk_sp<FontCollection> fontCollection,
                                                             sk_sp<SkUnicode> unicode) {
    return std::make_unique<ParagraphBuilderImpl>(style, std::move(fontCollection),
                                                  std::move(unicode));
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection, sk_sp<SkUnicode> unicode)
        : ParagraphBuilder()
        , fUtf8()
        , fFontCollection(std::move(fontCollection))
        , fParagraphStyle(style)
        , fUnicode(std::move(unicode))
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
        , fTextIsFinalized(false)
        , fUsingClientInfo(false)
#endif
{
    SkASSERT(fFontCollection);
    startStyledBlock();
}

ParagraphBuilderImpl::~ParagraphBuilderImpl() = default;

void ParagraphBuilderImpl::pushStyle(const TextStyle& style) {
    fTextStyles.push_back(style);
    if (!fStyledBlocks.empty() && fStyledBlocks.back().fRange.end == fUtf8.size() &&
        fStyledBlocks.back().fStyle == style) {
        // Just continue with the same style
    } else {
        // Go with the new style
        startStyledBlock();
    }
}

void ParagraphBuilderImpl::pop() {
    if (!fTextStyles.empty()) {
        fTextStyles.pop_back();
    } else {
        // In this case we use paragraph style and skip Pop operation
        SkDEBUGF("SkParagraphBuilder.Pop() called too many times.\n");
    }

    this->startStyledBlock();
}

const TextStyle& ParagraphBuilderImpl::internalPeekStyle() {
    if (fTextStyles.empty()) {
        return fParagraphStyle.getTextStyle();
    } else {
        return fTextStyles.back();
    }
}

TextStyle ParagraphBuilderImpl::peekStyle() {
    return this->internalPeekStyle();
}

void ParagraphBuilderImpl::addText(const std::u16string& text) {
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    SkASSERT(!fTextIsFinalized);
#endif
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text);
    fUtf8.append(utf8);
}

void ParagraphBuilderImpl::addText(const char* text) {
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    SkASSERT(!fTextIsFinalized);
#endif
    fUtf8.append(text);
}

void ParagraphBuilderImpl::addText(const char* text, size_t len) {
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    SkASSERT(!fTextIsFinalized);
#endif
    fUtf8.append(text, len);
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle) {
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    SkASSERT(!fTextIsFinalized);
#endif
    addPlaceholder(placeholderStyle, false);
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle, bool lastOne) {
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    // The very last placeholder is added automatically
    // and only AFTER finalize() is called
    SkASSERT(!fTextIsFinalized || lastOne);
#endif
    if (!fUtf8.isEmpty() && !lastOne) {
        // We keep the very last text style
        this->endRunIfNeeded();
    }

    BlockRange stylesBefore(fPlaceholders.empty() ? 0 : fPlaceholders.back().fBlocksBefore.end + 1,
                            fStyledBlocks.size());
    TextRange textBefore(fPlaceholders.empty() ? 0 : fPlaceholders.back().fRange.end,
                            fUtf8.size());
    auto start = fUtf8.size();
    auto topStyle = internalPeekStyle();
    if (!lastOne) {
        pushStyle(topStyle.cloneForPlaceholder());
        addText(std::u16string(1ull, 0xFFFC));
        pop();
    }
    auto end = fUtf8.size();
    fPlaceholders.emplace_back(start, end, placeholderStyle, topStyle, stylesBefore, textBefore);
}

void ParagraphBuilderImpl::endRunIfNeeded() {
    if (fStyledBlocks.empty()) {
        return;
    }

    auto& last = fStyledBlocks.back();
    if (last.fRange.start == fUtf8.size()) {
        fStyledBlocks.pop_back();
    } else {
        last.fRange.end = fUtf8.size();
    }
}

void ParagraphBuilderImpl::startStyledBlock() {
    endRunIfNeeded();
    fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), internalPeekStyle());
}

void ParagraphBuilderImpl::finalize() {
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    if (fTextIsFinalized) {
        return;
    }
#endif
    if (!fUtf8.isEmpty()) {
        this->endRunIfNeeded();
    }

#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    fTextIsFinalized = true;
#endif
}

std::unique_ptr<Paragraph> ParagraphBuilderImpl::Build() {
    this->finalize();
    // Add one fake placeholder with the rest of the text
    this->addPlaceholder(PlaceholderStyle(), true);

    fUTF8IndexForUTF16Index.clear();
    fUTF16IndexForUTF8Index.clear();
#if !defined(SK_DISABLE_LEGACY_PARAGRAPH_UNICODE) && defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    if (fUsingClientInfo && !fUnicode) {
        // This is the place where SkUnicode is paired with SkParagraph
        fUnicode = SkUnicodes::Client::Make(this->getText(),
                                            std::move(fWordsUtf16),
                                            std::move(fGraphemeBreaksUtf8),
                                            std::move(fLineBreaksUtf8));
    }
#endif

    SkASSERT_RELEASE(fUnicode);
    return std::make_unique<ParagraphImpl>(
            fUtf8, fParagraphStyle, fStyledBlocks, fPlaceholders, fFontCollection, fUnicode);
}

SkSpan<char> ParagraphBuilderImpl::getText() {
    this->finalize();
    return SkSpan<char>(fUtf8.isEmpty() ? nullptr : fUtf8.data(), fUtf8.size());
}

const ParagraphStyle& ParagraphBuilderImpl::getParagraphStyle() const {
    return fParagraphStyle;
}

void ParagraphBuilderImpl::ensureUTF16Mapping() {
    fillUTF16MappingOnce([&] {
        SkUnicode::extractUtfConversionMapping(
                this->getText(),
                [&](size_t index) { fUTF8IndexForUTF16Index.emplace_back(index); },
                [&](size_t index) { fUTF16IndexForUTF8Index.emplace_back(index); });
    });
}

#if !defined(SK_DISABLE_LEGACY_CLIENT_UNICODE) && defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
void ParagraphBuilderImpl::setWordsUtf8(std::vector<SkUnicode::Position> wordsUtf8) {
    ensureUTF16Mapping();
    std::vector<SkUnicode::Position> wordsUtf16;
    for (SkUnicode::Position indexUtf8: wordsUtf8) {
        wordsUtf16.emplace_back(fUTF16IndexForUTF8Index[indexUtf8]);
    }
    setWordsUtf16(wordsUtf16);
}

void ParagraphBuilderImpl::setWordsUtf16(std::vector<SkUnicode::Position> wordsUtf16) {
    fUsingClientInfo = true;
    fWordsUtf16 = std::move(wordsUtf16);
}

void ParagraphBuilderImpl::setGraphemeBreaksUtf8(std::vector<SkUnicode::Position> graphemeBreaksUtf8) {
    fUsingClientInfo = true;
    fGraphemeBreaksUtf8 = std::move(graphemeBreaksUtf8);
}

void ParagraphBuilderImpl::setGraphemeBreaksUtf16(std::vector<SkUnicode::Position> graphemeBreaksUtf16) {
    ensureUTF16Mapping();
    std::vector<SkUnicode::Position> graphemeBreaksUtf8;
    for (SkUnicode::Position indexUtf16: graphemeBreaksUtf16) {
        graphemeBreaksUtf8.emplace_back(fUTF8IndexForUTF16Index[indexUtf16]);
    }
    setGraphemeBreaksUtf8(graphemeBreaksUtf8);
}

void ParagraphBuilderImpl::setLineBreaksUtf8(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8) {
    fUsingClientInfo = true;
    fLineBreaksUtf8 = std::move(lineBreaksUtf8);
}

void ParagraphBuilderImpl::setLineBreaksUtf16(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf16) {
    ensureUTF16Mapping();
    std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8;
    for (SkUnicode::LineBreakBefore lineBreakUtf16: lineBreaksUtf16) {
        lineBreaksUtf8.emplace_back(SkUnicode::LineBreakBefore(
                fUTF8IndexForUTF16Index[lineBreakUtf16.pos], lineBreakUtf16.breakType));
    }
    setLineBreaksUtf8(lineBreaksUtf8);
}
#endif

void ParagraphBuilderImpl::Reset() {

    fTextStyles.clear();
    fUtf8.reset();
    fStyledBlocks.clear();
    fPlaceholders.clear();
    fUTF8IndexForUTF16Index.clear();
    fUTF16IndexForUTF8Index.clear();
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    fWordsUtf16.clear();
    fGraphemeBreaksUtf8.clear();
    fLineBreaksUtf8.clear();
    fTextIsFinalized = false;
#endif
    startStyledBlock();
}

bool ParagraphBuilderImpl::RequiresClientICU() {
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    return true;
#else
    return false;
#endif
}

}  // namespace textlayout
}  // namespace skia
