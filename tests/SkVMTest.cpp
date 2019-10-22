/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkCpu.h"
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

static void dump(skvm::Builder& builder, SkWStream* o) {
    skvm::Program program = builder.done();
    builder.dump(o);
    o->writeText("\n");
    program.dump(o);
    o->writeText("\n");
}

// TODO: I'd like this to go away and have every test in here run both JIT and interpreter.
template <typename Fn>
static void test_interpreter_only(skiatest::Reporter* r, skvm::Program&& program, Fn&& test) {
#if defined(SKVM_JIT)
    REPORTER_ASSERT(r, !program.hasJIT());
#endif
    test((const skvm::Program&) program);
}

template <typename Fn>
static void test_jit_and_interpreter(skiatest::Reporter* r, skvm::Program&& program, Fn&& test) {
#if defined(SKVM_JIT)
    const bool expect_jit
    #if defined(SK_CPU_X86)
        = SkCpu::Supports(SkCpu::HSW);
    #elif defined(SK_CPU_ARM64)
        = true;
    #else
        = false;
    #endif
    if (expect_jit) {
        REPORTER_ASSERT(r, program.hasJIT());
        test((const skvm::Program&) program);
        program.dropJIT();
    }
#endif
    test_interpreter_only(r, std::move(program), std::move(test));
}


DEF_TEST(SkVM, r) {
    SkDynamicMemoryWStream buf;

    // Write all combinations of SrcoverBuilder_F32
    for (int s = 0; s < 3; s++)
    for (int d = 0; d < 3; d++) {
        auto srcFmt = (Fmt)s,
             dstFmt = (Fmt)d;
        SrcoverBuilder_F32 builder{srcFmt, dstFmt};

        buf.writeText(fmt_name(srcFmt));
        buf.writeText(" over ");
        buf.writeText(fmt_name(dstFmt));
        buf.writeText("\n");
        dump(builder, &buf);
    }

    // Write the I32 Srcovers also.
    {
        SrcoverBuilder_I32_Naive builder;
        buf.writeText("I32 (Naive) 8888 over 8888\n");
        dump(builder, &buf);
    }
    {
        SrcoverBuilder_I32 builder;
        buf.writeText("I32 8888 over 8888\n");
        dump(builder, &buf);
    }
    {
        SrcoverBuilder_I32_SWAR builder;
        buf.writeText("I32 (SWAR) 8888 over 8888\n");
        dump(builder, &buf);
    }

    {
        skvm::Builder b;
        skvm::Arg arg = b.varying<int>();

        // x and y can both be hoisted,
        // and x can die at y, while y must live for the loop.
        skvm::I32 x = b.splat(1),
                  y = b.add(x, b.splat(2));
        b.store32(arg, b.mul(b.load32(arg), y));

        skvm::Program program = b.done();
        REPORTER_ASSERT(r, program.nregs() == 2);

        std::vector<skvm::Builder::Instruction> insts = b.program();
        REPORTER_ASSERT(r, insts.size() == 6);
        REPORTER_ASSERT(r,  insts[0].can_hoist && insts[0].death == 2 && !insts[0].used_in_loop);
        REPORTER_ASSERT(r,  insts[1].can_hoist && insts[1].death == 2 && !insts[1].used_in_loop);
        REPORTER_ASSERT(r,  insts[2].can_hoist && insts[2].death == 4 &&  insts[2].used_in_loop);
        REPORTER_ASSERT(r, !insts[3].can_hoist);
        REPORTER_ASSERT(r, !insts[4].can_hoist);
        REPORTER_ASSERT(r, !insts[5].can_hoist);

        dump(b, &buf);

        test_jit_and_interpreter(r, std::move(program), [&](const skvm::Program& program) {
            int arg[] = {0,1,2,3,4,5,6,7,8,9};

            program.eval(SK_ARRAY_COUNT(arg), arg);

            for (int i = 0; i < (int)SK_ARRAY_COUNT(arg); i++) {
                REPORTER_ASSERT(r, arg[i] == i*3);
            }
        });
    }

    {
        // Demonstrate the value of program reordering.
        skvm::Builder b;
        skvm::Arg sp = b.varying<int>(),
                  dp = b.varying<int>();

        skvm::I32 byte = b.splat(0xff);

        skvm::I32 src = b.load32(sp),
                  sr  = b.extract(src,  0, byte),
                  sg  = b.extract(src,  8, byte),
                  sb  = b.extract(src, 16, byte),
                  sa  = b.extract(src, 24, byte);

        skvm::I32 dst = b.load32(dp),
                  dr  = b.extract(dst,  0, byte),
                  dg  = b.extract(dst,  8, byte),
                  db  = b.extract(dst, 16, byte),
                  da  = b.extract(dst, 24, byte);

        skvm::I32 R = b.add(sr, dr),
                  G = b.add(sg, dg),
                  B = b.add(sb, db),
                  A = b.add(sa, da);

        skvm::I32 rg = b.pack(R, G, 8),
                  ba = b.pack(B, A, 8),
                  rgba = b.pack(rg, ba, 16);

        b.store32(dp, rgba);

        dump(b, &buf);
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

    auto test_8888 = [&](skvm::Program&& program) {
        uint32_t src[9];
        uint32_t dst[SK_ARRAY_COUNT(src)];

        test_jit_and_interpreter(r, std::move(program), [&](const skvm::Program& program) {
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
                    if (abs(d-w) >= 2) {
                        SkDebugf("d %02x, w %02x\n", d,w);
                    }
                    REPORTER_ASSERT(r, abs(d-w) < 2);
                    got  >>= 8;
                    want >>= 8;
                }
            }
        });
    };

    test_8888(SrcoverBuilder_F32{Fmt::RGBA_8888, Fmt::RGBA_8888}.done("srcover_f32"));
    test_8888(SrcoverBuilder_I32_Naive{}.done("srcover_i32_naive"));
    test_8888(SrcoverBuilder_I32{}.done("srcover_i32"));
    test_8888(SrcoverBuilder_I32_SWAR{}.done("srcover_i32_SWAR"));

    test_jit_and_interpreter(r, SrcoverBuilder_F32{Fmt::RGBA_8888, Fmt::G8}.done(),
                             [&](const skvm::Program& program) {
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
    });

    test_jit_and_interpreter(r, SrcoverBuilder_F32{Fmt::A8, Fmt::A8}.done(),
                             [&](const skvm::Program& program) {
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
    });
}

