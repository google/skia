// Copyright 2019 Google LLC.
#include "include/core/SkPaint.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkSpan.h"
#include "unicode/unistr.h"

namespace skia {
namespace textlayout {

std::unique_ptr<ParagraphBuilder> ParagraphBuilder::make(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection) {
    return skstd::make_unique<ParagraphBuilderImpl>(style, fontCollection);
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection)
        : ParagraphBuilder(style, fontCollection), fUtf8(), fFontCollection(std::move(fontCollection)) {
    this->setParagraphStyle(style);
}

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
        SkDebugf("SkParagraphBuilder.Pop() called too many times.\n");
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
    icu::UnicodeString unicode;
    unicode.setTo((UChar*)text.data());
    std::string str;
    unicode.toUTF8String(str);
    fUtf8.insert(fUtf8.size(), str.c_str());
}

void ParagraphBuilderImpl::addText(const char* text) {
    fUtf8.insert(fUtf8.size(), text);
}

void ParagraphBuilderImpl::addText(const char* text, size_t len) {
    fUtf8.insert(fUtf8.size(), text, len);
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle) {
    addPlaceholder(placeholderStyle, false);
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle, bool lastOne) {
    this->endRunIfNeeded();

    BlockRange stylesBefore(fPlaceholders.empty() ? 0 : fPlaceholders.back().fBlocksBefore.end + 1,
                            fStyledBlocks.size());
    TextRange textBefore(fPlaceholders.empty() ? 0 : fPlaceholders.back().fRange.end,
                            fUtf8.size());
    auto start = fUtf8.size();
    if (!lastOne) {
        pushStyle(TextStyle(fTextStyles.top(), true));
        addText(std::u16string(1ull, 0xFFFC));
        pop();
    }
    auto end = fUtf8.size();
    fPlaceholders.emplace_back(start, end, placeholderStyle, stylesBefore, textBefore);
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
    return skstd::make_unique<ParagraphImpl>(
            fUtf8, fParagraphStyle, fStyledBlocks, fPlaceholders, fFontCollection);
}

}  // namespace textlayout
}  // namespace skia
