/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkVx.h"
#include "src/core/SkVM.h"
#include <string.h>

namespace skvm {

    Program::Program(std::vector<Instruction> instructions, int regs)
        : fInstructions(std::move(instructions))
        , fRegs(regs)
    {}

    Program Builder::done() {
        // Basic dead code elimination.
        for (auto i = fProgram.size(); i --> 0; ) {
            Instruction& inst = fProgram[i];
            ID id = i+1;

            // All side-effect-only instructions (stores) are live.
            if (inst.op <= Op::store32) {
                inst.life = id;
            }
            // The arguments of any live instructions are live.
            if (inst.life) {
                // Skip id == 0 (N/A) and id < 0 (Args).
                if (inst.x > 0 && !fProgram[inst.x-1].life) { fProgram[inst.x-1].life = id; }
                if (inst.y > 0 && !fProgram[inst.y-1].life) { fProgram[inst.y-1].life = id; }
                if (inst.z > 0 && !fProgram[inst.z-1].life) { fProgram[inst.z-1].life = id; }
            }
        }

        // Map live vals to registers.  For now just a simple 1:1, smarter TBD.
        std::unordered_map<ID, ID> val_to_reg;
        ID reg = 0;
        for (int i = 0; i < (int)fProgram.size(); i++) {
            Instruction& inst = fProgram[i];
            ID id = i+1;
            if (!inst.life) {
                continue;
            }

            val_to_reg[id] = reg++;
        }

        // Add a dummy mapping for v0 -> r0 so that we can do simple unconditional lookups,
        // and let any negative IDs for Args pass through directly.
        auto lookup_register = [&](ID val) {
            if (val <= 0) {
                return val;
            }
            return val_to_reg[val];
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
            if (pinst.z.id == 0) {
                pinst.z.imm = inst.imm;
            }
            program.push_back(pinst);
        }

        return { std::move(program), reg };
    }

    // Most instructions produce a value and return it by ID, the value-producing
    // instructions's own index in the program vector plus 1, reserving ID 0 as a
    // sentinel.  Args share the same ID space, working down from ~0.

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

    struct Reg { ID id; };

    static void write(SkWStream* o, const char* s) {
        o->writeText(s);
    }

