/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkThreadID.h"
#include "include/private/SkVx.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#include "src/core/SkVM.h"
#include <string.h>
#if defined(SKVM_JIT)
    #include <sys/mman.h>
#endif

namespace skvm {

    Program::~Program() = default;

    Program::Program(Program&& other) {
        fInstructions = std::move(other.fInstructions);
        fRegs = other.fRegs;
        fLoop = other.fLoop;
        // Don't bother trying to move other.fJIT*.  We can just regenerate it.
    }

    Program& Program::operator=(Program&& other) {
        fInstructions = std::move(other.fInstructions);
        fRegs = other.fRegs;
        fLoop = other.fLoop;
        // Don't bother trying to move other.fJIT*.  We can just regenerate it,
        // but we do need to invalidate anything we have cached ourselves.
        fJITLock.acquire();
        fJIT = JIT();
        fJITLock.release();
        return *this;
    }

    Program::Program(std::vector<Instruction> instructions, int regs, int loop)
        : fInstructions(std::move(instructions))
        , fRegs(regs)
        , fLoop(loop) {}


    Program Builder::done() const {
        // Track per-instruction code hoisting, lifetime, and register assignment.
        struct Analysis {
            bool hoist = true;
            Val  life  = NA;
            Reg  reg   = 0;
        };
        std::vector<Analysis> analysis(fProgram.size());

        // Basic liveness analysis (and free dead code elimination).
        for (Val id = fProgram.size(); id --> 0; ) {
            const Instruction& inst = fProgram[id];

            // All side-effect-only instructions (stores) are live.
            if (inst.op <= Op::store32) {
                analysis[id].life = id;
            }
            // The arguments of a live instruction must live until that instruction.
            if (analysis[id].life != NA) {
                // Notice how we're walking backward, storing the latest instruction in life.
                if (inst.x != NA && analysis[inst.x].life == NA) { analysis[inst.x].life = id; }
                if (inst.y != NA && analysis[inst.y].life == NA) { analysis[inst.y].life = id; }
                if (inst.z != NA && analysis[inst.z].life == NA) { analysis[inst.z].life = id; }
            }
        }

        // Look to see if there are any instructions that can be hoisted outside the program's loop.
        for (Val id = 0; id < (Val)fProgram.size(); id++) {
            const Instruction& inst = fProgram[id];

            // Loads and stores cannot be hoisted out of the loop.
            if (inst.op <= Op::load32) {
                analysis[id].hoist = false;
            }

            // If any of an instruction's arguments can't be hoisted, it can't be hoisted itself.
            if (analysis[id].hoist) {
                if (inst.x != NA) { analysis[id].hoist &= analysis[inst.x].hoist; }
                if (inst.y != NA) { analysis[id].hoist &= analysis[inst.y].hoist; }
                if (inst.z != NA) { analysis[id].hoist &= analysis[inst.z].hoist; }
            }

            // Mark the lifetime of live hoisted instructions as the full program,
            // mostly to avoid recycling their registers, and also helps debugging sanity.
            if (analysis[id].hoist && analysis[id].life != NA) {
                analysis[id].life = (Val)fProgram.size();
            }
        }

        // We'll need to map each live value to a register.
        Reg next_reg = 0;

        // Our first pass of register assignment assigns hoisted values to eternal registers.
        for (Val id = 0; id < (Val)fProgram.size(); id++) {
            if (analysis[id].life == NA || !analysis[id].hoist) {
                continue;
            }
            // Hoisted values are needed forever, so they each get their own register.
            analysis[id].reg = next_reg++;
        }

        // Now assign non-hoisted values to registers.
        // When these values are no longer needed we can recycle their registers.
        std::vector<Reg> avail;
        for (Val id = 0; id < (Val)fProgram.size(); id++) {
            const Instruction& inst = fProgram[id];
            if (analysis[id].life == NA || analysis[id].hoist) {
                continue;
            }

            // If an Instruction's input is no longer live, we can recycle the register it occupies.
            auto maybe_recycle_register = [&](Val input) {
                // If this is a real input and it's lifetime ends with this
                // instruction, we can recycle the register it's occupying.
                if (input != NA && analysis[input].life == id) {
                    avail.push_back(analysis[input].reg);
                }
            };

            // Take care not to mark any register available twice, e.g. add(foo,foo).
            if (true                                ) { maybe_recycle_register(inst.x); }
            if (inst.y != inst.x                    ) { maybe_recycle_register(inst.y); }
            if (inst.z != inst.x && inst.z != inst.y) { maybe_recycle_register(inst.z); }

            // Allocate a register if we have to, but prefer to reuse one that's available.
            if (avail.empty()) {
                analysis[id].reg = next_reg++;
            } else {
                analysis[id].reg = avail.back();
                avail.pop_back();
            }
        }

        // Add a dummy mapping for the N/A sentinel value to any arbitrary register
        // so that the lookups don't have to know which arguments are used by which Ops.
        auto lookup_register = [&](Val id) {
            return id == NA ? (Reg)0
                            : analysis[id].reg;
        };

        // Finally translate Builder::Instructions to Program::Instructions by mapping values to
        // registers.  This will be two passes again, first outside the loop, then inside.

        // The loop begins at the loop'th Instruction.
        int loop = 0;
        std::vector<Program::Instruction> program;
        program.reserve(fProgram.size());

        auto push_instruction = [&](Val id, const Builder::Instruction& inst) {
            Program::Instruction pinst{
                inst.op,
                lookup_register(id),
                lookup_register(inst.x),
                lookup_register(inst.y),
               {lookup_register(inst.z)},
            };
            if (inst.z == NA) { pinst.imm = inst.imm; }
            program.push_back(pinst);
        };

        for (Val id = 0; id < (Val)fProgram.size(); id++) {
            const Instruction& inst = fProgram[id];
            if (analysis[id].life == NA || !analysis[id].hoist) {
                continue;
            }

            push_instruction(id, inst);
            loop++;
        }
        for (Val id = 0; id < (Val)fProgram.size(); id++) {
            const Instruction& inst = fProgram[id];
            if (analysis[id].life == NA || analysis[id].hoist) {
                continue;
            }

            push_instruction(id, inst);
        }

        return { std::move(program), /*register count = */next_reg, loop };
    }

    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.

