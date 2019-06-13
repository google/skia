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
        *a = extract(rgba, 24, splat(0xff));
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

    skvm::I32 invA = sub(splat(256), a);
    r = add(r, shr(mul(dr, invA), 8));
    g = add(g, shr(mul(dg, invA), 8));
    b = add(b, shr(mul(db, invA), 8));
    a = add(a, shr(mul(da, invA), 8));

    r = pack(r, g, 8);
    b = pack(b, a, 8);
    r = pack(r, b, 16);
    store32(dst, r);
}

SrcoverBuilder_I32_SWAR::SrcoverBuilder_I32_SWAR() {
    skvm::Arg src = arg(0),
              dst = arg(1);

    auto load = [&](skvm::Arg ptr,
                    skvm::I32* rb, skvm::I32* ga) {
        skvm::I32 rgba = load32(ptr);
        *rb = extract(rgba, 0, splat(0x00ff00ff));
        *ga = extract(rgba, 8, splat(0x00ff00ff));
    };

    skvm::I32 rb, ga;
    load(src, &rb, &ga);

    skvm::I32 drb, dga;
    load(dst, &drb, &dga);

    // Same approximation as above.
    skvm::I32 invA = sub(splat(256),
                         shr(ga, 16));

    skvm::I32 RB = shr(mul(drb, invA), 8),  // 8 high bits of results shifted back down.
              GA =     mul(dga, invA);      // Keep high bits of results in high lanes.
    RB = bit_and(RB, splat(0x00ff00ff));  // Mask off any low bits remaining.
    GA = bit_and(GA, splat(0xff00ff00));  // Ditto.

    rb = add(    rb    , RB);   // src += dst*invA
    ga = add(shl(ga, 8), GA);

    store32(dst, bit_or(rb,ga));
}
