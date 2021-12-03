/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkCpu.h"
#include "src/core/SkMSAN.h"
#include "src/core/SkVM.h"
#include "tests/Test.h"

template <typename Fn>
static void test_jit_and_interpreter(const skvm::Builder& b, Fn&& test) {
    skvm::Program p = b.done();
    test(p);
    if (p.hasJIT()) {
        test(b.done(/*debug_name=*/nullptr, /*allow_jit=*/false));
    }
}

DEF_TEST(SkVM_eliminate_dead_code, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();
        skvm::I32 l = b.load32(arg);
        skvm::I32 a = b.add(l, l);
        b.add(a, b.splat(7));
    }

    std::vector<skvm::Instruction> program = b.program();
    REPORTER_ASSERT(r, program.size() == 4);

    program = skvm::eliminate_dead_code(program);
    REPORTER_ASSERT(r, program.size() == 0);
}

DEF_TEST(SkVM_Pointless, r) {
    // Let's build a program with no memory arguments.
    // It should all be pegged as dead code, but we should be able to "run" it.
    skvm::Builder b;
    {
        b.add(b.splat(5.0f),
              b.splat(4.0f));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        for (int N = 0; N < 64; N++) {
            program.eval(N);
        }
    });

    for (const skvm::OptimizedInstruction& inst : b.optimize()) {
        REPORTER_ASSERT(r, inst.death == 0 && inst.can_hoist == true);
    }
}

DEF_TEST(SkVM_memset, r) {
    skvm::Builder b;
    b.store32(b.varying<int>(), b.splat(42));

    test_jit_and_interpreter(b, [&](const skvm::Program& p) {
        int buf[18];
        buf[17] = 47;

        p.eval(17, buf);
        for (int i = 0; i < 17; i++) {
            REPORTER_ASSERT(r, buf[i] == 42);
        }
        REPORTER_ASSERT(r, buf[17] == 47);
    });
}

DEF_TEST(SkVM_memcpy, r) {
    skvm::Builder b;
    {
        auto src = b.varying<int>(),
             dst = b.varying<int>();
        b.store32(dst, b.load32(src));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& p) {
        int src[] = {1,2,3,4,5,6,7,8,9},
            dst[] = {0,0,0,0,0,0,0,0,0};

        p.eval(SK_ARRAY_COUNT(src)-1, src, dst);
        for (size_t i = 0; i < SK_ARRAY_COUNT(src)-1; i++) {
            REPORTER_ASSERT(r, dst[i] == src[i]);
        }
        size_t i = SK_ARRAY_COUNT(src)-1;
        REPORTER_ASSERT(r, dst[i] == 0);
    });
}

DEF_TEST(SkVM_allow_jit, r) {
    skvm::Builder b;
    {
        auto src = b.varying<int>(),
             dst = b.varying<int>();
        b.store32(dst, b.load32(src));
    }

    if (b.done("test-allow_jit", /*allow_jit=*/true).hasJIT()) {
        REPORTER_ASSERT(r, !b.done("", false).hasJIT());
    }
}

DEF_TEST(SkVM_LoopCounts, r) {
    // Make sure we cover all the exact N we want.

    // buf[i] += 1
    skvm::Builder b;
    skvm::Ptr arg = b.varying<int>();
    b.store32(arg,
              b.add(b.splat(1),
                    b.load32(arg)));

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
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

DEF_TEST(SkVM_gather32, r) {
    skvm::Builder b;
    {
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf = b.varying<int>();
        skvm::I32 x = b.load32(buf);
        b.store32(buf, b.gather32(uniforms,0, b.bit_and(x, b.splat(7))));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        const int img[] = {12,34,56,78, 90,98,76,54};

        int buf[20];
        for (int i = 0; i < 20; i++) {
            buf[i] = i;
        }

        struct Uniforms {
            const int* img;
        } uniforms{img};

        program.eval(20, &uniforms, buf);
        int i = 0;
        REPORTER_ASSERT(r, buf[i] == 12); i++;
        REPORTER_ASSERT(r, buf[i] == 34); i++;
        REPORTER_ASSERT(r, buf[i] == 56); i++;
        REPORTER_ASSERT(r, buf[i] == 78); i++;
        REPORTER_ASSERT(r, buf[i] == 90); i++;
        REPORTER_ASSERT(r, buf[i] == 98); i++;
        REPORTER_ASSERT(r, buf[i] == 76); i++;
        REPORTER_ASSERT(r, buf[i] == 54); i++;

        REPORTER_ASSERT(r, buf[i] == 12); i++;
        REPORTER_ASSERT(r, buf[i] == 34); i++;
        REPORTER_ASSERT(r, buf[i] == 56); i++;
        REPORTER_ASSERT(r, buf[i] == 78); i++;
        REPORTER_ASSERT(r, buf[i] == 90); i++;
        REPORTER_ASSERT(r, buf[i] == 98); i++;
        REPORTER_ASSERT(r, buf[i] == 76); i++;
        REPORTER_ASSERT(r, buf[i] == 54); i++;

        REPORTER_ASSERT(r, buf[i] == 12); i++;
        REPORTER_ASSERT(r, buf[i] == 34); i++;
        REPORTER_ASSERT(r, buf[i] == 56); i++;
        REPORTER_ASSERT(r, buf[i] == 78); i++;
    });
}

DEF_TEST(SkVM_gathers, r) {
    skvm::Builder b;
    {
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf32    = b.varying<int>(),
                  buf16    = b.varying<uint16_t>(),
                  buf8     = b.varying<uint8_t>();

        skvm::I32 x = b.load32(buf32);

        b.store32(buf32, b.gather32(uniforms,0, b.bit_and(x, b.splat( 7))));
        b.store16(buf16, b.gather16(uniforms,0, b.bit_and(x, b.splat(15))));
        b.store8 (buf8 , b.gather8 (uniforms,0, b.bit_and(x, b.splat(31))));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        const int img[] = {12,34,56,78, 90,98,76,54};

        constexpr int N = 20;
        int      buf32[N];
        uint16_t buf16[N];
        uint8_t  buf8 [N];

        for (int i = 0; i < 20; i++) {
            buf32[i] = i;
        }

        struct Uniforms {
            const int* img;
        } uniforms{img};

        program.eval(N, &uniforms, buf32, buf16, buf8);
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

DEF_TEST(SkVM_gathers2, r) {
    skvm::Builder b;
    {
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf32    = b.varying<int>(),
                  buf16    = b.varying<uint16_t>(),
                  buf8     = b.varying<uint8_t>();

        skvm::I32 x = b.load32(buf32);

        b.store32(buf32, b.gather32(uniforms,0, x));
        b.store16(buf16, b.gather16(uniforms,0, x));
        b.store8 (buf8 , b.gather8 (uniforms,0, x));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        uint8_t img[256];
        for (int i = 0; i < 256; i++) {
            img[i] = i;
        }

        int      buf32[64];
        uint16_t buf16[64];
        uint8_t  buf8 [64];

        for (int i = 0; i < 64; i++) {
            buf32[i] = (i*47)&63;
            buf16[i] = 0;
            buf8 [i] = 0;
        }

        struct Uniforms {
            const uint8_t* img;
        } uniforms{img};

        program.eval(64, &uniforms, buf32, buf16, buf8);

        for (int i = 0; i < 64; i++) {
            REPORTER_ASSERT(r, buf8[i] == ((i*47)&63));  // 0,47,30,13,60,...
        }

        REPORTER_ASSERT(r, buf16[ 0] == 0x0100);
        REPORTER_ASSERT(r, buf16[63] == 0x2322);

        REPORTER_ASSERT(r, buf32[ 0] == 0x03020100);
        REPORTER_ASSERT(r, buf32[63] == 0x47464544);
    });
}

DEF_TEST(SkVM_bitops, r) {
    skvm::Builder b;
    {
        skvm::Ptr ptr = b.varying<int>();

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

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        int x = 0x42;
        program.eval(1, &x);
        REPORTER_ASSERT(r, x == 0x7fff'ffff);
    });
}

DEF_TEST(SkVM_select_is_NaN, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>(),
                  dst = b.varying<float>();

        skvm::F32 x = b.loadF(src);
        x = select(is_NaN(x), b.splat(0.0f)
                            , x);
        b.storeF(dst, x);
    }

    std::vector<skvm::OptimizedInstruction> program = b.optimize();
    REPORTER_ASSERT(r, program.size() == 4);
    REPORTER_ASSERT(r, program[0].op == skvm::Op::load32);
    REPORTER_ASSERT(r, program[1].op == skvm::Op::neq_f32);
    REPORTER_ASSERT(r, program[2].op == skvm::Op::bit_clear);
    REPORTER_ASSERT(r, program[3].op == skvm::Op::store32);

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        // ±NaN, ±0, ±1, ±inf
        uint32_t src[] = {0x7f80'0001, 0xff80'0001, 0x0000'0000, 0x8000'0000,
                          0x3f80'0000, 0xbf80'0000, 0x7f80'0000, 0xff80'0000};
        uint32_t dst[SK_ARRAY_COUNT(src)];
        program.eval(SK_ARRAY_COUNT(src), src, dst);

        for (int i = 0; i < (int)SK_ARRAY_COUNT(src); i++) {
            REPORTER_ASSERT(r, dst[i] == (i < 2 ? 0 : src[i]));
        }
    });
}

DEF_TEST(SkVM_f32, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<float>();

        skvm::F32 x = b.loadF(arg),
                  y = b.add(x,x),   // y = 2x
                  z = b.sub(y,x),   // z = 2x-x = x
                  w = b.div(z,x);   // w = x/x = 1
        b.storeF(arg, w);
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
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
    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
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
        skvm::F32 x = b.loadF(b.varying<float>());

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

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
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

DEF_TEST(SkVM_index, r) {
    skvm::Builder b;
    b.store32(b.varying<int>(), b.index());

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        int buf[23];
        program.eval(SK_ARRAY_COUNT(buf), buf);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == (int)SK_ARRAY_COUNT(buf)-i);
        }
    });
}