    Val Builder::push(Op op, Val x, Val y, Val z, int imm) {
        Instruction inst{op, x, y, z, imm};

        // Basic common subexpression elimination:
        // if we've already seen this exact Instruction, use it instead of creating a new one.
        if (Val* id = fIndex.find(inst)) {
            return *id;
        }
        Val id = static_cast<Val>(fProgram.size());
        fProgram.push_back(inst);
        fIndex.set(inst, id);
        return id;
    }

    bool Builder::isZero(Val id) const {
        return fProgram[id].op  == Op::splat
            && fProgram[id].imm == 0;
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

    F32 Builder::add(F32 x, F32 y       ) { return {this->push(Op::add_f32, x.id, y.id)}; }
    F32 Builder::sub(F32 x, F32 y       ) { return {this->push(Op::sub_f32, x.id, y.id)}; }
    F32 Builder::mul(F32 x, F32 y       ) { return {this->push(Op::mul_f32, x.id, y.id)}; }
    F32 Builder::div(F32 x, F32 y       ) { return {this->push(Op::div_f32, x.id, y.id)}; }
    F32 Builder::mad(F32 x, F32 y, F32 z) {
        if (this->isZero(z.id)) {
            return this->mul(x,y);
        }
        return {this->push(Op::mad_f32, x.id, y.id, z.id)};
    }

    I32 Builder::add(I32 x, I32 y) { return {this->push(Op::add_i32, x.id, y.id)}; }
    I32 Builder::sub(I32 x, I32 y) { return {this->push(Op::sub_i32, x.id, y.id)}; }
    I32 Builder::mul(I32 x, I32 y) { return {this->push(Op::mul_i32, x.id, y.id)}; }

    I32 Builder::sub_16x2(I32 x, I32 y) { return {this->push(Op::sub_i16x2, x.id, y.id)}; }
    I32 Builder::mul_16x2(I32 x, I32 y) { return {this->push(Op::mul_i16x2, x.id, y.id)}; }
    I32 Builder::shr_16x2(I32 x, int bits) { return {this->push(Op::shr_i16x2, x.id,NA,NA, bits)}; }

    I32 Builder::bit_and  (I32 x, I32 y) { return {this->push(Op::bit_and  , x.id, y.id)}; }
    I32 Builder::bit_or   (I32 x, I32 y) { return {this->push(Op::bit_or   , x.id, y.id)}; }
    I32 Builder::bit_xor  (I32 x, I32 y) { return {this->push(Op::bit_xor  , x.id, y.id)}; }
    I32 Builder::bit_clear(I32 x, I32 y) { return {this->push(Op::bit_clear, x.id, y.id)}; }

    I32 Builder::shl(I32 x, int bits) { return {this->push(Op::shl, x.id,NA,NA, bits)}; }
    I32 Builder::shr(I32 x, int bits) { return {this->push(Op::shr, x.id,NA,NA, bits)}; }
    I32 Builder::sra(I32 x, int bits) { return {this->push(Op::sra, x.id,NA,NA, bits)}; }

    I32 Builder::extract(I32 x, int bits, I32 y) {
        return {this->push(Op::extract, x.id,y.id,NA, bits)};
    }

    I32 Builder::pack(I32 x, I32 y, int bits) {
        return {this->push(Op::pack, x.id,y.id,NA, bits)};
    }

    I32 Builder::bytes(I32 x, int control) {
        return {this->push(Op::bytes, x.id,NA,NA, control)};
    }

    F32 Builder::to_f32(I32 x) { return {this->push(Op::to_f32, x.id)}; }
    I32 Builder::to_i32(F32 x) { return {this->push(Op::to_i32, x.id)}; }

    // ~~~~ Program::dump() and co. ~~~~ //

    struct V { Val id; };
    struct R { Reg id; };
    struct Shift { int bits; };
    struct Splat { int bits; };
    struct Hex   { int bits; };

    static void write(SkWStream* o, const char* s) {
        o->writeText(s);
    }

    static void write(SkWStream* o, Arg a) {
        write(o, "arg(");
        o->writeDecAsText(a.ix);
        write(o, ")");
    }
    static void write(SkWStream* o, V v) {
        write(o, "v");
        o->writeDecAsText(v.id);
    }
    static void write(SkWStream* o, R r) {
        write(o, "r");
        o->writeDecAsText(r.id);
    }
    static void write(SkWStream* o, Shift s) {
        o->writeDecAsText(s.bits);
    }
    static void write(SkWStream* o, Splat s) {
        float f;
        memcpy(&f, &s.bits, 4);
        o->writeHexAsText(s.bits);
        write(o, " (");
        o->writeScalarAsText(f);
        write(o, ")");
    }
    static void write(SkWStream* o, Hex h) {
        o->writeHexAsText(h.bits);
    }

    template <typename T, typename... Ts>
    static void write(SkWStream* o, T first, Ts... rest) {
        write(o, first);
        write(o, " ");
        write(o, rest...);
    }

    void Builder::dump(SkWStream* o) const {
        o->writeDecAsText(fProgram.size());
        o->writeText(" values:\n");
        for (Val id = 0; id < (Val)fProgram.size(); id++) {
            const Instruction& inst = fProgram[id];
            Op  op = inst.op;
            Val  x = inst.x,
                 y = inst.y,
                 z = inst.z;
            int imm = inst.imm;
            switch (op) {
                case Op::store8:  write(o, "store8" , Arg{imm}, V{x}); break;
                case Op::store32: write(o, "store32", Arg{imm}, V{x}); break;

                case Op::load8:  write(o, V{id}, "= load8" , Arg{imm}); break;
                case Op::load32: write(o, V{id}, "= load32", Arg{imm}); break;

                case Op::splat:  write(o, V{id}, "= splat", Splat{imm}); break;

                case Op::add_f32: write(o, V{id}, "= add_f32", V{x}, V{y}      ); break;
                case Op::sub_f32: write(o, V{id}, "= sub_f32", V{x}, V{y}      ); break;
                case Op::mul_f32: write(o, V{id}, "= mul_f32", V{x}, V{y}      ); break;
                case Op::div_f32: write(o, V{id}, "= div_f32", V{x}, V{y}      ); break;
                case Op::mad_f32: write(o, V{id}, "= mad_f32", V{x}, V{y}, V{z}); break;

                case Op::add_i32: write(o, V{id}, "= add_i32", V{x}, V{y}); break;
                case Op::sub_i32: write(o, V{id}, "= sub_i32", V{x}, V{y}); break;
                case Op::mul_i32: write(o, V{id}, "= mul_i32", V{x}, V{y}); break;

                case Op::sub_i16x2: write(o, V{id}, "= sub_i16x2", V{x}, V{y}); break;
                case Op::mul_i16x2: write(o, V{id}, "= mul_i16x2", V{x}, V{y}); break;
                case Op::shr_i16x2: write(o, V{id}, "= shr_i16x2", V{x}, Shift{imm}); break;

                case Op::bit_and  : write(o, V{id}, "= bit_and"  , V{x}, V{y}); break;
                case Op::bit_or   : write(o, V{id}, "= bit_or"   , V{x}, V{y}); break;
                case Op::bit_xor  : write(o, V{id}, "= bit_xor"  , V{x}, V{y}); break;
                case Op::bit_clear: write(o, V{id}, "= bit_clear", V{x}, V{y}); break;

                case Op::shl: write(o, V{id}, "= shl", V{x}, Shift{imm}); break;
                case Op::shr: write(o, V{id}, "= shr", V{x}, Shift{imm}); break;
                case Op::sra: write(o, V{id}, "= sra", V{x}, Shift{imm}); break;

                case Op::extract: write(o, V{id}, "= extract", V{x}, Shift{imm}, V{y}); break;
                case Op::pack:    write(o, V{id}, "= pack",    V{x}, V{y}, Shift{imm}); break;

                case Op::bytes:   write(o, V{id}, "= bytes", V{x}, Hex{imm}); break;

                case Op::to_f32: write(o, V{id}, "= to_f32", V{x}); break;
                case Op::to_i32: write(o, V{id}, "= to_i32", V{x}); break;
            }

            write(o, "\n");
        }
    }

    void Program::dump(SkWStream* o) const {
        o->writeDecAsText(fRegs);
        o->writeText(" registers, ");
        o->writeDecAsText(fInstructions.size());
        o->writeText(" instructions:\n");
        for (int i = 0; i < (int)fInstructions.size(); i++) {
            if (i == fLoop) {
                write(o, "loop:\n");
            }
            const Instruction& inst = fInstructions[i];
            Op   op = inst.op;
            Reg   d = inst.d,
                  x = inst.x,
                  y = inst.y,
                  z = inst.z;
            int imm = inst.imm;
            switch (op) {
                case Op::store8:  write(o, "store8" , Arg{imm}, R{x}); break;
                case Op::store32: write(o, "store32", Arg{imm}, R{x}); break;

                case Op::load8:  write(o, R{d}, "= load8" , Arg{imm}); break;
                case Op::load32: write(o, R{d}, "= load32", Arg{imm}); break;

                case Op::splat:  write(o, R{d}, "= splat", Splat{imm}); break;

                case Op::add_f32: write(o, R{d}, "= add_f32", R{x}, R{y}      ); break;
                case Op::sub_f32: write(o, R{d}, "= sub_f32", R{x}, R{y}      ); break;
                case Op::mul_f32: write(o, R{d}, "= mul_f32", R{x}, R{y}      ); break;
                case Op::div_f32: write(o, R{d}, "= div_f32", R{x}, R{y}      ); break;
                case Op::mad_f32: write(o, R{d}, "= mad_f32", R{x}, R{y}, R{z}); break;

                case Op::add_i32: write(o, R{d}, "= add_i32", R{x}, R{y}); break;
                case Op::sub_i32: write(o, R{d}, "= sub_i32", R{x}, R{y}); break;
                case Op::mul_i32: write(o, R{d}, "= mul_i32", R{x}, R{y}); break;

                case Op::sub_i16x2: write(o, R{d}, "= sub_i16x2", R{x}, R{y}); break;
                case Op::mul_i16x2: write(o, R{d}, "= mul_i16x2", R{x}, R{y}); break;
                case Op::shr_i16x2: write(o, R{d}, "= shr_i16x2", R{x}, Shift{imm}); break;

                case Op::bit_and  : write(o, R{d}, "= bit_and"  , R{x}, R{y}); break;
                case Op::bit_or   : write(o, R{d}, "= bit_or"   , R{x}, R{y}); break;
                case Op::bit_xor  : write(o, R{d}, "= bit_xor"  , R{x}, R{y}); break;
                case Op::bit_clear: write(o, R{d}, "= bit_clear", R{x}, R{y}); break;

                case Op::shl: write(o, R{d}, "= shl", R{x}, Shift{imm}); break;
                case Op::shr: write(o, R{d}, "= shr", R{x}, Shift{imm}); break;
                case Op::sra: write(o, R{d}, "= sra", R{x}, Shift{imm}); break;

                case Op::extract: write(o, R{d}, "= extract", R{x}, Shift{imm}, R{y}); break;
                case Op::pack:    write(o, R{d}, "= pack",    R{x}, R{y}, Shift{imm}); break;

                case Op::bytes: write(o, R{d}, "= bytes", R{x}, Hex{imm}); break;

                case Op::to_f32: write(o, R{d}, "= to_f32", R{x}); break;
                case Op::to_i32: write(o, R{d}, "= to_i32", R{x}); break;
            }
            write(o, "\n");
        }
    }

    // ~~~~ Program::eval() and co. ~~~~ //

    // Handy references for x86-64 instruction encoding:
    // https://wiki.osdev.org/X86-64_Instruction_Encoding
    // https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/x86.chm/x64.htm
    // https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/x86.chm/x86.htm
    // http://ref.x86asm.net/coder64.html

    // Used for ModRM / immediate instruction encoding.
    static uint8_t _233(int a, int b, int c) {
        return (a & 3) << 6
             | (b & 7) << 3
             | (c & 7) << 0;
    }

    // ModRM byte encodes the arguments of an opcode.
    enum class Mod { Indirect, OneByteImm, FourByteImm, Direct };
    static uint8_t mod_rm(Mod mod, int reg, int rm) {
        return _233((int)mod, reg, rm);
    }

#if 0
    // SIB byte encodes a memory address, base + (index * scale).
    enum class Scale { One, Two, Four, Eight };
    static uint8_t sib(Scale scale, int index, int base) {
        return _233((int)scale, index, base);
    }
#endif

    // The REX prefix is used to extend most old 32-bit instructions to 64-bit.
    static uint8_t rex(bool W,   // If set, operation is 64-bit, otherwise default, usually 32-bit.
                       bool R,   // Extra top bit to select ModRM reg, registers 8-15.
                       bool X,   // Extra top bit for SIB index register.
                       bool B) { // Extra top bit for SIB base or ModRM rm register.
        return 0b01000000   // Fixed 0100 for top four bits.
             | (W << 3)
             | (R << 2)
             | (X << 1)
             | (B << 0);
    }


    // The VEX prefix extends SSE operations to AVX.  Used generally, even with XMM.
    struct VEX {
        int     len;
        uint8_t bytes[3];
    };

    static VEX vex(bool  WE,   // Like REX W for int operations, or opcode extension for float?
                   bool   R,   // Same as REX R.  Pass high bit of dst register, dst>>3.
                   bool   X,   // Same as REX X.
                   bool   B,   // Same as REX B.  Pass y>>3 for 3-arg ops, x>>3 for 2-arg.
                   int  map,   // SSE opcode map selector: 0x0f, 0x380f, 0x3a0f.
                   int vvvv,   // 4-bit second operand register.  Pass our x for 3-arg ops.
                   bool   L,   // Set for 256-bit ymm operations, off for 128-bit xmm.
                   int   pp) { // SSE mandatory prefix: 0x66, 0xf3, 0xf2, else none.

        // Pack x86 opcode map selector to 5-bit VEX encoding.
        map = [map]{
            switch (map) {
                case   0x0f: return 0b00001;
                case 0x380f: return 0b00010;
                case 0x3a0f: return 0b00011;
                // Several more cases only used by XOP / TBM.
            }
            SkASSERT(false);
            return 0b00000;
        }();

        // Pack  mandatory SSE opcode prefix byte to 2-bit VEX encoding.
        pp = [pp]{
            switch (pp) {
                case 0x66: return 0b01;
                case 0xf3: return 0b10;
                case 0xf2: return 0b11;
            }
            return 0b00;
        }();

        VEX vex = {0, {0,0,0}};
        if (X == 0 && B == 0 && WE == 0 && map == 0b00001) {
            // With these conditions met, we can optionally compress VEX to 2-byte.
            vex.len = 2;
            vex.bytes[0] = 0xc5;
            vex.bytes[1] = (pp      &  3) << 0
                         | (L       &  1) << 2
                         | (~vvvv   & 15) << 3
                         | (~(int)R &  1) << 7;
        } else {
            // We could use this 3-byte VEX prefix all the time if we like.
            vex.len = 3;
            vex.bytes[0] = 0xc4;
            vex.bytes[1] = (map     & 31) << 0
                         | (~(int)B &  1) << 5
                         | (~(int)X &  1) << 6
                         | (~(int)R &  1) << 7;
            vex.bytes[2] = (pp    &  3) << 0
                         | (L     &  1) << 2
                         | (~vvvv & 15) << 3
                         | (WE    &  1) << 7;
        }
        return vex;
    }

    Assembler::Assembler(void* buf) : fCode((uint8_t*)buf), fSize(0) {}

    size_t Assembler::size() const { return fSize; }

    void Assembler::byte(const void* p, int n) {
        if (fCode) {
            memcpy(fCode, p, n);
            fCode += n;
        }
        fSize += n;
    }

    void Assembler::byte(uint8_t b) { this->byte(&b, 1); }

    template <typename... Rest>
    void Assembler::byte(uint8_t first, Rest... rest) {
        this->byte(first);
        this->byte(rest...);
    }


    void Assembler::nop() { this->byte(0x90); }
    void Assembler::align(int mod) {
        while (this->size() % mod) {
            this->nop();
        }
    }

    void Assembler::vzeroupper() { this->byte(0xc5, 0xf8, 0x77); }
    void Assembler::ret() { this->byte(0xc3); }

    // Common instruction building for 64-bit opcodes with an immediate argument.
    void Assembler::op(int opcode, int opcode_ext, GP64 dst, int imm) {
        opcode |= 0b0000'0001;   // low bit set for 64-bit operands
        opcode |= 0b1000'0000;   // top bit set for instructions with any immediate

        int imm_bytes = 4;
        if (SkTFitsIn<int8_t>(imm)) {
            imm_bytes = 1;
            opcode |= 0b0000'0010;  // second bit set for 8-bit immediate, else 32-bit.
        }

        this->byte(rex(1,dst>>3,0,0));
        this->byte(opcode);
        this->byte(mod_rm(Mod::Direct, opcode_ext, dst&7));
        this->byte(&imm, imm_bytes);
    }

    void Assembler::add(GP64 dst, int imm) { this->op(0,0b000, dst,imm); }
    void Assembler::sub(GP64 dst, int imm) { this->op(0,0b101, dst,imm); }

    void Assembler::op(int prefix, int map, int opcode, Ymm dst, Ymm x, Ymm y, bool W/*=false*/) {
        VEX v = vex(W, dst>>3, 0, y>>3,
                    map, x, 1/*ymm, not xmm*/, prefix);
        this->byte(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Direct, dst&7, y&7));
    }

