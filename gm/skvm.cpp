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

    // These two draws are supported by SkVMBlitter,
    // and the green draw will reuse the program cached by the blue draw.
    //
    // We don't have any API to detect this, but you can flip on the #if guard
    // around "calls to done" in SkVMBlitter.cpp and run this GM in isolation.
    // You should see 2 calls to done, one for the blitter that clears to white,
    // and one for the program used by both shader draws.
    //
    //    $ ninja -C out fm && out/fm -b cpu -s SkVMBlitter
    //    ...
    //    2 calls to done
    //
    // Clever readers might realize this could actually be one single blitter:
    // only the fact that some colors come via shader and the white clear via
    // the paint makes the two programs cache differently, despite basically
    // doing the same thing.  (TODO: unify these two paths also so they'd hit
    // the cache.)

    p.setShader(SkShaders::Color(SK_ColorBLUE));
    canvas->drawRect({0,0,50,50}, p);

    p.setShader(SkShaders::Color(SK_ColorGREEN));
    canvas->drawRect({50,50,100,100}, p);
}
