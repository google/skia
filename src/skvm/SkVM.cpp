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

    // Most ops produce a value and return it by ID, the value-producing op's own
    // index in the valops vector.  (Uniforms, Varyings, and Labels can all share
    // this common space, with args as pre-defined uniforms descending from ~0.)
    //
    // Ops that don't produce a value like jumps and stores are side-effect-only.
    SK_WARN_UNUSED_RESULT
    ID Builder::push(Op op, ID x=0, ID y=0, ID z=0) {
        this->valops.push_back({op, x, y, z});
        return static_cast<ID>(this->valops.size() - 1);
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
    V Builder::splat(int   n) { return {this->push(Op::splat, n) }; }
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

    V Builder::shl(V x, int bits) { return {this->push(Op::shl, x.id, bits)}; }
    V Builder::shr(V x, int bits) { return {this->push(Op::shr, x.id, bits)}; }
    V Builder::sra(V x, int bits) { return {this->push(Op::sra, x.id, bits)}; }

    V Builder::to_f32(V x) { return {this->push(Op::to_f32, x.id)}; }
    V Builder::to_i32(V x) { return {this->push(Op::to_i32, x.id)}; }

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
            Op op = valops[i].op;
            ID  x = valops[i].x,
                y = valops[i].y,
                z = valops[i].z;
            switch (op) {
                case Op::label: dumpln(L{i}, ":"); break;

                case Op::jump:        dumpln("jump"       , L{x}      ); break;
                case Op::jump_if:     dumpln("jump_if"    , L{x}, U{y}); break;
                case Op::jump_if_not: dumpln("jump_if_not", L{x}, U{y}); break;

                case Op::store8:  dumpln("store8" , U{x}, V{y}); break;
                case Op::store32: dumpln("store32", U{x}, V{y}); break;

                case Op::load8:  dumpln(V{i}, "= load8" , U{x}); break;
                case Op::load32: dumpln(V{i}, "= load32", U{x}); break;

                case Op::splat:  dumpln(V{i}, "= splat", x); break;

                case Op::add_f32: dumpln(V{i}, "= add_f32", V{x}, V{y}      ); break;
                case Op::sub_f32: dumpln(V{i}, "= sub_f32", V{x}, V{y}      ); break;
                case Op::mul_f32: dumpln(V{i}, "= mul_f32", V{x}, V{y}      ); break;
                case Op::div_f32: dumpln(V{i}, "= div_f32", V{x}, V{y}      ); break;
                case Op::mad_f32: dumpln(V{i}, "= mad_f32", V{x}, V{y}, V{z}); break;

                case Op::add_i32: dumpln(V{i}, "= add_i32", V{x}, V{y}); break;
                case Op::sub_i32: dumpln(V{i}, "= sub_i32", V{x}, V{y}); break;
                case Op::mul_i32: dumpln(V{i}, "= mul_i32", V{x}, V{y}); break;
                case Op::div_i32: dumpln(V{i}, "= div_i32", V{x}, V{y}); break;

                case Op::bit_and: dumpln(V{i}, "= bit_and", V{x}, V{y}); break;
                case Op::bit_or : dumpln(V{i}, "= bit_or" , V{x}, V{y}); break;
                case Op::bit_xor: dumpln(V{i}, "= bit_xor", V{x}, V{y}); break;

                case Op::shl: dumpln(V{i}, "= shl", V{x}, y); break;
                case Op::shr: dumpln(V{i}, "= shr", V{x}, y); break;
                case Op::sra: dumpln(V{i}, "= sra", V{x}, y); break;

                case Op::to_f32: dumpln(V{i}, "= to_f32", V{x}); break;
                case Op::to_i32: dumpln(V{i}, "= to_i32", V{x}); break;
            }
        }
    }

}
