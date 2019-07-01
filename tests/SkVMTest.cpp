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
#include "tools/SkVMBuilders.h"

using Fmt = SrcoverBuilder_F32::Fmt;
const char* fmt_name(Fmt fmt) {
    switch (fmt) {
        case Fmt::A8:        return "A8";
        case Fmt::G8:        return "G8";
        case Fmt::RGBA_8888: return "RGBA_8888";
    }
    return "";
}

DEF_TEST(SkVM, r) {
    SkDynamicMemoryWStream buf;

    // Write all combinations of SrcoverBuilder_F32
    for (int s = 0; s < 3; s++)
    for (int d = 0; d < 3; d++) {
        auto srcFmt = (Fmt)s,
             dstFmt = (Fmt)d;
        SrcoverBuilder_F32 builder{srcFmt, dstFmt};
        skvm::Program program = builder.done();

        buf.writeText(fmt_name(srcFmt));
        buf.writeText(" over ");
        buf.writeText(fmt_name(dstFmt));
        buf.writeText("\n");
        builder.dump(&buf);
        buf.writeText("\n");
        program.dump(&buf);
        buf.writeText("\n");
    }

    // Write the I32 Srcovers also.
    {
        skvm::Program program = SrcoverBuilder_I32_Naive{}.done();
        buf.writeText("I32 (Naive) 8888 over 8888\n");
        program.dump(&buf);
        buf.writeText("\n");
    }
    {
        skvm::Program program = SrcoverBuilder_I32{}.done();
        buf.writeText("I32 8888 over 8888\n");
        program.dump(&buf);
        buf.writeText("\n");
    }
    {
        skvm::Program program = SrcoverBuilder_I32_SWAR{}.done();
        buf.writeText("I32 (SWAR) 8888 over 8888\n");
        program.dump(&buf);
        buf.writeText("\n");
    }

    sk_sp<SkData> blob = buf.detachAsData();
    {

        sk_sp<SkData> expected = GetResourceAsData("SkVMTest.expected");
        REPORTER_ASSERT(r, expected, "Couldn't load SkVMTest.expected.");
        if (expected) {
            if (blob->size() != expected->size()
                    || 0 != memcmp(blob->data(), expected->data(), blob->size())) {

                ERRORF(r, "SkVMTest expected\n%.*s\nbut got\n%.*s\n",
                       expected->size(), expected->data(),
                       blob->size(), blob->data());
            }

            SkFILEWStream out(GetResourcePath("SkVMTest.expected").c_str());
            if (out.isValid()) {
                out.write(blob->data(), blob->size());
            }
        }
    }

    auto test_8888 = [&](const skvm::Program& program) {
        uint32_t src[9];
        uint32_t dst[SK_ARRAY_COUNT(src)];

        for (int i = 0; i < (int)SK_ARRAY_COUNT(src); i++) {
            src[i] = 0xbb007733;
            dst[i] = 0xffaaccee;
        }

        SkPMColor expected = SkPMSrcOver(src[0], dst[0]);  // 0xff2dad73

        program.eval((int)SK_ARRAY_COUNT(src), src, dst);

        // dst is probably 0xff2dad72.
        for (auto got : dst) {
            auto want = expected;
            for (int i = 0; i < 4; i++) {
                uint8_t d = got  & 0xff,
                        w = want & 0xff;
                REPORTER_ASSERT(r, abs(d-w) < 2);
                got  >>= 8;
                want >>= 8;
            }
        }
    };

    test_8888(SrcoverBuilder_F32{Fmt::RGBA_8888, Fmt::RGBA_8888}.done());
    test_8888(SrcoverBuilder_I32_Naive{}.done());
    test_8888(SrcoverBuilder_I32{}.done());
    test_8888(SrcoverBuilder_I32_SWAR{}.done());

    {
        skvm::Program program = SrcoverBuilder_F32{Fmt::RGBA_8888, Fmt::G8}.done();

        uint32_t src[9];
        uint8_t  dst[SK_ARRAY_COUNT(src)];

        for (int i = 0; i < (int)SK_ARRAY_COUNT(src); i++) {
            src[i] = 0xbb007733;
            dst[i] = 0x42;
        }

        SkPMColor over = SkPMSrcOver(SkPackARGB32(0xbb, 0x33, 0x77, 0x00),
                                     0xff424242);

        uint8_t want = SkComputeLuminance(SkGetPackedR32(over),
                                          SkGetPackedG32(over),
                                          SkGetPackedB32(over));
        program.eval((int)SK_ARRAY_COUNT(src), src, dst);

        for (auto got : dst) {
            REPORTER_ASSERT(r, abs(got-want) < 3);
        }
    }

    {
        skvm::Program program = SrcoverBuilder_F32{Fmt::A8, Fmt::A8}.done();

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

DEF_TEST(SkVM_LoopCounts, r) {
    // Make sure we cover all the exact N we want.

    int buf[64];
    for (int N = 0; N <= (int)SK_ARRAY_COUNT(buf); N++) {
        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            buf[i] = i;
        }

        // buf[i] += 1
        skvm::Builder b;
        b.store32(b.arg(0),
                  b.add(b.splat(1),
                        b.load32(b.arg(0))));

        skvm::Program program = b.done();
        program.eval(N, buf);

        for (int i = 0; i < N; i++) {
            REPORTER_ASSERT(r, buf[i] == i+1);
        }
        for (int i = N; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == i);
        }
    }
}