    static void write(SkWStream* o, Arg a) {
        write(o, "arg(");
        o->writeDecAsText(~a.id);
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
                case Op::store8:  write(o, "store8" , Arg{x}, Reg{y}); break;
                case Op::store32: write(o, "store32", Arg{x}, Reg{y}); break;

                case Op::load8:  write(o, Reg{d}, "= load8" , Arg{x}); break;
                case Op::load32: write(o, Reg{d}, "= load32", Arg{x}); break;

                case Op::splat:  write(o, Reg{d}, "= splat", z.imm); break;

                case Op::add_f32: write(o, Reg{d}, "= add_f32", Reg{x}, Reg{y}           ); break;
                case Op::sub_f32: write(o, Reg{d}, "= sub_f32", Reg{x}, Reg{y}           ); break;
                case Op::mul_f32: write(o, Reg{d}, "= mul_f32", Reg{x}, Reg{y}           ); break;
                case Op::div_f32: write(o, Reg{d}, "= div_f32", Reg{x}, Reg{y}           ); break;
                case Op::mad_f32: write(o, Reg{d}, "= mad_f32", Reg{x}, Reg{y}, Reg{z.id}); break;

                case Op::add_i32: write(o, Reg{d}, "= add_i32", Reg{x}, Reg{y}); break;
                case Op::sub_i32: write(o, Reg{d}, "= sub_i32", Reg{x}, Reg{y}); break;
                case Op::mul_i32: write(o, Reg{d}, "= mul_i32", Reg{x}, Reg{y}); break;
                case Op::div_i32: write(o, Reg{d}, "= div_i32", Reg{x}, Reg{y}); break;

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
        // We'll operate in SIMT style, knocking off K-size chunks from n while possible.
        constexpr int K = 4;
        using I32 = skvx::Vec<K, int>;
        using F32 = skvx::Vec<K, float>;
        using U32 = skvx::Vec<K, uint32_t>;
        using  U8 = skvx::Vec<K, uint8_t>;

        union Slot {
            I32 i32;
            U32 u32;
            F32 f32;
        };
        std::vector<Slot> r(fRegs);

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

                // d = op(x, y, z.id/z.imm)
                ID   d = inst.d,
                     x = inst.x,
                     y = inst.y;
                auto z = inst.z;

                // Ops that interact with memory need to know whether we're stride=1 or stride=K,
                // but all non-memory ops can run the same code no matter the stride.
                switch (2*(int)inst.op + (stride == K ? 1 : 0)) {

                #define STRIDE_1(op) case 2*(int)op
                #define STRIDE_K(op) case 2*(int)op + 1
                    STRIDE_1(Op::store8 ): memcpy(arg(x), &r[y].i32, 1); break;
                    STRIDE_1(Op::store32): memcpy(arg(x), &r[y].i32, 4); break;

                    STRIDE_K(Op::store8 ): skvx::cast<uint8_t>(r[y].i32).store(arg(x)); break;
                    STRIDE_K(Op::store32):                    (r[y].i32).store(arg(x)); break;

                    STRIDE_1(Op::load8 ): r[d].i32 = 0; memcpy(&r[d].i32, arg(x), 1); break;
                    STRIDE_1(Op::load32): r[d].i32 = 0; memcpy(&r[d].i32, arg(x), 4); break;

                    STRIDE_K(Op::load8 ): r[d].i32 = skvx::cast<int>(U8 ::Load(arg(x))); break;
                    STRIDE_K(Op::load32): r[d].i32 =                 I32::Load(arg(x)) ; break;
                #undef STRIDE_1
                #undef STRIDE_K

                    // Ops that don't interact with memory should never care about the stride.
                #define CASE(op) case 2*(int)op: /*fallthrough*/ case 2*(int)op+1
                    CASE(Op::splat): r[d].i32 = z.imm; break;

                    CASE(Op::add_f32): r[d].f32 = r[x].f32 + r[y].f32              ; break;
                    CASE(Op::sub_f32): r[d].f32 = r[x].f32 - r[y].f32              ; break;
                    CASE(Op::mul_f32): r[d].f32 = r[x].f32 * r[y].f32              ; break;
                    CASE(Op::div_f32): r[d].f32 = r[x].f32 / r[y].f32              ; break;
                    CASE(Op::mad_f32): r[d].f32 = r[x].f32 * r[y].f32 + r[z.id].f32; break;

                    CASE(Op::add_i32): r[d].i32 = r[x].i32 + r[y].i32; break;
                    CASE(Op::sub_i32): r[d].i32 = r[x].i32 - r[y].i32; break;
                    CASE(Op::mul_i32): r[d].i32 = r[x].i32 * r[y].i32; break;
                    CASE(Op::div_i32): r[d].i32 = r[x].i32 / r[y].i32; break;
                    CASE(Op::bit_and): r[d].i32 = r[x].i32 & r[y].i32; break;

                    CASE(Op::bit_or ): r[d].i32 = r[x].i32 | r[y].i32; break;
                    CASE(Op::bit_xor): r[d].i32 = r[x].i32 ^ r[y].i32; break;

                    CASE(Op::shl): r[d].i32 =                 r[x].i32 << z.imm ; break;
                    CASE(Op::sra): r[d].i32 =                 r[x].i32 >> z.imm ; break;
                    CASE(Op::shr): r[d].i32 = skvx::cast<int>(r[x].u32 >> z.imm); break;

                    CASE(Op::to_f32): r[d].f32 = skvx::cast<float>(r[x].i32); break;
                    CASE(Op::to_i32): r[d].i32 = skvx::cast<int>  (r[x].f32); break;
                #undef CASE
                }
            }
        }
    }
}
