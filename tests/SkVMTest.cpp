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
// our golden tests (kExpected) work portably.  In general there's no reason
// to fear nesting calls to Builder routines.

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

static const char* kExpected[] = {
R"(r0 = load8 arg(0)
r1 = splat 3B808081 (0.0039215689)
r0 = to_f32 r0
r0 = mul_f32 r1 r0
r2 = load8 arg(1)
r2 = to_f32 r2
r2 = mul_f32 r1 r2
r1 = splat 3F800000 (1)
r1 = sub_f32 r1 r0
r1 = mad_f32 r2 r1 r0
r2 = splat 437F0000 (255)
r0 = splat 3F000000 (0.5)
r0 = mad_f32 r1 r2 r0
r0 = to_i32 r0
store8 arg(1) r0
)",
R"(r0 = splat 0 (0)
r1 = load8 arg(0)
r2 = splat 3B808081 (0.0039215689)
r1 = to_f32 r1
r1 = mul_f32 r2 r1
r3 = load8 arg(1)
r3 = to_f32 r3
r3 = mul_f32 r2 r3
r2 = splat 3F800000 (1)
r2 = sub_f32 r2 r1
r2 = mad_f32 r3 r2 r0
r3 = splat 3E59B3D0 (0.21259999)
r0 = splat 3F371759 (0.71520001)
r1 = splat 3D93DD98 (0.0722)
r1 = mul_f32 r2 r1
r1 = mad_f32 r2 r0 r1
r1 = mad_f32 r2 r3 r1
r3 = splat 437F0000 (255)
r2 = splat 3F000000 (0.5)
r2 = mad_f32 r1 r3 r2
r2 = to_i32 r2
store8 arg(1) r2
)",
R"(r0 = splat 0 (0)
r1 = load8 arg(0)
r2 = splat 3B808081 (0.0039215689)
r1 = to_f32 r1
r1 = mul_f32 r2 r1
r3 = load32 arg(1)
r4 = splat FF (3.5733111e-43)
r5 = bit_and r3 r4
r5 = to_f32 r5
r5 = mul_f32 r2 r5
r6 = shr r3 8 (1.1210388e-44)
r6 = bit_and r6 r4
r6 = to_f32 r6
r6 = mul_f32 r2 r6
r7 = shr r3 10 (2.2420775e-44)
r7 = bit_and r7 r4
r7 = to_f32 r7
r7 = mul_f32 r2 r7
r3 = shr r3 18 (3.3631163e-44)
r3 = to_f32 r3
r3 = mul_f32 r2 r3
r2 = splat 3F800000 (1)
r2 = sub_f32 r2 r1
r5 = mad_f32 r5 r2 r0
r6 = mad_f32 r6 r2 r0
r7 = mad_f32 r7 r2 r0
r2 = mad_f32 r3 r2 r1
r3 = splat 437F0000 (255)
r1 = splat 3F000000 (0.5)
r5 = mad_f32 r5 r3 r1
r5 = to_i32 r5
r6 = mad_f32 r6 r3 r1
r6 = to_i32 r6
r6 = shl r6 8 (1.1210388e-44)
r7 = mad_f32 r7 r3 r1
r7 = to_i32 r7
r7 = shl r7 10 (2.2420775e-44)
r1 = mad_f32 r2 r3 r1
r1 = to_i32 r1
r1 = shl r1 18 (3.3631163e-44)
r6 = bit_or r5 r6
r6 = bit_or r6 r7
r6 = bit_or r6 r1
store32 arg(1) r6
)",
R"(r0 = splat 3B808081 (0.0039215689)
r1 = splat 3F800000 (1)
r2 = load8 arg(1)
r2 = to_f32 r2
r2 = mul_f32 r0 r2
r0 = sub_f32 r1 r1
r0 = mad_f32 r2 r0 r1
r2 = splat 437F0000 (255)
r1 = splat 3F000000 (0.5)
r1 = mad_f32 r0 r2 r1
r1 = to_i32 r1
store8 arg(1) r1
)",
R"(r0 = load8 arg(0)
r1 = splat 3B808081 (0.0039215689)
r0 = to_f32 r0
r0 = mul_f32 r1 r0
r2 = splat 3F800000 (1)
r3 = load8 arg(1)
r3 = to_f32 r3
r3 = mul_f32 r1 r3
r2 = sub_f32 r2 r2
r2 = mad_f32 r3 r2 r0
r3 = splat 3E59B3D0 (0.21259999)
r0 = splat 3F371759 (0.71520001)
r1 = splat 3D93DD98 (0.0722)
r1 = mul_f32 r2 r1
r1 = mad_f32 r2 r0 r1
r1 = mad_f32 r2 r3 r1
r3 = splat 437F0000 (255)
r2 = splat 3F000000 (0.5)
r2 = mad_f32 r1 r3 r2
r2 = to_i32 r2
store8 arg(1) r2
)",
R"(r0 = load8 arg(0)
r1 = splat 3B808081 (0.0039215689)
r0 = to_f32 r0
r0 = mul_f32 r1 r0
r2 = splat 3F800000 (1)
r3 = load32 arg(1)
r4 = splat FF (3.5733111e-43)
r5 = bit_and r3 r4
r5 = to_f32 r5
r5 = mul_f32 r1 r5
r6 = shr r3 8 (1.1210388e-44)
r6 = bit_and r6 r4
r6 = to_f32 r6
r6 = mul_f32 r1 r6
r7 = shr r3 10 (2.2420775e-44)
r7 = bit_and r7 r4
r7 = to_f32 r7
r7 = mul_f32 r1 r7
r3 = shr r3 18 (3.3631163e-44)
r3 = to_f32 r3
r3 = mul_f32 r1 r3
r1 = sub_f32 r2 r2
r5 = mad_f32 r5 r1 r0
r6 = mad_f32 r6 r1 r0
r7 = mad_f32 r7 r1 r0
r1 = mad_f32 r3 r1 r2
r3 = splat 437F0000 (255)
r2 = splat 3F000000 (0.5)
r5 = mad_f32 r5 r3 r2
r5 = to_i32 r5
r6 = mad_f32 r6 r3 r2
r6 = to_i32 r6
r6 = shl r6 8 (1.1210388e-44)
r7 = mad_f32 r7 r3 r2
r7 = to_i32 r7
r7 = shl r7 10 (2.2420775e-44)
r2 = mad_f32 r1 r3 r2
r2 = to_i32 r2
r2 = shl r2 18 (3.3631163e-44)
r6 = bit_or r5 r6
r6 = bit_or r6 r7
r6 = bit_or r6 r2
store32 arg(1) r6
)",
R"(r0 = load32 arg(0)
r1 = splat 3B808081 (0.0039215689)
r0 = shr r0 18 (3.3631163e-44)
r0 = to_f32 r0
r0 = mul_f32 r1 r0
r2 = load8 arg(1)
r2 = to_f32 r2
r2 = mul_f32 r1 r2
r1 = splat 3F800000 (1)
r1 = sub_f32 r1 r0
r1 = mad_f32 r2 r1 r0
r2 = splat 437F0000 (255)
r0 = splat 3F000000 (0.5)
r0 = mad_f32 r1 r2 r0
r0 = to_i32 r0
store8 arg(1) r0
)",
R"(r0 = load32 arg(0)
r1 = splat FF (3.5733111e-43)
r2 = bit_and r0 r1
r3 = splat 3B808081 (0.0039215689)
r2 = to_f32 r2
r2 = mul_f32 r3 r2
r4 = shr r0 8 (1.1210388e-44)
r4 = bit_and r4 r1
r4 = to_f32 r4
r4 = mul_f32 r3 r4
r5 = shr r0 10 (2.2420775e-44)
r5 = bit_and r5 r1
r5 = to_f32 r5
r5 = mul_f32 r3 r5
r0 = shr r0 18 (3.3631163e-44)
r0 = to_f32 r0
r0 = mul_f32 r3 r0
r1 = load8 arg(1)
r1 = to_f32 r1
r1 = mul_f32 r3 r1
r3 = splat 3F800000 (1)
r3 = sub_f32 r3 r0
r2 = mad_f32 r1 r3 r2
r4 = mad_f32 r1 r3 r4
r3 = mad_f32 r1 r3 r5
r1 = splat 3E59B3D0 (0.21259999)
r5 = splat 3F371759 (0.71520001)
r0 = splat 3D93DD98 (0.0722)
r0 = mul_f32 r3 r0
r0 = mad_f32 r4 r5 r0
r0 = mad_f32 r2 r1 r0
r1 = splat 437F0000 (255)
r2 = splat 3F000000 (0.5)
r2 = mad_f32 r0 r1 r2
r2 = to_i32 r2
store8 arg(1) r2
)",
R"(r0 = load32 arg(0)
r1 = splat FF (3.5733111e-43)
r2 = bit_and r0 r1
r3 = splat 3B808081 (0.0039215689)
r2 = to_f32 r2
r2 = mul_f32 r3 r2
r4 = shr r0 8 (1.1210388e-44)
r4 = bit_and r4 r1
r4 = to_f32 r4
r4 = mul_f32 r3 r4
r5 = shr r0 10 (2.2420775e-44)
r5 = bit_and r5 r1
r5 = to_f32 r5
r5 = mul_f32 r3 r5
r0 = shr r0 18 (3.3631163e-44)
r0 = to_f32 r0
r0 = mul_f32 r3 r0
r6 = load32 arg(1)
r7 = bit_and r6 r1
r7 = to_f32 r7
r7 = mul_f32 r3 r7
r8 = shr r6 8 (1.1210388e-44)
r8 = bit_and r8 r1
r8 = to_f32 r8
r8 = mul_f32 r3 r8
r9 = shr r6 10 (2.2420775e-44)
r9 = bit_and r9 r1
r9 = to_f32 r9
r9 = mul_f32 r3 r9
r6 = shr r6 18 (3.3631163e-44)
r6 = to_f32 r6
r6 = mul_f32 r3 r6
r3 = splat 3F800000 (1)
r3 = sub_f32 r3 r0
r7 = mad_f32 r7 r3 r2
r8 = mad_f32 r8 r3 r4
r9 = mad_f32 r9 r3 r5
r3 = mad_f32 r6 r3 r0
r6 = splat 437F0000 (255)
r0 = splat 3F000000 (0.5)
r7 = mad_f32 r7 r6 r0
r7 = to_i32 r7
r8 = mad_f32 r8 r6 r0
r8 = to_i32 r8
r8 = shl r8 8 (1.1210388e-44)
r9 = mad_f32 r9 r6 r0
r9 = to_i32 r9
r9 = shl r9 10 (2.2420775e-44)
r0 = mad_f32 r3 r6 r0
r0 = to_i32 r0
r0 = shl r0 18 (3.3631163e-44)
r8 = bit_or r7 r8
r8 = bit_or r8 r9
r8 = bit_or r8 r0
store32 arg(1) r8
)",
};

DEF_TEST(SkVM, r) {
    for (int s = 0; s < 3; s++)
    for (int d = 0; d < 3; d++) {
        auto srcFmt = (Fmt)s,
             dstFmt = (Fmt)d;
        skvm::Program program = SrcoverBuilder{srcFmt, dstFmt}.done();

        SkDynamicMemoryWStream buf;
        program.dump(&buf);
        sk_sp<SkData> blob = buf.detachAsData();

        bool train = false;
        if (train) {
            SkDebugf("R\"(%.*s)\",\n", blob->size(), blob->data());
        } else if (0 != memcmp(kExpected[3*s+d], blob->data(), blob->size())) {
            ERRORF(r, "SkVMTest needs retraining.\n");
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
            uint8_t d = dst,
                    w = want;
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
