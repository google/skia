/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVM.h"
#include <string.h>

namespace skvm {
    std::vector<ValOp> Builder::done() {
        // Basic dead code elimination.
        for (auto i = this->valops.size(); i --> 0; ) {
            ValOp& valop = this->valops[i];

            // All side-effect-only ops (stores) are live.
            if (valop.op <= Op::store32) {
                valop.live = true;
            }
            // The arguments of any live ops are live.
            if (valop.live) {
                // Skip id == 0 (N/A) and id < 0 (Args).
                if (valop.x > 0) { this->valops[valop.x-1].live = true; }
                if (valop.y > 0) { this->valops[valop.y-1].live = true; }
                if (valop.z > 0) { this->valops[valop.z-1].live = true; }
            }
        }
        return std::move(this->valops);
    }

    // Most ops produce a value and return it by ID, the value-producing op's own
    // index in the valops vector + 1, leaving 0 as a sentinel.  Args use the same
    // ID space, working down from ~0.
    SK_WARN_UNUSED_RESULT
    ID Builder::push(Op op, ID x=0, ID y=0, ID z=0, int imm=0) {
        ValOp valop{op, /*live=*/false, x, y, z, imm};

        auto lookup = this->index.find(valop);
        if (lookup != this->index.end()) {
            return lookup->second;
        }

        this->valops.push_back(valop);
        ID id = static_cast<ID>(this->valops.size());
        this->index[valop] = id;
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

    // ~~~~ dump() and co. ~~~~ //

    static void dump(Arg a) { SkDebugf("arg(%d)", ~a.id); }
    static void dump(Val v) { SkDebugf("v%d"    ,  v.id); }

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
                case Op::store8:  dumpln("store8" , Arg{x}, Val{y}); break;
                case Op::store32: dumpln("store32", Arg{x}, Val{y}); break;

                case Op::load8:  dumpln(Val{id}, "= load8" , Arg{x}); break;
                case Op::load32: dumpln(Val{id}, "= load32", Arg{x}); break;

                case Op::splat:  dumpln(Val{id}, "= splat", imm); break;

                case Op::add_f32: dumpln(Val{id}, "= add_f32", Val{x}, Val{y}        ); break;
                case Op::sub_f32: dumpln(Val{id}, "= sub_f32", Val{x}, Val{y}        ); break;
                case Op::mul_f32: dumpln(Val{id}, "= mul_f32", Val{x}, Val{y}        ); break;
                case Op::div_f32: dumpln(Val{id}, "= div_f32", Val{x}, Val{y}        ); break;
                case Op::mad_f32: dumpln(Val{id}, "= mad_f32", Val{x}, Val{y}, Val{z}); break;

                case Op::add_i32: dumpln(Val{id}, "= add_i32", Val{x}, Val{y}); break;
                case Op::sub_i32: dumpln(Val{id}, "= sub_i32", Val{x}, Val{y}); break;
                case Op::mul_i32: dumpln(Val{id}, "= mul_i32", Val{x}, Val{y}); break;
                case Op::div_i32: dumpln(Val{id}, "= div_i32", Val{x}, Val{y}); break;

                case Op::bit_and: dumpln(Val{id}, "= bit_and", Val{x}, Val{y}); break;
                case Op::bit_or : dumpln(Val{id}, "= bit_or" , Val{x}, Val{y}); break;
                case Op::bit_xor: dumpln(Val{id}, "= bit_xor", Val{x}, Val{y}); break;

                case Op::shl: dumpln(Val{id}, "= shl", Val{x}, imm); break;
                case Op::shr: dumpln(Val{id}, "= shr", Val{x}, imm); break;
                case Op::sra: dumpln(Val{id}, "= sra", Val{x}, imm); break;

                case Op::to_f32: dumpln(Val{id}, "= to_f32", Val{x}); break;
                case Op::to_i32: dumpln(Val{id}, "= to_i32", Val{x}); break;
            }
        }
    }

    // ~~~~ eval() and co. ~~~~ //

    void eval(const ValOp* valops, int n, void** args) {
        union Slot {
            int    i32;
            float  f32;
        };
        std::vector<Slot> vals(n+1);  // One extra slot because ID 0 is never used.

        for (int i = 0; i < n; i++) {
            if (!valops[i].live) {
                continue;
            }
            Op   op = valops[i].op;
            ID    x = valops[i].x,
                  y = valops[i].y,
                  z = valops[i].z;
            int imm = valops[i].imm;
            ID   id = i+1;
            switch (op) {
                case Op::store8:  *(uint8_t*)args[~x] = vals[y].i32; break;
                case Op::store32: *(int*    )args[~x] = vals[y].i32; break;

                case Op::load8:  vals[id].i32 = *(uint8_t*)args[~x]; break;
                case Op::load32: vals[id].i32 = *(int*    )args[~x]; break;

                case Op::splat:  vals[id].i32 = imm; break;

                case Op::add_f32: vals[id].f32 = vals[x].f32 + vals[y].f32              ; break;
                case Op::sub_f32: vals[id].f32 = vals[x].f32 - vals[y].f32              ; break;
                case Op::mul_f32: vals[id].f32 = vals[x].f32 * vals[y].f32              ; break;
                case Op::div_f32: vals[id].f32 = vals[x].f32 / vals[y].f32              ; break;
                case Op::mad_f32: vals[id].f32 = vals[x].f32 * vals[y].f32 + vals[z].f32; break;

                case Op::add_i32: vals[id].i32 = vals[x].i32 + vals[y].i32; break;
                case Op::sub_i32: vals[id].i32 = vals[x].i32 - vals[y].i32; break;
                case Op::mul_i32: vals[id].i32 = vals[x].i32 * vals[y].i32; break;
                case Op::div_i32: vals[id].i32 = vals[x].i32 / vals[y].i32; break;

                case Op::bit_and: vals[id].i32 = vals[x].i32 & vals[y].i32; break;
                case Op::bit_or : vals[id].i32 = vals[x].i32 | vals[y].i32; break;
                case Op::bit_xor: vals[id].i32 = vals[x].i32 ^ vals[y].i32; break;

                case Op::shl: vals[id].i32 = (uint32_t)vals[x].i32 << imm; break;
                case Op::shr: vals[id].i32 = (uint32_t)vals[x].i32 >> imm; break;
                case Op::sra: vals[id].i32 =           vals[x].i32 >> imm; break;

                case Op::to_f32: vals[id].f32 = vals[x].i32; break;
                case Op::to_i32: vals[id].i32 = vals[x].f32; break;
            }
        }
    }
}
