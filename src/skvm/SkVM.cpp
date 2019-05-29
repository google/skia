/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkVx.h"
#include "src/skvm/SkVM.h"
#include <string.h>

namespace skvm {

    Program::Program(std::vector<Instruction> instructions)
        : fInstructions(std::move(instructions))
    {}

    Program Builder::done() {
        // Basic dead code elimination.
        for (auto i = fProgram.size(); i --> 0; ) {
            Instruction& inst = fProgram[i];

            // All side-effect-only instructions (stores) are live.
            if (inst.op <= Op::store32) {
                inst.live = true;
            }
            // The arguments of any live instructions are live.
            if (inst.live) {
                // Skip id == 0 (N/A) and id < 0 (Args).
                if (inst.x > 0) { fProgram[inst.x-1].live = true; }
                if (inst.y > 0) { fProgram[inst.y-1].live = true; }
                if (inst.z > 0) { fProgram[inst.z-1].live = true; }
            }
        }
        return { std::move(fProgram) };
    }

    // Most instructions produce a value and return it by ID, the value-producing
    // nstructions's own index in the program vector, plus 1, leaving 0 as a sentinel.
    // Args share the same ID space, working down from ~0.
    SK_WARN_UNUSED_RESULT
    ID Builder::push(Op op, ID x=0, ID y=0, ID z=0, int imm=0) {
        Instruction inst{op, /*live=*/false, x, y, z, imm};

        // Basic common subexpression elimination:
        // if we've already seen this exact Instruction, use it instead of creating a new one.
        auto lookup = fIndex.find(inst);
        if (lookup != fIndex.end()) {
            return lookup->second;
        }

        fProgram.push_back(inst);
        ID id = static_cast<ID>(fProgram.size());
        fIndex[inst] = id;
        return id;
    }

    Arg Builder::arg(int i) { return {~i}; }

    void Builder::store8 (Arg ptr, Val val) { (void)this->push(Op::store8 , ptr.id, val.id); }
    void Builder::store32(Arg ptr, Val val) { (void)this->push(Op::store32, ptr.id, val.id); }

    Val Builder::load8 (Arg ptr) { return {this->push(Op::load8 , ptr.id) }; }
    Val Builder::load32(Arg ptr) { return {this->push(Op::load32, ptr.id) }; }

    // The two splat() functions are just syntax sugar over splatting a 4-byte bit pattern.
    Val Builder::splat(int   n) { return {this->push(Op::splat, 0,0,0, n) }; }
    Val Builder::splat(float f) {
        int n;
        memcpy(&n, &f, 4);
        return this->splat(n);
    }

    Val Builder::add_f32(Val x, Val y) { return {this->push(Op::add_f32, x.id, y.id)}; }
    Val Builder::sub_f32(Val x, Val y) { return {this->push(Op::sub_f32, x.id, y.id)}; }
    Val Builder::mul_f32(Val x, Val y) { return {this->push(Op::mul_f32, x.id, y.id)}; }
    Val Builder::div_f32(Val x, Val y) { return {this->push(Op::div_f32, x.id, y.id)}; }
    Val Builder::mad_f32(Val x, Val y, Val z) {
        return {this->push(Op::mad_f32, x.id, y.id, z.id)};
    }

    Val Builder::add_i32(Val x, Val y) { return {this->push(Op::add_i32, x.id, y.id)}; }
    Val Builder::sub_i32(Val x, Val y) { return {this->push(Op::sub_i32, x.id, y.id)}; }
    Val Builder::mul_i32(Val x, Val y) { return {this->push(Op::mul_i32, x.id, y.id)}; }
    Val Builder::div_i32(Val x, Val y) { return {this->push(Op::div_i32, x.id, y.id)}; }

    Val Builder::bit_and(Val x, Val y) { return {this->push(Op::bit_and, x.id, y.id)}; }
    Val Builder::bit_or (Val x, Val y) { return {this->push(Op::bit_or , x.id, y.id)}; }
    Val Builder::bit_xor(Val x, Val y) { return {this->push(Op::bit_xor, x.id, y.id)}; }

    Val Builder::shl(Val x, int bits) { return {this->push(Op::shl, x.id,0,0, bits)}; }
    Val Builder::shr(Val x, int bits) { return {this->push(Op::shr, x.id,0,0, bits)}; }
    Val Builder::sra(Val x, int bits) { return {this->push(Op::sra, x.id,0,0, bits)}; }

