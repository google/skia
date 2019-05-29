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
        add_i32, sub_i32, mul_i32, div_i32,
        bit_and, bit_or, bit_xor,
        shl, shr, sra,
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

        void dump(SkWStream*) const;

        template <typename... T>
        void eval(int n, T*... arg) const {
            void* args[] = { (void*)arg... };
            size_t strides[] = { sizeof(*arg)... };
            this->eval(n, args, strides, (int)sizeof...(arg));
        }

    private:
        void eval(int n, void* args[], size_t strides[], int nargs) const;

        std::vector<Instruction> fInstructions;
        int                      fRegs;
    };

    struct Arg { ID id; };
    struct Val { ID id; };

    class Builder {
    public:
        Program done();

        Arg arg(int);

        void store8 (Arg ptr, Val val);
        void store32(Arg ptr, Val val);

        Val load8 (Arg ptr);
        Val load32(Arg ptr);

        Val splat(int   n);
        Val splat(float f);

        Val add_f32(Val x, Val y);
        Val sub_f32(Val x, Val y);
        Val mul_f32(Val x, Val y);
        Val div_f32(Val x, Val y);
        Val mad_f32(Val x, Val y, Val z);

        Val add_i32(Val x, Val y);
        Val sub_i32(Val x, Val y);
        Val mul_i32(Val x, Val y);
        Val div_i32(Val x, Val y);

        Val bit_and(Val x, Val y);
        Val bit_or (Val x, Val y);
        Val bit_xor(Val x, Val y);

        Val shl(Val x, int bits);
        Val shr(Val x, int bits);
        Val sra(Val x, int bits);

        Val to_f32(Val x);
        Val to_i32(Val x);

    private:
        struct Instruction {
            Op  op;      // v* = op(x,y,z,imm), where * == index of this Instruction + 1.
            ID  life;    // ID of last instruction using this instruction's result.
            ID  x,y,z;   // Enough arguments for mad_f32(), blend().
            int imm;     // Immediate bit pattern or shift count.

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