    void Assembler::vpaddd (Ymm dst, Ymm x, Ymm y) { this->op(0x66,  0x0f,0xfe, dst,x,y); }
    void Assembler::vpsubd (Ymm dst, Ymm x, Ymm y) { this->op(0x66,  0x0f,0xfa, dst,x,y); }
    void Assembler::vpmulld(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0x40, dst,x,y); }

    void Assembler::vpsubw (Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xf9, dst,x,y); }
    void Assembler::vpmullw(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xd5, dst,x,y); }

    void Assembler::vpand (Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xdb, dst,x,y); }
    void Assembler::vpor  (Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xeb, dst,x,y); }
    void Assembler::vpxor (Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xef, dst,x,y); }
    void Assembler::vpandn(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xdf, dst,x,y); }

    void Assembler::vaddps(Ymm dst, Ymm x, Ymm y) { this->op(0,0x0f,0x58, dst,x,y); }
    void Assembler::vsubps(Ymm dst, Ymm x, Ymm y) { this->op(0,0x0f,0x5c, dst,x,y); }
    void Assembler::vmulps(Ymm dst, Ymm x, Ymm y) { this->op(0,0x0f,0x59, dst,x,y); }
    void Assembler::vdivps(Ymm dst, Ymm x, Ymm y) { this->op(0,0x0f,0x5e, dst,x,y); }

    void Assembler::vfmadd132ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0x98, dst,x,y); }
    void Assembler::vfmadd213ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xa8, dst,x,y); }
    void Assembler::vfmadd231ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xb8, dst,x,y); }

    void Assembler::vpackusdw(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0x2b, dst,x,y); }
    void Assembler::vpackuswb(Ymm dst, Ymm x, Ymm y) { this->op(0x66,  0x0f,0x67, dst,x,y); }

    // dst = x op /opcode_ext imm
    void Assembler::op(int prefix, int map, int opcode, int opcode_ext, Ymm dst, Ymm x, int imm) {
        // This is a little weird, but if we pass the opcode_ext as if it were the dst register,
        // the dst register as if x, and the x register as if y, all the bits end up where we want.
        this->op(prefix, map, opcode, (Ymm)opcode_ext,dst,x);
        this->byte(imm);
    }

    void Assembler::vpslld(Ymm dst, Ymm x, int imm) { this->op(0x66,0x0f,0x72,6, dst,x,imm); }
    void Assembler::vpsrld(Ymm dst, Ymm x, int imm) { this->op(0x66,0x0f,0x72,2, dst,x,imm); }
    void Assembler::vpsrad(Ymm dst, Ymm x, int imm) { this->op(0x66,0x0f,0x72,4, dst,x,imm); }

    void Assembler::vpsrlw(Ymm dst, Ymm x, int imm) { this->op(0x66,0x0f,0x71,2, dst,x,imm); }


    void Assembler::vpermq(Ymm dst, Ymm x, int imm) {
        // A bit unusual among the instructions we use, this is 64-bit operation, so we set W.
        bool W = true;
        this->op(0x66,0x3a0f,0x00, dst,x,W);
        this->byte(imm);
    }

    void Assembler::vcvtdq2ps (Ymm dst, Ymm x) { this->op(0,   0x0f,0x5b, dst,x); }
    void Assembler::vcvttps2dq(Ymm dst, Ymm x) { this->op(0xf3,0x0f,0x5b, dst,x); }

    Assembler::Label Assembler::here() {
        return { (int)this->size() };
    }

    void Assembler::op(int prefix, int map, int opcode, Ymm dst, Ymm x, Label l) {
        // IP-relative addressing uses Mod::Indirect with the R/M encoded as-if rbp or r13.
        const int rip = rbp;

        VEX v = vex(0, dst>>3, 0, rip>>3,
                    map, x, /*ymm?*/1, prefix);
        this->byte(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, dst&7, rip&7));

        // IP relative addresses are relative to IP _after_ this instruction.
        int imm = l.offset - (here().offset + 4);
        this->byte(&imm, 4);
    }

    void Assembler::vbroadcastss(Ymm dst, Label l) { this->op(0x66,0x380f,0x18, dst,l); }

    void Assembler::vpshufb(Ymm dst, Ymm x, Label l) { this->op(0x66,0x380f,0x00, dst,x,l); }

    void Assembler::jne(Label l) {
        // jne can be either 2 bytes (short) or 6 bytes (near):
        //    75     one-byte-disp
        //    0F 85 four-byte-disp
        // As usual, all displacements relative to the end of this instruction.
        int shrt = l.offset - (here().offset + 2),
            near = l.offset - (here().offset + 6);

        if (SkTFitsIn<int8_t>(shrt)) {
            this->byte(0x75);
            this->byte(&shrt, 1);
        } else {
            this->byte(0x0f, 0x85);
            this->byte(&near, 4);
        }
    }

    void Assembler::load_store(int prefix, int map, int opcode, Ymm ymm, GP64 ptr) {
        VEX v = vex(0, ymm>>3, 0, ptr>>3,
                    map, 0, /*ymm?*/1, prefix);
        this->byte(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, ymm&7, ptr&7));
    }

    void Assembler::vmovups  (Ymm dst, GP64 src) { this->load_store(0   ,  0x0f,0x10, dst,src); }
    void Assembler::vpmovzxbd(Ymm dst, GP64 src) { this->load_store(0x66,0x380f,0x31, dst,src); }
    void Assembler::vmovups  (GP64 dst, Ymm src) { this->load_store(0   ,  0x0f,0x11, src,dst); }
    void Assembler::vmovq    (GP64 dst, Xmm src) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0xd6;
        VEX v = vex(0, src>>3, 0, dst>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->byte(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, src&7, dst&7));
    }

    void Assembler::word(uint32_t w) {
        this->byte(&w, 4);
    }

    // https://static.docs.arm.com/ddi0596/a/DDI_0596_ARM_a64_instruction_set_architecture.pdf

    static int operator"" _mask(unsigned long long bits) { return (1<<(int)bits)-1; }

    void Assembler::op(uint32_t hi, V m, uint32_t lo, V n, V d) {
        this->word( (hi & 11_mask) << 21
                  | (m  &  5_mask) << 16
                  | (lo &  6_mask) << 10
                  | (n  &  5_mask) <<  5
                  | (d  &  5_mask) <<  0);
    }

    void Assembler::and16b(V d, V n, V m) { this->op(0b0'1'0'01110'00'1, m, 0b00011'1, n, d); }
    void Assembler::orr16b(V d, V n, V m) { this->op(0b0'1'0'01110'10'1, m, 0b00011'1, n, d); }
    void Assembler::eor16b(V d, V n, V m) { this->op(0b0'1'1'01110'00'1, m, 0b00011'1, n, d); }
    void Assembler::bic16b(V d, V n, V m) { this->op(0b0'1'0'01110'01'1, m, 0b00011'1, n, d); }

    void Assembler::add4s(V d, V n, V m) { this->op(0b0'1'0'01110'10'1, m, 0b10000'1, n, d); }
    void Assembler::sub4s(V d, V n, V m) { this->op(0b0'1'1'01110'10'1, m, 0b10000'1, n, d); }
    void Assembler::mul4s(V d, V n, V m) { this->op(0b0'1'0'01110'10'1, m, 0b10011'1, n, d); }

    void Assembler::sub8h(V d, V n, V m) { this->op(0b0'1'1'01110'01'1, m, 0b10000'1, n, d); }
    void Assembler::mul8h(V d, V n, V m) { this->op(0b0'1'0'01110'01'1, m, 0b10011'1, n, d); }

    void Assembler::fadd4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b11010'1, n, d); }
    void Assembler::fsub4s(V d, V n, V m) { this->op(0b0'1'0'01110'1'0'1, m, 0b11010'1, n, d); }
    void Assembler::fmul4s(V d, V n, V m) { this->op(0b0'1'1'01110'0'0'1, m, 0b11011'1, n, d); }
    void Assembler::fdiv4s(V d, V n, V m) { this->op(0b0'1'1'01110'0'0'1, m, 0b11111'1, n, d); }

    void Assembler::fmla4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b11001'1, n, d); }

    void Assembler::shift(uint32_t op, int imm, V n, V d) {
        this->word( (op & 22_mask) << 10
                  | imm            << 16   // imm is embedded inside op, bit size depends on op
                  | (n &   5_mask) <<  5
                  | (d &   5_mask) <<  0);
    }

    void Assembler::shl4s(V d, V n, int imm) {
        this->shift(0b0'1'0'011110'0100'000'01010'1,    ( imm&31), n, d);
    }
    void Assembler::sshr4s(V d, V n, int imm) {
        this->shift(0b0'1'0'011110'0100'000'00'0'0'0'1, (-imm&31), n, d);
    }
    void Assembler::ushr4s(V d, V n, int imm) {
        this->shift(0b0'1'1'011110'0100'000'00'0'0'0'1, (-imm&31), n, d);
    }
    void Assembler::ushr8h(V d, V n, int imm) {
        this->shift(0b0'1'1'011110'0010'000'00'0'0'0'1, (-imm&15), n, d);
    }

    void Assembler::scvtf4s(V d, V n) {
        this->word(0b0'1'0'01110'0'0'10000'11101'10 << 10
                  | (n & 5_mask) << 5
                  | (d & 5_mask) << 0);
    }
    void Assembler::fcvtzs4s(V d, V n) {
        this->word(0b0'1'0'01110'1'0'10000'1101'1'10 << 10
                  | (n & 5_mask) << 5
                  | (d & 5_mask) << 0);
    }

    void Assembler::ret(X n) {
        this->word(0b1101011'0'0'10'11111'0000'0'0 << 10
                  | (n & 5_mask) << 5);
    }

    void Assembler::add(X d, X n, int imm12) {
        this->word(0b1'0'0'10001'00   << 22
                  | (imm12 & 12_mask) << 10
                  | (n     &  5_mask) <<  5
                  | (d     &  5_mask) <<  0);
    }
    void Assembler::subs(X d, X n, int imm12) {
        this->word( 0b1'1'1'10001'00     << 22
                  | (imm12 & 12_mask) << 10
                  | (n     &  5_mask) <<  5
                  | (d     &  5_mask) <<  0);
    }

    void Assembler::bne(Label l) {
        // Jump in insts from before this one.
        const int imm19 = (l.offset - here().offset) / 4;
        this->word( 0b0101010'0       << 24
                  | (imm19 & 19_mask) <<  5
                  | 0b0'0001          <<  0);
    }

    void Assembler::ldrq(V dst, X src) {
        this->word( 0b00'111'1'01'11'000000000000 << 10
                  | (src & 5_mask) << 5
                  | (dst & 5_mask) << 0);
    }

    void Assembler::strq(V src, X dst) {
        this->word( 0b00'111'1'01'10'000000000000 << 10
                  | (dst & 5_mask) << 5
                  | (src & 5_mask) << 0);
    }

#if defined(SKVM_JIT)
    static bool can_jit(int regs, int nargs) {
        return true
            && SkCpu::Supports(SkCpu::HSW)   // TODO: SSE4.1 target?
            && regs  <= 15   // All 16 ymm registers, reserving one for us as tmp.
            && nargs <=  5;  // We can increase this if we push/pop GP registers.
    }

    // Returns stride of the JIT, currently always 8.
    static int jit(Assembler& a, size_t* code,
                   const std::vector<Program::Instruction>& instructions,
                   int regs, int loop, size_t strides[], int nargs) {
        using A = Assembler;

        SkASSERT(can_jit(regs,nargs));

        static constexpr int K = 8;

    #if defined(SK_BUILD_FOR_WIN)
        // TODO  Windows ABI?
    #else
        // These registers are used to pass the first 6 arguments,
        // so if we stick to these we need not push, pop, spill, or move anything around.
        A::GP64 N = A::rdi,
            arg[] = { A::rsi, A::rdx, A::rcx, A::r8, A::r9 };

        // All 16 ymm registers are available as scratch, keeping 15 as a temporary for us.
        auto r = [](Reg ix) { SkASSERT(ix < 16); return (A::Ymm)ix; };
        const int tmp = 15;
    #endif

        // We'll lay out our function as:
        //   - 32-byte aligned data (from Op::bytes)
        //   -  4-byte aligned data (from Op::splat)
        //   -    byte aligned code
        // This makes the code as compact as possible, requiring no alignment padding.
        // It also makes working with labels easy, as they'll all be resolved before
        // the instructions that use them... no relocations.

        // Map from our bytes() control imm to 32-byte mask for vpshufb.
        SkTHashMap<int, A::Label> vpshufb_masks;
        for (const Program::Instruction& inst : instructions) {
            if (inst.op == Op::bytes && vpshufb_masks.find(inst.imm) == nullptr) {
                // Translate bytes()'s control nibbles to vpshufb's control bytes.
                auto nibble_to_vpshufb = [](unsigned n) -> uint8_t {
                    return n == 0 ? 0xff  // Fill with zero.
                                  : n-1;  // Select n'th 1-indexed byte.
                };
                uint8_t control[] = {
                    nibble_to_vpshufb( (inst.imm >>  0) & 0xf ),
                    nibble_to_vpshufb( (inst.imm >>  4) & 0xf ),
                    nibble_to_vpshufb( (inst.imm >>  8) & 0xf ),
                    nibble_to_vpshufb( (inst.imm >> 12) & 0xf ),
                };

                // Now, vpshufb is one of those weird AVX instructions
                // that does everything in 2 128-bit chunks, so we'll
                // only really need 4 distinct values to write in our pattern:
                int p[4];
                for (int i = 0; i < 4; i++) {
                    p[i] = (int)control[0] <<  0
                        | (int)control[1] <<  8
                        | (int)control[2] << 16
                        | (int)control[3] << 24;

                    // Update each byte that refers to a byte index by 4 to
                    // point into the next 32-bit lane, but leave any 0xff
                    // that fills with zero alone.
                    control[0] += control[0] == 0xff ? 0 : 4;
                    control[1] += control[1] == 0xff ? 0 : 4;
                    control[2] += control[2] == 0xff ? 0 : 4;
                    control[3] += control[3] == 0xff ? 0 : 4;
                }

                // Notice, same pattern for top 4 32-bit lanes as bottom 4 lanes.
                SkASSERT(a.size() % 32 == 0);
                A::Label label = a.here();
                a.byte(p, sizeof(p));
                a.byte(p, sizeof(p));
                vpshufb_masks.set(inst.imm, label);
            }

        }

        // Map from splat bit pattern to 4-byte aligned data location holding that pattern.
        // (If we were really brave we could just point at the copy we already have in Program...)
        SkTHashMap<int, A::Label> splats;
        for (const Program::Instruction& inst : instructions) {
            if (inst.op == Op::splat) {
                // Splats are deduplicated at an earlier layer, so we shouldn't find any duplicates.
                // (It really wouldn't be that big a deal if we did, but they'd be assigned distinct
                // registers redundantly, so that's something we'd like to know about.)
                //
                // TODO: in an AVX-512 world, it makes less sense to assign splats to registers at
                // all.  Perhaps we should move the deduping / register coloring for splats here?
                SkASSERT(splats.find(inst.imm) == nullptr);

                SkASSERT(a.size() % 4 == 0);
                A::Label label = a.here();
                a.byte(&inst.imm, 4);
                splats.set(inst.imm, label);
            }
        }

        // Executable code starts here.
        *code = a.size();

        A::Label loop_label;
        for (int i = 0; i < (int)instructions.size(); i++) {
            if (i == loop) {
                loop_label = a.here();
            }
            const Program::Instruction& inst = instructions[i];
            Op  op = inst.op;

            Reg   d = inst.d,
                  x = inst.x,
                  y = inst.y,
                  z = inst.z;
            int imm = inst.imm;
            switch (op) {
                // Ops producing multiple AVX instructions should always
                // use tmp as the result of all but the final instruction
                // to avoid any possible dst/arg aliasing.  You don't want
                // to overwrite your arguments before you're done using them!

                case Op::store8:
                    // TODO: if SkCpu::Supports(SkCpu::SKX) { a.vpmovusdb(arg[imm], ar(x)) }
                    a.vpackusdw(r(tmp), r(x), r(x));      // pack 32-bit -> 16-bit
                    a.vpermq   (r(tmp), r(tmp), 0xd8);     // u64 tmp[0,1,2,3] = tmp[0,2,1,3]
                    a.vpackuswb(r(tmp), r(tmp), r(tmp));  // pack 16-bit -> 8-bit
                    a.vmovq    (arg[imm], (A::Xmm)tmp);    // store low 8 bytes
                    break;

                case Op::store32: a.vmovups(arg[imm], r(x)); break;

                case Op::load8:  a.vpmovzxbd(r(d), arg[imm]); break;
                case Op::load32: a.vmovups  (r(d), arg[imm]); break;

                case Op::splat: a.vbroadcastss(r(d), *splats.find(imm)); break;

                case Op::add_f32: a.vaddps(r(d), r(x), r(y)); break;
                case Op::sub_f32: a.vsubps(r(d), r(x), r(y)); break;
                case Op::mul_f32: a.vmulps(r(d), r(x), r(y)); break;
                case Op::div_f32: a.vdivps(r(d), r(x), r(y)); break;
                case Op::mad_f32:
                    if (d == x) { a.vfmadd132ps(r(x), r(z), r(y)); } else
                    if (d == y) { a.vfmadd213ps(r(y), r(x), r(z)); } else
                    if (d == z) { a.vfmadd231ps(r(z), r(x), r(y)); } else
                                { a.vmulps(r(tmp), r(x), r(y));
                                  a.vaddps(r(d), r(tmp), r(z)); }
                    break;

                case Op::add_i32: a.vpaddd (r(d), r(x), r(y)); break;
                case Op::sub_i32: a.vpsubd (r(d), r(x), r(y)); break;
                case Op::mul_i32: a.vpmulld(r(d), r(x), r(y)); break;

                case Op::sub_i16x2: a.vpsubw (r(d), r(x), r(y)); break;
                case Op::mul_i16x2: a.vpmullw(r(d), r(x), r(y)); break;
                case Op::shr_i16x2: a.vpsrlw (r(d), r(x),  imm); break;

                case Op::bit_and  : a.vpand (r(d), r(x), r(y)); break;
                case Op::bit_or   : a.vpor  (r(d), r(x), r(y)); break;
                case Op::bit_xor  : a.vpxor (r(d), r(x), r(y)); break;
                case Op::bit_clear: a.vpandn(r(d), r(y), r(x)); break;  // N.B. passing y then x.

                case Op::shl: a.vpslld(r(d), r(x), imm); break;
                case Op::shr: a.vpsrld(r(d), r(x), imm); break;
                case Op::sra: a.vpsrad(r(d), r(x), imm); break;

                case Op::extract: if (imm) {
                                      a.vpsrld(r(tmp), r(x), imm);
                                      a.vpand (r(d), r(tmp), r(y));
                                  } else {
                                      a.vpand (r(d), r(x), r(y));
                                  }
                                  break;

                case Op::pack: a.vpslld(r(tmp), r(y), imm);
                               a.vpor  (r(d), r(tmp), r(x));
                               break;

                case Op::to_f32: a.vcvtdq2ps (r(d), r(x)); break;
                case Op::to_i32: a.vcvttps2dq(r(d), r(x)); break;

                case Op::bytes: a.vpshufb(r(d), r(x), *vpshufb_masks.find(imm)); break;
            }
        }

        for (int i = 0; i < nargs; i++) {
            a.add(arg[i], K*(int)strides[i]);
        }
        a.sub(N, K);
        a.jne(loop_label);

        a.vzeroupper();
        a.ret();

        // Return mask to apply to N for elements the JIT can handle.
        return ~(K-1);
    }

    Program::JIT::~JIT() {
        if (buf) {
            munmap(buf,size);
        }
    }
#else
    Program::JIT::~JIT() { SkASSERT(buf == nullptr); }
#endif // defined(SKVM_JIT)

    void Program::eval(int n, void* args[], size_t strides[], int nargs) const {
        void (*entry)() = nullptr;
        int    mask     = 0;

    #if defined(SKVM_JIT)
        // If we can't grab this lock, another thread is probably assembling the program.
        // We can just fall through to the interpreter.
        if (fJITLock.tryAcquire()) {
            if (fJIT.entry) {
                // Use cached program.
                entry = fJIT.entry;
                mask  = fJIT.mask;
            } else if (can_jit(fRegs, nargs)) {
                // First assemble without any buffer to see how much memory we need to mmap.
                size_t code;
                Assembler a{nullptr};
                mask = jit(a, &code, fInstructions, fRegs, fLoop, strides, nargs);

                // mprotect() can only change at a page level granularity, so round a.size() up.
                size_t page = sysconf(_SC_PAGESIZE),                           // Probably 4096.
                       size = ((a.size() + page - 1) / page) * page;

                void* buf =
                    mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1,0);

                a = Assembler{buf};
                mask = jit(a,&code, fInstructions, fRegs, fLoop, strides, nargs);

                mprotect(buf,size, PROT_READ|PROT_EXEC);

                entry = (decltype(entry))( (const uint8_t*)buf + code );

                fJIT.buf   = buf;
                fJIT.size  = size;
                fJIT.entry = entry;
                fJIT.mask  = mask;

            #if defined(SKVM_PERF_DUMPS) // Debug dumps for perf.
                // We're doing some really stateful things below so one thread at a time please...
                static SkSpinlock dump_lock;
                SkAutoSpinlock lock(dump_lock);

                uint32_t hash = SkOpts::hash(fJIT.buf, fJIT.size);
                SkString name = SkStringPrintf("skvm-jit-%u", hash);

                // Create a jit-<pid>.dump file that we can `perf inject -j` into a
                // perf.data captured with `perf record -k 1`, letting us see each
                // JIT'd Program as if a function named skvm-jit-<hash>.   E.g.
                //
                //   ninja -C out nanobench
                //   perf record -k 1 out/nanobench -m SkVM_4096_I32\$
                //   perf inject -j -i perf.data -o perf.data.jit
                //   perf report -i perf.data.jit
                //
                // Running `perf inject -j` will also dump an .so for each JIT'd
                // program, named jitted-<pid>-<hash>.so.

                auto timestamp_ns = []() -> uint64_t {
                    // It's important to use CLOCK_MONOTONIC here so that perf can
                    // correlate our timestamps with those captured by `perf record
                    // -k 1`.  That's also what `-k 1` does, by the way, tell perf
                    // record to use CLOCK_MONOTONIC.
                    struct timespec ts;
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    return ts.tv_sec * (uint64_t)1e9 + ts.tv_nsec;
                };

                // We'll open the jit-<pid>.dump file and write a small header once,
                // and just leave it open forever because we're lazy.
                static FILE* jitdump = [&]{
                    // Must map as w+ for the mmap() call below to work.
                    FILE* f = fopen(SkStringPrintf("jit-%d.dump", getpid()).c_str(), "w+");

                    // Calling mmap() on the file adds a "hey they mmap()'d this" record to
                    // the perf.data file that will point `perf inject -j` at this log file.
                    // Kind of a strange way to tell `perf inject` where the file is...
                    void* marker = mmap(nullptr,
                                        sysconf(_SC_PAGESIZE),
                                        PROT_READ|PROT_EXEC,
                                        MAP_PRIVATE,
                                        fileno(f),
                                        /*offset=*/0);
                    SkASSERT_RELEASE(marker != MAP_FAILED);
                    // Like never calling fclose(f), we'll also just always leave marker mmap()'d.

                    struct Header {
                        uint32_t magic, version, header_size, elf_mach, reserved, pid;
                        uint64_t timestamp_us, flags;
                    } header = {
                        0x4A695444, 1, sizeof(Header), 62/*x86-64*/, 0, (uint32_t)getpid(),
                        timestamp_ns() / 1000, 0,
                    };
                    fwrite(&header, sizeof(header), 1, f);

                    return f;
                }();

                struct CodeLoad {
                    uint32_t event_type, event_size;
                    uint64_t timestamp_ns;

                    uint32_t pid, tid;
                    uint64_t vma/*???*/, code_addr, code_size, id;
                } load = {
                    0/*code load*/, (uint32_t)(sizeof(CodeLoad) + name.size() + 1 + fJIT.size),
                    timestamp_ns(),

                    (uint32_t)getpid(), (uint32_t)SkGetThreadID(),
                    (uint64_t)fJIT.buf, (uint64_t)fJIT.buf, fJIT.size, hash,
                };

                // Write the header, the JIT'd function name, and the JIT'd code itself.
                fwrite(&load, sizeof(load), 1, jitdump);
                fwrite(name.c_str(), 1, name.size(), jitdump);
                fwrite("\0", 1, 1, jitdump);
                fwrite(fJIT.buf, 1, fJIT.size, jitdump);
            #endif
            }
            fJITLock.release();   // pairs with tryAcquire() in the if().
        }
    #endif  // defined(SKVM_JIT)

        if (const int jitN = n & mask) {
            SkASSERT(entry);
            bool ran = true;

            switch (nargs) {
                case 0: ((void(*)(int              ))entry)(jitN                  ); break;
                case 1: ((void(*)(int, void*       ))entry)(jitN, args[0]         ); break;
                case 2: ((void(*)(int, void*, void*))entry)(jitN, args[0], args[1]); break;
                default: ran = false; break;
            }
            if (ran) {
                // Step n and arguments forward to where the JIT stopped.
                n -= jitN;

                void**        arg    = args;
                const size_t* stride = strides;
                for (; *arg; arg++, stride++) {
                    *arg = (void*)( (char*)*arg + jitN * *stride );
                }
                SkASSERT(arg == args + nargs);
            }
        }
        if (n) {
            SkOpts::eval(fInstructions.data(), (int)fInstructions.size(), fRegs, fLoop,
                         n, args, strides, nargs);
        }
    }
}

// TODO: argument strides (more generally types) should come earlier, the pointers themselves later.
