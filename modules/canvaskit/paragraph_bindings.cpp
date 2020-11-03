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
    SkColor4f color = {fourFloats[0], fourFloats[1], fourFloats[2], fourFloats[3]};
    return color;
}

struct SimpleFontStyle {
    SkFontStyle::Slant slant;
    SkFontStyle::Weight weight;
    SkFontStyle::Width width;
};

struct SimpleTextStyle {
    uintptr_t /* float* */ colorPtr;
    uintptr_t /* float* */ foregroundColorPtr;
    uintptr_t /* float* */ backgroundColorPtr;
    uint8_t decoration;
    SkScalar decorationThickness;
    uintptr_t /* float* */ decorationColorPtr;
    para::TextDecorationStyle decorationStyle;
    para::TextBaseline textBaseline;
    SkScalar fontSize;
    SkScalar letterSpacing;
    SkScalar wordSpacing;
    SkScalar heightMultiplier;
    uintptr_t /* const char* */ localePtr;
    int localeLen;
    SimpleFontStyle fontStyle;

    uintptr_t /* const char** */ fontFamiliesPtr;
    int fontFamiliesLen;

    int shadowLen;
    uintptr_t /* SkColor4f* */ shadowColorsPtr;
    uintptr_t /* SkPoint* */ shadowOffsetsPtr;
    uintptr_t /* float* */ shadowBlurRadiiPtr;

    int fontFeatureLen;
    uintptr_t /* float* */ fontFeatureNamesPtr;
    uintptr_t /* float* */ fontFeatureValuesPtr;
};

struct SimpleStrutStyle {
    uintptr_t /* const char** */ fontFamiliesPtr;
    int fontFamiliesLen;
    SimpleFontStyle fontStyle;
    SkScalar fontSize;
    SkScalar heightMultiplier;
    SkScalar leading;
    bool strutEnabled;
    bool forceStrutHeight;
};

para::StrutStyle toStrutStyle(const SimpleStrutStyle& s) {
    para::StrutStyle ss;

    const char** fontFamilies = reinterpret_cast<const char**>(s.fontFamiliesPtr);
    if (fontFamilies != nullptr) {
        std::vector<SkString> ff;
        for (int i = 0; i < s.fontFamiliesLen; i++) {
            ff.emplace_back(fontFamilies[i]);
        }
        ss.setFontFamilies(ff);
    }

    SkFontStyle fs(s.fontStyle.weight, s.fontStyle.width, s.fontStyle.slant);
    ss.setFontStyle(fs);

    if (s.fontSize != 0) {
        ss.setFontSize(s.fontSize);
    }
    if (s.heightMultiplier != 0) {
        ss.setHeight(s.heightMultiplier);
        ss.setHeightOverride(true);
    }
    if (s.leading != 0) {
        ss.setLeading(s.leading);
    }

    ss.setStrutEnabled(s.strutEnabled);
    ss.setForceStrutHeight(s.forceStrutHeight);

    return ss;
}

