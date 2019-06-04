/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkVx.h"
#include "src/core/SkOpts.h"
#include "src/core/SkVM.h"
#include <string.h>

namespace skvm {

    // We reserve the last ID as a sentinel meaning none, n/a, null, nil, etc.
    static const ID NA = ~0;

    Program::Program(std::vector<Instruction> instructions, int regs)
        : fInstructions(std::move(instructions))
        , fRegs(regs)
    {}

    Program Builder::done() {
        // Basic liveness analysis (and free dead code elimination).
        for (ID id = fProgram.size(); id --> 0; ) {
            Instruction& inst = fProgram[id];

            // All side-effect-only instructions (stores) are live.
            if (inst.op <= Op::store32) {
                inst.life = id;
            }
            // The arguments of a live instruction must live until that instruction.
            if (inst.life != NA) {
                // Notice how we're walking backward, storing the latest instruction in life.
                if (inst.x != NA && fProgram[inst.x].life == NA) { fProgram[inst.x].life = id; }
                if (inst.y != NA && fProgram[inst.y].life == NA) { fProgram[inst.y].life = id; }
                if (inst.z != NA && fProgram[inst.z].life == NA) { fProgram[inst.z].life = id; }
            }
        }

        // We'll need to map each live value to a register.
        std::unordered_map<ID, ID> val_to_reg;

        // Count the registers we've used so far, and track any registers available to reuse.
        ID next_reg = 0;
        std::vector<ID> avail;

        // A schedule of which registers become available as we reach any given instruction.
        std::unordered_map<ID, std::vector<ID>> deaths;

        for (ID val = 0; val < (ID)fProgram.size(); val++) {
            Instruction& inst = fProgram[val];
            if (inst.life == NA) {
                continue;
            }

            // All the values that are no longer needed after this instruction
            // can make their registers available to this and future values.
            const std::vector<ID>& dying = deaths[val];
            avail.insert(avail.end(),
                         dying.begin(), dying.end());

            // Allocate a register if we have to, but prefer to reuse one that's available.
            ID reg;
            if (avail.empty()) {
                reg = next_reg++;
            } else {
                reg = avail.back();
                avail.pop_back();
            }

            // Schedule this value's own death.  When we reach the instruction at inst.life,
            // this value is no longer needed and its register becomes available for reuse.
            deaths[inst.life].push_back(reg);

            val_to_reg[val] = reg;
        }

        // Add a dummy mapping for the N/A sentinel value to "register N/A",
        // so that the lookups don't have to know which arguments are used by which Ops.
        auto lookup_register = [&](ID val) {
            return val == NA ? NA
                             : val_to_reg[val];
        };

        std::vector<Program::Instruction> program;
        for (ID id = 0; id < (ID)fProgram.size(); id++) {
            Instruction& inst = fProgram[id];
            if (inst.life == NA) {
                continue;
            }

            Program::Instruction pinst{
                inst.op,
                lookup_register(id),
                lookup_register(inst.x),
                lookup_register(inst.y),
               {lookup_register(inst.z)},
            };
            // If the z argument is the N/A sentinel, copy in the immediate instead.
            // (No Op uses both 3 arguments and an immediate.)
            if (inst.z == NA) {
                pinst.z.imm = inst.imm;
            }
            program.push_back(pinst);
        }

        return { std::move(program), /*register count = */next_reg };
    }

    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.

    ID Builder::push(Op op, ID x=NA, ID y=NA, ID z=NA, int imm=0) {
        Instruction inst{op, /*life=*/NA, x, y, z, imm};

        // Simple peepholes that come up fairly often.
        if (op == Op::extract && imm == (int)0xff000000) { inst = { Op::shr, NA, x,NA,NA, 24 }; }

        auto is_zero = [&](ID id) {
            return fProgram[id].op  == Op::splat
                && fProgram[id].imm == 0;
        };

        // x*y+0 --> x*y
        if (op == Op::mad_f32 && is_zero(z)) { inst = { Op::mul_f32, NA, x,y,NA, 0 }; }


        // Basic common subexpression elimination:
        // if we've already seen this exact Instruction, use it instead of creating a new one.
        auto lookup = fIndex.find(inst);
        if (lookup != fIndex.end()) {
            return lookup->second;
        }

        ID id = static_cast<ID>(fProgram.size());
        fProgram.push_back(inst);
        fIndex[inst] = id;
        return id;
    }

