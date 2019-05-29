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

        auto byte_to_f32 = [&](skvm::Val byte) {
            skvm::Val _1_255 = splat(1/255.0f);
            return mul_f32(_1_255, to_f32(byte));
        };

        auto load = [&](skvm::Arg ptr, Fmt fmt,
                        skvm::Val* r, skvm::Val* g, skvm::Val* b, skvm::Val* a) {
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
                    skvm::Val rgba = load32(ptr),
                              _255 = splat(255);
                    *r = byte_to_f32(bit_and(    rgba     , _255));
                    *g = byte_to_f32(bit_and(shr(rgba,  8), _255));
                    *b = byte_to_f32(bit_and(shr(rgba, 16), _255));
                    *a = byte_to_f32(        shr(rgba, 24)       );
                } break;
            }
        };

        skvm::Val r,g,b,a;
        load(src, srcFmt, &r,&g,&b,&a);

        skvm::Val dr,dg,db,da;
        load(dst, dstFmt, &dr,&dg,&db,&da);

        skvm::Val invA = sub_f32(splat(1.0f), a);
        r = mad_f32(dr, invA, r);
        g = mad_f32(dg, invA, g);
        b = mad_f32(db, invA, b);
        a = mad_f32(da, invA, a);

        auto f32_to_byte = [&](skvm::Val f32) {
            skvm::Val _255 = splat(255.0f),
                      _0_5 = splat(0.5f);
            return to_i32(mad_f32(f32, _255, _0_5));
        };
        switch (dstFmt) {
            case A8: {
                store8(dst, f32_to_byte(a));
            } break;

            case G8: {
                skvm::Val _2126 = splat(0.2126f),
                          _7152 = splat(0.7152f),
                          _0722 = splat(0.0722f);
                store8(dst, f32_to_byte(mad_f32(r, _2126,
                                        mad_f32(g, _7152,
                                        mul_f32(b, _0722)))));
            } break;

            case RGBA_8888: {
                r =     f32_to_byte(r)     ;
                g = shl(f32_to_byte(g),  8);
                b = shl(f32_to_byte(b), 16);
                a = shl(f32_to_byte(a), 24);

                r = bit_or(r,g);
                r = bit_or(r,b);
                r = bit_or(r,a);

                store32(dst, r);
            } break;
        }
    }
};

