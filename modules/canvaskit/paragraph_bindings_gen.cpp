/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// This file is a part of a POC for more automated generation of binding code.
// It can be edited manually (for now).

#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skunicode/include/SkUnicode.h"

#include <emscripten/bind.h>

using namespace emscripten;

namespace para = skia::textlayout;

EMSCRIPTEN_BINDINGS(ParagraphGen) {
    enum_<para::Affinity>("Affinity")
            .value("Upstream", para::Affinity::kUpstream)
            .value("Downstream", para::Affinity::kDownstream);

    enum_<para::TextDecorationStyle>("DecorationStyle")
            .value("Solid", para::TextDecorationStyle::kSolid)
            .value("Double", para::TextDecorationStyle::kDouble)
            .value("Dotted", para::TextDecorationStyle::kDotted)
            .value("Dashed", para::TextDecorationStyle::kDashed)
            .value("Wavy", para::TextDecorationStyle::kWavy);

    enum_<SkFontStyle::Slant>("FontSlant")
            .value("Upright", SkFontStyle::Slant::kUpright_Slant)
            .value("Italic", SkFontStyle::Slant::kItalic_Slant)
            .value("Oblique", SkFontStyle::Slant::kOblique_Slant);

    enum_<SkFontStyle::Weight>("FontWeight")
            .value("Invisible", SkFontStyle::Weight::kInvisible_Weight)
            .value("Thin", SkFontStyle::Weight::kThin_Weight)
            .value("ExtraLight", SkFontStyle::Weight::kExtraLight_Weight)
            .value("Light", SkFontStyle::Weight::kLight_Weight)
            .value("Normal", SkFontStyle::Weight::kNormal_Weight)
            .value("Medium", SkFontStyle::Weight::kMedium_Weight)
            .value("SemiBold", SkFontStyle::Weight::kSemiBold_Weight)
            .value("Bold", SkFontStyle::Weight::kBold_Weight)
            .value("ExtraBold", SkFontStyle::Weight::kExtraBold_Weight)
            .value("Black", SkFontStyle::Weight::kBlack_Weight)
            .value("ExtraBlack", SkFontStyle::Weight::kExtraBlack_Weight);

    enum_<SkFontStyle::Width>("FontWidth")
            .value("UltraCondensed", SkFontStyle::Width::kUltraCondensed_Width)
            .value("ExtraCondensed", SkFontStyle::Width::kExtraCondensed_Width)
            .value("Condensed", SkFontStyle::Width::kCondensed_Width)
            .value("SemiCondensed", SkFontStyle::Width::kSemiCondensed_Width)
            .value("Normal", SkFontStyle::Width::kNormal_Width)
            .value("SemiExpanded", SkFontStyle::Width::kSemiExpanded_Width)
            .value("Expanded", SkFontStyle::Width::kExpanded_Width)
            .value("ExtraExpanded", SkFontStyle::Width::kExtraExpanded_Width)
            .value("UltraExpanded", SkFontStyle::Width::kUltraExpanded_Width);

    enum_<para::PlaceholderAlignment>("PlaceholderAlignment")
            .value("Baseline", para::PlaceholderAlignment::kBaseline)
            .value("AboveBaseline", para::PlaceholderAlignment::kAboveBaseline)
            .value("BelowBaseline", para::PlaceholderAlignment::kBelowBaseline)
            .value("Top", para::PlaceholderAlignment::kTop)
            .value("Bottom", para::PlaceholderAlignment::kBottom)
            .value("Middle", para::PlaceholderAlignment::kMiddle);

    enum_<para::RectHeightStyle>("RectHeightStyle")
            .value("Tight", para::RectHeightStyle::kTight)
            .value("Max", para::RectHeightStyle::kMax)
            .value("IncludeLineSpacingMiddle", para::RectHeightStyle::kIncludeLineSpacingMiddle)
            .value("IncludeLineSpacingTop", para::RectHeightStyle::kIncludeLineSpacingTop)
            .value("IncludeLineSpacingBottom", para::RectHeightStyle::kIncludeLineSpacingBottom)
            .value("Strut", para::RectHeightStyle::kStrut);

    enum_<para::RectWidthStyle>("RectWidthStyle")
            .value("Tight", para::RectWidthStyle::kTight)
            .value("Max", para::RectWidthStyle::kMax);

    enum_<para::TextAlign>("TextAlign")
            .value("Left", para::TextAlign::kLeft)
            .value("Right", para::TextAlign::kRight)
            .value("Center", para::TextAlign::kCenter)
            .value("Justify", para::TextAlign::kJustify)
            .value("Start", para::TextAlign::kStart)
            .value("End", para::TextAlign::kEnd);

    enum_<para::TextBaseline>("TextBaseline")
            .value("Alphabetic", para::TextBaseline::kAlphabetic)
            .value("Ideographic", para::TextBaseline::kIdeographic);

    enum_<para::TextDirection>("TextDirection")
            .value("LTR", para::TextDirection::kLtr)
            .value("RTL", para::TextDirection::kRtl);

    enum_<para::TextHeightBehavior>("TextHeightBehavior")
            .value("All", para::TextHeightBehavior::kAll)
            .value("DisableFirstAscent", para::TextHeightBehavior::kDisableFirstAscent)
            .value("DisableLastDescent", para::TextHeightBehavior::kDisableLastDescent)
            .value("DisableAll", para::TextHeightBehavior::kDisableAll);

    enum_<SkUnicode::LineBreakType>("LineBreakType")
            .value("SoftLineBreak", SkUnicode::LineBreakType::kSoftLineBreak)
            .value("HardLineBreak", SkUnicode::LineBreakType::kHardLineBreak);
}
