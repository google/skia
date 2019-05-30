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
#ifndef ParagraphBuilder_DEFINED
#define ParagraphBuilder_DEFINED

#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include "FontCollection.h"
#include "Paragraph.h"
#include "ParagraphStyle.h"
#include "TextStyle.h"
#include "unicode/unistr.h"

namespace skia {
namespace textlayout {

class ParagraphBuilder {
public:
    ParagraphBuilder(ParagraphStyle style, sk_sp<FontCollection> fontCollection);

    ~ParagraphBuilder();

    // Push a style to the stack. The corresponding text added with AddText will
    // use the top-most style.
    void pushStyle(const TextStyle& style);

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

    TextStyle peekStyle();

    // Adds text to the builder. Forms the proper runs to use the upper-most style
    // on the style_stack_;
    void addText(const std::u16string& text);

    // Converts to u16string before adding.
    void addText(const std::string& text);

    // Converts to u16string before adding.
    void addText(const char* text);

    void setParagraphStyle(const ParagraphStyle& style);

    // Constructs a SkParagraph object that can be used to layout and paint the text to a SkCanvas.
    std::unique_ptr<Paragraph> Build();

private:
    void endRunIfNeeded();

    std::string fUtf8;
    std::stack<TextStyle> fTextStyles;
    std::vector<Paragraph::Block> fStyledBlocks;
    sk_sp<FontCollection> fFontCollection;
    ParagraphStyle fParagraphStyle;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphBuilder_DEFINED