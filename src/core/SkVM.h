/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_DEFINED
#define SkVM_DEFINED

#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"
#include "include/private/SkSpinlock.h"
#include <vector>

namespace skvm {

    class Assembler {
    public:
        Assembler();
        ~Assembler();

        // This program is size() total bytes long starting at data().
        const uint8_t* data() const;
        size_t size() const;

        // Order matters... GP64, Xmm, Ymm values match 4-bit register encoding for each.
        enum GP64 {
            rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
            r8 , r9,  r10, r11, r12, r13, r14, r15,
        };
        enum Xmm {
            xmm0, xmm1, xmm2 , xmm3 , xmm4 , xmm5 , xmm6 , xmm7 ,
            xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,
        };
        enum Ymm {
            ymm0, ymm1, ymm2 , ymm3 , ymm4 , ymm5 , ymm6 , ymm7 ,
            ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15,
        };

        void byte(const void*, int);
        void byte(uint8_t);
        template <typename... Rest> void byte(uint8_t, Rest...);

        void nop();
        void align(int mod);

        void vzeroupper();
        void ret();

        void add(GP64, int imm);
        void sub(GP64, int imm);

        // All dst = x op y.
        using DstEqXOpY = void(Ymm dst, Ymm x, Ymm y);
        DstEqXOpY vpaddd, vpsubd, vpmulld,
                  vpsubw, vpmullw,
                  vpand, vpor, vpxor,
                  vaddps, vsubps, vmulps, vdivps,
                  vfmadd132ps, vfmadd213ps, vfmadd231ps,
                  vpackusdw, vpackuswb;

        using DstEqXOpImm = void(Ymm dst, Ymm x, int imm);
        DstEqXOpImm vpslld, vpsrld, vpsrad,
                    vpsrlw,
                    vpermq;

        using DstEqOpX = void(Ymm dst, Ymm x);
        DstEqOpX vcvtdq2ps, vcvttps2dq;

        struct Label { size_t offset; };
        Label here();

        void jne(Label);

        void vbroadcastss(Ymm dst, Label);
        void vpshufb(Ymm dst, Ymm x, Label);

        void vmovups  (Ymm dst, GP64 src);
        void vpmovzxbd(Ymm dst, GP64 src);

        void vmovups(GP64 dst, Ymm src);
        void vmovq  (GP64 dst, Xmm src);

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
        void op(int prefix, int map, int opcode, Ymm dst, Ymm x, Label l);
        void op(int prefix, int map, int opcode, Ymm dst,        Label l) {
            this->op(prefix, map, opcode, dst, (Ymm)0, l);
        }

        // *ptr = ymm or ymm = *ptr, depending on opcode.
        void load_store(int prefix, int map, int opcode, Ymm ymm, GP64 ptr);

        std::vector<uint8_t> fCode;
    };

    enum class Op : uint8_t {
        store8, store32,
        load8,  load32,
        splat,
        add_f32, sub_f32, mul_f32, div_f32, mad_f32,
        add_i32, sub_i32, mul_i32,
        sub_i16x2, mul_i16x2, shr_i16x2,
        bit_and, bit_or, bit_xor,
        shl, shr, sra,
        extract,
        pack,
        bytes,
        to_f32, to_i32,
    };

    using Reg = int;

    struct ProgramInstruction {   // d = op(x, y, z/imm)
        Op  op;
        Reg d,x,y;
        union { Reg z; int imm; };
    };

    class Program {
    public:
        // Moved outside Program so it can be forward-declared.
        using Instruction = ProgramInstruction;

        Program(std::vector<Instruction>, int regs, int loop);
        Program() : Program({}, 0, 0) {}

        ~Program();
        Program(Program&&);
        Program& operator=(Program&&);
        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;

        void dump(SkWStream*) const;

        template <typename... T>
        void eval(int n, T*... arg) const {
            void* args[] = { (void*)arg..., nullptr };
            size_t strides[] = { sizeof(*arg)... };
            this->eval(n, args, strides, (int)sizeof...(arg));
        }

    private:
        struct JIT {
            ~JIT();

            void*  buf      = nullptr;  // Raw mmap'd buffer.
            size_t size     = 0;        // Size of buf in bytes.
            void (*entry)() = nullptr;  // Entry point, offset into buf.
            int    mask     = 0;        // Mask of N the JIT'd code can handle.
        };

        void eval(int n, void* args[], size_t strides[], int nargs) const;

        std::vector<Instruction> fInstructions;
        int                      fRegs;
        int                      fLoop;
        mutable SkSpinlock       fJITLock;
        mutable JIT              fJIT;
    };

    using Val = int;

    struct Arg { int ix; };
    struct I32 { Val id; };
    struct F32 { Val id; };

    class Builder {
    public:
        Program done() const;

        Arg arg(int);

        void store8 (Arg ptr, I32 val);
        void store32(Arg ptr, I32 val);

        I32 load8 (Arg ptr);
        I32 load32(Arg ptr);

        I32 splat(int      n);
        I32 splat(unsigned u) { return this->splat((int)u); }
        F32 splat(float    f);

        F32 add(F32 x, F32 y);
        F32 sub(F32 x, F32 y);
        F32 mul(F32 x, F32 y);
        F32 div(F32 x, F32 y);
        F32 mad(F32 x, F32 y, F32 z);

        I32 add(I32 x, I32 y);
        I32 sub(I32 x, I32 y);
        I32 mul(I32 x, I32 y);

        I32 sub_16x2(I32 x, I32 y);
        I32 mul_16x2(I32 x, I32 y);
        I32 shr_16x2(I32 x, int bits);

        I32 bit_and(I32 x, I32 y);
        I32 bit_or (I32 x, I32 y);
        I32 bit_xor(I32 x, I32 y);

        I32 shl(I32 x, int bits);
        I32 shr(I32 x, int bits);
        I32 sra(I32 x, int bits);

        I32 extract(I32 x, int bits, I32 z);   // (x >> bits) & z
        I32 pack   (I32 x, I32 y, int bits);   // x | (y << bits)

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
        //
        I32 bytes(I32 x, int control);

        F32 to_f32(I32 x);
        I32 to_i32(F32 x);

        void dump(SkWStream*) const;

    private:
        // We reserve the last Val ID as a sentinel meaning none, n/a, null, nil, etc.
        static const Val NA = ~0;

        struct Instruction {
            Op  op;         // v* = op(x,y,z,imm), where * == index of this Instruction.
            Val x,y,z;      // Enough arguments for mad().
            int imm;        // Immediate bit pattern, shift count, argument index, etc.

            bool operator==(const Instruction& o) const {
                return op  == o.op
                    && x   == o.x
                    && y   == o.y
                    && z   == o.z
                    && imm == o.imm;
            }
        };

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
                     ^ Hash(inst.imm);
            }
        };

        Val push(Op, Val x, Val y=NA, Val z=NA, int imm=0);
        bool isZero(Val) const;

        SkTHashMap<Instruction, Val, InstructionHash> fIndex;
        std::vector<Instruction>                      fProgram;
    };

    // TODO: comparison operations, if_then_else
    // TODO: learn how to do control flow
    // TODO: gather, load_uniform
    // TODO: 16- and 64-bit loads and stores
    // TODO: 16- and 64-bit values?
    // TODO: x86-64 SSE2 / SSE4.1 / AVX2 / AVX-512F JIT
    // TODO: ARMv8 JIT
    // TODO: ARMv8.2+FP16 JIT
    // TODO: ARMv7 NEON JIT?
    // TODO: lower to LLVM?
    // TODO: lower to WebASM?
}

#endif//SkVM_DEFINED