DEF_TEST(SkVM_mad, r) {
    // This program is designed to exercise the tricky corners of instruction
    // and register selection for Op::mad_f32.

    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();

        skvm::F32 x = b.to_F32(b.load32(arg)),
                  y = b.mad(x,x,x),   // x is needed in the future, so r[x] != r[y].
                  z = b.mad(y,y,x),   // y is needed in the future, but r[z] = r[x] is ok.
                  w = b.mad(z,z,y),   // w can alias z but not y.
                  v = b.mad(w,y,w);   // Got to stop somewhere.
        b.store32(arg, b.trunc(v));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
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

DEF_TEST(SkVM_fms, r) {
    // Create a pattern that can be peepholed into an Op::fms_f32.
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();

        skvm::F32 x = b.to_F32(b.load32(arg)),
                  v = b.sub(b.mul(x, b.splat(2.0f)),
                            b.splat(1.0f));
        b.store32(arg, b.trunc(v));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = {0,1,2,3,4,5,6,7,8,9,10};
        program.eval((int)SK_ARRAY_COUNT(buf), &buf);

        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] = 2*i-1);
        }
    });
}

DEF_TEST(SkVM_fnma, r) {
    // Create a pattern that can be peepholed into an Op::fnma_f32.
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();

        skvm::F32 x = b.to_F32(b.load32(arg)),
                  v = b.sub(b.splat(1.0f),
                            b.mul(x, b.splat(2.0f)));
        b.store32(arg, b.trunc(v));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = {0,1,2,3,4,5,6,7,8,9,10};
        program.eval((int)SK_ARRAY_COUNT(buf), &buf);

        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] = 1-2*i);
        }
    });
}

DEF_TEST(SkVM_madder, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<float>();

        skvm::F32 x = b.loadF(arg),
                  y = b.mad(x,x,x),   // x is needed in the future, so r[x] != r[y].
                  z = b.mad(y,x,y),   // r[x] can be reused after this instruction, but not r[y].
                  w = b.mad(y,y,z);
        b.storeF(arg, w);
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        float x = 2.0f;
        // y = 2*2 + 2 = 6
        // z = 6*2 + 6 = 18
        // w = 6*6 + 18 = 54
        program.eval(1, &x);
        REPORTER_ASSERT(r, x == 54.0f);
    });
}

DEF_TEST(SkVM_floor, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<float>();
        b.storeF(arg, b.floor(b.loadF(arg)));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        float buf[]  = { -2.0f, -1.5f, -1.0f, 0.0f, 1.0f, 1.5f, 2.0f };
        float want[] = { -2.0f, -2.0f, -1.0f, 0.0f, 1.0f, 1.0f, 2.0f };
        program.eval(SK_ARRAY_COUNT(buf), buf);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_round, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>();
        skvm::Ptr dst = b.varying<int>();
        b.store32(dst, b.round(b.loadF(src)));
    }

    // The test cases on exact 0.5f boundaries assume the current rounding mode is nearest even.
    // We haven't explicitly guaranteed that here... it just probably is.
    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        float buf[]  = { -1.5f, -0.5f, 0.0f, 0.5f, 0.2f, 0.6f, 1.0f, 1.4f, 1.5f, 2.0f };
        int want[] =   { -2   ,  0   , 0   , 0   , 0   , 1   , 1   , 1   , 2   , 2    };
        int dst[SK_ARRAY_COUNT(buf)];

        program.eval(SK_ARRAY_COUNT(buf), buf, dst);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(dst); i++) {
            REPORTER_ASSERT(r, dst[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_min, r) {
    skvm::Builder b;
    {
        skvm::Ptr src1 = b.varying<float>();
        skvm::Ptr src2 = b.varying<float>();
        skvm::Ptr dst = b.varying<float>();

        b.storeF(dst, b.min(b.loadF(src1), b.loadF(src2)));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        float s1[]  =  { 0.0f, 1.0f, 4.0f, -1.0f, -1.0f};
        float s2[]  =  { 0.0f, 2.0f, 3.0f,  1.0f, -2.0f};
        float want[] = { 0.0f, 1.0f, 3.0f, -1.0f, -2.0f};
        float d[SK_ARRAY_COUNT(s1)];
        program.eval(SK_ARRAY_COUNT(d), s1, s2, d);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(d); i++) {
          REPORTER_ASSERT(r, d[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_max, r) {
    skvm::Builder b;
    {
        skvm::Ptr src1 = b.varying<float>();
        skvm::Ptr src2 = b.varying<float>();
        skvm::Ptr dst = b.varying<float>();

        b.storeF(dst, b.max(b.loadF(src1), b.loadF(src2)));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        float s1[]  =  { 0.0f, 1.0f, 4.0f, -1.0f, -1.0f};
        float s2[]  =  { 0.0f, 2.0f, 3.0f,  1.0f, -2.0f};
        float want[] = { 0.0f, 2.0f, 4.0f,  1.0f, -1.0f};
        float d[SK_ARRAY_COUNT(s1)];
        program.eval(SK_ARRAY_COUNT(d), s1, s2, d);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(d); i++) {
          REPORTER_ASSERT(r, d[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_hoist, r) {
    // This program uses enough constants that it will fail to JIT if we hoist them.
    // The JIT will try again without hoisting, and that'll just need 2 registers.
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();
        skvm::I32 x = b.load32(arg);
        for (int i = 0; i < 32; i++) {
            x = b.add(x, b.splat(i));
        }
        b.store32(arg, x);
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
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
        skvm::Ptr buf = b.varying<int>();

        skvm::I32 x = b.load32(buf);

        x = b.select( b.gt(x, b.splat(4)), x, b.splat(42) );

        b.store32(buf, x);
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = { 0,1,2,3,4,5,6,7,8 };
        program.eval(SK_ARRAY_COUNT(buf), buf);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == (i > 4 ? i : 42));
        }
    });
}

DEF_TEST(SkVM_swap, r) {
    skvm::Builder b;
    {
        // This program is the equivalent of
        //     x = *X
        //     y = *Y
        //     *X = y
        //     *Y = x
        // One rescheduling of the program based only on data flow of Op arguments is
        //     x = *X
        //     *Y = x
        //     y = *Y
        //     *X = y
        // but this reordering does not produce the same results and is invalid.
        skvm::Ptr X = b.varying<int>(),
                  Y = b.varying<int>();

        skvm::I32 x = b.load32(X),
                  y = b.load32(Y);

        b.store32(X, y);
        b.store32(Y, x);
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        int b1[] = { 0,1,2,3 };
        int b2[] = { 4,5,6,7 };
        program.eval(SK_ARRAY_COUNT(b1), b1, b2);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(b1); i++) {
            REPORTER_ASSERT(r, b1[i] == 4 + i);
            REPORTER_ASSERT(r, b2[i] == i);
        }
    });
}

DEF_TEST(SkVM_NewOps, r) {
    // Exercise a somewhat arbitrary set of new ops.
    skvm::Builder b;
    {
        skvm::Ptr buf = b.varying<int16_t>();
        skvm::UPtr uniforms = b.uniform();

        skvm::I32 x = b.load16(buf);

        const size_t kPtr = sizeof(const int*);

        x = b.add(x, b.uniform32(uniforms, kPtr+0));
        x = b.mul(x, b.uniform32(uniforms, kPtr+4));
        x = b.sub(x, b.uniform32(uniforms, kPtr+8));

        skvm::I32 limit = b.uniform32(uniforms, kPtr+12);
        x = b.select(b.lt(x, b.splat(0)), b.splat(0), x);
        x = b.select(b.gt(x, limit     ), limit     , x);

        x = b.gather8(uniforms,0, x);

        b.store16(buf, x);
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
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
            const uint8_t* img;
            int      add   = 5;
            int      mul   = 3;
            int      sub   = 18;
            int      limit = M-1;
        } uniforms{img};

        program.eval(N, buf, &uniforms);

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

DEF_TEST(SKVM_array32, r) {



    skvm::Builder b;
    skvm::Uniforms uniforms(b.uniform(), 0);
    // Take up the first slot, so other uniforms are not at 0 offset.
    uniforms.push(0);
    int i[] = {3, 7};
    skvm::Uniform array = uniforms.pushArray(i);
    float f[] = {5, 9};
    skvm::Uniform arrayF = uniforms.pushArrayF(f);
    {
        skvm::Ptr buf0     = b.varying<int32_t>(),
                  buf1     = b.varying<int32_t>(),
                  buf2     = b.varying<int32_t>();

        skvm::I32 j = b.array32(array, 0);
        b.store32(buf0, j);
        skvm::I32 k = b.array32(array, 1);
        b.store32(buf1, k);

        skvm::F32 x = b.arrayF(arrayF, 0);
        skvm::F32 y = b.arrayF(arrayF, 1);
        b.store32(buf2, b.trunc(b.add(x, y)));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        const int K = 10;
        int32_t buf0[K],
                buf1[K],
                buf2[K];

        // reset the i[0] for the two tests.
        i[0] = 3;
        f[1] = 9;
        program.eval(K, uniforms.buf.data(), buf0, buf1, buf2);
        for (auto v : buf0) {
            REPORTER_ASSERT(r, v == 3);
        }
        for (auto v : buf1) {
            REPORTER_ASSERT(r, v == 7);
        }
        for (auto v : buf2) {
            REPORTER_ASSERT(r, v == 14);
        }
        i[0] = 4;
        f[1] = 10;
        program.eval(K, uniforms.buf.data(), buf0, buf1, buf2);
        for (auto v : buf0) {
            REPORTER_ASSERT(r, v == 4);
        }
        for (auto v : buf1) {
            REPORTER_ASSERT(r, v == 7);
        }
        for (auto v : buf2) {
            REPORTER_ASSERT(r, v == 15);
        }
    });
}

DEF_TEST(SkVM_sqrt, r) {
    skvm::Builder b;
    auto buf = b.varying<int>();
    b.storeF(buf, b.sqrt(b.loadF(buf)));

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        constexpr int K = 17;
        float buf[K];
        for (int i = 0; i < K; i++) {
            buf[i] = (float)(i*i);
        }

        // x^2 -> x
        program.eval(K, buf);

        for (int i = 0; i < K; i++) {
            REPORTER_ASSERT(r, buf[i] == (float)i);
        }
    });
}

DEF_TEST(SkVM_MSAN, r) {
    // This little memset32() program should be able to JIT, but if we run that
    // JIT code in an MSAN build, it won't see the writes initialize buf.  So
    // this tests that we're using the interpreter instead.
    skvm::Builder b;
    b.store32(b.varying<int>(), b.splat(42));

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        constexpr int K = 17;
        int buf[K];                 // Intentionally uninitialized.
        program.eval(K, buf);
        sk_msan_assert_initialized(buf, buf+K);
        for (int x : buf) {
            REPORTER_ASSERT(r, x == 42);
        }
    });
}

DEF_TEST(SkVM_assert, r) {
    skvm::Builder b;
    b.assert_true(b.lt(b.load32(b.varying<int>()),
                       b.splat(42)));

    test_jit_and_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = { 0,1,2,3,4,5,6,7,8,9 };
        program.eval(SK_ARRAY_COUNT(buf), buf);
    });
}

