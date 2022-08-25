// Copyright 2019 Google LLC.

#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

#include <algorithm>
#include <utility>
#include "src/core/SkStringUtils.h"

namespace skia {
namespace textlayout {

std::unique_ptr<ParagraphBuilder> ParagraphBuilder::make(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection) {
    return ParagraphBuilderImpl::make(style, fontCollection);
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection) {
    auto unicode = SkUnicode::Make();
    if (nullptr == unicode) {
        return nullptr;
    }
    return std::make_unique<ParagraphBuilderImpl>(style, fontCollection);
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection, std::unique_ptr<SkUnicode> unicode) {
    if (nullptr == unicode) {
        return nullptr;
    }
    return std::make_unique<ParagraphBuilderImpl>(style, fontCollection, std::move(unicode));
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection, std::unique_ptr<SkUnicode> unicode)
        : ParagraphBuilder(style, fontCollection)
        , fUtf8()
        , fFontCollection(std::move(fontCollection))
        , fParagraphStyle(style)
        , fUnicode(std::move(unicode)) {
    startStyledBlock();
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection)
        : ParagraphBuilderImpl(style, fontCollection, SkUnicode::Make())
{ }

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

    startStyledBlock();
}

const TextStyle& ParagraphBuilderImpl::internalPeekStyle() {
    if (fTextStyles.empty()) {
        return fParagraphStyle.getTextStyle();
    } else {
        return fTextStyles.back();
    }
}

TextStyle ParagraphBuilderImpl::peekStyle() {
    return internalPeekStyle();
}

void ParagraphBuilderImpl::addText(const std::u16string& text) {
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text);
    fUtf8.append(utf8);
}

void ParagraphBuilderImpl::addText(const char* text) {
    fUtf8.append(text);
}

void ParagraphBuilderImpl::addText(const char* text, size_t len) {
    fUtf8.append(text, len);
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle) {
    addPlaceholder(placeholderStyle, false);
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle, bool lastOne) {
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

std::unique_ptr<Paragraph> ParagraphBuilderImpl::Build() {
    if (!fUtf8.isEmpty()) {
        this->endRunIfNeeded();
    }

    // Add one fake placeholder with the rest of the text
    addPlaceholder(PlaceholderStyle(), true);
    return std::make_unique<ParagraphImpl>(
            fUtf8, fParagraphStyle, fStyledBlocks, fPlaceholders, fFontCollection, fUnicode);
}


SkSpan<char> ParagraphBuilderImpl::getText() {
    return SkSpan<char>(fUtf8.isEmpty() ? nullptr : fUtf8.writable_str(), fUtf8.size());
}

const ParagraphStyle& ParagraphBuilderImpl::getParagraphStyle() const {
    return fParagraphStyle;
}

std::unique_ptr<Paragraph> ParagraphBuilderImpl::BuildWithClientInfo(
                std::vector<SkUnicode::BidiRegion> bidiRegions,
                std::vector<SkUnicode::Position> words,
                std::vector<SkUnicode::Position> graphemeBreaks,
                std::vector<SkUnicode::LineBreakBefore> lineBreaks) {
    // This is the place where SkUnicode is paired with SkParagraph
    fUnicode =
            SkUnicode::Make(SkSpan<char>(fUtf8.isEmpty() ? nullptr : &fUtf8[0], fUtf8.size()),
                            std::move(bidiRegions),
                            std::move(words),
                            std::move(graphemeBreaks),
                            std::move(lineBreaks));
    return this->Build();
}

void ParagraphBuilderImpl::Reset() {
    fTextStyles.reset();
    fUtf8.reset();
    fStyledBlocks.reset();
    fPlaceholders.reset();

    startStyledBlock();
}

}  // namespace textlayout
}  // namespace skia
