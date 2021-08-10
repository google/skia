/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_DEFINED
#define SkVM_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkVM_fwd.h"
#include <vector>      // std::vector

class SkWStream;

#if defined(SKVM_JIT_WHEN_POSSIBLE) && !defined(SK_BUILD_FOR_IOS)
    #if defined(__x86_64__) || defined(_M_X64)
        #if defined(_WIN32) || defined(__linux) || defined(__APPLE__)
            #define SKVM_JIT
        #endif
    #endif
    #if defined(__aarch64__)
        #if defined(__ANDROID__) || defined(__APPLE__)
            #define SKVM_JIT
        #endif
    #endif
#endif

#if 0
    #define SKVM_LLVM
#endif

#if 0
    #undef SKVM_JIT
#endif

namespace skvm {

    class Assembler {
    public:
        explicit Assembler(void* buf);

        size_t size() const;

        // Order matters... GP64, Xmm, Ymm values match 4-bit register encoding for each.
        enum GP64 {
            rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
            r8 , r9 , r10, r11, r12, r13, r14, r15,
        };
        enum Xmm {
            xmm0, xmm1, xmm2 , xmm3 , xmm4 , xmm5 , xmm6 , xmm7 ,
            xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,
        };
        enum Ymm {
            ymm0, ymm1, ymm2 , ymm3 , ymm4 , ymm5 , ymm6 , ymm7 ,
            ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15,
        };

        // X and V values match 5-bit encoding for each (nothing tricky).
        enum X {
            x0 , x1 , x2 , x3 , x4 , x5 , x6 , x7 ,
            x8 , x9 , x10, x11, x12, x13, x14, x15,
            x16, x17, x18, x19, x20, x21, x22, x23,
            x24, x25, x26, x27, x28, x29, x30, xzr, sp=xzr,
        };
        enum V {
            v0 , v1 , v2 , v3 , v4 , v5 , v6 , v7 ,
            v8 , v9 , v10, v11, v12, v13, v14, v15,
            v16, v17, v18, v19, v20, v21, v22, v23,
            v24, v25, v26, v27, v28, v29, v30, v31,
        };

        void bytes(const void*, int);
        void byte(uint8_t);
        void word(uint32_t);

        struct Label {
            int                                      offset = 0;
            enum { NotYetSet, ARMDisp19, X86Disp32 } kind = NotYetSet;
            SkSTArray<2, int>                        references;
        };

        // x86-64

        void align(int mod);

        void int3();
        void vzeroupper();
        void ret();

        // Mem represents a value at base + disp + scale*index,
        // or simply at base + disp if index=rsp.
        enum Scale { ONE, TWO, FOUR, EIGHT };
        struct Mem {
            GP64  base;
            int   disp  = 0;
            GP64  index = rsp;
            Scale scale = ONE;
        };

        struct Operand {
            union {
                int    reg;
                Mem    mem;
                Label* label;
            };
            enum { REG, MEM, LABEL } kind;

            Operand(GP64   r) : reg  (r), kind(REG  ) {}
            Operand(Xmm    r) : reg  (r), kind(REG  ) {}
            Operand(Ymm    r) : reg  (r), kind(REG  ) {}
            Operand(Mem    m) : mem  (m), kind(MEM  ) {}
            Operand(Label* l) : label(l), kind(LABEL) {}
        };

        void vpand (Ymm dst, Ymm x, Operand y);
        void vpandn(Ymm dst, Ymm x, Operand y);
        void vpor  (Ymm dst, Ymm x, Operand y);
        void vpxor (Ymm dst, Ymm x, Operand y);

        void vpaddd (Ymm dst, Ymm x, Operand y);
        void vpsubd (Ymm dst, Ymm x, Operand y);
        void vpmulld(Ymm dst, Ymm x, Operand y);

        void vpaddw   (Ymm dst, Ymm x, Operand y);
        void vpsubw   (Ymm dst, Ymm x, Operand y);
        void vpmullw  (Ymm dst, Ymm x, Operand y);

        void vpabsw   (Ymm dst, Operand x);
        void vpavgw   (Ymm dst, Ymm x, Operand y);  // dst = (x+y+1)>>1, unsigned.
        void vpmulhrsw(Ymm dst, Ymm x, Operand y);  // dst = (x*y + (1<<14)) >> 15, signed.
        void vpminsw  (Ymm dst, Ymm x, Operand y);
        void vpminuw  (Ymm dst, Ymm x, Operand y);
        void vpmaxsw  (Ymm dst, Ymm x, Operand y);
        void vpmaxuw  (Ymm dst, Ymm x, Operand y);

        void vaddps(Ymm dst, Ymm x, Operand y);
        void vsubps(Ymm dst, Ymm x, Operand y);
        void vmulps(Ymm dst, Ymm x, Operand y);
        void vdivps(Ymm dst, Ymm x, Operand y);
        void vminps(Ymm dst, Ymm x, Operand y);
        void vmaxps(Ymm dst, Ymm x, Operand y);

        void vsqrtps(Ymm dst, Operand x);

        void vfmadd132ps(Ymm dst, Ymm x, Operand y);
        void vfmadd213ps(Ymm dst, Ymm x, Operand y);
        void vfmadd231ps(Ymm dst, Ymm x, Operand y);

        void vfmsub132ps(Ymm dst, Ymm x, Operand y);
        void vfmsub213ps(Ymm dst, Ymm x, Operand y);
        void vfmsub231ps(Ymm dst, Ymm x, Operand y);

        void vfnmadd132ps(Ymm dst, Ymm x, Operand y);
        void vfnmadd213ps(Ymm dst, Ymm x, Operand y);
        void vfnmadd231ps(Ymm dst, Ymm x, Operand y);

        void vpackusdw(Ymm dst, Ymm x, Operand y);
        void vpackuswb(Ymm dst, Ymm x, Operand y);

        void vpunpckldq(Ymm dst, Ymm x, Operand y);
        void vpunpckhdq(Ymm dst, Ymm x, Operand y);

        void vpcmpeqd(Ymm dst, Ymm x, Operand y);
        void vpcmpgtd(Ymm dst, Ymm x, Operand y);
        void vpcmpeqw(Ymm dst, Ymm x, Operand y);
        void vpcmpgtw(Ymm dst, Ymm x, Operand y);

        void vcmpps   (Ymm dst, Ymm x, Operand y, int imm);
        void vcmpeqps (Ymm dst, Ymm x, Operand y) { this->vcmpps(dst,x,y,0); }
        void vcmpltps (Ymm dst, Ymm x, Operand y) { this->vcmpps(dst,x,y,1); }
        void vcmpleps (Ymm dst, Ymm x, Operand y) { this->vcmpps(dst,x,y,2); }
        void vcmpneqps(Ymm dst, Ymm x, Operand y) { this->vcmpps(dst,x,y,4); }

        // Sadly, the x parameter cannot be a general Operand for these shifts.
        void vpslld(Ymm dst, Ymm x, int imm);
        void vpsrld(Ymm dst, Ymm x, int imm);
        void vpsrad(Ymm dst, Ymm x, int imm);

        void vpsllw(Ymm dst, Ymm x, int imm);
        void vpsrlw(Ymm dst, Ymm x, int imm);
        void vpsraw(Ymm dst, Ymm x, int imm);

        void vpermq    (Ymm dst, Operand x, int imm);
        void vperm2f128(Ymm dst, Ymm x, Operand y, int imm);
        void vpermps   (Ymm dst, Ymm ix, Operand src);        // dst[i] = src[ix[i]]

        enum Rounding { NEAREST, FLOOR, CEIL, TRUNC, CURRENT };
        void vroundps(Ymm dst, Operand x, Rounding);

        void vmovdqa(Ymm dst, Operand x);
        void vmovups(Ymm dst, Operand x);
        void vmovups(Xmm dst, Operand x);
        void vmovups(Operand dst, Ymm x);
        void vmovups(Operand dst, Xmm x);

