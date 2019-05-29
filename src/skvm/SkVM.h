/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_DEFINED
#define SkVM_DEFINED

#include "include/core/SkTypes.h"
#include <vector>

namespace skvm {

    enum class Op : uint8_t {
        jump, jump_if, jump_if_not,
        store8, store32,
        label,
        load8,  load32,
        splat,
        add_f32, sub_f32, mul_f32, div_f32, mad_f32,
        add_i32, sub_i32, mul_i32, div_i32,
        bit_and, bit_or, bit_xor,
        shl, shr, sra,
        to_f32, to_i32,

        uniform,
        add1, sub1, mul1, div1, mod1,
        bit_and1, bit_or1, bit_xor1,
        shl1, shr1, sra1,
    };

    using ID = int;  // Could go 16-bit?

    struct ValOp {
        Op op;
        bool live;
        ID x,y,z;    // Enough arguments for mad_f32(), blend_f32().
        int imm;     // Immediate bit pattern or shift count.
    };

    void dump(const ValOp*, int n);

    struct Uniform { ID id; };
    struct Varying { ID id; };
    struct Label   { ID id; };

    class Builder {
    public:
        std::vector<ValOp> done();

        Uniform arg(int i);

        Label label();
        void jump       (Label target);
        void jump_if    (Label target, Uniform condition);
        void jump_if_not(Label target, Uniform condition);

        void store8 (Uniform ptr, Varying val);
        void store32(Uniform ptr, Varying val);

        Varying load8 (Uniform ptr);
        Varying load32(Uniform ptr);

        Varying splat(int   n);
        Varying splat(float f);

        Varying add_f32(Varying x, Varying y);
        Varying sub_f32(Varying x, Varying y);
        Varying mul_f32(Varying x, Varying y);
        Varying div_f32(Varying x, Varying y);
        Varying mad_f32(Varying x, Varying y, Varying z);

        Varying add_i32(Varying x, Varying y);
        Varying sub_i32(Varying x, Varying y);
        Varying mul_i32(Varying x, Varying y);
        Varying div_i32(Varying x, Varying y);

        Varying bit_and(Varying x, Varying y);
        Varying bit_or (Varying x, Varying y);
        Varying bit_xor(Varying x, Varying y);

        Varying shl(Varying x, int bits);
        Varying shr(Varying x, int bits);
        Varying sra(Varying x, int bits);

        Varying to_f32(Varying x);
        Varying to_i32(Varying x);

        Uniform uniform(int n);

        Uniform add    (Uniform x, Uniform y);
        Uniform sub    (Uniform x, Uniform y);
        Uniform mul    (Uniform x, Uniform y);
        Uniform div    (Uniform x, Uniform y);
        Uniform mod    (Uniform x, Uniform y);
        Uniform bit_and(Uniform x, Uniform y);
        Uniform bit_or (Uniform x, Uniform y);
        Uniform bit_xor(Uniform x, Uniform y);
        Uniform shl    (Uniform x, int bits);
        Uniform shr    (Uniform x, int bits);
        Uniform sra    (Uniform x, int bits);

    private:
        std::vector<ValOp> valops;

        ID push(Op, ID, ID, ID, int);
    };

    // TODO: real testing
    // TODO: simple dead code elimination
    //   - every op that returns void (jumps, stores, i.e. op < Op::Label) is live;
    //   - every argument to a live op is live;
    //   - anything else can be eliminated.
    // TODO: common subexpression elimination (literally, ValOp == ValOp)?
    // TODO: comparison operations on uniforms and varyings
    // TODO: learn how to actually do control flow
    // TODO: gatherN
    // TODO: load_uniformN / broadcastN
    // TODO: Varying if_then_else()
    // TODO: 16-bit loads and stores
    // TODO: 16-bit varyings?
    // TODO: 64-bit varyings?
    // TODO: simple interpreter
    // TODO: decent interpreter
    // TODO: lower to x86-64 SSE2 / SSE4.1 / AVX2 / AVX-512F
    // TODO: lower to ARMv8
    // TODO: lower to ARMv8.2+FP16
    // TODO: lower to ARMv7 NEON?
    // TODO: lower to LLVM?
    // TODO: lower to WebASM?
}

#endif//SkVM_DEFINED
