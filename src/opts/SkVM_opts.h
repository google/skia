// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkVM_opts_DEFINED
#define SkVM_opts_DEFINED

#include "include/private/SkVx.h"
#include "src/core/SkVM.h"

namespace SK_OPTS_NS {

    inline void interpret_skvm(const skvm::InterpreterInstruction insts[], const int ninsts,
                               const int nregs, const int loop,
                               const int strides[], const int nargs,
                               int n, void* args[]) {
        using namespace skvm;

        // We'll operate in SIMT style, knocking off K-size chunks from n while possible.
        // We noticed quad-pumping is slower than single-pumping and both were slower than double.
    #if defined(__AVX2__)
        constexpr int K = 16;
    #else
        constexpr int K = 8;
    #endif
        using I32 = skvx::Vec<K, int>;
        using F32 = skvx::Vec<K, float>;
        using U32 = skvx::Vec<K, uint32_t>;
        using U16 = skvx::Vec<K, uint16_t>;
        using  U8 = skvx::Vec<K, uint8_t>;

        using I16x2 = skvx::Vec<2*K,  int16_t>;
        using U16x2 = skvx::Vec<2*K, uint16_t>;

        union Slot {
            F32   f32;
            I32   i32;
            U32   u32;
            I16x2 i16x2;
            U16x2 u16x2;
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


        auto r = [&](Reg id) -> Slot& {
            SkASSERT(0 <= id && id < nregs);
            return regs[id];
        };
        auto arg = [&](int ix) {
            SkASSERT(0 <= ix && ix < nargs);
            return args[ix];
        };

        // Step each argument pointer ahead by its stride a number of times.
        auto step_args = [&](int times) {
            for (int i = 0; i < nargs; i++) {
                args[i] = (void*)( (char*)args[i] + times * strides[i] );
            }
        };

        int start = 0,
            stride;
        for ( ; n > 0; start = loop, n -= stride, step_args(stride)) {
            stride = n >= K ? K : 1;

            for (int i = start; i < ninsts; i++) {
                InterpreterInstruction inst = insts[i];

                // d = op(x,y/imm,z/imm)
                Reg   d = inst.d,
                      x = inst.x,
                      y = inst.y,
                      z = inst.z;
                int immy = inst.immy,
                    immz = inst.immz;

                // Ops that interact with memory need to know whether we're stride=1 or K,
                // but all non-memory ops can run the same code no matter the stride.
                switch (2*(int)inst.op + (stride == K ? 1 : 0)) {
                    default: SkUNREACHABLE;

                #define STRIDE_1(op) case 2*(int)op
                #define STRIDE_K(op) case 2*(int)op + 1
                    STRIDE_1(Op::store8 ): memcpy(arg(immy), &r(x).i32, 1); break;
                    STRIDE_1(Op::store16): memcpy(arg(immy), &r(x).i32, 2); break;
                    STRIDE_1(Op::store32): memcpy(arg(immy), &r(x).i32, 4); break;

                    STRIDE_K(Op::store8 ): skvx::cast<uint8_t> (r(x).i32).store(arg(immy)); break;
                    STRIDE_K(Op::store16): skvx::cast<uint16_t>(r(x).i32).store(arg(immy)); break;
                    STRIDE_K(Op::store32):                     (r(x).i32).store(arg(immy)); break;

                    STRIDE_1(Op::load8 ): r(d).i32 = 0; memcpy(&r(d).i32, arg(immy), 1); break;
                    STRIDE_1(Op::load16): r(d).i32 = 0; memcpy(&r(d).i32, arg(immy), 2); break;
                    STRIDE_1(Op::load32): r(d).i32 = 0; memcpy(&r(d).i32, arg(immy), 4); break;

                    STRIDE_K(Op::load8 ): r(d).i32= skvx::cast<int>(U8 ::Load(arg(immy))); break;
                    STRIDE_K(Op::load16): r(d).i32= skvx::cast<int>(U16::Load(arg(immy))); break;
                    STRIDE_K(Op::load32): r(d).i32=                 I32::Load(arg(immy)) ; break;

                    // The pointer we base our gather on is loaded indirectly from a uniform:
                    //     - arg(immy) is the uniform holding our gather base pointer somewhere;
                    //     - (const uint8_t*)arg(immy) + immz points to the gather base pointer;
                    //     - memcpy() loads the gather base and into a pointer of the right type.
                    // After all that we have an ordinary (uniform) pointer `ptr` to load from,
                    // and we then gather from it using the varying indices in r(x).
                    STRIDE_1(Op::gather8):
                        for (int i = 0; i < K; i++) {
                            const uint8_t* ptr;
                            memcpy(&ptr, (const uint8_t*)arg(immy) + immz, sizeof(ptr));
                            r(d).i32[i] = (i==0) ? ptr[ r(x).i32[i] ] : 0;
                        } break;
                    STRIDE_1(Op::gather16):
                        for (int i = 0; i < K; i++) {
                            const uint16_t* ptr;
                            memcpy(&ptr, (const uint8_t*)arg(immy) + immz, sizeof(ptr));
                            r(d).i32[i] = (i==0) ? ptr[ r(x).i32[i] ] : 0;
                        } break;
                    STRIDE_1(Op::gather32):
                        for (int i = 0; i < K; i++) {
                            const int* ptr;
                            memcpy(&ptr, (const uint8_t*)arg(immy) + immz, sizeof(ptr));
                            r(d).i32[i] = (i==0) ? ptr[ r(x).i32[i] ] : 0;
                        } break;

                    STRIDE_K(Op::gather8):
                        for (int i = 0; i < K; i++) {
                            const uint8_t* ptr;
                            memcpy(&ptr, (const uint8_t*)arg(immy) + immz, sizeof(ptr));
                            r(d).i32[i] = ptr[ r(x).i32[i] ];
                        } break;
                    STRIDE_K(Op::gather16):
                        for (int i = 0; i < K; i++) {
                            const uint16_t* ptr;
                            memcpy(&ptr, (const uint8_t*)arg(immy) + immz, sizeof(ptr));
                            r(d).i32[i] = ptr[ r(x).i32[i] ];
                        } break;
                    STRIDE_K(Op::gather32):
                        for (int i = 0; i < K; i++) {
                            const int* ptr;
                            memcpy(&ptr, (const uint8_t*)arg(immy) + immz, sizeof(ptr));
                            r(d).i32[i] = ptr[ r(x).i32[i] ];
                        } break;

                #undef STRIDE_1
                #undef STRIDE_K

                    // Ops that don't interact with memory should never care about the stride.
                #define CASE(op) case 2*(int)op: /*fallthrough*/ case 2*(int)op+1

                    CASE(Op::assert_true):
                    #ifdef SK_DEBUG
                        if (!all(r(x).i32)) {
                            SkDebugf("inst %d, register %d\n", i, y);
                            for (int i = 0; i < K; i++) {
                                SkDebugf("\t%2d: %08x (%g)\n", i, r(y).i32[i], r(y).f32[i]);
                            }
                        }
                        SkASSERT(all(r(x).i32));
                    #endif
                    break;

                    CASE(Op::index): {
                        const int iota[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
                                            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
                        static_assert(K <= SK_ARRAY_COUNT(iota), "");

                        r(d).i32 = n - I32::Load(iota);
                    } break;

                    CASE(Op::uniform8):
                        r(d).i32 = *(const uint8_t* )( (const char*)arg(immy) + immz );
                        break;
                    CASE(Op::uniform16):
                        r(d).i32 = *(const uint16_t*)( (const char*)arg(immy) + immz );
                        break;
                    CASE(Op::uniform32):
                        r(d).i32 = *(const int*     )( (const char*)arg(immy) + immz );
                        break;

                    CASE(Op::splat): r(d).i32 = immy; break;

                    CASE(Op::add_f32): r(d).f32 = r(x).f32 + r(y).f32; break;
                    CASE(Op::sub_f32): r(d).f32 = r(x).f32 - r(y).f32; break;
                    CASE(Op::mul_f32): r(d).f32 = r(x).f32 * r(y).f32; break;
                    CASE(Op::div_f32): r(d).f32 = r(x).f32 / r(y).f32; break;
                    CASE(Op::min_f32): r(d).f32 = min(r(x).f32, r(y).f32); break;
                    CASE(Op::max_f32): r(d).f32 = max(r(x).f32, r(y).f32); break;

                    // These _imm instructions are all x86/JIT only.
                    CASE(Op::add_f32_imm):
                    CASE(Op::sub_f32_imm):
                    CASE(Op::mul_f32_imm):
                    CASE(Op::min_f32_imm):
                    CASE(Op::max_f32_imm):
                    CASE(Op::bit_and_imm):
                    CASE(Op::bit_or_imm ):
                    CASE(Op::bit_xor_imm): SkUNREACHABLE; break;

                    CASE(Op::fma_f32): r(d).f32 = fma(r(x).f32, r(y).f32, r(z).f32); break;
                    CASE(Op::fms_f32): r(d).f32 = fma(r(x).f32, r(y).f32, -r(z).f32); break;
                    CASE(Op::fnma_f32): r(d).f32 = fma(-r(x).f32, r(y).f32, r(z).f32); break;

                    CASE(Op::sqrt_f32): r(d).f32 = sqrt(r(x).f32); break;

                    CASE(Op::add_i32): r(d).i32 = r(x).i32 + r(y).i32; break;
                    CASE(Op::sub_i32): r(d).i32 = r(x).i32 - r(y).i32; break;
                    CASE(Op::mul_i32): r(d).i32 = r(x).i32 * r(y).i32; break;

                    CASE(Op::add_i16x2): r(d).i16x2 = r(x).i16x2 + r(y).i16x2; break;
                    CASE(Op::sub_i16x2): r(d).i16x2 = r(x).i16x2 - r(y).i16x2; break;
                    CASE(Op::mul_i16x2): r(d).i16x2 = r(x).i16x2 * r(y).i16x2; break;

                    CASE(Op::shl_i32): r(d).i32 = r(x).i32 << immy; break;
                    CASE(Op::sra_i32): r(d).i32 = r(x).i32 >> immy; break;
                    CASE(Op::shr_i32): r(d).u32 = r(x).u32 >> immy; break;

                    CASE(Op::shl_i16x2): r(d).i16x2 = r(x).i16x2 << immy; break;
                    CASE(Op::sra_i16x2): r(d).i16x2 = r(x).i16x2 >> immy; break;
                    CASE(Op::shr_i16x2): r(d).u16x2 = r(x).u16x2 >> immy; break;

                    CASE(Op:: eq_f32): r(d).i32 = r(x).f32 == r(y).f32; break;
                    CASE(Op::neq_f32): r(d).i32 = r(x).f32 != r(y).f32; break;
                    CASE(Op:: gt_f32): r(d).i32 = r(x).f32 >  r(y).f32; break;
                    CASE(Op::gte_f32): r(d).i32 = r(x).f32 >= r(y).f32; break;

                    CASE(Op:: eq_i32): r(d).i32 = r(x).i32 == r(y).i32; break;
                    CASE(Op::neq_i32): r(d).i32 = r(x).i32 != r(y).i32; break;
                    CASE(Op:: gt_i32): r(d).i32 = r(x).i32 >  r(y).i32; break;
                    CASE(Op::gte_i32): r(d).i32 = r(x).i32 >= r(y).i32; break;

                    CASE(Op:: eq_i16x2): r(d).i16x2 = r(x).i16x2 == r(y).i16x2; break;
                    CASE(Op::neq_i16x2): r(d).i16x2 = r(x).i16x2 != r(y).i16x2; break;
                    CASE(Op:: gt_i16x2): r(d).i16x2 = r(x).i16x2 >  r(y).i16x2; break;
                    CASE(Op::gte_i16x2): r(d).i16x2 = r(x).i16x2 >= r(y).i16x2; break;

                    CASE(Op::bit_and  ): r(d).i32 = r(x).i32 &  r(y).i32; break;
                    CASE(Op::bit_or   ): r(d).i32 = r(x).i32 |  r(y).i32; break;
                    CASE(Op::bit_xor  ): r(d).i32 = r(x).i32 ^  r(y).i32; break;
                    CASE(Op::bit_clear): r(d).i32 = r(x).i32 & ~r(y).i32; break;

                    CASE(Op::select): r(d).i32 = skvx::if_then_else(r(x).i32, r(y).i32, r(z).i32);
                                      break;

                    CASE(Op::pack):    r(d).u32 = r(x).u32 | (r(y).u32 << immz); break;

                    CASE(Op::bytes): {
                        const U32 table[] = {
                            0,
                            (r(x).u32      ) & 0xff,
                            (r(x).u32 >>  8) & 0xff,
                            (r(x).u32 >> 16) & 0xff,
                            (r(x).u32 >> 24) & 0xff,
                        };
                        r(d).u32 = table[(immy >>  0) & 0xf] <<  0
                                 | table[(immy >>  4) & 0xf] <<  8
                                 | table[(immy >>  8) & 0xf] << 16
                                 | table[(immy >> 12) & 0xf] << 24;
                    } break;

                    CASE(Op::floor):  r(d).f32 =                   skvx::floor(r(x).f32) ; break;
                    CASE(Op::to_f32): r(d).f32 = skvx::cast<float>(            r(x).i32 ); break;
                    CASE(Op::trunc):  r(d).i32 = skvx::cast<int>  (            r(x).f32 ); break;
                    CASE(Op::round):  r(d).i32 = skvx::cast<int>  (skvx::lrint(r(x).f32)); break;
                #undef CASE
                }
            }
        }
    }

}

#endif//SkVM_opts_DEFINED
