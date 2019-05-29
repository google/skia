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

    Program::Program(std::vector<Instruction> instructions, int regs)
        : fInstructions(std::move(instructions))
        , fRegs(regs)
    {}

    Program Builder::done() {
        // Basic liveness analysis (and free dead code elimination).
        for (auto i = fProgram.size(); i --> 0; ) {
            Instruction& inst = fProgram[i];
            ID id = i+1;

            // All side-effect-only instructions (stores) are live.
            if (inst.op <= Op::store32) {
                inst.life = id;
            }
            // The arguments of a live instruction must live until that instruction.
            if (inst.life) {
                // Skip id == 0 (N/A).
                // Notice how we're walking backward, storing the latest instruction in life.
                if (inst.x && !fProgram[inst.x-1].life) { fProgram[inst.x-1].life = id; }
                if (inst.y && !fProgram[inst.y-1].life) { fProgram[inst.y-1].life = id; }
                if (inst.z && !fProgram[inst.z-1].life) { fProgram[inst.z-1].life = id; }
            }
        }

        // We'll need to map each live value to a register.
        std::unordered_map<ID, ID> val_to_reg;

        // Count the registers we've used so far, and track any registers available to reuse.
        ID next_reg = 0;
        std::vector<ID> avail;

        // A schedule of which registers become available as we reach any given instruction.
        std::unordered_map<ID, std::vector<ID>> deaths;

        for (int i = 0; i < (int)fProgram.size(); i++) {
            Instruction& inst = fProgram[i];
            ID val = i+1;
            if (!inst.life) {
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

        // Add a dummy mapping for v0 to any arbitrary register, here to r0,
        // so that the lookups don't have to know which arguments are used by which Ops.
        auto lookup_register = [&](ID val) {
            return val ? val_to_reg[val]
                       : 0;   // Or next_reg, or 1, or ~0, or anything really.
        };

        std::vector<Program::Instruction> program;
        for (int i = 0; i < (int)fProgram.size(); i++) {
            Instruction& inst = fProgram[i];
            ID id = i+1;
            if (!inst.life) {
                continue;
            }

            Program::Instruction pinst{
                inst.op,
                lookup_register(id),
                lookup_register(inst.x),
                lookup_register(inst.y),
               {lookup_register(inst.z)},
            };
            // If the z argument is the v0 sentinel, copy in the immediate instead.
            // (No Op uses both 3 arguments and an immediate.)
            if (inst.z == 0) {
                pinst.z.imm = inst.imm;
            }
            program.push_back(pinst);
        }

        return { std::move(program), /*register count = */next_reg };
    }

    // Most instructions produce a value and return it by ID, the value-producing
    // instructions's own index in the program vector plus 1, reserving ID 0 as a
    // sentinel.

    ID Builder::push(Op op, ID x=0, ID y=0, ID z=0, int imm=0) {
        Instruction inst{op, /*life=*/0, x, y, z, imm};

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

    Arg Builder::arg(int ix) { return {ix}; }

    void Builder::store8 (Arg ptr, I32 val) { (void)this->push(Op::store8 , val.id,0,0, ptr.ix); }
    void Builder::store32(Arg ptr, I32 val) { (void)this->push(Op::store32, val.id,0,0, ptr.ix); }

    I32 Builder::load8 (Arg ptr) { return {this->push(Op::load8 , 0,0,0, ptr.ix) }; }
    I32 Builder::load32(Arg ptr) { return {this->push(Op::load32, 0,0,0, ptr.ix) }; }

    // The two splat() functions are just syntax sugar over splatting a 4-byte bit pattern.
    I32 Builder::splat(int   n) { return {this->push(Op::splat, 0,0,0, n) }; }
    F32 Builder::splat(float f) {
        int bits;
        memcpy(&bits, &f, 4);
        return {this->push(Op::splat, 0,0,0, bits)};
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

    I32 Builder::shl(I32 x, int bits) { return {this->push(Op::shl, x.id,0,0, bits)}; }
    I32 Builder::shr(I32 x, int bits) { return {this->push(Op::shr, x.id,0,0, bits)}; }
    I32 Builder::sra(I32 x, int bits) { return {this->push(Op::sra, x.id,0,0, bits)}; }

    F32 Builder::to_f32(I32 x) { return {this->push(Op::to_f32, x.id)}; }
    I32 Builder::to_i32(F32 x) { return {this->push(Op::to_i32, x.id)}; }

    // ~~~~ Program::dump() and co. ~~~~ //

    struct Reg { ID id; };

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

    static void write(SkWStream* o, int bits) {
        float f;
        memcpy(&f, &bits, 4);
        o->writeHexAsText(bits);
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
        for (int i = 0; i < (int)fInstructions.size(); i++) {
            Instruction inst = fInstructions[i];
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

                case Op::splat:  write(o, Reg{d}, "= splat", z.imm); break;

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

                case Op::shl: write(o, Reg{d}, "= shl", Reg{x}, z.imm); break;
                case Op::shr: write(o, Reg{d}, "= shr", Reg{x}, z.imm); break;
                case Op::sra: write(o, Reg{d}, "= sra", Reg{x}, z.imm); break;

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
