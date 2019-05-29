/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#include "src/skvm/SkVM.h"

enum Fmt { A8, G8, RGBA_8888 };

// Here's a cute little trick that avoids the need to explicitly thread
// and skvm::Builder* through and make a lot of builder->foo() calls.
// Instead the builder becomes this, with this-> omitted for clarity.
//
// Think of this as
//    static void srcover(skvm::Builder*, Fmt srcFmt, Fmt dstFmt) { ... }

struct SrcoverBuilder : public skvm::Builder {
    SrcoverBuilder(Fmt srcFmt, Fmt dstFmt) {
        skvm::Uniform src = arg(0),
                      dst = arg(1);

        const skvm::Varying _255i   = splat(255),
                            _255f   = splat(255.0f),
                            _1_255f = splat(1/255.0f),
                            _0_5f   = splat(0.5f);

        auto byte_to_f32 = [&](skvm::Varying byte) {
            return mul_f32(_1_255f, to_f32(byte));
        };

        auto load = [&](skvm::Uniform ptr, Fmt fmt,
                        skvm::Varying* r, skvm::Varying* g, skvm::Varying* b, skvm::Varying* a) {
            switch (fmt) {
                case A8: {
                    *r = *g = *b = splat(0.0f);
                    *a = byte_to_f32(load8(ptr));
                } break;

                case G8: {
                    *r = *g = *b = byte_to_f32(load8(ptr));
                    *a = splat(1.0f);
                } break;

                case RGBA_8888: {
                    skvm::Varying rgba = load32(ptr);
                    *r = byte_to_f32(bit_and(    rgba     , _255i));
                    *g = byte_to_f32(bit_and(shr(rgba,  8), _255i));
                    *b = byte_to_f32(bit_and(shr(rgba, 16), _255i));
                    *a = byte_to_f32(        shr(rgba, 24)       );
                } break;
            }
        };

        label();
        skvm::Varying r,g,b,a;
        load(src, srcFmt, &r,&g,&b,&a);

        label();
        skvm::Varying dr,dg,db,da;
        load(dst, dstFmt, &dr,&dg,&db,&da);

        label();
        skvm::Varying invA = sub_f32(splat(1.0f), a);
        r = mad_f32(dr, invA, r);
        g = mad_f32(dg, invA, g);
        b = mad_f32(db, invA, b);
        a = mad_f32(da, invA, a);

        label();
        auto f32_to_byte = [&](skvm::Varying f32) {
            return to_i32(mad_f32(f32, _255f, _0_5f));
        };
        switch (dstFmt) {
            case A8: {
                store8(dst, f32_to_byte(a));
            } break;

            case G8: {
                store8(dst, f32_to_byte(mad_f32(r, splat(0.2126f),
                                        mad_f32(g, splat(0.7152f),
                                        mul_f32(b, splat(0.0722f))))));
            } break;

            case RGBA_8888: {
                store32(dst, bit_or(    f32_to_byte(r),
                             bit_or(shl(f32_to_byte(g),  8),
                             bit_or(shl(f32_to_byte(b), 16),
                                    shl(f32_to_byte(a), 24)))));
            } break;
        }
    }
};

DEF_TEST(SkVM, r) {
    for (int s = 0; s < 3; s++)
    for (int d = 0; d < 3; d++) {
        SrcoverBuilder b{(Fmt)s, (Fmt)d};

        SkDebugf("src %d, dst %d\n", s,d);
        skvm::dump(b.valops.data(), b.valops.size());
        SkDebugf("\n");
    }
}
