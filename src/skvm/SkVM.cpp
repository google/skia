/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVM.h"
#include <string.h>

namespace skvm {
    using U = Uniform;
    using V = Varying;
    using L = Label;

    std::vector<ValOp> Builder::done() {
        // Basic dead code elimination.
        for (auto i = this->valops.size(); i --> 0; ) {
            ValOp& valop = this->valops[i];

            // All side-effect-only ops (jumps, stores) are live.
            if (valop.op < Op::label) {
                valop.live = true;
            }
            // The arguments of any live ops are live.
            if (this->valops[i].live) {
                if (valop.x > 0) { this->valops[valop.x-1].live = true; }
                if (valop.y > 0) { this->valops[valop.y-1].live = true; }
                if (valop.z > 0) { this->valops[valop.z-1].live = true; }
            }
        }
        return std::move(this->valops);
    }

    // Most ops produce a value and return it by ID, the value-producing op's own
    // index in the valops vector + 1, leaving 0 as a sentinel.  (Uniforms,
    // Varyings, and Labels all share this common ID space, with args as
    // pre-defined uniforms descending from ~0.)
    //
    // Ops that don't produce a value like jumps and stores are side-effect-only.
    SK_WARN_UNUSED_RESULT
    ID Builder::push(Op op, ID x=0, ID y=0, ID z=0, int imm=0) {
        this->valops.push_back({op, /*live=*/false, x, y, z, imm});
        return static_cast<ID>(this->valops.size());
    }

    // arg(i) is just a convenient way to refer to a predefined uniform argument.
    // By convention we use the very top of the ID space for these, descending.
    U Builder::arg(int i) { return {~i}; }

    L Builder::label() { return {this->push(Op::label) }; }

    void Builder::jump       (L t     ) { (void)this->push(Op::jump       , t.id      ); }
    void Builder::jump_if    (L t, U c) { (void)this->push(Op::jump_if    , t.id, c.id); }
    void Builder::jump_if_not(L t, U c) { (void)this->push(Op::jump_if_not, t.id, c.id); }

    void Builder::store8 (U ptr, V val) { (void)this->push(Op::store8 , ptr.id, val.id); }
    void Builder::store32(U ptr, V val) { (void)this->push(Op::store32, ptr.id, val.id); }

    V Builder::load8 (U ptr) { return {this->push(Op::load8 , ptr.id) }; }
    V Builder::load32(U ptr) { return {this->push(Op::load32, ptr.id) }; }

    // The two splat() functions are just syntax sugar over splatting a 4-byte bit pattern.
    V Builder::splat(int   n) { return {this->push(Op::splat, 0,0,0, n) }; }
    V Builder::splat(float f) {
        int n;
        memcpy(&n, &f, 4);
        return this->splat(n);
    }

    V Builder::add_f32(V x, V y)      { return {this->push(Op::add_f32, x.id, y.id      )}; }
    V Builder::sub_f32(V x, V y)      { return {this->push(Op::sub_f32, x.id, y.id      )}; }
    V Builder::mul_f32(V x, V y)      { return {this->push(Op::mul_f32, x.id, y.id      )}; }
    V Builder::div_f32(V x, V y)      { return {this->push(Op::div_f32, x.id, y.id      )}; }
    V Builder::mad_f32(V x, V y, V z) { return {this->push(Op::mad_f32, x.id, y.id, z.id)}; }

    V Builder::add_i32(V x, V y) { return {this->push(Op::add_i32, x.id, y.id)}; }
    V Builder::sub_i32(V x, V y) { return {this->push(Op::sub_i32, x.id, y.id)}; }
    V Builder::mul_i32(V x, V y) { return {this->push(Op::mul_i32, x.id, y.id)}; }
    V Builder::div_i32(V x, V y) { return {this->push(Op::div_i32, x.id, y.id)}; }

    V Builder::bit_and(V x, V y) { return {this->push(Op::bit_and, x.id, y.id)}; }
    V Builder::bit_or (V x, V y) { return {this->push(Op::bit_or , x.id, y.id)}; }
    V Builder::bit_xor(V x, V y) { return {this->push(Op::bit_xor, x.id, y.id)}; }

    V Builder::shl(V x, int bits) { return {this->push(Op::shl, x.id,0,0, bits)}; }
    V Builder::shr(V x, int bits) { return {this->push(Op::shr, x.id,0,0, bits)}; }
    V Builder::sra(V x, int bits) { return {this->push(Op::sra, x.id,0,0, bits)}; }

    V Builder::to_f32(V x) { return {this->push(Op::to_f32, x.id)}; }
    V Builder::to_i32(V x) { return {this->push(Op::to_i32, x.id)}; }

    U Builder::uniform(int n) { return {this->push(Op::uniform, 0,0,0, n)}; }

    U Builder::add(U x, U y) { return {this->push(Op::add1, x.id, y.id)}; }
    U Builder::sub(U x, U y) { return {this->push(Op::sub1, x.id, y.id)}; }
    U Builder::mul(U x, U y) { return {this->push(Op::mul1, x.id, y.id)}; }
    U Builder::div(U x, U y) { return {this->push(Op::div1, x.id, y.id)}; }
    U Builder::mod(U x, U y) { return {this->push(Op::mod1, x.id, y.id)}; }