para::TextStyle toTextStyle(const SimpleTextStyle& s) {
    para::TextStyle ts;

    // textstyle.color doesn't support a 4f color, however the foreground and background fields
    // below do.
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
    if (s.letterSpacing != 0) {
        ts.setLetterSpacing(s.letterSpacing);
    }
    if (s.wordSpacing != 0) {
        ts.setWordSpacing(s.wordSpacing);
    }

    if (s.heightMultiplier != 0) {
        ts.setHeight(s.heightMultiplier);
        ts.setHeightOverride(true);
    }

    ts.setDecoration(para::TextDecoration(s.decoration));
    ts.setDecorationStyle(s.decorationStyle);
    if (s.decorationThickness != 0) {
        ts.setDecorationThicknessMultiplier(s.decorationThickness);
    }
    if (s.decorationColorPtr) {
        ts.setDecorationColor(toSkColor4f(s.decorationColorPtr).toSkColor());
    }

    if (s.localeLen > 0) {
        const char* localePtr = reinterpret_cast<const char*>(s.localePtr);
        SkString lStr(localePtr, s.localeLen);
        ts.setLocale(lStr);
    }

    const char** fontFamilies = reinterpret_cast<const char**>(s.fontFamiliesPtr);
    if (fontFamilies != nullptr) {
        std::vector<SkString> ff;
        for (int i = 0; i < s.fontFamiliesLen; i++) {
            ff.emplace_back(fontFamilies[i]);
        }
        ts.setFontFamilies(ff);
    }

    ts.setTextBaseline(s.textBaseline);

    SkFontStyle fs(s.fontStyle.weight, s.fontStyle.width, s.fontStyle.slant);
    ts.setFontStyle(fs);

    if (s.shadowLen > 0) {
        const SkColor4f* colors = reinterpret_cast<const SkColor4f*>(s.shadowColorsPtr);
        const SkPoint* offsets = reinterpret_cast<const SkPoint*>(s.shadowOffsetsPtr);
        const float* blurRadii = reinterpret_cast<const float*>(s.shadowBlurRadiiPtr);
        for (int i = 0; i < s.shadowLen; i++) {
            para::TextShadow shadow(colors[i].toSkColor(), offsets[i], blurRadii[i]);
            ts.addShadow(shadow);
        }
    }


    if (s.fontFeatureLen > 0) {
        const char** fontFeatureNames = reinterpret_cast<const char**>(s.fontFeatureNamesPtr);
        const int* fontFeatureValues = reinterpret_cast<const int*>(s.fontFeatureValuesPtr);
        for (int i = 0; i < s.fontFeatureLen; i++) {
            // Font features names are 4-character simple strings.
            SkString name(fontFeatureNames[i], 4);
            ts.addFontFeature(name, fontFeatureValues[i]);
        }
    }

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
    SimpleStrutStyle strutStyle;
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
    auto ss = toStrutStyle(s.strutStyle);
    ps.setStrutStyle(ss);
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

Float32Array TextBoxesToFloat32Array(std::vector<para::TextBox> boxes) {
    // Pack these text boxes into an array of n groups of 5 SkScalar (floats)
    if (!boxes.size()) {
        return emscripten::val::null();
    }
    SimpleTextBox* rects = new SimpleTextBox[boxes.size()];
    for (int i = 0; i < boxes.size(); i++) {
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
    return Float32Array(typed_memory_view(boxes.size() * 5, fPtr));
}

Float32Array GetRectsForRange(para::Paragraph& self,
                              unsigned start,
                              unsigned end,
                              para::RectHeightStyle heightStyle,
                              para::RectWidthStyle widthStyle) {
    std::vector<para::TextBox> boxes = self.getRectsForRange(start, end, heightStyle, widthStyle);
    return TextBoxesToFloat32Array(boxes);
}

Float32Array GetRectsForPlaceholders(para::Paragraph& self) {
    std::vector<para::TextBox> boxes = self.getRectsForPlaceholders();
    return TextBoxesToFloat32Array(boxes);
}

JSArray GetLineMetrics(para::Paragraph& self) {
    std::vector<skia::textlayout::LineMetrics> metrics;
    self.getLineMetrics(metrics);
    JSArray result = emscripten::val::array();
    for (auto metric : metrics) {
        JSObject m = emscripten::val::object();
        m.set("startIndex", metric.fStartIndex);
        m.set("endIndex", metric.fEndIndex);
        m.set("endExcludingWhitespaces", metric.fEndExcludingWhitespaces);
        m.set("endIncludingNewline", metric.fEndIncludingNewline);
        m.set("isHardBreak", metric.fHardBreak);
        m.set("ascent", metric.fAscent);
        m.set("descent", metric.fDescent);
        m.set("height", metric.fHeight);
        m.set("width", metric.fWidth);
        m.set("left", metric.fLeft);
        m.set("baseline", metric.fBaseline);
        m.set("lineNumber", metric.fLineNumber);
        result.call<void>("push", m);
    }
    return result;
}

EMSCRIPTEN_BINDINGS(Paragraph) {

    class_<para::Paragraph>("Paragraph")
        .function("didExceedMaxLines", &para::Paragraph::didExceedMaxLines)
        .function("getAlphabeticBaseline", &para::Paragraph::getAlphabeticBaseline)
        .function("getGlyphPositionAtCoordinate", &para::Paragraph::getGlyphPositionAtCoordinate)
        .function("getHeight", &para::Paragraph::getHeight)
        .function("getIdeographicBaseline", &para::Paragraph::getIdeographicBaseline)
        .function("getLineMetrics", &GetLineMetrics)
        .function("getLongestLine", &para::Paragraph::getLongestLine)
        .function("getMaxIntrinsicWidth", &para::Paragraph::getMaxIntrinsicWidth)
        .function("getMaxWidth", &para::Paragraph::getMaxWidth)
        .function("getMinIntrinsicWidth", &para::Paragraph::getMinIntrinsicWidth)
        .function("_getRectsForPlaceholders", &GetRectsForPlaceholders)
        .function("_getRectsForRange", &GetRectsForRange)
        .function("getWordBoundary", &para::Paragraph::getWordBoundary)
        .function("layout", &para::Paragraph::layout);

    class_<para::ParagraphBuilderImpl>("ParagraphBuilder")
            .class_function(
                    "_Make",
                    optional_override([](SimpleParagraphStyle style, sk_sp<SkFontMgr> fontMgr)
                                              -> std::unique_ptr<para::ParagraphBuilderImpl> {
                        auto fc = sk_make_sp<para::FontCollection>();
                        fc->setDefaultFontManager(fontMgr);
                        fc->enableFontFallback();
                        auto ps = toParagraphStyle(style);
                        auto pb = para::ParagraphBuilderImpl::make(ps, fc);
                        return std::unique_ptr<para::ParagraphBuilderImpl>(
                                static_cast<para::ParagraphBuilderImpl*>(pb.release()));
                    }),
                    allow_raw_pointers())
            .class_function(
                    "_MakeFromFontProvider",
                    optional_override([](SimpleParagraphStyle style,
                                         sk_sp<para::TypefaceFontProvider> fontProvider)
                                              -> std::unique_ptr<para::ParagraphBuilderImpl> {
                        auto fc = sk_make_sp<para::FontCollection>();
                        fc->setDefaultFontManager(fontProvider);
                        fc->enableFontFallback();
                        auto ps = toParagraphStyle(style);
                        auto pb = para::ParagraphBuilderImpl::make(ps, fc);
                        return std::unique_ptr<para::ParagraphBuilderImpl>(
                                static_cast<para::ParagraphBuilderImpl*>(pb.release()));
                    }),
                    allow_raw_pointers())
            .function("addText",
                      optional_override([](para::ParagraphBuilderImpl& self, std::string text) {
                          return self.addText(text.c_str(), text.length());
                      }))
            .function("build", &para::ParagraphBuilderImpl::Build, allow_raw_pointers())
            .function("pop", &para::ParagraphBuilderImpl::pop)
            .function("_pushStyle", optional_override([](para::ParagraphBuilderImpl& self,
                                                         SimpleTextStyle textStyle) {
                          auto ts = toTextStyle(textStyle);
                          self.pushStyle(ts);
                      }))
            // A method of pushing a textStyle with paints instead of colors for foreground and
            // background. Since SimpleTextStyle is a value object, it cannot contain paints, which
            // are not primitives. This binding is here to accept them. Any color that is specified
            // in the textStyle is overridden.
            .function("_pushPaintStyle",
                      optional_override([](para::ParagraphBuilderImpl& self,
                                           SimpleTextStyle textStyle, SkPaint foreground,
                                           SkPaint background) {
                          auto ts = toTextStyle(textStyle);
                          ts.setForegroundColor(foreground);
                          ts.setBackgroundColor(background);
                          self.pushStyle(ts);
                      }))
            .function("_addPlaceholder", optional_override([](para::ParagraphBuilderImpl& self,
                                                              SkScalar width,
                                                              SkScalar height,
                                                              para::PlaceholderAlignment alignment,
                                                              para::TextBaseline baseline,
                                                              SkScalar offset) {
                          para::PlaceholderStyle ps(width, height, alignment, baseline, offset);
                          self.addPlaceholder(ps);
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

    enum_<para::TextDecorationStyle>("DecorationStyle")
        .value("Solid",  para::TextDecorationStyle::kSolid)
        .value("Double", para::TextDecorationStyle::kDouble)
        .value("Dotted", para::TextDecorationStyle::kDotted)
        .value("Dashed", para::TextDecorationStyle::kDashed)
        .value("Wavy",   para::TextDecorationStyle::kWavy);

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

    enum_<para::PlaceholderAlignment>("PlaceholderAlignment")
        .value("Baseline",      para::PlaceholderAlignment::kBaseline)
        .value("AboveBaseline", para::PlaceholderAlignment::kAboveBaseline)
        .value("BelowBaseline", para::PlaceholderAlignment::kBelowBaseline)
        .value("Top",           para::PlaceholderAlignment::kTop)
        .value("Bottom",        para::PlaceholderAlignment::kBottom)
        .value("Middle",        para::PlaceholderAlignment::kMiddle);

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

    enum_<para::TextBaseline>("TextBaseline")
        .value("Alphabetic",  para::TextBaseline::kAlphabetic)
        .value("Ideographic", para::TextBaseline::kIdeographic);

    enum_<para::TextDirection>("TextDirection")
        .value("LTR",    para::TextDirection::kLtr)
        .value("RTL",    para::TextDirection::kRtl);

    // These value objects make it easier to send data across the wire.
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
        .field("textStyle",         &SimpleParagraphStyle::textStyle)
        .field("strutStyle",        &SimpleParagraphStyle::strutStyle);

    value_object<SimpleStrutStyle>("StrutStyle")
        .field("_fontFamiliesPtr", &SimpleStrutStyle::fontFamiliesPtr)
        .field("_fontFamiliesLen", &SimpleStrutStyle::fontFamiliesLen)
        .field("strutEnabled",     &SimpleStrutStyle::strutEnabled)
        .field("fontSize",         &SimpleStrutStyle::fontSize)
        .field("fontStyle",        &SimpleStrutStyle::fontStyle)
        .field("heightMultiplier", &SimpleStrutStyle::heightMultiplier)
        .field("leading",          &SimpleStrutStyle::leading)
        .field("forceStrutHeight", &SimpleStrutStyle::forceStrutHeight);

    value_object<SimpleTextStyle>("TextStyle")
        .field("_colorPtr",             &SimpleTextStyle::colorPtr)
        .field("_foregroundColorPtr",   &SimpleTextStyle::foregroundColorPtr)
        .field("_backgroundColorPtr",   &SimpleTextStyle::backgroundColorPtr)
        .field("decoration",            &SimpleTextStyle::decoration)
        .field("decorationThickness",   &SimpleTextStyle::decorationThickness)
        .field("_decorationColorPtr",   &SimpleTextStyle::decorationColorPtr)
        .field("decorationStyle",       &SimpleTextStyle::decorationStyle)
        .field("_fontFamiliesPtr",      &SimpleTextStyle::fontFamiliesPtr)
        .field("_fontFamiliesLen",      &SimpleTextStyle::fontFamiliesLen)
        .field("fontSize",              &SimpleTextStyle::fontSize)
        .field("letterSpacing",         &SimpleTextStyle::letterSpacing)
        .field("wordSpacing",           &SimpleTextStyle::wordSpacing)
        .field("heightMultiplier",      &SimpleTextStyle::heightMultiplier)
        .field("_localePtr",            &SimpleTextStyle::localePtr)
        .field("_localeLen",            &SimpleTextStyle::localeLen)
        .field("fontStyle",             &SimpleTextStyle::fontStyle)
        .field("_shadowLen",            &SimpleTextStyle::shadowLen)
        .field("_shadowColorsPtr",      &SimpleTextStyle::shadowColorsPtr)
        .field("_shadowOffsetsPtr",     &SimpleTextStyle::shadowOffsetsPtr)
        .field("_shadowBlurRadiiPtr",   &SimpleTextStyle::shadowBlurRadiiPtr)
        .field("_fontFeatureLen",       &SimpleTextStyle::fontFeatureLen)
        .field("_fontFeatureNamesPtr",  &SimpleTextStyle::fontFeatureNamesPtr)
        .field("_fontFeatureValuesPtr", &SimpleTextStyle::fontFeatureValuesPtr);

    // The U stands for unsigned - we can't bind a generic/template object, so we have to specify it
    // with the type we are using.
    // TODO(kjlubick) make this a typedarray.
    value_object<para::SkRange<size_t>>("URange")
        .field("start",    &para::SkRange<size_t>::start)
        .field("end",      &para::SkRange<size_t>::end);

    // TextDecoration should be a const because they can be combined
    constant("NoDecoration", int(para::TextDecoration::kNoDecoration));
    constant("UnderlineDecoration", int(para::TextDecoration::kUnderline));
    constant("OverlineDecoration", int(para::TextDecoration::kOverline));
    constant("LineThroughDecoration", int(para::TextDecoration::kLineThrough));
}