DEF_TEST(SkVM_trace_line, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void var(int, int32_t) override { fBuffer.push_back(-9999999); }
        void enter(int) override        { fBuffer.push_back(-9999999); }
        void exit(int) override         { fBuffer.push_back(-9999999); }
        void line(int lineNum) override { fBuffer.push_back(lineNum); }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTrace;
    int traceHookID = b.attachTraceHook(&testTrace);
    b.trace_line(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 123);
    b.trace_line(traceHookID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 456);
    b.trace_line(traceHookID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 567);
    b.trace_line(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 678);
    b.trace_line(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 789);
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTrace.fBuffer == std::vector<int>{123, 789}));
}

DEF_TEST(SkVM_trace_var, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void line(int) override                  { fBuffer.push_back(-9999999); }
        void enter(int) override                 { fBuffer.push_back(-9999999); }
        void exit(int) override                  { fBuffer.push_back(-9999999); }
        void var(int slot, int32_t val) override {
            fBuffer.push_back(slot);
            fBuffer.push_back(val);
        }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTrace;
    int traceHookID = b.attachTraceHook(&testTrace);
    b.trace_var(traceHookID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 2, b.splat(333));
    b.trace_var(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 4, b.splat(555));
    b.trace_var(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 5, b.splat(666));
    b.trace_var(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 6, b.splat(777));
    b.trace_var(traceHookID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 8, b.splat(999));
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTrace.fBuffer == std::vector<int>{4, 555, 6, 777}));
}

DEF_TEST(SkVM_trace_enter_exit, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void line(int) override                   { fBuffer.push_back(-9999999); }
        void var(int, int32_t) override           { fBuffer.push_back(-9999999); }
        void enter(int fnIdx) override {
            fBuffer.push_back(fnIdx);
            fBuffer.push_back(1);
        }
        void exit(int fnIdx) override {
            fBuffer.push_back(fnIdx);
            fBuffer.push_back(0);
        }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTrace;
    int traceHookID = b.attachTraceHook(&testTrace);
    b.trace_enter(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 99);
    b.trace_enter(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 12);
    b.trace_enter(traceHookID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 34);
    b.trace_exit(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 56);
    b.trace_exit(traceHookID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 78);
    b.trace_exit(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 90);
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTrace.fBuffer == std::vector<int>{12, 1, 56, 0}));
}

DEF_TEST(SkVM_trace_multiple_hooks, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void var(int, int32_t) override { fBuffer.push_back(-9999999); }
        void enter(int) override        { fBuffer.push_back(-9999999); }
        void exit(int) override         { fBuffer.push_back(-9999999); }
        void line(int lineNum) override { fBuffer.push_back(lineNum); }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTraceA, testTraceB, testTraceC;
    int traceHookAID = b.attachTraceHook(&testTraceA);
    int traceHookBID = b.attachTraceHook(&testTraceB);
    int traceHookCID = b.attachTraceHook(&testTraceC);
    b.trace_line(traceHookCID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 111);
    b.trace_line(traceHookAID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 222);
    b.trace_line(traceHookCID, b.splat(0x00000000), b.splat(0x00000000), 333);
    b.trace_line(traceHookBID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 444);
    b.trace_line(traceHookAID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 555);
    b.trace_line(traceHookBID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 666);
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTraceA.fBuffer == std::vector<int>{222}));
    REPORTER_ASSERT(r, (testTraceB.fBuffer == std::vector<int>{666}));
    REPORTER_ASSERT(r, (testTraceC.fBuffer == std::vector<int>{111}));
}