    Val Builder::to_f32(Val x) { return {this->push(Op::to_f32, x.id)}; }
    Val Builder::to_i32(Val x) { return {this->push(Op::to_i32, x.id)}; }

    // ~~~~ Program::dump() and co. ~~~~ //

    static void dump(SkWStream* o, const char* s) {
        o->writeText(s);
    }

    static void dump(SkWStream* o, Arg a) {
        dump(o, "arg(");
        o->writeDecAsText(~a.id);
        dump(o, ")");
    }
    static void dump(SkWStream* o, Val v) {
        dump(o, "v");
        o->writeDecAsText(v.id);
    }

    static void dump(SkWStream* o, int bits) {
        float f;
        memcpy(&f, &bits, 4);
        o->writeHexAsText(bits);
        dump(o, " (");
        o->writeScalarAsText(f);
        dump(o, ")");
    }

    template <typename T>
    static void dumpln(SkWStream* o, T v) {
        dump(o, v);
        dump(o, "\n");
    }

    template <typename T, typename... Ts>
    static void dumpln(SkWStream* o, T first, Ts... rest) {
        dump(o, first);
        dump(o, " ");
        dumpln(o, rest...);
    }

    void Program::dump(SkWStream* o) const {
        for (int i = 0; i < (int)fInstructions.size(); i++) {
            Instruction inst = fInstructions[i];
            Op   op = inst.op;
            ID    x = inst.x,
                  y = inst.y,
                  z = inst.z;
            int imm = inst.imm;
            ID   id = i+1;
            ::skvm::dump(o, inst.live ? " " : "☠️");
            switch (op) {
                case Op::store8:  dumpln(o, "store8" , Arg{x}, Val{y}); break;
                case Op::store32: dumpln(o, "store32", Arg{x}, Val{y}); break;

                case Op::load8:  dumpln(o, Val{id}, "= load8" , Arg{x}); break;
                case Op::load32: dumpln(o, Val{id}, "= load32", Arg{x}); break;

                case Op::splat:  dumpln(o, Val{id}, "= splat", imm); break;

                case Op::add_f32: dumpln(o, Val{id}, "= add_f32", Val{x}, Val{y}        ); break;
                case Op::sub_f32: dumpln(o, Val{id}, "= sub_f32", Val{x}, Val{y}        ); break;
                case Op::mul_f32: dumpln(o, Val{id}, "= mul_f32", Val{x}, Val{y}        ); break;
                case Op::div_f32: dumpln(o, Val{id}, "= div_f32", Val{x}, Val{y}        ); break;
                case Op::mad_f32: dumpln(o, Val{id}, "= mad_f32", Val{x}, Val{y}, Val{z}); break;

                case Op::add_i32: dumpln(o, Val{id}, "= add_i32", Val{x}, Val{y}); break;
                case Op::sub_i32: dumpln(o, Val{id}, "= sub_i32", Val{x}, Val{y}); break;
                case Op::mul_i32: dumpln(o, Val{id}, "= mul_i32", Val{x}, Val{y}); break;
                case Op::div_i32: dumpln(o, Val{id}, "= div_i32", Val{x}, Val{y}); break;

                case Op::bit_and: dumpln(o, Val{id}, "= bit_and", Val{x}, Val{y}); break;
                case Op::bit_or : dumpln(o, Val{id}, "= bit_or" , Val{x}, Val{y}); break;
                case Op::bit_xor: dumpln(o, Val{id}, "= bit_xor", Val{x}, Val{y}); break;

                case Op::shl: dumpln(o, Val{id}, "= shl", Val{x}, imm); break;
                case Op::shr: dumpln(o, Val{id}, "= shr", Val{x}, imm); break;
                case Op::sra: dumpln(o, Val{id}, "= sra", Val{x}, imm); break;

                case Op::to_f32: dumpln(o, Val{id}, "= to_f32", Val{x}); break;
                case Op::to_i32: dumpln(o, Val{id}, "= to_i32", Val{x}); break;
            }
        }
    }

    // ~~~~ Program::eval() and co. ~~~~ //

