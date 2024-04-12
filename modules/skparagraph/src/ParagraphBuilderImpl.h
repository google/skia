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

namespace skia {
namespace textlayout {

class ParagraphBuilderImpl : public ParagraphBuilder {
public:
    ParagraphBuilderImpl(const ParagraphStyle& style,
                         sk_sp<FontCollection> fontCollection,
                         sk_sp<SkUnicode> unicode);

#if !defined(SK_DISABLE_LEGACY_PARAGRAPH_UNICODE)
    // Just until we fix all the code; calls icu::make inside
    ParagraphBuilderImpl(const ParagraphStyle& style, sk_sp<FontCollection> fontCollection);
#endif

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
    SkSpan<char> getText() override;
    const ParagraphStyle& getParagraphStyle() const override;

#if !defined(SK_DISABLE_LEGACY_CLIENT_UNICODE) && defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    void setWordsUtf8(std::vector<SkUnicode::Position> wordsUtf8) override;
    void setWordsUtf16(std::vector<SkUnicode::Position> wordsUtf16) override;

    void setGraphemeBreaksUtf8(std::vector<SkUnicode::Position> graphemesUtf8) override;
    void setGraphemeBreaksUtf16(std::vector<SkUnicode::Position> graphemesUtf16) override;

    void setLineBreaksUtf8(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8) override;
    void setLineBreaksUtf16(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf16) override;

    std::tuple<std::vector<SkUnicode::Position>,
               std::vector<SkUnicode::Position>,
               std::vector<SkUnicode::LineBreakBefore>>
        getClientICUData() const override {
            return { fWordsUtf16, fGraphemeBreaksUtf8, fLineBreaksUtf8 };
    }

    void SetUnicode(sk_sp<SkUnicode> unicode) override {
        fUnicode = std::move(unicode);
    }
#endif
    // Support for Flutter optimization
    void Reset() override;

    static std::unique_ptr<ParagraphBuilder> make(const ParagraphStyle& style,
                                                  sk_sp<FontCollection> fontCollection,
                                                  sk_sp<SkUnicode> unicode);


#if !defined(SK_DISABLE_LEGACY_PARAGRAPH_UNICODE)
    // Just until we fix all the code; calls icu::make inside
    static std::unique_ptr<ParagraphBuilder> make(const ParagraphStyle& style,
                                                  sk_sp<FontCollection> fontCollection);
#endif

    static bool RequiresClientICU();
protected:
    void startStyledBlock();
    void endRunIfNeeded();
    const TextStyle& internalPeekStyle();
    void addPlaceholder(const PlaceholderStyle& placeholderStyle, bool lastOne);
    void finalize();

    SkString fUtf8;
    skia_private::STArray<4, TextStyle, true> fTextStyles;
    skia_private::STArray<4, Block, true> fStyledBlocks;
    skia_private::STArray<4, Placeholder, true> fPlaceholders;
    sk_sp<FontCollection> fFontCollection;
    ParagraphStyle fParagraphStyle;

    sk_sp<SkUnicode> fUnicode;
private:
    SkOnce fillUTF16MappingOnce;
    void ensureUTF16Mapping();
    skia_private::TArray<TextIndex, true> fUTF8IndexForUTF16Index;
    skia_private::TArray<TextIndex, true> fUTF16IndexForUTF8Index;
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
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