DEF_TEST(SkVM_premul, reporter) {
    // Test that premul is short-circuited when alpha is known opaque.
    {
        skvm::Builder p;
        auto rptr = p.varying<int>(),
             aptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.loadF(aptr);

        p.premul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, load alpha, red *= alpha, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() == 4);
    }

    {
        skvm::Builder p;
        auto rptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.splat(1.0f);

        p.premul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() == 2);
    }

    // Same deal for unpremul.
    {
        skvm::Builder p;
        auto rptr = p.varying<int>(),
             aptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.loadF(aptr);

        p.unpremul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, load alpha, a bunch of unpremul instructions, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() >= 4);
    }

    {
        skvm::Builder p;
        auto rptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.splat(1.0f);

        p.unpremul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() == 2);
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
        a.int3();
        a.vzeroupper();
        a.ret();
    },{
        0xcc,
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

        a.add(A::Mem{A::rsi}, 7);                       // addq $7, (%rsi)
        a.add(A::Mem{A::rsi, 12}, 7);                   // addq $7, 12(%rsi)
        a.add(A::Mem{A::rsp, 12}, 7);                   // addq $7, 12(%rsp)
        a.add(A::Mem{A::r12, 12}, 7);                   // addq $7, 12(%r12)
        a.add(A::Mem{A::rsp, 12, A::rax, A::FOUR}, 7);  // addq $7, 12(%rsp,%rax,4)
        a.add(A::Mem{A::r12, 12, A::rax, A::FOUR}, 7);  // addq $7, 12(%r12,%rax,4)
        a.add(A::Mem{A::rax, 12, A::r12, A::FOUR}, 7);  // addq $7, 12(%rax,%r12,4)
        a.add(A::Mem{A::r11, 12, A::r8 , A::TWO }, 7);  // addq $7, 12(%r11,%r8,2)
        a.add(A::Mem{A::r11, 12, A::rax}         , 7);  // addq $7, 12(%r11,%rax)
        a.add(A::Mem{A::rax, 12, A::r11}         , 7);  // addq $7, 12(%rax,%r11)

        a.sub(A::Mem{A::rax, 12, A::r11}         , 7);  // subq $7, 12(%rax,%r11)

        a.add(       A::rax     , A::rcx);              // addq %rcx, %rax
        a.add(A::Mem{A::rax}    , A::rcx);              // addq %rcx, (%rax)
        a.add(A::Mem{A::rax, 12}, A::rcx);              // addq %rcx, 12(%rax)
        a.add(A::rcx, A::Mem{A::rax, 12});              // addq 12(%rax), %rcx

        a.sub(A::rcx, A::Mem{A::rax, 12});              // subq 12(%rax), %rcx
    },{
        0x48, 0x83, 0b11'000'000, 0x08,
        0x48, 0x83, 0b11'101'000, 0x20,

        0x48, 0x83, 0b11'000'111, 0x0c,
        0x48, 0x83, 0b11'101'111, 0x08,

        0x49, 0x83, 0b11'000'000, 0x07,
        0x49, 0x83, 0b11'101'000, 0x04,

        0x48, 0x81, 0b11'000'110, 0x80, 0x00, 0x00, 0x00,
        0x49, 0x81, 0b11'101'000, 0x40, 0x42, 0x0f, 0x00,

        0x48,0x83,0x06,0x07,
        0x48,0x83,0x46,0x0c,0x07,
        0x48,0x83,0x44,0x24,0x0c,0x07,
        0x49,0x83,0x44,0x24,0x0c,0x07,
        0x48,0x83,0x44,0x84,0x0c,0x07,
        0x49,0x83,0x44,0x84,0x0c,0x07,
        0x4a,0x83,0x44,0xa0,0x0c,0x07,
        0x4b,0x83,0x44,0x43,0x0c,0x07,
        0x49,0x83,0x44,0x03,0x0c,0x07,
        0x4a,0x83,0x44,0x18,0x0c,0x07,

        0x4a,0x83,0x6c,0x18,0x0c,0x07,

        0x48,0x01,0xc8,
        0x48,0x01,0x08,
        0x48,0x01,0x48,0x0c,
        0x48,0x03,0x48,0x0c,
        0x48,0x2b,0x48,0x0c,
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
        a.vpaddw   (A::ymm4, A::ymm3, A::ymm2);
        a.vpavgw   (A::ymm4, A::ymm3, A::ymm2);
        a.vpcmpeqw (A::ymm4, A::ymm3, A::ymm2);
        a.vpcmpgtw (A::ymm4, A::ymm3, A::ymm2);

        a.vpminsw  (A::ymm4, A::ymm3, A::ymm2);
        a.vpmaxsw  (A::ymm4, A::ymm3, A::ymm2);
        a.vpminuw  (A::ymm4, A::ymm3, A::ymm2);
        a.vpmaxuw  (A::ymm4, A::ymm3, A::ymm2);

        a.vpmulhrsw(A::ymm4, A::ymm3, A::ymm2);
        a.vpabsw   (A::ymm4, A::ymm3);
        a.vpsllw   (A::ymm4, A::ymm3, 12);
        a.vpsraw   (A::ymm4, A::ymm3, 12);
    },{
        0xc5,     0xe5, 0xfd, 0xe2,
        0xc5,     0xe5, 0xe3, 0xe2,
        0xc5,     0xe5, 0x75, 0xe2,
        0xc5,     0xe5, 0x65, 0xe2,

        0xc5,     0xe5, 0xea, 0xe2,
        0xc5,     0xe5, 0xee, 0xe2,
        0xc4,0xe2,0x65, 0x3a, 0xe2,
        0xc4,0xe2,0x65, 0x3e, 0xe2,

        0xc4,0xe2,0x65, 0x0b, 0xe2,
        0xc4,0xe2,0x7d, 0x1d, 0xe3,
        0xc5,0xdd,0x71, 0xf3, 0x0c,
        0xc5,0xdd,0x71, 0xe3, 0x0c,
    });

    test_asm(r, [&](A& a) {
        A::Label l;
        a.vcmpeqps (A::ymm0, A::ymm1, &l);      // vcmpeqps 0x1c(%rip), %ymm1, %ymm0
        a.vpcmpeqd (A::ymm0, A::ymm1, A::ymm2);
        a.vpcmpgtd (A::ymm0, A::ymm1, A::ymm2);
        a.vcmpeqps (A::ymm0, A::ymm1, A::ymm2);
        a.vcmpltps (A::ymm0, A::ymm1, A::ymm2);
        a.vcmpleps (A::ymm0, A::ymm1, A::ymm2);
        a.vcmpneqps(A::ymm0, A::ymm1, A::ymm2);
        a.label(&l);   // 28 bytes after the vcmpeqps that uses it.
    },{
        0xc5,0xf4,0xc2,0x05,0x1c,0x00,0x00,0x00,0x00,
        0xc5,0xf5,0x76,0xc2,
        0xc5,0xf5,0x66,0xc2,
        0xc5,0xf4,0xc2,0xc2,0x00,
        0xc5,0xf4,0xc2,0xc2,0x01,
        0xc5,0xf4,0xc2,0xc2,0x02,
        0xc5,0xf4,0xc2,0xc2,0x04,
    });

    test_asm(r, [&](A& a) {
        a.vminps(A::ymm0, A::ymm1, A::ymm2);
        a.vmaxps(A::ymm0, A::ymm1, A::ymm2);
    },{
        0xc5,0xf4,0x5d,0xc2,
        0xc5,0xf4,0x5f,0xc2,
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
        A::Label l;
        a.vpermps(A::ymm1, A::ymm2, A::Mem{A::rdi, 32});
        a.vperm2f128(A::ymm1, A::ymm2, &l, 0x20);
        a.vpermq(A::ymm1, A::ymm2, 5);
        a.label(&l);  // 6 bytes after vperm2f128
    },{
        0xc4,0xe2,0x6d,0x16,0x4f,0x20,
        0xc4,0xe3,0x6d,0x06,0x0d,0x06,0x00,0x00,0x00,0x20,
        0xc4,0xe3,0xfd, 0x00,0xca, 0x05,
    });

    test_asm(r, [&](A& a) {
        a.vpunpckldq(A::ymm1, A::ymm2, A::Mem{A::rdi});
        a.vpunpckhdq(A::ymm1, A::ymm2, A::ymm3);
    },{
        0xc5,0xed,0x62,0x0f,
        0xc5,0xed,0x6a,0xcb,
    });

    test_asm(r, [&](A& a) {
        a.vroundps(A::ymm1, A::ymm2, A::NEAREST);
        a.vroundps(A::ymm1, A::ymm2, A::FLOOR);
        a.vroundps(A::ymm1, A::ymm2, A::CEIL);
        a.vroundps(A::ymm1, A::ymm2, A::TRUNC);
    },{
        0xc4,0xe3,0x7d,0x08,0xca,0x00,
        0xc4,0xe3,0x7d,0x08,0xca,0x01,
        0xc4,0xe3,0x7d,0x08,0xca,0x02,
        0xc4,0xe3,0x7d,0x08,0xca,0x03,
    });

    test_asm(r, [&](A& a) {
        A::Label l;
        a.label(&l);
        a.byte(1);
        a.byte(2);
        a.byte(3);
        a.byte(4);

        a.vbroadcastss(A::ymm0 , &l);
        a.vbroadcastss(A::ymm1 , &l);
        a.vbroadcastss(A::ymm8 , &l);
        a.vbroadcastss(A::ymm15, &l);

        a.vpshufb(A::ymm4, A::ymm3, &l);
        a.vpaddd (A::ymm4, A::ymm3, &l);
        a.vpsubd (A::ymm4, A::ymm3, &l);

        a.vptest(A::ymm4, &l);

        a.vmulps (A::ymm4, A::ymm3, &l);
    },{
        0x01, 0x02, 0x03, 0x4,

        /*     VEX    */  /*op*/ /*   ModRM    */  /*     offset     */
        0xc4, 0xe2, 0x7d,  0x18,   0b00'000'101,   0xf3,0xff,0xff,0xff,   // 0xfffffff3 == -13
        0xc4, 0xe2, 0x7d,  0x18,   0b00'001'101,   0xea,0xff,0xff,0xff,   // 0xffffffea == -22
        0xc4, 0x62, 0x7d,  0x18,   0b00'000'101,   0xe1,0xff,0xff,0xff,   // 0xffffffe1 == -31
        0xc4, 0x62, 0x7d,  0x18,   0b00'111'101,   0xd8,0xff,0xff,0xff,   // 0xffffffd8 == -40

        0xc4, 0xe2, 0x65,  0x00,   0b00'100'101,   0xcf,0xff,0xff,0xff,   // 0xffffffcf == -49

        0xc5, 0xe5,        0xfe,   0b00'100'101,   0xc7,0xff,0xff,0xff,   // 0xffffffc7 == -57
        0xc5, 0xe5,        0xfa,   0b00'100'101,   0xbf,0xff,0xff,0xff,   // 0xffffffbf == -65

        0xc4, 0xe2, 0x7d,  0x17,   0b00'100'101,   0xb6,0xff,0xff,0xff,   // 0xffffffb6 == -74

        0xc5, 0xe4,        0x59,   0b00'100'101,   0xae,0xff,0xff,0xff,   // 0xffffffaf == -82
    });

    test_asm(r, [&](A& a) {
        a.vbroadcastss(A::ymm0,  A::Mem{A::rdi,   0});
        a.vbroadcastss(A::ymm13, A::Mem{A::r14,   7});
        a.vbroadcastss(A::ymm8,  A::Mem{A::rdx, -12});
        a.vbroadcastss(A::ymm8,  A::Mem{A::rdx, 400});

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
        A::Label l;
        a.label(&l);
        a.jne(&l);
        a.jne(&l);
        a.je (&l);
        a.jmp(&l);
        a.jl (&l);
        a.jc (&l);

        a.cmp(A::rdx, 1);
        a.cmp(A::rax, 12);
        a.cmp(A::r14, 2000000000);
    },{
        0x0f,0x85, 0xfa,0xff,0xff,0xff,   // near jne -6 bytes
        0x0f,0x85, 0xf4,0xff,0xff,0xff,   // near jne -12 bytes
        0x0f,0x84, 0xee,0xff,0xff,0xff,   // near je  -18 bytes
        0xe9,      0xe9,0xff,0xff,0xff,   // near jmp -23 bytes
        0x0f,0x8c, 0xe3,0xff,0xff,0xff,   // near jl  -29 bytes
        0x0f,0x82, 0xdd,0xff,0xff,0xff,   // near jc  -35 bytes

        0x48,0x83,0xfa,0x01,
        0x48,0x83,0xf8,0x0c,
        0x49,0x81,0xfe,0x00,0x94,0x35,0x77,
    });

    test_asm(r, [&](A& a) {
        a.vmovups(A::ymm5, A::Mem{A::rsi});
        a.vmovups(A::Mem{A::rsi}, A::ymm5);

        a.vmovups(A::xmm5, A::Mem{A::rsi});
        a.vmovups(A::Mem{A::rsi}, A::xmm5);

        a.vpmovzxwd(A::ymm4, A::Mem{A::rsi});
        a.vpmovzxbd(A::ymm4, A::Mem{A::rsi});

        a.vmovq(A::Mem{A::rdx}, A::xmm15);
    },{
        /*    VEX    */  /*Op*/  /*  ModRM  */
        0xc5,     0xfc,   0x10,  0b00'101'110,
        0xc5,     0xfc,   0x11,  0b00'101'110,

        0xc5,     0xf8,   0x10,  0b00'101'110,
        0xc5,     0xf8,   0x11,  0b00'101'110,

        0xc4,0xe2,0x7d,   0x33,  0b00'100'110,
        0xc4,0xe2,0x7d,   0x31,  0b00'100'110,

        0xc5,     0x79,   0xd6,  0b00'111'010,
    });

    test_asm(r, [&](A& a) {
        a.vmovups(A::ymm5, A::Mem{A::rsp,  0});
        a.vmovups(A::ymm5, A::Mem{A::rsp, 64});
        a.vmovups(A::ymm5, A::Mem{A::rsp,128});

        a.vmovups(A::Mem{A::rsp,  0}, A::ymm5);
        a.vmovups(A::Mem{A::rsp, 64}, A::ymm5);
        a.vmovups(A::Mem{A::rsp,128}, A::ymm5);
    },{
        0xc5,0xfc,0x10,0x2c,0x24,
        0xc5,0xfc,0x10,0x6c,0x24,0x40,
        0xc5,0xfc,0x10,0xac,0x24,0x80,0x00,0x00,0x00,

        0xc5,0xfc,0x11,0x2c,0x24,
        0xc5,0xfc,0x11,0x6c,0x24,0x40,
        0xc5,0xfc,0x11,0xac,0x24,0x80,0x00,0x00,0x00,
    });

    test_asm(r, [&](A& a) {
        a.movzbq(A::rax, A::Mem{A::rsi});   // Low registers for src and dst.
        a.movzbq(A::rax, A::Mem{A::r8,});   // High src register.
        a.movzbq(A::r8 , A::Mem{A::rsi});   // High dst register.
        a.movzbq(A::r8,  A::Mem{A::rsi, 12});
        a.movzbq(A::r8,  A::Mem{A::rsi, 400});

        a.movzwq(A::rax, A::Mem{A::rsi});   // Low registers for src and dst.
        a.movzwq(A::rax, A::Mem{A::r8,});   // High src register.
        a.movzwq(A::r8 , A::Mem{A::rsi});   // High dst register.
        a.movzwq(A::r8,  A::Mem{A::rsi, 12});
        a.movzwq(A::r8,  A::Mem{A::rsi, 400});

        a.vmovd(A::Mem{A::rax}, A::xmm0);
        a.vmovd(A::Mem{A::rax}, A::xmm8);
        a.vmovd(A::Mem{A::r8 }, A::xmm0);

        a.vmovd(A::xmm0, A::Mem{A::rax});
        a.vmovd(A::xmm8, A::Mem{A::rax});
        a.vmovd(A::xmm0, A::Mem{A::r8 });

        a.vmovd(A::xmm0 , A::Mem{A::rax, 0, A::rcx, A::FOUR});
        a.vmovd(A::xmm15, A::Mem{A::rax, 0, A::r8,  A::TWO });
        a.vmovd(A::xmm0 , A::Mem{A::r8 , 0, A::rcx});

        a.vmovd(A::rax, A::xmm0);
        a.vmovd(A::rax, A::xmm8);
        a.vmovd(A::r8 ,  A::xmm0);

        a.vmovd(A::xmm0, A::rax);
        a.vmovd(A::xmm8, A::rax);
        a.vmovd(A::xmm0, A::r8 );

        a.movb(A::Mem{A::rdx}, A::rax);
        a.movb(A::Mem{A::rdx}, A::r8 );
        a.movb(A::Mem{A::r8 }, A::rax);

        a.movb(A::rdx, A::Mem{A::rax});
        a.movb(A::rdx, A::Mem{A::r8 });
        a.movb(A::r8 , A::Mem{A::rax});

        a.movb(A::rdx, 12);
        a.movb(A::rax,  4);
        a.movb(A::r8 , -1);

        a.movb(A::Mem{A::rdx}, 12);
        a.movb(A::Mem{A::rax},  4);
        a.movb(A::Mem{A::r8 }, -1);
    },{
        0x48,0x0f,0xb6,0x06,     // movzbq (%rsi), %rax
        0x49,0x0f,0xb6,0x00,
        0x4c,0x0f,0xb6,0x06,
        0x4c,0x0f,0xb6,0x46, 12,
        0x4c,0x0f,0xb6,0x86, 0x90,0x01,0x00,0x00,

        0x48,0x0f,0xb7,0x06,    // movzwq (%rsi), %rax
        0x49,0x0f,0xb7,0x00,
        0x4c,0x0f,0xb7,0x06,
        0x4c,0x0f,0xb7,0x46, 12,
        0x4c,0x0f,0xb7,0x86, 0x90,0x01,0x00,0x00,

        0xc5,0xf9,0x7e,0x00,
        0xc5,0x79,0x7e,0x00,
        0xc4,0xc1,0x79,0x7e,0x00,

        0xc5,0xf9,0x6e,0x00,
        0xc5,0x79,0x6e,0x00,
        0xc4,0xc1,0x79,0x6e,0x00,

        0xc5,0xf9,0x6e,0x04,0x88,
        0xc4,0x21,0x79,0x6e,0x3c,0x40,
        0xc4,0xc1,0x79,0x6e,0x04,0x08,

        0xc5,0xf9,0x7e,0xc0,
        0xc5,0x79,0x7e,0xc0,
        0xc4,0xc1,0x79,0x7e,0xc0,

        0xc5,0xf9,0x6e,0xc0,
        0xc5,0x79,0x6e,0xc0,
        0xc4,0xc1,0x79,0x6e,0xc0,

        0x48 ,0x88, 0x02,
        0x4c, 0x88, 0x02,
        0x49, 0x88, 0x00,

        0x48 ,0x8a, 0x10,
        0x49, 0x8a, 0x10,
        0x4c, 0x8a, 0x00,

        0x48, 0xc6, 0xc2, 0x0c,
        0x48, 0xc6, 0xc0, 0x04,
        0x49, 0xc6, 0xc0, 0xff,

        0x48, 0xc6, 0x02, 0x0c,
        0x48, 0xc6, 0x00, 0x04,
        0x49, 0xc6, 0x00, 0xff,
    });

    test_asm(r, [&](A& a) {
        a.vpinsrd(A::xmm1, A::xmm8, A::Mem{A::rsi}, 1);   // vpinsrd $1, (%rsi), %xmm8, %xmm1
        a.vpinsrd(A::xmm8, A::xmm1, A::Mem{A::r8 }, 3);   // vpinsrd $3, (%r8), %xmm1, %xmm8;

        a.vpinsrw(A::xmm1, A::xmm8, A::Mem{A::rsi}, 4);   // vpinsrw $4, (%rsi), %xmm8, %xmm1
        a.vpinsrw(A::xmm8, A::xmm1, A::Mem{A::r8 }, 12);  // vpinrsw $12, (%r8), %xmm1, %xmm8

        a.vpinsrb(A::xmm1, A::xmm8, A::Mem{A::rsi}, 4);   // vpinsrb $4, (%rsi), %xmm8, %xmm1
        a.vpinsrb(A::xmm8, A::xmm1, A::Mem{A::r8 }, 12);  // vpinsrb $12, (%r8), %xmm1, %xmm8

        a.vextracti128(A::xmm1, A::ymm8, 1);  // vextracti128 $1, %ymm8, %xmm1
        a.vextracti128(A::xmm8, A::ymm1, 0);  // vextracti128 $0, %ymm1, %xmm8

        a.vpextrd(A::Mem{A::rsi}, A::xmm8, 3);  // vpextrd  $3, %xmm8, (%rsi)
        a.vpextrd(A::Mem{A::r8 }, A::xmm1, 2);  // vpextrd  $2, %xmm1, (%r8)

        a.vpextrw(A::Mem{A::rsi}, A::xmm8, 7);
        a.vpextrw(A::Mem{A::r8 }, A::xmm1, 15);

        a.vpextrb(A::Mem{A::rsi}, A::xmm8, 7);
        a.vpextrb(A::Mem{A::r8 }, A::xmm1, 15);
    },{
        0xc4,0xe3,0x39, 0x22, 0x0e, 1,
        0xc4,0x43,0x71, 0x22, 0x00, 3,

        0xc5,0xb9,      0xc4, 0x0e,  4,
        0xc4,0x41,0x71, 0xc4, 0x00, 12,

        0xc4,0xe3,0x39, 0x20, 0x0e,  4,
        0xc4,0x43,0x71, 0x20, 0x00, 12,

        0xc4,0x63,0x7d,0x39,0xc1, 1,
        0xc4,0xc3,0x7d,0x39,0xc8, 0,

        0xc4,0x63,0x79,0x16,0x06, 3,
        0xc4,0xc3,0x79,0x16,0x08, 2,

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
        A::Label l;
        a.vmovdqa(A::ymm3, A::ymm2);                                // vmovdqa %ymm2         , %ymm3

        a.vmovdqa(A::ymm3, A::Mem{A::rsi});                         // vmovdqa  (%rsi)       , %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::rsp});                         // vmovdqa  (%rsp)       , %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::r11});                         // vmovdqa  (%r11)       , %ymm3

        a.vmovdqa(A::ymm3, A::Mem{A::rsi,  4});                     // vmovdqa 4(%rsi)       , %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::rsp,  4});                     // vmovdqa 4(%rsp)       , %ymm3

        a.vmovdqa(A::ymm3, A::Mem{A::rsi,  4, A::rax, A::EIGHT});   // vmovdqa 4(%rsi,%rax,8), %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::r11,  4, A::rax, A::TWO  });   // vmovdqa 4(%r11,%rax,2), %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::rsi,  4, A::r11, A::FOUR });   // vmovdqa 4(%rsi,%r11,4), %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::rsi,  4, A::r11, A::ONE  });   // vmovdqa 4(%rsi,%r11,1), %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::rsi,  4, A::r11});             // vmovdqa 4(%rsi,%r11)  , %ymm3

        a.vmovdqa(A::ymm3, A::Mem{A::rsi,  64, A::r11});            // vmovdqa  64(%rsi,%r11), %ymm3
        a.vmovdqa(A::ymm3, A::Mem{A::rsi, 128, A::r11});            // vmovdqa 128(%rsi,%r11), %ymm3
        a.vmovdqa(A::ymm3, &l);                                     // vmovdqa  16(%rip)     , %ymm3

        a.vcvttps2dq(A::ymm3, A::ymm2);
        a.vcvtdq2ps (A::ymm3, A::ymm2);
        a.vcvtps2dq (A::ymm3, A::ymm2);
        a.vsqrtps   (A::ymm3, A::ymm2);
        a.label(&l);
    },{
        0xc5,0xfd,0x6f,0xda,

        0xc5,0xfd,0x6f,0x1e,
        0xc5,0xfd,0x6f,0x1c,0x24,
        0xc4,0xc1,0x7d,0x6f,0x1b,

        0xc5,0xfd,0x6f,0x5e,0x04,
        0xc5,0xfd,0x6f,0x5c,0x24,0x04,

        0xc5,0xfd,0x6f,0x5c,0xc6,0x04,
        0xc4,0xc1,0x7d,0x6f,0x5c,0x43,0x04,
        0xc4,0xa1,0x7d,0x6f,0x5c,0x9e,0x04,
        0xc4,0xa1,0x7d,0x6f,0x5c,0x1e,0x04,
        0xc4,0xa1,0x7d,0x6f,0x5c,0x1e,0x04,

        0xc4,0xa1,0x7d,0x6f,0x5c,0x1e,0x40,
        0xc4,0xa1,0x7d,0x6f,0x9c,0x1e,0x80,0x00,0x00,0x00,

        0xc5,0xfd,0x6f,0x1d,0x10,0x00,0x00,0x00,

        0xc5,0xfe,0x5b,0xda,
        0xc5,0xfc,0x5b,0xda,
        0xc5,0xfd,0x5b,0xda,
        0xc5,0xfc,0x51,0xda,
    });

    test_asm(r, [&](A& a) {
        a.vcvtps2ph(A::xmm3, A::ymm2, A::CURRENT);
        a.vcvtps2ph(A::Mem{A::rsi, 32, A::rax, A::EIGHT}, A::ymm5, A::CEIL);

        a.vcvtph2ps(A::ymm15, A::Mem{A::rdi, 12, A::r9, A::ONE});
        a.vcvtph2ps(A::ymm2, A::xmm3);
    },{
        0xc4,0xe3,0x7d,0x1d,0xd3,0x04,
        0xc4,0xe3,0x7d,0x1d,0x6c,0xc6,0x20,0x02,

        0xc4,0x22,0x7d,0x13,0x7c,0x0f,0x0c,
        0xc4,0xe2,0x7d,0x13,0xd3,
    });

    test_asm(r, [&](A& a) {
        a.vgatherdps(A::ymm1 , A::FOUR , A::ymm0 , A::rdi, A::ymm2 );
        a.vgatherdps(A::ymm0 , A::ONE  , A::ymm2 , A::rax, A::ymm1 );
        a.vgatherdps(A::ymm10, A::ONE  , A::ymm2 , A::rax, A::ymm1 );
        a.vgatherdps(A::ymm0 , A::ONE  , A::ymm12, A::rax, A::ymm1 );
        a.vgatherdps(A::ymm0 , A::ONE  , A::ymm2 , A::r9 , A::ymm1 );
        a.vgatherdps(A::ymm0 , A::ONE  , A::ymm2 , A::rax, A::ymm12);
        a.vgatherdps(A::ymm0 , A::EIGHT, A::ymm2 , A::rax, A::ymm12);
    },{
        0xc4,0xe2,0x6d,0x92,0x0c,0x87,
        0xc4,0xe2,0x75,0x92,0x04,0x10,
        0xc4,0x62,0x75,0x92,0x14,0x10,
        0xc4,0xa2,0x75,0x92,0x04,0x20,
        0xc4,0xc2,0x75,0x92,0x04,0x11,
        0xc4,0xe2,0x1d,0x92,0x04,0x10,
        0xc4,0xe2,0x1d,0x92,0x04,0xd0,
    });

    test_asm(r, [&](A& a) {
        a.mov(A::rax, A::Mem{A::rdi,   0});
        a.mov(A::rax, A::Mem{A::rdi,   1});
        a.mov(A::rax, A::Mem{A::rdi, 512});
        a.mov(A::r15, A::Mem{A::r13,  42});
        a.mov(A::rax, A::Mem{A::r13,  42});
        a.mov(A::r15, A::Mem{A::rax,  42});
        a.mov(A::rax, 1);
        a.mov(A::rax, A::rcx);
    },{
        0x48, 0x8b, 0x07,
        0x48, 0x8b, 0x47, 0x01,
        0x48, 0x8b, 0x87, 0x00,0x02,0x00,0x00,
        0x4d, 0x8b, 0x7d, 0x2a,
        0x49, 0x8b, 0x45, 0x2a,
        0x4c, 0x8b, 0x78, 0x2a,
        0x48, 0xc7, 0xc0, 0x01,0x00,0x00,0x00,
        0x48, 0x89, 0xc8,
    });

    // echo "fmul v4.4s, v3.4s, v1.4s" | llvm-mc -show-encoding -arch arm64

    test_asm(r, [&](A& a) {
        a.and16b(A::v4, A::v3, A::v1);
        a.orr16b(A::v4, A::v3, A::v1);
        a.eor16b(A::v4, A::v3, A::v1);
        a.bic16b(A::v4, A::v3, A::v1);
        a.bsl16b(A::v4, A::v3, A::v1);
        a.not16b(A::v4, A::v3);

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
        a.fmin4s(A::v4, A::v3, A::v1);
        a.fmax4s(A::v4, A::v3, A::v1);

        a.fneg4s (A::v4, A::v3);
        a.fsqrt4s(A::v4, A::v3);

        a.fmla4s(A::v4, A::v3, A::v1);
        a.fmls4s(A::v4, A::v3, A::v1);

        a.fcmeq4s(A::v4, A::v3, A::v1);
        a.fcmgt4s(A::v4, A::v3, A::v1);
        a.fcmge4s(A::v4, A::v3, A::v1);
    },{
        0x64,0x1c,0x21,0x4e,
        0x64,0x1c,0xa1,0x4e,
        0x64,0x1c,0x21,0x6e,
        0x64,0x1c,0x61,0x4e,
        0x64,0x1c,0x61,0x6e,
        0x64,0x58,0x20,0x6e,

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
        0x64,0xf4,0xa1,0x4e,
        0x64,0xf4,0x21,0x4e,

        0x64,0xf8,0xa0,0x6e,
        0x64,0xf8,0xa1,0x6e,

        0x64,0xcc,0x21,0x4e,
        0x64,0xcc,0xa1,0x4e,

        0x64,0xe4,0x21,0x4e,
        0x64,0xe4,0xa1,0x6e,
        0x64,0xe4,0x21,0x6e,
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
        a.fcvtns4s(A::v4, A::v3);
        a.frintp4s(A::v4, A::v3);
        a.frintm4s(A::v4, A::v3);
        a.fcvtn   (A::v4, A::v3);
        a.fcvtl   (A::v4, A::v3);
    },{
        0x64,0xd8,0x21,0x4e,
        0x64,0xb8,0xa1,0x4e,
        0x64,0xa8,0x21,0x4e,
        0x64,0x88,0xa1,0x4e,
        0x64,0x98,0x21,0x4e,
        0x64,0x68,0x21,0x0e,
        0x64,0x78,0x21,0x0e,
    });

    test_asm(r, [&](A& a) {
        a.sub (A::sp, A::sp, 32);  // sub   sp, sp, #32
        a.strq(A::v0, A::sp, 1);   // str   q0, [sp, #16]
        a.strq(A::v1, A::sp);      // str   q1, [sp]
        a.strd(A::v0, A::sp, 6);   // str   s0, [sp, #48]
        a.strs(A::v0, A::sp, 6);   // str   s0, [sp, #24]
        a.strh(A::v0, A::sp, 10);  // str   h0, [sp, #20]
        a.strb(A::v0, A::sp, 47);  // str   b0, [sp, #47]
        a.ldrb(A::v9, A::sp, 42);  // ldr   b9, [sp, #42]
        a.ldrh(A::v9, A::sp, 47);  // ldr   h9, [sp, #94]
        a.ldrs(A::v7, A::sp, 10);  // ldr   s7, [sp, #40]
        a.ldrd(A::v7, A::sp,  1);  // ldr   d7, [sp, #8]
        a.ldrq(A::v5, A::sp, 128); // ldr   q5, [sp, #2048]
        a.add (A::sp, A::sp, 32);  // add   sp, sp, #32
    },{
         0xff,0x83,0x00,0xd1,
         0xe0,0x07,0x80,0x3d,
         0xe1,0x03,0x80,0x3d,
         0xe0,0x1b,0x00,0xfd,
         0xe0,0x1b,0x00,0xbd,
         0xe0,0x2b,0x00,0x7d,
         0xe0,0xbf,0x00,0x3d,
         0xe9,0xab,0x40,0x3d,
         0xe9,0xbf,0x40,0x7d,
         0xe7,0x2b,0x40,0xbd,
         0xe7,0x07,0x40,0xfd,
         0xe5,0x03,0xc2,0x3d,
         0xff,0x83,0x00,0x91,
    });

    test_asm(r, [&](A& a) {
        a.brk(0);
        a.brk(65535);

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

        A::Label l;
        a.label(&l);
        a.bne(&l);
        a.bne(&l);
        a.blt(&l);
        a.b(&l);
        a.cbnz(A::x2, &l);
        a.cbz(A::x2, &l);

        a.add(A::x3, A::x2, A::x1);             // add x3,x2,x1
        a.add(A::x3, A::x2, A::x1, A::ASR, 3);  // add x3,x2,x1, asr #3
    },{
        0x00,0x00,0x20,0xd4,
        0xe0,0xff,0x3f,0xd4,

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

        0x43,0x00,0x01,0x8b,
        0x43,0x0c,0x81,0x8b,
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
        A::Label l1;
        a.label(&l1);
        a.add(A::x3, A::x2, 32);
        a.cbz(A::x2, &l1);          // This will jump backward... nothing sneaky.

        A::Label l2;                // Start off the same...
        a.label(&l2);
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
        a.dup4s  (A::v0, A::x8);
        a.ld1r4s (A::v0, A::x8);  // echo 'ld1r.4s {v0}, [x8]' | llvm-mc --show-encoding
        a.ld1r8h (A::v0, A::x8);
        a.ld1r16b(A::v0, A::x8);
    },{
        0x00,0x0d,0x04,0x4e,
        0x00,0xc9,0x40,0x4d,
        0x00,0xc5,0x40,0x4d,
        0x00,0xc1,0x40,0x4d,
    });

    test_asm(r, [&](A& a) {
        a.ld24s(A::v0, A::x8);  // echo 'ld2.4s {v0,v1}, [x8]' | llvm-mc --show-encoding
        a.ld44s(A::v0, A::x8);
        a.st24s(A::v0, A::x8);
        a.st44s(A::v0, A::x8);  // echo 'st4.4s {v0,v1,v2,v3}, [x8]' | llvm-mc --show-encoding

        a.ld24s(A::v0, A::x8, 0);  //echo 'ld2 {v0.s,v1.s}[0], [x8]' | llvm-mc --show-encoding
        a.ld24s(A::v0, A::x8, 1);
        a.ld24s(A::v0, A::x8, 2);
        a.ld24s(A::v0, A::x8, 3);

        a.ld44s(A::v0, A::x8, 0);  // ld4 {v0.s,v1.s,v2.s,v3.s}[0], [x8]
        a.ld44s(A::v0, A::x8, 1);
        a.ld44s(A::v0, A::x8, 2);
        a.ld44s(A::v0, A::x8, 3);
    },{
        0x00,0x89,0x40,0x4c,
        0x00,0x09,0x40,0x4c,
        0x00,0x89,0x00,0x4c,
        0x00,0x09,0x00,0x4c,

        0x00,0x81,0x60,0x0d,
        0x00,0x91,0x60,0x0d,
        0x00,0x81,0x60,0x4d,
        0x00,0x91,0x60,0x4d,

        0x00,0xa1,0x60,0x0d,
        0x00,0xb1,0x60,0x0d,
        0x00,0xa1,0x60,0x4d,
        0x00,0xb1,0x60,0x4d,
    });

    test_asm(r, [&](A& a) {
        a.xtns2h(A::v0, A::v0);
        a.xtnh2b(A::v0, A::v0);
        a.strs  (A::v0, A::x0);

        a.ldrs   (A::v0, A::x0);
        a.uxtlb2h(A::v0, A::v0);
        a.uxtlh2s(A::v0, A::v0);

        a.uminv4s(A::v3, A::v4);
        a.movs   (A::x3, A::v4,0);  // mov.s w3,v4[0]
        a.movs   (A::x3, A::v4,1);  // mov.s w3,v4[1]
        a.inss   (A::v4, A::x3,3);  // ins.s v4[3],w3
    },{
        0x00,0x28,0x61,0x0e,
        0x00,0x28,0x21,0x0e,
        0x00,0x00,0x00,0xbd,

        0x00,0x00,0x40,0xbd,
        0x00,0xa4,0x08,0x2f,
        0x00,0xa4,0x10,0x2f,

        0x83,0xa8,0xb1,0x6e,
        0x83,0x3c,0x04,0x0e,
        0x83,0x3c,0x0c,0x0e,
        0x64,0x1c,0x1c,0x4e,
    });

    test_asm(r, [&](A& a) {
        a.ldrb(A::v0, A::x8);
        a.strb(A::v0, A::x8);
    },{
        0x00,0x01,0x40,0x3d,
        0x00,0x01,0x00,0x3d,
    });

    test_asm(r, [&](A& a) {
        a.ldrd(A::x0, A::x1, 3);   // ldr  x0, [x1, #24]
        a.ldrs(A::x0, A::x1, 3);   // ldr  w0, [x1, #12]
        a.ldrh(A::x0, A::x1, 3);   // ldrh w0, [x1, #6]
        a.ldrb(A::x0, A::x1, 3);   // ldrb w0, [x1, #3]

        a.strs(A::x0, A::x1, 3);   // str  w0, [x1, #12]
    },{
        0x20,0x0c,0x40,0xf9,
        0x20,0x0c,0x40,0xb9,
        0x20,0x0c,0x40,0x79,
        0x20,0x0c,0x40,0x39,

        0x20,0x0c,0x00,0xb9,
    });

    test_asm(r, [&](A& a) {
        a.tbl   (A::v0, A::v1, A::v2);
        a.uzp14s(A::v0, A::v1, A::v2);
        a.uzp24s(A::v0, A::v1, A::v2);
        a.zip14s(A::v0, A::v1, A::v2);
        a.zip24s(A::v0, A::v1, A::v2);
    },{
        0x20,0x00,0x02,0x4e,
        0x20,0x18,0x82,0x4e,
        0x20,0x58,0x82,0x4e,
        0x20,0x38,0x82,0x4e,
        0x20,0x78,0x82,0x4e,
    });
}

