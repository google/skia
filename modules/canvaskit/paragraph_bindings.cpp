/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"
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

struct SimpleFontStyle {
    SkFontStyle::Slant  slant;
    SkFontStyle::Weight weight;
    SkFontStyle::Width  width;
};

struct SimpleTextStyle {
    SkColor backgroundColor;
    SkColor color;
    uint8_t decoration;
    SkScalar decorationThickness;
    SkScalar fontSize;
    SimpleFontStyle fontStyle;
    SkColor foregroundColor;

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

    SkFontStyle fs(s.fontStyle.weight, s.fontStyle.width, s.fontStyle.slant);
    ts.setFontStyle(fs);

    return ts;
}

struct SimpleParagraphStyle {
    bool disableHinting;
    uintptr_t /* const char* */ ellipsisPtr;
    size_t ellipsisLen;
    SkScalar heightMultiplier;
    size_t maxLines;
    para::TextAlign textAlign;
    para::TextDirection textDirection;
    SimpleTextStyle textStyle;
};

para::ParagraphStyle toParagraphStyle(const SimpleParagraphStyle& s) {
    para::ParagraphStyle ps;
    if (s.disableHinting) {
        ps.turnHintingOff();
    }

    if (s.ellipsisLen > 0) {
        const char* ellipsisPtr = reinterpret_cast<const char*>(s.ellipsisPtr);
        SkString eStr(ellipsisPtr, s.ellipsisLen);
        ps.setEllipsis(eStr);
    }
    ps.setTextAlign(s.textAlign);
    ps.setTextDirection(s.textDirection);
    auto ts = toTextStyle(s.textStyle);
    ps.setTextStyle(ts);
    if (s.heightMultiplier != 0) {
        ps.setHeight(s.heightMultiplier);
    }
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

    enum_<SkFontStyle::Slant>("FontSlant")
        .value("Upright",              SkFontStyle::Slant::kUpright_Slant)
        .value("Italic",               SkFontStyle::Slant::kItalic_Slant)
        .value("Oblique",              SkFontStyle::Slant::kOblique_Slant);

    enum_<SkFontStyle::Weight>("FontWeight")
        .value("Invisible",            SkFontStyle::Weight::kInvisible_Weight)
        .value("Thin",                 SkFontStyle::Weight::kThin_Weight)
        .value("ExtraLight",           SkFontStyle::Weight::kExtraLight_Weight)
        .value("Light",                SkFontStyle::Weight::kLight_Weight)
        .value("Normal",               SkFontStyle::Weight::kNormal_Weight)
        .value("Medium",               SkFontStyle::Weight::kMedium_Weight)
        .value("SemiBold",             SkFontStyle::Weight::kSemiBold_Weight)
        .value("Bold",                 SkFontStyle::Weight::kBold_Weight)
        .value("ExtraBold",            SkFontStyle::Weight::kExtraBold_Weight)
        .value("Black"    ,            SkFontStyle::Weight::kBlack_Weight)
        .value("ExtraBlack",           SkFontStyle::Weight::kExtraBlack_Weight);

    enum_<SkFontStyle::Width>("FontWidth")
        .value("UltraCondensed",       SkFontStyle::Width::kUltraCondensed_Width)
        .value("ExtraCondensed",       SkFontStyle::Width::kExtraCondensed_Width)
        .value("Condensed",            SkFontStyle::Width::kCondensed_Width)
        .value("SemiCondensed",        SkFontStyle::Width::kSemiCondensed_Width)
        .value("Normal",               SkFontStyle::Width::kNormal_Width)
        .value("SemiExpanded",         SkFontStyle::Width::kSemiExpanded_Width)
        .value("Expanded",             SkFontStyle::Width::kExpanded_Width)
        .value("ExtraExpanded",        SkFontStyle::Width::kExtraExpanded_Width)
        .value("UltraExpanded",        SkFontStyle::Width::kUltraExpanded_Width);

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

    enum_<para::TextDirection>("TextDirection")
        .value("LTR",    para::TextDirection::kLtr)
        .value("RTL",    para::TextDirection::kRtl);


    value_object<para::PositionWithAffinity>("PositionWithAffinity")
        .field("pos",      &para::PositionWithAffinity::position)
        .field("affinity", &para::PositionWithAffinity::affinity);

 value_object<SimpleFontStyle>("FontStyle")
        .field("slant",     &SimpleFontStyle::slant)
        .field("weight",    &SimpleFontStyle::weight)
        .field("width",     &SimpleFontStyle::width);

    value_object<SimpleParagraphStyle>("ParagraphStyle")
        .field("disableHinting",    &SimpleParagraphStyle::disableHinting)
        .field("_ellipsisPtr",      &SimpleParagraphStyle::ellipsisPtr)
        .field("_ellipsisLen",      &SimpleParagraphStyle::ellipsisLen)
        .field("heightMultiplier",  &SimpleParagraphStyle::heightMultiplier)
        .field("maxLines",          &SimpleParagraphStyle::maxLines)
        .field("textAlign",         &SimpleParagraphStyle::textAlign)
        .field("textDirection",     &SimpleParagraphStyle::textDirection)
        .field("textStyle",         &SimpleParagraphStyle::textStyle);

    value_object<SimpleTextStyle>("TextStyle")
        .field("backgroundColor",     &SimpleTextStyle::backgroundColor)
        .field("color",               &SimpleTextStyle::color)
        .field("decoration",          &SimpleTextStyle::decoration)
        .field("decorationThickness", &SimpleTextStyle::decorationThickness)
        .field("_fontFamilies",       &SimpleTextStyle::fontFamilies)
        .field("fontSize",            &SimpleTextStyle::fontSize)
        .field("fontStyle",           &SimpleTextStyle::fontStyle)
        .field("foregroundColor",     &SimpleTextStyle::foregroundColor)
        .field("_numFontFamilies",    &SimpleTextStyle::numFontFamilies);

    // TextDecoration should be a const because they can be combined
    constant("NoDecoration", int(para::TextDecoration::kNoDecoration));
    constant("UnderlineDecoration", int(para::TextDecoration::kUnderline));
    constant("OverlineDecoration", int(para::TextDecoration::kOverline));
    constant("LineThroughDecoration", int(para::TextDecoration::kLineThrough));
}