    void Program::eval(int n, void* args[], size_t strides[], int nargs) const {
        // We'll operate in SIMT style, knocking off K-size chunks from n while possible.
        constexpr int K = 4;
        using I32 = skvx::Vec<K, int>;
        using F32 = skvx::Vec<K, float>;
        using U32 = skvx::Vec<K, uint32_t>;
        using  U8 = skvx::Vec<K, uint8_t>;

        // The simplest register assignment possible:
        // allocate a distinct slot for each Val, with vN stored in r[N].
        union Reg {
            I32 i32;
            U32 u32;
            F32 f32;
        };

        // vN is in r[N], so we need an extra slot.  (v0 is an unused sentinel).
        std::vector<Reg> r(fInstructions.size() + 1);

        // Our convention for argument IDs works down from ~0.
        auto arg = [&](ID x) { return args[~x]; };

        // Step each argument pointer ahead by its stride a number of times.
        auto step_args = [&](int times) {
            for (int i = 0; i < nargs; i++) {
                args[i] = (void*)( (char*)args[i] + times*strides[i] );
            }
        };

        int stride;
        for ( ; n > 0; n -= stride, step_args(stride)) {
            // This could be n >= K; we skip the last stride=K pass to give stride=1 more coverage.
            stride = n > K ? K : 1;

            for (int i = 0; i < (int)fInstructions.size(); i++) {
                Instruction inst = fInstructions[i];
                if (!inst.live) {
                    continue;
                }

                // Short names for commonly used IDs: this op's arguments and its own result.
                ID x = inst.x,
                   y = inst.y,
                   z = inst.z,
                  id = i+1;

                // Ops that interact with memory need to know whether we're stride=1 or stride=K,
                // but all non-memory ops can run the same code no matter the stride.
                switch (2*(int)inst.op + (stride == K ? 1 : 0)) {

                #define STRIDE_1(op) case 2*(int)op
                #define STRIDE_K(op) case 2*(int)op + 1
                    STRIDE_1(Op::store8 ): memcpy(arg(x), &r[y].i32, 1); break;
                    STRIDE_1(Op::store32): memcpy(arg(x), &r[y].i32, 4); break;

                    STRIDE_K(Op::store8 ): skvx::cast<uint8_t>(r[y].i32).store(arg(x)); break;
                    STRIDE_K(Op::store32):                    (r[y].i32).store(arg(x)); break;

                    STRIDE_1(Op::load8 ): r[id].i32 = 0; memcpy(&r[id].i32, arg(x), 1); break;
                    STRIDE_1(Op::load32): r[id].i32 = 0; memcpy(&r[id].i32, arg(x), 4); break;

                    STRIDE_K(Op::load8 ): r[id].i32 = skvx::cast<int>(U8 ::Load(arg(x))); break;
                    STRIDE_K(Op::load32): r[id].i32 =                 I32::Load(arg(x)) ; break;
                #undef TAIL
                #undef BODY

                    // Ops that don't interact with memory should never care about the stride.
                #define CASE(op) case 2*(int)op: /*fallthrough*/ case 2*(int)op+1
                    CASE(Op::splat): r[id].i32 = inst.imm; break;

                    CASE(Op::add_f32): r[id].f32 = r[x].f32 + r[y].f32           ; break;
                    CASE(Op::sub_f32): r[id].f32 = r[x].f32 - r[y].f32           ; break;
                    CASE(Op::mul_f32): r[id].f32 = r[x].f32 * r[y].f32           ; break;
                    CASE(Op::div_f32): r[id].f32 = r[x].f32 / r[y].f32           ; break;
                    CASE(Op::mad_f32): r[id].f32 = r[x].f32 * r[y].f32 + r[z].f32; break;

                    CASE(Op::add_i32): r[id].i32 = r[x].i32 + r[y].i32; break;
                    CASE(Op::sub_i32): r[id].i32 = r[x].i32 - r[y].i32; break;
                    CASE(Op::mul_i32): r[id].i32 = r[x].i32 * r[y].i32; break;
                    CASE(Op::div_i32): r[id].i32 = r[x].i32 / r[y].i32; break;
                    CASE(Op::bit_and): r[id].i32 = r[x].i32 & r[y].i32; break;

                    CASE(Op::bit_or ): r[id].i32 = r[x].i32 | r[y].i32; break;
                    CASE(Op::bit_xor): r[id].i32 = r[x].i32 ^ r[y].i32; break;

                    CASE(Op::shl): r[id].i32 =                 r[x].i32 << inst.imm ; break;
                    CASE(Op::sra): r[id].i32 =                 r[x].i32 >> inst.imm ; break;
                    CASE(Op::shr): r[id].i32 = skvx::cast<int>(r[x].u32 >> inst.imm); break;

                    CASE(Op::to_f32): r[id].f32 = skvx::cast<float>(r[x].i32); break;
                    CASE(Op::to_i32): r[id].i32 = skvx::cast<int>  (r[x].f32); break;
                #undef CASE
                }
            }
        }
    }
}