    U Builder::bit_and(U x, U y) { return {this->push(Op::bit_and1, x.id, y.id)}; }
    U Builder::bit_or (U x, U y) { return {this->push(Op::bit_or1 , x.id, y.id)}; }
    U Builder::bit_xor(U x, U y) { return {this->push(Op::bit_xor1, x.id, y.id)}; }

    U Builder::shl(U x, int bits) { return {this->push(Op::shl1, x.id,0,0, bits)}; }
    U Builder::shr(U x, int bits) { return {this->push(Op::shr1, x.id,0,0, bits)}; }
    U Builder::sra(U x, int bits) { return {this->push(Op::sra1, x.id,0,0, bits)}; }

    // ~~~~ dump() and co. ~~~~ //

    static void dump(U u) {
        if (u.id < 0) {
            SkDebugf("arg(%d)", ~u.id);
        } else {
            SkDebugf("u%d", u.id);
        }
    }
    static void dump(V v) { SkDebugf("v%d", v.id); }
    static void dump(L l) { SkDebugf("l%d", l.id); }

    static void dump(int bits) {
        float f;
        memcpy(&f, &bits, 4);
        SkDebugf("%08x (%f)", bits, f);
    }
    static void dump(const char* s) { SkDebugf("%s", s); }

    template <typename T>
    static void dumpln(T v) {
        dump(v);
        SkDebugf("\n");
    }

    template <typename T, typename... Ts>
    static void dumpln(T first, Ts... rest) {
        dump(first);
        SkDebugf(" ");
        dumpln(rest...);
    }

    void dump(const ValOp* valops, int n) {
        for (int i = 0; i < n; i++) {
            Op   op = valops[i].op;
            ID    x = valops[i].x,
                  y = valops[i].y,
                  z = valops[i].z;
            int imm = valops[i].imm;
            ID   id = i+1;
            SkDebugf("%s ", valops[i].live ? " " : "☠️");
            switch (op) {
                case Op::label: dumpln(L{id}, ":"); break;

                case Op::jump:        dumpln("jump"       , L{x}      ); break;
                case Op::jump_if:     dumpln("jump_if"    , L{x}, U{y}); break;
                case Op::jump_if_not: dumpln("jump_if_not", L{x}, U{y}); break;

                case Op::store8:  dumpln("store8" , U{x}, V{y}); break;
                case Op::store32: dumpln("store32", U{x}, V{y}); break;

                case Op::load8:  dumpln(V{id}, "= load8" , U{x}); break;
                case Op::load32: dumpln(V{id}, "= load32", U{x}); break;

                case Op::splat:  dumpln(V{id}, "= splat", imm); break;

                case Op::add_f32: dumpln(V{id}, "= add_f32", V{x}, V{y}      ); break;
                case Op::sub_f32: dumpln(V{id}, "= sub_f32", V{x}, V{y}      ); break;
                case Op::mul_f32: dumpln(V{id}, "= mul_f32", V{x}, V{y}      ); break;
                case Op::div_f32: dumpln(V{id}, "= div_f32", V{x}, V{y}      ); break;
                case Op::mad_f32: dumpln(V{id}, "= mad_f32", V{x}, V{y}, V{z}); break;

                case Op::add_i32: dumpln(V{id}, "= add_i32", V{x}, V{y}); break;
                case Op::sub_i32: dumpln(V{id}, "= sub_i32", V{x}, V{y}); break;
                case Op::mul_i32: dumpln(V{id}, "= mul_i32", V{x}, V{y}); break;
                case Op::div_i32: dumpln(V{id}, "= div_i32", V{x}, V{y}); break;

                case Op::bit_and: dumpln(V{id}, "= bit_and", V{x}, V{y}); break;
                case Op::bit_or : dumpln(V{id}, "= bit_or" , V{x}, V{y}); break;
                case Op::bit_xor: dumpln(V{id}, "= bit_xor", V{x}, V{y}); break;

                case Op::shl: dumpln(V{id}, "= shl", V{x}, imm); break;
                case Op::shr: dumpln(V{id}, "= shr", V{x}, imm); break;
                case Op::sra: dumpln(V{id}, "= sra", V{x}, imm); break;

                case Op::to_f32: dumpln(V{id}, "= to_f32", V{x}); break;
                case Op::to_i32: dumpln(V{id}, "= to_i32", V{x}); break;

                case Op::uniform: dumpln(U{id}, "=", imm); break;

                case Op::add1: dumpln(U{id}, "= add", U{x}, U{y}); break;
                case Op::sub1: dumpln(U{id}, "= sub", U{x}, U{y}); break;
                case Op::mul1: dumpln(U{id}, "= mul", U{x}, U{y}); break;
                case Op::div1: dumpln(U{id}, "= div", U{x}, U{y}); break;
                case Op::mod1: dumpln(U{id}, "= mod", U{x}, U{y}); break;

                case Op::bit_and1: dumpln(U{id}, "= bit_and", U{x}, U{y}); break;
                case Op::bit_or1 : dumpln(U{id}, "= bit_or" , U{x}, U{y}); break;
                case Op::bit_xor1: dumpln(U{id}, "= bit_xor", U{x}, U{y}); break;

                case Op::shl1: dumpln(U{id}, "= shl", U{x}, imm); break;
                case Op::shr1: dumpln(U{id}, "= shr", U{x}, imm); break;
                case Op::sra1: dumpln(U{id}, "= sra", U{x}, imm); break;
            }
        }
    }

}
