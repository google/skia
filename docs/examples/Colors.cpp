// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Colors, 128, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const struct { SkColor4f fColor; const char* fName; } kColors[] = {
        {SkColors::kBlack,   "SkColors::kBlack"},
        {SkColors::kDkGray,  "SkColors::kDkGray"},
        {SkColors::kGray,    "SkColors::kGray"},
        {SkColors::kLtGray,  "SkColors::kLtGray"},
        {SkColors::kWhite,   "SkColors::kWhite"},
        {SkColors::kRed,     "SkColors::kRed"},
        {SkColors::kGreen,   "SkColors::kGreen"},
        {SkColors::kBlue,    "SkColors::kBlue"},
        {SkColors::kYellow,  "SkColors::kYellow"},
        {SkColors::kCyan,    "SkColors::kCyan"},
        {SkColors::kMagenta, "SkColors::kMagenta"},
    };
    float y = 0;
    constexpr float kSize = 256.0f / (sizeof(kColors) / sizeof(kColors[0]));
    const SkColor4f kBrown{0.5f, 0.25f, 0, 1};
    SkFont defaultFont = SkFont(fontMgr->matchFamilyStyle(nullptr, {}));
    for (const auto& c : kColors) {
        canvas->drawRect(SkRect{0, y, 128, y + kSize}, SkPaint(c.fColor));
        canvas->drawString(c.fName, 4, y + kSize * 0.7f, defaultFont, SkPaint(kBrown));
        y += kSize;
    }
}
}  // END FIDDLE
