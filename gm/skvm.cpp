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
#include "include/effects/SkColorMatrix.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

// This GM exercises interesting cases in SkVMBlitter,
// and mostly draws uninteresting simple shapes and colors.
// At the moment it's really only interesting if you #define SK_USE_SKVM_BLITTER.

// Just a tiny example that the (x,y) coordinate parameters are vaguely working.
// In this case we'll fade the red channel over its span vertically using `y`,
// and green horizontally using `x`.
struct Fade : public SkShaderBase {
    explicit Fade(sk_sp<SkShader> shader) : fShader(std::move(shader)) {}

    sk_sp<SkShader> fShader;

    bool isOpaque() const override { return fShader->isOpaque(); }

    bool onProgram(skvm::Builder* p,
                   SkColorSpace* dstCS,
                   skvm::Uniforms* uniforms,
                   skvm::F32 x, skvm::F32 y,
                   skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) const override {
        if (as_SB(fShader)->program(p, dstCS,
                                    uniforms,
                                    x,y, r,g,b,a)) {
            // In this GM `y` will range over 0-50 and `x` over 50-100.
            *r = p->mul(y, p->splat(1/ 50.0f));
            *g = p->mul(x, p->splat(1/100.0f));
            return true;
        }
        return false;
    }

    // Flattening is not really necessary, just nice to make serialize-8888 etc. not crash.
    void flatten(SkWriteBuffer& b) const override {
        b.writeFlattenable(fShader.get());
    }
    SK_FLATTENABLE_HOOKS(Fade)
};
sk_sp<SkFlattenable> Fade::CreateProc(SkReadBuffer& b) {
    return sk_make_sp<Fade>(b.readShader());
}
static struct RegisterFade {
    RegisterFade() { SkFlattenable::Register("Fade", Fade::CreateProc); }
} now;

DEF_SIMPLE_GM(SkVMBlitter, canvas, 100, 150) {
    SkPaint p;

    // These draws are all supported by SkVMBlitter,
    // and the some draws will reuse programs cached by earlier draws.
    //
    // We don't have any API to detect this, but you can flip on the #if guard
    // around "calls to done" in SkVMBlitter.cpp and run this GM in isolation.
    // You should see 2 call to done.
    //
    //    $ ninja -C out fm && out/fm -b cpu -s SkVMBlitter
    //    ...
    //    2 calls to done
    //
    // The first program is actually created when the GM framework first
    // clears the buffer white by setting a paint color to SK_ColorWHITE like
    // we do with SK_ColorRED.  SkVMBlitter is clever enough now to build the
    // same program for paint colors or color shaders.
    //
    // The second program handles the draw with the Fade shader.

    /*
    4 registers, 9 instructions:
    r0 = uniform32 arg(0) 8
    r1 = splat FF (3.5733111e-43)
    r2 = extract r0 0 r1
    r3 = extract r0 8 r1
    r3 = pack r2 r3 8
    r0 = extract r0 16 r1
    r1 = pack r0 r1 8
    r1 = pack r3 r1 16
    loop:
        store32 arg(1) r1
    */
    p.setShader(SkShaders::Color(SK_ColorBLUE));
    canvas->drawRect({0,0, 50,50}, p);

    p.setShader(SkShaders::Color(SK_ColorGREEN));
    canvas->drawRect({50,50, 100,100}, p);

    p.setShader(nullptr);
    p.setColor(SK_ColorRED);
    canvas->drawRect({0,50, 50,100}, p);

    /*
    5 registers, 19 instructions:
    r0 = uniform32 arg(0) 4                    // load y
    r0 = to_f32 r0
    r1 = splat 40A33333 (5.0999999)
    r1 = mul_f32 r0 r1
    r1 = to_i32 r1                             // r1 = red channel, depends on y, is uniform
    r0 = uniform32 arg(0) 0                    // load right edge, used to calculate x in loop
    r2 = splat 40233333 (2.55)
    r3 = splat FF (3.5733111e-43)              // color shader alpha, known to be opaque
    r4 = uniform32 arg(0) 8                    // load color shader for blue
    r4 = extract r4 16 r3                      // extract blue
    r3 = pack r4 r3 8                          // r3 = blue and alpha from color shader
    loop:
        r4 = index
        r4 = sub_i32 r0 r4                     // r4 = x
        r4 = to_f32 r4
        r4 = mul_f32 r4 r2
        r4 = to_i32 r4                         // r4 = green channel, depends on x, is varying
        r4 = pack r1 r4 8
        r4 = pack r4 r3 16
        store32 arg(1) r4
    */
    p.setShader(sk_make_sp<Fade>(SkShaders::Color(SK_ColorYELLOW)));
    canvas->drawRect({50,0, 100,50}, p);

    // Draw with color filter, w/ and w/o alpha modulation.
    SkColorMatrix m;
    m.setSaturation(0.3f);

    p.setShader(SkShaders::Color(SK_ColorMAGENTA));
    p.setColorFilter(SkColorFilters::Matrix(m));
    canvas->drawRect({0,100, 50,150}, p);

    p.setShader(SkShaders::Color(0xffffaa00));  // tan
    p.setColorFilter(SkColorFilters::Matrix(m));
    p.setAlphaf(0.5f);
    canvas->drawRect({25,100, 75,150}, p); // overlap a bit with purple and white
}