    Arg Builder::arg(int ix) { return {ix}; }

    void Builder::store8 (Arg ptr, I32 val) { (void)this->push(Op::store8 , val.id,NA,NA, ptr.ix); }
    void Builder::store32(Arg ptr, I32 val) { (void)this->push(Op::store32, val.id,NA,NA, ptr.ix); }

    I32 Builder::load8 (Arg ptr) { return {this->push(Op::load8 , NA,NA,NA, ptr.ix) }; }
    I32 Builder::load32(Arg ptr) { return {this->push(Op::load32, NA,NA,NA, ptr.ix) }; }

    // The two splat() functions are just syntax sugar over splatting a 4-byte bit pattern.
    I32 Builder::splat(int   n) { return {this->push(Op::splat, NA,NA,NA, n) }; }
    F32 Builder::splat(float f) {
        int bits;
        memcpy(&bits, &f, 4);
        return {this->push(Op::splat, NA,NA,NA, bits)};
    }

    F32 Builder::add(F32 x, F32 y       ) { return {this->push(Op::add_f32, x.id, y.id      )}; }
    F32 Builder::sub(F32 x, F32 y       ) { return {this->push(Op::sub_f32, x.id, y.id      )}; }
    F32 Builder::mul(F32 x, F32 y       ) { return {this->push(Op::mul_f32, x.id, y.id      )}; }
    F32 Builder::div(F32 x, F32 y       ) { return {this->push(Op::div_f32, x.id, y.id      )}; }
    F32 Builder::mad(F32 x, F32 y, F32 z) { return {this->push(Op::mad_f32, x.id, y.id, z.id)}; }

    I32 Builder::add(I32 x, I32 y) { return {this->push(Op::add_i32, x.id, y.id)}; }
    I32 Builder::sub(I32 x, I32 y) { return {this->push(Op::sub_i32, x.id, y.id)}; }
    I32 Builder::mul(I32 x, I32 y) { return {this->push(Op::mul_i32, x.id, y.id)}; }

    I32 Builder::bit_and(I32 x, I32 y) { return {this->push(Op::bit_and, x.id, y.id)}; }
    I32 Builder::bit_or (I32 x, I32 y) { return {this->push(Op::bit_or , x.id, y.id)}; }
    I32 Builder::bit_xor(I32 x, I32 y) { return {this->push(Op::bit_xor, x.id, y.id)}; }

    I32 Builder::shl(I32 x, int bits) { return {this->push(Op::shl, x.id,NA,NA, bits)}; }
    I32 Builder::shr(I32 x, int bits) { return {this->push(Op::shr, x.id,NA,NA, bits)}; }
    I32 Builder::sra(I32 x, int bits) { return {this->push(Op::sra, x.id,NA,NA, bits)}; }

    I32 Builder::mul_unorm8(I32 x, I32 y) { return {this->push(Op::mul_unorm8, x.id, y.id)}; }

    I32 Builder::extract(I32 x, int mask) { return {this->push(Op::extract, x.id,NA,NA, mask)}; }
    I32 Builder::pack(I32 x, I32 y, int bits) { return {this->push(Op::pack, x.id,y.id,NA, bits)}; }

    F32 Builder::to_f32(I32 x) { return {this->push(Op::to_f32, x.id)}; }
    I32 Builder::to_i32(F32 x) { return {this->push(Op::to_i32, x.id)}; }

    // ~~~~ Program::dump() and co. ~~~~ //

    struct Reg { ID id; };
    struct Shift { int bits; };
    struct Mask  { int bits; };
    struct Splat { int bits; };

    static void write(SkWStream* o, const char* s) {
        o->writeText(s);
    }