DEF_TEST(SkVM_approx_math, r) {
    auto eval = [](int N, float values[], auto fn) {
        skvm::Builder b;
        skvm::Ptr inout  = b.varying<float>();

        b.storeF(inout, fn(&b, b.loadF(inout)));

        b.done().eval(N, values);
    };

    auto compare = [r](int N, const float values[], const float expected[]) {
        for (int i = 0; i < N; ++i) {
            REPORTER_ASSERT(r, SkScalarNearlyEqual(values[i], expected[i], 0.001f));
        }
    };

    // log2
    {
        float values[] = {0.25f, 0.5f, 1, 2, 4, 8};
        constexpr int N = SK_ARRAY_COUNT(values);
        eval(N, values, [](skvm::Builder* b, skvm::F32 v) {
            return b->approx_log2(v);
        });
        const float expected[] = {-2, -1, 0, 1, 2, 3};
        compare(N, values, expected);
    }

    // pow2
    {
        float values[] = {-2, -1, 0, 1, 2, 3};
        constexpr int N = SK_ARRAY_COUNT(values);
        eval(N, values, [](skvm::Builder* b, skvm::F32 v) {
            return b->approx_pow2(v);
        });
        const float expected[] = {0.25f, 0.5f, 1, 2, 4, 8};
        compare(N, values, expected);
    }

    // powf -- x^0.5
    {
        float bases[] = {0, 1, 4, 9, 16};
        constexpr int N = SK_ARRAY_COUNT(bases);
        eval(N, bases, [](skvm::Builder* b, skvm::F32 base) {
            return b->approx_powf(base, b->splat(0.5f));
        });
        const float expected[] = {0, 1, 2, 3, 4};
        compare(N, bases, expected);
    }
    // powf -- 3^x
    {
        float exps[] = {-2, -1, 0, 1, 2};
        constexpr int N = SK_ARRAY_COUNT(exps);
        eval(N, exps, [](skvm::Builder* b, skvm::F32 exp) {
            return b->approx_powf(b->splat(3.0f), exp);
        });
        const float expected[] = {1/9.0f, 1/3.0f, 1, 3, 9};
        compare(N, exps, expected);
    }

    auto test = [r](float arg, float expected, float tolerance, auto prog) {
        skvm::Builder b;
        skvm::Ptr inout  = b.varying<float>();
        b.storeF(inout, prog(b.loadF(inout)));
        float actual = arg;
        b.done().eval(1, &actual);

        float err = std::abs(actual - expected);

        if (err > tolerance) {
    //        SkDebugf("arg %g, expected %g, actual %g\n", arg, expected, actual);
            REPORTER_ASSERT(r, true);
        }
        return err;
    };

    auto test2 = [r](float arg0, float arg1, float expected, float tolerance, auto prog) {
        skvm::Builder b;
        skvm::Ptr in0  = b.varying<float>();
        skvm::Ptr in1  = b.varying<float>();
        skvm::Ptr out  = b.varying<float>();
        b.storeF(out, prog(b.loadF(in0), b.loadF(in1)));
        float actual;
        b.done().eval(1, &arg0, &arg1, &actual);

        float err = std::abs(actual - expected);

        if (err > tolerance) {
    //        SkDebugf("[%g, %g]: expected %g, actual %g\n", arg0, arg1, expected, actual);
            REPORTER_ASSERT(r, true);
        }
        return err;
    };

    // sine, cosine, tangent
    {
        constexpr float P = SK_ScalarPI;
        constexpr float tol = 0.00175f;
        for (float rad = -5*P; rad <= 5*P; rad += 0.1f) {
            test(rad, sk_float_sin(rad), tol, [](skvm::F32 x) {
                return approx_sin(x);
            });
            test(rad, sk_float_cos(rad), tol, [](skvm::F32 x) {
                return approx_cos(x);
            });
        }

        // Our tangent diverge more as we get near infinities (x near +- Pi/2),
        // so bring in the domain a little.
        constexpr float eps = 0.16f;
        float err = 0;
        for (float rad = -P/2 + eps; rad <= P/2 - eps; rad += 0.01f) {
            err += test(rad, sk_float_tan(rad), tol, [](skvm::F32 x) {
                return approx_tan(x);
            });
            // try again with some multiples of P, to check our periodicity
            test(rad, sk_float_tan(rad), tol, [=](skvm::F32 x) {
                return approx_tan(x + 3*P);
            });
            test(rad, sk_float_tan(rad), tol, [=](skvm::F32 x) {
                return approx_tan(x - 3*P);
            });
        }
        if (0) { SkDebugf("tan error %g\n", err); }
    }

    // asin, acos, atan
    {
        constexpr float tol = 0.00175f;
        float err = 0;
        for (float x = -1; x <= 1; x += 1.0f/64) {
            err += test(x, asin(x), tol, [](skvm::F32 x) {
                return approx_asin(x);
            });
            test(x, acos(x), tol, [](skvm::F32 x) {
                return approx_acos(x);
            });
        }
        if (0) { SkDebugf("asin error %g\n", err); }

        err = 0;
        for (float x = -10; x <= 10; x += 1.0f/16) {
            err += test(x, atan(x), tol, [](skvm::F32 x) {
                return approx_atan(x);
            });
        }
        if (0) { SkDebugf("atan error %g\n", err); }

        for (float y = -3; y <= 3; y += 1) {
            for (float x = -3; x <= 3; x += 1) {
                err += test2(y, x, atan2(y,x), tol, [](skvm::F32 y, skvm::F32 x) {
                    return approx_atan2(y,x);
                });
            }
        }
        if (0) { SkDebugf("atan2 error %g\n", err); }
    }
}

