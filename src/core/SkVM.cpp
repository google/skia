/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkSpinlock.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkThreadID.h"
#include "include/private/SkVx.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#include "src/core/SkVM.h"
#include <algorithm>
#include <atomic>
#include <queue>

#if defined(SKVM_LLVM)
    #include <future>
    #include <llvm/Bitcode/BitcodeWriter.h>
    #include <llvm/ExecutionEngine/ExecutionEngine.h>
    #include <llvm/IR/IRBuilder.h>
    #include <llvm/IR/Verifier.h>
    #include <llvm/Support/TargetSelect.h>
#endif

bool gSkVMJITViaDylib{false};

// JIT code isn't MSAN-instrumented, so we won't see when it uses
// uninitialized memory, and we'll not see the writes it makes as properly
// initializing memory.  Instead force the interpreter, which should let
// MSAN see everything our programs do properly.
//
// Similarly, we can't get ASAN's checks unless we let it instrument our interpreter.
#if defined(__has_feature)
    #if __has_feature(memory_sanitizer) || __has_feature(address_sanitizer)
        #undef SKVM_JIT
    #endif
#endif

#if defined(SKVM_JIT)
    #include <dlfcn.h>      // dlopen, dlsym
    #include <sys/mman.h>   // mmap, mprotect
#endif

namespace skvm {

    struct Program::Impl {
        std::vector<InterpreterInstruction> instructions;
        int regs = 0;
        int loop = 0;
        std::vector<int> strides;

        std::atomic<void*> jit_entry{nullptr};   // TODO: minimal std::memory_orders
        size_t jit_size = 0;
        void*  dylib    = nullptr;

    #if defined(SKVM_LLVM)
        std::unique_ptr<llvm::LLVMContext>     llvm_ctx;
        std::unique_ptr<llvm::ExecutionEngine> llvm_ee;
        std::future<void>                      llvm_compiling;
    #endif
    };

    // Debugging tools, mostly for printing various data structures out to a stream.

    namespace {
        class SkDebugfStream final : public SkWStream {
            size_t fBytesWritten = 0;

            bool write(const void* buffer, size_t size) override {
                SkDebugf("%.*s", size, buffer);
                fBytesWritten += size;
                return true;
            }

            size_t bytesWritten() const override {
                return fBytesWritten;
            }
        };

        struct V { Val id; };
        struct R { Reg id; };
        struct Shift { int bits; };
        struct Splat { int bits; };
        struct Hex   { int bits; };

        static void write(SkWStream* o, const char* s) {
            o->writeText(s);
        }

        static const char* name(Op op) {
            switch (op) {
            #define M(x) case Op::x: return #x;
                SKVM_OPS(M)
            #undef M
            }
            return "unknown op";
        }

        static void write(SkWStream* o, Op op) {
            const char* raw = name(op);
            if (const char* found = strstr(raw, "_imm")) {
                o->write(raw, found-raw);
            } else {
                o->writeText(raw);
            }
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
    }

    void Builder::dot(SkWStream* o, bool for_jit) const {
        SkDebugfStream debug;
        if (!o) { o = &debug; }

        std::vector<OptimizedInstruction> optimized = this->optimize(for_jit);

        o->writeText("digraph {\n");
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            auto [op, x,y,z, immy,immz, death,can_hoist,used_in_loop] = optimized[id];

            switch (op) {
                default:
                    write(o, "\t", V{id}, " [label = \"", V{id}, op);
                    // Not a perfect heuristic; sometimes y/z == NA and there is no immy/z.
                    // On the other hand, sometimes immy/z=0 is meaningful and should be printed.
                    if (y == NA) { write(o, "", Hex{immy}); }
                    if (z == NA) { write(o, "", Hex{immz}); }
                    write(o, "\"]\n");

                    write(o, "\t", V{id}, " -> {");
                    // In contrast to the heuristic imm labels, these dependences are exact.
                    if (x != NA) { write(o, "", V{x}); }
                    if (y != NA) { write(o, "", V{y}); }
                    if (z != NA) { write(o, "", V{z}); }
                    write(o, " }\n");

                    break;

                // That default: impl works pretty well for most instructions,
                // but some are nicer to see with a specialized label.

                case Op::splat:
                    write(o, "\t", V{id}, " [label = \"", V{id}, op, Splat{immy}, "\"]\n");
                    break;
            }
        }
        o->writeText("}\n");
    }

