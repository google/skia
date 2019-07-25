/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_DEFINED
#define SkVM_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"
#include <vector>

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
            x24, x25, x26, x27, x28, x29, x30, xzr,
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

        // x86-64

        void align(int mod);

        void vzeroupper();
        void ret();

        void add(GP64, int imm);
        void sub(GP64, int imm);

        // All dst = x op y.
        using DstEqXOpY = void(Ymm dst, Ymm x, Ymm y);
        DstEqXOpY vpand, vpor, vpxor, vpandn,
                  vpaddd, vpsubd, vpmulld,
                          vpsubw, vpmullw,
                  vaddps, vsubps, vmulps, vdivps,
                  vfmadd132ps, vfmadd213ps, vfmadd231ps,
                  vpackusdw, vpackuswb;

        using DstEqXOpImm = void(Ymm dst, Ymm x, int imm);
        DstEqXOpImm vpslld, vpsrld, vpsrad,
                    vpsrlw,
                    vpermq;

        using DstEqOpX = void(Ymm dst, Ymm x);
        DstEqOpX vcvtdq2ps, vcvttps2dq;

        struct Label {
            int                                 offset = 0;
            enum { None, ARMDisp19, X86Disp32 } kind = None;
            std::vector<int>                    references;
        };

        Label here();
        void label(Label*);

        void jmp(Label*);
        void je (Label*);
        void jne(Label*);
        void jl (Label*);
        void cmp(GP64, int imm);

        void vbroadcastss(Ymm dst, Label*);
        void vpshufb(Ymm dst, Ymm x, Label*);

        void vmovups  (Ymm dst, GP64 ptr);   // dst = *ptr, 256-bit
        void vpmovzxbd(Ymm dst, GP64 ptr);   // dst = *ptr,  64-bit, each uint8_t expanded to int
        void vmovd    (Xmm dst, GP64 ptr);   // dst = *ptr,  32-bit

        void vmovups(GP64 ptr, Ymm src);     // *ptr = src, 256-bit
        void vmovq  (GP64 ptr, Xmm src);     // *ptr = src,  64-bit
        void vmovd  (GP64 ptr, Xmm src);     // *ptr = src,  32-bit

        void movzbl(GP64 dst, GP64 ptr);     // dst = *ptr, 8-bit, uint8_t expanded to int
        void movb  (GP64 ptr, GP64 src);     // *ptr = src, 8-bit

        void vmovd_direct(GP64 dst, Xmm src);  // dst = src, 32-bit
        void vmovd_direct(Xmm dst, GP64 src);  // dst = src, 32-bit

        void vpinsrb(Xmm dst, Xmm src, GP64 ptr, int imm);  // dst = src; dst[imm] = *ptr, 8-bit
        void vpextrb(GP64 ptr, Xmm src, int imm);           // *dst = src[imm]           , 8-bit

        // aarch64

        // d = op(n,m)
        using DOpNM = void(V d, V n, V m);
        DOpNM  and16b, orr16b, eor16b, bic16b,
               add4s,  sub4s,  mul4s,
                       sub8h,  mul8h,
              fadd4s, fsub4s, fmul4s, fdiv4s,
              tbl;

        // d += n*m
        void fmla4s(V d, V n, V m);

        // d = op(n,imm)
        using DOpNImm = void(V d, V n, int imm);
        DOpNImm sli4s,
                shl4s, sshr4s, ushr4s,
                               ushr8h;

        // d = op(n)
        using DOpN = void(V d, V n);
        DOpN scvtf4s,   // int -> float
             fcvtzs4s,  // truncate float -> int
             xtns2h,    // u32 -> u16
             xtnh2b,    // u16 -> u8
             uxtlb2h,   // u8 -> u16
             uxtlh2s;   // u16 -> u32

        // TODO: both these platforms support rounding float->int (vcvtps2dq, fcvtns.4s)... use?

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

        void ldrq(V dst, X src);  // 128-bit dst = *src
        void ldrs(V dst, X src);  //  32-bit dst = *src
        void ldrb(V dst, X src);  //   8-bit dst = *src

        void strq(V src, X dst);  // 128-bit *dst = src
        void strs(V src, X dst);  //  32-bit *dst = src
        void strb(V src, X dst);  //   8-bit *dst = src

    private:
        // dst = op(dst, imm)
        void op(int opcode, int opcode_ext, GP64 dst, int imm);


        // dst = op(x,y) or op(x)
        void op(int prefix, int map, int opcode, Ymm dst, Ymm x, Ymm y, bool W=false);
        void op(int prefix, int map, int opcode, Ymm dst, Ymm x,        bool W=false) {
            // Two arguments ops seem to pass them in dst and y, forcing x to 0 so VEX.vvvv == 1111.
            this->op(prefix, map, opcode, dst,(Ymm)0,x, W);
        }

        // dst = op(x,imm)
        void op(int prefix, int map, int opcode, int opcode_ext, Ymm dst, Ymm x, int imm);

        // dst = op(x,label) or op(label)
        void op(int prefix, int map, int opcode, Ymm dst, Ymm x, Label* l);
        void op(int prefix, int map, int opcode, Ymm dst,        Label* l) {
            this->op(prefix, map, opcode, dst, (Ymm)0, l);
        }

        // *ptr = ymm or ymm = *ptr, depending on opcode.
        void load_store(int prefix, int map, int opcode, Ymm ymm, GP64 ptr);

        // Opcode for 3-arguments ops is split between hi and lo:
        //    [11 bits hi] [5 bits m] [6 bits lo] [5 bits n] [5 bits d]
        void op(uint32_t hi, V m, uint32_t lo, V n, V d);

        // 2-argument ops, with or without an immediate.
        void op(uint32_t op22, int imm, V n, V d);
        void op(uint32_t op22, V n, V d) { this->op(op22,0,n,d); }
        void op(uint32_t op22, X x, V v) { this->op(op22,0,(V)x,v); }

        // Order matters... value is 4-bit encoding for condition code.
        enum class Condition { eq,ne,cs,cc,mi,pl,vs,vc,hi,ls,ge,lt,gt,le,al };
        void b(Condition, Label*);

        void jump(uint8_t condition, Label*);

        int disp19(Label*);
        int disp32(Label*);

        uint8_t* fCode;
        uint8_t* fCurr;
        size_t   fSize;
    };

    enum class Op : uint8_t {
          store8,   store16,   store32,
    // ↑ side effects / no side effects ↓

           load8,    load16,    load32,
         gather8,  gather16,  gather32,
    // ↑ always varying / uniforms, constants, Just Math ↓

        uniform8, uniform16, uniform32,
        splat,

        add_f32, add_i32, add_i16x2,
        sub_f32, sub_i32, sub_i16x2,
        mul_f32, mul_i32, mul_i16x2,
        div_f32,
        mad_f32,
                 shl_i32, shl_i16x2,
                 shr_i32, shr_i16x2,
                 sra_i32, sra_i16x2,

         to_i32,  to_f32,

         eq_f32,  eq_i32,  eq_i16x2,
        neq_f32, neq_i32, neq_i16x2,
         lt_f32,  lt_i32,  lt_i16x2,
        lte_f32, lte_i32, lte_i16x2,
         gt_f32,  gt_i32,  gt_i16x2,
        gte_f32, gte_i32, gte_i16x2,

        bit_and,
        bit_or,
        bit_xor,
        bit_clear,
        select,

        bytes, extract, pack,
    };

    using Val = int;
    // We reserve the last Val ID as a sentinel meaning none, n/a, null, nil, etc.
    static const Val NA = ~0;

    struct Arg { int ix; };
    struct I32 { Val id; };
    struct F32 { Val id; };

    class Program;

    class Builder {
    public:
        struct Instruction {
            Op  op;         // v* = op(x,y,z,imm), where * == index of this Instruction.
            Val x,y,z;      // Enough arguments for mad().
            int imm;        // Immediate bit pattern, shift count, argument index, etc.

            // Not populated until done() has been called.
            int  death;     // Index of last live instruction taking this input; live if != 0.
            bool hoist;     // Value independent of all loop variables?
        };

        Program done(const char* debug_name = nullptr);

        // Mostly for debugging, tests, etc.
        std::vector<Instruction> program() const { return fProgram; }


        // Declare an argument with given stride (use stride=0 for uniforms).
        Arg arg(int stride);

        // Convenience arg() wrapper for most common strides, sizeof(T) and 0.
        template <typename T>
        Arg arg() { return this->arg(sizeof(T)); }

        Arg uniform() { return this->arg(0); }

        // TODO: allow uniform (i.e. Arg) offsets to store* and load*?
        // TODO: sign extension (signed types) for <32-bit loads?
        // TODO: unsigned integer operations where relevant (just comparisons?)?

        // Store {8,16,32}-bit varying.
        void store8 (Arg ptr, I32 val);
        void store16(Arg ptr, I32 val);
        void store32(Arg ptr, I32 val);

        // Load u8,u16,i32 varying.
        I32 load8 (Arg ptr);
        I32 load16(Arg ptr);
        I32 load32(Arg ptr);

        // Gather u8,u16,i32 with varying element-count offset.
        I32 gather8 (Arg ptr, I32 offset);
        I32 gather16(Arg ptr, I32 offset);
        I32 gather32(Arg ptr, I32 offset);

        // Load u8,u16,i32 uniform with optional byte-count offset.
        I32 uniform8 (Arg ptr, int offset=0);
        I32 uniform16(Arg ptr, int offset=0);
        I32 uniform32(Arg ptr, int offset=0);

        // Load an immediate constant.
        I32 splat(int      n);
        I32 splat(unsigned u) { return this->splat((int)u); }
        F32 splat(float    f);

        // float math, comparisons, etc.
        F32 add(F32 x, F32 y);
        F32 sub(F32 x, F32 y);
        F32 mul(F32 x, F32 y);
        F32 div(F32 x, F32 y);
        F32 mad(F32 x, F32 y, F32 z);  //  x*y+z, often an FMA

        I32 eq (F32 x, F32 y);
        I32 neq(F32 x, F32 y);
        I32 lt (F32 x, F32 y);
        I32 lte(F32 x, F32 y);
        I32 gt (F32 x, F32 y);
        I32 gte(F32 x, F32 y);

        I32 to_i32(F32 x);
        I32 bit_cast(F32 x) { return {x.id}; }

        // int math, comparisons, etc.
        I32 add(I32 x, I32 y);
        I32 sub(I32 x, I32 y);
        I32 mul(I32 x, I32 y);

        I32 shl(I32 x, int bits);
        I32 shr(I32 x, int bits);
        I32 sra(I32 x, int bits);

        I32 eq (I32 x, I32 y);
        I32 neq(I32 x, I32 y);
        I32 lt (I32 x, I32 y);
        I32 lte(I32 x, I32 y);
        I32 gt (I32 x, I32 y);
        I32 gte(I32 x, I32 y);

        F32 to_f32(I32 x);
        F32 bit_cast(I32 x) { return {x.id}; }

        // Treat each 32-bit lane as a pair of 16-bit ints.
        I32 add_16x2(I32 x, I32 y);
        I32 sub_16x2(I32 x, I32 y);
        I32 mul_16x2(I32 x, I32 y);

        I32 shl_16x2(I32 x, int bits);
        I32 shr_16x2(I32 x, int bits);
        I32 sra_16x2(I32 x, int bits);

        I32     eq_16x2(I32 x, I32 y);
        I32    neq_16x2(I32 x, I32 y);
        I32     lt_16x2(I32 x, I32 y);
        I32    lte_16x2(I32 x, I32 y);
        I32     gt_16x2(I32 x, I32 y);
        I32    gte_16x2(I32 x, I32 y);

        // Bitwise operations.
        I32 bit_and  (I32 x, I32 y);
        I32 bit_or   (I32 x, I32 y);
        I32 bit_xor  (I32 x, I32 y);
        I32 bit_clear(I32 x, I32 y);   // x & ~y

        I32 select(I32 cond, I32 t, I32 f);  // cond ? t : f
        F32 select(I32 cond, F32 t, F32 f) {
            return this->bit_cast(this->select(cond, this->bit_cast(t)
                                                   , this->bit_cast(f)));
        }

        // More complex operations...

        // Shuffle the bytes in x according to each nibble of control, as if
        //
        //    uint8_t bytes[] = {
        //        0,
        //        ((uint32_t)x      ) & 0xff,
        //        ((uint32_t)x >>  8) & 0xff,
        //        ((uint32_t)x >> 16) & 0xff,
        //        ((uint32_t)x >> 24) & 0xff,
        //    };
        //    return (uint32_t)bytes[(control >>  0) & 0xf] <<  0
        //         | (uint32_t)bytes[(control >>  4) & 0xf] <<  8
        //         | (uint32_t)bytes[(control >>  8) & 0xf] << 16
        //         | (uint32_t)bytes[(control >> 12) & 0xf] << 24;
        //
        // So, e.g.,
        //    - bytes(x, 0x1111) splats the low byte of x to all four bytes
        //    - bytes(x, 0x4321) is x, an identity
        //    - bytes(x, 0x0000) is 0
        //    - bytes(x, 0x0404) transforms an RGBA pixel into an A0A0 bit pattern.
        I32 bytes  (I32 x, int control);

        I32 extract(I32 x, int bits, I32 y);   // (x >> bits) & y
        I32 pack   (I32 x, I32 y, int bits);   // x | (y << bits), assuming (x & (y << bits)) == 0

    private:
        struct InstructionHash {
            template <typename T>
            static size_t Hash(T val) {
                return std::hash<T>{}(val);
            }
            size_t operator()(const Instruction& inst) const {
                return Hash((uint8_t)inst.op)
                     ^ Hash(inst.x)
                     ^ Hash(inst.y)
                     ^ Hash(inst.z)
                     ^ Hash(inst.imm)
                     ^ Hash(inst.death)
                     ^ Hash(inst.hoist);
            }
        };

        Val push(Op, Val x, Val y=NA, Val z=NA, int imm=0);
        bool isZero(Val) const;

        SkTHashMap<Instruction, Val, InstructionHash> fIndex;
        std::vector<Instruction>                      fProgram;
        std::vector<int>                              fStrides;
    };

    using Reg = int;

    class Program {
    public:
        struct Instruction {   // d = op(x, y, z/imm)
            Op  op;
            Reg d,x,y;
            union { Reg z; int imm; };
        };

        Program(const std::vector<Builder::Instruction>& instructions,
                const std::vector<int>                 & strides,
                const char* debug_name);
        Program() : Program({}, {}, nullptr) {}

        ~Program();
        Program(Program&&);
        Program& operator=(Program&&);
        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;

        template <typename... T>
        void eval(int n, T*... arg) const {
            void* args[] = { (void*)arg..., nullptr };
            this->eval(n, args);
        }

        std::vector<Instruction> instructions() const { return fInstructions; }
        int nregs() const { return fRegs; }
        int loop() const { return fLoop; }

        // If this Program has been JITted, drop it, forcing interpreter fallback.
        void dropJIT();

    private:
        void eval(int n, void* args[]) const;

        void setupInterpreter(const std::vector<Builder::Instruction>&);
        void setupJIT        (const std::vector<Builder::Instruction>&, const char* debug_name);

        bool jit(const std::vector<Builder::Instruction>&,
                 bool hoist,
                 Assembler*) const;

        // Dump jit-*.dump files for perf inject.
        void dumpJIT(const char* debug_name, size_t size) const;

        std::vector<Instruction> fInstructions;
        int                      fRegs;
        int                      fLoop;
        std::vector<int>         fStrides;

        void*  fJITBuf  = nullptr;
        size_t fJITSize = 0;
    };

    // TODO: control flow
    // TODO: 64-bit values?
    // TODO: SSE2/SSE4.1, AVX-512F, ARMv8.2 JITs?
    // TODO: lower to LLVM or WebASM for comparison?
}

#endif//SkVM_DEFINED