DEF_TEST(SkVM_min_max, r) {
    // min() and max() have subtle behavior when one argument is NaN and
    // the other isn't.  It's not sound to blindly swap their arguments.
    //
    // All backends must behave like std::min() and std::max(), which are
    //
    //    min(x,y) = y<x ? y : x
    //    max(x,y) = x<y ? y : x

    // ±NaN, ±0, ±1, ±inf
    const uint32_t bits[] = {0x7f80'0001, 0xff80'0001, 0x0000'0000, 0x8000'0000,
                             0x3f80'0000, 0xbf80'0000, 0x7f80'0000, 0xff80'0000};

    float f[8];
    memcpy(f, bits, sizeof(bits));

    auto identical = [&](float x, float y) {
        uint32_t X,Y;
        memcpy(&X, &x, 4);
        memcpy(&Y, &y, 4);
        return X == Y;
    };

    // Test min/max with non-constant x, non-constant y.
    // (Whether x and y are varying or uniform shouldn't make any difference.)
    {
        skvm::Builder b;
        {
            skvm::Ptr src = b.varying<float>(),
                       mn = b.varying<float>(),
                       mx = b.varying<float>();

            skvm::F32 x = b.loadF(src),
                      y = b.uniformF(b.uniform(), 0);

            b.storeF(mn, b.min(x,y));
            b.storeF(mx, b.max(x,y));
        }

        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            float mn[8], mx[8];
            for (int i = 0; i < 8; i++) {
                // min() and max() everything with f[i].
                program.eval(8, f,mn,mx, &f[i]);

                for (int j = 0; j < 8; j++) {
                    REPORTER_ASSERT(r, identical(mn[j], std::min(f[j], f[i])));
                    REPORTER_ASSERT(r, identical(mx[j], std::max(f[j], f[i])));
                }
            }
        });
    }

    // Test each with constant on the right.
    for (int i = 0; i < 8; i++) {
        skvm::Builder b;
        {
            skvm::Ptr src = b.varying<float>(),
                       mn = b.varying<float>(),
                       mx = b.varying<float>();

            skvm::F32 x = b.loadF(src),
                      y = b.splat(f[i]);

            b.storeF(mn, b.min(x,y));
            b.storeF(mx, b.max(x,y));
        }

        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            float mn[8], mx[8];
            program.eval(8, f,mn,mx);
            for (int j = 0; j < 8; j++) {
                REPORTER_ASSERT(r, identical(mn[j], std::min(f[j], f[i])));
                REPORTER_ASSERT(r, identical(mx[j], std::max(f[j], f[i])));
            }
        });
    }

    // Test each with constant on the left.
    for (int i = 0; i < 8; i++) {
        skvm::Builder b;
        {
            skvm::Ptr src = b.varying<float>(),
                       mn = b.varying<float>(),
                       mx = b.varying<float>();

            skvm::F32 x = b.splat(f[i]),
                      y = b.loadF(src);

            b.storeF(mn, b.min(x,y));
            b.storeF(mx, b.max(x,y));
        }

        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            float mn[8], mx[8];
            program.eval(8, f,mn,mx);
            for (int j = 0; j < 8; j++) {
                REPORTER_ASSERT(r, identical(mn[j], std::min(f[i], f[j])));
                REPORTER_ASSERT(r, identical(mx[j], std::max(f[i], f[j])));
            }
        });
    }
}