DEF_TEST(SkVM_Pointless, r) {
    // Let's build a program with no memory arguments.
    // It should all be pegged as dead code, but we should be able to "run" it.
    skvm::Builder b;
    {
        b.add(b.splat(5.0f),
              b.splat(4.0f));
    }

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        for (int N = 0; N < 64; N++) {
            program.eval(N);
        }
    });

    for (const skvm::Builder::Instruction& inst : b.program()) {
        REPORTER_ASSERT(r, inst.death == 0 && inst.can_hoist == true);
    }
}

DEF_TEST(SkVM_LoopCounts, r) {
    // Make sure we cover all the exact N we want.

    // buf[i] += 1
    skvm::Builder b;
    skvm::Arg arg = b.varying<int>();
    b.store32(arg,
              b.add(b.splat(1),
                    b.load32(arg)));

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        int buf[64];
        for (int N = 0; N <= (int)SK_ARRAY_COUNT(buf); N++) {
            for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
                buf[i] = i;
            }
            program.eval(N, buf);

            for (int i = 0; i < N; i++) {
                REPORTER_ASSERT(r, buf[i] == i+1);
            }
            for (int i = N; i < (int)SK_ARRAY_COUNT(buf); i++) {
                REPORTER_ASSERT(r, buf[i] == i);
            }
        }
    });
}

DEF_TEST(SkVM_gathers, r) {
    skvm::Builder b;
    {
        skvm::Arg img   = b.uniform(),
                  buf32 = b.varying<int>(),
                  buf16 = b.varying<uint16_t>(),
                  buf8  = b.varying<uint8_t>();

        skvm::I32 x = b.load32(buf32);

        b.store32(buf32, b.gather32(img, b.bit_and(x, b.splat( 7))));
        b.store16(buf16, b.gather16(img, b.bit_and(x, b.splat(15))));
        b.store8 (buf8 , b.gather8 (img, b.bit_and(x, b.splat(31))));
    }

    test_interpreter_only(r, b.done(), [&](const skvm::Program& program) {
        const int img[] = {12,34,56,78, 90,98,76,54};

        constexpr int N = 20;
        int      buf32[N];
        uint16_t buf16[N];
        uint8_t  buf8 [N];

        for (int i = 0; i < 20; i++) {
            buf32[i] = i;
        }

        program.eval(N, img, buf32, buf16, buf8);
        int i = 0;
        REPORTER_ASSERT(r, buf32[i] == 12 && buf16[i] == 12 && buf8[i] == 12); i++;
        REPORTER_ASSERT(r, buf32[i] == 34 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 56 && buf16[i] == 34 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 78 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 90 && buf16[i] == 56 && buf8[i] == 34); i++;
        REPORTER_ASSERT(r, buf32[i] == 98 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 76 && buf16[i] == 78 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 54 && buf16[i] ==  0 && buf8[i] ==  0); i++;

        REPORTER_ASSERT(r, buf32[i] == 12 && buf16[i] == 90 && buf8[i] == 56); i++;
        REPORTER_ASSERT(r, buf32[i] == 34 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 56 && buf16[i] == 98 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 78 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 90 && buf16[i] == 76 && buf8[i] == 78); i++;
        REPORTER_ASSERT(r, buf32[i] == 98 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 76 && buf16[i] == 54 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 54 && buf16[i] ==  0 && buf8[i] ==  0); i++;

        REPORTER_ASSERT(r, buf32[i] == 12 && buf16[i] == 12 && buf8[i] == 90); i++;
        REPORTER_ASSERT(r, buf32[i] == 34 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 56 && buf16[i] == 34 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 78 && buf16[i] ==  0 && buf8[i] ==  0); i++;
    });
}

