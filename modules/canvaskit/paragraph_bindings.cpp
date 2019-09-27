/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkString.h"

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
    SkScalar height;
};

para::ParagraphStyle toParagraphStyle(const SimpleParagraphStyle& s) {
    para::ParagraphStyle ps;
    auto ts = toTextStyle(s.textStyle);
    ps.setTextStyle(ts);
    if (s.height != 0) {
        ps.setHeight(s.height);
    }
    return ps;
}

EMSCRIPTEN_BINDINGS(Paragraph) {

    class_<para::ParagraphImpl>("ParagraphImpl")
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
            return self.addText(text.c_str());
        }))
        .function("build", &para::ParagraphBuilderImpl::Build, allow_raw_pointers())
        .function("pop", &para::ParagraphBuilderImpl::pop)
        .function("pushStyle",  optional_override([](para::ParagraphBuilderImpl& self,
                                                     SimpleTextStyle textStyle) {
            auto ts = toTextStyle(textStyle);
            self.pushStyle(ts);
        }));

    value_object<SimpleParagraphStyle>("ParagraphStyle")
        .field("height",    &SimpleParagraphStyle::height)
        .field("textStyle", &SimpleParagraphStyle::textStyle);

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
    constant("NoDecoration", para::TextDecoration::kNoDecoration);
    constant("UnderlineDecoration", para::TextDecoration::kUnderline);
    constant("OverlineDecoration", para::TextDecoration::kOverline);
    constant("LineThroughDecoration", para::TextDecoration::kLineThrough);
}
