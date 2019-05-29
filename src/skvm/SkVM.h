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

#define OPS(M)                                             \
    M(jump) M(jump_if) M(jump_if_not)                      \
    M(store8) M(store32)                                   \
    M(label)                                               \
    M(load8)  M(load32)                                    \
    M(splat)                                               \
    M(add_f32) M(sub_f32) M(mul_f32) M(div_f32) M(mad_f32) \
    M(add_i32) M(sub_i32) M(mul_i32) M(div_i32)            \
    M(bit_and) M(bit_or) M(bit_xor)                        \
    M(shl) M(shr) M(sra)                                   \
    M(to_f32) M(to_i32)

    enum class Op : int {
    #define M(op) op,
        OPS(M)
    #undef M
    };

    using ID = int;

    struct ValOp {
        Op op;
        ID x,y,z;
    };

    void dump(const ValOp*, int n);

    struct Uniform { ID id; };
    struct Varying { ID id; };
    struct Label   { ID id; };

    class Builder {
    public:
        std::vector<ValOp> valops;

        Label label();
        void jump       (Label target);
        void jump_if    (Label target, Uniform condition);
        void jump_if_not(Label target, Uniform condition);

        void store8 (Uniform ptr, Varying val);
        void store32(Uniform ptr, Varying val);

        Varying load8 (Uniform ptr);
        Varying load32(Uniform ptr);

        Uniform arg(int i);

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
