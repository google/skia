// Copyright 2019 Google LLC.
#ifndef ParagraphBuilderImpl_DEFINED
#define ParagraphBuilderImpl_DEFINED

#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skunicode/include/SkUnicode.h"

namespace skia {
namespace textlayout {

class ParagraphBuilderImpl : public ParagraphBuilder {
public:
    ParagraphBuilderImpl(const ParagraphStyle& style,
        sk_sp<FontCollection> fontCollection,
        std::unique_ptr<SkUnicode> unicode);

    // Just until we fix all the code; calls icu::make inside
    ParagraphBuilderImpl(const ParagraphStyle& style, sk_sp<FontCollection> fontCollection);

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
    // on the style_stack.
    void addText(const std::u16string& text) override;

    // Adds text to the builder, using the top-most style on on the style_stack.
    void addText(const char* text) override; // Don't use this one - going away soon
    void addText(const char* text, size_t len) override;

    void addPlaceholder(const PlaceholderStyle& placeholderStyle) override;

    // Constructs a SkParagraph object that can be used to layout and paint the text to a SkCanvas.
    std::unique_ptr<Paragraph> Build() override;

    // Support for "Client" unicode
    SkSpan<char> getText();
    const ParagraphStyle& getParagraphStyle() const;

    void setWordsUtf8(std::vector<SkUnicode::Position> wordsUtf8);
    void setWordsUtf16(std::vector<SkUnicode::Position> wordsUtf16);

    void setGraphemeBreaksUtf8(std::vector<SkUnicode::Position> graphemesUtf8);
    void setGraphemeBreaksUtf16(std::vector<SkUnicode::Position> graphemesUtf16);

    void setLineBreaksUtf8(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8);
    void setLineBreaksUtf16(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf16);

    void SetUnicode(std::unique_ptr<SkUnicode> unicode) {
        fUnicode = std::move(unicode);
    }
    // Support for Flutter optimization
    void Reset() override;

    static std::unique_ptr<ParagraphBuilder> make(const ParagraphStyle& style,
                                                  sk_sp<FontCollection> fontCollection,
                                                  std::unique_ptr<SkUnicode> unicode);

    // Just until we fix all the code; calls icu::make inside
    static std::unique_ptr<ParagraphBuilder> make(const ParagraphStyle& style,
                                                  sk_sp<FontCollection> fontCollection);

    static bool RequiresClientICU();
protected:
    void startStyledBlock();
    void endRunIfNeeded();
    const TextStyle& internalPeekStyle();
    void addPlaceholder(const PlaceholderStyle& placeholderStyle, bool lastOne);
    void finalize();

    SkString fUtf8;
    SkSTArray<4, TextStyle, true> fTextStyles;
    SkSTArray<4, Block, true> fStyledBlocks;
    SkSTArray<4, Placeholder, true> fPlaceholders;
    sk_sp<FontCollection> fFontCollection;
    ParagraphStyle fParagraphStyle;

    std::shared_ptr<SkUnicode> fUnicode;
private:
    SkOnce fillUTF16MappingOnce;
    void ensureUTF16Mapping();
    SkTArray<TextIndex, true> fUTF8IndexForUTF16Index;
    SkTArray<TextIndex, true> fUTF16IndexForUTF8Index;
#if !defined(SK_UNICODE_ICU_IMPLEMENTATION) && defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    bool fTextIsFinalized;
    bool fUsingClientInfo;
    std::vector<SkUnicode::Position> fWordsUtf16;
    std::vector<SkUnicode::Position> fGraphemeBreaksUtf8;
    std::vector<SkUnicode::LineBreakBefore> fLineBreaksUtf8;
#endif
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphBuilderImpl_DEFINED
