/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if defined(SK_ENABLE_PARAGRAPH)

using namespace skia::textlayout;
namespace {
const uint8_t MAX_TEXT_LENGTH = 255;
const uint8_t MAX_TEXT_ADDITIONS = 4;
// Use 250 so uint8 can create text and layout width larger than the canvas.
const uint16_t TEST_CANVAS_DIM = 250;

class ResourceFontCollection : public FontCollection {
public:
    ResourceFontCollection(bool testOnly = false)
            : fFontsFound(false)
            , fResolvedFonts(0)
            , fResourceDir(GetResourcePath("fonts").c_str())
            , fFontProvider(sk_make_sp<TypefaceFontProvider>()) {
        std::vector<SkString> fonts;
        SkOSFile::Iter iter(fResourceDir.c_str());

        SkString path;
        sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
        while (iter.next(&path)) {
            if (path.endsWith("Roboto-Italic.ttf")) {
                fFontsFound = true;
            }
            fonts.emplace_back(path);
        }

        if (!fFontsFound) {
            // SkDebugf("Fonts not found, skipping all the tests\n");
            return;
        }
        // Only register fonts if we have to
        for (auto& font : fonts) {
            SkString file_path;
            file_path.printf("%s/%s", fResourceDir.c_str(), font.c_str());
            fFontProvider->registerTypeface(mgr->makeFromFile(file_path.c_str(), 0));
        }

        if (testOnly) {
            this->setTestFontManager(std::move(fFontProvider));
        } else {
            this->setAssetFontManager(std::move(fFontProvider));
        }
        this->disableFontFallback();
    }

    size_t resolvedFonts() const { return fResolvedFonts; }