template <typename Fn>
static void test_asm(skiatest::Reporter* r, Fn&& fn, std::initializer_list<uint8_t> expected) {
    uint8_t buf[4096];
    skvm::Assembler a{buf};
    fn(a);

    REPORTER_ASSERT(r, a.size() == expected.size());

    auto got = (const uint8_t*)buf,
         want = expected.begin();
    for (int i = 0; i < (int)std::min(a.size(), expected.size()); i++) {
        REPORTER_ASSERT(r, got[i] == want[i],
                        "byte %d was %02x, want %02x", i, got[i], want[i]);
    }
}

DEF_TEST(SkVM_Assembler, r) {
    // Easiest way to generate test cases is
    //
    //   echo '...some asm...' | llvm-mc -show-encoding -x86-asm-syntax=intel
    //
    // The -x86-asm-syntax=intel bit is optional, controlling the
    // input syntax only; the output will always be AT&T  op x,y,dst style.
    // Our APIs read more like Intel op dst,x,y as op(dst,x,y), so I find
    // that a bit easier to use here, despite maybe favoring AT&T overall.

    using A = skvm::Assembler;
    // Our exit strategy from AVX code.
    test_asm(r, [&](A& a) {
        a.vzeroupper();
        a.ret();
    },{
        0xc5, 0xf8, 0x77,
        0xc3,
    });

    // Align should pad with nop().
    test_asm(r, [&](A& a) {
        a.ret();
        a.align(4);
    },{
        0xc3,
        0x90, 0x90, 0x90,
    });

    test_asm(r, [&](A& a) {
        a.add(A::rax, 8);       // Always good to test rax.
        a.sub(A::rax, 32);

        a.add(A::rdi, 12);      // Last 0x48 REX
        a.sub(A::rdi, 8);

        a.add(A::r8 , 7);       // First 0x4c REX
        a.sub(A::r8 , 4);

        a.add(A::rsi, 128);     // Requires 4 byte immediate.
        a.sub(A::r8 , 1000000);
    },{
        0x48, 0x83, 0b11'000'000, 0x08,
        0x48, 0x83, 0b11'101'000, 0x20,

        0x48, 0x83, 0b11'000'111, 0x0c,
        0x48, 0x83, 0b11'101'111, 0x08,

        0x4c, 0x83, 0b11'000'000, 0x07,
        0x4c, 0x83, 0b11'101'000, 0x04,

        0x48, 0x81, 0b11'000'110, 0x80, 0x00, 0x00, 0x00,
        0x4c, 0x81, 0b11'101'000, 0x40, 0x42, 0x0f, 0x00,
    });


    test_asm(r, [&](A& a) {
        a.vpaddd (A::ymm0, A::ymm1, A::ymm2);  // Low registers and 0x0f map     -> 2-byte VEX.
        a.vpaddd (A::ymm8, A::ymm1, A::ymm2);  // A high dst register is ok      -> 2-byte VEX.
        a.vpaddd (A::ymm0, A::ymm8, A::ymm2);  // A high first argument register -> 2-byte VEX.
        a.vpaddd (A::ymm0, A::ymm1, A::ymm8);  // A high second argument         -> 3-byte VEX.
        a.vpmulld(A::ymm0, A::ymm1, A::ymm2);  // Using non-0x0f map instruction -> 3-byte VEX.
        a.vpsubd (A::ymm0, A::ymm1, A::ymm2);  // Test vpsubd to ensure argument order is right.
    },{
        /*    VEX     */ /*op*/ /*modRM*/
        0xc5,       0xf5, 0xfe, 0xc2,
        0xc5,       0x75, 0xfe, 0xc2,
        0xc5,       0xbd, 0xfe, 0xc2,
        0xc4, 0xc1, 0x75, 0xfe, 0xc0,
        0xc4, 0xe2, 0x75, 0x40, 0xc2,
        0xc5,       0xf5, 0xfa, 0xc2,
    });

    test_asm(r, [&](A& a) {
        a.vpsrld(A::ymm15, A::ymm2, 8);
        a.vpsrld(A::ymm0 , A::ymm8, 5);
    },{
        0xc5,     0x85, 0x72,0xd2, 0x08,
        0xc4,0xc1,0x7d, 0x72,0xd0, 0x05,
    });

    test_asm(r, [&](A& a) {
        a.vpermq(A::ymm1, A::ymm2, 5);
    },{
        0xc4,0xe3,0xfd, 0x00,0xca, 0x05,
    });

    test_asm(r, [&](A& a) {
        A::Label l = a.here();
        a.byte(1);
        a.byte(2);
        a.byte(3);
        a.byte(4);

        a.vbroadcastss(A::ymm0 , l);
        a.vbroadcastss(A::ymm1 , l);
        a.vbroadcastss(A::ymm8 , l);
        a.vbroadcastss(A::ymm15, l);

        a.vpshufb(A::ymm4, A::ymm3, l);
    },{
        0x01, 0x02, 0x03, 0x4,

        /*     VEX    */  /*op*/ /*   ModRM    */  /*     offset     */
        0xc4, 0xe2, 0x7d,  0x18,   0b00'000'101,   0xf3,0xff,0xff,0xff,   // 0xfffffff3 == -13
        0xc4, 0xe2, 0x7d,  0x18,   0b00'001'101,   0xea,0xff,0xff,0xff,   // 0xffffffea == -22
        0xc4, 0x62, 0x7d,  0x18,   0b00'000'101,   0xe1,0xff,0xff,0xff,   // 0xffffffe1 == -31
        0xc4, 0x62, 0x7d,  0x18,   0b00'111'101,   0xd8,0xff,0xff,0xff,   // 0xffffffd8 == -40

        0xc4, 0xe2, 0x65,  0x00,   0b00'100'101,   0xcf,0xff,0xff,0xff,   // 0xffffffcf == -49
    });

    test_asm(r, [&](A& a) {
        A::Label l = a.here();
        a.jne(l);
        for (int i = 0; i < 124; i++) {
            a.nop();
        }
        a.jne(l);
        a.jne(l);
    },{
        0x75, 0xfe,  // short jump -2 bytes

        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,

        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,

        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,
        0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90,

        0x90, 0x90, 0x90, 0x90,

        0x75, 0x80,                        // short jump -128 bytes
        0x0f, 0x85, 0x7a,0xff,0xff,0xff,   // near jump back -134 bytes
    });

    test_asm(r, [&](A& a) {
        a.vmovups(A::ymm5, A::rsi);
        a.vmovups(A::rsi, A::ymm5);

        a.vpmovzxbd(A::ymm4, A::rsi);

        a.vmovq(A::rdx, A::xmm15);
    },{
        /*    VEX    */  /*Op*/  /*  ModRM  */
        0xc5,     0xfc,   0x10,  0b00'101'110,
        0xc5,     0xfc,   0x11,  0b00'101'110,

        0xc4,0xe2,0x7d,   0x31,  0b00'100'110,

        0xc5,     0x79,   0xd6,  0b00'111'010,
    });

    test_asm(r, [&](A& a) {
        a.vpandn(A::ymm3, A::ymm12, A::ymm2);
    },{
        0xc5, 0x9d, 0xdf, 0xda,
    });

    // echo "fmul v4.4s, v3.4s, v1.4s" | llvm-mc -show-encoding -arch arm64

    test_asm(r, [&](A& a) {
        a.and16b(A::v4, A::v3, A::v1);
        a.orr16b(A::v4, A::v3, A::v1);
        a.eor16b(A::v4, A::v3, A::v1);
        a.bic16b(A::v4, A::v3, A::v1);

        a.add4s(A::v4, A::v3, A::v1);
        a.sub4s(A::v4, A::v3, A::v1);
        a.mul4s(A::v4, A::v3, A::v1);

        a.sub8h(A::v4, A::v3, A::v1);
        a.mul8h(A::v4, A::v3, A::v1);

        a.fadd4s(A::v4, A::v3, A::v1);
        a.fsub4s(A::v4, A::v3, A::v1);
        a.fmul4s(A::v4, A::v3, A::v1);
        a.fdiv4s(A::v4, A::v3, A::v1);

        a.fmla4s(A::v4, A::v3, A::v1);
    },{
        0x64,0x1c,0x21,0x4e,
        0x64,0x1c,0xa1,0x4e,
        0x64,0x1c,0x21,0x6e,
        0x64,0x1c,0x61,0x4e,

        0x64,0x84,0xa1,0x4e,
        0x64,0x84,0xa1,0x6e,
        0x64,0x9c,0xa1,0x4e,

        0x64,0x84,0x61,0x6e,
        0x64,0x9c,0x61,0x4e,

        0x64,0xd4,0x21,0x4e,
        0x64,0xd4,0xa1,0x4e,
        0x64,0xdc,0x21,0x6e,
        0x64,0xfc,0x21,0x6e,

        0x64,0xcc,0x21,0x4e,
    });

    test_asm(r, [&](A& a) {
        a.shl4s(A::v4, A::v3,  0);
        a.shl4s(A::v4, A::v3,  1);
        a.shl4s(A::v4, A::v3,  8);
        a.shl4s(A::v4, A::v3, 16);
        a.shl4s(A::v4, A::v3, 31);

        a.sshr4s(A::v4, A::v3,  1);
        a.sshr4s(A::v4, A::v3,  8);
        a.sshr4s(A::v4, A::v3, 31);

        a.ushr4s(A::v4, A::v3,  1);
        a.ushr4s(A::v4, A::v3,  8);
        a.ushr4s(A::v4, A::v3, 31);

        a.ushr8h(A::v4, A::v3,  1);
        a.ushr8h(A::v4, A::v3,  8);
        a.ushr8h(A::v4, A::v3, 15);
    },{
        0x64,0x54,0x20,0x4f,
        0x64,0x54,0x21,0x4f,
        0x64,0x54,0x28,0x4f,
        0x64,0x54,0x30,0x4f,
        0x64,0x54,0x3f,0x4f,

        0x64,0x04,0x3f,0x4f,
        0x64,0x04,0x38,0x4f,
        0x64,0x04,0x21,0x4f,

        0x64,0x04,0x3f,0x6f,
        0x64,0x04,0x38,0x6f,
        0x64,0x04,0x21,0x6f,

        0x64,0x04,0x1f,0x6f,
        0x64,0x04,0x18,0x6f,
        0x64,0x04,0x11,0x6f,
    });

    test_asm(r, [&](A& a) {
        a.scvtf4s (A::v4, A::v3);
        a.fcvtzs4s(A::v4, A::v3);
    },{
        0x64,0xd8,0x21,0x4e,
        0x64,0xb8,0xa1,0x4e,
    });

    test_asm(r, [&](A& a) {
        a.ret(A::x30);   // Conventional ret using link register.
        a.ret(A::x13);   // Can really return using any register if we like.

        a.add(A::x2, A::x2,  4);
        a.add(A::x3, A::x2, 32);

        a.subs(A::x2, A::x2,  4);
        a.subs(A::x3, A::x2, 32);

        A::Label l = a.here();
        a.bne(l);
        a.bne(l);
    },{
        0xc0,0x03,0x5f,0xd6,
        0xa0,0x01,0x5f,0xd6,

        0x42,0x10,0x00,0x91,
        0x43,0x80,0x00,0x91,

        0x42,0x10,0x00,0xf1,
        0x43,0x80,0x00,0xf1,

        0x01,0x00,0x00,0x54,
        0xe1,0xff,0xff,0x54,
    });

    test_asm(r, [&](A& a) {
        a.ldrq(A::v0, A::x8);
        a.strq(A::v0, A::x8);
    },{
        0x00, 0x01, 0xc0, 0x3d,
        0x00, 0x01, 0x80, 0x3d,
    });

    test_asm(r, [&](A& a) {
        a.xtns2h(A::v0, A::v0);
        a.xtnh2b(A::v0, A::v0);
        a.strs  (A::v0, A::x0);

        a.ldrs   (A::v0, A::x0);
        a.uxtlb2h(A::v0, A::v0);
        a.uxtlh2s(A::v0, A::v0);
    },{
        0x00,0x28,0x61,0x0e,
        0x00,0x28,0x21,0x0e,
        0x00,0x00,0x00,0xbd,

        0x00,0x00,0x40,0xbd,
        0x00,0xa4,0x08,0x2f,
        0x00,0xa4,0x10,0x2f,
    });
}