static const char* kExpected[] = {
R"(r0 = load8 arg(0)
r1 = splat 3B808081 (0.0039215689)
r2 = to_f32 r0
r3 = mul_f32 r1 r2
r4 = load8 arg(1)
r5 = to_f32 r4
r6 = mul_f32 r1 r5
r7 = splat 3F800000 (1)
r8 = sub_f32 r7 r3
r9 = mad_f32 r6 r8 r3
r10 = splat 437F0000 (255)
r11 = splat 3F000000 (0.5)
r12 = mad_f32 r9 r10 r11
r13 = to_i32 r12
store8 arg(1) r13
)",
R"(r0 = splat 0 (0)
r1 = load8 arg(0)
r2 = splat 3B808081 (0.0039215689)
r3 = to_f32 r1
r4 = mul_f32 r2 r3
r5 = load8 arg(1)
r6 = to_f32 r5
r7 = mul_f32 r2 r6
r8 = splat 3F800000 (1)
r9 = sub_f32 r8 r4
r10 = mad_f32 r7 r9 r0
r11 = splat 3E59B3D0 (0.21259999)
r12 = splat 3F371759 (0.71520001)
r13 = splat 3D93DD98 (0.0722)
r14 = mul_f32 r10 r13
r15 = mad_f32 r10 r12 r14
r16 = mad_f32 r10 r11 r15
r17 = splat 437F0000 (255)
r18 = splat 3F000000 (0.5)
r19 = mad_f32 r16 r17 r18
r20 = to_i32 r19
store8 arg(1) r20
)",
R"(r0 = splat 0 (0)
r1 = load8 arg(0)
r2 = splat 3B808081 (0.0039215689)
r3 = to_f32 r1
r4 = mul_f32 r2 r3
r5 = load32 arg(1)
r6 = splat FF (3.5733111e-43)
r7 = bit_and r5 r6
r8 = to_f32 r7
r9 = mul_f32 r2 r8
r10 = shr r5 8 (1.1210388e-44)
r11 = bit_and r10 r6
r12 = to_f32 r11
r13 = mul_f32 r2 r12
r14 = shr r5 10 (2.2420775e-44)
r15 = bit_and r14 r6
r16 = to_f32 r15
r17 = mul_f32 r2 r16
r18 = shr r5 18 (3.3631163e-44)
r19 = to_f32 r18
r20 = mul_f32 r2 r19
r21 = splat 3F800000 (1)
r22 = sub_f32 r21 r4
r23 = mad_f32 r9 r22 r0
r24 = mad_f32 r13 r22 r0
r25 = mad_f32 r17 r22 r0
r26 = mad_f32 r20 r22 r4
r27 = splat 437F0000 (255)
r28 = splat 3F000000 (0.5)
r29 = mad_f32 r23 r27 r28
r30 = to_i32 r29
r31 = mad_f32 r24 r27 r28
r32 = to_i32 r31
r33 = shl r32 8 (1.1210388e-44)
r34 = mad_f32 r25 r27 r28
r35 = to_i32 r34
r36 = shl r35 10 (2.2420775e-44)
r37 = mad_f32 r26 r27 r28
r38 = to_i32 r37
r39 = shl r38 18 (3.3631163e-44)
r40 = bit_or r30 r33
r41 = bit_or r40 r36
r42 = bit_or r41 r39
store32 arg(1) r42
)",
R"(r0 = splat 3B808081 (0.0039215689)
r1 = splat 3F800000 (1)
r2 = load8 arg(1)
r3 = to_f32 r2
r4 = mul_f32 r0 r3
r5 = sub_f32 r1 r1
r6 = mad_f32 r4 r5 r1
r7 = splat 437F0000 (255)
r8 = splat 3F000000 (0.5)
r9 = mad_f32 r6 r7 r8
r10 = to_i32 r9
store8 arg(1) r10
)",
R"(r0 = load8 arg(0)
r1 = splat 3B808081 (0.0039215689)
r2 = to_f32 r0
r3 = mul_f32 r1 r2
r4 = splat 3F800000 (1)
r5 = load8 arg(1)
r6 = to_f32 r5
r7 = mul_f32 r1 r6
r8 = sub_f32 r4 r4
r9 = mad_f32 r7 r8 r3
r10 = splat 3E59B3D0 (0.21259999)
r11 = splat 3F371759 (0.71520001)
r12 = splat 3D93DD98 (0.0722)
r13 = mul_f32 r9 r12
r14 = mad_f32 r9 r11 r13
r15 = mad_f32 r9 r10 r14
r16 = splat 437F0000 (255)
r17 = splat 3F000000 (0.5)
r18 = mad_f32 r15 r16 r17
r19 = to_i32 r18
store8 arg(1) r19
)",
R"(r0 = load8 arg(0)
r1 = splat 3B808081 (0.0039215689)
r2 = to_f32 r0
r3 = mul_f32 r1 r2
r4 = splat 3F800000 (1)
r5 = load32 arg(1)
r6 = splat FF (3.5733111e-43)
r7 = bit_and r5 r6
r8 = to_f32 r7
r9 = mul_f32 r1 r8
r10 = shr r5 8 (1.1210388e-44)
r11 = bit_and r10 r6
r12 = to_f32 r11
r13 = mul_f32 r1 r12
r14 = shr r5 10 (2.2420775e-44)
r15 = bit_and r14 r6
r16 = to_f32 r15
r17 = mul_f32 r1 r16
r18 = shr r5 18 (3.3631163e-44)
r19 = to_f32 r18
r20 = mul_f32 r1 r19
r21 = sub_f32 r4 r4
r22 = mad_f32 r9 r21 r3
r23 = mad_f32 r13 r21 r3
r24 = mad_f32 r17 r21 r3
r25 = mad_f32 r20 r21 r4
r26 = splat 437F0000 (255)
r27 = splat 3F000000 (0.5)
r28 = mad_f32 r22 r26 r27
r29 = to_i32 r28
r30 = mad_f32 r23 r26 r27
r31 = to_i32 r30
r32 = shl r31 8 (1.1210388e-44)
r33 = mad_f32 r24 r26 r27
r34 = to_i32 r33
r35 = shl r34 10 (2.2420775e-44)
r36 = mad_f32 r25 r26 r27
r37 = to_i32 r36
r38 = shl r37 18 (3.3631163e-44)
r39 = bit_or r29 r32
r40 = bit_or r39 r35
r41 = bit_or r40 r38
store32 arg(1) r41
)",
R"(r0 = load32 arg(0)
r1 = splat 3B808081 (0.0039215689)
r2 = shr r0 18 (3.3631163e-44)
r3 = to_f32 r2
r4 = mul_f32 r1 r3
r5 = load8 arg(1)
r6 = to_f32 r5
r7 = mul_f32 r1 r6
r8 = splat 3F800000 (1)
r9 = sub_f32 r8 r4
r10 = mad_f32 r7 r9 r4
r11 = splat 437F0000 (255)
r12 = splat 3F000000 (0.5)
r13 = mad_f32 r10 r11 r12
r14 = to_i32 r13
store8 arg(1) r14
)",
R"(r0 = load32 arg(0)
r1 = splat FF (3.5733111e-43)
r2 = bit_and r0 r1
r3 = splat 3B808081 (0.0039215689)
r4 = to_f32 r2
r5 = mul_f32 r3 r4
r6 = shr r0 8 (1.1210388e-44)
r7 = bit_and r6 r1
r8 = to_f32 r7
r9 = mul_f32 r3 r8
r10 = shr r0 10 (2.2420775e-44)
r11 = bit_and r10 r1
r12 = to_f32 r11
r13 = mul_f32 r3 r12
r14 = shr r0 18 (3.3631163e-44)
r15 = to_f32 r14
r16 = mul_f32 r3 r15
r17 = load8 arg(1)
r18 = to_f32 r17
r19 = mul_f32 r3 r18
r20 = splat 3F800000 (1)
r21 = sub_f32 r20 r16
r22 = mad_f32 r19 r21 r5
r23 = mad_f32 r19 r21 r9
r24 = mad_f32 r19 r21 r13
r25 = splat 3E59B3D0 (0.21259999)
r26 = splat 3F371759 (0.71520001)
r27 = splat 3D93DD98 (0.0722)
r28 = mul_f32 r24 r27
r29 = mad_f32 r23 r26 r28
r30 = mad_f32 r22 r25 r29
r31 = splat 437F0000 (255)
r32 = splat 3F000000 (0.5)
r33 = mad_f32 r30 r31 r32
r34 = to_i32 r33
store8 arg(1) r34
)",
R"(r0 = load32 arg(0)
r1 = splat FF (3.5733111e-43)
r2 = bit_and r0 r1
r3 = splat 3B808081 (0.0039215689)
r4 = to_f32 r2
r5 = mul_f32 r3 r4
r6 = shr r0 8 (1.1210388e-44)
r7 = bit_and r6 r1
r8 = to_f32 r7
r9 = mul_f32 r3 r8
r10 = shr r0 10 (2.2420775e-44)
r11 = bit_and r10 r1
r12 = to_f32 r11
r13 = mul_f32 r3 r12
r14 = shr r0 18 (3.3631163e-44)
r15 = to_f32 r14
r16 = mul_f32 r3 r15
r17 = load32 arg(1)
r18 = bit_and r17 r1
r19 = to_f32 r18
r20 = mul_f32 r3 r19
r21 = shr r17 8 (1.1210388e-44)
r22 = bit_and r21 r1
r23 = to_f32 r22
r24 = mul_f32 r3 r23
r25 = shr r17 10 (2.2420775e-44)
r26 = bit_and r25 r1
r27 = to_f32 r26
r28 = mul_f32 r3 r27
r29 = shr r17 18 (3.3631163e-44)
r30 = to_f32 r29
r31 = mul_f32 r3 r30
r32 = splat 3F800000 (1)
r33 = sub_f32 r32 r16
r34 = mad_f32 r20 r33 r5
r35 = mad_f32 r24 r33 r9
r36 = mad_f32 r28 r33 r13
r37 = mad_f32 r31 r33 r16
r38 = splat 437F0000 (255)
r39 = splat 3F000000 (0.5)
r40 = mad_f32 r34 r38 r39
r41 = to_i32 r40
r42 = mad_f32 r35 r38 r39
r43 = to_i32 r42
r44 = shl r43 8 (1.1210388e-44)
r45 = mad_f32 r36 r38 r39
r46 = to_i32 r45
r47 = shl r46 10 (2.2420775e-44)
r48 = mad_f32 r37 r38 r39
r49 = to_i32 r48
r50 = shl r49 18 (3.3631163e-44)
r51 = bit_or r41 r44
r52 = bit_or r51 r47
r53 = bit_or r52 r50
store32 arg(1) r53
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