    // TODO: temp solution until we check in fonts
    bool fontsFound() const { return fFontsFound; }

private:
    bool fFontsFound;
    size_t fResolvedFonts;
    std::string fResourceDir;
    sk_sp<TypefaceFontProvider> fFontProvider;
};

// buffer must be at least MAX_TEXT_LENGTH in length.
// Returns size of text placed in buffer.
template <typename T>
uint8_t RandomText(T* buffer, Fuzz* fuzz) {
    uint8_t text_length;
    fuzz->nextRange(&text_length, 0, MAX_TEXT_LENGTH);
    fuzz->nextN(buffer, text_length);
    return text_length;
}

// Add random bytes to the paragraph.
void AddASCIIText(ParagraphBuilder* builder,Fuzz* fuzz) {
    char text[MAX_TEXT_LENGTH];
    const auto text_length = RandomText(text, fuzz);
    builder->addText(text, text_length);
}
// Add random bytes to the paragraph.
void AddUnicodeText(ParagraphBuilder* builder,Fuzz* fuzz) {
    char16_t text[MAX_TEXT_LENGTH];
    const auto text_length = RandomText(text, fuzz);
    builder->addText(std::u16string(text, text_length));
}

// Combining characters to produce 'Zalgo' text.
const std::u16string COMBINING_DOWN = u"\u0316\u0317\u0318\u0319\u031c\u031d\u031e\u031f\u0320\u0324\u0325\u0326\u0329\u032a\u032b\u032c\u032d\u032e\u032f\u0330\u0331\u0332\u0333\u0339\u033a\u033b\u033c\u0345\u0347\u0348\u0349\u034d\u034e\u0353\u0354\u0355\u0356\u0359\u035a\u0323";
const std::u16string COMBINING_UP = u"\u030d\u030e\u0304\u0305\u033f\u0311\u0306\u0310\u0352\u0357\u0351\u0307\u0308\u030a\u0342\u0343\u0344\u034a\u034b\u034c\u0303\u0302\u030c\u0350\u0300\u0301\u030b\u030f\u0312\u0313\u0314\u033d\u0309\u0363\u0364\u0365\u0366\u0367\u0368\u0369\u036a\u036b\u036c\u036d\u036e\u035b\u0346\u031a";
const std::u16string COMBINING_MIDDLE = u"\u0315\u031b\u0340\u0341\u0358\u0321\u0322\u0327\u0328\u0334\u0335\u0336\u034f\u035c\u035d\u035e\u035f\u0360\u0362\u0338\u0337\u0361\u0489";
// Add random Zalgo text to the paragraph.
void AddZalgoText(ParagraphBuilder* builder, Fuzz* fuzz) {
    char text[MAX_TEXT_LENGTH];
    const auto text_length = RandomText(text, fuzz);
    std::u16string result;

    for (auto& c : std::string(text, text_length)) {
        result += c;
        uint8_t mark_count;
        fuzz->next(&mark_count);
        for (int i = 0; i < mark_count; i++) {
            uint8_t mark_type, mark_index;
            fuzz->next(&mark_type, &mark_index);
            switch (mark_type % 3) {
                case 0:
                    result += COMBINING_UP[mark_index % COMBINING_UP.size()];
                    break;
                case 1:
                    result += COMBINING_MIDDLE[mark_index % COMBINING_MIDDLE.size()];
                    break;
                case 2:
                default:
                    result += COMBINING_DOWN[mark_index % COMBINING_DOWN.size()];
                    break;
            }
        }
    }
    builder->addText(result);
}

void AddStyle(ParagraphBuilder* builder, Fuzz*  fuzz) {
    // TODO(westont): Make this probabilistic, and fill in the remaining TextStyle fields.
    TextStyle ts;
    ts.setFontFamilies({SkString("Roboto")});
    //ts.setColor(SK_ColorBLACK);
    //ts.setForegroundColor
    //ts.setBackgroundColor
    //ts.setDecoration(TextDecoration decoration);
    //ts.setDecorationMode(TextDecorationMode mode);
    //ts.setDecorationStyle(TextDecorationStyle style);
    //ts.setDecorationColor(SkColor color);
    //ts.setDecorationThicknessMultiplier(SkScalar m);
    //ts.setFontStyle
    //ts.addShadow
    //ts.addFontFeature
    //ts.setFontSize
    //ts.setHeight
    //ts.setHeightOverride
    //ts.setletterSpacing
    //ts.setWordSpacing
    //ts.setTypeface
    //ts.setLocale
    //ts.setTextBaseline
    //ts.setPlaceholder

    builder->pushStyle(ts);
}
void RemoveStyle(ParagraphBuilder* builder, Fuzz*  fuzz) {
    bool pop;
    fuzz->next(&pop);
    if (pop) {
        builder->pop();
    }
}

void AddStyleAndText(ParagraphBuilder* builder, Fuzz*  fuzz) {
    AddStyle(builder, fuzz);
    uint8_t text_type;
    fuzz->next(&text_type);
    switch (text_type % 3) {
        case 0:
            AddASCIIText(builder, fuzz);
            break;
        case 1:
            AddUnicodeText(builder, fuzz);
            break;
        case 2:
            AddZalgoText(builder, fuzz);
            break;
    }
    RemoveStyle(builder, fuzz);

}

ParagraphStyle BuildParagraphStyle(Fuzz* fuzz) {
    ParagraphStyle ps;
    bool hinting;
    fuzz->next(&hinting);
    if (hinting) {
        ps.turnHintingOff();
    }
    StrutStyle ss;
    // TODO(westont): Fuzz this object.
    ps.setStrutStyle(ss);
    TextDirection td;
    fuzz->nextEnum(&td, TextDirection::kRtl);
    ps.setTextDirection(td);
    TextAlign ta;
    fuzz->nextEnum(&ta, TextAlign::kEnd);
    ps.setTextAlign(ta);
    size_t ml;
    fuzz->next(&ml);
    ps.setMaxLines(ml);
    // TODO(westont): Randomize with other values and no value.
    ps.setEllipsis(u"\u2026");
    SkScalar h;
    fuzz->next(&h);
    ps.setHeight(h);
    TextHeightBehavior thb = TextHeightBehavior::kAll;
    // TODO(westont): This crashes our seed test case, why?
    //fuzz->nextEnum(&thb, TextHeightBehavior::kDisableAll);
    ps.setTextHeightBehavior(thb);

    return ps;
}

}  // namespace

DEF_FUZZ(SkParagraph, fuzz) {
    static sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    ParagraphStyle paragraph_style = BuildParagraphStyle(fuzz);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    uint8_t iterations;
    fuzz->nextRange(&iterations, 1, MAX_TEXT_ADDITIONS);
    for (int i = 0; i < iterations; i++) {
        AddStyleAndText(&builder, fuzz);
    }
    // TODO(westont): Figure out if we can get more converage by having fontsFound, current
    // they're not.
    // if (!fontCollection->fontsFound()) return;

    builder.pop();
    auto paragraph = builder.Build();

    SkBitmap bm;
    if (!bm.tryAllocN32Pixels(TEST_CANVAS_DIM, TEST_CANVAS_DIM)) {
        return;
    }
    SkCanvas canvas(bm);
    uint8_t layout_width;
    fuzz->next(&layout_width);
    paragraph->layout(layout_width);
    paragraph->paint(&canvas, 0, 0);
}

#endif // SK_ENABLE_PARAGRAPH