        void vcvtdq2ps (Ymm dst, Operand x);
        void vcvttps2dq(Ymm dst, Operand x);
        void vcvtps2dq (Ymm dst, Operand x);

        void vcvtps2ph(Operand dst, Ymm x, Rounding);
        void vcvtph2ps(Ymm dst, Operand x);

        void vpblendvb(Ymm dst, Ymm x, Operand y, Ymm z);

        void vpshufb(Ymm dst, Ymm x, Operand y);

        void vptest(Ymm x, Operand y);

        void vbroadcastss(Ymm dst, Operand y);

        void vpmovzxwd(Ymm dst, Operand src);   // dst = src, 128-bit, uint16_t -> int
        void vpmovzxbd(Ymm dst, Operand src);   // dst = src,  64-bit, uint8_t  -> int

        void vmovq(Operand dst, Xmm src);  // dst = src,  64-bit
        void vmovd(Operand dst, Xmm src);  // dst = src,  32-bit
        void vmovd(Xmm dst, Operand src);  // dst = src,  32-bit

        void vpinsrd(Xmm dst, Xmm src, Operand y, int imm);  // dst = src; dst[imm] = y, 32-bit
        void vpinsrw(Xmm dst, Xmm src, Operand y, int imm);  // dst = src; dst[imm] = y, 16-bit
        void vpinsrb(Xmm dst, Xmm src, Operand y, int imm);  // dst = src; dst[imm] = y,  8-bit

        void vextracti128(Operand dst, Ymm src, int imm);    // dst = src[imm], 128-bit
        void vpextrd     (Operand dst, Xmm src, int imm);    // dst = src[imm],  32-bit
        void vpextrw     (Operand dst, Xmm src, int imm);    // dst = src[imm],  16-bit
        void vpextrb     (Operand dst, Xmm src, int imm);    // dst = src[imm],   8-bit

        // if (mask & 0x8000'0000) {
        //     dst = base[scale*ix];
        // }
        // mask = 0;
        void vgatherdps(Ymm dst, Scale scale, Ymm ix, GP64 base, Ymm mask);


        void label(Label*);

        void jmp(Label*);
        void je (Label*);
        void jne(Label*);
        void jl (Label*);
        void jc (Label*);

        void add (Operand dst, int imm);
        void sub (Operand dst, int imm);
        void cmp (Operand dst, int imm);
        void mov (Operand dst, int imm);
        void movb(Operand dst, int imm);

        void add (Operand dst, GP64 x);
        void sub (Operand dst, GP64 x);
        void cmp (Operand dst, GP64 x);
        void mov (Operand dst, GP64 x);
        void movb(Operand dst, GP64 x);

        void add (GP64 dst, Operand x);
        void sub (GP64 dst, Operand x);
        void cmp (GP64 dst, Operand x);
        void mov (GP64 dst, Operand x);
        void movb(GP64 dst, Operand x);

        // Disambiguators... choice is arbitrary (but generates different code!).
        void add (GP64 dst, GP64 x) { this->add (Operand(dst), x); }
        void sub (GP64 dst, GP64 x) { this->sub (Operand(dst), x); }
        void cmp (GP64 dst, GP64 x) { this->cmp (Operand(dst), x); }
        void mov (GP64 dst, GP64 x) { this->mov (Operand(dst), x); }
        void movb(GP64 dst, GP64 x) { this->movb(Operand(dst), x); }

        void movzbq(GP64 dst, Operand x);  // dst = x, uint8_t  -> int
        void movzwq(GP64 dst, Operand x);  // dst = x, uint16_t -> int

        // aarch64

        // d = op(n,m)
        using DOpNM = void(V d, V n, V m);
        DOpNM  and16b, orr16b, eor16b, bic16b, bsl16b,
               add4s,  sub4s,  mul4s,
              cmeq4s, cmgt4s,
                       sub8h,  mul8h,
              fadd4s, fsub4s, fmul4s, fdiv4s, fmin4s, fmax4s,
              fcmeq4s, fcmgt4s, fcmge4s,
              tbl,
              uzp14s, uzp24s,
              zip14s, zip24s;

        // TODO: there are also float ==,<,<=,>,>= instructions with an immediate 0.0f,
        // and the register comparison > and >= can also compare absolute values.  Interesting.

        // d += n*m
        void fmla4s(V d, V n, V m);

        // d -= n*m
        void fmls4s(V d, V n, V m);

        // d = op(n,imm)
        using DOpNImm = void(V d, V n, int imm);
        DOpNImm sli4s,
                shl4s, sshr4s, ushr4s,
                               ushr8h;

        // d = op(n)
        using DOpN = void(V d, V n);
        DOpN not16b,    // d = ~n
             fneg4s,    // d = -n
             fsqrt4s,   // d = sqrtf(n)
             scvtf4s,   // int -> float
             fcvtzs4s,  // truncate float -> int
             fcvtns4s,  // round float -> int  (nearest even)
             frintp4s,  // round float -> int as float, toward plus infinity  (ceil)
             frintm4s,  // round float -> int as float, toward minus infinity (floor)
             fcvtn,     // f32 -> f16 in low half
             fcvtl,     // f16 in low half -> f32
             xtns2h,    // u32 -> u16
             xtnh2b,    // u16 -> u8
             uxtlb2h,   // u8 -> u16    (TODO: this is a special case of ushll.8h)
             uxtlh2s,   // u16 -> u32   (TODO: this is a special case of ushll.4s)
             uminv4s;   // dst[0] = min(n[0],n[1],n[2],n[3]), n as unsigned

        void brk (int imm16);
        void ret (X);
        void add (X d, X n, int imm12);
        void sub (X d, X n, int imm12);
        void subs(X d, X n, int imm12);  // subtract setting condition flags

        enum Shift { LSL,LSR,ASR,ROR };
        void add (X d, X n, X m, Shift=LSL, int imm6=0);  // d=n+Shift(m,imm6), for Shift != ROR.

        // There's another encoding for unconditional branches that can jump further,
        // but this one encoded as b.al is simple to implement and should be fine.
        void b  (Label* l) { this->b(Condition::al, l); }
        void bne(Label* l) { this->b(Condition::ne, l); }
        void blt(Label* l) { this->b(Condition::lt, l); }

        // "cmp ..." is just an assembler mnemonic for "subs xzr, ..."!
        void cmp(X n, int imm12) { this->subs(xzr, n, imm12); }

        // Compare and branch if zero/non-zero, as if
        //      cmp(t,0)
        //      beq/bne(l)
        // but without setting condition flags.
        void cbz (X t, Label* l);
        void cbnz(X t, Label* l);

        // TODO: there are ldur variants with unscaled imm, useful?
        void ldrd(X dst, X src, int imm12=0);  // 64-bit dst = *(src+imm12*8)
        void ldrs(X dst, X src, int imm12=0);  // 32-bit dst = *(src+imm12*4)
        void ldrh(X dst, X src, int imm12=0);  // 16-bit dst = *(src+imm12*2)
        void ldrb(X dst, X src, int imm12=0);  //  8-bit dst = *(src+imm12)

        void ldrq(V dst, Label*);  // 128-bit PC-relative load

        void ldrq(V dst, X src, int imm12=0);  // 128-bit dst = *(src+imm12*16)
        void ldrd(V dst, X src, int imm12=0);  //  64-bit dst = *(src+imm12*8)
        void ldrs(V dst, X src, int imm12=0);  //  32-bit dst = *(src+imm12*4)
        void ldrh(V dst, X src, int imm12=0);  //  16-bit dst = *(src+imm12*2)
        void ldrb(V dst, X src, int imm12=0);  //   8-bit dst = *(src+imm12)

        void strs(X src, X dst, int imm12=0);  // 32-bit *(dst+imm12*4) = src

