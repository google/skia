/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"

// This GM exercises interesting cases in SkVMBlitter,
// and mostly draws uninteresting simple shapes and colors.
// At the moment it's really only interesting if you #define SK_USE_SKVM_BLITTER.

DEF_SIMPLE_GM(SkVMBlitter, canvas, 100, 100) {
    SkPaint p;

    // These three draws are supported by SkVMBlitter,
    // and the later draws will reuse the program cached by earlier draws.
    //
    // We don't have any API to detect this, but you can flip on the #if guard
    // around "calls to done" in SkVMBlitter.cpp and run this GM in isolation.
    // You should see 1 call to done.
    //
    //    $ ninja -C out fm && out/fm -b cpu -s SkVMBlitter
    //    ...
    //    1 calls to done
    //
    // The program is actually first created when the GM framework first
    // clears the buffer white by setting a paint color to SK_ColorWHITE like
    // we do with SK_ColorRED.  SkVMBlitter is clever enough now to build the
    // same program for paint colors or color shaders.

    p.setShader(SkShaders::Color(SK_ColorBLUE));
    canvas->drawRect({0,0, 50,50}, p);

    p.setShader(SkShaders::Color(SK_ColorGREEN));
    canvas->drawRect({50,50, 100,100}, p);

    p.setShader(nullptr);
    p.setColor(SK_ColorRED);
    canvas->drawRect({0,50, 50,100}, p);
}
