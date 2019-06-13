/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_opts_DEFINED
#define SkVM_opts_DEFINED

#include "src/core/SkVM.h"

namespace SK_OPTS_NS {

    inline void eval(const skvm::Program::Instruction insts[], const int ninsts,
                     const int nregs, const int loop,
                     int n, void* args[], size_t strides[], const int nargs) {
        using namespace skvm;

        // We'll operate in SIMT style, knocking off K-size chunks from n while possible.
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
        constexpr int K = 32;
    #else
        constexpr int K = 16;
    #endif
        using I32 = skvx::Vec<K, int>;
        using F32 = skvx::Vec<K, float>;
        using U32 = skvx::Vec<K, uint32_t>;
        using  U8 = skvx::Vec<K, uint8_t>;

        using I16x2 = skvx::Vec<2*K, int16_t>;

        union Slot {
            I32 i32;
            U32 u32;
            F32 f32;
        };

        Slot                     few_regs[16];
        std::unique_ptr<char[]> many_regs;

        Slot* regs = few_regs;

        if (nregs > (int)SK_ARRAY_COUNT(few_regs)) {
            // Annoyingly we can't trust that malloc() or new will work with Slot because
            // the skvx::Vec types may have alignment greater than what they provide.
            // We'll overallocate one extra register so we can align manually.
            many_regs.reset(new char[ sizeof(Slot) * (nregs + 1) ]);

            uintptr_t addr = (uintptr_t)many_regs.get();
            addr += alignof(Slot) -
                     (addr & (alignof(Slot) - 1));
            SkASSERT((addr & (alignof(Slot) - 1)) == 0);
            regs = (Slot*)addr;
        }


        auto r = [&](ID id) -> Slot& {
            SkASSERT(0 <= id && id < nregs);
            return regs[id];
        };
        auto arg = [&](int ix) {
            SkASSERT(0 <= ix && ix < nargs);
            return args[ix];
        };

        // Step each argument pointer ahead by its stride a number of times.
        auto step_args = [&](int times) {
            // Looping by marching pointers until *arg == nullptr helps the
            // compiler to keep this loop scalar.  Otherwise it'd create a
            // rather large and useless autovectorized version.
            void**        arg    = args;
            const size_t* stride = strides;
            for (; *arg; arg++, stride++) {
                *arg = (void*)( (char*)*arg + times * *stride );
            }
            SkASSERT(arg == args + nargs);
        };

        int start = 0,
            stride;
        for ( ; n > 0; start = loop, n -= stride, step_args(stride)) {
            stride = n >= K ? K : 1;

            for (int i = start; i < ninsts; i++) {
                skvm::Program::Instruction inst = insts[i];

                // d = op(x, y.id/z.imm, z.id/z.imm)
                ID   d = inst.d,
                     x = inst.x;
                auto y = inst.y,
                     z = inst.z;

                // Ops that interact with memory need to know whether we're stride=1 or stride=K,
                // but all non-memory ops can run the same code no matter the stride.
                switch (2*(int)inst.op + (stride == K ? 1 : 0)) {

                #define STRIDE_1(op) case 2*(int)op
                #define STRIDE_K(op) case 2*(int)op + 1
                    STRIDE_1(Op::store8 ): memcpy(arg(y.imm), &r(x).i32, 1); break;
                    STRIDE_1(Op::store32): memcpy(arg(y.imm), &r(x).i32, 4); break;

                    STRIDE_K(Op::store8 ): skvx::cast<uint8_t>(r(x).i32).store(arg(y.imm)); break;
                    STRIDE_K(Op::store32):                    (r(x).i32).store(arg(y.imm)); break;

                    STRIDE_1(Op::load8 ): r(d).i32 = 0; memcpy(&r(d).i32, arg(y.imm), 1); break;
                    STRIDE_1(Op::load32): r(d).i32 = 0; memcpy(&r(d).i32, arg(y.imm), 4); break;

                    STRIDE_K(Op::load8 ): r(d).i32 = skvx::cast<int>(U8 ::Load(arg(y.imm))); break;
                    STRIDE_K(Op::load32): r(d).i32 =                 I32::Load(arg(y.imm)) ; break;
                #undef STRIDE_1
                #undef STRIDE_K

                    // Ops that don't interact with memory should never care about the stride.
                #define CASE(op) case 2*(int)op: /*fallthrough*/ case 2*(int)op+1
                    CASE(Op::splat): r(d).i32 = y.imm; break;

                    CASE(Op::add_f32): r(d).f32 = r(x).f32 + r(y.id).f32; break;
                    CASE(Op::sub_f32): r(d).f32 = r(x).f32 - r(y.id).f32; break;
                    CASE(Op::mul_f32): r(d).f32 = r(x).f32 * r(y.id).f32; break;
                    CASE(Op::div_f32): r(d).f32 = r(x).f32 / r(y.id).f32; break;

                    CASE(Op::mad_f32): r(d).f32 = r(x).f32 * r(y.id).f32 + r(z.id).f32; break;

                    CASE(Op::add_i32): r(d).i32 = r(x).i32 + r(y.id).i32; break;
                    CASE(Op::sub_i32): r(d).i32 = r(x).i32 - r(y.id).i32; break;
                    CASE(Op::mul_i32): r(d).i32 = r(x).i32 * r(y.id).i32; break;

                    CASE(Op::mul_i16x2):
                        r(d).i32 = skvx::bit_pun<I32>(skvx::bit_pun<I16x2>(r(x   ).i32) *
                                                      skvx::bit_pun<I16x2>(r(y.id).i32) ); break;

                    CASE(Op::bit_and): r(d).i32 = r(x).i32 & r(y.id).i32; break;
                    CASE(Op::bit_or ): r(d).i32 = r(x).i32 | r(y.id).i32; break;
                    CASE(Op::bit_xor): r(d).i32 = r(x).i32 ^ r(y.id).i32; break;

                    CASE(Op::shl): r(d).i32 = r(x).i32 << y.imm; break;
                    CASE(Op::sra): r(d).i32 = r(x).i32 >> y.imm; break;
                    CASE(Op::shr): r(d).u32 = r(x).u32 >> y.imm; break;

                    CASE(Op::extract): r(d).u32 = (r(x).u32 >> y.imm) & r(z.id).u32; break;
                    CASE(Op::pack):    r(d).u32 = r(x).u32 | (r(y.id).u32 << z.imm); break;

                    CASE(Op::to_f32): r(d).f32 = skvx::cast<float>(r(x).i32); break;
                    CASE(Op::to_i32): r(d).i32 = skvx::cast<int>  (r(x).f32); break;
                #undef CASE
                }
            }
        }
    }

}  // namespace SK_OPTS_NS

#endif//SkVM_opts_DEFINED
