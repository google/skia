// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkVM_opts_DEFINED
#define SkVM_opts_DEFINED

#include "include/private/SkVx.h"
#include "src/core/SkVM.h"

template <int N>
static inline skvx::Vec<N,int> gather32(const int* ptr, const skvx::Vec<N,int>& ix) {
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    if constexpr (N == 8) {
        return skvx::bit_pun<skvx::Vec<N,int>>(
                _mm256_i32gather_epi32(ptr, skvx::bit_pun<__m256i>(ix), 4));
    }
#endif
    // Try to recurse on specializations, falling back on standard scalar map()-based impl.
    if constexpr (N > 8) {
        return join(gather32(ptr, ix.lo),
                    gather32(ptr, ix.hi));
    }
    return map([&](int i) { return ptr[i]; }, ix);
}

namespace SK_OPTS_NS {

namespace SkVMInterpreterTypes {
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    constexpr inline int K = 32;  // 1024-bit: 4 ymm or 2 zmm at a time
#else
    constexpr inline int K = 8;   // 256-bit: 2 xmm, 2 v-registers, etc.
#endif
    using I32 = skvx::Vec<K, int>;
    using I16 = skvx::Vec<K, int16_t>;
    using F32 = skvx::Vec<K, float>;
    using U64 = skvx::Vec<K, uint64_t>;
    using U32 = skvx::Vec<K, uint32_t>;
    using U16 = skvx::Vec<K, uint16_t>;
    using  U8 = skvx::Vec<K, uint8_t>;
    union Slot {
        F32   f32;
        I32   i32;
        U32   u32;
        I16   i16;
        U16   u16;
    };
}  // namespace SkVMInterpreterTypes