DEF_TEST(SkVM_bitops, r) {
    skvm::Builder b;
    {
        skvm::Arg ptr = b.varying<int>();

        skvm::I32 x = b.load32(ptr);

        x = b.bit_and  (x, b.splat(0xf1));  // 0x40
        x = b.bit_or   (x, b.splat(0x80));  // 0xc0
        x = b.bit_xor  (x, b.splat(0xfe));  // 0x3e
        x = b.bit_clear(x, b.splat(0x30));  // 0x0e

        x = b.shl(x, 28);  // 0xe000'0000
        x = b.sra(x, 28);  // 0xffff'fffe
        x = b.shr(x,  1);  // 0x7fff'ffff

        b.store32(ptr, x);
    }

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        int x = 0x42;
        program.eval(1, &x);
        REPORTER_ASSERT(r, x == 0x7fff'ffff);
    });
}

DEF_TEST(SkVM_f32, r) {
    skvm::Builder b;
    {
        skvm::Arg arg = b.varying<float>();

        skvm::F32 x = b.bit_cast(b.load32(arg)),
                  y = b.add(x,x),   // y = 2x
                  z = b.sub(y,x),   // z = 2x-x = x
                  w = b.div(z,x);   // w = x/x = 1
        b.store32(arg, b.bit_cast(w));
    }

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        float buf[] = { 1,2,3,4,5,6,7,8,9 };
        program.eval(SK_ARRAY_COUNT(buf), buf);
        for (float v : buf) {
            REPORTER_ASSERT(r, v == 1.0f);
        }
    });
}

DEF_TEST(SkVM_cmp_i32, r) {
    skvm::Builder b;
    {
        skvm::I32 x = b.load32(b.varying<int>());

        auto to_bit = [&](int shift, skvm::I32 mask) {
            return b.shl(b.bit_and(mask, b.splat(0x1)), shift);
        };

        skvm::I32 m = b.splat(0);
        m = b.bit_or(m, to_bit(0, b. eq(x, b.splat(0))));
        m = b.bit_or(m, to_bit(1, b.neq(x, b.splat(1))));
        m = b.bit_or(m, to_bit(2, b. lt(x, b.splat(2))));
        m = b.bit_or(m, to_bit(3, b.lte(x, b.splat(3))));
        m = b.bit_or(m, to_bit(4, b. gt(x, b.splat(4))));
        m = b.bit_or(m, to_bit(5, b.gte(x, b.splat(5))));

        b.store32(b.varying<int>(), m);
    }

    test_interpreter_only(r, b.done(), [&](const skvm::Program& program) {
        int in[] = { 0,1,2,3,4,5,6,7,8,9 };
        int out[SK_ARRAY_COUNT(in)];

        program.eval(SK_ARRAY_COUNT(in), in, out);

        REPORTER_ASSERT(r, out[0] == 0b001111);
        REPORTER_ASSERT(r, out[1] == 0b001100);
        REPORTER_ASSERT(r, out[2] == 0b001010);
        REPORTER_ASSERT(r, out[3] == 0b001010);
        REPORTER_ASSERT(r, out[4] == 0b000010);
        for (int i = 5; i < (int)SK_ARRAY_COUNT(out); i++) {
            REPORTER_ASSERT(r, out[i] == 0b110010);
        }
    });
}

DEF_TEST(SkVM_cmp_f32, r) {
    skvm::Builder b;
    {
        skvm::F32 x = b.bit_cast(b.load32(b.varying<float>()));

        auto to_bit = [&](int shift, skvm::I32 mask) {
            return b.shl(b.bit_and(mask, b.splat(0x1)), shift);
        };

        skvm::I32 m = b.splat(0);
        m = b.bit_or(m, to_bit(0, b. eq(x, b.splat(0.0f))));
        m = b.bit_or(m, to_bit(1, b.neq(x, b.splat(1.0f))));
        m = b.bit_or(m, to_bit(2, b. lt(x, b.splat(2.0f))));
        m = b.bit_or(m, to_bit(3, b.lte(x, b.splat(3.0f))));
        m = b.bit_or(m, to_bit(4, b. gt(x, b.splat(4.0f))));
        m = b.bit_or(m, to_bit(5, b.gte(x, b.splat(5.0f))));

        b.store32(b.varying<int>(), m);
    }

    test_interpreter_only(r, b.done(), [&](const skvm::Program& program) {
        float in[] = { 0,1,2,3,4,5,6,7,8,9 };
        int out[SK_ARRAY_COUNT(in)];

        program.eval(SK_ARRAY_COUNT(in), in, out);

        REPORTER_ASSERT(r, out[0] == 0b001111);
        REPORTER_ASSERT(r, out[1] == 0b001100);
        REPORTER_ASSERT(r, out[2] == 0b001010);
        REPORTER_ASSERT(r, out[3] == 0b001010);
        REPORTER_ASSERT(r, out[4] == 0b000010);
        for (int i = 5; i < (int)SK_ARRAY_COUNT(out); i++) {
            REPORTER_ASSERT(r, out[i] == 0b110010);
        }
    });
}

