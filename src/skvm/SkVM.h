/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_DEFINED
#define SkVM_DEFINED

#include "include/core/SkTypes.h"
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

    struct ValOp {
        Op op;
        bool live;
        ID x,y,z;    // Enough arguments for mad_f32(), blend_f32().
        int imm;     // Immediate bit pattern or shift count.
    };

    inline bool operator==(const ValOp& a, const ValOp& b) {
        return a.op   == b.op
            && a.live == b.live
            && a.x    == b.x
            && a.y    == b.y
            && a.z    == b.z
            && a.imm  == b.imm;
    }
    struct ValOpHash {
        template <typename T>
        static size_t Hash(T val) {
            return std::hash<T>{}(val);
        }
        size_t operator()(const ValOp& valop) const {
            return Hash(valop.op)
                 ^ Hash(valop.live)
                 ^ Hash(valop.x)
                 ^ Hash(valop.y)
                 ^ Hash(valop.z)
                 ^ Hash(valop.imm);
        }
    };


    void dump(const ValOp*, int n);
    void eval(const ValOp*, int n, void** args);

    struct Arg { ID id; };
    struct Val { ID id; };

    class Builder {
    public:
        std::vector<ValOp> done();

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
        std::unordered_map<ValOp, ID, ValOpHash> index;
        std::vector<ValOp> valops;

        ID push(Op, ID, ID, ID, int);
    };

    // TODO: real testing
    // TODO: comparison operations on uniforms and varyings
    // TODO: learn how to actually do control flow
    // TODO: gatherN
    // TODO: load_uniformN / broadcastN
    // TODO: Varying if_then_else()
    // TODO: 16-bit loads and stores
    // TODO: 16-bit varyings?
    // TODO: 64-bit varyings?
    // TODO: decent interpreter
    // TODO: lower to x86-64 SSE2 / SSE4.1 / AVX2 / AVX-512F
    // TODO: lower to ARMv8
    // TODO: lower to ARMv8.2+FP16
    // TODO: lower to ARMv7 NEON?
    // TODO: lower to LLVM?
    // TODO: lower to WebASM?
}

#endif//SkVM_DEFINED
