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

    enum class Op : int /* 8-bit is probably plenty. */ {
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
    };

    using ID = int;  // Could go 16-bit?

    struct ValOp {
        Op op;
        ID x,y,z;  // Enough arguments for mad_f32(), blend_f32().
    };

    void dump(const ValOp*, int n);

    struct Uniform { ID id; };
    struct Varying { ID id; };
    struct Label   { ID id; };

    class Builder {
    public:
        std::vector<ValOp> valops;

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

    private:
        ID push(Op, ID, ID, ID);
    };

    // TODO: real testing
    // TODO: simple dead code elimination
    //   - every op that returns void (jumps, stores, i.e. op < Op::Label) is live;
    //   - every argument to a live op is live;
    //   - anything else can be eliminated.
    // TODO: common subexpression elimination (literally, ValOp == ValOp)?
    // TODO: integer operations on uniforms
    // TODO: comparison operations on uniforms and varyings
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
