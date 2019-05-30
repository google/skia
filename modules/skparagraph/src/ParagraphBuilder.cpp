/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "ParagraphImpl.h"
#include "include/core/SkPaint.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

ParagraphBuilder::ParagraphBuilder(
        ParagraphStyle style, sk_sp<FontCollection> font_collection)
        : fFontCollection(std::move(font_collection)) {
    this->setParagraphStyle(style);
}

ParagraphBuilder::~ParagraphBuilder() = default;

void ParagraphBuilder::setParagraphStyle(const ParagraphStyle& style) {
    fParagraphStyle = style;
    fTextStyles.push(fParagraphStyle.getTextStyle());
    fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), fParagraphStyle.getTextStyle());
}

void ParagraphBuilder::pushStyle(const TextStyle& style) {
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

void ParagraphBuilder::pop() {
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

TextStyle ParagraphBuilder::peekStyle() {
    this->endRunIfNeeded();

    if (!fTextStyles.empty()) {
        return fTextStyles.top();
    } else {
        SkDebugf("SkParagraphBuilder._styles is empty.\n");
        return fParagraphStyle.getTextStyle();
    }
}

void ParagraphBuilder::addText(const std::u16string& text) {
    icu::UnicodeString unicode;
    unicode.setTo((UChar*)text.data());
    unicode.toUTF8String(fUtf8);
}

void ParagraphBuilder::addText(const std::string& text) {
    icu::UnicodeString unicode;
    unicode.setTo(text.data());
    unicode.toUTF8String(fUtf8);
}

void ParagraphBuilder::addText(const char* text) {
    icu::UnicodeString unicode;
    unicode.setTo(text);
    unicode.toUTF8String(fUtf8);
}

void ParagraphBuilder::endRunIfNeeded() {
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

std::unique_ptr<Paragraph> ParagraphBuilder::Build() {
    this->endRunIfNeeded();

    return std::make_unique<ParagraphImpl>(
            fUtf8, fParagraphStyle, fStyledBlocks, fFontCollection);
}

}  // namespace textlayout
}  // namespace skia
