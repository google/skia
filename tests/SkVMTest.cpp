/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkVM.h"
#include "tests/Test.h"
#include "tools/Resources.h"

enum Fmt { A8, G8, RGBA_8888 };
const char* fmt_name(Fmt fmt) {
    switch (fmt) {
        case A8:        return "A8";
        case G8:        return "G8";
        case RGBA_8888: return "RGBA_8888";
    }
    return "";
}

// Here's a cute little trick that avoids the need to explicitly thread
// and skvm::Builder* through and make a lot of builder->foo() calls.
// Instead the builder becomes this, with this-> omitted for clarity.
//
// Think of this as
//    static void srcover(skvm::Builder*, Fmt srcFmt, Fmt dstFmt) { ... }
//
// Some parts of this builder code are written less fluently than possible,
// to avoid any ambiguity of function argument evaluation order.  This lets
// our golden tests work portably.  In general there's no reason to fear
// nesting calls to Builder routines.

struct SrcoverBuilder : public skvm::Builder {
    SrcoverBuilder(Fmt srcFmt, Fmt dstFmt) {
        skvm::Arg src = arg(0),
                  dst = arg(1);

        auto byte_to_f32 = [&](skvm::I32 byte) {
            skvm::F32 _1_255 = splat(1/255.0f);
            return mul(_1_255, to_f32(byte));
        };

        auto load = [&](skvm::Arg ptr, Fmt fmt,
                        skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) {
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
                    skvm::I32 rgba = load32(ptr),
                              _255 = splat(255);
                    *r = byte_to_f32(bit_and(    rgba     , _255));
                    *g = byte_to_f32(bit_and(shr(rgba,  8), _255));
                    *b = byte_to_f32(bit_and(shr(rgba, 16), _255));
                    *a = byte_to_f32(        shr(rgba, 24)       );
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
            case A8: {
                store8(dst, f32_to_byte(a));
            } break;

            case G8: {
                skvm::F32 _2126 = splat(0.2126f),
                          _7152 = splat(0.7152f),
                          _0722 = splat(0.0722f);
                store8(dst, f32_to_byte(mad(r, _2126,
                                        mad(g, _7152,
                                        mul(b, _0722)))));
            } break;

            case RGBA_8888: {
                skvm::I32 R =     f32_to_byte(r)     ,
                          G = shl(f32_to_byte(g),  8),
                          B = shl(f32_to_byte(b), 16),
                          A = shl(f32_to_byte(a), 24);

                R = bit_or(R,G);
                R = bit_or(R,B);
                R = bit_or(R,A);

                store32(dst, R);
            } break;
        }
    }
};

DEF_TEST(SkVM, r) {
    SkDynamicMemoryWStream buf;
    for (int s = 0; s < 3; s++)
    for (int d = 0; d < 3; d++) {
        auto srcFmt = (Fmt)s,
             dstFmt = (Fmt)d;
        skvm::Program program = SrcoverBuilder{srcFmt, dstFmt}.done();

        buf.writeText(fmt_name(srcFmt));
        buf.writeText(" over ");
        buf.writeText(fmt_name(dstFmt));
        buf.writeText("\n");
        program.dump(&buf);
        buf.writeText("\n");
    }

    sk_sp<SkData> blob = buf.detachAsData();
    {

        sk_sp<SkData> expected = GetResourceAsData("SkVMTest.expected");
        REPORTER_ASSERT(r, expected
                        && blob->size() == expected->size()
                        && 0 == memcmp(blob->data(), expected->data(), blob->size()));

        SkFILEWStream out(GetResourcePath("SkVMTest.expected").c_str());
        if (out.isValid()) {
            out.write(blob->data(), blob->size());
        }
    }

    {
        skvm::Program program = SrcoverBuilder{RGBA_8888, RGBA_8888}.done();

        uint32_t src = 0xbb007733,
                 dst = 0xffaaccee;
        SkPMColor want = SkPMSrcOver(src, dst);  // 0xff2dad73

        program.eval(1, &src, &dst);

        // dst is probably 0xff2dad72.
        for (int i = 0; i < 4; i++) {
            uint8_t d = dst  & 0xff,
                    w = want & 0xff;
            REPORTER_ASSERT(r, abs(d-w) < 2);
            dst  >>= 8;
            want >>= 8;
        }
    }

    {
        skvm::Program program = SrcoverBuilder{RGBA_8888, G8}.done();

        uint32_t src = 0xbb007733;
        uint8_t dst = 0x42;
        SkPMColor over = SkPMSrcOver(SkPackARGB32(0xbb, 0x33, 0x77, 0x00), 0xff424242);

        uint8_t want = SkComputeLuminance(SkGetPackedR32(over),
                                          SkGetPackedG32(over),
                                          SkGetPackedB32(over));
        program.eval(1, &src, &dst);

        REPORTER_ASSERT(r, abs(dst-want) < 3);
    }

    {
        skvm::Program program = SrcoverBuilder{A8, A8}.done();

        uint8_t src[256],
                dst[256];
        for (int i = 0; i < 256; i++) {
            src[i] = 255 - i;
            dst[i] = i;
        }

        program.eval(256, src, dst);

        for (int i = 0; i < 256; i++) {
            uint8_t want = SkGetPackedA32(SkPMSrcOver(SkPackARGB32(src[i], 0,0,0),
                                                      SkPackARGB32(     i, 0,0,0)));
            REPORTER_ASSERT(r, abs(dst[i]-want) < 2);
        }
    }
}
