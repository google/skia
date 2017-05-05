/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

// Hue, Saturation, Color, and Luminosity blend modes are oddballs.
// They nominally convert their inputs to unpremul, then to HSL, then
// mix-and-match H,S,and L from Src and Dst, then convert back, then premul.
//
// Each mode's name describes the Src H,S,L components to keep, taking the
// others from Dst, where Color == Hue + Saturation.  Color and Luminosity
// are each other's complements; Hue and Saturation have no complement.
//
// We have had many inconsistent implementations of these modes.
// This GM tries to demonstrate unambigously how they should work.
//
// TODO: double- and triple-check expected colors
// TODO: how does gamma-correction factor into this?

DEF_SIMPLE_GM(hsl, canvas, 600, 100) {
    SkPaint label;
    sk_tool_utils::set_portable_typeface(&label);
    label.setAntiAlias(true);

    const char* comment = "HSL blend modes are correct when you see no circles in the squares.";
    canvas->drawText(comment, strlen(comment), 10,10, label);

    // Just to keep things reaaaal simple, we'll only use opaque colors.
    SkPaint bg, fg;
    bg.setColor(0xff00ff00);  // Fully-saturated bright green,  H = 120°, S = 100%, L =  50%.
    fg.setColor(0xff7f3f7f);  // Partly-saturated dim magenta,  H = 300°, S = ~33%, L = ~37%.

    struct {
        SkBlendMode mode;
        SkColor expected;
    } tests[] = {
        { SkBlendMode::kSrc,     fg.getColor() },
        { SkBlendMode::kDst,     bg.getColor() },
        { SkBlendMode::kHue,        0xffff00ff },  // bright magenta, H = 300°, S = 100%, L =  50%
        { SkBlendMode::kSaturation, 0xff55aa55 },  // a duller green, H = 120°, S = ~33%, L =  50%
        { SkBlendMode::kColor,      0xffaa55aa },  // a lighter fg,   H = 300°, S = ~33%, L =  50%
        { SkBlendMode::kLuminosity, 0xff00bd00 },  // a darker bg,    H = 120°, S = 100%, L = ~37%
    };
    for (auto test : tests) {
        canvas->drawRect({20,20,80,80}, bg);

        fg.setBlendMode(test.mode);
        canvas->drawRect({20,20,80,80}, fg);

        SkPaint expected;
        expected.setColor(test.expected);
        canvas->drawCircle(50,50, 20, expected);

        const char* name = SkBlendMode_Name(test.mode);
        canvas->drawText(name, strlen(name), 20,90, label);

        canvas->translate(100,0);
    }
}
