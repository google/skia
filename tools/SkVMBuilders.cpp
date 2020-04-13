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
    auto load = [&](Fmt fmt, skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) {
        skvm::Arg ptr;
        switch (fmt) {
            case Fmt::A8: {
                ptr = varying<uint8_t>();
                *r = *g = *b = splat(0.0f);
                *a = from_unorm(8, load8(ptr));
            } break;

            case Fmt::G8: {
                ptr = varying<uint8_t>();
                *r = *g = *b = from_unorm(8, load8(ptr));
                *a = splat(1.0f);
            } break;

            case Fmt::RGBA_8888: {
                ptr = varying<int>();
                skvm::I32 rgba = load32(ptr);
                *r = from_unorm(8, extract(rgba,  0, splat(0xff)));
                *g = from_unorm(8, extract(rgba,  8, splat(0xff)));
                *b = from_unorm(8, extract(rgba, 16, splat(0xff)));
                *a = from_unorm(8, extract(rgba, 24, splat(0xff)));
            } break;
        }
        return ptr;
    };

    skvm::F32 r,g,b,a;
    (void)load(srcFmt, &r,&g,&b,&a);

    skvm::F32 dr,dg,db,da;
    skvm::Arg dst = load(dstFmt, &dr,&dg,&db,&da);

    skvm::F32 invA = sub(splat(1.0f), a);
    r = mad(dr, invA, r);
    g = mad(dg, invA, g);
    b = mad(db, invA, b);
    a = mad(da, invA, a);

    switch (dstFmt) {
        case Fmt::A8: {
            store8(dst, to_unorm(8, a));
        } break;

        case Fmt::G8: {
            skvm::F32 _2126 = splat(0.2126f),
                      _7152 = splat(0.7152f),
                      _0722 = splat(0.0722f);
            store8(dst, to_unorm(8, mad(r, _2126,
                                    mad(g, _7152,
                                    mul(b, _0722)))));
        } break;

        case Fmt::RGBA_8888: {
            skvm::I32 R = to_unorm(8, r),
                      G = to_unorm(8, g),
                      B = to_unorm(8, b),
                      A = to_unorm(8, a);

            R = pack(R, G, 8);
            B = pack(B, A, 8);
            R = pack(R, B, 16);

            store32(dst, R);
        } break;
    }
}

SrcoverBuilder_I32_Naive::SrcoverBuilder_I32_Naive() {
    skvm::Arg src = varying<int>(),
              dst = varying<int>();

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
