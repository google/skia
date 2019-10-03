/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkString.h"

#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>
#include "modules/canvaskit/WasmAliases.h"

using namespace emscripten;

namespace para = skia::textlayout;

struct SimpleTextStyle {
    SkColor color;
    SkColor foregroundColor;
    SkColor backgroundColor;
    uint8_t decoration;
    SkScalar fontSize;
    SkScalar decorationThickness;
    uintptr_t /* const char** */ fontFamilies;
    int numFontFamilies;
};

para::TextStyle toTextStyle(const SimpleTextStyle& s) {
    para::TextStyle ts;
    if (s.color != 0) {
        ts.setColor(s.color);
    }

    if (s.foregroundColor != 0) {
        SkPaint p;
        p.setColor(s.foregroundColor);
        ts.setForegroundColor(p);
    }

    if (s.backgroundColor != 0) {
        SkPaint p;
        p.setColor(s.backgroundColor);
        ts.setBackgroundColor(p);
    }

    if (s.fontSize != 0) {
        ts.setFontSize(s.fontSize);
    }

    ts.setDecoration(para::TextDecoration(s.decoration));
    if (s.decorationThickness != 0) {
        ts.setDecorationThicknessMultiplier(s.decorationThickness);
    }

    const char** fontFamilies = reinterpret_cast<const char**>(s.fontFamilies);
    if (s.numFontFamilies > 0 && fontFamilies != nullptr) {
        std::vector<SkString> ff;
        for (int i = 0; i< s.numFontFamilies; i++) {
            ff.emplace_back(fontFamilies[i]);
        }
        ts.setFontFamilies(ff);
    }

    return ts;
}

struct SimpleParagraphStyle {
    SimpleTextStyle textStyle;
    SkScalar heightMultiplier;
    para::TextAlign textAlign;
    size_t maxLines;
};

para::ParagraphStyle toParagraphStyle(const SimpleParagraphStyle& s) {
    para::ParagraphStyle ps;
    auto ts = toTextStyle(s.textStyle);
    ps.setTextStyle(ts);
    if (s.heightMultiplier != 0) {
        ps.setHeight(s.heightMultiplier);
    }
    ps.setTextAlign(s.textAlign);
    if (s.maxLines != 0) {
        ps.setMaxLines(s.maxLines);
    }
    return ps;
}

Float32Array GetRectsForRange(para::ParagraphImpl& self, unsigned start, unsigned end,
                            para::RectHeightStyle heightStyle, para::RectWidthStyle widthStyle) {
    std::vector<para::TextBox> boxes = self.getRectsForRange(start, end, heightStyle, widthStyle);
    // Pack these text boxes into an array of n groups of 4 SkScalar (floats)
    if (!boxes.size()) {
        return emscripten::val::null();
    }
    SkRect* rects = new SkRect[boxes.size()];
    for (int i = 0; i< boxes.size(); i++) {
        rects[i] = boxes[i].rect;
    }
    float* fPtr = reinterpret_cast<float*>(rects);
    // Of note: now that we have cast rects to float*, emscripten is smart enough to wrap this
    // into a Float32Array for us.
    return Float32Array(typed_memory_view(boxes.size()*4, fPtr));
}

EMSCRIPTEN_BINDINGS(Paragraph) {

    class_<para::Paragraph>("Paragraph");

    // This "base<>" tells Emscripten that ParagraphImpl is a Paragraph and can get substituted
    // in properly in drawParagraph. However, Emscripten will not let us bind pure virtual methods
    // so we have to "expose" the ParagraphImpl and its methods.
    class_<para::ParagraphImpl, base<para::Paragraph>>("ParagraphImpl")
        .function("_getRectsForRange", &GetRectsForRange)
        .function("getGlyphPositionAtCoordinate", &para::ParagraphImpl::getGlyphPositionAtCoordinate)
        .function("layout", &para::ParagraphImpl::layout);

    class_<para::ParagraphBuilderImpl>("ParagraphBuilder")
        .class_function("Make", optional_override([](SimpleParagraphStyle style,
                                                     sk_sp<SkFontMgr> fontMgr)-> para::ParagraphBuilderImpl {
            auto fc = sk_make_sp<para::FontCollection>();
            fc->setDefaultFontManager(fontMgr);
            auto ps = toParagraphStyle(style);
            para::ParagraphBuilderImpl pbi(ps, fc);
            return pbi;
        }), allow_raw_pointers())
        .function("addText", optional_override([](para::ParagraphBuilderImpl& self, std::string text) {
            return self.addText(text.c_str(), text.length());
        }))
        .function("build", &para::ParagraphBuilderImpl::Build, allow_raw_pointers())
        .function("pop", &para::ParagraphBuilderImpl::pop)
        .function("pushStyle",  optional_override([](para::ParagraphBuilderImpl& self,
                                                     SimpleTextStyle textStyle) {
            auto ts = toTextStyle(textStyle);
            self.pushStyle(ts);
        }));


    enum_<para::Affinity>("Affinity")
        .value("Upstream",   para::Affinity::kUpstream)
        .value("Downstream", para::Affinity::kDownstream);

    enum_<para::RectHeightStyle>("RectHeightStyle")
        .value("Tight",  para::RectHeightStyle::kTight)
        .value("Max",    para::RectHeightStyle::kMax);

    enum_<para::RectWidthStyle>("RectWidthStyle")
        .value("Tight",  para::RectWidthStyle::kTight)
        .value("Max",    para::RectWidthStyle::kMax);

    enum_<para::TextAlign>("TextAlign")
        .value("Left",    para::TextAlign::kLeft)
        .value("Right",   para::TextAlign::kRight)
        .value("Center",  para::TextAlign::kCenter)
        .value("Justify", para::TextAlign::kJustify)
        .value("Start",   para::TextAlign::kStart)
        .value("End",     para::TextAlign::kEnd);


    value_object<para::PositionWithAffinity>("PositionWithAffinity")
        .field("pos",      &para::PositionWithAffinity::position)
        .field("affinity", &para::PositionWithAffinity::affinity);

    value_object<SimpleParagraphStyle>("ParagraphStyle")
        .field("heightMultiplier",  &SimpleParagraphStyle::heightMultiplier)
        .field("maxLines",          &SimpleParagraphStyle::maxLines)
        .field("textAlign",         &SimpleParagraphStyle::textAlign)
        .field("textStyle",         &SimpleParagraphStyle::textStyle);

    value_object<SimpleTextStyle>("TextStyle")
        .field("backgroundColor",     &SimpleTextStyle::backgroundColor)
        .field("color",               &SimpleTextStyle::color)
        .field("decoration",          &SimpleTextStyle::decoration)
        .field("decorationThickness", &SimpleTextStyle::decorationThickness)
        .field("_fontFamilies",       &SimpleTextStyle::fontFamilies)
        .field("fontSize",            &SimpleTextStyle::fontSize)
        .field("foregroundColor",     &SimpleTextStyle::foregroundColor)
        .field("_numFontFamilies",    &SimpleTextStyle::numFontFamilies);

    // TextDecoration should be a const because they can be combined
    constant("NoDecoration", int(para::TextDecoration::kNoDecoration));
    constant("UnderlineDecoration", int(para::TextDecoration::kUnderline));
    constant("OverlineDecoration", int(para::TextDecoration::kOverline));
    constant("LineThroughDecoration", int(para::TextDecoration::kLineThrough));
}
