/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSpinlock.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkThreadID.h"
#include "include/private/SkVx.h"
#include "src/core/SkCpu.h"
#include "src/core/SkVM.h"
#include <string.h>
#if defined(SKVM_JIT)
    #include <sys/mman.h>
#endif

namespace skvm {

    Program Builder::done(const char* debug_name) {
        // Basic liveness analysis:
        // an instruction is live until all live instructions that need its input have retired.
        for (Val id = fProgram.size(); id --> 0; ) {
            Instruction& inst = fProgram[id];
            // All side-effect-only instructions (stores) are live.
            if (inst.op <= Op::store32) {
                inst.death = id;
            }
            // The arguments of a live instruction must live until at least that instruction.
            if (inst.death != 0) {
                // Notice how we're walking backward, storing the latest instruction in death.
                if (inst.x != NA && fProgram[inst.x].death == 0) { fProgram[inst.x].death = id; }
                if (inst.y != NA && fProgram[inst.y].death == 0) { fProgram[inst.y].death = id; }
                if (inst.z != NA && fProgram[inst.z].death == 0) { fProgram[inst.z].death = id; }
            }
        }

        return {fProgram, fStrides, debug_name};
    }

    static bool operator==(const Builder::Instruction& a, const Builder::Instruction& b) {
        return a.op    == b.op
            && a.x     == b.x
            && a.y     == b.y
            && a.z     == b.z
            && a.imm   == b.imm
            && a.death == b.death;
    }

    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.
    Val Builder::push(Op op, Val x, Val y, Val z, int imm) {
        Instruction inst{op, x, y, z, imm, /*death=*/0};

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

    Arg Builder::arg(int stride) {
        int ix = (int)fStrides.size();
        fStrides.push_back(stride);
        return {ix};
    }

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

    Assembler::Assembler(void* buf) : fCode((uint8_t*)buf), fCurr(fCode), fSize(0) {}

    size_t Assembler::size() const { return fSize; }

    void Assembler::bytes(const void* p, int n) {
        if (fCurr) {
            memcpy(fCurr, p, n);
            fCurr += n;
        }
        fSize += n;
    }

    void Assembler::byte(uint8_t b) { this->bytes(&b, 1); }
    void Assembler::word(uint32_t w) { this->bytes(&w, 4); }

    void Assembler::align(int mod) {
        while (this->size() % mod) {
            this->byte(0x00);
        }
    }

    void Assembler::vzeroupper() {
        this->byte(0xc5);
        this->byte(0xf8);
        this->byte(0x77);
    }
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

        this->byte(rex(1,0,0,dst>>3));
        this->byte(opcode);
        this->byte(mod_rm(Mod::Direct, opcode_ext, dst&7));
        this->bytes(&imm, imm_bytes);
    }

    void Assembler::add(GP64 dst, int imm) { this->op(0,0b000, dst,imm); }
    void Assembler::sub(GP64 dst, int imm) { this->op(0,0b101, dst,imm); }
    void Assembler::cmp(GP64 reg, int imm) { this->op(0,0b111, reg,imm); }

    void Assembler::op(int prefix, int map, int opcode, Ymm dst, Ymm x, Ymm y, bool W/*=false*/) {
        VEX v = vex(W, dst>>3, 0, y>>3,
                    map, x, 1/*ymm, not xmm*/, prefix);
        this->bytes(v.bytes, v.len);
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
        return { (int)this->size(), Label::None, {} };
    }

    int Assembler::disp19(Label* l) {
        SkASSERT(l->kind == Label::None ||
                 l->kind == Label::ARMDisp19);
        l->kind = Label::ARMDisp19;
        l->references.push_back(here().offset);
        // ARM 19-bit instruction count, from the beginning of this instruction.
        return (l->offset - here().offset) / 4;
    }

    int Assembler::disp32(Label* l) {
        SkASSERT(l->kind == Label::None ||
                 l->kind == Label::X86Disp32);
        l->kind = Label::X86Disp32;
        l->references.push_back(here().offset);
        // x86 32-bit byte count, from the end of this instruction.
        return l->offset - (here().offset + 4);
    }

