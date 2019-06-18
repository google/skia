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
#include <unordered_map>
#include <vector>

namespace Xbyak { class CodeGenerator; }

namespace skvm {

    class Assembler {
    public:
        Assembler();
        ~Assembler();

        void*  code() const;
        size_t size() const;

        void nop();
        void align(int mod);

        void vzeroupper();
        void ret();

        void byte(uint8_t);

        template <typename... Rest>
        void byte(uint8_t, Rest...);

    //private:
        std::unique_ptr<Xbyak::CodeGenerator> X;
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

    using ID = int;  // Could go 16-bit?

    class Program {
    public:
        struct Instruction {   // d = op(x, y.id/y.imm, z.id/z.imm)
            Op op;
            ID d,x;
            union { ID id; int imm; } y,z;
        };

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
        void eval(int n, void* args[], size_t strides[], int nargs) const;

        std::vector<Instruction> fInstructions;
        int                      fRegs;
        int                      fLoop;
    #if defined(SKVM_JIT)
        // TODO: what a mess, clean up
        mutable int                        fJITMask = 0;
        mutable std::unique_ptr<Assembler> fJIT;
    #endif
    };

    struct Arg { int ix; };
    struct I32 { ID id; };
    struct F32 { ID id; };

    class Builder {
    public:
        Program done();

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

        // Call after done() to make sure all analysis has been performed.
        void dump(SkWStream*) const;

    private:
        // We reserve the last ID as a sentinel meaning none, n/a, null, nil, etc.
        static const ID NA = ~0;

        struct Instruction {
            Op   op;         // v* = op(x,y,z,imm), where * == index of this Instruction.
            bool hoist;      // Can this instruction be hoisted outside our implicit loop?
            ID   life;       // ID of last instruction using this instruction's result.
            ID   x,y,z;      // Enough arguments for mad().
            int  immy,immz;  // Immediate bit patterns, shift counts, argument indexes.

            bool operator==(const Instruction& o) const {
                return op    == o.op
                    && hoist == o.hoist
                    && life  == o.life
                    && x     == o.x
                    && y     == o.y
                    && z     == o.z
                    && immy  == o.immy
                    && immz  == o.immz;
            }
        };

        struct InstructionHash {
            template <typename T>
            static size_t Hash(T val) {
                return std::hash<T>{}(val);
            }
            size_t operator()(const Instruction& inst) const {
                return Hash((uint8_t)inst.op)
                     ^ Hash(inst.hoist)
                     ^ Hash(inst.life)
                     ^ Hash(inst.x)
                     ^ Hash(inst.y)
                     ^ Hash(inst.z)
                     ^ Hash(inst.immy)
                     ^ Hash(inst.immz);
            }
        };

        ID push(Op, ID x, ID y=NA, ID z=NA, int immy=0, int immz=0);
        bool isZero(ID) const;

        std::unordered_map<Instruction, ID, InstructionHash> fIndex;
        std::vector<Instruction>                             fProgram;
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
