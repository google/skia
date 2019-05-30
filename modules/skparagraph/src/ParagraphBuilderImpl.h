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
#ifndef ParagraphBuilderImpl_DEFINED
#define ParagraphBuilderImpl_DEFINED

#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include "FontCollection.h"
#include "Paragraph.h"
#include "ParagraphBuilder.h"
#include "ParagraphStyle.h"
#include "TextStyle.h"

namespace skia {
namespace textlayout {

class ParagraphBuilderImpl : public ParagraphBuilder {
public:
    ParagraphBuilderImpl(ParagraphStyle style, sk_sp<FontCollection> fontCollection);

    ~ParagraphBuilderImpl() override;

    // Push a style to the stack. The corresponding text added with AddText will
    // use the top-most style.
    void pushStyle(const TextStyle& style) override;

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
    void pop() override;

    TextStyle peekStyle() override;

    // Adds text to the builder. Forms the proper runs to use the upper-most style
    // on the style_stack_;
    void addText(const std::u16string& text) override;

    // Converts to u16string before adding.
    void addText(const char* text) override;

    void setParagraphStyle(const ParagraphStyle& style) override;

    // Constructs a SkParagraph object that can be used to layout and paint the text to a SkCanvas.
    std::unique_ptr<Paragraph> Build() override;

private:
    void endRunIfNeeded();

    SkString fUtf8;
    std::stack<TextStyle> fTextStyles;
    std::vector<Block> fStyledBlocks;
    sk_sp<FontCollection> fFontCollection;
    ParagraphStyle fParagraphStyle;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphBuilderImpl_DEFINED