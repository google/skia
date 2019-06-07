// Copyright 2019 Google LLC.
#include "ParagraphBuilderImpl.h"
#include "ParagraphImpl.h"
#include "include/core/SkPaint.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "src/core/SkSpan.h"
#include "src/core/SkMakeUnique.h"
#include "unicode/unistr.h"

namespace skia {
namespace textlayout {

std::shared_ptr<ParagraphBuilder> ParagraphBuilder::make(
        ParagraphStyle style, sk_sp<FontCollection> fontCollection) {
    return std::make_shared<ParagraphBuilderImpl>(style, fontCollection);
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        ParagraphStyle style, sk_sp<FontCollection> fontCollection)
        : ParagraphBuilder(style, fontCollection), fFontCollection(std::move(fontCollection)) {
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
    if (!fStyledBlocks.empty() && fStyledBlocks.back().fEnd == fUtf8.size() &&
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
    //SkDebugf("Layout text16: '%s'\n", str.c_str());
    fUtf8.insert(fUtf8.size(), str.c_str());
}

void ParagraphBuilderImpl::addText(const char* text) {
    //SkDebugf("Layout text8: '%s'\n", text);
    fUtf8.insert(fUtf8.size(), text);
}

void ParagraphBuilderImpl::endRunIfNeeded() {
    if (fStyledBlocks.empty()) {
        return;
    }

    auto& last = fStyledBlocks.back();
    if (last.fStart == fUtf8.size()) {
        fStyledBlocks.pop_back();
    } else {
        last.fEnd = fUtf8.size();
    }
}

std::unique_ptr<Paragraph> ParagraphBuilderImpl::Build() {
    if (!fUtf8.isEmpty()) {
        this->endRunIfNeeded();
    }
    return skstd::make_unique<ParagraphImpl>(fUtf8, fParagraphStyle, fStyledBlocks, fFontCollection);
}

}  // namespace textlayout
}  // namespace skia