        void strq(V src, X dst, int imm12=0);  // 128-bit *(dst+imm12*16) = src
        void strd(V src, X dst, int imm12=0);  //  64-bit *(dst+imm12*8)  = src
        void strs(V src, X dst, int imm12=0);  //  32-bit *(dst+imm12*4)  = src
        void strh(V src, X dst, int imm12=0);  //  16-bit *(dst+imm12*2)  = src
        void strb(V src, X dst, int imm12=0);  //   8-bit *(dst+imm12)    = src

        void movs(X dst, V src, int lane);  // dst = 32-bit src[lane]
        void inss(V dst, X src, int lane);  // dst[lane] = 32-bit src

        void dup4s  (V dst, X src);  // Each 32-bit lane = src

        void ld1r4s (V dst, X src);  // Each 32-bit lane = *src
        void ld1r8h (V dst, X src);  // Each 16-bit lane = *src
        void ld1r16b(V dst, X src);  // Each  8-bit lane = *src

        void ld24s(V dst, X src);  // deinterleave(dst,dst+1)             = 256-bit *src
        void ld44s(V dst, X src);  // deinterleave(dst,dst+1,dst+2,dst+3) = 512-bit *src
        void st24s(V src, X dst);  // 256-bit *dst = interleave_32bit_lanes(src,src+1)
        void st44s(V src, X dst);  // 512-bit *dst = interleave_32bit_lanes(src,src+1,src+2,src+3)

        void ld24s(V dst, X src, int lane);  // Load 2 32-bit values into given lane of dst..dst+1
        void ld44s(V dst, X src, int lane);  // Load 4 32-bit values into given lane of dst..dst+3

    private:
        uint8_t* fCode;
        size_t   fSize;

        // x86-64
        enum W { W0, W1 };      // Are the lanes 64-bit (W1) or default (W0)?  Intel Vol 2A 2.3.5.5
        enum L { L128, L256 };  // Is this a 128- or 256-bit operation?        Intel Vol 2A 2.3.6.2

        // Helpers for vector instructions.
        void op(int prefix, int map, int opcode, int dst, int x, Operand y, W,L);
        void op(int p, int m, int o, Ymm d, Ymm x, Operand y, W w=W0) { op(p,m,o, d,x,y,w,L256); }
        void op(int p, int m, int o, Ymm d,        Operand y, W w=W0) { op(p,m,o, d,0,y,w,L256); }
        void op(int p, int m, int o, Xmm d, Xmm x, Operand y, W w=W0) { op(p,m,o, d,x,y,w,L128); }
        void op(int p, int m, int o, Xmm d,        Operand y, W w=W0) { op(p,m,o, d,0,y,w,L128); }

        // Helpers for GP64 instructions.
        void op(int opcode, Operand dst, GP64 x);
        void op(int opcode, int opcode_ext, Operand dst, int imm);

        void jump(uint8_t condition, Label*);
        int disp32(Label*);
        void imm_byte_after_operand(const Operand&, int byte);

        // aarch64

        // Opcode for 3-arguments ops is split between hi and lo:
        //    [11 bits hi] [5 bits m] [6 bits lo] [5 bits n] [5 bits d]
        void op(uint32_t hi, V m, uint32_t lo, V n, V d);

        // 0,1,2-argument ops, with or without an immediate:
        //    [ 22 bits op ] [5 bits n] [5 bits d]
        // Any immediate falls in the middle somewhere overlapping with either op, n, or both.
        void op(uint32_t op22, V n, V d, int imm=0);
        void op(uint32_t op22, X n, V d, int imm=0) { this->op(op22,(V)n,   d,imm); }
        void op(uint32_t op22, V n, X d, int imm=0) { this->op(op22,   n,(V)d,imm); }
        void op(uint32_t op22, X n, X d, int imm=0) { this->op(op22,(V)n,(V)d,imm); }
        void op(uint32_t op22,           int imm=0) { this->op(op22,(V)0,(V)0,imm); }
        // (1-argument ops don't seem to have a consistent convention of passing as n or d.)


        // Order matters... value is 4-bit encoding for condition code.
        enum class Condition { eq,ne,cs,cc,mi,pl,vs,vc,hi,ls,ge,lt,gt,le,al };
        void b(Condition, Label*);
        int disp19(Label*);
    };

    // Order matters a little: Ops <=store128 are treated as having side effects.
    #define SKVM_OPS(M)                                              \
        M(assert_true)                                               \
        M(store8)   M(store16)   M(store32) M(store64) M(store128)   \
        M(load8)    M(load16)    M(load32)  M(load64) M(load128)     \
        M(index)                                                     \
        M(gather8)  M(gather16)  M(gather32)                         \
                                 M(uniform32)                        \
                                 M(array32)                          \
        M(splat)                                                     \
        M(add_f32) M(add_i32)                                        \
        M(sub_f32) M(sub_i32)                                        \
        M(mul_f32) M(mul_i32)                                        \
        M(div_f32)                                                   \
        M(min_f32) M(max_f32)                                        \
        M(fma_f32) M(fms_f32) M(fnma_f32)                            \
        M(sqrt_f32)                                                  \
        M(shl_i32) M(shr_i32) M(sra_i32)                             \
        M(ceil) M(floor) M(trunc) M(round) M(to_fp16) M(from_fp16)   \
        M(to_f32)                                                    \
        M(neq_f32) M(eq_f32) M(eq_i32)                               \
        M(gte_f32) M(gt_f32) M(gt_i32)                               \
        M(bit_and)     M(bit_or)     M(bit_xor)     M(bit_clear)     \
        M(select)
    // End of SKVM_OPS

    enum class Op : int {
    #define M(op) op,
        SKVM_OPS(M)
    #undef M
    };

    static inline bool has_side_effect(Op op) {
        return op <= Op::store128;
    }
    static inline bool touches_varying_memory(Op op) {
        return Op::store8 <= op && op <= Op::load128;
    }
    static inline bool is_always_varying(Op op) {
        return Op::store8 <= op && op <= Op::index;
    }

    using Val = int;
    // We reserve an impossibe Val ID as a sentinel
    // NA meaning none, n/a, null, nil, etc.
    static const Val NA = -1;

    // Ptr and UPtr are an index into the registers args[]. The two styles of using args are
    // varyings and uniforms. Varyings use Ptr, have a stride associated with them, and are
    // evaluated everytime through the loop. Uniforms use UPtr, don't have a stride, and are
    // usually hoisted above the loop.
    struct Ptr { int ix; };
    struct UPtr : public Ptr {};

    bool operator!=(Ptr a, Ptr b);

    struct I32 {
        Builder* builder = nullptr;
        Val      id      = NA;
        explicit operator bool() const { return id != NA; }
        Builder* operator->()    const { return builder; }
    };

    struct F32 {
        Builder* builder = nullptr;
        Val      id      = NA;
        explicit operator bool() const { return id != NA; }
        Builder* operator->()    const { return builder; }
    };

    struct Color {
        F32 r,g,b,a;
        explicit operator bool() const { return r && g && b && a; }
        Builder* operator->()    const { return a.operator->(); }
    };

    struct HSLA {
        F32 h,s,l,a;
        explicit operator bool() const { return h && s && l && a; }
        Builder* operator->()    const { return a.operator->(); }
    };

    struct Coord {
        F32 x,y;
        explicit operator bool() const { return x && y; }
        Builder* operator->()    const { return x.operator->(); }
    };

    struct Uniform {
        UPtr ptr;
        int offset;
    };
    struct Uniforms {
        UPtr             base;
        std::vector<int> buf;

        Uniforms(UPtr ptr, int init) : base(ptr), buf(init) {}

        Uniform push(int val) {
            buf.push_back(val);
            return {base, (int)( sizeof(int)*(buf.size() - 1) )};
        }

        Uniform pushF(float val) {
            int bits;
            memcpy(&bits, &val, sizeof(int));
            return this->push(bits);
        }