DEF_TEST(SkVM_halfs, r) {
    const uint16_t hs[] = {0x0000,0x3800,0x3c00,0x4000,
                           0xc400,0xb800,0xbc00,0xc000};
    const float fs[] = {+0.0f,+0.5f,+1.0f,+2.0f,
                        -4.0f,-0.5f,-1.0f,-2.0f};
    {
        skvm::Builder b;
        skvm::Ptr src = b.varying<uint16_t>(),
                  dst = b.varying<float>();
        b.storeF(dst, b.from_fp16(b.load16(src)));

        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            float dst[8];
            program.eval(8, hs, dst);
            for (int i = 0; i < 8; i++) {
                REPORTER_ASSERT(r, dst[i] == fs[i]);
            }
        });
    }
    {
        skvm::Builder b;
        skvm::Ptr src = b.varying<float>(),
                  dst = b.varying<uint16_t>();
        b.store16(dst, b.to_fp16(b.loadF(src)));

        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            uint16_t dst[8];
            program.eval(8, fs, dst);
            for (int i = 0; i < 8; i++) {
                REPORTER_ASSERT(r, dst[i] == hs[i]);
            }
        });
    }
}

DEF_TEST(SkVM_64bit, r) {
    uint32_t lo[65],
             hi[65];
    uint64_t wide[65];
    for (int i = 0; i < 65; i++) {
        lo[i] = 2*i+0;
        hi[i] = 2*i+1;
        wide[i] = ((uint64_t)lo[i] <<  0)
                | ((uint64_t)hi[i] << 32);
    }

    {
        skvm::Builder b;
        {
            skvm::Ptr widePtr = b.varying<uint64_t>(),
                        loPtr = b.varying<int>(),
                        hiPtr = b.varying<int>();
            b.store32(loPtr, b.load64(widePtr, 0));
            b.store32(hiPtr, b.load64(widePtr, 1));
        }
        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            uint32_t l[65], h[65];
            program.eval(65, wide,l,h);
            for (int i = 0; i < 65; i++) {
                REPORTER_ASSERT(r, l[i] == lo[i]);
                REPORTER_ASSERT(r, h[i] == hi[i]);
            }
        });
    }

    {
        skvm::Builder b;
        {
            skvm::Ptr widePtr = b.varying<uint64_t>(),
                        loPtr = b.varying<int>(),
                        hiPtr = b.varying<int>();
            b.store64(widePtr, b.load32(loPtr), b.load32(hiPtr));
        }
        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            uint64_t w[65];
            program.eval(65, w,lo,hi);
            for (int i = 0; i < 65; i++) {
                REPORTER_ASSERT(r, w[i] == wide[i]);
            }
        });
    }
}

