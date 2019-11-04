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
#include "src/shaders/SkShaderBase.h"

// This GM exercises interesting cases in SkVMBlitter,
// and mostly draws uninteresting simple shapes and colors.
// At the moment it's really only interesting if you #define SK_USE_SKVM_BLITTER.

// Just a tiny example that the (x,y) coordinate parameters are vaguely working.
// In this case we'll fade the red channel over its span vertically using `y`.
// `x` is not useful yet, since it's (incorrectly) uniform and will always be constant.
struct Fade : public SkShaderBase {
    explicit Fade(sk_sp<SkShader> shader) : fShader(std::move(shader)) {}

    sk_sp<SkShader> fShader;

    bool onProgram(skvm::Builder* p,
                   SkColorSpace* dstCS,
                   skvm::Arg uniforms, int offset,
                   skvm::F32 x, skvm::F32 y,
                   skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) const override {
        if (as_SB(fShader)->program(p, dstCS,
                                    uniforms, offset,
                                    x,y, r,g,b,a)) {
            if (p) {
                // In this GM `y` will range over 0-50.
                *r = p->to_i32(p->mul(y, p->splat(255/50.0f)));
            }
            return true;
        }
        return false;
    }

    size_t uniforms(SkColorSpace* dstCS, uint8_t* buf) const override {
        return as_SB(fShader)->uniforms(dstCS, buf);
    }

    // Only created here, should never be flattened / unflattened.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return "Fade"; }
};

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

    p.setShader(sk_make_sp<Fade>(SkShaders::Color(SK_ColorYELLOW)));
    canvas->drawRect({50,0, 100,50}, p);
}