    static void write(SkWStream* o, Arg a) {
        write(o, "arg(");
        o->writeDecAsText(a.ix);
        write(o, ")");
    }
    static void write(SkWStream* o, Reg r) {
        write(o, "r");
        o->writeDecAsText(r.id);
    }
    static void write(SkWStream* o, Shift s) {
        o->writeDecAsText(s.bits);
    }
    static void write(SkWStream* o, Mask m) {
        o->writeHexAsText(m.bits);
    }
    static void write(SkWStream* o, Splat s) {
        float f;
        memcpy(&f, &s.bits, 4);
        o->writeHexAsText(s.bits);
        write(o, " (");
        o->writeScalarAsText(f);
        write(o, ")");
    }

    template <typename T, typename... Ts>
    static void write(SkWStream* o, T first, Ts... rest) {
        write(o, first);
        write(o, " ");
        write(o, rest...);
    }

    void Program::dump(SkWStream* o) const {
        for (const Instruction& inst : fInstructions) {
            Op  op = inst.op;
            ID   d = inst.d,
                 x = inst.x,
                 y = inst.y;
            auto z = inst.z;
            switch (op) {
                case Op::store8:  write(o, "store8" , Arg{z.imm}, Reg{x}); break;
                case Op::store32: write(o, "store32", Arg{z.imm}, Reg{x}); break;

                case Op::load8:  write(o, Reg{d}, "= load8" , Arg{z.imm}); break;
                case Op::load32: write(o, Reg{d}, "= load32", Arg{z.imm}); break;

                case Op::splat:  write(o, Reg{d}, "= splat", Splat{z.imm}); break;

                case Op::add_f32: write(o, Reg{d}, "= add_f32", Reg{x}, Reg{y}           ); break;
                case Op::sub_f32: write(o, Reg{d}, "= sub_f32", Reg{x}, Reg{y}           ); break;
                case Op::mul_f32: write(o, Reg{d}, "= mul_f32", Reg{x}, Reg{y}           ); break;
                case Op::div_f32: write(o, Reg{d}, "= div_f32", Reg{x}, Reg{y}           ); break;
                case Op::mad_f32: write(o, Reg{d}, "= mad_f32", Reg{x}, Reg{y}, Reg{z.id}); break;

                case Op::add_i32: write(o, Reg{d}, "= add_i32", Reg{x}, Reg{y}); break;
                case Op::sub_i32: write(o, Reg{d}, "= sub_i32", Reg{x}, Reg{y}); break;
                case Op::mul_i32: write(o, Reg{d}, "= mul_i32", Reg{x}, Reg{y}); break;

                case Op::bit_and: write(o, Reg{d}, "= bit_and", Reg{x}, Reg{y}); break;
                case Op::bit_or : write(o, Reg{d}, "= bit_or" , Reg{x}, Reg{y}); break;
                case Op::bit_xor: write(o, Reg{d}, "= bit_xor", Reg{x}, Reg{y}); break;

                case Op::shl: write(o, Reg{d}, "= shl", Reg{x}, Shift{z.imm}); break;
                case Op::shr: write(o, Reg{d}, "= shr", Reg{x}, Shift{z.imm}); break;
                case Op::sra: write(o, Reg{d}, "= sra", Reg{x}, Shift{z.imm}); break;

                case Op::mul_unorm8: write(o, Reg{d}, "= mul_unorm8", Reg{x}, Reg{y}); break;

                case Op::extract: write(o, Reg{d}, "= extract", Reg{x}, Mask{z.imm}); break;
                case Op::pack: write(o, Reg{d}, "= pack", Reg{x}, Reg{y}, Shift{z.imm}); break;

                case Op::to_f32: write(o, Reg{d}, "= to_f32", Reg{x}); break;
                case Op::to_i32: write(o, Reg{d}, "= to_i32", Reg{x}); break;
            }
            write(o, "\n");
        }
    }

    // ~~~~ Program::eval() and co. ~~~~ //

    void Program::eval(int n, void* args[], size_t strides[], int nargs) const {
        SkOpts::eval(fInstructions.data(), (int)fInstructions.size(), fRegs,
                     n, args, strides, nargs);
    }
}