        Uniform pushPtr(const void* ptr) {
            // Jam the pointer into 1 or 2 ints.
            int ints[sizeof(ptr) / sizeof(int)];
            memcpy(ints, &ptr, sizeof(ptr));
            for (int bits : ints) {
                buf.push_back(bits);
            }
            return {base, (int)( sizeof(int)*(buf.size() - SK_ARRAY_COUNT(ints)) )};
        }

        Uniform pushArray(int32_t a[]) {
            return this->pushPtr(a);
        }

        Uniform pushArrayF(float a[]) {
            return this->pushPtr(a);
        }
    };

    struct PixelFormat {
        enum { UNORM, SRGB, FLOAT} encoding;
        int r_bits,  g_bits,  b_bits,  a_bits,
            r_shift, g_shift, b_shift, a_shift;
    };
    PixelFormat SkColorType_to_PixelFormat(SkColorType);

    SK_BEGIN_REQUIRE_DENSE
    struct Instruction {
        Op  op;              // v* = op(x,y,z,w,immA,immB), where * == index of this Instruction.
        Val x,y,z,w;         // Enough arguments for Op::store128.
        int immA,immB,immC;  // Immediate bit pattern, shift count, pointer index, byte offset, etc.
    };
    SK_END_REQUIRE_DENSE

    bool operator==(const Instruction&, const Instruction&);
    struct InstructionHash {
        uint32_t operator()(const Instruction&, uint32_t seed=0) const;
    };

    struct OptimizedInstruction {
        Op op;
        Val x,y,z,w;
        int immA,immB,immC;

        Val  death;
        bool can_hoist;
    };

    struct Features {
        bool fma   = false;
        bool fp16  = false;
    };

    class Builder {
    public:

        Builder();
        explicit Builder(Features);

        Program done(const char* debug_name = nullptr, bool allow_jit=true) const;

        // Mostly for debugging, tests, etc.
        std::vector<Instruction> program() const { return fProgram; }
        std::vector<OptimizedInstruction> optimize() const;

        // Convenience arg() wrappers for most common strides, sizeof(T) and 0.
        template <typename T>
        Ptr varying() { return this->arg(sizeof(T)); }
        Ptr varying(int stride) { SkASSERT(stride > 0); return this->arg(stride); }
        UPtr uniform() { Ptr p = this->arg(0); return UPtr{{p.ix}}; }

        // TODO: allow uniform (i.e. Ptr) offsets to store* and load*?
        // TODO: sign extension (signed types) for <32-bit loads?
        // TODO: unsigned integer operations where relevant (just comparisons?)?

        // Assert cond is true, printing debug when not.
        void assert_true(I32 cond, I32 debug);
        void assert_true(I32 cond, F32 debug) { assert_true(cond, pun_to_I32(debug)); }
        void assert_true(I32 cond)            { assert_true(cond, cond); }

        // Store {8,16,32,64,128}-bit varying.
        void store8  (Ptr ptr, I32 val);
        void store16 (Ptr ptr, I32 val);
        void store32 (Ptr ptr, I32 val);
        void storeF  (Ptr ptr, F32 val) { store32(ptr, pun_to_I32(val)); }
        void store64 (Ptr ptr, I32 lo, I32 hi);              // *ptr = lo|(hi<<32)
        void store128(Ptr ptr, I32 x, I32 y, I32 z, I32 w);  // *ptr = x|(y<<32)|(z<<64)|(w<<96)

        // Returns varying {n, n-1, n-2, ..., 1}, where n is the argument to Program::eval().
        I32 index();

        // Load {8,16,32,64,128}-bit varying.
        I32 load8  (Ptr ptr);
        I32 load16 (Ptr ptr);
        I32 load32 (Ptr ptr);
        F32 loadF  (Ptr ptr) { return pun_to_F32(load32(ptr)); }
        I32 load64 (Ptr ptr, int lane);  // Load 32-bit lane 0-1 of  64-bit value.
        I32 load128(Ptr ptr, int lane);  // Load 32-bit lane 0-3 of 128-bit value.

        // Load i32/f32 uniform with byte-count offset.
        I32 uniform32(UPtr ptr, int offset);
        F32 uniformF (UPtr ptr, int offset) { return pun_to_F32(uniform32(ptr,offset)); }

        // Load i32/f32 uniform with byte-count offset and an c-style array index. The address of
        // the element is (*(ptr + byte-count offset))[index].
        I32 array32  (UPtr ptr, int offset, int index);
        F32 arrayF   (UPtr ptr, int offset, int index) {
            return pun_to_F32(array32(ptr, offset, index));
        }

        // Push and load this color as a uniform.
        Color uniformColor(SkColor4f, Uniforms*);

        // Gather u8,u16,i32 with varying element-count index from *(ptr + byte-count offset).
        I32 gather8 (UPtr ptr, int offset, I32 index);
        I32 gather16(UPtr ptr, int offset, I32 index);
        I32 gather32(UPtr ptr, int offset, I32 index);
        F32 gatherF (UPtr ptr, int offset, I32 index) {
            return pun_to_F32(gather32(ptr, offset, index));
        }

        // Convenience methods for working with skvm::Uniform(s).
        I32 uniform32(Uniform u)            { return this->uniform32(u.ptr, u.offset); }
        F32 uniformF (Uniform u)            { return this->uniformF (u.ptr, u.offset); }
        I32 gather8  (Uniform u, I32 index) { return this->gather8  (u.ptr, u.offset, index); }
        I32 gather16 (Uniform u, I32 index) { return this->gather16 (u.ptr, u.offset, index); }
        I32 gather32 (Uniform u, I32 index) { return this->gather32 (u.ptr, u.offset, index); }
        F32 gatherF  (Uniform u, I32 index) { return this->gatherF  (u.ptr, u.offset, index); }

        // Convenience methods for working with array pointers in skvm::Uniforms. Index is an
        // array index and not a byte offset. The array pointer is stored at u.
        I32 array32  (Uniform a, int index) { return this->array32  (a.ptr, a.offset, index); }
        F32 arrayF   (Uniform a, int index) { return this->arrayF   (a.ptr, a.offset, index); }

        // Load an immediate constant.
        I32 splat(int      n);
        I32 splat(unsigned u) { return splat((int)u); }
        F32 splat(float    f) {
            int bits;
            memcpy(&bits, &f, 4);
            return pun_to_F32(splat(bits));
        }

        // Some operations make sense with immediate arguments,
        // so we provide overloads inline to make that seamless.
        //
        // We omit overloads that may indicate a bug or performance issue.
        // In general it does not make sense to pass immediates to unary operations,
        // and even sometimes not for binary operations, e.g.
        //
        //   div(x, y)    -- normal every day divide
        //   div(3.0f, y) -- yep, makes sense
        //   div(x, 3.0f) -- omitted as a reminder you probably want mul(x, 1/3.0f).
        //
        // You can of course always splat() to override these opinions.

        // float math, comparisons, etc.
        F32 add(F32, F32);
        F32 add(F32 x, float y) { return add(x, splat(y)); }
        F32 add(float x, F32 y) { return add(splat(x), y); }

        F32 sub(F32, F32);
        F32 sub(F32 x, float y) { return sub(x, splat(y)); }
        F32 sub(float x, F32 y) { return sub(splat(x), y); }

        F32 mul(F32, F32);
        F32 mul(F32 x, float y) { return mul(x, splat(y)); }
        F32 mul(float x, F32 y) { return mul(splat(x), y); }

        // mul(), but allowing optimizations not strictly legal under IEEE-754 rules.
        F32 fast_mul(F32, F32);
        F32 fast_mul(F32 x, float y) { return fast_mul(x, splat(y)); }
        F32 fast_mul(float x, F32 y) { return fast_mul(splat(x), y); }

        F32 div(F32, F32);
        F32 div(float x, F32 y) { return div(splat(x), y); }

