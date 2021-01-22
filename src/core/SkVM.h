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
#include "include/private/SkMacros.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkSpan.h"
#include "src/core/SkVM_fwd.h"
#include <vector>      // std::vector

class SkWStream;

#if defined(SKVM_JIT_WHEN_POSSIBLE)
    #if defined(__x86_64__) || defined(_M_X64)
        #if defined(_WIN32) || defined(__linux) || defined(__APPLE__)
            #if !defined(SK_BUILD_FOR_IOS)  // Exclude iOS simulator.
                #define SKVM_JIT
            #endif
        #endif
    #endif
    #if defined(__aarch64__)
        #if defined(__ANDROID__)
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

    bool fma_supported();

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
              tbl;

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
             scvtf4s,   // int -> float
             fcvtzs4s,  // truncate float -> int
             fcvtns4s,  // round float -> int  (nearest even)
             xtns2h,    // u32 -> u16
             xtnh2b,    // u16 -> u8
             uxtlb2h,   // u8 -> u16
             uxtlh2s,   // u16 -> u32
             uminv4s;   // dst[0] = min(n[0],n[1],n[2],n[3]), n as unsigned

        void brk (int imm16);
        void ret (X);
        void add (X d, X n, int imm12);
        void sub (X d, X n, int imm12);
        void subs(X d, X n, int imm12);  // subtract setting condition flags

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

        void ldrq(V dst, Label*);  // 128-bit PC-relative load

        void ldrq(V dst, X src, int imm12=0);  // 128-bit dst = *(src+imm12*16)
        void ldrs(V dst, X src, int imm12=0);  //  32-bit dst = *(src+imm12*4)
        void ldrb(V dst, X src, int imm12=0);  //   8-bit dst = *(src+imm12)

        void strq(V src, X dst, int imm12=0);  // 128-bit *(dst+imm12*16) = src
        void strs(V src, X dst, int imm12=0);  //  32-bit *(dst+imm12*4)  = src
        void strb(V src, X dst, int imm12=0);  //   8-bit *(dst+imm12)    = src

        void fmovs(X dst, V src); // dst = 32-bit src[0]

    private:
        // TODO: can probably track two of these three?
        uint8_t* fCode;
        uint8_t* fCurr;
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
        M(index)                                                     \
        M(load8)    M(load16)    M(load32)  M(load64) M(load128)     \
        M(gather8)  M(gather16)  M(gather32)                         \
        M(uniform8) M(uniform16) M(uniform32)                        \
        M(splat) M(splat_q14)                                        \
        M(add_f32) M(add_i32) M(add_q14)                             \
        M(sub_f32) M(sub_i32) M(sub_q14)                             \
        M(mul_f32) M(mul_i32) M(mul_q14)                             \
        M(div_f32)                                                   \
        M(min_f32) M(max_f32)                                        \
        M(min_q14) M(max_q14) M(uavg_q14)                            \
        M(fma_f32) M(fms_f32) M(fnma_f32)                            \
        M(sqrt_f32)                                                  \
        M(shl_i32) M(shr_i32) M(sra_i32)                             \
        M(shl_q14) M(shr_q14) M(sra_q14)                             \
        M(ceil) M(floor) M(trunc) M(round) M(to_half) M(from_half)   \
        M(to_f32) M(to_q14) M(from_q14)                              \
        M(neq_f32) M(eq_f32) M(eq_i32) M(eq_q14)                     \
        M(gte_f32) M(gt_f32) M(gt_i32) M(gt_q14)                     \
        M(bit_and)     M(bit_or)     M(bit_xor)     M(bit_clear)     \
        M(bit_and_q14) M(bit_or_q14) M(bit_xor_q14) M(bit_clear_q14) \
        M(select) M(select_q14) M(pack)                              \
    // End of SKVM_OPS

    enum class Op : int {
    #define M(op) op,
        SKVM_OPS(M)
    #undef M
    };

    static inline bool has_side_effect(Op op) {
        return op <= Op::store128;
    }
    static inline bool is_always_varying(Op op) {
        return op <= Op::gather32 && op != Op::assert_true;
    }

    using Val = int;
    // We reserve an impossibe Val ID as a sentinel
    // NA meaning none, n/a, null, nil, etc.
    static const Val NA = -1;

    struct Arg { int ix; };

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

    struct Q14 {
        Builder* builder = nullptr;
        Val      id      = NA;
        explicit operator bool() const { return id != NA; }
        Builder* operator->()    const { return builder; }
    };

    // Some operations make sense with immediate arguments,
    // so we use I32a and F32a to receive them transparently.
    //
    // We omit overloads that may indicate a bug or performance issue.
    // In general it does not make sense to pass immediates to unary operations,
    // and even sometimes not for binary operations, e.g.
    //
    //   div(x,y)    -- normal every day divide
    //   div(3.0f,y) -- yep, makes sense
    //   div(x,3.0f) -- omitted as a reminder you probably want mul(x, 1/3.0f).
    //
    // You can of course always splat() to override these opinions.
    struct I32a {
        I32a(I32 v) : SkDEBUGCODE(builder(v.builder),) id(v.id) {}
        I32a(int v) : imm(v) {}

        SkDEBUGCODE(Builder* builder = nullptr;)
        Val id  = NA;
        int imm = 0;
    };

    struct F32a {
        F32a(F32   v) : SkDEBUGCODE(builder(v.builder),) id(v.id) {}
        F32a(float v) : imm(v) {}

        SkDEBUGCODE(Builder* builder = nullptr;)
        Val   id  = NA;
        float imm = 0;
    };

    struct Q14a {
        Q14a(Q14 v) : SkDEBUGCODE(builder(v.builder),) id(v.id) {}
        Q14a(int bits) : imm{SkTo<int16_t>(bits)} {}   // 0x0000'4000 -> 0x4000
        Q14a(float f) : Q14a{(int)(f * 16384.0f)} {}   //        1.0f -> 0x4000

        SkDEBUGCODE(Builder* builder = nullptr;)
        Val     id  = NA;
        int16_t imm = 0;
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

    struct Color_Q14 {
        Q14 r,g,b,a;
        explicit operator bool() const { return r && g && b && a; }
        Builder* operator->()    const { return a.operator->(); }
    };

    struct Coord {
        F32 x,y;
        explicit operator bool() const { return x && y; }
        Builder* operator->()    const { return x.operator->(); }
    };

    struct Uniform {
        Arg ptr;
        int offset;
    };
    struct Uniforms {
        Arg              base;
        std::vector<int> buf;

        explicit Uniforms(int init) : base(Arg{0}), buf(init) {}

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
    };

    struct PixelFormat {
        enum { UNORM, FLOAT} encoding;
        int r_bits,  g_bits,  b_bits,  a_bits,
            r_shift, g_shift, b_shift, a_shift;
    };
    bool SkColorType_to_PixelFormat(SkColorType, PixelFormat*);

    SK_BEGIN_REQUIRE_DENSE
    struct Instruction {
        Op  op;         // v* = op(x,y,z,imm), where * == index of this Instruction.
        Val x,y,z;      // Enough arguments for mad().
        int immy,immz;  // Immediate bit pattern, shift count, argument index, etc.
    };
    SK_END_REQUIRE_DENSE

    bool operator==(const Instruction&, const Instruction&);
    struct InstructionHash {
        uint32_t operator()(const Instruction&, uint32_t seed=0) const;
    };

    struct OptimizedInstruction {
        Op op;
        Val x,y,z;
        int immy,immz;

        Val  death;
        bool can_hoist;
    };

    class Builder {
    public:

        Program done(const char* debug_name = nullptr) const;

        // Mostly for debugging, tests, etc.
        std::vector<Instruction> program() const { return fProgram; }
        std::vector<OptimizedInstruction> optimize() const;

        // Declare an argument with given stride (use stride=0 for uniforms).
        // TODO: different types for varying and uniforms?
        Arg arg(int stride);

        // Convenience arg() wrappers for most common strides, sizeof(T) and 0.
        template <typename T>
        Arg varying() { return this->arg(sizeof(T)); }
        Arg uniform() { return this->arg(0); }

        // TODO: allow uniform (i.e. Arg) offsets to store* and load*?
        // TODO: sign extension (signed types) for <32-bit loads?
        // TODO: unsigned integer operations where relevant (just comparisons?)?

        // Assert cond is true, printing debug when not.
        void assert_true(I32 cond, I32 debug);
        void assert_true(I32 cond, F32 debug) { assert_true(cond, bit_cast(debug)); }
        void assert_true(I32 cond)            { assert_true(cond, cond); }

        // Store {8,16,32,64,128}-bit varying.
        void store8  (Arg ptr, I32 val);
        void store16 (Arg ptr, I32 val);
        void store32 (Arg ptr, I32 val);
        void storeF  (Arg ptr, F32 val) { store32(ptr, bit_cast(val)); }
        void store64 (Arg ptr, I32 lo, I32 hi);            // *ptr = lo|(hi<<32)
        void store128(Arg ptr, I32 lo, I32 hi, int lane);  // 64-bit lane 0-1 at ptr = lo|(hi<<32).

        // Returns varying {n, n-1, n-2, ..., 1}, where n is the argument to Program::eval().
        I32 index();

        // Load {8,16,32,64,128}-bit varying.
        I32 load8  (Arg ptr);
        I32 load16 (Arg ptr);
        I32 load32 (Arg ptr);
        F32 loadF  (Arg ptr) { return bit_cast(load32(ptr)); }
        I32 load64 (Arg ptr, int lane);  // Load 32-bit lane 0-1 of  64-bit value.
        I32 load128(Arg ptr, int lane);  // Load 32-bit lane 0-3 of 128-bit value.

        // Load u8,u16,i32 uniform with byte-count offset.
        I32 uniform8 (Arg ptr, int offset);
        I32 uniform16(Arg ptr, int offset);
        I32 uniform32(Arg ptr, int offset);
        F32 uniformF (Arg ptr, int offset) { return this->bit_cast(this->uniform32(ptr,offset)); }

        // Push and load this color as a uniform.
        Color uniformColor(SkColor4f, Uniforms*);

        // Gather u8,u16,i32 with varying element-count index from *(ptr + byte-count offset).
        I32 gather8 (Arg ptr, int offset, I32 index);
        I32 gather16(Arg ptr, int offset, I32 index);
        I32 gather32(Arg ptr, int offset, I32 index);
        F32 gatherF (Arg ptr, int offset, I32 index) {
            return bit_cast(gather32(ptr, offset, index));
        }

        // Convenience methods for working with skvm::Uniform(s).
        I32 uniform8 (Uniform u)            { return this->uniform8 (u.ptr, u.offset); }
        I32 uniform16(Uniform u)            { return this->uniform16(u.ptr, u.offset); }
        I32 uniform32(Uniform u)            { return this->uniform32(u.ptr, u.offset); }
        F32 uniformF (Uniform u)            { return this->uniformF (u.ptr, u.offset); }
        I32 gather8  (Uniform u, I32 index) { return this->gather8  (u.ptr, u.offset, index); }
        I32 gather16 (Uniform u, I32 index) { return this->gather16 (u.ptr, u.offset, index); }
        I32 gather32 (Uniform u, I32 index) { return this->gather32 (u.ptr, u.offset, index); }
        F32 gatherF  (Uniform u, I32 index) { return this->gatherF  (u.ptr, u.offset, index); }

        // Load an immediate constant.
        I32 splat(int      n);
        I32 splat(unsigned u) { return splat((int)u); }
        F32 splat(float    f) {
            int bits;
            memcpy(&bits, &f, 4);
            return bit_cast(splat(bits));
        }

        // Load an immediate Q14, expressed as either integer (16384, 0x4000) or float (1.0f).
        Q14 splat_q14(int      n);
        Q14 splat_q14(unsigned u) { return splat_q14((int)u); }
        Q14 splat_q14(float    f) { return splat_q14(Q14a{f}.imm); }

        // float math, comparisons, etc.
        F32 add(F32, F32);  F32 add(F32a x, F32a y) { return add(_(x), _(y)); }
        F32 sub(F32, F32);  F32 sub(F32a x, F32a y) { return sub(_(x), _(y)); }
        F32 mul(F32, F32);  F32 mul(F32a x, F32a y) { return mul(_(x), _(y)); }
        F32 div(F32, F32);  F32 div(F32a x, F32  y) { return div(_(x),   y ); }
        F32 min(F32, F32);  F32 min(F32a x, F32a y) { return min(_(x), _(y)); }
        F32 max(F32, F32);  F32 max(F32a x, F32a y) { return max(_(x), _(y)); }

        F32 mad(F32  x, F32  y, F32  z) { return add(mul(x,y), z); }
        F32 mad(F32a x, F32a y, F32a z) { return mad(_(x), _(y), _(z)); }

        F32        sqrt(F32);
        F32 approx_log2(F32);
        F32 approx_pow2(F32);
        F32 approx_log (F32 x) { return mul(0.69314718f, approx_log2(x)); }
        F32 approx_exp (F32 x) { return approx_pow2(mul(x, 1.4426950408889634074f)); }

        F32 approx_powf(F32  base, F32  exp);
        F32 approx_powf(F32a base, F32a exp) { return approx_powf(_(base), _(exp)); }

        F32 approx_sin(F32 radians);
        F32 approx_cos(F32 radians) { return approx_sin(add(radians, SK_ScalarPI/2)); }
        F32 approx_tan(F32 radians);

        F32 approx_asin(F32 x);
        F32 approx_acos(F32 x) { return sub(SK_ScalarPI/2, approx_asin(x)); }
        F32 approx_atan(F32 x);
        F32 approx_atan2(F32 y, F32 x);

        F32 lerp(F32  lo, F32  hi, F32  t);
        F32 lerp(F32a lo, F32a hi, F32a t) { return lerp(_(lo), _(hi), _(t)); }

        F32 clamp(F32  x, F32  lo, F32  hi) { return max(lo, min(x, hi)); }
        F32 clamp(F32a x, F32a lo, F32a hi) { return clamp(_(x), _(lo), _(hi)); }
        F32 clamp01(F32 x) { return clamp(x, 0.0f, 1.0f); }

        F32    abs(F32 x) { return bit_cast(bit_and(bit_cast(x), 0x7fff'ffff)); }
        F32  fract(F32 x) { return sub(x, floor(x)); }
        F32   ceil(F32);
        F32  floor(F32);
        I32 is_NaN   (F32 x) { return neq(x,x); }
        I32 is_finite(F32 x) { return lt(bit_and(bit_cast(x), 0x7f80'0000), 0x7f80'0000); }

        I32 trunc(F32 x);
        I32 round(F32 x);  // Round to int using current rounding mode (as if lrintf()).
        I32 bit_cast(F32 x) { return {x.builder, x.id}; }

        I32   to_half(F32 x);
        F32 from_half(I32 x);

        F32 norm(F32 x, F32 y) {
            return sqrt(add(mul(x,x),
                            mul(y,y)));
        }
        F32 norm(F32a x, F32a y) { return norm(_(x), _(y)); }

        I32  eq(F32, F32);  I32  eq(F32a x, F32a y) { return  eq(_(x), _(y)); }
        I32 neq(F32, F32);  I32 neq(F32a x, F32a y) { return neq(_(x), _(y)); }
        I32 lt (F32, F32);  I32 lt (F32a x, F32a y) { return lt (_(x), _(y)); }
        I32 lte(F32, F32);  I32 lte(F32a x, F32a y) { return lte(_(x), _(y)); }
        I32 gt (F32, F32);  I32 gt (F32a x, F32a y) { return gt (_(x), _(y)); }
        I32 gte(F32, F32);  I32 gte(F32a x, F32a y) { return gte(_(x), _(y)); }

        // int math, comparisons, etc.
        I32 add(I32, I32);  I32 add(I32a x, I32a y) { return add(_(x), _(y)); }
        I32 sub(I32, I32);  I32 sub(I32a x, I32a y) { return sub(_(x), _(y)); }
        I32 mul(I32, I32);  I32 mul(I32a x, I32a y) { return mul(_(x), _(y)); }

        I32 shl(I32 x, int bits);
        I32 shr(I32 x, int bits);
        I32 sra(I32 x, int bits);

        I32 eq (I32 x, I32 y);  I32  eq(I32a x, I32a y) { return  eq(_(x), _(y)); }
        I32 neq(I32 x, I32 y);  I32 neq(I32a x, I32a y) { return neq(_(x), _(y)); }
        I32 lt (I32 x, I32 y);  I32 lt (I32a x, I32a y) { return lt (_(x), _(y)); }
        I32 lte(I32 x, I32 y);  I32 lte(I32a x, I32a y) { return lte(_(x), _(y)); }
        I32 gt (I32 x, I32 y);  I32 gt (I32a x, I32a y) { return gt (_(x), _(y)); }
        I32 gte(I32 x, I32 y);  I32 gte(I32a x, I32a y) { return gte(_(x), _(y)); }

        F32 to_F32(I32 x);
        F32 bit_cast(I32 x) { return {x.builder, x.id}; }

        // Bitwise operations.
        I32 bit_and  (I32, I32);  I32 bit_and  (I32a x, I32a y) { return bit_and  (_(x), _(y)); }
        I32 bit_or   (I32, I32);  I32 bit_or   (I32a x, I32a y) { return bit_or   (_(x), _(y)); }
        I32 bit_xor  (I32, I32);  I32 bit_xor  (I32a x, I32a y) { return bit_xor  (_(x), _(y)); }
        I32 bit_clear(I32, I32);  I32 bit_clear(I32a x, I32a y) { return bit_clear(_(x), _(y)); }

        I32 min(I32 x, I32 y) { return select(lte(x,y), x, y); }
        I32 max(I32 x, I32 y) { return select(gte(x,y), x, y); }

        I32 min(I32a x, I32a y) { return min(_(x), _(y)); }
        I32 max(I32a x, I32a y) { return max(_(x), _(y)); }

        I32 select(I32 cond, I32 t, I32 f);  // cond ? t : f
        F32 select(I32 cond, F32 t, F32 f) {
            return bit_cast(select(cond, bit_cast(t)
                                       , bit_cast(f)));
        }
        Q14 select(Q14 cond, Q14 t, Q14 f);

        I32 select(I32a cond, I32a t, I32a f) { return select(_(cond), _(t), _(f)); }
        F32 select(I32a cond, F32a t, F32a f) { return select(_(cond), _(t), _(f)); }
        Q14 select(Q14a cond, Q14a t, Q14a f) { return select(_(cond), _(t), _(f)); }

        I32 extract(I32 x, int bits, I32 z);   // (x>>bits) & z
        I32 pack   (I32 x, I32 y, int bits);   // x | (y << bits), assuming (x & (y << bits)) == 0

        I32 extract(I32a x, int bits, I32a z) { return extract(_(x), bits, _(z)); }
        I32 pack   (I32a x, I32a y, int bits) { return pack   (_(x), _(y), bits); }

        Q14 add(Q14, Q14);  Q14 add(Q14a x, Q14a y) { return add(_(x), _(y)); }
        Q14 sub(Q14, Q14);  Q14 sub(Q14a x, Q14a y) { return sub(_(x), _(y)); }
        Q14 mul(Q14, Q14);  Q14 mul(Q14a x, Q14a y) { return mul(_(x), _(y)); }

        Q14 min(Q14, Q14);  Q14 min(Q14a x, Q14a y) { return min(_(x), _(y)); }
        Q14 max(Q14, Q14);  Q14 max(Q14a x, Q14a y) { return max(_(x), _(y)); }

        Q14 shl(Q14, int bits);
        Q14 shr(Q14, int bits);
        Q14 sra(Q14, int bits);

        Q14 eq (Q14, Q14);  Q14  eq(Q14a x, Q14a y) { return  eq(_(x), _(y)); }
        Q14 neq(Q14, Q14);  Q14 neq(Q14a x, Q14a y) { return neq(_(x), _(y)); }
        Q14 lt (Q14, Q14);  Q14 lt (Q14a x, Q14a y) { return lt (_(x), _(y)); }
        Q14 lte(Q14, Q14);  Q14 lte(Q14a x, Q14a y) { return lte(_(x), _(y)); }
        Q14 gt (Q14, Q14);  Q14 gt (Q14a x, Q14a y) { return gt (_(x), _(y)); }
        Q14 gte(Q14, Q14);  Q14 gte(Q14a x, Q14a y) { return gte(_(x), _(y)); }

        Q14 bit_and  (Q14, Q14);  Q14 bit_and  (Q14a x, Q14a y) { return bit_and  (_(x), _(y)); }
        Q14 bit_or   (Q14, Q14);  Q14 bit_or   (Q14a x, Q14a y) { return bit_or   (_(x), _(y)); }
        Q14 bit_xor  (Q14, Q14);  Q14 bit_xor  (Q14a x, Q14a y) { return bit_xor  (_(x), _(y)); }
        Q14 bit_clear(Q14, Q14);  Q14 bit_clear(Q14a x, Q14a y) { return bit_clear(_(x), _(y)); }

        Q14 unsigned_avg(Q14  x, Q14  y);  // (x+y+1)>>1
        Q14 unsigned_avg(Q14a x, Q14a y) { return unsigned_avg(_(x), _(y)); }

        Q14 to_Q14(F32); F32 to_F32(Q14);   // Converts values, e.g. 0x4000 <-> 1.0f
        Q14 to_Q14(I32); I32 to_I32(Q14);   // Preserves bits, e.g. 0x4000 <-> 0x00004000

        // Common idioms used in several places, worth centralizing for consistency.
        F32 from_unorm(int bits, I32);   // E.g. from_unorm(8, x) -> x * (1/255.0f)
        I32   to_unorm(int bits, F32);   // E.g.   to_unorm(8, x) -> round(x * 255)

        Color   load(PixelFormat, Arg ptr);
        bool   store(PixelFormat, Arg ptr, Color);
        Color gather(PixelFormat, Arg ptr, int offset, I32 index);
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
        void dot (SkWStream* = nullptr) const;

        uint64_t hash() const;

        Val push(Instruction);
    private:
        Val push(Op op, Val x, Val y=NA, Val z=NA, int immy=0, int immz=0) {
            return this->push(Instruction{op, x,y,z, immy,immz});
        }

        I32 _(I32a x) {
            if (x.id != NA) {
                SkASSERT(x.builder == this);
                return {this, x.id};
            }
            return splat(x.imm);
        }

        F32 _(F32a x) {
            if (x.id != NA) {
                SkASSERT(x.builder == this);
                return {this, x.id};
            }
            return splat(x.imm);
        }

        Q14 _(Q14a x) {
            if (x.id != NA) {
                SkASSERT(x.builder == this);
                return {this, x.id};
            }
            return splat_q14(x.imm);
        }

        bool allImm() const;

        template <typename T, typename... Rest>
        bool allImm(Val, T* imm, Rest...) const;

        template <typename T>
        bool isImm(Val id, T want) const {
            T imm = 0;
            return this->allImm(id, &imm) && imm == want;
        }

        SkTHashMap<Instruction, Val, InstructionHash> fIndex;
        std::vector<Instruction>                      fProgram;
        std::vector<int>                              fStrides;
    };

    template <typename... Fs>
    void dump_instructions(const std::vector<Instruction>& instructions,
                           SkWStream* o = nullptr,
                           Fs... fs);

    // Optimization passes and data structures normally used by Builder::optimize(),
    // extracted here so they can be unit tested.
    std::vector<Instruction>          eliminate_dead_code(std::vector<Instruction>);
    std::vector<Instruction>          schedule           (std::vector<Instruction>);
    std::vector<OptimizedInstruction> finalize           (std::vector<Instruction>);

    class Usage {
    public:
        Usage(const std::vector<Instruction>&);

        // Return a sorted span of Vals which use result of Instruction id.
        SkSpan<const Val> operator[](Val id) const;

    private:
        std::vector<int> fIndex;
        std::vector<Val> fTable;
    };

    using Reg = int;

    // d = op(x, y/imm, z/imm)
    struct InterpreterInstruction {
        Op  op;
        Reg d,x;
        union { Reg y; int immy; };
        union { Reg z; int immz; };
    };

    class Program {
    public:
        Program(const std::vector<OptimizedInstruction>& instructions,
                const std::vector<int>& strides,
                const char* debug_name);

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
        void dropJIT();       // If hasJIT(), drop it, forcing interpreter fallback.

        void dump(SkWStream* = nullptr) const;

    private:
        void setupInterpreter(const std::vector<OptimizedInstruction>&);
        void setupJIT        (const std::vector<OptimizedInstruction>&, const char* debug_name);
        void setupLLVM       (const std::vector<OptimizedInstruction>&, const char* debug_name);

        bool jit(const std::vector<OptimizedInstruction>&,
                 int* stack_hint, uint32_t* registers_used,
                 Assembler*) const;

        void waitForLLVM() const;

        struct Impl;
        std::unique_ptr<Impl> fImpl;
    };

    // TODO: control flow
    // TODO: 64-bit values?

    static inline Q14 operator+(Q14   x, Q14a y) { return x->add(x,y); }
    static inline Q14 operator+(float x, Q14  y) { return y->add(x,y); }

    static inline Q14 operator-(Q14   x, Q14a y) { return x->sub(x,y); }
    static inline Q14 operator-(float x, Q14  y) { return y->sub(x,y); }

    static inline Q14 operator*(Q14   x, Q14a y) { return x->mul(x,y); }
    static inline Q14 operator*(float x, Q14  y) { return y->mul(x,y); }

    static inline Q14 min(Q14   x, Q14a y) { return x->min(x,y); }
    static inline Q14 min(float x, Q14  y) { return y->min(x,y); }

    static inline Q14 max(Q14   x, Q14a y) { return x->max(x,y); }
    static inline Q14 max(float x, Q14  y) { return y->max(x,y); }

    static inline Q14 unsigned_avg(Q14   x, Q14a y) { return x->unsigned_avg(x,y); }
    static inline Q14 unsigned_avg(float x, Q14  y) { return y->unsigned_avg(x,y); }

    static inline Q14 operator==(Q14   x, Q14   y) { return x->eq(x,y); }
    static inline Q14 operator==(Q14   x, float y) { return x->eq(x,y); }
    static inline Q14 operator==(float x, Q14   y) { return y->eq(x,y); }

    static inline Q14 operator!=(Q14   x, Q14   y) { return x->neq(x,y); }
    static inline Q14 operator!=(Q14   x, float y) { return x->neq(x,y); }
    static inline Q14 operator!=(float x, Q14   y) { return y->neq(x,y); }

    static inline Q14 operator< (Q14   x, Q14a y) { return x->lt(x,y); }
    static inline Q14 operator< (float x, Q14  y) { return y->lt(x,y); }

    static inline Q14 operator<=(Q14   x, Q14a y) { return x->lte(x,y); }
    static inline Q14 operator<=(float x, Q14  y) { return y->lte(x,y); }

    static inline Q14 operator> (Q14   x, Q14a y) { return x->gt(x,y); }
    static inline Q14 operator> (float x, Q14  y) { return y->gt(x,y); }

    static inline Q14 operator>=(Q14   x, Q14a y) { return x->gte(x,y); }
    static inline Q14 operator>=(float x, Q14  y) { return y->gte(x,y); }


    static inline I32 operator+(I32 x, I32a y) { return x->add(x,y); }
    static inline I32 operator+(int x, I32  y) { return y->add(x,y); }

    static inline I32 operator-(I32 x, I32a y) { return x->sub(x,y); }
    static inline I32 operator-(int x, I32  y) { return y->sub(x,y); }

    static inline I32 operator*(I32 x, I32a y) { return x->mul(x,y); }
    static inline I32 operator*(int x, I32  y) { return y->mul(x,y); }

    static inline I32 min(I32 x, I32a y) { return x->min(x,y); }
    static inline I32 min(int x, I32  y) { return y->min(x,y); }

    static inline I32 max(I32 x, I32a y) { return x->max(x,y); }
    static inline I32 max(int x, I32  y) { return y->max(x,y); }

    static inline I32 operator==(I32 x, I32 y) { return x->eq(x,y); }
    static inline I32 operator==(I32 x, int y) { return x->eq(x,y); }
    static inline I32 operator==(int x, I32 y) { return y->eq(x,y); }

    static inline I32 operator!=(I32 x, I32 y) { return x->neq(x,y); }
    static inline I32 operator!=(I32 x, int y) { return x->neq(x,y); }
    static inline I32 operator!=(int x, I32 y) { return y->neq(x,y); }

    static inline I32 operator< (I32 x, I32a y) { return x->lt(x,y); }
    static inline I32 operator< (int x, I32  y) { return y->lt(x,y); }

    static inline I32 operator<=(I32 x, I32a y) { return x->lte(x,y); }
    static inline I32 operator<=(int x, I32  y) { return y->lte(x,y); }

    static inline I32 operator> (I32 x, I32a y) { return x->gt(x,y); }
    static inline I32 operator> (int x, I32  y) { return y->gt(x,y); }

    static inline I32 operator>=(I32 x, I32a y) { return x->gte(x,y); }
    static inline I32 operator>=(int x, I32  y) { return y->gte(x,y); }


    static inline F32 operator+(F32   x, F32a y) { return x->add(x,y); }
    static inline F32 operator+(float x, F32  y) { return y->add(x,y); }

    static inline F32 operator-(F32   x, F32a y) { return x->sub(x,y); }
    static inline F32 operator-(float x, F32  y) { return y->sub(x,y); }

    static inline F32 operator*(F32   x, F32a y) { return x->mul(x,y); }
    static inline F32 operator*(float x, F32  y) { return y->mul(x,y); }

    static inline F32 operator/(F32   x, F32  y) { return x->div(x,y); }
    static inline F32 operator/(float x, F32  y) { return y->div(x,y); }

    static inline F32 min(F32   x, F32a y) { return x->min(x,y); }
    static inline F32 min(float x, F32  y) { return y->min(x,y); }

    static inline F32 max(F32   x, F32a y) { return x->max(x,y); }
    static inline F32 max(float x, F32  y) { return y->max(x,y); }

    static inline I32 operator==(F32   x, F32   y) { return x->eq(x,y); }
    static inline I32 operator==(F32   x, float y) { return x->eq(x,y); }
    static inline I32 operator==(float x, F32   y) { return y->eq(x,y); }

    static inline I32 operator!=(F32   x, F32   y) { return x->neq(x,y); }
    static inline I32 operator!=(F32   x, float y) { return x->neq(x,y); }
    static inline I32 operator!=(float x, F32   y) { return y->neq(x,y); }

    static inline I32 operator< (F32   x, F32a y) { return x->lt(x,y); }
    static inline I32 operator< (float x, F32  y) { return y->lt(x,y); }

    static inline I32 operator<=(F32   x, F32a y) { return x->lte(x,y); }
    static inline I32 operator<=(float x, F32  y) { return y->lte(x,y); }

    static inline I32 operator> (F32   x, F32a y) { return x->gt(x,y); }
    static inline I32 operator> (float x, F32  y) { return y->gt(x,y); }

    static inline I32 operator>=(F32   x, F32a y) { return x->gte(x,y); }
    static inline I32 operator>=(float x, F32  y) { return y->gte(x,y); }

    static inline Q14& operator+=(Q14& x, Q14a y) { return (x = x + y); }
    static inline Q14& operator-=(Q14& x, Q14a y) { return (x = x - y); }
    static inline Q14& operator*=(Q14& x, Q14a y) { return (x = x * y); }

    static inline I32& operator+=(I32& x, I32a y) { return (x = x + y); }
    static inline I32& operator-=(I32& x, I32a y) { return (x = x - y); }
    static inline I32& operator*=(I32& x, I32a y) { return (x = x * y); }

    static inline F32& operator+=(F32& x, F32a y) { return (x = x + y); }
    static inline F32& operator-=(F32& x, F32a y) { return (x = x - y); }
    static inline F32& operator*=(F32& x, F32a y) { return (x = x * y); }

    static inline void assert_true(I32 cond, I32 debug) { cond->assert_true(cond,debug); }
    static inline void assert_true(I32 cond, F32 debug) { cond->assert_true(cond,debug); }
    static inline void assert_true(I32 cond)            { cond->assert_true(cond); }

    static inline void store8  (Arg ptr, I32 val)                { val->store8  (ptr, val); }
    static inline void store16 (Arg ptr, I32 val)                { val->store16 (ptr, val); }
    static inline void store32 (Arg ptr, I32 val)                { val->store32 (ptr, val); }
    static inline void storeF  (Arg ptr, F32 val)                { val->storeF  (ptr, val); }
    static inline void store64 (Arg ptr, I32 lo, I32 hi)         { lo ->store64 (ptr, lo,hi); }
    static inline void store128(Arg ptr, I32 lo, I32 hi, int ix) { lo ->store128(ptr, lo,hi, ix); }

    static inline I32 gather8 (Arg ptr, int off, I32 ix) { return ix->gather8 (ptr, off, ix); }
    static inline I32 gather16(Arg ptr, int off, I32 ix) { return ix->gather16(ptr, off, ix); }
    static inline I32 gather32(Arg ptr, int off, I32 ix) { return ix->gather32(ptr, off, ix); }
    static inline F32 gatherF (Arg ptr, int off, I32 ix) { return ix->gatherF (ptr, off, ix); }

    static inline I32 gather8 (Uniform u, I32 ix) { return ix->gather8 (u, ix); }
    static inline I32 gather16(Uniform u, I32 ix) { return ix->gather16(u, ix); }
    static inline I32 gather32(Uniform u, I32 ix) { return ix->gather32(u, ix); }
    static inline F32 gatherF (Uniform u, I32 ix) { return ix->gatherF (u, ix); }

    static inline F32        sqrt(F32 x) { return x->       sqrt(x); }
    static inline F32 approx_log2(F32 x) { return x->approx_log2(x); }
    static inline F32 approx_pow2(F32 x) { return x->approx_pow2(x); }
    static inline F32 approx_log (F32 x) { return x->approx_log (x); }
    static inline F32 approx_exp (F32 x) { return x->approx_exp (x); }

    static inline F32 approx_powf(F32   base, F32a exp) { return base->approx_powf(base, exp); }
    static inline F32 approx_powf(float base, F32  exp) { return  exp->approx_powf(base, exp); }

    static inline F32 approx_sin(F32 radians) { return radians->approx_sin(radians); }
    static inline F32 approx_cos(F32 radians) { return radians->approx_cos(radians); }
    static inline F32 approx_tan(F32 radians) { return radians->approx_tan(radians); }

    static inline F32 approx_asin(F32 x) { return x->approx_asin(x); }
    static inline F32 approx_acos(F32 x) { return x->approx_acos(x); }
    static inline F32 approx_atan(F32 x) { return x->approx_atan(x); }
    static inline F32 approx_atan2(F32 y, F32 x) { return x->approx_atan2(y, x); }

    static inline F32   clamp01(F32 x) { return x->  clamp01(x); }
    static inline F32       abs(F32 x) { return x->      abs(x); }
    static inline F32      ceil(F32 x) { return x->     ceil(x); }
    static inline F32     fract(F32 x) { return x->    fract(x); }
    static inline F32     floor(F32 x) { return x->    floor(x); }
    static inline I32    is_NaN(F32 x) { return x->   is_NaN(x); }
    static inline I32 is_finite(F32 x) { return x->is_finite(x); }

    static inline I32     trunc(F32 x) { return x->    trunc(x); }
    static inline I32     round(F32 x) { return x->    round(x); }
    static inline I32  bit_cast(F32 x) { return x-> bit_cast(x); }
    static inline F32  bit_cast(I32 x) { return x-> bit_cast(x); }
    static inline F32    to_F32(I32 x) { return x->   to_F32(x); }
    static inline I32   to_half(F32 x) { return x->  to_half(x); }
    static inline F32 from_half(I32 x) { return x->from_half(x); }

    static inline F32 to_F32(Q14 x) { return x->to_F32(x); }
    static inline I32 to_I32(Q14 x) { return x->to_I32(x); }
    static inline Q14 to_Q14(F32 x) { return x->to_Q14(x); }
    static inline Q14 to_Q14(I32 x) { return x->to_Q14(x); }

    static inline F32 lerp(F32   lo, F32a  hi, F32a t) { return lo->lerp(lo,hi,t); }
    static inline F32 lerp(float lo, F32   hi, F32a t) { return hi->lerp(lo,hi,t); }
    static inline F32 lerp(float lo, float hi, F32  t) { return  t->lerp(lo,hi,t); }

    static inline F32 clamp(F32   x, F32a  lo, F32a hi) { return  x->clamp(x,lo,hi); }
    static inline F32 clamp(float x, F32   lo, F32a hi) { return lo->clamp(x,lo,hi); }
    static inline F32 clamp(float x, float lo, F32  hi) { return hi->clamp(x,lo,hi); }

    static inline F32 norm(F32   x, F32a y) { return x->norm(x,y); }
    static inline F32 norm(float x, F32  y) { return y->norm(x,y); }

    static inline I32 operator<<(I32 x, int bits) { return x->shl(x, bits); }
    static inline I32        shl(I32 x, int bits) { return x->shl(x, bits); }
    static inline I32        shr(I32 x, int bits) { return x->shr(x, bits); }
    static inline I32        sra(I32 x, int bits) { return x->sra(x, bits); }

    static inline Q14 operator<<(Q14 x, int bits) { return x->shl(x, bits); }
    static inline Q14        shl(Q14 x, int bits) { return x->shl(x, bits); }
    static inline Q14        shr(Q14 x, int bits) { return x->shr(x, bits); }
    static inline Q14        sra(Q14 x, int bits) { return x->sra(x, bits); }
    static inline Q14 operator>>(Q14 x, int bits) { return x->sra(x, bits); }

    static inline I32 operator&(I32 x, I32a y) { return x->bit_and(x,y); }
    static inline I32 operator&(int x, I32  y) { return y->bit_and(x,y); }

    static inline I32 operator|(I32 x, I32a y) { return x->bit_or (x,y); }
    static inline I32 operator|(int x, I32  y) { return y->bit_or (x,y); }

    static inline I32 operator^(I32 x, I32a y) { return x->bit_xor(x,y); }
    static inline I32 operator^(int x, I32  y) { return y->bit_xor(x,y); }

    static inline I32& operator&=(I32& x, I32a y) { return (x = x & y); }
    static inline I32& operator|=(I32& x, I32a y) { return (x = x | y); }
    static inline I32& operator^=(I32& x, I32a y) { return (x = x ^ y); }

    static inline Q14 operator&(Q14 x, Q14a y) { return x->bit_and(x,y); }
    static inline Q14 operator&(int x, Q14  y) { return y->bit_and(x,y); }

    static inline Q14 operator|(Q14 x, Q14a y) { return x->bit_or(x,y); }
    static inline Q14 operator|(int x, Q14  y) { return y->bit_or(x,y); }

    static inline Q14 operator^(Q14 x, Q14a y) { return x->bit_xor(x,y); }
    static inline Q14 operator^(int x, Q14  y) { return y->bit_xor(x,y); }

    static inline Q14& operator&=(Q14& x, Q14a y) { return (x = x & y); }
    static inline Q14& operator|=(Q14& x, Q14a y) { return (x = x | y); }
    static inline Q14& operator^=(Q14& x, Q14a y) { return (x = x ^ y); }

    static inline I32 bit_clear(I32 x, I32a y) { return x->bit_clear(x,y); }
    static inline I32 bit_clear(int x, I32  y) { return y->bit_clear(x,y); }

    static inline I32 select(I32 cond, I32a t, I32a f) { return cond->select(cond,t,f); }
    static inline F32 select(I32 cond, F32a t, F32a f) { return cond->select(cond,t,f); }
    static inline Q14 select(Q14 cond, Q14a t, Q14a f) { return cond->select(cond,t,f); }

    static inline I32 extract(I32 x, int bits, I32a z) { return x->extract(x,bits,z); }
    static inline I32 extract(int x, int bits, I32  z) { return z->extract(x,bits,z); }
    static inline I32 pack   (I32 x, I32a y, int bits) { return x->pack   (x,y,bits); }
    static inline I32 pack   (int x, I32  y, int bits) { return y->pack   (x,y,bits); }

    static inline I32 operator~(I32 x) { return ~0 ^ x; }
    static inline Q14 operator~(Q14 x) { return ~0 ^ x; }
    static inline I32 operator-(I32 x) { return  0 - x; }
    static inline Q14 operator-(Q14 x) { return  0 - x; }
    static inline F32 operator-(F32 x) { return 0.0f - x; }

    static inline F32 from_unorm(int bits, I32 x) { return x->from_unorm(bits,x); }
    static inline I32   to_unorm(int bits, F32 x) { return x->  to_unorm(bits,x); }

    static inline bool store(PixelFormat f, Arg p, Color c) { return c->store(f,p,c); }
    static inline Color gather(PixelFormat f, Arg p, int off, I32 ix) {
        return ix->gather(f,p,off,ix);
    }
    static inline Color gather(PixelFormat f, Uniform u, I32 ix) {
        return ix->gather(f,u,ix);
    }

    static inline void   premul(F32* r, F32* g, F32* b, F32 a) { a->  premul(r,g,b,a); }
    static inline void unpremul(F32* r, F32* g, F32* b, F32 a) { a->unpremul(r,g,b,a); }

    static inline Color   premul(Color c) { return c->  premul(c); }
    static inline Color unpremul(Color c) { return c->unpremul(c); }

    static inline Color lerp(Color lo, Color hi, F32 t) { return t->lerp(lo,hi,t); }

    static inline Color blend(SkBlendMode m, Color s, Color d) { return s->blend(m,s,d); }

    static inline Color clamp01(Color c) { return c->clamp01(c); }

    static inline HSLA  to_hsla(Color c) { return c->to_hsla(c); }
    static inline Color to_rgba(HSLA  c) { return c->to_rgba(c); }

    // Evaluate polynomials: ax^n + bx^(n-1) + ... for n >= 1
    template <typename... Rest>
    static inline F32 poly(F32 x, F32a a, F32a b, Rest... rest) {
        if constexpr (sizeof...(rest) == 0) {
            return x*a+b;
        } else {
            return poly(x, x*a+b, rest...);
        }
    }
}  // namespace skvm

#endif//SkVM_DEFINED