    inline void interpret_skvm(const skvm::InterpreterInstruction insts[], const int ninsts,
                               const int nregs, const int loop,
                               const int strides[], const int nargs,
                               int n, void* args[]) {
        using namespace skvm;

        using SkVMInterpreterTypes::K;
        using SkVMInterpreterTypes::I32;
        using SkVMInterpreterTypes::I16;
        using SkVMInterpreterTypes::F32;
        using SkVMInterpreterTypes::U64;
        using SkVMInterpreterTypes::U32;
        using SkVMInterpreterTypes::U16;
        using SkVMInterpreterTypes::U8;
        using SkVMInterpreterTypes::Slot;

        // We'll operate in SIMT style, knocking off K-size chunks from n while possible.

        Slot                     few_regs[16];
        std::unique_ptr<char[]> many_regs;

        Slot* r = few_regs;

        if (nregs > (int)SK_ARRAY_COUNT(few_regs)) {
            // Annoyingly we can't trust that malloc() or new will work with Slot because
            // the skvx::Vec types may have alignment greater than what they provide.
            // We'll overallocate one extra register so we can align manually.
            many_regs.reset(new char[ sizeof(Slot) * (nregs + 1) ]);

            uintptr_t addr = (uintptr_t)many_regs.get();
            addr += alignof(Slot) -
                     (addr & (alignof(Slot) - 1));
            SkASSERT((addr & (alignof(Slot) - 1)) == 0);
            r = (Slot*)addr;
        }


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

            for (int instIdx = start; instIdx < ninsts; instIdx++) {
                InterpreterInstruction inst = insts[instIdx];

                // d = op(x,y,z,w, immA,immB)
                Reg   d = inst.d,
                      x = inst.x,
                      y = inst.y,
                      z = inst.z,
                      w = inst.w;
                int immA = inst.immA,
                    immB = inst.immB,
                    immC = inst.immC;

                // Ops that interact with memory need to know whether we're stride=1 or K,
                // but all non-memory ops can run the same code no matter the stride.
                switch (2*(int)inst.op + (stride == K ? 1 : 0)) {
                    default: SkUNREACHABLE;

                #define STRIDE_1(op) case 2*(int)op
                #define STRIDE_K(op) case 2*(int)op + 1
                    STRIDE_1(Op::store8 ): memcpy(args[immA], &r[x].i32, 1); break;
                    STRIDE_1(Op::store16): memcpy(args[immA], &r[x].i32, 2); break;
                    STRIDE_1(Op::store32): memcpy(args[immA], &r[x].i32, 4); break;
                    STRIDE_1(Op::store64): memcpy((char*)args[immA]+0, &r[x].i32, 4);
                                           memcpy((char*)args[immA]+4, &r[y].i32, 4); break;

                    STRIDE_K(Op::store8 ): skvx::cast<uint8_t> (r[x].i32).store(args[immA]); break;
                    STRIDE_K(Op::store16): skvx::cast<uint16_t>(r[x].i32).store(args[immA]); break;
                    STRIDE_K(Op::store32):                     (r[x].i32).store(args[immA]); break;
                    STRIDE_K(Op::store64): (skvx::cast<uint64_t>(r[x].u32) << 0 |
                                            skvx::cast<uint64_t>(r[y].u32) << 32).store(args[immA]);
                                           break;

                    STRIDE_1(Op::load8 ): r[d].i32 = 0; memcpy(&r[d].i32, args[immA], 1); break;
                    STRIDE_1(Op::load16): r[d].i32 = 0; memcpy(&r[d].i32, args[immA], 2); break;
                    STRIDE_1(Op::load32): r[d].i32 = 0; memcpy(&r[d].i32, args[immA], 4); break;
                    STRIDE_1(Op::load64):
                        r[d].i32 = 0; memcpy(&r[d].i32, (char*)args[immA] + 4*immB, 4); break;

                    STRIDE_K(Op::load8 ): r[d].i32= skvx::cast<int>(U8 ::Load(args[immA])); break;
                    STRIDE_K(Op::load16): r[d].i32= skvx::cast<int>(U16::Load(args[immA])); break;
                    STRIDE_K(Op::load32): r[d].i32=                 I32::Load(args[immA]) ; break;
                    STRIDE_K(Op::load64):
                        // Low 32 bits if immB=0, or high 32 bits if immB=1.
                        r[d].i32 = skvx::cast<int>(U64::Load(args[immA]) >> (32*immB)); break;

                    // The pointer we base our gather on is loaded indirectly from a uniform:
                    //     - args[immA] is the uniform holding our gather base pointer somewhere;
                    //     - (const uint8_t*)args[immA] + immB points to the gather base pointer;
                    //     - memcpy() loads the gather base and into a pointer of the right type.
                    // After all that we have an ordinary (uniform) pointer `ptr` to load from,
                    // and we then gather from it using the varying indices in r[x].
                    STRIDE_1(Op::gather8): {
                        const uint8_t* ptr;
                        memcpy(&ptr, (const uint8_t*)args[immA] + immB, sizeof(ptr));
                        r[d].i32 = ptr[ r[x].i32[0] ];
                    } break;
                    STRIDE_1(Op::gather16): {
                        const uint16_t* ptr;
                        memcpy(&ptr, (const uint8_t*)args[immA] + immB, sizeof(ptr));
                        r[d].i32 = ptr[ r[x].i32[0] ];
                    } break;
                    STRIDE_1(Op::gather32): {
                        const int* ptr;
                        memcpy(&ptr, (const uint8_t*)args[immA] + immB, sizeof(ptr));
                        r[d].i32 = ptr[ r[x].i32[0] ];
                    } break;

                    STRIDE_K(Op::gather8): {
                        const uint8_t* ptr;
                        memcpy(&ptr, (const uint8_t*)args[immA] + immB, sizeof(ptr));
                        r[d].i32 = map([&](int ix) { return (int)ptr[ix]; }, r[x].i32);
                    } break;
                    STRIDE_K(Op::gather16): {
                        const uint16_t* ptr;
                        memcpy(&ptr, (const uint8_t*)args[immA] + immB, sizeof(ptr));
                        r[d].i32 = map([&](int ix) { return (int)ptr[ix]; }, r[x].i32);
                    } break;
                    STRIDE_K(Op::gather32): {
                        const int* ptr;
                        memcpy(&ptr, (const uint8_t*)args[immA] + immB, sizeof(ptr));
                        r[d].i32 = gather32(ptr, r[x].i32);
                    } break;

                #undef STRIDE_1
                #undef STRIDE_K

                    // Ops that don't interact with memory should never care about the stride.
                #define CASE(op) case 2*(int)op: /*fallthrough*/ case 2*(int)op+1

                    // These 128-bit ops are implemented serially for simplicity.
                    CASE(Op::store128): {
                        U64 lo = (skvx::cast<uint64_t>(r[x].u32) << 0 |
                                  skvx::cast<uint64_t>(r[y].u32) << 32),
                            hi = (skvx::cast<uint64_t>(r[z].u32) << 0 |
                                  skvx::cast<uint64_t>(r[w].u32) << 32);
                        for (int i = 0; i < stride; i++) {
                            memcpy((char*)args[immA] + 16*i + 0, &lo[i], 8);
                            memcpy((char*)args[immA] + 16*i + 8, &hi[i], 8);
                        }
                    } break;

                    CASE(Op::load128):
                        r[d].i32 = 0;
                        for (int i = 0; i < stride; i++) {
                            memcpy(&r[d].i32[i], (const char*)args[immA] + 16*i+ 4*immB, 4);
                        } break;

                    CASE(Op::assert_true):
                    #ifdef SK_DEBUG
                        if (!all(r[x].i32)) {
                            SkDebugf("inst %d, register %d\n", instIdx, y);
                            for (int i = 0; i < K; i++) {
                                SkDebugf("\t%2d: %08x (%g)\n",
                                         instIdx, r[y].i32[instIdx], r[y].f32[instIdx]);
                            }
                            SkASSERT(false);
                        }
                    #endif
                    break;

                    CASE(Op::trace_line):
                    #ifdef SK_DEBUG
                    // TODO(skia:12614): this opcode will check the mask; if it's set, we write the
                    // line number from immA into the trace buffer.
                    #endif
                    break;

                    CASE(Op::trace_var):
                    #ifdef SK_DEBUG
                    // TODO(skia:12614): this opcode will check the mask; if it's set, we write the
                    // variable-assignment slot and value to the trace buffer.
                    #endif
                    break;

                    CASE(Op::trace_call):
                    #ifdef SK_DEBUG
                    // TODO(skia:12614): this opcode will be used to keep track of function entrance
                    // and exits, enabling step-over of function calls.
                    #endif
                    break;

                    CASE(Op::index): {
                        const int iota[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
                                            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
                                            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
                                            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 };
                        static_assert(K <= SK_ARRAY_COUNT(iota), "");

                        r[d].i32 = n - I32::Load(iota);
                    } break;

                    CASE(Op::uniform32):
                        r[d].i32 = *(const int*)( (const char*)args[immA] + immB );
                        break;

                    CASE(Op::array32):
                        const int* ptr;
                        memcpy(&ptr, (const uint8_t*)args[immA] + immB, sizeof(ptr));
                        r[d].i32 = ptr[immC/sizeof(int)];
                        break;

                    CASE(Op::splat): r[d].i32 = immA; break;

                    CASE(Op::add_f32): r[d].f32 = r[x].f32 + r[y].f32; break;
                    CASE(Op::sub_f32): r[d].f32 = r[x].f32 - r[y].f32; break;
                    CASE(Op::mul_f32): r[d].f32 = r[x].f32 * r[y].f32; break;
                    CASE(Op::div_f32): r[d].f32 = r[x].f32 / r[y].f32; break;
                    CASE(Op::min_f32): r[d].f32 = min(r[x].f32, r[y].f32); break;
                    CASE(Op::max_f32): r[d].f32 = max(r[x].f32, r[y].f32); break;

                    CASE(Op::fma_f32):  r[d].f32 = fma( r[x].f32, r[y].f32,  r[z].f32); break;
                    CASE(Op::fms_f32):  r[d].f32 = fma( r[x].f32, r[y].f32, -r[z].f32); break;
                    CASE(Op::fnma_f32): r[d].f32 = fma(-r[x].f32, r[y].f32,  r[z].f32); break;

                    CASE(Op::sqrt_f32): r[d].f32 = sqrt(r[x].f32); break;

                    CASE(Op::add_i32): r[d].i32 = r[x].i32 + r[y].i32; break;
                    CASE(Op::sub_i32): r[d].i32 = r[x].i32 - r[y].i32; break;
                    CASE(Op::mul_i32): r[d].i32 = r[x].i32 * r[y].i32; break;

                    CASE(Op::shl_i32): r[d].i32 = r[x].i32 << immA; break;
                    CASE(Op::sra_i32): r[d].i32 = r[x].i32 >> immA; break;
                    CASE(Op::shr_i32): r[d].u32 = r[x].u32 >> immA; break;

                    CASE(Op:: eq_f32): r[d].i32 = r[x].f32 == r[y].f32; break;
                    CASE(Op::neq_f32): r[d].i32 = r[x].f32 != r[y].f32; break;
                    CASE(Op:: gt_f32): r[d].i32 = r[x].f32 >  r[y].f32; break;
                    CASE(Op::gte_f32): r[d].i32 = r[x].f32 >= r[y].f32; break;

                    CASE(Op:: eq_i32): r[d].i32 = r[x].i32 == r[y].i32; break;
                    CASE(Op:: gt_i32): r[d].i32 = r[x].i32 >  r[y].i32; break;

                    CASE(Op::bit_and  ): r[d].i32 = r[x].i32 &  r[y].i32; break;
                    CASE(Op::bit_or   ): r[d].i32 = r[x].i32 |  r[y].i32; break;
                    CASE(Op::bit_xor  ): r[d].i32 = r[x].i32 ^  r[y].i32; break;
                    CASE(Op::bit_clear): r[d].i32 = r[x].i32 & ~r[y].i32; break;

                    CASE(Op::select): r[d].i32 = skvx::if_then_else(r[x].i32, r[y].i32, r[z].i32);
                                      break;

                    CASE(Op::ceil):   r[d].f32 =                    skvx::ceil(r[x].f32) ; break;
                    CASE(Op::floor):  r[d].f32 =                   skvx::floor(r[x].f32) ; break;
                    CASE(Op::to_f32): r[d].f32 = skvx::cast<float>(            r[x].i32 ); break;
                    CASE(Op::trunc):  r[d].i32 = skvx::cast<int>  (            r[x].f32 ); break;
                    CASE(Op::round):  r[d].i32 = skvx::cast<int>  (skvx::lrint(r[x].f32)); break;

                    CASE(Op::to_fp16):
                        r[d].i32 = skvx::cast<int>(skvx::to_half(r[x].f32));
                        break;
                    CASE(Op::from_fp16):
                        r[d].f32 = skvx::from_half(skvx::cast<uint16_t>(r[x].i32));
                        break;

                #undef CASE
                }
            }
        }
    }

}  // namespace SK_OPTS_NS

#endif//SkVM_opts_DEFINED
