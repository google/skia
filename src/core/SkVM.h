/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_DEFINED
#define SkVM_DEFINED

#include "include/core/SkTypes.h"
#include "include/core/SkStream.h"
#include <unordered_map>
#include <vector>

namespace skvm {

    enum class Op : uint8_t {
        store8, store32,
        load8,  load32,
        splat,
        add_f32, sub_f32, mul_f32, div_f32, mad_f32,
        add_i32, sub_i32, mul_i32,
        bit_and, bit_or, bit_xor,
        shl, shr, sra,
        mul_unorm8,
        pack,
        to_f32, to_i32,
    };

    using ID = int;  // Could go 16-bit?

    class Program {
    public:
        struct Instruction {   // d = op(x,y, z.id/z.imm)
            Op op;
            ID d,x,y;
            union { ID id; int imm; } z;
        };

        Program(std::vector<Instruction>, int regs);
        Program() : Program({}, 0) {}

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

        I32 splat(int   n);
        F32 splat(float f);

        F32 add(F32 x, F32 y);
        F32 sub(F32 x, F32 y);
        F32 mul(F32 x, F32 y);
        F32 div(F32 x, F32 y);
        F32 mad(F32 x, F32 y, F32 z);

        I32 add(I32 x, I32 y);
        I32 sub(I32 x, I32 y);
        I32 mul(I32 x, I32 y);

        I32 bit_and(I32 x, I32 y);
        I32 bit_or (I32 x, I32 y);
        I32 bit_xor(I32 x, I32 y);

        I32 shl(I32 x, int bits);
        I32 shr(I32 x, int bits);
        I32 sra(I32 x, int bits);

        I32 mul_unorm8(I32 x, I32 y);   // (x*y+255)/256, approximating (x*y+127)/255.

        // Interlace bits from x and y as if x | (y << bits),
        // assuming no bits from x and (y << bits) collide with each other, (x & (y << bits)) == 0.
        // (This allows implementation with SSE punpckl?? or NEON vzip.?? instructions.)
        I32 pack(I32 x, I32 y, int bits);

        F32 to_f32(I32 x);
        I32 to_i32(F32 x);

    private:
        struct Instruction {
            Op  op;      // v* = op(x,y,z,imm), where * == index of this Instruction.
            ID  life;    // ID of last instruction using this instruction's result.
            ID  x,y,z;   // Enough arguments for mad().
            int imm;     // Immediate bit pattern, shift count, or argument index.

            bool operator==(const Instruction& o) const {
                return op   == o.op
                    && life == o.life
                    && x    == o.x
                    && y    == o.y
                    && z    == o.z
                    && imm  == o.imm;
            }
        };

        struct InstructionHash {
            template <typename T>
            static size_t Hash(T val) {
                return std::hash<T>{}(val);
            }
            size_t operator()(const Instruction& inst) const {
                return Hash((uint8_t)inst.op)
                     ^ Hash(inst.life)
                     ^ Hash(inst.x)
                     ^ Hash(inst.y)
                     ^ Hash(inst.z)
                     ^ Hash(inst.imm);
            }
        };

        std::unordered_map<Instruction, ID, InstructionHash> fIndex;
        std::vector<Instruction>                             fProgram;

        ID push(Op, ID, ID, ID, int);
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