        F32 min(F32, F32);
        F32 min(F32 x, float y) { return min(x, splat(y)); }
        F32 min(float x, F32 y) { return min(splat(x), y); }

        F32 max(F32, F32);
        F32 max(F32 x, float y) { return max(x, splat(y)); }
        F32 max(float x, F32 y) { return max(splat(x), y); }

        // TODO: remove mad()?  It's just sugar.
        F32 mad(F32   x, F32   y, F32   z) { return add(mul(x,y), z); }
        F32 mad(F32   x, F32   y, float z) { return mad(      x ,       y , splat(z)); }
        F32 mad(F32   x, float y, F32   z) { return mad(      x , splat(y),       z ); }
        F32 mad(F32   x, float y, float z) { return mad(      x , splat(y), splat(z)); }
        F32 mad(float x, F32   y, F32   z) { return mad(splat(x),       y ,       z ); }
        F32 mad(float x, F32   y, float z) { return mad(splat(x),       y , splat(z)); }
        F32 mad(float x, float y, F32   z) { return mad(splat(x), splat(y),       z ); }

        F32        sqrt(F32);
        F32 approx_log2(F32);
        F32 approx_pow2(F32);
        F32 approx_log (F32 x) { return mul(0.69314718f, approx_log2(x)); }
        F32 approx_exp (F32 x) { return approx_pow2(mul(x, 1.4426950408889634074f)); }

        F32 approx_powf(F32 base, F32 exp);
        F32 approx_powf(F32 base, float exp) { return approx_powf(base, splat(exp)); }
        F32 approx_powf(float base, F32 exp) { return approx_powf(splat(base), exp); }


        F32 approx_sin(F32 radians);
        F32 approx_cos(F32 radians) { return approx_sin(add(radians, SK_ScalarPI/2)); }
        F32 approx_tan(F32 radians);

        F32 approx_asin(F32 x);
        F32 approx_acos(F32 x) { return sub(SK_ScalarPI/2, approx_asin(x)); }
        F32 approx_atan(F32 x);
        F32 approx_atan2(F32 y, F32 x);

        F32 lerp(F32   lo, F32   hi, F32   t);
        F32 lerp(F32   lo, F32   hi, float t) { return lerp(      lo ,       hi , splat(t)); }
        F32 lerp(F32   lo, float hi, float t) { return lerp(      lo , splat(hi), splat(t)); }
        F32 lerp(F32   lo, float hi, F32   t) { return lerp(      lo , splat(hi),       t ); }
        F32 lerp(float lo, F32   hi, F32   t) { return lerp(splat(lo),       hi ,       t ); }
        F32 lerp(float lo, F32   hi, float t) { return lerp(splat(lo),       hi , splat(t)); }
        F32 lerp(float lo, float hi, F32   t) { return lerp(splat(lo), splat(hi),       t ); }

        F32 clamp(F32   x, F32   lo, F32   hi) { return max(lo, min(x, hi)); }
        F32 clamp(F32   x, F32   lo, float hi) { return clamp(      x ,       lo , splat(hi)); }
        F32 clamp(F32   x, float lo, float hi) { return clamp(      x , splat(lo), splat(hi)); }
        F32 clamp(F32   x, float lo, F32   hi) { return clamp(      x , splat(lo),       hi ); }
        F32 clamp(float x, F32   lo, F32   hi) { return clamp(splat(x),       lo ,       hi ); }
        F32 clamp(float x, F32   lo, float hi) { return clamp(splat(x),       lo , splat(hi)); }
        F32 clamp(float x, float lo, F32   hi) { return clamp(splat(x), splat(lo),       hi ); }

        F32 clamp01(F32 x) { return clamp(x, 0.0f, 1.0f); }