DEF_TEST(SkVM_128bit, r) {
    float   floats[4*63];
    uint8_t packed[4*63];

    for (int i = 0; i < 4*63; i++) {
        floats[i] = i * (1/255.0f);
    }

    skvm::PixelFormat rgba_ffff = skvm::SkColorType_to_PixelFormat(kRGBA_F32_SkColorType),
                      rgba_8888 = skvm::SkColorType_to_PixelFormat(kRGBA_8888_SkColorType);

    {  // Convert RGBA F32 to RGBA 8888, testing 128-bit loads.
        skvm::Builder b;
        {
            skvm::Ptr dst = b.varying(4),
                      src = b.varying(16);

            skvm::Color c = b.load(rgba_ffff, src);
            b.store(rgba_8888, dst, c);
        }
        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            memset(packed, 0, sizeof(packed));
            program.eval(63, packed, floats);
            for (int i = 0; i < 4*63; i++) {
                REPORTER_ASSERT(r, packed[i] == i);
            }
        });
    }


    {  // Convert RGBA 8888 to RGBA F32, testing 128-bit stores.
        skvm::Builder b;
        {
            skvm::Ptr dst = b.varying(16),
                      src = b.varying(4);

            skvm::Color c = b.load(rgba_8888, src);
            b.store(rgba_ffff, dst, c);
        }
        test_jit_and_interpreter(b, [&](const skvm::Program& program){
            memset(floats, 0, sizeof(floats));
            program.eval(63, floats, packed);
            for (int i = 0; i < 4*63; i++) {
                REPORTER_ASSERT(r, floats[i] == i * (1/255.0f));
            }
        });
    }

}

DEF_TEST(SkVM_is_NaN_is_finite, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>(),
                  nan = b.varying<int>(),
                  fin = b.varying<int>();
        b.store32(nan, is_NaN   (b.loadF(src)));
        b.store32(fin, is_finite(b.loadF(src)));
    }
    test_jit_and_interpreter(b, [&](const skvm::Program& program){
        // ±NaN, ±0, ±1, ±inf
        const uint32_t bits[] = {0x7f80'0001, 0xff80'0001, 0x0000'0000, 0x8000'0000,
                                 0x3f80'0000, 0xbf80'0000, 0x7f80'0000, 0xff80'0000};
        uint32_t nan[8], fin[8];
        program.eval(8, bits, nan,fin);

        for (int i = 0; i < 8; i++) {
            REPORTER_ASSERT(r, nan[i] == ((i == 0 || i == 1) ? 0xffffffff : 0));
            REPORTER_ASSERT(r, fin[i] == ((i == 2 || i == 3 ||
                                           i == 4 || i == 5) ? 0xffffffff : 0));
        }
    });
}

DEF_TEST(SkVM_args, r) {
    // Test we can handle at least six arguments.
    skvm::Builder b;
    {
        skvm::Ptr dst = b.varying<float>(),
                    A = b.varying<float>(),
                    B = b.varying<float>(),
                    C = b.varying<float>(),
                    D = b.varying<float>(),
                    E = b.varying<float>();
        storeF(dst, b.loadF(A)
                  + b.loadF(B)
                  + b.loadF(C)
                  + b.loadF(D)
                  + b.loadF(E));
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program){
        float dst[17],A[17],B[17],C[17],D[17],E[17];
        for (int i = 0; i < 17; i++) {
            A[i] = B[i] = C[i] = D[i] = E[i] = (float)i;
        }
        program.eval(17, dst,A,B,C,D,E);
        for (int i = 0; i < 17; i++) {
            REPORTER_ASSERT(r, dst[i] == 5.0f*i);
        }
    });
}

DEF_TEST(SkVM_badpack, reporter) {
    // Test case distilled from actual failing draw,
    // originally with a bad arm64 implementation of pack().
    skvm::Builder p;
    {
        skvm::UPtr uniforms = p.uniform();
        skvm::Ptr dst = p.varying<uint16_t>();

        skvm::I32 r = round(p.uniformF(uniforms, 8) * 15),
                  a = p.splat(0xf);

        skvm::I32 _4444 = p.splat(0);
        _4444 = pack(_4444, r, 12);
        _4444 = pack(_4444, a,  0);
        store16(dst, _4444);
    }

    test_jit_and_interpreter(p, [&](const skvm::Program& program){
        const float uniforms[] = { 0.0f, 0.0f,
                                   1.0f, 0.0f, 0.0f, 1.0f };

        uint16_t dst[17] = {0};
        program.eval(17, uniforms,dst);
        for (int i = 0; i < 17; i++) {
            REPORTER_ASSERT(reporter, dst[i] == 0xf00f, "got %04x, want %04x\n", dst[i], 0xf00f);
        }
    });
}

DEF_TEST(SkVM_features, r) {
    auto build_program = [](skvm::Builder* b) {
        skvm::F32 x = b->loadF(b->varying<float>());
        b->storeF(b->varying<float>(), x*x+x);
    };

    {   // load-fma-store with FMA available.
        skvm::Features features;
        features.fma = true;
        skvm::Builder b(features);
        build_program(&b);
        REPORTER_ASSERT(r, b.optimize().size() == 3);
    }

    {   // load-mul-add-store without FMA.
        skvm::Features features;
        features.fma = false;
        skvm::Builder b(features);
        build_program(&b);
        REPORTER_ASSERT(r, b.optimize().size() == 4);
    }

    {   // Auto-detected, could be either.
        skvm::Builder b;
        build_program(&b);
        REPORTER_ASSERT(r, b.optimize().size() == 3
                        || b.optimize().size() == 4);
    }
}

DEF_TEST(SkVM_gather_can_hoist, r) {
    // A gather instruction isn't necessarily varying... it's whatever its index is.
    // First a typical gather scenario with varying index.
    {
        skvm::Builder b;
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf = b.varying<int>();
        skvm::I32 ix = b.load32(buf);
        b.store32(buf, b.gather32(uniforms,0, ix));

        skvm::Program p = b.done();

        // ix is varying, so the gather is too.
        //
        // loop:
        //     v0 = load32 buf
        //     v1 = gather32 uniforms+0 v0
        //     store32 buf v1
        REPORTER_ASSERT(r, p.instructions().size() == 3);
        REPORTER_ASSERT(r, p.loop() == 0);
    }

    // Now the same but with a uniform index instead.
    {
        skvm::Builder b;
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf = b.varying<int>();
        skvm::I32 ix = b.uniform32(uniforms,8);
        b.store32(buf, b.gather32(uniforms,0, ix));

        skvm::Program p = b.done();

        // ix is uniform, so the gather is too.
        //
        // v0 = uniform32 uniforms+8
        // v1 = gather32 uniforms+0 v0
        // loop:
        //     store32 buf v1
        REPORTER_ASSERT(r, p.instructions().size() == 3);
        REPORTER_ASSERT(r, p.loop() == 2);
    }
}

DEF_TEST(SkVM_dont_dedup_loads, r) {
    // We've been assuming that all Ops with the same arguments produce the same value
    // and deduplicating them, which results in a simple common subexpression eliminator.
    //
    // But we can't soundly dedup two identical loads with a store between.
    // If we dedup the loads in this test program it will always increment by 1, not K.
    constexpr int K = 2;
    skvm::Builder b;
    {
        skvm::Ptr buf = b.varying<int>();
        for (int i = 0; i < K; i++) {
            b.store32(buf, b.load32(buf) + 1);
        }
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program){
        int buf[] = { 0,1,2,3,4 };
        program.eval(SK_ARRAY_COUNT(buf), buf);
        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == i+K);
        }
    });
}

DEF_TEST(SkVM_dont_dedup_stores, r) {
    // Following a similar line of reasoning to SkVM_dont_dedup_loads,
    // we cannot dedup stores either.  A different store between two identical stores
    // will invalidate the first store, meaning we do need to reissue that store operation.
    skvm::Builder b;
    {
        skvm::Ptr buf = b.varying<int>();
        b.store32(buf, b.splat(4));
        b.store32(buf, b.splat(5));
        b.store32(buf, b.splat(4));   // If we dedup'd, we'd skip this store.
    }

    test_jit_and_interpreter(b, [&](const skvm::Program& program){
        int buf[42];
        program.eval(SK_ARRAY_COUNT(buf), buf);
        for (int x : buf) {
            REPORTER_ASSERT(r, x == 4);
        }
    });
}

DEF_TEST(SkVM_fast_mul, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>(),
                 fast = b.varying<float>(),
                 slow = b.varying<float>();
        skvm::F32 x = b.loadF(src);
        b.storeF(fast, fast_mul(0.0f, x));
        b.storeF(slow, 0.0f * x);
    }
    test_jit_and_interpreter(b, [&](const skvm::Program& program){
        const uint32_t bits[] = {
            0x0000'0000, 0x8000'0000, //±0
            0x3f80'0000, 0xbf80'0000, //±1
            0x7f80'0000, 0xff80'0000, //±inf
            0x7f80'0001, 0xff80'0001, //±NaN
        };
        float fast[8],
              slow[8];
        program.eval(8,bits,fast,slow);

        for (int i = 0; i < 8; i++) {
            REPORTER_ASSERT(r, fast[i] == 0.0f);

            if (i < 4) {
                REPORTER_ASSERT(r, slow[i] == 0.0f);
            } else {
                REPORTER_ASSERT(r, isnan(slow[i]));
            }
        }
    });
}
