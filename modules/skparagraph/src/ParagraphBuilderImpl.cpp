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
        , fUnicode(std::move(unicode)) {
    SkASSERT(fUnicode);
    this->setParagraphStyle(style);
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection)
        : ParagraphBuilderImpl(style, fontCollection, SkUnicode::Make())
{ }

ParagraphBuilderImpl::~ParagraphBuilderImpl() = default;

void ParagraphBuilderImpl::setParagraphStyle(const ParagraphStyle& style) {
    fParagraphStyle = style;
    fTextStyles.push(fParagraphStyle.getTextStyle());
    fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), fParagraphStyle.getTextStyle());
}

void ParagraphBuilderImpl::pushStyle(const TextStyle& style) {
    this->endRunIfNeeded();

    fTextStyles.push(style);
    if (!fStyledBlocks.empty() && fStyledBlocks.back().fRange.end == fUtf8.size() &&
        fStyledBlocks.back().fStyle == style) {
        // Just continue with the same style
    } else {
        // Go with the new style
        fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), fTextStyles.top());
    }
}

void ParagraphBuilderImpl::pop() {
    this->endRunIfNeeded();

    if (fTextStyles.size() > 1) {
        fTextStyles.pop();
    } else {
        // In this case we use paragraph style and skip Pop operation
        SkDEBUGF("SkParagraphBuilder.Pop() called too many times.\n");
    }

    auto top = fTextStyles.top();
    fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), top);
}

TextStyle ParagraphBuilderImpl::peekStyle() {
    this->endRunIfNeeded();

    if (!fTextStyles.empty()) {
        return fTextStyles.top();
    } else {
        SkDebugf("SkParagraphBuilder._styles is empty.\n");
        return fParagraphStyle.getTextStyle();
    }
}

void ParagraphBuilderImpl::addText(const std::u16string& text) {
    auto utf8 = fUnicode->convertUtf16ToUtf8(text);
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
    auto topStyle = fTextStyles.top();
    if (!lastOne) {
        pushStyle(TextStyle(topStyle, true));
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

std::unique_ptr<Paragraph> ParagraphBuilderImpl::Build() {
    if (!fUtf8.isEmpty()) {
        this->endRunIfNeeded();
    }

    // Add one fake placeholder with the rest of the text
    addPlaceholder(PlaceholderStyle(), true);
    return std::make_unique<ParagraphImpl>(
            fUtf8, fParagraphStyle, fStyledBlocks, fPlaceholders, fFontCollection, std::move(fUnicode));
}

}  // namespace textlayout
}  // namespace skia