        F32    abs(F32 x) { return pun_to_F32(bit_and(pun_to_I32(x), 0x7fff'ffff)); }
        F32  fract(F32 x) { return sub(x, floor(x)); }
        F32   ceil(F32);
        F32  floor(F32);
        I32 is_NaN   (F32 x) { return neq(x,x); }
        I32 is_finite(F32 x) { return lt(bit_and(pun_to_I32(x), 0x7f80'0000), 0x7f80'0000); }

        I32 trunc(F32 x);
        I32 round(F32 x);  // Round to int using current rounding mode (as if lrintf()).
        I32 pun_to_I32(F32 x) { return {x.builder, x.id}; }

        I32   to_fp16(F32 x);
        F32 from_fp16(I32 x);

        I32 eq(F32, F32);
        I32 eq(F32 x, float y) { return eq(x, splat(y)); }
        I32 eq(float x, F32 y) { return eq(splat(x), y); }

        I32 neq(F32, F32);
        I32 neq(F32 x, float y) { return neq(x, splat(y)); }
        I32 neq(float x, F32 y) { return neq(splat(x), y); }

        I32 lt(F32, F32);
        I32 lt(F32 x, float y) { return lt(x, splat(y)); }
        I32 lt(float x, F32 y) { return lt(splat(x), y); }

        I32 lte(F32, F32);
        I32 lte(F32 x, float y) { return lte(x, splat(y)); }
        I32 lte(float x, F32 y) { return lte(splat(x), y); }

        I32 gt(F32, F32);
        I32 gt(F32 x, float y) { return gt(x, splat(y)); }
        I32 gt(float x, F32 y) { return gt(splat(x), y); }

        I32 gte(F32, F32);
        I32 gte(F32 x, float y) { return gte(x, splat(y)); }
        I32 gte(float x, F32 y) { return gte(splat(x), y); }

        // int math, comparisons, etc.
        I32 add(I32, I32);
        I32 add(I32 x, int y) { return add(x, splat(y)); }
        I32 add(int x, I32 y) { return add(splat(x), y); }

        I32 sub(I32, I32);
        I32 sub(I32 x, int y) { return sub(x, splat(y)); }
        I32 sub(int x, I32 y) { return sub(splat(x), y); }

        I32 mul(I32, I32);
        I32 mul(I32 x, int y) { return mul(x, splat(y)); }
        I32 mul(int x, I32 y) { return mul(splat(x), y); }

        I32 shl(I32 x, int bits);
        I32 shr(I32 x, int bits);
        I32 sra(I32 x, int bits);

        I32 eq(I32, I32);
        I32 eq(I32 x, int y) { return eq(x, splat(y)); }
        I32 eq(int x, I32 y) { return eq(splat(x), y); }

        I32 neq(I32, I32);
        I32 neq(I32 x, int y) { return neq(x, splat(y)); }
        I32 neq(int x, I32 y) { return neq(splat(x), y); }

        I32 lt(I32, I32);
        I32 lt(I32 x, int y) { return lt(x, splat(y)); }
        I32 lt(int x, I32 y) { return lt(splat(x), y); }

        I32 lte(I32, I32);
        I32 lte(I32 x, int y) { return lte(x, splat(y)); }
        I32 lte(int x, I32 y) { return lte(splat(x), y); }

        I32 gt(I32, I32);
        I32 gt(I32 x, int y) { return gt(x, splat(y)); }
        I32 gt(int x, I32 y) { return gt(splat(x), y); }

        I32 gte(I32, I32);
        I32 gte(I32 x, int y) { return gte(x, splat(y)); }
        I32 gte(int x, I32 y) { return gte(splat(x), y); }

        F32 to_F32(I32 x);
        F32 pun_to_F32(I32 x) { return {x.builder, x.id}; }

        // Bitwise operations.
        I32 bit_and(I32, I32);
        I32 bit_and(I32 x, int y) { return bit_and(x, splat(y)); }
        I32 bit_and(int x, I32 y) { return bit_and(splat(x), y); }

        I32 bit_or(I32, I32);
        I32 bit_or(I32 x, int y) { return bit_or(x, splat(y)); }
        I32 bit_or(int x, I32 y) { return bit_or(splat(x), y); }

        I32 bit_xor(I32, I32);
        I32 bit_xor(I32 x, int y) { return bit_xor(x, splat(y)); }
        I32 bit_xor(int x, I32 y) { return bit_xor(splat(x), y); }

        I32 bit_clear(I32, I32);
        I32 bit_clear(I32 x, int y) { return bit_clear(x, splat(y)); }
        I32 bit_clear(int x, I32 y) { return bit_clear(splat(x), y); }

        I32 min(I32 x, I32 y) { return select(lte(x,y), x, y); }
        I32 min(I32 x, int y) { return min(x, splat(y)); }
        I32 min(int x, I32 y) { return min(splat(x), y); }

        I32 max(I32 x, I32 y) { return select(gte(x,y), x, y); }
        I32 max(I32 x, int y) { return max(x, splat(y)); }
        I32 max(int x, I32 y) { return max(splat(x), y); }

        I32 select(I32 cond, I32 t, I32 f);  // cond ? t : f
        I32 select(I32 cond, int t, I32 f) { return select(cond, splat(t),       f ); }
        I32 select(I32 cond, I32 t, int f) { return select(cond,       t , splat(f)); }
        I32 select(I32 cond, int t, int f) { return select(cond, splat(t), splat(f)); }

        F32 select(I32 cond, F32 t, F32 f) {
            return pun_to_F32(select(cond, pun_to_I32(t)
                                         , pun_to_I32(f)));
        }
        F32 select(I32 cond, float t, F32   f) { return select(cond, splat(t),       f ); }
        F32 select(I32 cond, F32   t, float f) { return select(cond,       t , splat(f)); }
        F32 select(I32 cond, float t, float f) { return select(cond, splat(t), splat(f)); }

        I32 extract(I32 x, int bits, I32 z);   // (x>>bits) & z
        I32 extract(I32 x, int bits, int z) { return extract(x, bits, splat(z)); }
        I32 extract(int x, int bits, I32 z) { return extract(splat(x), bits, z); }

        I32 pack(I32 x, I32 y, int bits);   // x | (y<<bits)
        I32 pack(I32 x, int y, int bits) { return pack(x, splat(y), bits); }
        I32 pack(int x, I32 y, int bits) { return pack(splat(x), y, bits); }


        // Common idioms used in several places, worth centralizing for consistency.
        F32 from_unorm(int bits, I32);   // E.g. from_unorm(8, x) -> x * (1/255.0f)
        I32   to_unorm(int bits, F32);   // E.g.   to_unorm(8, x) -> round(x * 255)

        Color   load(PixelFormat, Ptr ptr);
        void   store(PixelFormat, Ptr ptr, Color);
        Color gather(PixelFormat, UPtr ptr, int offset, I32 index);
        Color gather(PixelFormat f, Uniform u, I32 index) {
            return gather(f, u.ptr, u.offset, index);
        }

        void   premul(F32* r, F32* g, F32* b, F32 a);
        void unpremul(F32* r, F32* g, F32* b, F32 a);

        Color   premul(Color c) {   this->premul(&c.r, &c.g, &c.b, c.a); return c; }
        Color unpremul(Color c) { this->unpremul(&c.r, &c.g, &c.b, c.a); return c; }

        Color lerp(Color lo, Color hi, F32 t);
        Color blend(SkBlendMode, Color src, Color dst);

        Color clamp01(Color c) {
            return { clamp01(c.r), clamp01(c.g), clamp01(c.b), clamp01(c.a) };
        }

        HSLA  to_hsla(Color);
        Color to_rgba(HSLA);

        void dump(SkWStream* = nullptr) const;

        uint64_t hash() const;

        Val push(Instruction);

        bool allImm() const { return true; }

        template <typename T, typename... Rest>
        bool allImm(Val id, T* imm, Rest... rest) const {
            if (fProgram[id].op == Op::splat) {
                static_assert(sizeof(T) == 4);
                memcpy(imm, &fProgram[id].immA, 4);
                return this->allImm(rest...);
            }
            return false;
        }

        bool allUniform() const { return true; }

        template <typename... Rest>
        bool allUniform(Val id, Uniform* uni, Rest... rest) const {
            if (fProgram[id].op == Op::uniform32) {
                uni->ptr.ix = fProgram[id].immA;
                uni->offset = fProgram[id].immB;
                return this->allUniform(rest...);
            }
            return false;
        }

    private:
        // Declare an argument with given stride (use stride=0 for uniforms).
        Ptr arg(int stride);

        Val push(
                Op op, Val x=NA, Val y=NA, Val z=NA, Val w=NA, int immA=0, int immB=0, int immC=0) {
            return this->push(Instruction{op, x,y,z,w, immA,immB,immC});
        }

        template <typename T>
        bool isImm(Val id, T want) const {
            T imm = 0;
            return this->allImm(id, &imm) && imm == want;
        }

        SkTHashMap<Instruction, Val, InstructionHash> fIndex;
        std::vector<Instruction>                      fProgram;
        std::vector<int>                              fStrides;
        const Features                                fFeatures;
    };

    // Optimization passes and data structures normally used by Builder::optimize(),
    // extracted here so they can be unit tested.
    std::vector<Instruction>          eliminate_dead_code(std::vector<Instruction>);
    std::vector<OptimizedInstruction> finalize           (std::vector<Instruction>);

    using Reg = int;

    // d = op(x,y,z,w, immA,immB)
    struct InterpreterInstruction {
        Op  op;
        Reg d,x,y,z,w;
        int immA,immB,immC;
    };

    class Program {
    public:
        Program(const std::vector<OptimizedInstruction>& instructions,
                const std::vector<int>& strides,
                const char* debug_name, bool allow_jit);

        Program();
        ~Program();

        Program(Program&&);
        Program& operator=(Program&&);

        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;

        void eval(int n, void* args[]) const;

        template <typename... T>
        void eval(int n, T*... arg) const {
            SkASSERT(sizeof...(arg) == this->nargs());
            // This nullptr isn't important except that it makes args[] non-empty if you pass none.
            void* args[] = { (void*)arg..., nullptr };
            this->eval(n, args);
        }

        std::vector<InterpreterInstruction> instructions() const;
        int  nargs() const;
        int  nregs() const;
        int  loop () const;
        bool empty() const;

        bool hasJIT() const;  // Has this Program been JITted?

        void dump(SkWStream* = nullptr) const;

    private:
        void setupInterpreter(const std::vector<OptimizedInstruction>&);
        void setupJIT        (const std::vector<OptimizedInstruction>&, const char* debug_name);
        void setupLLVM       (const std::vector<OptimizedInstruction>&, const char* debug_name);

        bool jit(const std::vector<OptimizedInstruction>&,
                 int* stack_hint, uint32_t* registers_used,
                 Assembler*) const;

        void waitForLLVM() const;
        void dropJIT();

        struct Impl;
        std::unique_ptr<Impl> fImpl;
    };

    // TODO: control flow
    // TODO: 64-bit values?

#define SI static inline

    SI I32 operator+(I32 x, I32 y) { return x->add(x,y); }
    SI I32 operator+(I32 x, int y) { return x->add(x,y); }
    SI I32 operator+(int x, I32 y) { return y->add(x,y); }

    SI I32 operator-(I32 x, I32 y) { return x->sub(x,y); }
    SI I32 operator-(I32 x, int y) { return x->sub(x,y); }
    SI I32 operator-(int x, I32 y) { return y->sub(x,y); }

    SI I32 operator*(I32 x, I32 y) { return x->mul(x,y); }
    SI I32 operator*(I32 x, int y) { return x->mul(x,y); }
    SI I32 operator*(int x, I32 y) { return y->mul(x,y); }

    SI I32 min(I32 x, I32 y) { return x->min(x,y); }
    SI I32 min(I32 x, int y) { return x->min(x,y); }
    SI I32 min(int x, I32 y) { return y->min(x,y); }

    SI I32 max(I32 x, I32 y) { return x->max(x,y); }
    SI I32 max(I32 x, int y) { return x->max(x,y); }
    SI I32 max(int x, I32 y) { return y->max(x,y); }

    SI I32 operator==(I32 x, I32 y) { return x->eq(x,y); }
    SI I32 operator==(I32 x, int y) { return x->eq(x,y); }
    SI I32 operator==(int x, I32 y) { return y->eq(x,y); }

    SI I32 operator!=(I32 x, I32 y) { return x->neq(x,y); }
    SI I32 operator!=(I32 x, int y) { return x->neq(x,y); }
    SI I32 operator!=(int x, I32 y) { return y->neq(x,y); }

    SI I32 operator< (I32 x, I32 y) { return x->lt(x,y); }
    SI I32 operator< (I32 x, int y) { return x->lt(x,y); }
    SI I32 operator< (int x, I32 y) { return y->lt(x,y); }

    SI I32 operator<=(I32 x, I32 y) { return x->lte(x,y); }
    SI I32 operator<=(I32 x, int y) { return x->lte(x,y); }
    SI I32 operator<=(int x, I32 y) { return y->lte(x,y); }

    SI I32 operator> (I32 x, I32 y) { return x->gt(x,y); }
    SI I32 operator> (I32 x, int y) { return x->gt(x,y); }
    SI I32 operator> (int x, I32 y) { return y->gt(x,y); }

    SI I32 operator>=(I32 x, I32 y) { return x->gte(x,y); }
    SI I32 operator>=(I32 x, int y) { return x->gte(x,y); }
    SI I32 operator>=(int x, I32 y) { return y->gte(x,y); }


    SI F32 operator+(F32   x, F32   y) { return x->add(x,y); }
    SI F32 operator+(F32   x, float y) { return x->add(x,y); }
    SI F32 operator+(float x, F32   y) { return y->add(x,y); }

    SI F32 operator-(F32   x, F32   y) { return x->sub(x,y); }
    SI F32 operator-(F32   x, float y) { return x->sub(x,y); }
    SI F32 operator-(float x, F32   y) { return y->sub(x,y); }

    SI F32 operator*(F32   x, F32   y) { return x->mul(x,y); }
    SI F32 operator*(F32   x, float y) { return x->mul(x,y); }
    SI F32 operator*(float x, F32   y) { return y->mul(x,y); }

    SI F32 fast_mul(F32   x, F32   y) { return x->fast_mul(x,y); }
    SI F32 fast_mul(F32   x, float y) { return x->fast_mul(x,y); }
    SI F32 fast_mul(float x, F32   y) { return y->fast_mul(x,y); }

    SI F32 operator/(F32   x, F32  y) { return x->div(x,y); }
    SI F32 operator/(float x, F32  y) { return y->div(x,y); }

    SI F32 min(F32   x, F32   y) { return x->min(x,y); }
    SI F32 min(F32   x, float y) { return x->min(x,y); }
    SI F32 min(float x, F32   y) { return y->min(x,y); }

    SI F32 max(F32   x, F32   y) { return x->max(x,y); }
    SI F32 max(F32   x, float y) { return x->max(x,y); }
    SI F32 max(float x, F32   y) { return y->max(x,y); }

    SI I32 operator==(F32   x, F32   y) { return x->eq(x,y); }
    SI I32 operator==(F32   x, float y) { return x->eq(x,y); }
    SI I32 operator==(float x, F32   y) { return y->eq(x,y); }

    SI I32 operator!=(F32   x, F32   y) { return x->neq(x,y); }
    SI I32 operator!=(F32   x, float y) { return x->neq(x,y); }
    SI I32 operator!=(float x, F32   y) { return y->neq(x,y); }

    SI I32 operator< (F32   x, F32   y) { return x->lt(x,y); }
    SI I32 operator< (F32   x, float y) { return x->lt(x,y); }
    SI I32 operator< (float x, F32   y) { return y->lt(x,y); }

    SI I32 operator<=(F32   x, F32   y) { return x->lte(x,y); }
    SI I32 operator<=(F32   x, float y) { return x->lte(x,y); }
    SI I32 operator<=(float x, F32   y) { return y->lte(x,y); }

    SI I32 operator> (F32   x, F32   y) { return x->gt(x,y); }
    SI I32 operator> (F32   x, float y) { return x->gt(x,y); }
    SI I32 operator> (float x, F32   y) { return y->gt(x,y); }

    SI I32 operator>=(F32   x, F32   y) { return x->gte(x,y); }
    SI I32 operator>=(F32   x, float y) { return x->gte(x,y); }
    SI I32 operator>=(float x, F32   y) { return y->gte(x,y); }

    SI I32& operator+=(I32& x, I32 y) { return (x = x + y); }
    SI I32& operator+=(I32& x, int y) { return (x = x + y); }

    SI I32& operator-=(I32& x, I32 y) { return (x = x - y); }
    SI I32& operator-=(I32& x, int y) { return (x = x - y); }

    SI I32& operator*=(I32& x, I32 y) { return (x = x * y); }
    SI I32& operator*=(I32& x, int y) { return (x = x * y); }

    SI F32& operator+=(F32& x, F32   y) { return (x = x + y); }
    SI F32& operator+=(F32& x, float y) { return (x = x + y); }

    SI F32& operator-=(F32& x, F32   y) { return (x = x - y); }
    SI F32& operator-=(F32& x, float y) { return (x = x - y); }

    SI F32& operator*=(F32& x, F32   y) { return (x = x * y); }
    SI F32& operator*=(F32& x, float y) { return (x = x * y); }

    SI F32& operator/=(F32& x, F32   y) { return (x = x / y); }

    SI void assert_true(I32 cond, I32 debug) { cond->assert_true(cond,debug); }
    SI void assert_true(I32 cond, F32 debug) { cond->assert_true(cond,debug); }
    SI void assert_true(I32 cond)            { cond->assert_true(cond); }

    SI void store8  (Ptr ptr, I32 val)                    { val->store8  (ptr, val); }
    SI void store16 (Ptr ptr, I32 val)                    { val->store16 (ptr, val); }
    SI void store32 (Ptr ptr, I32 val)                    { val->store32 (ptr, val); }
    SI void storeF  (Ptr ptr, F32 val)                    { val->storeF  (ptr, val); }
    SI void store64 (Ptr ptr, I32 lo, I32 hi)             { lo ->store64 (ptr, lo,hi); }
    SI void store128(Ptr ptr, I32 x, I32 y, I32 z, I32 w) { x  ->store128(ptr, x,y,z,w); }

    SI I32 gather8 (UPtr ptr, int off, I32 ix) { return ix->gather8 (ptr, off, ix); }
    SI I32 gather16(UPtr ptr, int off, I32 ix) { return ix->gather16(ptr, off, ix); }
    SI I32 gather32(UPtr ptr, int off, I32 ix) { return ix->gather32(ptr, off, ix); }
    SI F32 gatherF (UPtr ptr, int off, I32 ix) { return ix->gatherF (ptr, off, ix); }

    SI I32 gather8 (Uniform u, I32 ix) { return ix->gather8 (u, ix); }
    SI I32 gather16(Uniform u, I32 ix) { return ix->gather16(u, ix); }
    SI I32 gather32(Uniform u, I32 ix) { return ix->gather32(u, ix); }
    SI F32 gatherF (Uniform u, I32 ix) { return ix->gatherF (u, ix); }

    SI F32        sqrt(F32 x) { return x->       sqrt(x); }
    SI F32 approx_log2(F32 x) { return x->approx_log2(x); }
    SI F32 approx_pow2(F32 x) { return x->approx_pow2(x); }
    SI F32 approx_log (F32 x) { return x->approx_log (x); }
    SI F32 approx_exp (F32 x) { return x->approx_exp (x); }

    SI F32 approx_powf(F32   base, F32   exp) { return base->approx_powf(base, exp); }
    SI F32 approx_powf(F32   base, float exp) { return base->approx_powf(base, exp); }
    SI F32 approx_powf(float base, F32   exp) { return  exp->approx_powf(base, exp); }

    SI F32 approx_sin(F32 radians) { return radians->approx_sin(radians); }
    SI F32 approx_cos(F32 radians) { return radians->approx_cos(radians); }
    SI F32 approx_tan(F32 radians) { return radians->approx_tan(radians); }

    SI F32 approx_asin(F32 x) { return x->approx_asin(x); }
    SI F32 approx_acos(F32 x) { return x->approx_acos(x); }
    SI F32 approx_atan(F32 x) { return x->approx_atan(x); }
    SI F32 approx_atan2(F32 y, F32 x) { return x->approx_atan2(y, x); }

    SI F32   clamp01(F32 x) { return x->  clamp01(x); }
    SI F32       abs(F32 x) { return x->      abs(x); }
    SI F32      ceil(F32 x) { return x->     ceil(x); }
    SI F32     fract(F32 x) { return x->    fract(x); }
    SI F32     floor(F32 x) { return x->    floor(x); }
    SI I32    is_NaN(F32 x) { return x->   is_NaN(x); }
    SI I32 is_finite(F32 x) { return x->is_finite(x); }

    SI I32      trunc(F32 x) { return x->      trunc(x); }
    SI I32      round(F32 x) { return x->      round(x); }
    SI I32 pun_to_I32(F32 x) { return x-> pun_to_I32(x); }
    SI F32 pun_to_F32(I32 x) { return x-> pun_to_F32(x); }
    SI F32     to_F32(I32 x) { return x->     to_F32(x); }
    SI I32    to_fp16(F32 x) { return x->    to_fp16(x); }
    SI F32  from_fp16(I32 x) { return x->  from_fp16(x); }

    SI F32 lerp(F32   lo, F32   hi, F32   t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(F32   lo, F32   hi, float t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(F32   lo, float hi, F32   t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(F32   lo, float hi, float t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(float lo, F32   hi, F32   t) { return hi->lerp(lo,hi,t); }
    SI F32 lerp(float lo, F32   hi, float t) { return hi->lerp(lo,hi,t); }
    SI F32 lerp(float lo, float hi, F32   t) { return  t->lerp(lo,hi,t); }

    SI F32 clamp(F32   x, F32   lo, F32   hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(F32   x, F32   lo, float hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(F32   x, float lo, F32   hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(F32   x, float lo, float hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(float x, F32   lo, F32   hi) { return lo->clamp(x,lo,hi); }
    SI F32 clamp(float x, F32   lo, float hi) { return lo->clamp(x,lo,hi); }
    SI F32 clamp(float x, float lo, F32   hi) { return hi->clamp(x,lo,hi); }

    SI I32 operator<<(I32 x, int bits) { return x->shl(x, bits); }
    SI I32        shl(I32 x, int bits) { return x->shl(x, bits); }
    SI I32        shr(I32 x, int bits) { return x->shr(x, bits); }
    SI I32        sra(I32 x, int bits) { return x->sra(x, bits); }

    SI I32 operator&(I32 x, I32 y) { return x->bit_and(x,y); }
    SI I32 operator&(I32 x, int y) { return x->bit_and(x,y); }
    SI I32 operator&(int x, I32 y) { return y->bit_and(x,y); }

    SI I32 operator|(I32 x, I32 y) { return x->bit_or (x,y); }
    SI I32 operator|(I32 x, int y) { return x->bit_or (x,y); }
    SI I32 operator|(int x, I32 y) { return y->bit_or (x,y); }

    SI I32 operator^(I32 x, I32 y) { return x->bit_xor(x,y); }
    SI I32 operator^(I32 x, int y) { return x->bit_xor(x,y); }
    SI I32 operator^(int x, I32 y) { return y->bit_xor(x,y); }

    SI I32& operator&=(I32& x, I32 y) { return (x = x & y); }
    SI I32& operator&=(I32& x, int y) { return (x = x & y); }
    SI I32& operator|=(I32& x, I32 y) { return (x = x | y); }
    SI I32& operator|=(I32& x, int y) { return (x = x | y); }
    SI I32& operator^=(I32& x, I32 y) { return (x = x ^ y); }
    SI I32& operator^=(I32& x, int y) { return (x = x ^ y); }

    SI I32 bit_clear(I32 x, I32 y) { return x->bit_clear(x,y); }
    SI I32 bit_clear(I32 x, int y) { return x->bit_clear(x,y); }
    SI I32 bit_clear(int x, I32 y) { return y->bit_clear(x,y); }

    SI I32 select(I32 c, I32 t, I32 f) { return c->select(c,          t ,          f ); }
    SI I32 select(I32 c, I32 t, int f) { return c->select(c,          t , c->splat(f)); }
    SI I32 select(I32 c, int t, I32 f) { return c->select(c, c->splat(t),          f ); }
    SI I32 select(I32 c, int t, int f) { return c->select(c, c->splat(t), c->splat(f)); }

    SI F32 select(I32 c, F32   t, F32   f) { return c->select(c,          t ,          f ); }
    SI F32 select(I32 c, F32   t, float f) { return c->select(c,          t , c->splat(f)); }
    SI F32 select(I32 c, float t, F32   f) { return c->select(c, c->splat(t),          f ); }
    SI F32 select(I32 c, float t, float f) { return c->select(c, c->splat(t), c->splat(f)); }

    SI I32 extract(I32 x, int bits, I32 z) { return x->extract(x,bits,z); }
    SI I32 extract(I32 x, int bits, int z) { return x->extract(x,bits,z); }
    SI I32 extract(int x, int bits, I32 z) { return z->extract(x,bits,z); }

    SI I32 pack(I32 x, I32 y, int bits) { return x->pack   (x,y,bits); }
    SI I32 pack(I32 x, int y, int bits) { return x->pack   (x,y,bits); }
    SI I32 pack(int x, I32 y, int bits) { return y->pack   (x,y,bits); }

    SI I32 operator~(I32 x) { return ~0 ^ x; }
    SI I32 operator-(I32 x) { return  0 - x; }
    SI F32 operator-(F32 x) { return 0.0f - x; }

    SI F32 from_unorm(int bits, I32 x) { return x->from_unorm(bits,x); }
    SI I32   to_unorm(int bits, F32 x) { return x->  to_unorm(bits,x); }

    SI void store(PixelFormat f, Ptr p, Color c) { return c->store(f,p,c); }

    SI Color gather(PixelFormat f, UPtr p, int off, I32 ix) { return ix->gather(f,p,off,ix); }
    SI Color gather(PixelFormat f, Uniform u     , I32 ix)  { return ix->gather(f,u,ix); }

    SI void   premul(F32* r, F32* g, F32* b, F32 a) { a->  premul(r,g,b,a); }
    SI void unpremul(F32* r, F32* g, F32* b, F32 a) { a->unpremul(r,g,b,a); }

    SI Color   premul(Color c) { return c->  premul(c); }
    SI Color unpremul(Color c) { return c->unpremul(c); }

    SI Color lerp(Color lo, Color hi, F32 t) { return t->lerp(lo,hi,t); }

    SI Color blend(SkBlendMode m, Color s, Color d) { return s->blend(m,s,d); }

    SI Color clamp01(Color c) { return c->clamp01(c); }

    SI HSLA  to_hsla(Color c) { return c->to_hsla(c); }
    SI Color to_rgba(HSLA  c) { return c->to_rgba(c); }

    // Evaluate polynomials: ax^n + bx^(n-1) + ... for n >= 1
    template <typename F32_or_float, typename... Rest>
    SI F32 poly(F32 x, F32_or_float a, float b, Rest... rest) {
        if constexpr (sizeof...(rest) == 0) {
            return x*a+b;
        } else {
            return poly(x, x*a+b, rest...);
        }
    }
#undef SI
}  // namespace skvm

#endif//SkVM_DEFINED