    void Builder::dump(SkWStream* o) const {
        SkDebugfStream debug;
        if (!o) { o = &debug; }

        std::vector<OptimizedInstruction> optimized = this->optimize();
        o->writeDecAsText(optimized.size());
        o->writeText(" values (originally ");
        o->writeDecAsText(fProgram.size());
        o->writeText("):\n");
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            const OptimizedInstruction& inst = optimized[id];
            Op  op = inst.op;
            Val  x = inst.x,
                 y = inst.y,
                 z = inst.z;
            int immy = inst.immy,
                immz = inst.immz;
            write(o, !inst.can_hoist    ? "  " :
                      inst.used_in_loop ? "↑ " :
                                          "↟ ");
            switch (op) {
                case Op::assert_true: write(o, op, V{x}, V{y}); break;

                case Op::store8:  write(o, op, Arg{immy}, V{x}); break;
                case Op::store16: write(o, op, Arg{immy}, V{x}); break;
                case Op::store32: write(o, op, Arg{immy}, V{x}); break;

                case Op::index: write(o, V{id}, "=", op); break;

                case Op::load8:  write(o, V{id}, "=", op, Arg{immy}); break;
                case Op::load16: write(o, V{id}, "=", op, Arg{immy}); break;
                case Op::load32: write(o, V{id}, "=", op, Arg{immy}); break;

                case Op::gather8:  write(o, V{id}, "=", op, Arg{immy}, Hex{immz}, V{x}); break;
                case Op::gather16: write(o, V{id}, "=", op, Arg{immy}, Hex{immz}, V{x}); break;
                case Op::gather32: write(o, V{id}, "=", op, Arg{immy}, Hex{immz}, V{x}); break;

                case Op::uniform8:  write(o, V{id}, "=", op, Arg{immy}, Hex{immz}); break;
                case Op::uniform16: write(o, V{id}, "=", op, Arg{immy}, Hex{immz}); break;
                case Op::uniform32: write(o, V{id}, "=", op, Arg{immy}, Hex{immz}); break;

                case Op::splat:  write(o, V{id}, "=", op, Splat{immy}); break;


                case Op::add_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::sub_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::mul_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::div_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::min_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::max_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::fma_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;
                case Op::fms_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;
                case Op::fnma_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;


                case Op::sqrt_f32: write(o, V{id}, "=", op, V{x}); break;

                case Op::add_f32_imm: write(o, V{id}, "=", op, V{x}, Splat{immy}); break;
                case Op::sub_f32_imm: write(o, V{id}, "=", op, V{x}, Splat{immy}); break;
                case Op::mul_f32_imm: write(o, V{id}, "=", op, V{x}, Splat{immy}); break;
                case Op::min_f32_imm: write(o, V{id}, "=", op, V{x}, Splat{immy}); break;
                case Op::max_f32_imm: write(o, V{id}, "=", op, V{x}, Splat{immy}); break;

                case Op:: eq_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::neq_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op:: gt_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::gte_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;


                case Op::add_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::sub_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::mul_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;

                case Op::shl_i32: write(o, V{id}, "=", op, V{x}, Shift{immy}); break;
                case Op::shr_i32: write(o, V{id}, "=", op, V{x}, Shift{immy}); break;
                case Op::sra_i32: write(o, V{id}, "=", op, V{x}, Shift{immy}); break;

                case Op:: eq_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::neq_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op:: gt_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::gte_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;

                case Op::add_i16x2: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::sub_i16x2: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::mul_i16x2: write(o, V{id}, "=", op, V{x}, V{y}); break;

                case Op::shl_i16x2: write(o, V{id}, "=", op, V{x}, Shift{immy}); break;
                case Op::shr_i16x2: write(o, V{id}, "=", op, V{x}, Shift{immy}); break;
                case Op::sra_i16x2: write(o, V{id}, "=", op, V{x}, Shift{immy}); break;

                case Op:: eq_i16x2: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::neq_i16x2: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op:: gt_i16x2: write(o, V{id}, "=", op, V{x}, V{y}); break;
                case Op::gte_i16x2: write(o, V{id}, "=", op, V{x}, V{y}); break;

                case Op::bit_and  : write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::bit_or   : write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::bit_xor  : write(o, V{id}, "=", op, V{x}, V{y}      ); break;
                case Op::bit_clear: write(o, V{id}, "=", op, V{x}, V{y}      ); break;

                case Op::bit_and_imm: write(o, V{id}, "=", op, V{x}, Hex{immy}); break;
                case Op::bit_or_imm : write(o, V{id}, "=", op, V{x}, Hex{immy}); break;
                case Op::bit_xor_imm: write(o, V{id}, "=", op, V{x}, Hex{immy}); break;

                case Op::select:  write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;
                case Op::bytes:   write(o, V{id}, "=", op, V{x}, Hex{immy}); break;
                case Op::pack:    write(o, V{id}, "=", op, V{x}, V{y}, Shift{immz}); break;

                case Op::floor:  write(o, V{id}, "=", op, V{x}); break;
                case Op::to_f32: write(o, V{id}, "=", op, V{x}); break;
                case Op::trunc:  write(o, V{id}, "=", op, V{x}); break;
                case Op::round:  write(o, V{id}, "=", op, V{x}); break;
            }

            write(o, "\n");
        }
    }

    void Program::dump(SkWStream* o) const {
        SkDebugfStream debug;
        if (!o) { o = &debug; }

        o->writeDecAsText(fImpl->regs);
        o->writeText(" registers, ");
        o->writeDecAsText(fImpl->instructions.size());
        o->writeText(" instructions:\n");
        for (Val i = 0; i < (Val)fImpl->instructions.size(); i++) {
            if (i == fImpl->loop) { write(o, "loop:\n"); }
            o->writeDecAsText(i);
            o->writeText("\t");
            if (i >= fImpl->loop) { write(o, "    "); }
            const InterpreterInstruction& inst = fImpl->instructions[i];
            Op   op = inst.op;
            Reg   d = inst.d,
                  x = inst.x,
                  y = inst.y,
                  z = inst.z;
            int immy = inst.immy,
                immz = inst.immz;
            switch (op) {
                case Op::assert_true: write(o, op, R{x}, R{y}); break;

                case Op::store8:  write(o, op, Arg{immy}, R{x}); break;
                case Op::store16: write(o, op, Arg{immy}, R{x}); break;
                case Op::store32: write(o, op, Arg{immy}, R{x}); break;

                case Op::index: write(o, R{d}, "=", op); break;

                case Op::load8:  write(o, R{d}, "=", op, Arg{immy}); break;
                case Op::load16: write(o, R{d}, "=", op, Arg{immy}); break;
                case Op::load32: write(o, R{d}, "=", op, Arg{immy}); break;

                case Op::gather8:  write(o, R{d}, "=", op, Arg{immy}, Hex{immz}, R{x}); break;
                case Op::gather16: write(o, R{d}, "=", op, Arg{immy}, Hex{immz}, R{x}); break;
                case Op::gather32: write(o, R{d}, "=", op, Arg{immy}, Hex{immz}, R{x}); break;

                case Op::uniform8:  write(o, R{d}, "=", op, Arg{immy}, Hex{immz}); break;
                case Op::uniform16: write(o, R{d}, "=", op, Arg{immy}, Hex{immz}); break;
                case Op::uniform32: write(o, R{d}, "=", op, Arg{immy}, Hex{immz}); break;

                case Op::splat:  write(o, R{d}, "=", op, Splat{immy}); break;


                case Op::add_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::sub_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::mul_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::div_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::min_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::max_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::fma_f32: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;
                case Op::fms_f32: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;
                case Op::fnma_f32: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;

                case Op::sqrt_f32: write(o, R{d}, "=", op, R{x}); break;

                case Op::add_f32_imm: write(o, R{d}, "=", op, R{x}, Splat{immy}); break;
                case Op::sub_f32_imm: write(o, R{d}, "=", op, R{x}, Splat{immy}); break;
                case Op::mul_f32_imm: write(o, R{d}, "=", op, R{x}, Splat{immy}); break;
                case Op::min_f32_imm: write(o, R{d}, "=", op, R{x}, Splat{immy}); break;
                case Op::max_f32_imm: write(o, R{d}, "=", op, R{x}, Splat{immy}); break;

                case Op:: eq_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::neq_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op:: gt_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::gte_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;


                case Op::add_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::sub_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::mul_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::shl_i32: write(o, R{d}, "=", op, R{x}, Shift{immy}); break;
                case Op::shr_i32: write(o, R{d}, "=", op, R{x}, Shift{immy}); break;
                case Op::sra_i32: write(o, R{d}, "=", op, R{x}, Shift{immy}); break;

                case Op:: eq_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::neq_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op:: gt_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::gte_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;


                case Op::add_i16x2: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::sub_i16x2: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::mul_i16x2: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::shl_i16x2: write(o, R{d}, "=", op, R{x}, Shift{immy}); break;
                case Op::shr_i16x2: write(o, R{d}, "=", op, R{x}, Shift{immy}); break;
                case Op::sra_i16x2: write(o, R{d}, "=", op, R{x}, Shift{immy}); break;

                case Op:: eq_i16x2: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::neq_i16x2: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op:: gt_i16x2: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::gte_i16x2: write(o, R{d}, "=", op, R{x}, R{y}); break;


                case Op::bit_and  : write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::bit_or   : write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::bit_xor  : write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::bit_clear: write(o, R{d}, "=", op, R{x}, R{y}      ); break;

                case Op::bit_and_imm: write(o, R{d}, "=", op, R{x}, Hex{immy}); break;
                case Op::bit_or_imm : write(o, R{d}, "=", op, R{x}, Hex{immy}); break;
                case Op::bit_xor_imm: write(o, R{d}, "=", op, R{x}, Hex{immy}); break;

                case Op::select:  write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;
                case Op::bytes:   write(o, R{d}, "=", op,  R{x}, Hex{immy}); break;
                case Op::pack:    write(o, R{d}, "=", op,   R{x}, R{y}, Shift{immz}); break;

                case Op::floor:  write(o, R{d}, "=", op, R{x}); break;
                case Op::to_f32: write(o, R{d}, "=", op, R{x}); break;
                case Op::trunc:  write(o, R{d}, "=", op, R{x}); break;
                case Op::round:  write(o, R{d}, "=", op, R{x}); break;
            }
            write(o, "\n");
        }
    }

    void specialize_for_jit(std::vector<Instruction>* program) {
        Builder specialized;
        for (Val i = 0; i < (Val)program->size(); i++) {
            Instruction inst = (*program)[i];

            #if defined(SK_CPU_X86)
            auto is_imm = [&](Val id, int* bits) {
                *bits = (*program)[id].immy;
                return  (*program)[id].op == Op::splat;
            };

            switch (Op imm_op; inst.op) {
                default: break;

                case Op::add_f32: imm_op = Op::add_f32_imm; goto try_imm_x_and_y;
                case Op::mul_f32: imm_op = Op::mul_f32_imm; goto try_imm_x_and_y;
                case Op::min_f32: imm_op = Op::min_f32_imm; goto try_imm_x_and_y;
                case Op::max_f32: imm_op = Op::max_f32_imm; goto try_imm_x_and_y;
                case Op::bit_and: imm_op = Op::bit_and_imm; goto try_imm_x_and_y;
                case Op::bit_or:  imm_op = Op::bit_or_imm ; goto try_imm_x_and_y;
                case Op::bit_xor: imm_op = Op::bit_xor_imm; goto try_imm_x_and_y;

                try_imm_x_and_y:
                    if (int bits; is_imm(inst.x, &bits)) {
                        inst.op   = imm_op;
                        inst.x    = inst.y;
                        inst.y    = NA;
                        inst.immy = bits;
                    } else if (int bits; is_imm(inst.y, &bits)) {
                        inst.op   = imm_op;
                        inst.y    = NA;
                        inst.immy = bits;
                    } break;

                case Op::sub_f32:
                    if (int bits; is_imm(inst.y, &bits)) {
                        inst.op   = Op::sub_f32_imm;
                        inst.y    = NA;
                        inst.immy = bits;
                    } break;

                case Op::bit_clear:
                    if (int bits; is_imm(inst.y, &bits)) {
                        inst.op   = Op::bit_and_imm;
                        inst.y    = NA;
                        inst.immy = ~bits;
                    } break;
            }
            #endif
            SkDEBUGCODE(Val id =) specialized.push(inst);
            // If we replace single instructions with multiple, this will start breaking,
            // and we'll need a table to remap them like we have in optimize().
            SkASSERT(id == i);
        }

        *program = specialized.program();
    }

    std::vector<OptimizedInstruction> Builder::optimize(bool for_jit) const {
        std::vector<Instruction> program = this->program();
        if (for_jit) {
            specialize_for_jit(&program);
        }

        std::vector<bool> live_instructions;
        std::vector<Val> frontier;
        int liveInstructionCount = liveness_analysis(program, &live_instructions, &frontier);
        skvm::Usage usage{program, live_instructions};

        std::vector<int> remaining_uses;
        for (Val id = 0; id < (Val)program.size(); id++) {
            remaining_uses.push_back((int)usage.users(id).size());
        }

        // Map old Val index to rewritten index in optimized.
        std::vector<Val> new_index(program.size(), NA);

        auto pressure_change = [&](Val id) -> int {
            int pressure = 0;
            Instruction inst = program[id];

            // If this is not a sink, then it takes up a register
            if (inst.op > Op::store32) { pressure += 1; }

            // If this is the last use of the value, then that register will be free.
            if (inst.x != NA && remaining_uses[inst.x] == 1) { pressure -= 1; }
            if (inst.y != NA && remaining_uses[inst.y] == 1) { pressure -= 1; }
            if (inst.z != NA && remaining_uses[inst.z] == 1) { pressure -= 1; }
            return pressure;
        };

        auto compare = [&](Val lhs, Val rhs) {
            SkASSERT(lhs != rhs);
            int lhs_change = pressure_change(lhs);
            int rhs_change = pressure_change(rhs);

            // This comparison operator orders instructions from least (likely negative) register
            // pressure to most register pressure,  breaking ties arbitrarily using original
            // program order comparing the instruction index itself.
            //
            // We'll use this operator with std::{make,push,pop}_heap() to maintain a max heap
            // frontier of instructions that are ready to schedule.  We iterate backwards through
            // the program, scheduling later instruction slots before earlier ones, and that means
            // an instruction becomes ready to schedule once all instructions using its result have
            // been scheduled (in later slots).
            //
            // All together that means we'll be issuing the instructions that hurt register pressure
            // as late as possible, and issuing the instructions that help register pressure as soon
            // as possible.
            //
            // This heuristic of greedily issuing the instruction that most immediately decreases
            // register pressure approximates a more expensive search to find a schedule that
            // minimizes the high-water maximum register pressure, the number of registers we'll
            // need to run this program.
            //
            // The tie-breaker heuristic was found through experimentation.
            return lhs_change < rhs_change || (lhs_change == rhs_change && lhs > rhs);
        };

        // Order the instructions.
        std::make_heap(frontier.begin(), frontier.end(), compare);

        // Schedule the instructions last to first from the DAG. Produce a schedule that executes
        // instructions that reduce register pressure before ones that increase register
        // pressure.
        std::vector<OptimizedInstruction> optimized;
        optimized.resize(liveInstructionCount);
        for (int i = liveInstructionCount; i-- > 0;) {
            SkASSERT(!frontier.empty());
            std::pop_heap(frontier.begin(), frontier.end(), compare);
            Val id = frontier.back();
            frontier.pop_back();
            new_index[id] = i;
            Instruction inst = program[id];
            SkASSERT(remaining_uses[id] == 0);

            // Use the old indices, and fix them up later.
            optimized[i] = {inst.op,
                            inst.x, inst.y, inst.z,
                            inst.immy, inst.immz,
                             /*death=*/0, /*can_hoist=*/true, /*used_in_loop=*/false};

            auto maybe_issue = [&](Val input) {
              if (input != NA) {
                  if (remaining_uses[input] == 1) {
                      frontier.push_back(input);
                      std::push_heap(frontier.begin(), frontier.end(), compare);
                  }
                  remaining_uses[input]--;
              }
            };
            maybe_issue(inst.x);
            maybe_issue(inst.y);
            maybe_issue(inst.z);
        }

        // Fix up the optimized program to use the optimized indices.
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            OptimizedInstruction& inst = optimized[id];
            if (inst.x != NA ) { inst.x = new_index[inst.x]; }
            if (inst.y != NA ) { inst.y = new_index[inst.y]; }
            if (inst.z != NA ) { inst.z = new_index[inst.z]; }
        }

        SkASSERT(frontier.empty());

        // We're done with `program` now... everything below will analyze `optimized`.

        // We'll want to know when it's safe to recycle registers holding the values
        // produced by each instruction, that is, when no future instruction needs it.
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            OptimizedInstruction& inst = optimized[id];
            // Stores don't really produce values.  Just mark them as dying on issue.
            if (inst.op <= Op::store32) {
                inst.death = id;
            }
            // Extend the lifetime of this instruction's inputs to live until it issues.
            // (We're walking in order, so this is the same as max()ing.)
            if (inst.x != NA) { optimized[inst.x].death = id; }
            if (inst.y != NA) { optimized[inst.y].death = id; }
            if (inst.z != NA) { optimized[inst.z].death = id; }
        }

        // Mark which values don't depend on the loop and can be hoisted.
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            OptimizedInstruction& inst = optimized[id];

            // Varying loads (and gathers) and stores cannot be hoisted out of the loop.
            if (inst.op <= Op::gather32 && inst.op != Op::assert_true) {
                inst.can_hoist = false;
            }

            // If any of an instruction's inputs can't be hoisted, it can't be hoisted itself.
            if (inst.can_hoist) {
                if (inst.x != NA) { inst.can_hoist &= optimized[inst.x].can_hoist; }
                if (inst.y != NA) { inst.can_hoist &= optimized[inst.y].can_hoist; }
                if (inst.z != NA) { inst.can_hoist &= optimized[inst.z].can_hoist; }
            }

            // We'll want to know if hoisted values are used in the loop;
            // if not, we can recycle their registers like we do loop values.
            if (!inst.can_hoist /*i.e. we're in the loop, so the arguments are used_in_loop*/) {
                if (inst.x != NA) { optimized[inst.x].used_in_loop = true; }
                if (inst.y != NA) { optimized[inst.y].used_in_loop = true; }
                if (inst.z != NA) { optimized[inst.z].used_in_loop = true; }
            }
        }

        return optimized;
    }

    Program Builder::done(const char* debug_name) const {
        char buf[64] = "skvm-jit-";
        if (!debug_name) {
            *SkStrAppendU32(buf+9, this->hash()) = '\0';
            debug_name = buf;
        }

    #if defined(SKVM_LLVM) || defined(SKVM_JIT)
        return {this->optimize(false), this->optimize(true), fStrides, debug_name};
    #else
        return {this->optimize(false), fStrides};
    #endif
    }

    uint64_t Builder::hash() const {
        uint32_t lo = SkOpts::hash(fProgram.data(), fProgram.size() * sizeof(Instruction), 0),
                 hi = SkOpts::hash(fProgram.data(), fProgram.size() * sizeof(Instruction), 1);
        return (uint64_t)lo | (uint64_t)hi << 32;
    }

    bool operator==(const Instruction& a, const Instruction& b) {
        return a.op   == b.op
            && a.x    == b.x
            && a.y    == b.y
            && a.z    == b.z
            && a.immy == b.immy
            && a.immz == b.immz;
    }

    uint32_t InstructionHash::operator()(const Instruction& inst, uint32_t seed) const {
        return SkOpts::hash(&inst, sizeof(inst), seed);
    }


    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.
    Val Builder::push(Instruction inst) {
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

    bool Builder::allImm() const { return true; }

    template <typename T, typename... Rest>
    bool Builder::allImm(Val id, T* imm, Rest... rest) const {
        if (fProgram[id].op == Op::splat) {
            static_assert(sizeof(T) == 4);
            memcpy(imm, &fProgram[id].immy, 4);
            return this->allImm(rest...);
        }
        return false;
    }

    Arg Builder::arg(int stride) {
        int ix = (int)fStrides.size();
        fStrides.push_back(stride);
        return {ix};
    }

    void Builder::assert_true(I32 cond, I32 debug) {
    #ifdef SK_DEBUG
        int imm;
        if (this->allImm(cond.id,&imm)) { SkASSERT(imm); return; }
        (void)push(Op::assert_true, cond.id,debug.id,NA);
    #endif
    }

    void Builder::store8 (Arg ptr, I32 val) { (void)push(Op::store8 , val.id,NA,NA, ptr.ix); }
    void Builder::store16(Arg ptr, I32 val) { (void)push(Op::store16, val.id,NA,NA, ptr.ix); }
    void Builder::store32(Arg ptr, I32 val) { (void)push(Op::store32, val.id,NA,NA, ptr.ix); }

    I32 Builder::index() { return {this, push(Op::index , NA,NA,NA,0) }; }

    I32 Builder::load8 (Arg ptr) { return {this, push(Op::load8 , NA,NA,NA, ptr.ix) }; }
    I32 Builder::load16(Arg ptr) { return {this, push(Op::load16, NA,NA,NA, ptr.ix) }; }
    I32 Builder::load32(Arg ptr) { return {this, push(Op::load32, NA,NA,NA, ptr.ix) }; }

    I32 Builder::gather8 (Arg ptr, int offset, I32 index) {
        return {this, push(Op::gather8 , index.id,NA,NA, ptr.ix,offset)};
    }
    I32 Builder::gather16(Arg ptr, int offset, I32 index) {
        return {this, push(Op::gather16, index.id,NA,NA, ptr.ix,offset)};
    }
    I32 Builder::gather32(Arg ptr, int offset, I32 index) {
        return {this, push(Op::gather32, index.id,NA,NA, ptr.ix,offset)};
    }

    I32 Builder::uniform8(Arg ptr, int offset) {
        return {this, push(Op::uniform8, NA,NA,NA, ptr.ix, offset)};
    }
    I32 Builder::uniform16(Arg ptr, int offset) {
        return {this, push(Op::uniform16, NA,NA,NA, ptr.ix, offset)};
    }
    I32 Builder::uniform32(Arg ptr, int offset) {
        return {this, push(Op::uniform32, NA,NA,NA, ptr.ix, offset)};
    }

    // The two splat() functions are just syntax sugar over splatting a 4-byte bit pattern.
    I32 Builder::splat(int   n) { return {this, push(Op::splat, NA,NA,NA, n) }; }
    F32 Builder::splat(float f) {
        int bits;
        memcpy(&bits, &f, 4);
        return {this, push(Op::splat, NA,NA,NA, bits)};
    }

    static bool fma_supported() {
        static const bool supported =
     #if defined(SK_CPU_X86)
         SkCpu::Supports(SkCpu::HSW);
     #elif defined(SK_CPU_ARM64)
         true;
     #else
         false;
     #endif
         return supported;
    }

    // Be careful peepholing float math!  Transformations you might expect to
    // be legal can fail in the face of NaN/Inf, e.g. 0*x is not always 0.
    // Float peepholes must pass this equivalence test for all ~4B floats:
    //
    //     bool equiv(float x, float y) { return (x == y) || (isnanf(x) && isnanf(y)); }
    //
    //     unsigned bits = 0;
    //     do {
    //        float f;
    //        memcpy(&f, &bits, 4);
    //        if (!equiv(f, ...)) {
    //           abort();
    //        }
    //     } while (++bits != 0);

    F32 Builder::add(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X+Y); }
        if (this->isImm(y.id, 0.0f)) { return x; }   // x+0 == x
        if (this->isImm(x.id, 0.0f)) { return y; }   // 0+y == y

        if (fma_supported()) {
            if (fProgram[x.id].op == Op::mul_f32) {
                return {this, push(Op::fma_f32, fProgram[x.id].x, fProgram[x.id].y, y.id)};
            }
            if (fProgram[y.id].op == Op::mul_f32) {
                return {this, push(Op::fma_f32, fProgram[y.id].x, fProgram[y.id].y, x.id)};
            }
        }
        return {this, push(Op::add_f32, x.id, y.id)};
    }

    F32 Builder::sub(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X-Y); }
        if (this->isImm(y.id, 0.0f)) { return x; }   // x-0 == x
        if (fma_supported()) {
            if (fProgram[x.id].op == Op::mul_f32) {
                return {this, push(Op::fms_f32, fProgram[x.id].x, fProgram[x.id].y, y.id)};
            }
            if (fProgram[y.id].op == Op::mul_f32) {
                return {this, push(Op::fnma_f32, fProgram[y.id].x, fProgram[y.id].y, x.id)};
            }
        }
        return {this, push(Op::sub_f32, x.id, y.id)};
    }

    F32 Builder::mul(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X*Y); }
        if (this->isImm(y.id, 1.0f)) { return x; }  // x*1 == x
        if (this->isImm(x.id, 1.0f)) { return y; }  // 1*y == y
        return {this, push(Op::mul_f32, x.id, y.id)};
    }

    F32 Builder::div(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X/Y); }
        if (this->isImm(y.id, 1.0f)) { return x; }  // x/1 == x
        return {this, push(Op::div_f32, x.id, y.id)};
    }

    F32 Builder::sqrt(F32 x) {
        float X;
        if (this->allImm(x.id,&X)) { return this->splat(std::sqrt(X)); }
        return {this, push(Op::sqrt_f32, x.id,NA,NA)};
    }

    // See http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html.
    F32 Builder::approx_log2(F32 x) {
        // e - 127 is a fair approximation of log2(x) in its own right...
        F32 e = mul(to_f32(bit_cast(x)), splat(1.0f / (1<<23)));

        // ... but using the mantissa to refine its error is _much_ better.
        F32 m = bit_cast(bit_or(bit_and(bit_cast(x), 0x007fffff),
                                0x3f000000));
        F32 approx = sub(e,        124.225514990f);
            approx = sub(approx, mul(1.498030302f, m));
            approx = sub(approx, div(1.725879990f, add(0.3520887068f, m)));

        return approx;
    }

    F32 Builder::approx_pow2(F32 x) {
        F32 f = fract(x);
        F32 approx = add(x,         121.274057500f);
            approx = sub(approx, mul( 1.490129070f, f));
            approx = add(approx, div(27.728023300f, sub(4.84252568f, f)));

        return bit_cast(round(mul(1.0f * (1<<23), approx)));
    }

    F32 Builder::approx_powf(F32 x, F32 y) {
        auto is_x = bit_or(eq(x, 0.0f),
                           eq(x, 1.0f));
        return select(is_x, x, approx_pow2(mul(approx_log2(x), y)));
    }

    F32 Builder::min(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(std::min(X,Y)); }
        return {this, push(Op::min_f32, x.id, y.id)};
    }
    F32 Builder::max(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(std::max(X,Y)); }
        return {this, push(Op::max_f32, x.id, y.id)};
    }

    I32 Builder::add(I32 x, I32 y) { return {this, push(Op::add_i32, x.id, y.id)}; }
    I32 Builder::sub(I32 x, I32 y) { return {this, push(Op::sub_i32, x.id, y.id)}; }
    I32 Builder::mul(I32 x, I32 y) { return {this, push(Op::mul_i32, x.id, y.id)}; }

    I32 Builder::add_16x2(I32 x, I32 y) { return {this, push(Op::add_i16x2, x.id, y.id)}; }
    I32 Builder::sub_16x2(I32 x, I32 y) { return {this, push(Op::sub_i16x2, x.id, y.id)}; }
    I32 Builder::mul_16x2(I32 x, I32 y) { return {this, push(Op::mul_i16x2, x.id, y.id)}; }

    I32 Builder::shl(I32 x, int bits) {
        if (bits == 0) { return x; }
        int X;
        if (this->allImm(x.id,&X)) { return this->splat(X << bits); }
        return {this, push(Op::shl_i32, x.id,NA,NA, bits)};
    }
    I32 Builder::shr(I32 x, int bits) {
        if (bits == 0) { return x; }
        int X;
        if (this->allImm(x.id,&X)) { return this->splat(unsigned(X) >> bits); }
        return {this, push(Op::shr_i32, x.id,NA,NA, bits)};
    }
    I32 Builder::sra(I32 x, int bits) {
        if (bits == 0) { return x; }
        int X;
        if (this->allImm(x.id,&X)) { return this->splat(X >> bits); }
        return {this, push(Op::sra_i32, x.id,NA,NA, bits)};
    }

    I32 Builder::shl_16x2(I32 x, int k) { return {this, push(Op::shl_i16x2, x.id,NA,NA, k)}; }
    I32 Builder::shr_16x2(I32 x, int k) { return {this, push(Op::shr_i16x2, x.id,NA,NA, k)}; }
    I32 Builder::sra_16x2(I32 x, int k) { return {this, push(Op::sra_i16x2, x.id,NA,NA, k)}; }

    I32 Builder:: eq(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X==Y ? ~0 : 0); }
        return {this, push(Op::eq_f32, x.id, y.id)};
    }
    I32 Builder::neq(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X!=Y ? ~0 : 0); }
        return {this, push(Op::neq_f32, x.id, y.id)};
    }
    I32 Builder::lt(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(Y> X ? ~0 : 0); }
        return {this, push(Op::gt_f32, y.id, x.id)};
    }
    I32 Builder::lte(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(Y>=X ? ~0 : 0); }
        return {this, push(Op::gte_f32, y.id, x.id)};
    }
    I32 Builder::gt(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X> Y ? ~0 : 0); }
        return {this, push(Op::gt_f32, x.id, y.id)};
    }
    I32 Builder::gte(F32 x, F32 y) {
        float X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X>=Y ? ~0 : 0); }
        return {this, push(Op::gte_f32, x.id, y.id)};
    }

    I32 Builder:: eq(I32 x, I32 y) { return {this, push(Op:: eq_i32, x.id, y.id)}; }
    I32 Builder::neq(I32 x, I32 y) { return {this, push(Op::neq_i32, x.id, y.id)}; }
    I32 Builder:: lt(I32 x, I32 y) { return {this, push(Op:: gt_i32, y.id, x.id)}; }
    I32 Builder::lte(I32 x, I32 y) { return {this, push(Op::gte_i32, y.id, x.id)}; }
    I32 Builder:: gt(I32 x, I32 y) { return {this, push(Op:: gt_i32, x.id, y.id)}; }
    I32 Builder::gte(I32 x, I32 y) { return {this, push(Op::gte_i32, x.id, y.id)}; }

    I32 Builder:: eq_16x2(I32 x, I32 y) { return {this, push(Op:: eq_i16x2, x.id, y.id)}; }
    I32 Builder::neq_16x2(I32 x, I32 y) { return {this, push(Op::neq_i16x2, x.id, y.id)}; }
    I32 Builder:: lt_16x2(I32 x, I32 y) { return {this, push(Op:: gt_i16x2, y.id, x.id)}; }
    I32 Builder::lte_16x2(I32 x, I32 y) { return {this, push(Op::gte_i16x2, y.id, x.id)}; }
    I32 Builder:: gt_16x2(I32 x, I32 y) { return {this, push(Op:: gt_i16x2, x.id, y.id)}; }
    I32 Builder::gte_16x2(I32 x, I32 y) { return {this, push(Op::gte_i16x2, x.id, y.id)}; }

    I32 Builder::bit_and(I32 x, I32 y) {
        int X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X&Y); }
        if (this->isImm(y.id, 0)) { return this->splat(0); }   // (x & false) == false
        if (this->isImm(x.id, 0)) { return this->splat(0); }   // (false & y) == false
        if (this->isImm(y.id,~0)) { return x; }                // (x & true) == x
        if (this->isImm(x.id,~0)) { return y; }                // (true & y) == y
        return {this, push(Op::bit_and, x.id, y.id)};
    }
    I32 Builder::bit_or(I32 x, I32 y) {
        int X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X|Y); }
        if (this->isImm(y.id, 0)) { return x; }                 // (x | false) == x
        if (this->isImm(x.id, 0)) { return y; }                 // (false | y) == y
        if (this->isImm(y.id,~0)) { return this->splat(~0); }   // (x | true) == true
        if (this->isImm(x.id,~0)) { return this->splat(~0); }   // (true | y) == true
        return {this, push(Op::bit_or, x.id, y.id)};
    }
    I32 Builder::bit_xor(I32 x, I32 y) {
        int X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X^Y); }
        if (this->isImm(y.id, 0)) { return x; }   // (x ^ false) == x
        if (this->isImm(x.id, 0)) { return y; }   // (false ^ y) == y
        return {this, push(Op::bit_xor, x.id, y.id)};
    }
    I32 Builder::bit_clear(I32 x, I32 y) {
        int X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X&~Y); }
        if (this->isImm(y.id, 0)) { return x; }                // (x & ~false) == x
        if (this->isImm(y.id,~0)) { return this->splat(0); }   // (x & ~true) == false
        if (this->isImm(x.id, 0)) { return this->splat(0); }   // (false & ~y) == false
        return {this, push(Op::bit_clear, x.id, y.id)};
    }

    I32 Builder::select(I32 x, I32 y, I32 z) {
        int X,Y,Z;
        if (this->allImm(x.id,&X, y.id,&Y, z.id,&Z)) { return this->splat(X?Y:Z); }
        // TODO: some cases to reduce to bit_and when y == 0 or z == 0?
        return {this, push(Op::select, x.id, y.id, z.id)};
    }

    I32 Builder::extract(I32 x, int bits, I32 z) {
        int Z;
        if (this->allImm(z.id,&Z) && (~0u>>bits) == (unsigned)Z) { return this->shr(x, bits); }
        return this->bit_and(z, this->shr(x, bits));
    }

    I32 Builder::pack(I32 x, I32 y, int bits) {
        int X,Y;
        if (this->allImm(x.id,&X, y.id,&Y)) { return this->splat(X|(Y<<bits)); }
        return {this, push(Op::pack, x.id,y.id,NA, 0,bits)};
    }

    I32 Builder::bytes(I32 x, int control) {
        return {this, push(Op::bytes, x.id,NA,NA, control)};
    }

    F32 Builder::floor(F32 x) {
        float X;
        if (this->allImm(x.id,&X)) { return this->splat(floorf(X)); }
        return {this, push(Op::floor, x.id)};
    }
    F32 Builder::to_f32(I32 x) {
        int X;
        if (this->allImm(x.id,&X)) { return this->splat((float)X); }
        return {this, push(Op::to_f32, x.id)};
    }
    I32 Builder::trunc(F32 x) {
        float X;
        if (this->allImm(x.id,&X)) { return this->splat((int)X); }
        return {this, push(Op::trunc, x.id)};
    }
    I32 Builder::round(F32 x) {
        float X;
        if (this->allImm(x.id,&X)) { return this->splat((int)lrintf(X)); }
        return {this, push(Op::round, x.id)};
    }

    F32 Builder::from_unorm(int bits, I32 x) {
        F32 limit = splat(1 / ((1<<bits)-1.0f));
        return mul(to_f32(x), limit);
    }
    I32 Builder::to_unorm(int bits, F32 x) {
        F32 limit = splat((1<<bits)-1.0f);
        return round(mul(x, limit));
    }

    Color Builder::unpack_1010102(I32 rgba) {
        return {
            from_unorm(10, extract(rgba,  0, 0x3ff)),
            from_unorm(10, extract(rgba, 10, 0x3ff)),
            from_unorm(10, extract(rgba, 20, 0x3ff)),
            from_unorm( 2, extract(rgba, 30, 0x3  )),
        };
    }
    Color Builder::unpack_8888(I32 rgba) {
        return {
            from_unorm(8, extract(rgba,  0, 0xff)),
            from_unorm(8, extract(rgba,  8, 0xff)),
            from_unorm(8, extract(rgba, 16, 0xff)),
            from_unorm(8, extract(rgba, 24, 0xff)),
        };
    }
    Color Builder::unpack_565(I32 bgr) {
        return {
            from_unorm(5, extract(bgr, 11, 0b011'111)),
            from_unorm(6, extract(bgr,  5, 0b111'111)),
            from_unorm(5, extract(bgr,  0, 0b011'111)),
            splat(1.0f),
        };
    }

    void Builder::unpremul(F32* r, F32* g, F32* b, F32 a) {
        skvm::F32 invA = div(1.0f, a),
                  inf  = bit_cast(splat(0x7f800000));
        // If a is 0, so are *r,*g,*b, so set invA to 0 to avoid 0*inf=NaN (instead 0*0 = 0).
        invA = bit_cast(bit_and(lt(invA, inf),
                                bit_cast(invA)));
        *r = mul(*r, invA);
        *g = mul(*g, invA);
        *b = mul(*b, invA);
    }

    void Builder::premul(F32* r, F32* g, F32* b, F32 a) {
        *r = mul(*r, a);
        *g = mul(*g, a);
        *b = mul(*b, a);
    }

    Color Builder::uniformPremul(SkColor4f color,    SkColorSpace* src,
                                 Uniforms* uniforms, SkColorSpace* dst) {
        SkColorSpaceXformSteps(src, kUnpremul_SkAlphaType,
                               dst,   kPremul_SkAlphaType).apply(color.vec());
        return {
            uniformF(uniforms->pushF(color.fR)),
            uniformF(uniforms->pushF(color.fG)),
            uniformF(uniforms->pushF(color.fB)),
            uniformF(uniforms->pushF(color.fA)),
        };
    }

    Color Builder::lerp(Color lo, Color hi, F32 t) {
        return {
            lerp(lo.r, hi.r, t),
            lerp(lo.g, hi.g, t),
            lerp(lo.b, hi.b, t),
            lerp(lo.a, hi.a, t),
        };
    }

    HSLA Builder::to_hsla(Color c) {
        F32 mx = max(max(c.r,c.g),c.b),
            mn = min(min(c.r,c.g),c.b),
             d = mx - mn,
        g_lt_b = select(c.g < c.b, splat(6.0f)
                                 , splat(0.0f));

        auto diffm = [&](auto a, auto b) {
            return (a - b) * (1 / d);
        };

        F32 h = mul(1/6.0f,
                        select(eq(mx,  mn), 0.0f,
                        select(eq(mx, c.r), add(diffm(c.g,c.b), g_lt_b),
                        select(eq(mx, c.g), add(diffm(c.b,c.r), 2.0f)
                                          , add(diffm(c.r,c.g), 4.0f)))));

        F32 sum = add(mx,mn);
        F32   l = mul(sum, 0.5f);
        F32   s = select(eq(mx,mn), 0.0f
                                  , div(d, select(gt(l,0.5f), sub(2.0f,sum)
                                                            , sum)));
        return {h, s, l, c.a};
    }

    Color Builder::to_rgba(HSLA c) {
        // See GrRGBToHSLFilterEffect.fp

        auto [h,s,l,a] = c;
        F32 x = mul(sub(1.0f, abs(sub(add(l,l), 1.0f))), s);

        auto hue_to_rgb = [&,l=l](auto hue) {
            auto q = sub(abs(mad(fract(hue), 6.0f, -3.0f)), 1.0f);
            return mad(sub(clamp01(q), 0.5f), x, l);
        };

        return {
            hue_to_rgb(add(h, 0/3.0f)),
            hue_to_rgb(add(h, 2/3.0f)),
            hue_to_rgb(add(h, 1/3.0f)),
            c.a,
        };
    }

    // We're basing our implementation of non-separable blend modes on
    //   https://www.w3.org/TR/compositing-1/#blendingnonseparable.
    // and
    //   https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
    // They're equivalent, but ES' math has been better simplified.
    //
    // Anything extra we add beyond that is to make the math work with premul inputs.

    static skvm::F32 saturation(skvm::Builder* p, skvm::F32 r, skvm::F32 g, skvm::F32 b) {
        return max(r, max(g, b))
             - min(r, min(g, b));
    }

    static skvm::F32 luminance(skvm::Builder* p, skvm::F32 r, skvm::F32 g, skvm::F32 b) {
        return r*0.30f + (g*0.59f + b*0.11f);
    }

    static void set_sat(skvm::Builder* p, skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 s) {
        F32 mn  = min(*r, min(*g, *b)),
            mx  = max(*r, max(*g, *b)),
            sat = mx - mn;

        // Map min channel to 0, max channel to s, and scale the middle proportionally.
        auto scale = [&](auto c) {
            // TODO: better to divide and check for non-finite result?
            return select(sat == 0.0f, 0.0f
                                     , ((c - mn) * s) / sat);
        };
        *r = scale(*r);
        *g = scale(*g);
        *b = scale(*b);
    }

    static void set_lum(skvm::Builder* p, skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 lu) {
        auto diff = lu - luminance(p, *r, *g, *b);
        *r += diff;
        *g += diff;
        *b += diff;
    }

    static void clip_color(skvm::Builder* p,
                           skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 a) {
        F32 mn  = min(*r, min(*g, *b)),
            mx  = max(*r, max(*g, *b)),
            lu = luminance(p, *r, *g, *b);

        auto clip = [&](auto c) {
            c = select(mn >= 0, c
                              , lu + ((c-lu)*(  lu)) / (lu-mn));
            c = select(mx >  a, lu + ((c-lu)*(a-lu)) / (mx-lu)
                              , c);
            // Sometimes without this we may dip just a little negative.
            return max(c, 0.0f);
        };
        *r = clip(*r);
        *g = clip(*g);
        *b = clip(*b);
    }

    Color Builder::blend(SkBlendMode mode, Color src, Color dst) {
        auto mma = [this](skvm::F32 x, skvm::F32 y, skvm::F32 z, skvm::F32 w) {
            return mad(x,y, mul(z,w));
        };

        auto two = [this](skvm::F32 x) { return add(x, x); };

        auto apply_rgba = [&](auto fn) {
            return Color {
                fn(src.r, dst.r),
                fn(src.g, dst.g),
                fn(src.b, dst.b),
                fn(src.a, dst.a),
            };
        };

        auto apply_rgb_srcover_a = [&](auto fn) {
            return Color {
                fn(src.r, dst.r),
                fn(src.g, dst.g),
                fn(src.b, dst.b),
                mad(dst.a, 1-src.a, src.a),   // srcover for alpha
            };
        };

        auto non_sep = [&](auto R, auto G, auto B) {
            return Color{
                add(mma(src.r, 1-dst.a,  dst.r, 1-src.a), R),
                add(mma(src.g, 1-dst.a,  dst.g, 1-src.a), G),
                add(mma(src.b, 1-dst.a,  dst.b, 1-src.a), B),
                mad(dst.a,1-src.a, src.a),  // srcover
            };
        };

        switch (mode) {
            default: SkASSERT(false); /*but also, for safety, fallthrough*/

            case SkBlendMode::kClear: return { splat(0.0f), splat(0.0f), splat(0.0f), splat(0.0f) };

            case SkBlendMode::kSrc: return src;
            case SkBlendMode::kDst: return dst;

            case SkBlendMode::kDstOver: std::swap(src, dst); // fall-through
            case SkBlendMode::kSrcOver:
                return apply_rgba([&](auto s, auto d) {
                    return mad(d,1-src.a, s);
                });

            case SkBlendMode::kDstIn: std::swap(src, dst); // fall-through
            case SkBlendMode::kSrcIn:
                return apply_rgba([&](auto s, auto d) {
                    return mul(s, dst.a);
                });

            case SkBlendMode::kDstOut: std::swap(src, dst); // fall-through
            case SkBlendMode::kSrcOut:
                return apply_rgba([&](auto s, auto d) {
                    return mul(s, 1-dst.a);
                });

            case SkBlendMode::kDstATop: std::swap(src, dst); // fall-through
            case SkBlendMode::kSrcATop:
                return apply_rgba([&](auto s, auto d) {
                    return mma(s, dst.a,  d, 1-src.a);
                });

            case SkBlendMode::kXor:
                return apply_rgba([&](auto s, auto d) {
                    return mma(s, 1-dst.a,  d, 1-src.a);
                });

            case SkBlendMode::kPlus:
                return apply_rgba([&](auto s, auto d) {
                    return min(add(s, d), 1.0f);
                });

            case SkBlendMode::kModulate:
                return apply_rgba([&](auto s, auto d) {
                    return mul(s, d);
                });

            case SkBlendMode::kScreen:
                // (s+d)-(s*d) gave us trouble with our "r,g,b <= after blending" asserts.
                // It's kind of plausible that s + (d - sd) keeps more precision?
                return apply_rgba([&](auto s, auto d) {
                    return add(s, sub(d, mul(s, d)));
                });

            case SkBlendMode::kDarken:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return add(s, sub(d, max(mul(s, dst.a),
                                             mul(d, src.a))));
                });

            case SkBlendMode::kLighten:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return add(s, sub(d, min(mul(s, dst.a),
                                             mul(d, src.a))));
                });

            case SkBlendMode::kDifference:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return add(s, sub(d, two(min(mul(s, dst.a),
                                                 mul(d, src.a)))));
                });

            case SkBlendMode::kExclusion:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return add(s, sub(d, two(mul(s, d))));
                });

            case SkBlendMode::kColorBurn:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    // TODO: divide and check for non-finite result instead of checking for s == 0.
                    auto mn   = min(dst.a,
                                    div(mul(sub(dst.a, d), src.a), s)),
                         burn = mad(src.a, sub(dst.a, mn), mma(s, 1-dst.a, d, 1-src.a));
                    return select(eq(d, dst.a), mad(s, 1-dst.a, d),
                           select(eq(s,  0.0f), mul(d, 1-src.a)
                                              , burn));
                });

            case SkBlendMode::kColorDodge:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    // TODO: divide and check for non-finite result instead of checking for s == sa.
                    auto dodge = mad(src.a, min(dst.a,
                                                div(mul(d, src.a), sub(src.a, s))),
                                     mma(s, 1-dst.a, d, 1-src.a));
                    return select(eq(d,  0.0f), mul(s, 1-dst.a),
                           select(eq(s, src.a), mad(d, 1-src.a, s)
                                              , dodge));
                });

            case SkBlendMode::kHardLight:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return add(mma(s, 1-dst.a, d, 1-src.a),
                               select(lte(two(s), src.a),
                                      two(mul(s, d)),
                                      sub(mul(src.a, dst.a), two(mul(sub(dst.a, d), sub(src.a, s))))));
                });

            case SkBlendMode::kOverlay:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return add(mma(s, 1-dst.a, d, 1-src.a),
                               select(lte(two(d), dst.a),
                                      two(mul(s, d)),
                                      sub(mul(src.a, dst.a), two(mul(sub(dst.a, d), sub(src.a, s))))));
                });

            case SkBlendMode::kMultiply:
                return apply_rgba([&](auto s, auto d) {
                    return add(mma(s, 1-dst.a, d, 1-src.a), mul(s, d));
                });

            case SkBlendMode::kSoftLight:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    auto  m = select(gt(dst.a, 0.0f), div(d, dst.a), 0.0f),
                         s2 = two(s),
                         m4 = two(two(m));

                         // The logic forks three ways:
                         //    1. dark src?
                         //    2. light src, dark dst?
                         //    3. light src, light dst?

                         // Used in case 1
                    auto darkSrc = mul(d, mad(sub(s2, src.a), 1-m, src.a)),
                         // Used in case 2
                         darkDst = mad(mad(m4, m4, m4), sub(m, 1.0f), mul(7.0f, m)),
                         // Used in case 3.
                         liteDst = sub(sqrt(m), m),
                         // Used in 2 or 3?
                         liteSrc = mad(mul(dst.a, sub(s2, src.a)),
                                       select(lte(two(two(d)), dst.a), darkDst, liteDst),
                                       mul(d, src.a));
                    return mad(s, 1-dst.a, mad(d,
                                               1-src.a,
                                               select(lte(s2, src.a), darkSrc, liteSrc)));


                });

            case SkBlendMode::kHue: {
                skvm::F32 R = mul(src.r, src.a),
                          G = mul(src.g, src.a),
                          B = mul(src.b, src.a);

                set_sat(this, &R, &G, &B, mul(saturation(this, dst.r, dst.g, dst.b), src.a));
                set_lum(this, &R, &G, &B, mul( luminance(this, dst.r, dst.g, dst.b), src.a));
                clip_color(this, &R, &G, &B, mul(src.a, dst.a));

                return non_sep(R, G, B);
            }

            case SkBlendMode::kSaturation: {
                skvm::F32 R = mul(dst.r, src.a),
                          G = mul(dst.g, src.a),
                          B = mul(dst.b, src.a);

                set_sat(this, &R, &G, &B, mul(saturation(this, src.r, src.g, src.b), dst.a));
                set_lum(this, &R, &G, &B, mul( luminance(this, dst.r, dst.g, dst.b), src.a));
                clip_color(this, &R, &G, &B, mul(src.a, dst.a));

                return non_sep(R, G, B);
            }

            case SkBlendMode::kColor: {
                skvm::F32 R = mul(src.r, dst.a),
                          G = mul(src.g, dst.a),
                          B = mul(src.b, dst.a);

                set_lum(this, &R, &G, &B, mul(luminance(this, dst.r, dst.g, dst.b), src.a));
                clip_color(this, &R, &G, &B, mul(src.a, dst.a));

                return non_sep(R, G, B);
            }

            case SkBlendMode::kLuminosity: {
                skvm::F32 R = mul(dst.r, src.a),
                          G = mul(dst.g, src.a),
                          B = mul(dst.b, src.a);

                set_lum(this, &R, &G, &B, mul(luminance(this, src.r, src.g, src.b), dst.a));
                clip_color(this, &R, &G, &B, mul(src.a, dst.a));

                return non_sep(R, G, B);
            }
        }
    }

    // Fill live and sinks each if non-null:
    //    - (*live)[id]: notes whether each input instruction is live
    //    - *sinks: an unsorted set of live instructions with side effects (stores, assert_true)
    // Returns the number of live instructions.
    int liveness_analysis(const std::vector<Instruction>& instructions,
                          std::vector<bool>* live,
                          std::vector<Val>*  sinks) {
        int instruction_count = instructions.size();
        live->resize(instruction_count, false);
        int liveInstructionCount = 0;
        auto trace = [&](Val id, auto& recurse) -> void {
          if (!(*live)[id]) {
              (*live)[id] = true;
              liveInstructionCount++;
              Instruction inst = instructions[id];
              if (inst.x != NA) { recurse(inst.x, recurse); }
              if (inst.y != NA) { recurse(inst.y, recurse); }
              if (inst.z != NA) { recurse(inst.z, recurse); }
          }
        };

        // For all the sink instructions.
        for (Val id = 0; id < instruction_count; id++) {
            if (instructions[id].op <= skvm::Op::store32) {
                sinks->push_back(id);
                trace(id, trace);
            }
        }
        return liveInstructionCount;
    }

    // For a given program we'll store each Instruction's users contiguously in a table,
    // and track where each Instruction's span of users starts and ends in another index.
    // Here's a simple program that loads x and stores kx+k:
    //
    //  v0 = splat(k)
    //  v1 = load(...)
    //  v2 = mul(v1, v0)
    //  v3 = add(v2, v0)
    //  v4 = store(..., v3)
    //
    // This program has 5 instructions v0-v4.
    //    - v0 is used by v2 and v3
    //    - v1 is used by v2
    //    - v2 is used by v3
    //    - v3 is used by v4
    //    - v4 has a side-effect
    //
    // For this program we fill out these two arrays:
    //     table:  [v2,v3, v2, v3, v4]
    //     index:  [0,     2,  3,  4,  5]
    //
    // The table is just those "is used by ..." I wrote out above in order,
    // and the index tracks where an Instruction's span of users starts, table[index[id]].
    // The span continues up until the start of the next Instruction, table[index[id+1]].
    SkSpan<const Val> Usage::users(Val id) const {
        int begin = fIndex[id];
        int end   = fIndex[id + 1];
        return SkMakeSpan(fTable.data() + begin, end - begin);
    }

    Usage::Usage(const std::vector<Instruction>& program, const std::vector<bool>& live) {
        // uses[id] counts the number of times each Instruction is used.
        std::vector<int> uses(program.size(), 0);
        for (Val id = 0; id < (Val)program.size(); id++) {
            if (live[id]) {
                Instruction inst = program[id];
                if (inst.x != NA) { ++uses[inst.x]; }
                if (inst.y != NA) { ++uses[inst.y]; }
                if (inst.z != NA) { ++uses[inst.z]; }
            }
        }

        // Build our index into fTable, with an extra entry marking the final Instruction's end.
        fIndex.reserve(program.size() + 1);
        int total_uses = 0;
        for (int n : uses) {
            fIndex.push_back(total_uses);
            total_uses += n;
        }
        fIndex.push_back(total_uses);

        // Tick down each Instruction's uses to fill in fTable.
        fTable.resize(total_uses, NA);
        for (Val id = (Val)program.size(); id --> 0; ) {
            if (live[id]) {
                Instruction inst = program[id];
                if (inst.x != NA) { fTable[fIndex[inst.x] + --uses[inst.x]] = id; }
                if (inst.y != NA) { fTable[fIndex[inst.y] + --uses[inst.y]] = id; }
                if (inst.z != NA) { fTable[fIndex[inst.z] + --uses[inst.z]] = id; }
            }
        }
        for (int n  : uses  ) { (void)n;  SkASSERT(n  == 0 ); }
        for (Val id : fTable) { (void)id; SkASSERT(id != NA); }
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

    static Mod mod(int imm) {
        if (imm == 0)               { return Mod::Indirect; }
        if (SkTFitsIn<int8_t>(imm)) { return Mod::OneByteImm; }
        return Mod::FourByteImm;
    }

    static int imm_bytes(Mod mod) {
        switch (mod) {
            case Mod::Indirect:    return 0;
            case Mod::OneByteImm:  return 1;
            case Mod::FourByteImm: return 4;
            case Mod::Direct: SkUNREACHABLE;
        }
        SkUNREACHABLE;
    }

    // SIB byte encodes a memory address, base + (index * scale).
    static uint8_t sib(Assembler::Scale scale, int index, int base) {
        return _233((int)scale, index, base);
    }

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
            SkUNREACHABLE;
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

    void Assembler::int3() {
        this->byte(0xcc);
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

    void Assembler::movq(GP64 dst, GP64 src, int off) {
        this->byte(rex(1,dst>>3,0,src>>3));
        this->byte(0x8b);
        this->byte(mod_rm(mod(off), dst&7, src&7));
        this->bytes(&off, imm_bytes(mod(off)));
    }

    void Assembler::op(int prefix, int map, int opcode, Ymm dst, Ymm x, Ymm y, bool W/*=false*/) {
        VEX v = vex(W, dst>>3, 0, y>>3,
                    map, x, 1/*ymm, not xmm*/, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Direct, dst&7, y&7));
    }

    void Assembler::vpaddd (Ymm dst, Ymm x, YmmOrLabel y) { this->op(0x66,  0x0f,0xfe, dst,x,y); }
    void Assembler::vpsubd (Ymm dst, Ymm x, YmmOrLabel y) { this->op(0x66,  0x0f,0xfa, dst,x,y); }
    void Assembler::vpmulld(Ymm dst, Ymm x, Ymm        y) { this->op(0x66,0x380f,0x40, dst,x,y); }

    void Assembler::vpsubw (Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xf9, dst,x,y); }
    void Assembler::vpmullw(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0xd5, dst,x,y); }

    void Assembler::vpand (Ymm dst, Ymm x, YmmOrLabel y) { this->op(0x66,0x0f,0xdb, dst,x,y); }
    void Assembler::vpor  (Ymm dst, Ymm x, YmmOrLabel y) { this->op(0x66,0x0f,0xeb, dst,x,y); }
    void Assembler::vpxor (Ymm dst, Ymm x, YmmOrLabel y) { this->op(0x66,0x0f,0xef, dst,x,y); }
    void Assembler::vpandn(Ymm dst, Ymm x, Ymm        y) { this->op(0x66,0x0f,0xdf, dst,x,y); }

    void Assembler::vaddps(Ymm dst, Ymm x, YmmOrLabel y) { this->op(0,0x0f,0x58, dst,x,y); }
    void Assembler::vsubps(Ymm dst, Ymm x, YmmOrLabel y) { this->op(0,0x0f,0x5c, dst,x,y); }
    void Assembler::vmulps(Ymm dst, Ymm x, YmmOrLabel y) { this->op(0,0x0f,0x59, dst,x,y); }
    void Assembler::vdivps(Ymm dst, Ymm x, Ymm        y) { this->op(0,0x0f,0x5e, dst,x,y); }
    void Assembler::vminps(Ymm dst, Ymm x, YmmOrLabel y) { this->op(0,0x0f,0x5d, dst,x,y); }
    void Assembler::vmaxps(Ymm dst, Ymm x, YmmOrLabel y) { this->op(0,0x0f,0x5f, dst,x,y); }

    void Assembler::vfmadd132ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0x98, dst,x,y); }
    void Assembler::vfmadd213ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xa8, dst,x,y); }
    void Assembler::vfmadd231ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xb8, dst,x,y); }

    void Assembler::vfmsub132ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0x9a, dst,x,y); }
    void Assembler::vfmsub213ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xaa, dst,x,y); }
    void Assembler::vfmsub231ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xba, dst,x,y); }

    void Assembler::vfnmadd132ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0x9c, dst,x,y); }
    void Assembler::vfnmadd213ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xac, dst,x,y); }
    void Assembler::vfnmadd231ps(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0xbc, dst,x,y); }

    void Assembler::vpackusdw(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x380f,0x2b, dst,x,y); }
    void Assembler::vpackuswb(Ymm dst, Ymm x, Ymm y) { this->op(0x66,  0x0f,0x67, dst,x,y); }

    void Assembler::vpcmpeqd(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0x76, dst,x,y); }
    void Assembler::vpcmpgtd(Ymm dst, Ymm x, Ymm y) { this->op(0x66,0x0f,0x66, dst,x,y); }

    void Assembler::vcmpps(Ymm dst, Ymm x, Ymm y, int imm) {
        this->op(0,0x0f,0xc2, dst,x,y);
        this->byte(imm);
    }

    void Assembler::vpblendvb(Ymm dst, Ymm x, Ymm y, Ymm z) {
        int prefix = 0x66,
            map    = 0x3a0f,
            opcode = 0x4c;
        VEX v = vex(0, dst>>3, 0, y>>3,
                    map, x, /*ymm?*/1, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Direct, dst&7, y&7));
        this->byte(z << 4);
    }

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

    void Assembler::vroundps(Ymm dst, Ymm x, int imm) {
        this->op(0x66,0x3a0f,0x08, dst,x);
        this->byte(imm);
    }

    void Assembler::vmovdqa(Ymm dst, Ymm src) { this->op(0x66,0x0f,0x6f, dst,src); }

    void Assembler::vcvtdq2ps (Ymm dst, Ymm x) { this->op(   0,0x0f,0x5b, dst,x); }
    void Assembler::vcvttps2dq(Ymm dst, Ymm x) { this->op(0xf3,0x0f,0x5b, dst,x); }
    void Assembler::vcvtps2dq (Ymm dst, Ymm x) { this->op(0x66,0x0f,0x5b, dst,x); }
    void Assembler::vsqrtps   (Ymm dst, Ymm x) { this->op(   0,0x0f,0x51, dst,x); }

    Assembler::Label Assembler::here() {
        return { (int)this->size(), Label::NotYetSet, {} };
    }

    int Assembler::disp19(Label* l) {
        SkASSERT(l->kind == Label::NotYetSet ||
                 l->kind == Label::ARMDisp19);
        l->kind = Label::ARMDisp19;
        l->references.push_back(here().offset);
        // ARM 19-bit instruction count, from the beginning of this instruction.
        return (l->offset - here().offset) / 4;
    }

    int Assembler::disp32(Label* l) {
        SkASSERT(l->kind == Label::NotYetSet ||
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

    void Assembler::op(int prefix, int map, int opcode, Ymm dst, Ymm x, YmmOrLabel y) {
        y.label ? this->op(prefix,map,opcode,dst,x, y.label)
                : this->op(prefix,map,opcode,dst,x, y.ymm  );
    }

    void Assembler::vpshufb(Ymm dst, Ymm x, Label* l) { this->op(0x66,0x380f,0x00, dst,x,l); }
    void Assembler::vptest(Ymm dst, Label* l) { this->op(0x66, 0x380f, 0x17, dst, (Ymm)0, l); }

    void Assembler::vbroadcastss(Ymm dst, Label* l) { this->op(0x66,0x380f,0x18, dst, (Ymm)0, l); }
    void Assembler::vbroadcastss(Ymm dst, Xmm src)  { this->op(0x66,0x380f,0x18, dst, (Ymm)src); }
    void Assembler::vbroadcastss(Ymm dst, GP64 ptr, int off) {
        int prefix = 0x66,
               map = 0x380f,
            opcode = 0x18;
        VEX v = vex(0, dst>>3, 0, ptr>>3,
                    map, 0, /*ymm?*/1, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);

        this->byte(mod_rm(mod(off), dst&7, ptr&7));
        this->bytes(&off, imm_bytes(mod(off)));
    }

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
    void Assembler::jc (Label* l) { this->jump(0x82, l); }

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
    void Assembler::vpmovzxwd(Ymm dst, GP64 src) { this->load_store(0x66,0x380f,0x33, dst,src); }
    void Assembler::vpmovzxbd(Ymm dst, GP64 src) { this->load_store(0x66,0x380f,0x31, dst,src); }

    void Assembler::vmovups  (GP64 dst, Ymm src) { this->load_store(0   ,  0x0f,0x11, src,dst); }
    void Assembler::vmovups  (GP64 dst, Xmm src) {
        // Same as vmovups(GP64,YMM) and load_store() except ymm? is 0.
        int prefix = 0,
            map    = 0x0f,
            opcode = 0x11;
        VEX v = vex(0, src>>3, 0, dst>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, src&7, dst&7));
    }

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

    void Assembler::vmovd(Xmm dst, Scale scale, GP64 index, GP64 base) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0x6e;
        VEX v = vex(0, dst>>3, index>>3, base>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, dst&7, rsp));
        this->byte(sib(scale, index&7, base&7));
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

    void Assembler::movzbl(GP64 dst, GP64 src, int off) {
        if ((dst>>3) || (src>>3)) {
            this->byte(rex(0,dst>>3,0,src>>3));
        }
        this->byte(0x0f);
        this->byte(0xb6);
        this->byte(mod_rm(mod(off), dst&7, src&7));
        this->bytes(&off, imm_bytes(mod(off)));
    }


    void Assembler::movb(GP64 dst, GP64 src) {
        if ((dst>>3) || (src>>3)) {
            this->byte(rex(0,src>>3,0,dst>>3));
        }
        this->byte(0x88);
        this->byte(mod_rm(Mod::Indirect, src&7, dst&7));
    }

    void Assembler::vpinsrw(Xmm dst, Xmm src, GP64 ptr, int imm) {
        int prefix = 0x66,
            map    = 0x0f,
            opcode = 0xc4;
        VEX v = vex(0, dst>>3, 0, ptr>>3,
                    map, src, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, dst&7, ptr&7));
        this->byte(imm);
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

    void Assembler::vpextrw(GP64 ptr, Xmm src, int imm) {
        int prefix = 0x66,
            map    = 0x3a0f,
            opcode = 0x15;

        VEX v = vex(0, src>>3, 0, ptr>>3,
                    map, 0, /*ymm?*/0, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, src&7, ptr&7));
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

    void Assembler::vgatherdps(Ymm dst, Scale scale, Ymm ix, GP64 base, Ymm mask) {
        // Unlike most instructions, no aliasing is permitted here.
        SkASSERT(dst != ix);
        SkASSERT(dst != mask);
        SkASSERT(mask != ix);

        int prefix = 0x66,
            map    = 0x380f,
            opcode = 0x92;
        VEX v = vex(0, dst>>3, ix>>3, base>>3,
                    map, mask, /*ymm?*/1, prefix);
        this->bytes(v.bytes, v.len);
        this->byte(opcode);
        this->byte(mod_rm(Mod::Indirect, dst&7, rsp));
        this->byte(sib(scale, ix&7, base&7));
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
    void Assembler::bsl16b(V d, V n, V m) { this->op(0b0'1'1'01110'01'1, m, 0b00011'1, n, d); }
    void Assembler::not16b(V d, V n)      { this->op(0b0'1'1'01110'00'10000'00101'10,  n, d); }

    void Assembler::add4s(V d, V n, V m) { this->op(0b0'1'0'01110'10'1, m, 0b10000'1, n, d); }
    void Assembler::sub4s(V d, V n, V m) { this->op(0b0'1'1'01110'10'1, m, 0b10000'1, n, d); }
    void Assembler::mul4s(V d, V n, V m) { this->op(0b0'1'0'01110'10'1, m, 0b10011'1, n, d); }

    void Assembler::cmeq4s(V d, V n, V m) { this->op(0b0'1'1'01110'10'1, m, 0b10001'1, n, d); }
    void Assembler::cmgt4s(V d, V n, V m) { this->op(0b0'1'0'01110'10'1, m, 0b0011'0'1, n, d); }

    void Assembler::sub8h(V d, V n, V m) { this->op(0b0'1'1'01110'01'1, m, 0b10000'1, n, d); }
    void Assembler::mul8h(V d, V n, V m) { this->op(0b0'1'0'01110'01'1, m, 0b10011'1, n, d); }

    void Assembler::fadd4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b11010'1, n, d); }
    void Assembler::fsub4s(V d, V n, V m) { this->op(0b0'1'0'01110'1'0'1, m, 0b11010'1, n, d); }
    void Assembler::fmul4s(V d, V n, V m) { this->op(0b0'1'1'01110'0'0'1, m, 0b11011'1, n, d); }
    void Assembler::fdiv4s(V d, V n, V m) { this->op(0b0'1'1'01110'0'0'1, m, 0b11111'1, n, d); }
    void Assembler::fmin4s(V d, V n, V m) { this->op(0b0'1'0'01110'1'0'1, m, 0b11110'1, n, d); }
    void Assembler::fmax4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b11110'1, n, d); }
    void Assembler::fneg4s(V d, V n)      { this->op(0b0'1'1'01110'1'0'10000'01111'10,  n, d); }

    void Assembler::fcmeq4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b1110'0'1, n, d); }
    void Assembler::fcmgt4s(V d, V n, V m) { this->op(0b0'1'1'01110'1'0'1, m, 0b1110'0'1, n, d); }
    void Assembler::fcmge4s(V d, V n, V m) { this->op(0b0'1'1'01110'0'0'1, m, 0b1110'0'1, n, d); }

    void Assembler::fmla4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b11001'1, n, d); }
    void Assembler::fmls4s(V d, V n, V m) { this->op(0b0'1'0'01110'1'0'1, m, 0b11001'1, n, d); }

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
    void Assembler::fcvtns4s(V d, V n) { this->op(0b0'1'0'01110'0'0'10000'1101'0'10, n,d); }

    void Assembler::xtns2h(V d, V n) { this->op(0b0'0'0'01110'01'10000'10010'10, n,d); }
    void Assembler::xtnh2b(V d, V n) { this->op(0b0'0'0'01110'00'10000'10010'10, n,d); }

    void Assembler::uxtlb2h(V d, V n) { this->op(0b0'0'1'011110'0001'000'10100'1, n,d); }
    void Assembler::uxtlh2s(V d, V n) { this->op(0b0'0'1'011110'0010'000'10100'1, n,d); }

    void Assembler::uminv4s(V d, V n) { this->op(0b0'1'1'01110'10'11000'1'1010'10, n,d); }

    void Assembler::brk(int imm16) {
        this->word(0b11010100'001'0000000000000000'000'00
                  | (imm16 & 16_mask) << 5);
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

    void Assembler::fmovs(X dst, V src) {
        this->word(0b0'0'0'11110'00'1'00'110'000000 << 10
                  | (src & 5_mask)                  << 5
                  | (dst & 5_mask)                  << 0);
    }

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
    #define SKVM_JIT_STATS 0
    #if SKVM_JIT_STATS
        static std::atomic<int64_t>  calls{0}, jits{0},
                                    pixels{0}, fast{0};
        pixels += n;
        if (0 == calls++) {
            atexit([]{
                int64_t num = jits .load(),
                        den = calls.load();
                SkDebugf("%.3g%% of %lld eval() calls went through JIT.\n", (100.0 * num)/den, den);
                num = fast  .load();
                den = pixels.load();
                SkDebugf("%.3g%% of %lld pixels went through JIT.\n", (100.0 * num)/den, den);
            });
        }
    #endif
        // This may fail either simply because we can't JIT, or when using LLVM,
        // because the work represented by fImpl->llvm_compiling hasn't finished yet.
        if (const void* b = fImpl->jit_entry.load()) {
    #if SKVM_JIT_STATS
            jits++;
            fast += n;
    #endif
            void** a = args;
            switch (fImpl->strides.size()) {
                case 0: return ((void(*)(int                        ))b)(n                    );
                case 1: return ((void(*)(int,void*                  ))b)(n,a[0]               );
                case 2: return ((void(*)(int,void*,void*            ))b)(n,a[0],a[1]          );
                case 3: return ((void(*)(int,void*,void*,void*      ))b)(n,a[0],a[1],a[2]     );
                case 4: return ((void(*)(int,void*,void*,void*,void*))b)(n,a[0],a[1],a[2],a[3]);
                case 5: return ((void(*)(int,void*,void*,void*,void*,void*))b)
                                (n,a[0],a[1],a[2],a[3],a[4]);
                default: SkUNREACHABLE;  // TODO
            }
        }

        // So we'll sometimes use the interpreter here even if later calls will use the JIT.
        SkOpts::interpret_skvm(fImpl->instructions.data(), (int)fImpl->instructions.size(),
                               this->nregs(), this->loop(), fImpl->strides.data(), this->nargs(),
                               n, args);
    }

#if defined(SKVM_LLVM)
    void Program::setupLLVM(const std::vector<OptimizedInstruction>& instructions,
                            const char* debug_name) {
        auto ctx = std::make_unique<llvm::LLVMContext>();

        auto mod = std::make_unique<llvm::Module>("", *ctx);
        // All the scary bare pointers from here on are owned by ctx or mod, I think.

        // Everything I've tested runs faster at K=8 (using ymm) than K=16 (zmm) on SKX machines.
        const int K = (true && SkCpu::Supports(SkCpu::HSW)) ? 8 : 4;

        llvm::Type *ptr = llvm::Type::getInt8Ty(*ctx)->getPointerTo(),
                   *i32 = llvm::Type::getInt32Ty(*ctx);

        std::vector<llvm::Type*> arg_types = { i32 };
        for (size_t i = 0; i < fImpl->strides.size(); i++) {
            arg_types.push_back(ptr);
        }

        llvm::FunctionType* fn_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx),
                                                              arg_types, /*vararg?=*/false);
        llvm::Function* fn
            = llvm::Function::Create(fn_type, llvm::GlobalValue::ExternalLinkage, debug_name, *mod);
        for (size_t i = 0; i < fImpl->strides.size(); i++) {
            fn->addParamAttr(i+1, llvm::Attribute::NoAlias);
        }

        llvm::BasicBlock *enter  = llvm::BasicBlock::Create(*ctx, "enter" , fn),
                         *hoistK = llvm::BasicBlock::Create(*ctx, "hoistK", fn),
                         *testK  = llvm::BasicBlock::Create(*ctx, "testK" , fn),
                         *loopK  = llvm::BasicBlock::Create(*ctx, "loopK" , fn),
                         *hoist1 = llvm::BasicBlock::Create(*ctx, "hoist1", fn),
                         *test1  = llvm::BasicBlock::Create(*ctx, "test1" , fn),
                         *loop1  = llvm::BasicBlock::Create(*ctx, "loop1" , fn),
                         *leave  = llvm::BasicBlock::Create(*ctx, "leave" , fn);

        using IRBuilder = llvm::IRBuilder<>;

        llvm::PHINode*                 n;
        std::vector<llvm::PHINode*> args;
        std::vector<llvm::Value*> vals(instructions.size());

        auto emit = [&](size_t i, bool scalar, IRBuilder* b) {
            auto [op, x,y,z, immy,immz, death,can_hoist,used_in_loop] = instructions[i];

            llvm::Type *i1    = llvm::Type::getInt1Ty (*ctx),
                       *i8    = llvm::Type::getInt8Ty (*ctx),
                       *i8x4  = llvm::VectorType::get(i8, 4),
                       *i16   = llvm::Type::getInt16Ty(*ctx),
                       *i16x2 = llvm::VectorType::get(i16, 2),
                       *f32   = llvm::Type::getFloatTy(*ctx),
                       *I1    = scalar ? i1    : llvm::VectorType::get(i1 , K  ),
                       *I8    = scalar ? i8    : llvm::VectorType::get(i8 , K  ),
                       *I8x4  = scalar ? i8x4  : llvm::VectorType::get(i8 , K*4),
                       *I16   = scalar ? i16   : llvm::VectorType::get(i16, K  ),
                       *I16x2 = scalar ? i16x2 : llvm::VectorType::get(i16, K*2),
                       *I32   = scalar ? i32   : llvm::VectorType::get(i32, K  ),
                       *F32   = scalar ? f32   : llvm::VectorType::get(f32, K  );

            auto I  = [&](llvm::Value* v) { return b->CreateBitCast(v, I32  ); };
            auto F  = [&](llvm::Value* v) { return b->CreateBitCast(v, F32  ); };
            auto x2 = [&](llvm::Value* v) { return b->CreateBitCast(v, I16x2); };

            auto S = [&](llvm::Type* dst, llvm::Value* v) { return b->CreateSExt(v, dst); };

            switch (llvm::Type* t = nullptr; op) {
                default:
                    SkDebugf("can't llvm %s (%d)\n", name(op), op);
                    return false;

                case Op::assert_true: /*TODO*/ break;

                case Op::index:
                    if (I32->isVectorTy()) {
                        std::vector<llvm::Constant*> iota(K);
                        for (int j = 0; j < K; j++) {
                            iota[j] = b->getInt32(j);
                        }
                        vals[i] = b->CreateSub(b->CreateVectorSplat(K, n),
                                               llvm::ConstantVector::get(iota));
                    } else {
                        vals[i] = n;
                    } break;

                case Op::load8:  t = I8 ; goto load;
                case Op::load16: t = I16; goto load;
                case Op::load32: t = I32; goto load;
                load: {
                    llvm::Value* ptr = b->CreateBitCast(args[immy], t->getPointerTo());
                    vals[i] = b->CreateZExt(b->CreateAlignedLoad(ptr, 1), I32);
                } break;


                case Op::splat: vals[i] = llvm::ConstantInt::get(I32, immy); break;

                case Op::uniform8:  t = i8 ; goto uniform;
                case Op::uniform16: t = i16; goto uniform;
                case Op::uniform32: t = i32; goto uniform;
                uniform: {
                    llvm::Value* ptr = b->CreateBitCast(b->CreateConstInBoundsGEP1_32(nullptr,
                                                                                      args[immy],
                                                                                      immz),
                                                        t->getPointerTo());
                    llvm::Value* val = b->CreateZExt(b->CreateAlignedLoad(ptr, 1), i32);
                    vals[i] = I32->isVectorTy() ? b->CreateVectorSplat(K, val)
                                                : val;
                } break;

                case Op::gather8:  t = i8 ; goto gather;
                case Op::gather16: t = i16; goto gather;
                case Op::gather32: t = i32; goto gather;
                gather: {
                    // Our gather base pointer is immz bytes off of uniform immy.
                    llvm::Value* base =
                        b->CreateLoad(b->CreateBitCast(b->CreateConstInBoundsGEP1_32(nullptr,
                                                                                     args[immy],
                                                                                     immz),
                                                       t->getPointerTo()->getPointerTo()));

                    llvm::Value* ptr = b->CreateInBoundsGEP(nullptr, base, vals[x]);
                    llvm::Value* gathered;
                    if (ptr->getType()->isVectorTy()) {
                        gathered = b->CreateMaskedGather(ptr, 1);
                    } else {
                        gathered = b->CreateAlignedLoad(ptr, 1);
                    }
                    vals[i] = b->CreateZExt(gathered, I32);
                } break;

                case Op::store8:  t = I8 ; goto store;
                case Op::store16: t = I16; goto store;
                case Op::store32: t = I32; goto store;
                store: {
                    llvm::Value* val = b->CreateTrunc(vals[x], t);
                    llvm::Value* ptr = b->CreateBitCast(args[immy],
                                                        val->getType()->getPointerTo());
                    vals[i] = b->CreateAlignedStore(val, ptr, 1);
                } break;

                case Op::bit_and:   vals[i] = b->CreateAnd(vals[x], vals[y]); break;
                case Op::bit_or :   vals[i] = b->CreateOr (vals[x], vals[y]); break;
                case Op::bit_xor:   vals[i] = b->CreateXor(vals[x], vals[y]); break;
                case Op::bit_clear: vals[i] = b->CreateAnd(vals[x], b->CreateNot(vals[y])); break;

                case Op::pack: vals[i] = b->CreateOr(vals[x], b->CreateShl(vals[y], immz)); break;

                case Op::select:
                    vals[i] = b->CreateSelect(b->CreateTrunc(vals[x], I1), vals[y], vals[z]);
                    break;

                case Op::add_i32: vals[i] = b->CreateAdd(vals[x], vals[y]); break;
                case Op::sub_i32: vals[i] = b->CreateSub(vals[x], vals[y]); break;
                case Op::mul_i32: vals[i] = b->CreateMul(vals[x], vals[y]); break;

                case Op::shl_i32: vals[i] = b->CreateShl (vals[x], immy); break;
                case Op::sra_i32: vals[i] = b->CreateAShr(vals[x], immy); break;
                case Op::shr_i32: vals[i] = b->CreateLShr(vals[x], immy); break;

                case Op:: eq_i32: vals[i] = S(I32, b->CreateICmpEQ (vals[x], vals[y])); break;
                case Op::neq_i32: vals[i] = S(I32, b->CreateICmpNE (vals[x], vals[y])); break;
                case Op:: gt_i32: vals[i] = S(I32, b->CreateICmpSGT(vals[x], vals[y])); break;
                case Op::gte_i32: vals[i] = S(I32, b->CreateICmpSGE(vals[x], vals[y])); break;

                case Op::add_f32: vals[i] = I(b->CreateFAdd(F(vals[x]), F(vals[y]))); break;
                case Op::sub_f32: vals[i] = I(b->CreateFSub(F(vals[x]), F(vals[y]))); break;
                case Op::mul_f32: vals[i] = I(b->CreateFMul(F(vals[x]), F(vals[y]))); break;
                case Op::div_f32: vals[i] = I(b->CreateFDiv(F(vals[x]), F(vals[y]))); break;

                case Op:: eq_f32: vals[i] = S(I32, b->CreateFCmpOEQ(F(vals[x]), F(vals[y]))); break;
                case Op::neq_f32: vals[i] = S(I32, b->CreateFCmpUNE(F(vals[x]), F(vals[y]))); break;
                case Op:: gt_f32: vals[i] = S(I32, b->CreateFCmpOGT(F(vals[x]), F(vals[y]))); break;
                case Op::gte_f32: vals[i] = S(I32, b->CreateFCmpOGE(F(vals[x]), F(vals[y]))); break;

                case Op::fma_f32:
                    vals[i] = I(b->CreateIntrinsic(llvm::Intrinsic::fma, {F32},
                                                   {F(vals[x]), F(vals[y]), F(vals[z])}));
                    break;

                case Op::fms_f32:
                    vals[i] = I(b->CreateIntrinsic(llvm::Intrinsic::fma, {F32},
                                                   {F(vals[x]), F(vals[y]),
                                                    b->CreateFNeg(F(vals[z]))}));
                    break;

                case Op::fnma_f32:
                    vals[i] = I(b->CreateIntrinsic(llvm::Intrinsic::fma, {F32},
                                                   {b->CreateFNeg(F(vals[x])), F(vals[y]),
                                                    F(vals[z])}));
                    break;

                case Op::floor:
                    vals[i] = I(b->CreateUnaryIntrinsic(llvm::Intrinsic::floor, F(vals[x])));
                    break;

                case Op::max_f32:
                    vals[i] = I(b->CreateSelect(b->CreateFCmpOLT(F(vals[x]), F(vals[y])),
                                                F(vals[y]), F(vals[x])));
                    break;
                case Op::min_f32:
                    vals[i] = I(b->CreateSelect(b->CreateFCmpOLT(F(vals[y]), F(vals[x])),
                                                F(vals[y]), F(vals[x])));
                    break;

                case Op::sqrt_f32:
                    vals[i] = I(b->CreateUnaryIntrinsic(llvm::Intrinsic::sqrt, F(vals[x])));
                    break;

                case Op::to_f32: vals[i] = I(b->CreateSIToFP(  vals[x] , F32)); break;
                case Op::trunc : vals[i] =   b->CreateFPToSI(F(vals[x]), I32) ; break;
                case Op::round : {
                    // Basic impl when we can't use cvtps2dq and co.
                    auto round = b->CreateUnaryIntrinsic(llvm::Intrinsic::rint, F(vals[x]));
                    vals[i] = b->CreateFPToSI(round, I32);

                #if 1 && defined(SK_CPU_X86)
                    // Using b->CreateIntrinsic(..., {}, {...}) to avoid name mangling.
                    if (scalar) {
                        // cvtss2si is float x4 -> int, ignoring input lanes 1,2,3.  ¯\_(ツ)_/¯
                        llvm::Value* v = llvm::UndefValue::get(llvm::VectorType::get(f32, 4));
                        v = b->CreateInsertElement(v, F(vals[x]), (uint64_t)0);
                        vals[i] = b->CreateIntrinsic(llvm::Intrinsic::x86_sse_cvtss2si, {}, {v});
                    } else {
                        SkASSERT(K == 4  || K == 8);
                        auto intr = K == 4 ?   llvm::Intrinsic::x86_sse2_cvtps2dq :
                                 /* K == 8 ?*/ llvm::Intrinsic::x86_avx_cvt_ps2dq_256;
                        vals[i] = b->CreateIntrinsic(intr, {}, {F(vals[x])});
                    }
                #endif
                } break;

                case Op::add_i16x2: vals[i] = I(b->CreateAdd(x2(vals[x]), x2(vals[y]))); break;
                case Op::sub_i16x2: vals[i] = I(b->CreateSub(x2(vals[x]), x2(vals[y]))); break;
                case Op::mul_i16x2: vals[i] = I(b->CreateMul(x2(vals[x]), x2(vals[y]))); break;

                case Op::shl_i16x2: vals[i] = I(b->CreateShl (x2(vals[x]), immy)); break;
                case Op::sra_i16x2: vals[i] = I(b->CreateAShr(x2(vals[x]), immy)); break;
                case Op::shr_i16x2: vals[i] = I(b->CreateLShr(x2(vals[x]), immy)); break;

                case Op:: eq_i16x2:
                    vals[i] = I(S(I16x2, b->CreateICmpEQ (x2(vals[x]), x2(vals[y]))));
                    break;
                case Op::neq_i16x2:
                    vals[i] = I(S(I16x2, b->CreateICmpNE (x2(vals[x]), x2(vals[y]))));
                    break;
                case Op:: gt_i16x2:
                    vals[i] = I(S(I16x2, b->CreateICmpSGT(x2(vals[x]), x2(vals[y]))));
                    break;
                case Op::gte_i16x2:
                    vals[i] = I(S(I16x2, b->CreateICmpSGE(x2(vals[x]), x2(vals[y]))));
                    break;

                case Op::bytes: {
                    int N = vals[x]->getType()->isVectorTy() ? K : 1;

                    uint32_t off = 0;
                    auto nibble_to_mask = [&](uint8_t n) -> uint32_t {
                        switch (n) {
                            case 0: return       4*N;   // Select any byte in the second (zero) arg.
                            case 1: return off +   0;   // 1st byte in this arg.
                            case 2: return off +   1;   // 2nd ...
                            case 3: return off +   2;   // 3rd ...
                            case 4: return off +   3;   // 4th byte in this arg.
                        }
                        SkUNREACHABLE;
                        return 0;
                    };

                    std::vector<uint32_t> mask(N*4);
                    for (int i = 0; i < N; i++) {
                        mask[4*i+0] = nibble_to_mask( (immy >>  0) & 0xf );
                        mask[4*i+1] = nibble_to_mask( (immy >>  4) & 0xf );
                        mask[4*i+2] = nibble_to_mask( (immy >>  8) & 0xf );
                        mask[4*i+3] = nibble_to_mask( (immy >> 12) & 0xf );
                        off += 4;
                    }

                    llvm::Value* input =  b->CreateBitCast(vals[x], I8x4);
                    llvm::Value* zero  = llvm::Constant::getNullValue(I8x4);
                    vals[i] = I(b->CreateShuffleVector(input, zero, mask));
                } break;
            }
            return true;
        };

        {
            IRBuilder b(enter);
            b.CreateBr(hoistK);
        }

        // hoistK: emit each hoistable vector instruction; goto testK;
        // LLVM can do this sort of thing itself, but we've got the information cheap,
        // and pointer aliasing makes it easier to manually hoist than teach LLVM it's safe.
        {
            IRBuilder b(hoistK);

            // Hoisted instructions will need args (think, uniforms), so set that up now.
            // These phi nodes are degenerate... they'll always be the passed-in args from enter.
            // Later on when we start looping the phi nodes will start looking useful.
            llvm::Argument* arg = fn->arg_begin();
            (void)arg++;  // Leave n as nullptr... it'd be a bug to use n in a hoisted instruction.
            for (size_t i = 0; i < fImpl->strides.size(); i++) {
                args.push_back(b.CreatePHI(arg->getType(), 1));
                args.back()->addIncoming(arg++, enter);
            }

            for (size_t i = 0; i < instructions.size(); i++) {
                if (instructions[i].can_hoist && !emit(i, false, &b)) {
                    return;
                }
            }

            b.CreateBr(testK);
        }

        // testK:  if (N >= K) goto loopK; else goto hoist1;
        {
            IRBuilder b(testK);

            // New phi nodes for `n` and each pointer argument from hoistK; later we'll add loopK.
            // These also start as the initial function arguments; hoistK can't have changed them.
            llvm::Argument* arg = fn->arg_begin();

            n = b.CreatePHI(arg->getType(), 2);
            n->addIncoming(arg++, hoistK);

            for (size_t i = 0; i < fImpl->strides.size(); i++) {
                args[i] = b.CreatePHI(arg->getType(), 2);
                args[i]->addIncoming(arg++, hoistK);
            }

            b.CreateCondBr(b.CreateICmpSGE(n, b.getInt32(K)), loopK, hoist1);
        }

        // loopK:  ... insts on K x T vectors; N -= K, args += K*stride; goto testK;
        {
            IRBuilder b(loopK);
            for (size_t i = 0; i < instructions.size(); i++) {
                if (!instructions[i].can_hoist && !emit(i, false, &b)) {
                    return;
                }
            }

            // n -= K
            llvm::Value* n_next = b.CreateSub(n, b.getInt32(K));
            n->addIncoming(n_next, loopK);

            // Each arg ptr += K
            for (size_t i = 0; i < fImpl->strides.size(); i++) {
                llvm::Value* arg_next
                    = b.CreateConstInBoundsGEP1_32(nullptr, args[i], K*fImpl->strides[i]);
                args[i]->addIncoming(arg_next, loopK);
            }
            b.CreateBr(testK);
        }

        // hoist1: emit each hoistable scalar instruction; goto test1;
        {
            IRBuilder b(hoist1);
            for (size_t i = 0; i < instructions.size(); i++) {
                if (instructions[i].can_hoist && !emit(i, true, &b)) {
                    return;
                }
            }
            b.CreateBr(test1);
        }

        // test1:  if (N >= 1) goto loop1; else goto leave;
        {
            IRBuilder b(test1);

            // Set up new phi nodes for `n` and each pointer argument, now from hoist1 and loop1.
            llvm::PHINode* n_new = b.CreatePHI(n->getType(), 2);
            n_new->addIncoming(n, hoist1);
            n = n_new;

            for (size_t i = 0; i < fImpl->strides.size(); i++) {
                llvm::PHINode* arg_new = b.CreatePHI(args[i]->getType(), 2);
                arg_new->addIncoming(args[i], hoist1);
                args[i] = arg_new;
            }

            b.CreateCondBr(b.CreateICmpSGE(n, b.getInt32(1)), loop1, leave);
        }

        // loop1:  ... insts on scalars; N -= 1, args += stride; goto test1;
        {
            IRBuilder b(loop1);
            for (size_t i = 0; i < instructions.size(); i++) {
                if (!instructions[i].can_hoist && !emit(i, true, &b)) {
                    return;
                }
            }

            // n -= 1
            llvm::Value* n_next = b.CreateSub(n, b.getInt32(1));
            n->addIncoming(n_next, loop1);

            // Each arg ptr += K
            for (size_t i = 0; i < fImpl->strides.size(); i++) {
                llvm::Value* arg_next
                    = b.CreateConstInBoundsGEP1_32(nullptr, args[i], fImpl->strides[i]);
                args[i]->addIncoming(arg_next, loop1);
            }
            b.CreateBr(test1);
        }

        // leave:  ret
        {
            IRBuilder b(leave);
            b.CreateRetVoid();
        }

        SkASSERT(false == llvm::verifyModule(*mod, &llvm::outs()));

        if (true) {
            SkString path = SkStringPrintf("/tmp/%s.bc", debug_name);
            std::error_code err;
            llvm::raw_fd_ostream os(path.c_str(), err);
            if (err) {
                return;
            }
            llvm::WriteBitcodeToFile(*mod, os);
        }

        static SkOnce once;
        once([]{
            SkAssertResult(false == llvm::InitializeNativeTarget());
            SkAssertResult(false == llvm::InitializeNativeTargetAsmPrinter());
        });

        if (llvm::ExecutionEngine* ee = llvm::EngineBuilder(std::move(mod))
                                            .setEngineKind(llvm::EngineKind::JIT)
                                            .setMCPU(llvm::sys::getHostCPUName())
                                            .create()) {
            fImpl->llvm_ctx = std::move(ctx);
            fImpl->llvm_ee.reset(ee);

            // We have to be careful here about what we close over and how, in case fImpl moves.
            // fImpl itself may change, but its pointee fields won't, so close over them by value.
            // Also, debug_name will almost certainly leave scope, so copy it.
            fImpl->llvm_compiling = std::async(std::launch::async, [dst  = &fImpl->jit_entry,
                                                                    ee   =  fImpl->llvm_ee.get(),
                                                                    name = std::string(debug_name)]{
                // std::atomic<void*>*    dst;
                // llvm::ExecutionEngine* ee;
                // std::string            name;
                dst->store( (void*)ee->getFunctionAddress(name.c_str()) );
            });
        }
    }
#endif

    void Program::waitForLLVM() const {
    #if defined(SKVM_LLVM)
        if (fImpl->llvm_compiling.valid()) {
            fImpl->llvm_compiling.wait();
        }
    #endif
    }

    bool Program::hasJIT() const {
        // Program::hasJIT() is really just a debugging / test aid,
        // so we don't mind adding a sync point here to wait for compilation.
        this->waitForLLVM();

        return fImpl->jit_entry.load() != nullptr;
    }

    void Program::dropJIT() {
    #if defined(SKVM_LLVM)
        this->waitForLLVM();
        fImpl->llvm_ee .reset(nullptr);
        fImpl->llvm_ctx.reset(nullptr);
    #elif defined(SKVM_JIT)
        if (fImpl->dylib) {
            dlclose(fImpl->dylib);
        } else if (auto jit_entry = fImpl->jit_entry.load()) {
            munmap(jit_entry, fImpl->jit_size);
        }
    #else
        SkASSERT(!this->hasJIT());
    #endif

        fImpl->jit_entry.store(nullptr);
        fImpl->jit_size  = 0;
        fImpl->dylib     = nullptr;
    }

    Program::Program() : fImpl(std::make_unique<Impl>()) {}

    Program::~Program() {
        // Moved-from Programs may have fImpl == nullptr.
        if (fImpl) {
            this->dropJIT();
        }
    }

    Program::Program(Program&& other) : fImpl(std::move(other.fImpl)) {}

    Program& Program::operator=(Program&& other) {
        fImpl = std::move(other.fImpl);
        return *this;
    }

    Program::Program(const std::vector<OptimizedInstruction>& interpreter,
                     const std::vector<int>& strides) : Program() {
        fImpl->strides = strides;
        this->setupInterpreter(interpreter);
    }

    Program::Program(const std::vector<OptimizedInstruction>& interpreter,
                     const std::vector<OptimizedInstruction>& jit,
                     const std::vector<int>& strides,
                     const char* debug_name) : Program() {
        fImpl->strides = strides;
    #if 1 && defined(SKVM_LLVM)
        this->setupLLVM(interpreter, debug_name);
    #elif 1 && defined(SKVM_JIT)
        this->setupJIT(jit, debug_name);
    #endif

        // Might as well do this after setupLLVM() to get a little more time to compile.
        this->setupInterpreter(interpreter);
    }

    std::vector<InterpreterInstruction> Program::instructions() const { return fImpl->instructions; }
    int  Program::nargs() const { return (int)fImpl->strides.size(); }
    int  Program::nregs() const { return fImpl->regs; }
    int  Program::loop () const { return fImpl->loop; }
    bool Program::empty() const { return fImpl->instructions.empty(); }

    // Translate OptimizedInstructions to InterpreterInstructions.
    void Program::setupInterpreter(const std::vector<OptimizedInstruction>& instructions) {
        // Register each instruction is assigned to.
        std::vector<Reg> reg(instructions.size());

        // This next bit is a bit more complicated than strictly necessary;
        // we could just assign every instruction to its own register.
        //
        // But recycling registers is fairly cheap, and good practice for the
        // JITs where minimizing register pressure really is important.
        //
        // Since we have effectively infinite registers, we hoist any value we can.
        // (The JIT may choose a more complex policy to reduce register pressure.)
        auto hoisted = [&](Val id) { return instructions[id].can_hoist; };

        fImpl->regs = 0;
        std::vector<Reg> avail;

        // Assign this value to a register, recycling them where we can.
        auto assign_register = [&](Val id) {
            const OptimizedInstruction& inst = instructions[id];

            // If this is a real input and it's lifetime ends at this instruction,
            // we can recycle the register it's occupying.
            auto maybe_recycle_register = [&](Val input) {
                if (input != NA
                        && instructions[input].death == id
                        && !(hoisted(input) && instructions[input].used_in_loop)) {
                    avail.push_back(reg[input]);
                }
            };

            // Take care to not recycle the same register twice.
            if (true                                ) { maybe_recycle_register(inst.x); }
            if (inst.y != inst.x                    ) { maybe_recycle_register(inst.y); }
            if (inst.z != inst.x && inst.z != inst.y) { maybe_recycle_register(inst.z); }

            // Instructions that die at themselves (stores) don't need a register.
            if (inst.death != id) {
                // Allocate a register if we have to, preferring to reuse anything available.
                if (avail.empty()) {
                    reg[id] = fImpl->regs++;
                } else {
                    reg[id] = avail.back();
                    avail.pop_back();
                }
            }
        };

        // Assign a register to each hoisted instruction, then each non-hoisted loop instruction.
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            if ( hoisted(id)) { assign_register(id); }
        }
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            if (!hoisted(id)) { assign_register(id); }
        }

        // Translate OptimizedInstructions to InterpreterIstructions by mapping values to
        // registers.  This will be two passes, first hoisted instructions, then inside the loop.

        // The loop begins at the fImpl->loop'th Instruction.
        fImpl->loop = 0;
        fImpl->instructions.reserve(instructions.size());

        // Add a dummy mapping for the N/A sentinel Val to any arbitrary register
        // so lookups don't have to know which arguments are used by which Ops.
        auto lookup_register = [&](Val id) {
            return id == NA ? (Reg)0
                            : reg[id];
        };

        auto push_instruction = [&](Val id, const OptimizedInstruction& inst) {
            InterpreterInstruction pinst{
                inst.op,
                lookup_register(id),
                lookup_register(inst.x),
               {lookup_register(inst.y)},
               {lookup_register(inst.z)},
            };
            if (inst.y == NA) { pinst.immy = inst.immy; }
            if (inst.z == NA) { pinst.immz = inst.immz; }
            fImpl->instructions.push_back(pinst);
        };

        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const OptimizedInstruction& inst = instructions[id];
            if (hoisted(id)) {
                push_instruction(id, inst);
                fImpl->loop++;
            }
        }
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const OptimizedInstruction& inst = instructions[id];
            if (!hoisted(id)) {
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

    bool Program::jit(const std::vector<OptimizedInstruction>& instructions,
                      const bool try_hoisting,
                      Assembler* a) const {
        using A = Assembler;

        auto debug_dump = [&] {
        #if 0
            SkDebugfStream stream;
            this->dump(&stream);
            return true;
        #else
            return false;
        #endif
        };

    #if defined(__x86_64__)
        if (!SkCpu::Supports(SkCpu::HSW)) {
            return false;
        }
        A::GP64 N        = A::rdi,
                scratch  = A::rax,
                scratch2 = A::r11,
                arg[]    = { A::rsi, A::rdx, A::rcx, A::r8, A::r9 };

        // All 16 ymm registers are available to use.
        using Reg = A::Ymm;
        uint32_t avail = 0xffff;

    #elif defined(__aarch64__)
        A::X N       = A::x0,
             scratch = A::x8,
             arg[]   = { A::x1, A::x2, A::x3, A::x4, A::x5, A::x6, A::x7 };

        // We can use v0-v7 and v16-v31 freely; we'd need to preserve v8-v15.
        using Reg = A::V;
        uint32_t avail = 0xffff00ff;
    #endif

        if (SK_ARRAY_COUNT(arg) < fImpl->strides.size()) {
            return false;
        }

        auto hoisted = [&](Val id) { return try_hoisting && instructions[id].can_hoist; };

        std::vector<Reg> r(instructions.size());

        struct LabelAndReg {
            A::Label label;
            Reg      reg;
        };
        SkTHashMap<int, LabelAndReg> constants,    // All constants share the same pool.
                                     bytes_masks;  // These vary per-lane.
        LabelAndReg                  iota;         // Exists _only_ to vary per-lane.

        auto warmup = [&](Val id) {
            const OptimizedInstruction& inst = instructions[id];

            switch (inst.op) {
                default: break;

                case Op::bytes: if (!bytes_masks.find(inst.immy)) {
                                    bytes_masks.set(inst.immy, {});
                                    if (try_hoisting) {
                                        // vpshufb can always work with the mask from memory,
                                        // but it helps to hoist the mask to a register for tbl.
                                    #if defined(__aarch64__)
                                        LabelAndReg* entry = bytes_masks.find(inst.immy);
                                        if (int found = __builtin_ffs(avail)) {
                                            entry->reg = (Reg)(found-1);
                                            avail ^= 1 << entry->reg;
                                            a->ldrq(entry->reg, &entry->label);
                                        } else {
                                            return false;
                                        }
                                    #endif
                                    }
                                }
                                break;
            }
            return true;
        };

        auto emit = [&](Val id, bool scalar) {
            const OptimizedInstruction& inst = instructions[id];

            Op op = inst.op;
            Val x = inst.x,
                y = inst.y,
                z = inst.z;
            int immy = inst.immy,
                immz = inst.immz;

            // Most (but not all) ops create an output value and need a register to hold it, dst.
            // We track each instruction's dst in r[] so we can thread it through as an input
            // to any future instructions needing that value.
            //
            // And some ops may need a temporary register, tmp.  Some need both tmp and dst.
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
                        // This is a temporary register just for this op,
                        // so we leave it marked available for future ops.
                        tmp_reg = (Reg)(found - 1);
                    } else {
                        // We needed a tmp register but couldn't find one available. :'(
                        // This will cause emit() to return false, in turn causing jit() to fail.
                        if (debug_dump()) {
                            SkDebugf("\nCould not find a register to hold tmp\n");
                        }
                        ok = false;
                    }
                }
                return tmp_reg;
            };

            // Now make available any registers that are consumed by this instruction.
            // (The register pool we can pick dst from is >= the pool for tmp, adding any of these.)
            auto maybe_recycle_register = [&](Val input) {
                if (input != NA
                        && instructions[input].death == id
                        && !(hoisted(input) && instructions[input].used_in_loop)) {
                    avail |= 1 << r[input];
                }
            };
            maybe_recycle_register(x);
            maybe_recycle_register(y);
            maybe_recycle_register(z);
            // set_dst() and dst() will work read/write with this perhaps-just-updated avail.

            // Some ops may decide dst on their own to best fit the instruction (see Op::fma_f32).
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
                        if (debug_dump()) {
                            SkDebugf("\nCould not find a register to hold value %d\n", id);
                        }
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
                default:
                    if (debug_dump()) {
                        SkDEBUGFAILF("\nOp::%s (%d) not yet implemented\n", name(op), op);
                    }
                    return false;  // TODO: many new ops

            #if defined(__x86_64__)
                case Op::assert_true: {
                    a->vptest (r[x], &constants[0xffffffff].label);
                    A::Label all_true;
                    a->jc(&all_true);
                    a->int3();
                    a->label(&all_true);
                } break;

                case Op::store8: if (scalar) { a->vpextrb  (arg[immy], (A::Xmm)r[x], 0); }
                                 else        { a->vpackusdw(tmp(), r[x], r[x]);
                                               a->vpermq   (tmp(), tmp(), 0xd8);
                                               a->vpackuswb(tmp(), tmp(), tmp());
                                               a->vmovq    (arg[immy], (A::Xmm)tmp()); }
                                               break;

                case Op::store16: if (scalar) { a->vpextrw  (arg[immy], (A::Xmm)r[x], 0); }
                                  else        { a->vpackusdw(tmp(), r[x], r[x]);
                                                a->vpermq   (tmp(), tmp(), 0xd8);
                                                a->vmovups  (arg[immy], (A::Xmm)tmp()); }
                                                break;

                case Op::store32: if (scalar) { a->vmovd  (arg[immy], (A::Xmm)r[x]); }
                                  else        { a->vmovups(arg[immy],         r[x]); }
                                                break;

                case Op::load8:  if (scalar) {
                                     a->vpxor  (dst(), dst(), dst());
                                     a->vpinsrb((A::Xmm)dst(), (A::Xmm)dst(), arg[immy], 0);
                                 } else {
                                     a->vpmovzxbd(dst(), arg[immy]);
                                 } break;

                case Op::load16: if (scalar) {
                                     a->vpxor  (dst(), dst(), dst());
                                     a->vpinsrw((A::Xmm)dst(), (A::Xmm)dst(), arg[immy], 0);
                                 } else {
                                     a->vpmovzxwd(dst(), arg[immy]);
                                 } break;

                case Op::load32: if (scalar) { a->vmovd  ((A::Xmm)dst(), arg[immy]); }
                                 else        { a->vmovups(        dst(), arg[immy]); }
                                 break;

                case Op::gather32:
                if (scalar) {
                    auto base  = scratch,
                         index = scratch2;
                    // Our gather base pointer is immz bytes off of uniform immy.
                    a->movq(base, arg[immy], immz);

                    // Grab our index from lane 0 of the index argument.
                    a->vmovd_direct(index, (A::Xmm)r[x]);

                    // dst = *(base + 4*index)
                    a->vmovd((A::Xmm)dst(), A::FOUR, index, base);
                } else {
                    // We may not let any of dst(), index, or mask use the same register,
                    // so we must allocate registers manually and very carefully.

                    // index is argument x and has already been maybe_recycle_register()'d,
                    // so we explicitly ignore its availability during this op.
                    A::Ymm index = r[x];
                    uint32_t avail_during_gather = avail & ~(1<<index);

                    // Choose dst() to not overlap with index.
                    if (int found = __builtin_ffs(avail_during_gather)) {
                        set_dst((A::Ymm)(found-1));
                        avail_during_gather ^= (1<<dst());
                    } else {
                        ok = false;
                        break;
                    }

                    // Choose (temporary) mask to not overlap with dst() or index.
                    A::Ymm mask;
                    if (int found = __builtin_ffs(avail_during_gather)) {
                        mask = (A::Ymm)(found-1);
                    } else {
                        ok = false;
                        break;
                    }

                    // Our gather base pointer is immz bytes off of uniform immy.
                    auto base = scratch;
                    a->movq(base, arg[immy], immz);
                    a->vpcmpeqd(mask, mask, mask);   // (All lanes enabled.)
                    a->vgatherdps(dst(), A::FOUR, index, base, mask);
                }
                break;

                case Op::uniform8: a->movzbl(scratch, arg[immy], immz);
                                   a->vmovd_direct((A::Xmm)dst(), scratch);
                                   a->vbroadcastss(dst(), (A::Xmm)dst());
                                   break;

                case Op::uniform32: a->vbroadcastss(dst(), arg[immy], immz);
                                    break;

                case Op::index: a->vmovd_direct((A::Xmm)tmp(), N);
                                a->vbroadcastss(tmp(), (A::Xmm)tmp());
                                a->vpsubd(dst(), tmp(), &iota.label);
                                break;

                case Op::splat: if (immy) { a->vbroadcastss(dst(), &constants[immy].label); }
                                else      { a->vpxor(dst(), dst(), dst()); }
                                break;

                case Op::add_f32: a->vaddps(dst(), r[x], r[y]); break;
                case Op::sub_f32: a->vsubps(dst(), r[x], r[y]); break;
                case Op::mul_f32: a->vmulps(dst(), r[x], r[y]); break;
                case Op::div_f32: a->vdivps(dst(), r[x], r[y]); break;
                case Op::min_f32: a->vminps(dst(), r[x], r[y]); break;
                case Op::max_f32: a->vmaxps(dst(), r[x], r[y]); break;

                case Op::fma_f32:
                    if      (avail & (1<<r[x])) { set_dst(r[x]); a->vfmadd132ps(r[x], r[z], r[y]); }
                    else if (avail & (1<<r[y])) { set_dst(r[y]); a->vfmadd213ps(r[y], r[x], r[z]); }
                    else if (avail & (1<<r[z])) { set_dst(r[z]); a->vfmadd231ps(r[z], r[x], r[y]); }
                    else                        {                SkASSERT(dst() == tmp());
                                                                 a->vmovdqa    (dst(),r[x]);
                                                                 a->vfmadd132ps(dst(),r[z], r[y]); }
                                                                 break;

                case Op::fms_f32:
                    if      (avail & (1<<r[x])) { set_dst(r[x]); a->vfmsub132ps(r[x], r[z], r[y]); }
                    else if (avail & (1<<r[y])) { set_dst(r[y]); a->vfmsub213ps(r[y], r[x], r[z]); }
                    else if (avail & (1<<r[z])) { set_dst(r[z]); a->vfmsub231ps(r[z], r[x], r[y]); }
                    else                        {                SkASSERT(dst() == tmp());
                                                                 a->vmovdqa    (dst(),r[x]);
                                                                 a->vfmsub132ps(dst(),r[z], r[y]); }
                                                                 break;

                case Op::fnma_f32:
                    if      (avail & (1<<r[x])) { set_dst(r[x]); a->vfnmadd132ps(r[x],r[z], r[y]); }
                    else if (avail & (1<<r[y])) { set_dst(r[y]); a->vfnmadd213ps(r[y],r[x], r[z]); }
                    else if (avail & (1<<r[z])) { set_dst(r[z]); a->vfnmadd231ps(r[z],r[x], r[y]); }
                    else                        {                SkASSERT(dst() == tmp());
                                                                 a->vmovdqa    (dst(),r[x]);
                                                                 a->vfnmadd132ps(dst(),r[z],r[y]); }
                                                                 break;

                case Op::sqrt_f32: a->vsqrtps(dst(), r[x]); break;

                case Op::add_f32_imm: a->vaddps(dst(), r[x], &constants[immy].label); break;
                case Op::sub_f32_imm: a->vsubps(dst(), r[x], &constants[immy].label); break;
                case Op::mul_f32_imm: a->vmulps(dst(), r[x], &constants[immy].label); break;
                case Op::min_f32_imm: a->vminps(dst(), r[x], &constants[immy].label); break;
                case Op::max_f32_imm: a->vmaxps(dst(), r[x], &constants[immy].label); break;

                case Op::add_i32: a->vpaddd (dst(), r[x], r[y]); break;
                case Op::sub_i32: a->vpsubd (dst(), r[x], r[y]); break;
                case Op::mul_i32: a->vpmulld(dst(), r[x], r[y]); break;

                case Op::sub_i16x2: a->vpsubw (dst(), r[x], r[y]); break;
                case Op::mul_i16x2: a->vpmullw(dst(), r[x], r[y]); break;
                case Op::shr_i16x2: a->vpsrlw (dst(), r[x], immy); break;

                case Op::bit_and  : a->vpand (dst(), r[x], r[y]); break;
                case Op::bit_or   : a->vpor  (dst(), r[x], r[y]); break;
                case Op::bit_xor  : a->vpxor (dst(), r[x], r[y]); break;
                case Op::bit_clear: a->vpandn(dst(), r[y], r[x]); break;  // N.B. Y then X.
                case Op::select   : a->vpblendvb(dst(), r[z], r[y], r[x]); break;

                case Op::bit_and_imm: a->vpand (dst(), r[x], &constants[immy].label); break;
                case Op::bit_or_imm : a->vpor  (dst(), r[x], &constants[immy].label); break;
                case Op::bit_xor_imm: a->vpxor (dst(), r[x], &constants[immy].label); break;

                case Op::shl_i32: a->vpslld(dst(), r[x], immy); break;
                case Op::shr_i32: a->vpsrld(dst(), r[x], immy); break;
                case Op::sra_i32: a->vpsrad(dst(), r[x], immy); break;

                case Op::eq_i32: a->vpcmpeqd(dst(), r[x], r[y]); break;
                case Op::gt_i32: a->vpcmpgtd(dst(), r[x], r[y]); break;

                case Op:: eq_f32: a->vcmpeqps (dst(), r[x], r[y]); break;
                case Op::neq_f32: a->vcmpneqps(dst(), r[x], r[y]); break;
                case Op:: gt_f32: a->vcmpltps (dst(), r[y], r[x]); break;
                case Op::gte_f32: a->vcmpleps (dst(), r[y], r[x]); break;

                case Op::pack: a->vpslld(tmp(),  r[y], immz);
                               a->vpor  (dst(), tmp(), r[x]);
                               break;

                case Op::floor : a->vroundps  (dst(), r[x], Assembler::FLOOR); break;
                case Op::to_f32: a->vcvtdq2ps (dst(), r[x]); break;
                case Op::trunc : a->vcvttps2dq(dst(), r[x]); break;
                case Op::round : a->vcvtps2dq (dst(), r[x]); break;

                case Op::bytes: a->vpshufb(dst(), r[x], &bytes_masks.find(immy)->label);
                                break;

            #elif defined(__aarch64__)
                case Op::assert_true: {
                    a->uminv4s(tmp(), r[x]);   // uminv acts like an all() across the vector.
                    a->fmovs(scratch, tmp());
                    A::Label all_true;
                    a->cbnz(scratch, &all_true);
                    a->brk(0);
                    a->label(&all_true);
                } break;

                case Op::store8: a->xtns2h(tmp(), r[x]);
                                 a->xtnh2b(tmp(), tmp());
                   if (scalar) { a->strb  (tmp(), arg[immy]); }
                   else        { a->strs  (tmp(), arg[immy]); }
                                 break;
                // TODO: another case where it'd be okay to alias r[x] and tmp if r[x] dies here.

                case Op::store32: if (scalar) { a->strs(r[x], arg[immy]); }
                                  else        { a->strq(r[x], arg[immy]); }
                                                break;

                case Op::load8: if (scalar) { a->ldrb(tmp(), arg[immy]); }
                                else        { a->ldrs(tmp(), arg[immy]); }
                                              a->uxtlb2h(tmp(), tmp());
                                              a->uxtlh2s(dst(), tmp());
                                              break;

                case Op::load32: if (scalar) { a->ldrs(dst(), arg[immy]); }
                                 else        { a->ldrq(dst(), arg[immy]); }
                                               break;

                case Op::splat: if (immy) { a->ldrq(dst(), &constants[immy].label); }
                                else      { a->eor16b(dst(), dst(), dst()); }
                                break;
                                // TODO: If we hoist these, pack 4 values in each register
                                // and use vector/lane operations, cutting the register
                                // pressure cost of hoisting by 4?

                case Op::add_f32: a->fadd4s(dst(), r[x], r[y]); break;
                case Op::sub_f32: a->fsub4s(dst(), r[x], r[y]); break;
                case Op::mul_f32: a->fmul4s(dst(), r[x], r[y]); break;
                case Op::div_f32: a->fdiv4s(dst(), r[x], r[y]); break;
                case Op::min_f32: a->fmin4s(dst(), r[x], r[y]); break;
                case Op::max_f32: a->fmax4s(dst(), r[x], r[y]); break;

                case Op::fma_f32: // fmla.4s is z += x*y
                    if (avail & (1<<r[z])) { set_dst(r[z]); a->fmla4s( r[z],  r[x],  r[y]);   }
                    else {                                  a->orr16b(tmp(),  r[z],  r[z]);
                                                            a->fmla4s(tmp(),  r[x],  r[y]);
                                       if(dst() != tmp()) { a->orr16b(dst(), tmp(), tmp()); } }
                                                            break;

                case Op::fnma_f32:  // fmls.4s is z -= x*y
                    if (avail & (1<<r[z])) { set_dst(r[z]); a->fmls4s( r[z],  r[x],  r[y]);   }
                    else {                                  a->orr16b(tmp(),  r[z],  r[z]);
                                                            a->fmls4s(tmp(),  r[x],  r[y]);
                                       if(dst() != tmp()) { a->orr16b(dst(), tmp(), tmp()); } }
                                                            break;

                case Op::fms_f32:
                    // first dst() = xy - z as if fnma_f32
                    if (avail & (1<<r[z])) { set_dst(r[z]); a->fmls4s( r[z],  r[x],  r[y]);   }
                    else {                                  a->orr16b(tmp(),  r[z],  r[z]);
                                                            a->fmls4s(tmp(),  r[x],  r[y]);
                                       if(dst() != tmp()) { a->orr16b(dst(), tmp(), tmp()); } }
                    // then dst() = -dst()  (i.e. z - xy)
                                                            a->fneg4s(dst(), dst());
                                                            break;

                // These _imm instructions are all x86/JIT only.
                case  Op::add_f32_imm :
                case  Op::sub_f32_imm :
                case  Op::mul_f32_imm :
                case  Op::min_f32_imm :
                case  Op::max_f32_imm :
                case  Op::bit_and_imm :
                case  Op::bit_or_imm  :
                case  Op::bit_xor_imm : SkUNREACHABLE; break;

                case Op:: gt_f32: a->fcmgt4s (dst(), r[x], r[y]); break;
                case Op::gte_f32: a->fcmge4s (dst(), r[x], r[y]); break;
                case Op:: eq_f32: a->fcmeq4s (dst(), r[x], r[y]); break;
                case Op::neq_f32: a->fcmeq4s (tmp(), r[x], r[y]);
                                  a->not16b  (dst(), tmp());      break;


                case Op::add_i32: a->add4s(dst(), r[x], r[y]); break;
                case Op::sub_i32: a->sub4s(dst(), r[x], r[y]); break;
                case Op::mul_i32: a->mul4s(dst(), r[x], r[y]); break;

                case Op::sub_i16x2: a->sub8h (dst(), r[x], r[y]); break;
                case Op::mul_i16x2: a->mul8h (dst(), r[x], r[y]); break;
                case Op::shr_i16x2: a->ushr8h(dst(), r[x], immy); break;

                case Op::bit_and  : a->and16b(dst(), r[x], r[y]); break;
                case Op::bit_or   : a->orr16b(dst(), r[x], r[y]); break;
                case Op::bit_xor  : a->eor16b(dst(), r[x], r[y]); break;
                case Op::bit_clear: a->bic16b(dst(), r[x], r[y]); break;

                case Op::select: // bsl16b is x = x ? y : z
                    if (avail & (1<<r[x])) { set_dst(r[x]); a->bsl16b( r[x],  r[y],  r[z]); }
                    else {                                  a->orr16b(tmp(),  r[x],  r[x]);
                                                            a->bsl16b(tmp(),  r[y],  r[z]);
                                       if(dst() != tmp()) { a->orr16b(dst(), tmp(), tmp()); } }
                                                            break;

                case Op::shl_i32: a-> shl4s(dst(), r[x], immy); break;
                case Op::shr_i32: a->ushr4s(dst(), r[x], immy); break;
                case Op::sra_i32: a->sshr4s(dst(), r[x], immy); break;

                case Op::eq_i32: a->cmeq4s(dst(), r[x], r[y]); break;
                case Op::gt_i32: a->cmgt4s(dst(), r[x], r[y]); break;

                case Op::pack:
                    if (avail & (1<<r[x])) { set_dst(r[x]); a->sli4s ( r[x],  r[y],  immz); }
                    else                   {                a->shl4s (tmp(),  r[y],  immz);
                                                            a->orr16b(dst(), tmp(),  r[x]); }
                                                            break;

                case Op::to_f32: a->scvtf4s (dst(), r[x]); break;
                case Op::trunc:  a->fcvtzs4s(dst(), r[x]); break;
                case Op::round:  a->fcvtns4s(dst(), r[x]); break;
                // TODO: fcvtns.4s rounds to nearest even.
                // I think we actually want frintx -> fcvtzs to round to current mode.

                case Op::bytes:
                    if (try_hoisting) { a->tbl (dst(), r[x], bytes_masks.find(immy)->reg); }
                    else              { a->ldrq(tmp(), &bytes_masks.find(immy)->label);
                                        a->tbl (dst(), r[x], tmp()); }
                                        break;
            #endif
            }

            // Calls to tmp() or dst() might have flipped this false from its default true state.
            return ok;
        };


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

        A::Label body,
                 tail,
                 done;

        for (Val id = 0; id < (Val)instructions.size(); id++) {
            if (!warmup(id)) {
                return false;
            }
            if (hoisted(id) && !emit(id, /*scalar=*/false)) {
                return false;
            }
        }

        a->label(&body);
        {
            a->cmp(N, K);
            jump_if_less(&tail);
            for (Val id = 0; id < (Val)instructions.size(); id++) {
                if (!hoisted(id) && !emit(id, /*scalar=*/false)) {
                    return false;
                }
            }
            for (int i = 0; i < (int)fImpl->strides.size(); i++) {
                if (fImpl->strides[i]) {
                    add(arg[i], K*fImpl->strides[i]);
                }
            }
            sub(N, K);
            jump(&body);
        }

        a->label(&tail);
        {
            a->cmp(N, 1);
            jump_if_less(&done);
            for (Val id = 0; id < (Val)instructions.size(); id++) {
                if (!hoisted(id) && !emit(id, /*scalar=*/true)) {
                    return false;
                }
            }
            for (int i = 0; i < (int)fImpl->strides.size(); i++) {
                if (fImpl->strides[i]) {
                    add(arg[i], 1*fImpl->strides[i]);
                }
            }
            sub(N, 1);
            jump(&tail);
        }

        a->label(&done);
        {
            exit();
        }

        // Except for explicit aligned load and store instructions, AVX allows
        // memory operands to be unaligned.  So even though we're creating 16
        // byte patterns on ARM or 32-byte patterns on x86, we only need to
        // align to 4 bytes, the element size and alignment requirement.

        constants.foreach([&](int imm, LabelAndReg* entry) {
            a->align(4);
            a->label(&entry->label);
            for (int i = 0; i < K; i++) {
                a->word(imm);
            }
        });

        bytes_masks.foreach([&](int imm, LabelAndReg* entry) {
            // One 16-byte pattern for ARM tbl, that same pattern twice for x86-64 vpshufb.
            a->align(4);
            a->label(&entry->label);
            int mask[4];
            bytes_control(imm, mask);
            a->bytes(mask, sizeof(mask));
        #if defined(__x86_64__)
            a->bytes(mask, sizeof(mask));
        #endif
        });

        if (!iota.label.references.empty()) {
            a->align(4);
            a->label(&iota.label);
            for (int i = 0; i < K; i++) {
                a->word(i);
            }
        }

        return true;
    }

    void Program::setupJIT(const std::vector<OptimizedInstruction>& instructions,
                           const char* debug_name) {
        // Assemble with no buffer to determine a.size(), the number of bytes we'll assemble.
        Assembler a{nullptr};

        // First try allowing code hoisting (faster code)
        // then again without if that fails (lower register pressure).
        bool try_hoisting = true;
        if (!this->jit(instructions, try_hoisting, &a)) {
            try_hoisting = false;
            if (!this->jit(instructions, try_hoisting, &a)) {
                return;
            }
        }

        // Allocate space that we can remap as executable.
        const size_t page = sysconf(_SC_PAGESIZE);

        // mprotect works at page granularity.
        fImpl->jit_size = ((a.size() + page - 1) / page) * page;

        void* jit_entry
             = mmap(nullptr,fImpl->jit_size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1,0);
        fImpl->jit_entry.store(jit_entry);

        // Assemble the program for real.
        a = Assembler{jit_entry};
        SkAssertResult(this->jit(instructions, try_hoisting, &a));
        SkASSERT(a.size() <= fImpl->jit_size);

        // Remap as executable, and flush caches on platforms that need that.
        mprotect(jit_entry, fImpl->jit_size, PROT_READ|PROT_EXEC);
        __builtin___clear_cache((char*)jit_entry,
                                (char*)jit_entry + fImpl->jit_size);

        // For profiling and debugging, it's helpful to have this code loaded
        // dynamically rather than just jumping info fImpl->jit_entry.
        if (gSkVMJITViaDylib) {
            // Dump the raw program binary.
            SkString path = SkStringPrintf("/tmp/%s.XXXXXX", debug_name);
            int fd = mkstemp(path.writable_str());
            ::write(fd, jit_entry, a.size());
            close(fd);

            this->dropJIT();  // (unmap and null out fImpl->jit_entry.)

            // Convert it in-place to a dynamic library with a single symbol "skvm_jit":
            SkString cmd = SkStringPrintf(
                    "echo '.global _skvm_jit\n_skvm_jit: .incbin \"%s\"'"
                    " | clang -x assembler -shared - -o %s",
                    path.c_str(), path.c_str());
            system(cmd.c_str());

            // Load that dynamic library and look up skvm_jit().
            fImpl->dylib = dlopen(path.c_str(), RTLD_NOW|RTLD_LOCAL);
            fImpl->jit_entry.store(dlsym(fImpl->dylib, "skvm_jit"));
        }
    }
#endif

}  // namespace skvm