    void Assembler::op(int prefix, int map, int opcode, Ymm dst, Ymm x, Label* l) {
        // IP-relative addressing uses Mod::Indirect with the R/M encoded as-if rbp or r13.
        const int rip = rbp;

        VEX v = vex(0, dst>>3, 0, rip>>3,
                    map, x, /*ymm?*/1, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, dst&7, rip&7));
        this->word(this->disp32(l));
    }

    void Assembler::vbroadcastss(Ymm dst, Label* l) { this->op(0x66,0x380f,0x18, dst,l); }

    void Assembler::vpshufb(Ymm dst, Ymm x, Label* l) { this->op(0x66,0x380f,0x00, dst,x,l); }

    void Assembler::jump(uint8_t condition, Label* l) {
        // These conditional jumps can be either 2 bytes (short) or 6 bytes (near):
        //    7?     one-byte-disp
        //    0F 8? four-byte-disp
        // We always use the near displacement to make updating labels simpler (no resizing).
        this->byte(0x0f);
        this->byte(condition);
        this->word(this->disp32(l));
    }
    void Assembler::je (Label* l) { this->jump(0x84, l); }
    void Assembler::jne(Label* l) { this->jump(0x85, l); }
    void Assembler::jl (Label* l) { this->jump(0x8c, l); }

    void Assembler::jmp(Label* l) {
        // Like above in jump(), we could use 8-bit displacement here, but always use 32-bit.
        this->byte(0xe9);
        this->word(this->disp32(l));
    }

    void Assembler::load_store(int prefix, int map, int opcode, Ymm ymm, GP64 ptr) {
        VEX v = vex(0, ymm>>3, 0, ptr>>3,
                    map, 0, /*ymm?*/1, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, ymm&7, ptr&7));
    }

    void Assembler::vmovups  (Ymm dst, GP64 src) { this->load_store(0   ,  0x0f,0x10, dst,src); }
    void Assembler::vpmovzxbd(Ymm dst, GP64 src) { this->load_store(0x66,0x380f,0x31, dst,src); }
    void Assembler::vmovups  (GP64 dst, Ymm src) { this->load_store(0   ,  0x0f,0x11, src,dst); }

    void Assembler::vmovq(GP64 dst, Xmm src) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0xd6;
        VEX v = vex(0, src>>3, 0, dst>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, src&7, dst&7));
    }

    void Assembler::vmovd(GP64 dst, Xmm src) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0x7e;
        VEX v = vex(0, src>>3, 0, dst>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, src&7, dst&7));
    }

    void Assembler::vmovd_direct(GP64 dst, Xmm src) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0x7e;
        VEX v = vex(0, src>>3, 0, dst>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Direct, src&7, dst&7));
    }

    void Assembler::vmovd(Xmm dst, GP64 src) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0x6e;
        VEX v = vex(0, dst>>3, 0, src>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, dst&7, src&7));
    }

    void Assembler::vmovd_direct(Xmm dst, GP64 src) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0x6e;
        VEX v = vex(0, dst>>3, 0, src>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Direct, dst&7, src&7));
    }

    void Assembler::movzbl(GP64 dst, GP64 src) {
        if ((dst>>3) || (src>>3)) {
            this->byte(rex(0,dst>>3,0,src>>3));
        }
        this->byte(0x0f);
        this->byte(0xb6);
        this->byte(mod_rm(Mod::Indirect, dst&7, src&7));
    }


    void Assembler::movb(GP64 dst, GP64 src) {
        if ((dst>>3) || (src>>3)) {
            this->byte(rex(0,src>>3,0,dst>>3));
        }
        this->byte(0x88);
        this->byte(mod_rm(Mod::Indirect, src&7, dst&7));
    }

    void Assembler::vpinsrb(Xmm dst, Xmm src, GP64 ptr, int imm) {
        int prefix = 0x66,
            map    = 0x3a0f,
            opcode = 0x20;
        VEX v = vex(0, dst>>3, 0, ptr>>3,
                    map, src, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, dst&7, ptr&7));
        this->byte(imm);
    }

    void Assembler::vpextrb(GP64 ptr, Xmm src, int imm) {
        int prefix = 0x66,
            map    = 0x3a0f,
            opcode = 0x14;

        VEX v = vex(0, src>>3, 0, ptr>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, src&7, ptr&7));
        this->byte(imm);
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

    void Assembler::tbl(V d, V n, V m) { this->op(0b0'1'001110'00'0, m, 0b0'00'0'00, n, d); }

    void Assembler::op(uint32_t op22, int imm, V n, V d) {
        this->word( (op22 & 22_mask) << 10
                  | imm              << 16   // imm is embedded inside op, bit size depends on op
                  | (n    &  5_mask) <<  5
                  | (d    &  5_mask) <<  0);
    }

    void Assembler::sli4s(V d, V n, int imm) {
        this->op(0b0'1'1'011110'0100'000'01010'1,    ( imm&31), n, d);
    }
    void Assembler::shl4s(V d, V n, int imm) {
        this->op(0b0'1'0'011110'0100'000'01010'1,    ( imm&31), n, d);
    }
    void Assembler::sshr4s(V d, V n, int imm) {
        this->op(0b0'1'0'011110'0100'000'00'0'0'0'1, (-imm&31), n, d);
    }
    void Assembler::ushr4s(V d, V n, int imm) {
        this->op(0b0'1'1'011110'0100'000'00'0'0'0'1, (-imm&31), n, d);
    }
    void Assembler::ushr8h(V d, V n, int imm) {
        this->op(0b0'1'1'011110'0010'000'00'0'0'0'1, (-imm&15), n, d);
    }

    void Assembler::scvtf4s (V d, V n) { this->op(0b0'1'0'01110'0'0'10000'11101'10, n,d); }
    void Assembler::fcvtzs4s(V d, V n) { this->op(0b0'1'0'01110'1'0'10000'1101'1'10, n,d); }

    void Assembler::xtns2h(V d, V n) { this->op(0b0'0'0'01110'01'10000'10010'10, n,d); }
    void Assembler::xtnh2b(V d, V n) { this->op(0b0'0'0'01110'00'10000'10010'10, n,d); }

    void Assembler::uxtlb2h(V d, V n) { this->op(0b0'0'1'011110'0001'000'10100'1, n,d); }
    void Assembler::uxtlh2s(V d, V n) { this->op(0b0'0'1'011110'0010'000'10100'1, n,d); }

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
    void Assembler::sub(X d, X n, int imm12) {
        this->word( 0b1'1'0'10001'00  << 22
                  | (imm12 & 12_mask) << 10
                  | (n     &  5_mask) <<  5
                  | (d     &  5_mask) <<  0);
    }
    void Assembler::subs(X d, X n, int imm12) {
        this->word( 0b1'1'1'10001'00  << 22
                  | (imm12 & 12_mask) << 10
                  | (n     &  5_mask) <<  5
                  | (d     &  5_mask) <<  0);
    }

    void Assembler::b(Condition cond, Label* l) {
        const int imm19 = this->disp19(l);
        this->word( 0b0101010'0           << 24
                  | (imm19     & 19_mask) <<  5
                  | ((int)cond &  4_mask) <<  0);
    }
    void Assembler::cbz(X t, Label* l) {
        const int imm19 = this->disp19(l);
        this->word( 0b1'011010'0      << 24
                  | (imm19 & 19_mask) <<  5
                  | (t     &  5_mask) <<  0);
    }
    void Assembler::cbnz(X t, Label* l) {
        const int imm19 = this->disp19(l);
        this->word( 0b1'011010'1      << 24
                  | (imm19 & 19_mask) <<  5
                  | (t     &  5_mask) <<  0);
    }

    void Assembler::ldrq(V dst, X src) { this->op(0b00'111'1'01'11'000000000000, src, dst); }
    void Assembler::ldrs(V dst, X src) { this->op(0b10'111'1'01'01'000000000000, src, dst); }
    void Assembler::ldrb(V dst, X src) { this->op(0b00'111'1'01'01'000000000000, src, dst); }

    void Assembler::strq(V src, X dst) { this->op(0b00'111'1'01'10'000000000000, dst, src); }
    void Assembler::strs(V src, X dst) { this->op(0b10'111'1'01'00'000000000000, dst, src); }
    void Assembler::strb(V src, X dst) { this->op(0b00'111'1'01'00'000000000000, dst, src); }

    void Assembler::ldrq(V dst, Label* l) {
        const int imm19 = this->disp19(l);
        this->word( 0b10'011'1'00     << 24
                  | (imm19 & 19_mask) << 5
                  | (dst   &  5_mask) << 0);
    }

    void Assembler::label(Label* l) {
        if (fCode) {
            // The instructions all currently point to l->offset.
            // We'll want to add a delta to point them to here().
            int delta = here().offset - l->offset;
            l->offset = here().offset;

            if (l->kind == Label::ARMDisp19) {
                for (int ref : l->references) {
                    // ref points to a 32-bit instruction with 19-bit displacement in instructions.
                    uint32_t inst;
                    memcpy(&inst, fCode + ref, 4);

                    // [ 8 bits to preserve] [ 19 bit signed displacement ] [ 5 bits to preserve ]
                    int disp = (int)(inst << 8) >> 13;

                    disp += delta/4;  // delta is in bytes, we want instructions.

                    // Put it all back together, preserving the high 8 bits and low 5.
                    inst = ((disp << 5) &  (19_mask << 5))
                         | ((inst     ) & ~(19_mask << 5));

                    memcpy(fCode + ref, &inst, 4);
                }
            }

            if (l->kind == Label::X86Disp32) {
                for (int ref : l->references) {
                    // ref points to a 32-bit displacement in bytes.
                    int disp;
                    memcpy(&disp, fCode + ref, 4);

                    disp += delta;

                    memcpy(fCode + ref, &disp, 4);
                }
            }
        }
    }

    void Program::eval(int n, void* args[]) const {
        const int nargs = (int)fStrides.size();

        if (fJITBuf) {
            switch (nargs) {
                case 0: return ((void(*)(int              ))fJITBuf)(n                  );
                case 1: return ((void(*)(int, void*       ))fJITBuf)(n, args[0]         );
                case 2: return ((void(*)(int, void*, void*))fJITBuf)(n, args[0], args[1]);
                default: SkUNREACHABLE;  // TODO
            }
        }

        // We'll operate in SIMT style, knocking off K-size chunks from n while possible.
        constexpr int K = 16;
        using I32 = skvx::Vec<K, int>;
        using F32 = skvx::Vec<K, float>;
        using U32 = skvx::Vec<K, uint32_t>;
        using  U8 = skvx::Vec<K, uint8_t>;

        using I16x2 = skvx::Vec<2*K,  int16_t>;
        using U16x2 = skvx::Vec<2*K, uint16_t>;

        union Slot {
            I32 i32;
            U32 u32;
            F32 f32;
        };

        Slot                     few_regs[16];
        std::unique_ptr<char[]> many_regs;

        Slot* regs = few_regs;

        if (fRegs > (int)SK_ARRAY_COUNT(few_regs)) {
            // Annoyingly we can't trust that malloc() or new will work with Slot because
            // the skvx::Vec types may have alignment greater than what they provide.
            // We'll overallocate one extra register so we can align manually.
            many_regs.reset(new char[ sizeof(Slot) * (fRegs + 1) ]);

            uintptr_t addr = (uintptr_t)many_regs.get();
            addr += alignof(Slot) -
                     (addr & (alignof(Slot) - 1));
            SkASSERT((addr & (alignof(Slot) - 1)) == 0);
            regs = (Slot*)addr;
        }


        auto r = [&](Reg id) -> Slot& {
            SkASSERT(0 <= id && id < fRegs);
            return regs[id];
        };
        auto arg = [&](int ix) {
            SkASSERT(0 <= ix && ix < nargs);
            return args[ix];
        };

        // Step each argument pointer ahead by its stride a number of times.
        auto step_args = [&](int times) {
            // Looping by marching pointers until *arg == nullptr helps the
            // compiler to keep this loop scalar.  Otherwise it'd create a
            // rather large and useless autovectorized version.
            void**        arg = args;
            const int* stride = fStrides.data();
            for (; *arg; arg++, stride++) {
                *arg = (void*)( (char*)*arg + times * *stride );
            }
            SkASSERT(arg == args + nargs);
        };

        int start = 0,
            stride;
        for ( ; n > 0; start = fLoop, n -= stride, step_args(stride)) {
            stride = n >= K ? K : 1;

            for (int i = start; i < (int)fInstructions.size(); i++) {
                Instruction inst = fInstructions[i];

                // d = op(x,y,z/imm)
                Reg   d = inst.d,
                      x = inst.x,
                      y = inst.y,
                      z = inst.z;
                int imm = inst.imm;

                // Ops that interact with memory need to know whether we're stride=1 or K,
                // but all non-memory ops can run the same code no matter the stride.
                switch (2*(int)inst.op + (stride == K ? 1 : 0)) {

                #define STRIDE_1(op) case 2*(int)op
                #define STRIDE_K(op) case 2*(int)op + 1
                    STRIDE_1(Op::store8 ): memcpy(arg(imm), &r(x).i32, 1); break;
                    STRIDE_1(Op::store32): memcpy(arg(imm), &r(x).i32, 4); break;

                    STRIDE_K(Op::store8 ): skvx::cast<uint8_t>(r(x).i32).store(arg(imm)); break;
                    STRIDE_K(Op::store32):                    (r(x).i32).store(arg(imm)); break;

                    STRIDE_1(Op::load8 ): r(d).i32 = 0; memcpy(&r(d).i32, arg(imm), 1); break;
                    STRIDE_1(Op::load32): r(d).i32 = 0; memcpy(&r(d).i32, arg(imm), 4); break;

                    STRIDE_K(Op::load8 ): r(d).i32= skvx::cast<int>(U8 ::Load(arg(imm))); break;
                    STRIDE_K(Op::load32): r(d).i32=                 I32::Load(arg(imm)) ; break;
                #undef STRIDE_1
                #undef STRIDE_K

                    // Ops that don't interact with memory should never care about the stride.
                #define CASE(op) case 2*(int)op: /*fallthrough*/ case 2*(int)op+1
                    CASE(Op::splat): r(d).i32 = imm; break;

                    CASE(Op::add_f32): r(d).f32 = r(x).f32 + r(y).f32; break;
                    CASE(Op::sub_f32): r(d).f32 = r(x).f32 - r(y).f32; break;
                    CASE(Op::mul_f32): r(d).f32 = r(x).f32 * r(y).f32; break;
                    CASE(Op::div_f32): r(d).f32 = r(x).f32 / r(y).f32; break;

                    CASE(Op::mad_f32): r(d).f32 = r(x).f32 * r(y).f32 + r(z).f32; break;

                    CASE(Op::add_i32): r(d).i32 = r(x).i32 + r(y).i32; break;
                    CASE(Op::sub_i32): r(d).i32 = r(x).i32 - r(y).i32; break;
                    CASE(Op::mul_i32): r(d).i32 = r(x).i32 * r(y).i32; break;

                    CASE(Op::sub_i16x2):
                        r(d).i32 = skvx::bit_pun<I32>(skvx::bit_pun<I16x2>(r(x).i32) -
                                                      skvx::bit_pun<I16x2>(r(y).i32) ); break;
                    CASE(Op::mul_i16x2):
                        r(d).i32 = skvx::bit_pun<I32>(skvx::bit_pun<I16x2>(r(x).i32) *
                                                      skvx::bit_pun<I16x2>(r(y).i32) ); break;
                    CASE(Op::shr_i16x2):
                        r(d).i32 = skvx::bit_pun<I32>(skvx::bit_pun<U16x2>(r(x).i32) >> imm);
                        break;

                    CASE(Op::bit_and):   r(d).i32 = r(x).i32 &  r(y).i32; break;
                    CASE(Op::bit_or ):   r(d).i32 = r(x).i32 |  r(y).i32; break;
                    CASE(Op::bit_xor):   r(d).i32 = r(x).i32 ^  r(y).i32; break;
                    CASE(Op::bit_clear): r(d).i32 = r(x).i32 & ~r(y).i32; break;

                    CASE(Op::shl): r(d).i32 = r(x).i32 << imm; break;
                    CASE(Op::sra): r(d).i32 = r(x).i32 >> imm; break;
                    CASE(Op::shr): r(d).u32 = r(x).u32 >> imm; break;

                    CASE(Op::extract): r(d).u32 = (r(x).u32 >> imm) & r(y).u32; break;
                    CASE(Op::pack):    r(d).u32 = r(x).u32 | (r(y).u32 << imm); break;

                    CASE(Op::bytes): {
                        const U32 table[] = {
                            0,
                            (r(x).u32      ) & 0xff,
                            (r(x).u32 >>  8) & 0xff,
                            (r(x).u32 >> 16) & 0xff,
                            (r(x).u32 >> 24) & 0xff,
                        };
                        r(d).u32 = table[(imm >>  0) & 0xf] <<  0
                                 | table[(imm >>  4) & 0xf] <<  8
                                 | table[(imm >>  8) & 0xf] << 16
                                 | table[(imm >> 12) & 0xf] << 24;
                    } break;

                    CASE(Op::to_f32): r(d).f32 = skvx::cast<float>(r(x).i32); break;
                    CASE(Op::to_i32): r(d).i32 = skvx::cast<int>  (r(x).f32); break;
                #undef CASE
                }
            }
        }
    }

    void Program::dropJIT() {
    #if defined(SKVM_JIT)
        if (fJITBuf) {
            munmap(fJITBuf, fJITSize);
        }
    #else
        SkASSERT(fJITBuf == nullptr);
    #endif

        fJITBuf   = nullptr;
        fJITSize  = 0;
    }

    Program::~Program() { this->dropJIT(); }

    Program::Program(Program&& other) {
        fInstructions = std::move(other.fInstructions);
        fRegs         = other.fRegs;
        fLoop         = other.fLoop;
        fStrides      = std::move(other.fStrides);

        std::swap(fJITBuf  , other.fJITBuf);
        std::swap(fJITSize , other.fJITSize);
    }

    Program& Program::operator=(Program&& other) {
        fInstructions = std::move(other.fInstructions);
        fRegs         = other.fRegs;
        fLoop         = other.fLoop;
        fStrides      = std::move(other.fStrides);

        std::swap(fJITBuf  , other.fJITBuf);
        std::swap(fJITSize , other.fJITSize);
        return *this;
    }

    Program::Program(const std::vector<Builder::Instruction>& instructions,
                     const std::vector<int>& strides,
                     const char* debug_name) : fStrides(strides) {
        this->setupInterpreter(instructions);
    #if defined(SKVM_JIT)
        this->setupJIT(instructions, debug_name);
    #endif
    }

    // Translate Builder::Instructions to Program::Instructions used by the interpreter.
    void Program::setupInterpreter(const std::vector<Builder::Instruction>& instructions) {
        struct Analysis {
            bool hoist = true;  // Can this instruction be hoisted outside the implicit loop?
            Reg  reg   = 0;     // Register this instruction's output is assigned to.
        };
        std::vector<Analysis> analysis(instructions.size());

        // Hoisting out non-loop-dependent values is pretty valuable to the interpreter.
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const Builder::Instruction& inst = instructions[id];

            // Loads and stores cannot be hoisted out of the loop.
            if (inst.op <= Op::load32) {
                analysis[id].hoist = false;
            }

            // If any of an instruction's inputs can't be hoisted, it can't be hoisted itself.
            if (analysis[id].hoist) {
                if (inst.x != NA) { analysis[id].hoist &= analysis[inst.x].hoist; }
                if (inst.y != NA) { analysis[id].hoist &= analysis[inst.y].hoist; }
                if (inst.z != NA) { analysis[id].hoist &= analysis[inst.z].hoist; }
            }
        }

        // This next bit is a bit more complicated than strictly necessary;
        // we could just assign every live instruction to its own register.
        //
        // But recycling registers in the loop is fairly cheap, and good practice
        // for the JITs where minimizing register pressure really is important.
        // (Also helps minimize unit test diffs.)

        // Assign a register to each live hoisted instruction.  We'll never recycle these.
        fRegs = 0;
        int live_instructions = 0;
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const Builder::Instruction& inst = instructions[id];
            if (inst.death != 0 && analysis[id].hoist) {
                live_instructions++;
                analysis[id].reg = fRegs++;
            }
        }

        // Assign registers to each live loop instruction, recycling them when we can.
        std::vector<Reg> avail;
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const Builder::Instruction& inst = instructions[id];
            if (inst.death != 0 && !analysis[id].hoist) {
                live_instructions++;

                /// If an instruction's input is no longer live, we can recycle its register.
                auto maybe_recycle_register = [&](Val input) {
                    // If this is a real input and it's lifetime ends at this instruction,
                    // we can recycle the register it's occupying.
                    if (input != NA
                            && !analysis[input].hoist
                            && instructions[input].death == id) {
                        avail.push_back(analysis[input].reg);
                    }
                };

                // Take care to not recycle the same register twice.
                if (true                                ) { maybe_recycle_register(inst.x); }
                if (inst.y != inst.x                    ) { maybe_recycle_register(inst.y); }
                if (inst.z != inst.x && inst.z != inst.y) { maybe_recycle_register(inst.z); }

                // Allocate a register if we have to, preferring to reuse anything available.
                if (avail.empty()) {
                    analysis[id].reg = fRegs++;
                } else {
                    analysis[id].reg = avail.back();
                    avail.pop_back();
                }
            }
        }

        // Translate Builder::Instructions to Program::Instructions by mapping values to
        // registers.  This will be two passes, first hoisted instructions, then inside the loop.

        // The loop begins at the fLoop'th Instruction.
        fLoop = 0;
        fInstructions.reserve(live_instructions);

        // Add a dummy mapping for the N/A sentinel Val to any arbitrary register
        // so lookups don't have to know which arguments are used by which Ops.
        auto lookup_register = [&](Val id) {
            return id == NA ? (Reg)0
                            : analysis[id].reg;
        };

        auto push_instruction = [&](Val id, const Builder::Instruction& inst) {
            Program::Instruction pinst{
                inst.op,
                lookup_register(id),
                lookup_register(inst.x),
                lookup_register(inst.y),
               {lookup_register(inst.z)},
            };
            if (inst.z == NA) { pinst.imm = inst.imm; }
            fInstructions.push_back(pinst);
        };

        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const Builder::Instruction& inst = instructions[id];
            if (inst.death != 0 && analysis[id].hoist) {
                push_instruction(id, inst);
                fLoop++;
            }
        }
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const Builder::Instruction& inst = instructions[id];
            if (inst.death != 0 && !analysis[id].hoist) {
                push_instruction(id, inst);
            }
        }
    }

#if defined(SKVM_JIT)

    // Just so happens that we can translate the immediate control for our bytes() op
    // to a single 128-bit mask that can be consumed by both AVX2 vpshufb and NEON tbl!
    static void bytes_control(int imm, int mask[4]) {
        auto nibble_to_vpshufb = [](uint8_t n) -> uint8_t {
            // 0 -> 0xff,    Fill with zero
            // 1 -> 0x00,    Select byte 0
            // 2 -> 0x01,         "      1
            // 3 -> 0x02,         "      2
            // 4 -> 0x03,         "      3
            return n - 1;
        };
        uint8_t control[] = {
            nibble_to_vpshufb( (imm >>  0) & 0xf ),
            nibble_to_vpshufb( (imm >>  4) & 0xf ),
            nibble_to_vpshufb( (imm >>  8) & 0xf ),
            nibble_to_vpshufb( (imm >> 12) & 0xf ),
        };
        for (int i = 0; i < 4; i++) {
            mask[i] = (int)control[0] <<  0
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
    }

    bool Program::jit(const std::vector<Builder::Instruction>& instructions, Assembler* a) const {
        using A = Assembler;

    #if defined(__x86_64__)
        if (!SkCpu::Supports(SkCpu::HSW)) {
            return false;
        }
        A::GP64 N     = A::rdi,
                arg[] = { A::rsi, A::rdx, A::rcx, A::r8, A::r9 };

        // All 16 ymm registers are available to use.
        using Reg = A::Ymm;
        uint32_t avail = 0xffff;

    #elif defined(__aarch64__)
        A::X N     = A::x0,
             arg[] = { A::x1, A::x2, A::x3, A::x4, A::x5, A::x6, A::x7 };

        // We can use v0-v7 and v16-v31 freely; we'd need to preseve v8-v15.
        using Reg = A::V;
        uint32_t avail = 0xffff00ff;
    #endif

        if (SK_ARRAY_COUNT(arg) < fStrides.size()) {
            return false;
        }

        std::vector<Reg> r(instructions.size());
        SkTHashMap<int, A::Label> bytes_masks,
                                  splats;

        auto emit = [&](Val id, bool scalar) {
            const Builder::Instruction& inst = instructions[id];

            // No need to emit dead code instructions that produce values that are never used.
            if (inst.death == 0) {
                return true;
            }

            Op op = inst.op;
            Val x = inst.x,
                y = inst.y,
                z = inst.z;
            int imm = inst.imm;

            // Most (but not all) ops create an output value and need a register to hold it, dst.
            // We track each instruction's dst in r[] so we can thread it through as an input
            // to any future instructions needing that value.
            //
            // And some ops may need a temporary scratch register, tmp.  Some need both tmp and dst.
            //
            // tmp and dst are very similar and can and will often be assigned the same register,
            // but tmp may never alias any of the instructions's inputs, while dst may when this
            // instruction consumes that input, i.e. if the input reaches its end of life here.
            //
            // We'll assign both registers lazily to keep register pressure as low as possible.
            bool tmp_is_set = false,
                 dst_is_set = false;
            Reg tmp_reg = (Reg)0;  // This initial value won't matter... anything legal is fine.

            bool ok = true;   // Set to false if we need to assign a register and none's available.

            // First lock in how to choose tmp if we need to based on the registers
            // available before this instruction, not including any of its input registers.
            auto tmp = [&,avail/*important, closing over avail's current value*/]{
                if (!tmp_is_set) {
                    tmp_is_set = true;
                    if (int found = __builtin_ffs(avail)) {
                        // This is a scratch register just for this op,
                        // so we leave it marked available for future ops.
                        tmp_reg = (Reg)(found - 1);
                    } else {
                        // We needed a tmp register but couldn't find one available. :'(
                        // This will cause emit() to return false, in turn causing jit() to fail.
                        ok = false;
                    }
                }
                return tmp_reg;
            };

            // Now make available any registers that are consumed by this instruction.
            // (The register pool we can pick dst from is >= the pool for tmp, adding any of these.)
            if (x != NA && instructions[x].death == id) { avail |= 1 << r[x]; }
            if (y != NA && instructions[y].death == id) { avail |= 1 << r[y]; }
            if (z != NA && instructions[z].death == id) { avail |= 1 << r[z]; }
            // set_dst() and dst() will work read/write with this perhaps-just-updated avail.

            // Some ops may decide dst on their own to best fit the instruction (see Op::mad_f32).
            auto set_dst = [&](Reg reg){
                SkASSERT(dst_is_set == false);
                dst_is_set = true;

                SkASSERT(avail & (1<<reg));
                avail ^= 1<<reg;

                r[id] = reg;
            };

            // Thanks to AVX and NEON's 3-argument instruction sets,
            // most ops can use any register as dst.
            auto dst = [&]{
                if (!dst_is_set) {
                    if (int found = __builtin_ffs(avail)) {
                        set_dst((Reg)(found-1));
                    } else {
                        // Same deal as with tmp... all the registers are occupied.  Time to fail!
                        ok = false;
                    }
                }
                return r[id];
            };

            // Because we use the same logic to pick an arbitrary dst and to pick tmp,
            // and we know that tmp will never overlap any of the inputs, `dst() == tmp()`
            // is a simple idiom to check that the destination does not overlap any of the inputs.
            // Sometimes we can use this knowledge to do better instruction selection.

            // Ok!  Keep in mind that we haven't assigned tmp or dst yet,
            // just laid out hooks for how to do so if we need them, depending on the instruction.
            //
            // Now let's actually assemble the instruction!
            switch (op) {
            #if defined(__x86_64__)
                case Op::store8: if (scalar) { a->vpextrb  (arg[imm], (A::Xmm)r[x], 0); }
                                 else        { a->vpackusdw(tmp(), r[x], r[x]);
                                               a->vpermq   (tmp(), tmp(), 0xd8);
                                               a->vpackuswb(tmp(), tmp(), tmp());
                                               a->vmovq    (arg[imm], (A::Xmm)tmp()); }
                                 break;

                case Op::store32: if (scalar) { a->vmovd  (arg[imm], (A::Xmm)r[x]); }
                                  else        { a->vmovups(arg[imm],         r[x]); }
                                  break;

                case Op::load8:  if (scalar) {
                                     a->vpxor  (dst(), dst(), dst());
                                     a->vpinsrb((A::Xmm)dst(), (A::Xmm)dst(), arg[imm], 0);
                                 } else {
                                     a->vpmovzxbd(dst(), arg[imm]);
                                 } break;

                case Op::load32: if (scalar) { a->vmovd  ((A::Xmm)dst(), arg[imm]); }
                                 else        { a->vmovups(        dst(), arg[imm]); }
                                 break;

                case Op::splat:  if (!splats.find(imm)) { splats.set(imm, {}); }
                                 a->vbroadcastss(dst(), splats.find(imm));
                                 break;

                case Op::add_f32: a->vaddps(dst(), r[x], r[y]); break;
                case Op::sub_f32: a->vsubps(dst(), r[x], r[y]); break;
                case Op::mul_f32: a->vmulps(dst(), r[x], r[y]); break;
                case Op::div_f32: a->vdivps(dst(), r[x], r[y]); break;

                case Op::mad_f32:
                    if      (avail & (1<<r[x])) { set_dst(r[x]); a->vfmadd132ps(r[x], r[z], r[y]); }
                    else if (avail & (1<<r[y])) { set_dst(r[y]); a->vfmadd213ps(r[y], r[x], r[z]); }
                    else if (avail & (1<<r[z])) { set_dst(r[z]); a->vfmadd231ps(r[z], r[x], r[y]); }
                    else                        {                SkASSERT(dst() == tmp());
                                                                 // TODO: vpor -> vmovdqa here?
                                                                 a->vpor       (dst(),r[x], r[x]);
                                                                 a->vfmadd132ps(dst(),r[z], r[y]); }
                                                                 break;

                case Op::add_i32: a->vpaddd (dst(), r[x], r[y]); break;
                case Op::sub_i32: a->vpsubd (dst(), r[x], r[y]); break;
                case Op::mul_i32: a->vpmulld(dst(), r[x], r[y]); break;

                case Op::sub_i16x2: a->vpsubw (dst(), r[x], r[y]); break;
                case Op::mul_i16x2: a->vpmullw(dst(), r[x], r[y]); break;
                case Op::shr_i16x2: a->vpsrlw (dst(), r[x],  imm); break;

                case Op::bit_and:   a->vpand (dst(), r[x], r[y]); break;
                case Op::bit_or :   a->vpor  (dst(), r[x], r[y]); break;
                case Op::bit_xor:   a->vpxor (dst(), r[x], r[y]); break;
                case Op::bit_clear: a->vpandn(dst(), r[y], r[x]); break;  // N.B. Y then X.

                case Op::shl: a->vpslld(dst(), r[x], imm); break;
                case Op::shr: a->vpsrld(dst(), r[x], imm); break;
                case Op::sra: a->vpsrad(dst(), r[x], imm); break;

                case Op::extract: if (imm == 0) { a->vpand (dst(),  r[x], r[y]); }
                                  else          { a->vpsrld(tmp(),  r[x], imm);
                                                  a->vpand (dst(), tmp(), r[y]); }
                                  break;

                case Op::pack: a->vpslld(tmp(),  r[y], imm);
                               a->vpor  (dst(), tmp(), r[x]);
                               break;

                case Op::to_f32: a->vcvtdq2ps (dst(), r[x]); break;
                case Op::to_i32: a->vcvttps2dq(dst(), r[x]); break;

                case Op::bytes:  if (!bytes_masks.find(imm)) { bytes_masks.set(imm, {}); }
                                 a->vpshufb(dst(), r[x], bytes_masks.find(imm));
                                 break;
            #elif defined(__aarch64__)
                case Op::store8: a->xtns2h(tmp(), r[x]);
                                 a->xtnh2b(tmp(), tmp());
                   if (scalar) { a->strb  (tmp(), arg[imm]); }
                   else        { a->strs  (tmp(), arg[imm]); }
                                 break;

                case Op::store32: if (scalar) { a->strs(r[x], arg[imm]); }
                                  else        { a->strq(r[x], arg[imm]); }
                                                break;

                case Op::load8: if (scalar) { a->ldrb(tmp(), arg[imm]); }
                                else        { a->ldrs(tmp(), arg[imm]); }
                                              a->uxtlb2h(tmp(), tmp());
                                              a->uxtlh2s(dst(), tmp());
                                              break;

                case Op::load32: if (scalar) { a->ldrs(dst(), arg[imm]); }
                                 else        { a->ldrq(dst(), arg[imm]); }
                                               break;

                case Op::splat:  if (!splats.find(imm)) { splats.set(imm, {}); }
                                 a->ldrq(dst(), splats.find(imm));
                                 break;

                case Op::add_f32: a->fadd4s(dst(), r[x], r[y]); break;
                case Op::sub_f32: a->fsub4s(dst(), r[x], r[y]); break;
                case Op::mul_f32: a->fmul4s(dst(), r[x], r[y]); break;
                case Op::div_f32: a->fdiv4s(dst(), r[x], r[y]); break;

                case Op::mad_f32:
                    if (avail & (1<<r[z])) { set_dst(r[z]); a->fmla4s( r[z],  r[x],  r[y]);   }
                    else                   {                a->orr16b(tmp(),  r[z],  r[z]);
                                                            a->fmla4s(tmp(),  r[x],  r[y]);
                                       if(dst() != tmp()) { a->orr16b(dst(), tmp(), tmp()); } }
                                                            break;


                case Op::add_i32: a->add4s(dst(), r[x], r[y]); break;
                case Op::sub_i32: a->sub4s(dst(), r[x], r[y]); break;
                case Op::mul_i32: a->mul4s(dst(), r[x], r[y]); break;

                case Op::sub_i16x2: a->sub8h (dst(), r[x], r[y]); break;
                case Op::mul_i16x2: a->mul8h (dst(), r[x], r[y]); break;
                case Op::shr_i16x2: a->ushr8h(dst(), r[x],  imm); break;

                case Op::bit_and:   a->and16b(dst(), r[x], r[y]); break;
                case Op::bit_or :   a->orr16b(dst(), r[x], r[y]); break;
                case Op::bit_xor:   a->eor16b(dst(), r[x], r[y]); break;
                case Op::bit_clear: a->bic16b(dst(), r[x], r[y]); break;

                case Op::shl: a-> shl4s(dst(), r[x], imm); break;
                case Op::shr: a->ushr4s(dst(), r[x], imm); break;
                case Op::sra: a->sshr4s(dst(), r[x], imm); break;

                case Op::extract: if (imm) { a->ushr4s(tmp(), r[x], imm);
                                             a->and16b(dst(), tmp(), r[y]); }
                                  else     { a->and16b(dst(), r[x], r[y]); }
                                             break;

                case Op::pack:
                    if (avail & (1<<r[x])) { set_dst(r[x]); a->sli4s ( r[x],  r[y],  imm); }
                    else                   {                a->shl4s (tmp(),  r[y],  imm);
                                                            a->orr16b(dst(), tmp(), r[x]); }
                                                            break;

                case Op::to_f32: a->scvtf4s (dst(), r[x]); break;
                case Op::to_i32: a->fcvtzs4s(dst(), r[x]); break;

                case Op::bytes:  if (!bytes_masks.find(imm)) { bytes_masks.set(imm, {}); }
                                 a->ldrq(tmp(), bytes_masks.find(imm));  // TODO: hoist these
                                 a->tbl (dst(), r[x], tmp());
                                 break;
            #endif
            }

            // Calls to tmp() or dst() might have flipped this false from its default true state.
            return ok;
        };

        A::Label body,
                 tail,
                 done;

        #if defined(__x86_64__)
            const int K = 8;
            auto jump_if_less = [&](A::Label* l) { a->jl (l); };
            auto jump         = [&](A::Label* l) { a->jmp(l); };

            auto add = [&](A::GP64 gp, int imm) { a->add(gp, imm); };
            auto sub = [&](A::GP64 gp, int imm) { a->sub(gp, imm); };

            auto exit = [&]{ a->vzeroupper(); a->ret(); };
        #elif defined(__aarch64__)
            const int K = 4;
            auto jump_if_less = [&](A::Label* l) { a->blt(l); };
            auto jump         = [&](A::Label* l) { a->b  (l); };

            auto add = [&](A::X gp, int imm) { a->add(gp, gp, imm); };
            auto sub = [&](A::X gp, int imm) { a->sub(gp, gp, imm); };

            auto exit = [&]{ a->ret(A::x30); };
        #endif

        a->label(&body);
        {
            a->cmp(N, K);
            jump_if_less(&tail);
            for (Val id = 0; id < (Val)instructions.size(); id++) {
                if (!emit(id, /*scalar=*/false)) {
                    return false;
                }
            }
            for (int i = 0; i < (int)fStrides.size(); i++) {
                add(arg[i], K*fStrides[i]);
            }
            sub(N, K);
            jump(&body);
        }

        a->label(&tail);
        {
            a->cmp(N, 1);
            jump_if_less(&done);
            for (Val id = 0; id < (Val)instructions.size(); id++) {
                if (!emit(id, /*scalar=*/true)) {
                    return false;
                }
            }
            for (int i = 0; i < (int)fStrides.size(); i++) {
                add(arg[i], 1*fStrides[i]);
            }
            sub(N, 1);
            jump(&tail);
        }

        a->label(&done);
        {
            exit();
        }

        bytes_masks.foreach([&](int imm, A::Label* l) {
            // One 16-byte pattern for ARM tbl, that same pattern twice for x86-64 vpshufb.
        #if defined(__x86_64__)
            a->align(32);
        #elif defined(__aarch64__)
            a->align(4);
        #endif

            a->label(l);
            int mask[4];
            bytes_control(imm, mask);
            a->bytes(mask, sizeof(mask));
        #if defined(__x86_64__)
            a->bytes(mask, sizeof(mask));
        #endif
        });

        splats.foreach([&](int imm, A::Label* l) {
            // vbroadcastss 4 bytes on x86-64, or simply load 16-bytes on aarch64.
            a->align(4);
            a->label(l);
            a->word(imm);
        #if defined(__aarch64__)
            a->word(imm);
            a->word(imm);
            a->word(imm);
        #endif
        });

        return true;
    }

    void Program::setupJIT(const std::vector<Builder::Instruction>& instructions,
                           const char* debug_name) {
        // Run first with no buffer to determine a.size(), the number of bytes we'll assemble.
        Assembler a{nullptr};
        if (!this->jit(instructions, &a)) {
            return;
        }

        // Allocate space that we can remap as executable.
        const size_t page = sysconf(_SC_PAGESIZE);
        fJITSize = ((a.size() + page - 1) / page) * page;  // mprotect works at page granularity.
        fJITBuf = mmap(nullptr,fJITSize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1,0);

        // Assemble the program for real.
        a = Assembler{fJITBuf};
        SkAssertResult(this->jit(instructions, &a));
        SkASSERT(a.size() <= fJITSize);

        // Remap as executable, and flush caches on platforms that need that.
        mprotect(fJITBuf, fJITSize, PROT_READ|PROT_EXEC);
        __builtin___clear_cache((char*)fJITBuf,
                                (char*)fJITBuf + fJITSize);
    #if defined(SKVM_PERF_DUMPS)
        this->dumpJIT(debug_name, a.size());
    #endif
    }
#endif

#if defined(SKVM_PERF_DUMPS)
    void Program::dumpJIT(const char* debug_name, size_t size) const {
    #if defined(__aarch64__)
        if (debug_name) {
            SkDebugf("\n%s:", debug_name);
        }
        // cat | llvm-mc -arch aarch64 -disassemble
        auto cur = (const uint8_t*)fJITBuf;
        for (int i = 0; i < (int)size; i++) {
            if (i % 4 == 0) {
                SkDebugf("\n");
            }
            SkDebugf("0x%02x ", *cur++);
        }
        SkDebugf("\n");
    #endif

        // We're doing some really stateful things below so one thread at a time please...
        static SkSpinlock dump_lock;
        SkAutoSpinlock lock(dump_lock);

        auto fnv1a = [](const void* vbuf, size_t n) {
            uint32_t hash = 2166136261;
            for (auto buf = (const uint8_t*)vbuf; n --> 0; buf++) {
                hash ^= *buf;
                hash *= 16777619;
            }
            return hash;
        };


        char name[64];
        uint32_t hash = fnv1a(fJITBuf, size);
        if (debug_name) {
            sprintf(name, "skvm-jit-%s", debug_name);
        } else {
            sprintf(name, "skvm-jit-%u", hash);
        }

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
        //
        //    https://lwn.net/Articles/638566/
        //    https://v8.dev/docs/linux-perf
        //    https://cs.chromium.org/chromium/src/v8/src/diagnostics/perf-jit.cc
        //    https://lore.kernel.org/patchwork/patch/622240/


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
            char path[64];
            sprintf(path, "jit-%d.dump", getpid());
            FILE* f = fopen(path, "w+");

            // Calling mmap() on the file adds a "hey they mmap()'d this" record to
            // the perf.data file that will point `perf inject -j` at this log file.
            // Kind of a strange way to tell `perf inject` where the file is...
            void* marker = mmap(nullptr, sysconf(_SC_PAGESIZE),
                                PROT_READ|PROT_EXEC, MAP_PRIVATE,
                                fileno(f), /*offset=*/0);
            SkASSERT_RELEASE(marker != MAP_FAILED);
            // Like never calling fclose(f), we'll also just always leave marker mmap()'d.

        #if defined(__x86_64__)
            const uint32_t elf_mach = 62;
        #elif defined(__aarch64__)
            const uint32_t elf_mach = 183;
        #endif

            struct Header {
                uint32_t magic, version, header_size, elf_mach, reserved, pid;
                uint64_t timestamp_us, flags;
            } header = {
                0x4A695444, 1, sizeof(Header), elf_mach, 0, (uint32_t)getpid(),
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
            0/*code load*/, (uint32_t)(sizeof(CodeLoad) + strlen(name) + 1 + size),
            timestamp_ns(),

            (uint32_t)getpid(), (uint32_t)SkGetThreadID(),
            (uint64_t)fJITBuf, (uint64_t)fJITBuf, size, hash,
        };

        // Write the header, the JIT'd function name, and the JIT'd code itself.
        fwrite(&load, sizeof(load), 1, jitdump);
        fwrite(name, 1, strlen(name), jitdump);
        fwrite("\0", 1, 1, jitdump);
        fwrite(fJITBuf, 1, size, jitdump);
    }
#endif

}  // namespace skvm
