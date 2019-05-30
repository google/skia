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
#pragma once

#include <memory>
#include <stack>
#include <string>
#include <tuple>

#include "unicode/unistr.h"

#include "SkFontCollection.h"
#include "SkParagraph.h"
#include "SkParagraphStyle.h"
#include "SkTextStyle.h"

class SkParagraphBuilder {
public:
    SkParagraphBuilder(SkParagraphStyle style, sk_sp<SkFontCollection> fontCollection);

    ~SkParagraphBuilder();

    // Push a style to the stack. The corresponding text added with AddText will
    // use the top-most style.
    void pushStyle(const SkTextStyle& style);

    // Remove a style from the stack. Useful to apply different styles to chunks
    // of text such as bolding.
    // Example:
    //   builder.PushStyle(normal_style);
    //   builder.AddText("Hello this is normal. ");
    //
    //   builder.PushStyle(bold_style);
    //   builder.AddText("And this is BOLD. ");
    //
    //   builder.Pop();
    //   builder.AddText(" Back to normal again.");
    void pop();

    SkTextStyle peekStyle();

    // Adds text to the builder. Forms the proper runs to use the upper-most style
    // on the style_stack_;
    void addText(const std::u16string& text);

    // Converts to u16string before adding.
    void addText(const std::string& text);

    // Converts to u16string before adding.
    void addText(const char* text);

    void setParagraphStyle(const SkParagraphStyle& style);

    // Constructs a SkParagraph object that can be used to layout and paint the text to a SkCanvas.
    std::unique_ptr<SkParagraph> Build();

private:
    void endRunIfNeeded();

    std::string fUtf8;
    std::stack<SkTextStyle> fTextStyles;
    std::vector<SkParagraph::Block> fStyledBlocks;
    sk_sp<SkFontCollection> fFontCollection;
    SkParagraphStyle fParagraphStyle;
};
