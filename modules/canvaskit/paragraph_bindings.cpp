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
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>
#include "modules/canvaskit/WasmCommon.h"

using namespace emscripten;

namespace para = skia::textlayout;

SkColor4f toSkColor4f(uintptr_t /* float* */ cPtr) {
    float* fourFloats = reinterpret_cast<float*>(cPtr);
    SkColor4f color = { fourFloats[0], fourFloats[1], fourFloats[2], fourFloats[3] };
    return color;
}

struct SimpleFontStyle {
    SkFontStyle::Slant  slant;
    SkFontStyle::Weight weight;
    SkFontStyle::Width  width;
};

struct SimpleTextStyle {
    uintptr_t /* float* */ colorPtr;
    uintptr_t /* float* */ foregroundColorPtr;
    uintptr_t /* float* */ backgroundColorPtr;
    uint8_t decoration;
    SkScalar decorationThickness;
    SkScalar fontSize;
    SimpleFontStyle fontStyle;

    uintptr_t /* const char** */ fontFamiliesPtr;
    int fontFamiliesLen;
};

para::TextStyle toTextStyle(const SimpleTextStyle& s) {
    para::TextStyle ts;

    // textstyle.color doesn't support a 4f color, however the foreground and background fields below do.
    ts.setColor(toSkColor4f(s.colorPtr).toSkColor());

    // It is functionally important that these paints be unset when no value was provided.
    if (s.foregroundColorPtr) {
        SkPaint p1;
        p1.setColor4f(toSkColor4f(s.foregroundColorPtr));
        ts.setForegroundColor(p1);
    }

    if (s.backgroundColorPtr) {
        SkPaint p2;
        p2.setColor4f(toSkColor4f(s.backgroundColorPtr));
        ts.setBackgroundColor(p2);
    }

    if (s.fontSize != 0) {
        ts.setFontSize(s.fontSize);
    }

    ts.setDecoration(para::TextDecoration(s.decoration));
    if (s.decorationThickness != 0) {
        ts.setDecorationThicknessMultiplier(s.decorationThickness);
    }

    const char** fontFamilies = reinterpret_cast<const char**>(s.fontFamiliesPtr);
    if (s.fontFamiliesLen > 0 && fontFamilies != nullptr) {
        std::vector<SkString> ff;
        for (int i = 0; i < s.fontFamiliesLen; i++) {
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

struct SimpleTextBox {
    SkRect rect;
    // This isn't the most efficient way to represent this, but it is much easier to keep
    // everything as floats when unpacking on the JS side.
    // 0.0 = RTL, 1.0 = LTr
    SkScalar direction;
};

Float32Array GetRectsForRange(para::ParagraphImpl& self, unsigned start, unsigned end,
                            para::RectHeightStyle heightStyle, para::RectWidthStyle widthStyle) {
    std::vector<para::TextBox> boxes = self.getRectsForRange(start, end, heightStyle, widthStyle);
    // Pack these text boxes into an array of n groups of 5 SkScalar (floats)
    if (!boxes.size()) {
        return emscripten::val::null();
    }
    SimpleTextBox* rects = new SimpleTextBox[boxes.size()];
    for (int i = 0; i< boxes.size(); i++) {
        rects[i].rect = boxes[i].rect;
        if (boxes[i].direction == para::TextDirection::kRtl) {
            rects[i].direction = 0;
        } else {
            rects[i].direction = 1;
        }
    }
    float* fPtr = reinterpret_cast<float*>(rects);
    // Of note: now that we have cast rects to float*, emscripten is smart enough to wrap this
    // into a Float32Array for us.
    return Float32Array(typed_memory_view(boxes.size()*5, fPtr));
}

EMSCRIPTEN_BINDINGS(Paragraph) {

    class_<para::Paragraph>("Paragraph");

    // This "base<>" tells Emscripten that ParagraphImpl is a Paragraph and can get substituted
    // in properly in drawParagraph. However, Emscripten will not let us bind pure virtual methods
    // so we have to "expose" the ParagraphImpl in those cases.
    class_<para::ParagraphImpl, base<para::Paragraph>>("ParagraphImpl")
        .function("didExceedMaxLines", &para::Paragraph::didExceedMaxLines)
        .function("getAlphabeticBaseline", &para::Paragraph::getAlphabeticBaseline)
        .function("getGlyphPositionAtCoordinate", &para::ParagraphImpl::getGlyphPositionAtCoordinate)
        .function("getHeight", &para::Paragraph::getHeight)
        .function("getIdeographicBaseline", &para::Paragraph::getIdeographicBaseline)
        .function("getLongestLine", &para::Paragraph::getLongestLine)
        .function("getMaxIntrinsicWidth", &para::Paragraph::getMaxIntrinsicWidth)
        .function("getMaxWidth", &para::Paragraph::getMaxWidth)
        .function("getMinIntrinsicWidth", &para::Paragraph::getMinIntrinsicWidth)
        .function("_getRectsForRange", &GetRectsForRange)
        .function("getWordBoundary", &para::ParagraphImpl::getWordBoundary)
        .function("layout", &para::ParagraphImpl::layout);

    class_<para::ParagraphBuilderImpl>("ParagraphBuilder")
        .class_function("_Make", optional_override([](SimpleParagraphStyle style, sk_sp<SkFontMgr> fontMgr)
                        -> std::unique_ptr<para::ParagraphBuilderImpl> {
            auto fc = sk_make_sp<para::FontCollection>();
            fc->setDefaultFontManager(fontMgr);
            auto ps = toParagraphStyle(style);
            auto pb = para::ParagraphBuilderImpl::make(ps, fc);
            return std::unique_ptr<para::ParagraphBuilderImpl>(static_cast<para::ParagraphBuilderImpl*>(pb.release()));
        }), allow_raw_pointers())
      .class_function("_MakeFromFontProvider", optional_override([](SimpleParagraphStyle style,
                      sk_sp<para::TypefaceFontProvider> fontProvider)-> std::unique_ptr<para::ParagraphBuilderImpl> {
            auto fc = sk_make_sp<para::FontCollection>();
            fc->setDefaultFontManager(fontProvider);
            auto ps = toParagraphStyle(style);
            auto pb = para::ParagraphBuilderImpl::make(ps, fc);
            return std::unique_ptr<para::ParagraphBuilderImpl>(static_cast<para::ParagraphBuilderImpl*>(pb.release()));
      }), allow_raw_pointers())
        .function("addText", optional_override([](para::ParagraphBuilderImpl& self, std::string text) {
            return self.addText(text.c_str(), text.length());
        }))
        .function("build", &para::ParagraphBuilderImpl::Build, allow_raw_pointers())
        .function("pop", &para::ParagraphBuilderImpl::pop)
        .function("_pushStyle",  optional_override([](para::ParagraphBuilderImpl& self,
                                                     SimpleTextStyle textStyle) {
            auto ts = toTextStyle(textStyle);
            self.pushStyle(ts);
        }))
        // A method of pushing a textStyle with paints instead of colors for foreground and
        // background. Since SimpleTextStyle is a value object, it cannot contain paints, which are not primitives. This binding is here to accept them. Any color that is specified in the textStyle is overridden.
        .function("_pushPaintStyle",  optional_override([](para::ParagraphBuilderImpl& self,
                SimpleTextStyle textStyle, SkPaint foreground, SkPaint background) {
            auto ts = toTextStyle(textStyle);
            ts.setForegroundColor(foreground);
            ts.setBackgroundColor(background);
            self.pushStyle(ts);
        }));

    class_<para::TypefaceFontProvider, base<SkFontMgr>>("TypefaceFontProvider")
      .smart_ptr<sk_sp<para::TypefaceFontProvider>>("sk_sp<TypefaceFontProvider>")
      .class_function("Make", optional_override([]()-> sk_sp<para::TypefaceFontProvider> {
          return sk_make_sp<para::TypefaceFontProvider>();
      }))
      .function("_registerFont", optional_override([](para::TypefaceFontProvider& self,
                                                      sk_sp<SkTypeface> typeface,
                                                      uintptr_t familyPtr) {
          const char* fPtr = reinterpret_cast<const char*>(familyPtr);
          SkString fStr(fPtr);
          self.registerTypeface(typeface, fStr);
      }), allow_raw_pointers());


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
        .value("Tight",                     para::RectHeightStyle::kTight)
        .value("Max",                       para::RectHeightStyle::kMax)
        .value("IncludeLineSpacingMiddle",  para::RectHeightStyle::kIncludeLineSpacingMiddle)
        .value("IncludeLineSpacingTop",     para::RectHeightStyle::kIncludeLineSpacingTop)
        .value("IncludeLineSpacingBottom",  para::RectHeightStyle::kIncludeLineSpacingBottom);

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
        .field("_colorPtr",           &SimpleTextStyle::colorPtr)
        .field("_foregroundColorPtr", &SimpleTextStyle::foregroundColorPtr)
        .field("_backgroundColorPtr", &SimpleTextStyle::backgroundColorPtr)
        .field("decoration",          &SimpleTextStyle::decoration)
        .field("decorationThickness", &SimpleTextStyle::decorationThickness)
        .field("_fontFamiliesPtr",    &SimpleTextStyle::fontFamiliesPtr)
        .field("_fontFamiliesLen",    &SimpleTextStyle::fontFamiliesLen)
        .field("fontSize",            &SimpleTextStyle::fontSize)
        .field("fontStyle",           &SimpleTextStyle::fontStyle);

    // The U stands for unsigned - we can't bind a generic/template object, so we have to specify it
    // with the type we are using.
    value_object<para::SkRange<size_t>>("URange")
        .field("start",    &para::SkRange<size_t>::start)
        .field("end",      &para::SkRange<size_t>::end);

    // TextDecoration should be a const because they can be combined
    constant("NoDecoration", int(para::TextDecoration::kNoDecoration));
    constant("UnderlineDecoration", int(para::TextDecoration::kUnderline));
    constant("OverlineDecoration", int(para::TextDecoration::kOverline));
    constant("LineThroughDecoration", int(para::TextDecoration::kLineThrough));
}