DEF_TEST(SkVM_i16x2, r) {
    skvm::Builder b;
    {
        skvm::Arg buf = b.varying<int>();

        skvm::I32 x = b.load32(buf),
                  y = b.add_16x2(x,x),   // y = 2x
                  z = b.mul_16x2(x,y),   // z = 2x^2
                  w = b.sub_16x2(z,x),   // w = x(2x-1)
                  v = b.shl_16x2(w,7),   // These shifts will be a no-op
                  u = b.sra_16x2(v,7);   // for all but x=12 and x=13.
        b.store32(buf, u);
    }

    test_interpreter_only(r, b.done(), [&](const skvm::Program& program) {
        uint16_t buf[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };

        program.eval(SK_ARRAY_COUNT(buf)/2, buf);
        for (int i = 0; i < 12; i++) {
            REPORTER_ASSERT(r, buf[i] == i*(2*i-1));
        }
        REPORTER_ASSERT(r, buf[12] == 0xff14);   // 12*23 = 0x114
        REPORTER_ASSERT(r, buf[13] == 0xff45);   // 13*25 = 0x145
    });
}

DEF_TEST(SkVM_cmp_i16, r) {
    skvm::Builder b;
    {
        skvm::Arg buf = b.varying<int>();
        skvm::I32 x = b.load32(buf);

        auto to_bit = [&](int shift, skvm::I32 mask) {
            return b.shl_16x2(b.bit_and(mask, b.splat(0x0001'0001)), shift);
        };

        skvm::I32 m = b.splat(0);
        m = b.bit_or(m, to_bit(0, b. eq_16x2(x, b.splat(0x0000'0000))));
        m = b.bit_or(m, to_bit(1, b.neq_16x2(x, b.splat(0x0001'0001))));
        m = b.bit_or(m, to_bit(2, b. lt_16x2(x, b.splat(0x0002'0002))));
        m = b.bit_or(m, to_bit(3, b.lte_16x2(x, b.splat(0x0003'0003))));
        m = b.bit_or(m, to_bit(4, b. gt_16x2(x, b.splat(0x0004'0004))));
        m = b.bit_or(m, to_bit(5, b.gte_16x2(x, b.splat(0x0005'0005))));

        b.store32(buf, m);
    }

    test_interpreter_only(r, b.done(), [&](const skvm::Program& program) {
        int16_t buf[] = { 0,1, 2,3, 4,5, 6,7, 8,9 };

        program.eval(SK_ARRAY_COUNT(buf)/2, buf);

        REPORTER_ASSERT(r, buf[0] == 0b001111);
        REPORTER_ASSERT(r, buf[1] == 0b001100);
        REPORTER_ASSERT(r, buf[2] == 0b001010);
        REPORTER_ASSERT(r, buf[3] == 0b001010);
        REPORTER_ASSERT(r, buf[4] == 0b000010);
        for (int i = 5; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == 0b110010);
        }
    });
}


DEF_TEST(SkVM_mad, r) {
    // This program is designed to exercise the tricky corners of instruction
    // and register selection for Op::mad_f32.

    skvm::Builder b;
    {
        skvm::Arg arg = b.varying<int>();

        skvm::F32 x = b.to_f32(b.load32(arg)),
                  y = b.mad(x,x,x),   // x is needed in the future, so r[x] != r[y].
                  z = b.mad(y,y,x),   // y is needed in the future, but r[z] = r[x] is ok.
                  w = b.mad(z,z,y),   // w can alias z but not y.
                  v = b.mad(w,y,w);   // Got to stop somewhere.
        b.store32(arg, b.to_i32(v));
    }

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        int x = 2;
        program.eval(1, &x);
        // x = 2
        // y = 2*2 + 2 = 6
        // z = 6*6 + 2 = 38
        // w = 38*38 + 6 = 1450
        // v = 1450*6 + 1450 = 10150
        REPORTER_ASSERT(r, x == 10150);
    });
}

DEF_TEST(SkVM_madder, r) {
    skvm::Builder b;
    {
        skvm::Arg arg = b.varying<float>();

        skvm::F32 x = b.bit_cast(b.load32(arg)),
                  y = b.mad(x,x,x),   // x is needed in the future, so r[x] != r[y].
                  z = b.mad(y,x,y),   // r[x] can be reused after this instruction, but not r[y].
                  w = b.mad(y,y,z);
        b.store32(arg, b.bit_cast(w));
    }

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        float x = 2.0f;
        // y = 2*2 + 2 = 6
        // z = 6*2 + 6 = 18
        // w = 6*6 + 18 = 54
        program.eval(1, &x);
        REPORTER_ASSERT(r, x == 54.0f);
    });
}

DEF_TEST(SkVM_hoist, r) {
    // This program uses enough constants that it will fail to JIT if we hoist them.
    // The JIT will try again without hoisting, and that'll just need 2 registers.
    skvm::Builder b;
    {
        skvm::Arg arg = b.varying<int>();
        skvm::I32 x = b.load32(arg);
        for (int i = 0; i < 32; i++) {
            x = b.add(x, b.splat(i));
        }
        b.store32(arg, x);
    }

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        int x = 4;
        program.eval(1, &x);
        // x += 0 + 1 + 2 + 3 + ... + 30 + 31
        // x += 496
        REPORTER_ASSERT(r, x == 500);
    });
}

DEF_TEST(SkVM_select, r) {
    skvm::Builder b;
    {
        skvm::Arg buf = b.varying<int>();

        skvm::I32 x = b.load32(buf);

        x = b.select( b.gt(x, b.splat(4)), x, b.splat(42) );

        b.store32(buf, x);
    }

    test_jit_and_interpreter(r, b.done(), [&](const skvm::Program& program) {
        int buf[] = { 0,1,2,3,4,5,6,7,8 };
        program.eval(SK_ARRAY_COUNT(buf), buf);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == (i > 4 ? i : 42));
        }
    });
}

DEF_TEST(SkVM_NewOps, r) {
    // Exercise a somewhat arbitrary set of new ops.
    skvm::Builder b;
    {
        skvm::Arg buf      = b.varying<int16_t>(),
                  img      = b.uniform(),
                  uniforms = b.uniform();

        skvm::I32 x = b.load16(buf);

        x = b.add(x, b.uniform32(uniforms, 0));
        x = b.mul(x, b.uniform8 (uniforms, 4));
        x = b.sub(x, b.uniform16(uniforms, 6));

        skvm::I32 limit = b.uniform32(uniforms, 8);
        x = b.select(b.lt(x, b.splat(0)), b.splat(0), x);
        x = b.select(b.gt(x, limit     ), limit     , x);

        x = b.gather8(img, x);

        b.store16(buf, x);
    }

    if ((false)) {
        SkDynamicMemoryWStream buf;
        dump(b, &buf);
        sk_sp<SkData> blob = buf.detachAsData();
        SkDebugf("%.*s\n", blob->size(), blob->data());
    }

    test_interpreter_only(r, b.done(), [&](const skvm::Program& program) {
        const int N = 31;
        int16_t buf[N];
        for (int i = 0; i < N; i++) {
            buf[i] = i;
        }

        const int M = 16;
        uint8_t img[M];
        for (int i = 0; i < M; i++) {
            img[i] = i*i;
        }

        struct {
            int      add   = 5;
            uint8_t  mul   = 3;
            uint16_t sub   = 18;
            int      limit = M-1;
        } uniforms;

        program.eval(N, buf, img, &uniforms);

        for (int i = 0; i < N; i++) {
            // Our first math calculates x = (i+5)*3 - 18 a.k.a 3*(i-1).
            int x = 3*(i-1);

            // Then that's pinned to the limits of img.
            if (i < 2) { x =  0; }  // Notice i == 1 hits x == 0 exactly...
            if (i > 5) { x = 15; }  // ...and i == 6 hits x == 15 exactly
            REPORTER_ASSERT(r, buf[i] == img[x]);
        }
    });
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

    // Align should pad with zero
    test_asm(r, [&](A& a) {
        a.ret();
        a.align(4);
    },{
        0xc3,
        0x00, 0x00, 0x00,
    });

    test_asm(r, [&](A& a) {
        a.add(A::rax, 8);       // Always good to test rax.
        a.sub(A::rax, 32);

        a.add(A::rdi, 12);      // Last 0x48 REX
        a.sub(A::rdi, 8);

        a.add(A::r8 , 7);       // First 0x49 REX
        a.sub(A::r8 , 4);

        a.add(A::rsi, 128);     // Requires 4 byte immediate.
        a.sub(A::r8 , 1000000);
    },{
        0x48, 0x83, 0b11'000'000, 0x08,
        0x48, 0x83, 0b11'101'000, 0x20,

        0x48, 0x83, 0b11'000'111, 0x0c,
        0x48, 0x83, 0b11'101'111, 0x08,

        0x49, 0x83, 0b11'000'000, 0x07,
        0x49, 0x83, 0b11'101'000, 0x04,

        0x48, 0x81, 0b11'000'110, 0x80, 0x00, 0x00, 0x00,
        0x49, 0x81, 0b11'101'000, 0x40, 0x42, 0x0f, 0x00,
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
        a.vpcmpeqd(A::ymm0, A::ymm1, A::ymm2);
        a.vpcmpgtd(A::ymm0, A::ymm1, A::ymm2);
    },{
        0xc5,0xf5,0x76,0xc2,
        0xc5,0xf5,0x66,0xc2,
    });

    test_asm(r, [&](A& a) {
        a.vpblendvb(A::ymm0, A::ymm1, A::ymm2, A::ymm3);
    },{
        0xc4,0xe3,0x75, 0x4c, 0xc2, 0x30,
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

        a.vbroadcastss(A::ymm0 , &l);
        a.vbroadcastss(A::ymm1 , &l);
        a.vbroadcastss(A::ymm8 , &l);
        a.vbroadcastss(A::ymm15, &l);

        a.vpshufb(A::ymm4, A::ymm3, &l);
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
        a.vbroadcastss(A::ymm0,  A::rdi,   0);
        a.vbroadcastss(A::ymm13, A::r14,   7);
        a.vbroadcastss(A::ymm8,  A::rdx, -12);
        a.vbroadcastss(A::ymm8,  A::rdx, 400);

        a.vbroadcastss(A::ymm8,  A::xmm0);
        a.vbroadcastss(A::ymm0,  A::xmm13);
    },{
        /*   VEX    */ /*op*/     /*ModRM*/   /*offset*/
        0xc4,0xe2,0x7d, 0x18,   0b00'000'111,
        0xc4,0x42,0x7d, 0x18,   0b01'101'110,  0x07,
        0xc4,0x62,0x7d, 0x18,   0b01'000'010,  0xf4,
        0xc4,0x62,0x7d, 0x18,   0b10'000'010,  0x90,0x01,0x00,0x00,

        0xc4,0x62,0x7d, 0x18,   0b11'000'000,
        0xc4,0xc2,0x7d, 0x18,   0b11'000'101,
    });

    test_asm(r, [&](A& a) {
        A::Label l = a.here();
        a.jne(&l);
        a.jne(&l);
        a.je (&l);
        a.jmp(&l);
        a.jl (&l);

        a.cmp(A::rdx, 0);
        a.cmp(A::rax, 12);
        a.cmp(A::r14, 2000000000);
    },{
        0x0f,0x85, 0xfa,0xff,0xff,0xff,   // near jne -6 bytes
        0x0f,0x85, 0xf4,0xff,0xff,0xff,   // near jne -12 bytes
        0x0f,0x84, 0xee,0xff,0xff,0xff,   // near je  -18 bytes
        0xe9,      0xe9,0xff,0xff,0xff,   // near jmp -23 bytes
        0x0f,0x8c, 0xe3,0xff,0xff,0xff,   // near jl  -29 bytes

        0x48,0x83,0xfa,0x00,
        0x48,0x83,0xf8,0x0c,
        0x49,0x81,0xfe,0x00,0x94,0x35,0x77,
    });

    test_asm(r, [&](A& a) {
        a.vmovups(A::ymm5, A::rsi);
        a.vmovups(A::rsi, A::ymm5);

        a.vmovups(A::rsi, A::xmm5);

        a.vpmovzxwd(A::ymm4, A::rsi);
        a.vpmovzxbd(A::ymm4, A::rsi);

        a.vmovq(A::rdx, A::xmm15);
    },{
        /*    VEX    */  /*Op*/  /*  ModRM  */
        0xc5,     0xfc,   0x10,  0b00'101'110,
        0xc5,     0xfc,   0x11,  0b00'101'110,

        0xc5,     0xf8,   0x11,  0b00'101'110,

        0xc4,0xe2,0x7d,   0x33,  0b00'100'110,
        0xc4,0xe2,0x7d,   0x31,  0b00'100'110,

        0xc5,     0x79,   0xd6,  0b00'111'010,
    });

    test_asm(r, [&](A& a) {
        a.movzbl(A::rax, A::rsi, 0);   // Low registers for src and dst.
        a.movzbl(A::rax, A::r8,  0);   // High src register.
        a.movzbl(A::r8 , A::rsi, 0);   // High dst register.
        a.movzbl(A::r8,  A::rsi, 12);
        a.movzbl(A::r8,  A::rsi, 400);

        a.vmovd(A::rax, A::xmm0);
        a.vmovd(A::rax, A::xmm8);
        a.vmovd(A::r8,  A::xmm0);

        a.vmovd(A::xmm0, A::rax);
        a.vmovd(A::xmm8, A::rax);
        a.vmovd(A::xmm0, A::r8);

        a.vmovd_direct(A::rax, A::xmm0);
        a.vmovd_direct(A::rax, A::xmm8);
        a.vmovd_direct(A::r8,  A::xmm0);

        a.vmovd_direct(A::xmm0, A::rax);
        a.vmovd_direct(A::xmm8, A::rax);
        a.vmovd_direct(A::xmm0, A::r8);

        a.movb(A::rdx, A::rax);
        a.movb(A::rdx, A::r8);
        a.movb(A::r8 , A::rax);
    },{
        0x0f,0xb6,0x06,
        0x41,0x0f,0xb6,0x00,
        0x44,0x0f,0xb6,0x06,
        0x44,0x0f,0xb6,0x46, 12,
        0x44,0x0f,0xb6,0x86, 0x90,0x01,0x00,0x00,

        0xc5,0xf9,0x7e,0x00,
        0xc5,0x79,0x7e,0x00,
        0xc4,0xc1,0x79,0x7e,0x00,

        0xc5,0xf9,0x6e,0x00,
        0xc5,0x79,0x6e,0x00,
        0xc4,0xc1,0x79,0x6e,0x00,

        0xc5,0xf9,0x7e,0xc0,
        0xc5,0x79,0x7e,0xc0,
        0xc4,0xc1,0x79,0x7e,0xc0,

        0xc5,0xf9,0x6e,0xc0,
        0xc5,0x79,0x6e,0xc0,
        0xc4,0xc1,0x79,0x6e,0xc0,

        0x88, 0x02,
        0x44, 0x88, 0x02,
        0x41, 0x88, 0x00,
    });

    test_asm(r, [&](A& a) {
        a.vpinsrw(A::xmm1, A::xmm8, A::rsi, 4);
        a.vpinsrw(A::xmm8, A::xmm1, A::r8, 12);

        a.vpinsrb(A::xmm1, A::xmm8, A::rsi, 4);
        a.vpinsrb(A::xmm8, A::xmm1, A::r8, 12);

        a.vpextrw(A::rsi, A::xmm8, 7);
        a.vpextrw(A::r8,  A::xmm1, 15);

        a.vpextrb(A::rsi, A::xmm8, 7);
        a.vpextrb(A::r8,  A::xmm1, 15);
    },{
        0xc5,0xb9,      0xc4, 0x0e,  4,
        0xc4,0x41,0x71, 0xc4, 0x00, 12,

        0xc4,0xe3,0x39, 0x20, 0x0e,  4,
        0xc4,0x43,0x71, 0x20, 0x00, 12,

        0xc4,0x63,0x79, 0x15, 0x06,  7,
        0xc4,0xc3,0x79, 0x15, 0x08, 15,

        0xc4,0x63,0x79, 0x14, 0x06,  7,
        0xc4,0xc3,0x79, 0x14, 0x08, 15,
    });

    test_asm(r, [&](A& a) {
        a.vpandn(A::ymm3, A::ymm12, A::ymm2);
    },{
        0xc5, 0x9d, 0xdf, 0xda,
    });

    test_asm(r, [&](A& a) {
        a.vmovdqa   (A::ymm3, A::ymm2);
        a.vcvttps2dq(A::ymm3, A::ymm2);
        a.vcvtdq2ps (A::ymm3, A::ymm2);
    },{
        0xc5,0xfd,0x6f,0xda,
        0xc5,0xfe,0x5b,0xda,
        0xc5,0xfc,0x5b,0xda,
    });

    // echo "fmul v4.4s, v3.4s, v1.4s" | llvm-mc -show-encoding -arch arm64

    test_asm(r, [&](A& a) {
        a.and16b(A::v4, A::v3, A::v1);
        a.orr16b(A::v4, A::v3, A::v1);
        a.eor16b(A::v4, A::v3, A::v1);
        a.bic16b(A::v4, A::v3, A::v1);
        a.bsl16b(A::v4, A::v3, A::v1);

        a.add4s(A::v4, A::v3, A::v1);
        a.sub4s(A::v4, A::v3, A::v1);
        a.mul4s(A::v4, A::v3, A::v1);

        a.cmeq4s(A::v4, A::v3, A::v1);
        a.cmgt4s(A::v4, A::v3, A::v1);

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
        0x64,0x1c,0x61,0x6e,

        0x64,0x84,0xa1,0x4e,
        0x64,0x84,0xa1,0x6e,
        0x64,0x9c,0xa1,0x4e,

        0x64,0x8c,0xa1,0x6e,
        0x64,0x34,0xa1,0x4e,

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
        a.sli4s(A::v4, A::v3,  0);
        a.sli4s(A::v4, A::v3,  1);
        a.sli4s(A::v4, A::v3,  8);
        a.sli4s(A::v4, A::v3, 16);
        a.sli4s(A::v4, A::v3, 31);
    },{
        0x64,0x54,0x20,0x6f,
        0x64,0x54,0x21,0x6f,
        0x64,0x54,0x28,0x6f,
        0x64,0x54,0x30,0x6f,
        0x64,0x54,0x3f,0x6f,
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

        a.sub(A::x2, A::x2, 4);
        a.sub(A::x3, A::x2, 32);

        a.subs(A::x2, A::x2,  4);
        a.subs(A::x3, A::x2, 32);

        a.subs(A::xzr, A::x2, 4);  // These are actually the same instruction!
        a.cmp(A::x2, 4);

        A::Label l = a.here();
        a.bne(&l);
        a.bne(&l);
        a.blt(&l);
        a.b(&l);
        a.cbnz(A::x2, &l);
        a.cbz(A::x2, &l);
    },{
        0xc0,0x03,0x5f,0xd6,
        0xa0,0x01,0x5f,0xd6,

        0x42,0x10,0x00,0x91,
        0x43,0x80,0x00,0x91,

        0x42,0x10,0x00,0xd1,
        0x43,0x80,0x00,0xd1,

        0x42,0x10,0x00,0xf1,
        0x43,0x80,0x00,0xf1,

        0x5f,0x10,0x00,0xf1,
        0x5f,0x10,0x00,0xf1,

        0x01,0x00,0x00,0x54,   // b.ne #0
        0xe1,0xff,0xff,0x54,   // b.ne #-4
        0xcb,0xff,0xff,0x54,   // b.lt #-8
        0xae,0xff,0xff,0x54,   // b.al #-12
        0x82,0xff,0xff,0xb5,   // cbnz x2, #-16
        0x62,0xff,0xff,0xb4,   // cbz x2, #-20
    });

    // Can we cbz() to a not-yet-defined label?
    test_asm(r, [&](A& a) {
        A::Label l;
        a.cbz(A::x2, &l);
        a.add(A::x3, A::x2, 32);
        a.label(&l);
        a.ret(A::x30);
    },{
        0x42,0x00,0x00,0xb4,  // cbz x2, #8
        0x43,0x80,0x00,0x91,  // add x3, x2, #32
        0xc0,0x03,0x5f,0xd6,  // ret
    });

    // If we start a label as a backward label,
    // can we redefine it to be a future label?
    // (Not sure this is useful... just want to test it works.)
    test_asm(r, [&](A& a) {
        A::Label l1 = a.here();
        a.add(A::x3, A::x2, 32);
        a.cbz(A::x2, &l1);          // This will jump backward... nothing sneaky.

        A::Label l2 = a.here();     // Start off the same...
        a.add(A::x3, A::x2, 32);
        a.cbz(A::x2, &l2);          // Looks like this will go backward...
        a.add(A::x2, A::x2, 4);
        a.add(A::x3, A::x2, 32);
        a.label(&l2);               // But no... actually forward!  What a switcheroo!
    },{
        0x43,0x80,0x00,0x91,  // add x3, x2, #32
        0xe2,0xff,0xff,0xb4,  // cbz x2, #-4

        0x43,0x80,0x00,0x91,  // add x3, x2, #32
        0x62,0x00,0x00,0xb4,  // cbz x2, #12
        0x42,0x10,0x00,0x91,  // add x2, x2, #4
        0x43,0x80,0x00,0x91,  // add x3, x2, #32
    });

    // Loading from a label on ARM.
    test_asm(r, [&](A& a) {
        A::Label fore,aft;
        a.label(&fore);
        a.word(0x01234567);
        a.ldrq(A::v1, &fore);
        a.ldrq(A::v2, &aft);
        a.label(&aft);
        a.word(0x76543210);
    },{
        0x67,0x45,0x23,0x01,
        0xe1,0xff,0xff,0x9c,  // ldr q1, #-4
        0x22,0x00,0x00,0x9c,  // ldr q2, #4
        0x10,0x32,0x54,0x76,
    });

    test_asm(r, [&](A& a) {
        a.ldrq(A::v0, A::x8);
        a.strq(A::v0, A::x8);
    },{
        0x00,0x01,0xc0,0x3d,
        0x00,0x01,0x80,0x3d,
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

    test_asm(r, [&](A& a) {
        a.ldrb(A::v0, A::x8);
        a.strb(A::v0, A::x8);
    },{
        0x00,0x01,0x40,0x3d,
        0x00,0x01,0x00,0x3d,
    });

    test_asm(r, [&](A& a) {
        a.tbl(A::v0, A::v1, A::v2);
    },{
        0x20,0x00,0x02,0x4e,
    });
}
