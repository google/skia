/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkVMBuilders.h"

// Some parts of this builder code are written less fluently than possible,
// to avoid any ambiguity of function argument evaluation order.  This lets
// our golden tests work portably.  In general there's no reason to fear
// nesting calls to Builder routines.

SrcoverBuilder_F32::SrcoverBuilder_F32(Fmt srcFmt, Fmt dstFmt) {
    skvm::Arg src = arg(0),
              dst = arg(1);

    auto byte_to_f32 = [&](skvm::I32 byte) {
        skvm::F32 _1_255 = splat(1/255.0f);
        return mul(_1_255, to_f32(byte));
    };

    auto load = [&](skvm::Arg ptr, Fmt fmt,
                    skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) {
        switch (fmt) {
            case Fmt::A8: {
                *r = *g = *b = splat(0.0f);
                *a = byte_to_f32(load8(ptr));
            } break;

            case Fmt::G8: {
                *r = *g = *b = byte_to_f32(load8(ptr));
                *a = splat(1.0f);
            } break;

            case Fmt::RGBA_8888: {
                skvm::I32 rgba = load32(ptr);
                *r = byte_to_f32(extract(rgba,  0, splat(0xff)));
                *g = byte_to_f32(extract(rgba,  8, splat(0xff)));
                *b = byte_to_f32(extract(rgba, 16, splat(0xff)));
                *a = byte_to_f32(extract(rgba, 24, splat(0xff)));
            } break;
        }
    };

    skvm::F32 r,g,b,a;
    load(src, srcFmt, &r,&g,&b,&a);

    skvm::F32 dr,dg,db,da;
    load(dst, dstFmt, &dr,&dg,&db,&da);

    skvm::F32 invA = sub(splat(1.0f), a);
    r = mad(dr, invA, r);
    g = mad(dg, invA, g);
    b = mad(db, invA, b);
    a = mad(da, invA, a);

    auto f32_to_byte = [&](skvm::F32 f32) {
        skvm::F32 _255 = splat(255.0f),
                  _0_5 = splat(0.5f);
        return to_i32(mad(f32, _255, _0_5));
    };
    switch (dstFmt) {
        case Fmt::A8: {
            store8(dst, f32_to_byte(a));
        } break;

        case Fmt::G8: {
            skvm::F32 _2126 = splat(0.2126f),
                      _7152 = splat(0.7152f),
                      _0722 = splat(0.0722f);
            store8(dst, f32_to_byte(mad(r, _2126,
                                    mad(g, _7152,
                                    mul(b, _0722)))));
        } break;

        case Fmt::RGBA_8888: {
            skvm::I32 R = f32_to_byte(r),
                      G = f32_to_byte(g),
                      B = f32_to_byte(b),
                      A = f32_to_byte(a);

            R = pack(R, G, 8);
            B = pack(B, A, 8);
            R = pack(R, B, 16);

            store32(dst, R);
        } break;
    }
}

SrcoverBuilder_I32::SrcoverBuilder_I32() {
    skvm::Arg src = arg(0),
              dst = arg(1);

    auto load = [&](skvm::Arg ptr,
                    skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) {
        skvm::I32 rgba = load32(ptr);
        *r = extract(rgba,  0, splat(0xff));
        *g = extract(rgba,  8, splat(0xff));
        *b = extract(rgba, 16, splat(0xff));
        *a = shr    (rgba, 24);
    };

    skvm::I32 r,g,b,a;
    load(src, &r,&g,&b,&a);

    skvm::I32 dr,dg,db,da;
    load(dst, &dr,&dg,&db,&da);

    // (xy + x)/256 is a good approximation of (xy + 127)/255
    //
    //   == (d*(255-a) + d)/256
    //   == (d*(255-a+1)  )/256
    //   == (d*(256-a  )  )/256

    // We're doing 8x8 bit multiplies in 32-bit lanes.
    // Since the inputs and results both fit in 16 bits,
    // we can use mul_16x2, which tends to be faster than mul.
    //
    // (The top 2 zero bytes of the inputs will also multiply
    // with each other to produce zero... perfect.)

    skvm::I32 invA = sub(splat(256), a);
    r = add(r, shr(mul_16x2(dr, invA), 8));
    g = add(g, shr(mul_16x2(dg, invA), 8));
    b = add(b, shr(mul_16x2(db, invA), 8));
    a = add(a, shr(mul_16x2(da, invA), 8));

    r = pack(r, g, 8);
    b = pack(b, a, 8);
    r = pack(r, b, 16);
    store32(dst, r);
}

SrcoverBuilder_I32_SWAR::SrcoverBuilder_I32_SWAR() {
    skvm::Arg src = arg(0),
              dst = arg(1);

    auto load = [&](skvm::Arg ptr,
                    skvm::I32* rb, skvm::I32* ga, skvm::I32* a) {
        skvm::I32 rgba = load32(ptr);
        *rb = extract(rgba, 0, splat(0x00ff00ff));
        *ga = extract(rgba, 8, splat(0x00ff00ff));
        * a = shr    (rgba, 24);
    };

    skvm::I32 rb, ga, a;
    load(src, &rb, &ga, &a);

    skvm::I32 ax2    = pack(a,a, 16),
              invAx2 = sub_16x2(splat(0x01000100/*256 x2*/), ax2);

    skvm::I32 drb, dga, da;
    load(dst, &drb, &dga, &da);

    // Same approximation as above,
    // but this time we make sure to use both i16 multiplies to our benefit,
    // one for r/g, the other for b/a simultaneously.

    skvm::I32 RB = shr_16x2(mul_16x2(drb, invAx2), 8),  // 8 high bits of results shifted back down.
              GA =          mul_16x2(dga, invAx2)    ;  // Keep high bits of results in high lanes,
    GA = bit_and(GA, splat(0xff00ff00));                // and mask off any low bits remaining.

    rb = add(    rb    , RB);   // src += dst*invA
    ga = add(shl(ga, 8), GA);

    store32(dst, bit_or(rb,ga));
}
