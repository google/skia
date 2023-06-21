/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkThreadID.h"
#include "src/base/SkHalf.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCpu.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkOpts.h"
#include "src/core/SkStreamPriv.h"
#include "src/core/SkVM.h"
#include <algorithm>
#include <atomic>
#include <queue>

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif

using namespace skia_private;

#if defined(SK_ENABLE_SKVM)

#if defined(SKSL_STANDALONE)
    // skslc needs to link against this module (for the VM code generator). This module pulls in
    // color-space code, but attempting to add those transitive dependencies to skslc gets out of
    // hand. So we terminate the chain here with stub functions. Note that skslc's usage of SkVM
    // never cares about color management.
    skvm::F32 sk_program_transfer_fn(
        skvm::F32 v, skcms_TFType tf_type,
        skvm::F32 G, skvm::F32 A, skvm::F32 B, skvm::F32 C, skvm::F32 D, skvm::F32 E, skvm::F32 F) {
            return v;
    }

    const skcms_TransferFunction* skcms_sRGB_TransferFunction() { return nullptr; }
    const skcms_TransferFunction* skcms_sRGB_Inverse_TransferFunction() { return nullptr; }
#endif

namespace skvm {

    static Features detect_features() {
        static const bool fma =
        #if defined(SK_CPU_X86)
            SkCpu::Supports(SkCpu::HSW);
        #elif defined(SK_CPU_ARM64)
            true;
        #else
            false;
        #endif

        static const bool fp16 = false;  // TODO

        return { fma, fp16 };
    }

    Builder::Builder(bool createDuplicates)
        : fFeatures(detect_features()), fCreateDuplicates(createDuplicates) {}
    Builder::Builder(Features features, bool createDuplicates)
        : fFeatures(features         ), fCreateDuplicates(createDuplicates) {}

    struct Program::Impl {
        std::vector<InterpreterInstruction> instructions;
        int regs = 0;
        int loop = 0;
        std::vector<int> strides;
        std::vector<SkSL::TraceHook*> traceHooks;
    };

    // Debugging tools, mostly for printing various data structures out to a stream.

    namespace {
        struct V { Val id; };
        struct R { Reg id; };
        struct Shift       { int bits; };
        struct Splat       { int bits; };
        struct Hex         { int bits; };
        struct TraceHookID { int bits; };
        // For op `trace_line`
        struct Line  { int bits; };
        // For op `trace_var`
        struct VarSlot { int bits; };
        // For op `trace_enter`/`trace_exit`
        struct FnIdx { int bits; };

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
            o->writeText(name(op));
        }
        static void write(SkWStream* o, Ptr p) {
            write(o, "ptr");
            o->writeDecAsText(p.ix);
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
        static void write(SkWStream* o, TraceHookID h) {
            o->writeDecAsText(h.bits);
        }
        static void write(SkWStream* o, Line d) {
            write(o, "L");
            o->writeDecAsText(d.bits);
        }
        static void write(SkWStream* o, VarSlot s) {
            write(o, "$");
            o->writeDecAsText(s.bits);
        }
        static void write(SkWStream* o, FnIdx s) {
            write(o, "F");
            o->writeDecAsText(s.bits);
        }
        template <typename T, typename... Ts>
        static void write(SkWStream* o, T first, Ts... rest) {
            write(o, first);
            write(o, " ");
            write(o, rest...);
        }
    }  // namespace

    static void write_one_instruction(Val id, const OptimizedInstruction& inst, SkWStream* o) {
        Op  op = inst.op;
        Val  x = inst.x,
             y = inst.y,
             z = inst.z,
             w = inst.w;
        int immA = inst.immA,
            immB = inst.immB,
            immC = inst.immC;
        switch (op) {
            case Op::assert_true: write(o, op, V{x}, V{y}); break;

            case Op::trace_line:  write(o, op, TraceHookID{immA}, V{x}, V{y}, Line{immB}); break;
            case Op::trace_var:   write(o, op, TraceHookID{immA}, V{x}, V{y},
                                                                  VarSlot{immB}, "=", V{z}); break;
            case Op::trace_enter: write(o, op, TraceHookID{immA}, V{x}, V{y}, FnIdx{immB}); break;
            case Op::trace_exit:  write(o, op, TraceHookID{immA}, V{x}, V{y}, FnIdx{immB}); break;
            case Op::trace_scope: write(o, op, TraceHookID{immA}, V{x}, V{y}, Shift{immB}); break;

            case Op::store8:   write(o, op, Ptr{immA}, V{x}               ); break;
            case Op::store16:  write(o, op, Ptr{immA}, V{x}               ); break;
            case Op::store32:  write(o, op, Ptr{immA}, V{x}               ); break;
            case Op::store64:  write(o, op, Ptr{immA}, V{x},V{y}          ); break;
            case Op::store128: write(o, op, Ptr{immA}, V{x},V{y},V{z},V{w}); break;

            case Op::index: write(o, V{id}, "=", op); break;

            case Op::load8:   write(o, V{id}, "=", op, Ptr{immA}); break;
            case Op::load16:  write(o, V{id}, "=", op, Ptr{immA}); break;
            case Op::load32:  write(o, V{id}, "=", op, Ptr{immA}); break;
            case Op::load64:  write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}); break;
            case Op::load128: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}); break;

            case Op::gather8:  write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, V{x}); break;
            case Op::gather16: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, V{x}); break;
            case Op::gather32: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, V{x}); break;

            case Op::uniform32: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}); break;
            case Op::array32:   write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, Hex{immC}); break;

            case Op::splat: write(o, V{id}, "=", op, Splat{immA}); break;

            case Op:: add_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: sub_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: mul_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: div_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: min_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: max_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: fma_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;
            case Op:: fms_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;
            case Op::fnma_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;


            case Op::sqrt_f32: write(o, V{id}, "=", op, V{x}); break;

            case Op:: eq_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::neq_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op:: gt_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::gte_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;


            case Op::add_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::sub_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::mul_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;

            case Op::shl_i32: write(o, V{id}, "=", op, V{x}, Shift{immA}); break;
            case Op::shr_i32: write(o, V{id}, "=", op, V{x}, Shift{immA}); break;
            case Op::sra_i32: write(o, V{id}, "=", op, V{x}, Shift{immA}); break;

            case Op::eq_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::gt_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;


            case Op::bit_and  : write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::bit_or   : write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::bit_xor  : write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::bit_clear: write(o, V{id}, "=", op, V{x}, V{y}); break;

            case Op::select: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;

            case Op::ceil:      write(o, V{id}, "=", op, V{x}); break;
            case Op::floor:     write(o, V{id}, "=", op, V{x}); break;
            case Op::to_f32:    write(o, V{id}, "=", op, V{x}); break;
            case Op::to_fp16:   write(o, V{id}, "=", op, V{x}); break;
            case Op::from_fp16: write(o, V{id}, "=", op, V{x}); break;
            case Op::trunc:     write(o, V{id}, "=", op, V{x}); break;
            case Op::round:     write(o, V{id}, "=", op, V{x}); break;

            case Op::duplicate: write(o, V{id}, "=", op, Hex{immA}); break;
        }

        write(o, "\n");
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
            write(o, inst.can_hoist ? "↑ " : "  ");
            write_one_instruction(id, inst, o);
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
                  z = inst.z,
                  w = inst.w;
            int immA = inst.immA,
                immB = inst.immB,
                immC = inst.immC;
            switch (op) {
                case Op::assert_true: write(o, op, R{x}, R{y}); break;

                case Op::trace_line:  write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, Line{immB}); break;
                case Op::trace_var:   write(o, op, TraceHookID{immA}, R{x}, R{y},
                                                   VarSlot{immB}, "=", R{z}); break;
                case Op::trace_enter: write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, FnIdx{immB}); break;
                case Op::trace_exit:  write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, FnIdx{immB}); break;
                case Op::trace_scope: write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, Shift{immB}); break;

                case Op::store8:   write(o, op, Ptr{immA}, R{x}                  ); break;
                case Op::store16:  write(o, op, Ptr{immA}, R{x}                  ); break;
                case Op::store32:  write(o, op, Ptr{immA}, R{x}                  ); break;
                case Op::store64:  write(o, op, Ptr{immA}, R{x}, R{y}            ); break;
                case Op::store128: write(o, op, Ptr{immA}, R{x}, R{y}, R{z}, R{w}); break;

                case Op::index: write(o, R{d}, "=", op); break;

                case Op::load8:   write(o, R{d}, "=", op, Ptr{immA}); break;
                case Op::load16:  write(o, R{d}, "=", op, Ptr{immA}); break;
                case Op::load32:  write(o, R{d}, "=", op, Ptr{immA}); break;
                case Op::load64:  write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}); break;
                case Op::load128: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}); break;

                case Op::gather8:  write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, R{x}); break;
                case Op::gather16: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, R{x}); break;
                case Op::gather32: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, R{x}); break;

                case Op::uniform32: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}); break;
                case Op::array32:   write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, Hex{immC}); break;

                case Op::splat:     write(o, R{d}, "=", op, Splat{immA}); break;

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

                case Op:: eq_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::neq_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op:: gt_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::gte_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;


                case Op::add_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::sub_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::mul_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::shl_i32: write(o, R{d}, "=", op, R{x}, Shift{immA}); break;
                case Op::shr_i32: write(o, R{d}, "=", op, R{x}, Shift{immA}); break;
                case Op::sra_i32: write(o, R{d}, "=", op, R{x}, Shift{immA}); break;

                case Op::eq_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::gt_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::bit_and  : write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::bit_or   : write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::bit_xor  : write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::bit_clear: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::select: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;

                case Op::ceil:      write(o, R{d}, "=", op, R{x}); break;
                case Op::floor:     write(o, R{d}, "=", op, R{x}); break;
                case Op::to_f32:    write(o, R{d}, "=", op, R{x}); break;
                case Op::to_fp16:   write(o, R{d}, "=", op, R{x}); break;
                case Op::from_fp16: write(o, R{d}, "=", op, R{x}); break;
                case Op::trunc:     write(o, R{d}, "=", op, R{x}); break;
                case Op::round:     write(o, R{d}, "=", op, R{x}); break;

                case Op::duplicate: write(o, R{d}, "=", op, Hex{immA}); break;
            }
            write(o, "\n");
        }
    }
    std::vector<Instruction> eliminate_dead_code(std::vector<Instruction> program) {
        // Determine which Instructions are live by working back from side effects.
        std::vector<bool> live(program.size(), false);
        for (Val id = program.size(); id--;) {
            if (live[id] || has_side_effect(program[id].op)) {
                live[id] = true;
                const Instruction& inst = program[id];
                for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                    if (arg != NA) { live[arg] = true; }
                }
            }
        }

        // Rewrite the program with only live Instructions:
        //   - remap IDs in live Instructions to what they'll be once dead Instructions are removed;
        //   - then actually remove the dead Instructions.
        std::vector<Val> new_id(program.size(), NA);
        for (Val id = 0, next = 0; id < (Val)program.size(); id++) {
            if (live[id]) {
                Instruction& inst = program[id];
                for (Val* arg : {&inst.x, &inst.y, &inst.z, &inst.w}) {
                    if (*arg != NA) {
                        *arg = new_id[*arg];
                        SkASSERT(*arg != NA);
                    }
                }
                new_id[id] = next++;
            }
        }

        // Eliminate any non-live ops.
        auto it = std::remove_if(program.begin(), program.end(), [&](const Instruction& inst) {
            Val id = (Val)(&inst - program.data());
            return !live[id];
        });
        program.erase(it, program.end());

        return program;
    }

    std::vector<OptimizedInstruction> finalize(const std::vector<Instruction> program) {
        std::vector<OptimizedInstruction> optimized(program.size());
        for (Val id = 0; id < (Val)program.size(); id++) {
            Instruction inst = program[id];
            optimized[id] = {inst.op, inst.x,inst.y,inst.z,inst.w,
                             inst.immA,inst.immB,inst.immC,
                             /*death=*/id, /*can_hoist=*/true};
        }

        // Each Instruction's inputs need to live at least until that Instruction issues.
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            OptimizedInstruction& inst = optimized[id];
            for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                // (We're walking in order, so this is the same as max()ing with the existing Val.)
                if (arg != NA) { optimized[arg].death = id; }
            }
        }

        // Mark which values don't depend on the loop and can be hoisted.
        for (OptimizedInstruction& inst : optimized) {
            // Varying loads (and gathers) and stores cannot be hoisted out of the loop.
            if (is_always_varying(inst.op) || is_trace(inst.op)) {
                inst.can_hoist = false;
            }

            // If any of an instruction's inputs can't be hoisted, it can't be hoisted itself.
            if (inst.can_hoist) {
                for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                    if (arg != NA) { inst.can_hoist &= optimized[arg].can_hoist; }
                }
            }
        }

        // Extend the lifetime of any hoisted value that's used in the loop to infinity.
        for (OptimizedInstruction& inst : optimized) {
            if (!inst.can_hoist /*i.e. we're in the loop, so the arguments are used-in-loop*/) {
                for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                    if (arg != NA && optimized[arg].can_hoist) {
                        optimized[arg].death = (Val)program.size();
                    }
                }
            }
        }

        return optimized;
    }

    std::vector<OptimizedInstruction> Builder::optimize() const {
        std::vector<Instruction> program = this->program();
        program = eliminate_dead_code(std::move(program));
        return    finalize           (std::move(program));
    }

    Program Builder::done(const char* debug_name, bool) const {
        char buf[64] = "skvm-jit-";
        if (!debug_name) {
            *SkStrAppendU32(buf+9, this->hash()) = '\0';
            debug_name = buf;
        }

        auto optimized = this->optimize();
        return {optimized,
                fStrides,
                fTraceHooks, debug_name, false};
    }

    uint64_t Builder::hash() const {
        return SkChecksum::Hash64(fProgram.data(), fProgram.size() * sizeof(Instruction));
    }

    bool operator!=(Ptr a, Ptr b) { return a.ix != b.ix; }

    bool operator==(const Instruction& a, const Instruction& b) {
        return a.op   == b.op
            && a.x    == b.x
            && a.y    == b.y
            && a.z    == b.z
            && a.w    == b.w
            && a.immA == b.immA
            && a.immB == b.immB
            && a.immC == b.immC;
    }

    uint32_t InstructionHash::operator()(const Instruction& inst, uint32_t seed) const {
        return SkChecksum::Hash32(&inst, sizeof(inst), seed);
    }


    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.
    Val Builder::push(Instruction inst) {
        // Basic common subexpression elimination:
        // if we've already seen this exact Instruction, use it instead of creating a new one.
        //
        // But we never dedup loads or stores: an intervening store could change that memory.
        // Uniforms and gathers touch only uniform memory, so they're fine to dedup,
        // and index is varying but doesn't touch memory, so it's fine to dedup too.
        if (!touches_varying_memory(inst.op) && !is_trace(inst.op)) {
            if (Val* id = fIndex.find(inst)) {
                if (fCreateDuplicates) {
                    inst.op = Op::duplicate;
                    inst.immA = *id;
                    fProgram.push_back(inst);
                }
                return *id;
            }
        }

        Val id = static_cast<Val>(fProgram.size());
        fProgram.push_back(inst);
        fIndex.set(inst, id);
        return id;
    }

    Ptr Builder::arg(int stride) {
        int ix = (int)fStrides.size();
        fStrides.push_back(stride);
        return {ix};
    }

    void Builder::assert_true(I32 cond, I32 debug) {
    #ifdef SK_DEBUG
        int imm;
        if (this->allImm(cond.id,&imm)) { SkASSERT(imm); return; }
        (void)push(Op::assert_true, cond.id, debug.id);
    #endif
    }

    int Builder::attachTraceHook(SkSL::TraceHook* hook) {
        int traceHookID = (int)fTraceHooks.size();
        fTraceHooks.push_back(hook);
        return traceHookID;
    }

    bool Builder::mergeMasks(I32& mask, I32& traceMask) {
        if (this->isImm(mask.id,      0)) { return false; }
        if (this->isImm(traceMask.id, 0)) { return false; }
        if (this->isImm(mask.id,     ~0)) { mask = traceMask; }
        if (this->isImm(traceMask.id,~0)) { traceMask = mask; }
        return true;
    }

    void Builder::trace_line(int traceHookID, I32 mask, I32 traceMask, int line) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_line, mask.id,traceMask.id,NA,NA, traceHookID, line);
    }
    void Builder::trace_var(int traceHookID, I32 mask, I32 traceMask, int slot, I32 val) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_var, mask.id,traceMask.id,val.id,NA, traceHookID, slot);
    }
    void Builder::trace_enter(int traceHookID, I32 mask, I32 traceMask, int fnIdx) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_enter, mask.id,traceMask.id,NA,NA, traceHookID, fnIdx);
    }
    void Builder::trace_exit(int traceHookID, I32 mask, I32 traceMask, int fnIdx) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_exit, mask.id,traceMask.id,NA,NA, traceHookID, fnIdx);
    }
    void Builder::trace_scope(int traceHookID, I32 mask, I32 traceMask, int delta) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_scope, mask.id,traceMask.id,NA,NA, traceHookID, delta);
    }

    void Builder::store8 (Ptr ptr, I32 val) { (void)push(Op::store8 , val.id,NA,NA,NA, ptr.ix); }
    void Builder::store16(Ptr ptr, I32 val) { (void)push(Op::store16, val.id,NA,NA,NA, ptr.ix); }
    void Builder::store32(Ptr ptr, I32 val) { (void)push(Op::store32, val.id,NA,NA,NA, ptr.ix); }
    void Builder::store64(Ptr ptr, I32 lo, I32 hi) {
        (void)push(Op::store64, lo.id,hi.id,NA,NA, ptr.ix);
    }
    void Builder::store128(Ptr ptr, I32 x, I32 y, I32 z, I32 w) {
        (void)push(Op::store128, x.id,y.id,z.id,w.id, ptr.ix);
    }

    I32 Builder::index() { return {this, push(Op::index)}; }

    I32 Builder::load8 (Ptr ptr) { return {this, push(Op::load8 , NA,NA,NA,NA, ptr.ix) }; }
    I32 Builder::load16(Ptr ptr) { return {this, push(Op::load16, NA,NA,NA,NA, ptr.ix) }; }
    I32 Builder::load32(Ptr ptr) { return {this, push(Op::load32, NA,NA,NA,NA, ptr.ix) }; }
    I32 Builder::load64(Ptr ptr, int lane) {
        return {this, push(Op::load64 , NA,NA,NA,NA, ptr.ix,lane) };
    }
    I32 Builder::load128(Ptr ptr, int lane) {
        return {this, push(Op::load128, NA,NA,NA,NA, ptr.ix,lane) };
    }

    I32 Builder::gather8 (UPtr ptr, int offset, I32 index) {
        return {this, push(Op::gather8 , index.id,NA,NA,NA, ptr.ix,offset)};
    }
    I32 Builder::gather16(UPtr ptr, int offset, I32 index) {
        return {this, push(Op::gather16, index.id,NA,NA,NA, ptr.ix,offset)};
    }
    I32 Builder::gather32(UPtr ptr, int offset, I32 index) {
        return {this, push(Op::gather32, index.id,NA,NA,NA, ptr.ix,offset)};
    }

    I32 Builder::uniform32(UPtr ptr, int offset) {
        return {this, push(Op::uniform32, NA,NA,NA,NA, ptr.ix, offset)};
    }

    // Note: this converts the array index into a byte offset for the op.
    I32 Builder::array32  (UPtr ptr, int offset, int index) {
        return {this, push(Op::array32, NA,NA,NA,NA, ptr.ix, offset, index * sizeof(int))};
    }

    I32 Builder::splat(int n) { return {this, push(Op::splat, NA,NA,NA,NA, n) }; }

    template <typename F32_or_I32>
    void Builder::canonicalizeIdOrder(F32_or_I32& x, F32_or_I32& y) {
        bool immX = fProgram[x.id].op == Op::splat;
        bool immY = fProgram[y.id].op == Op::splat;
        if (immX != immY) {
            if (immX) {
                // Prefer (val, imm) over (imm, val).
                std::swap(x, y);
            }
            return;
        }
        if (x.id > y.id) {
            // Prefer (lower-ID, higher-ID) over (higher-ID, lower-ID).
            std::swap(x, y);
        }
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
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X+Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0.0f)) { return x; }   // x+0 == x

        if (fFeatures.fma) {
            if (fProgram[x.id].op == Op::mul_f32) {
                return {this, this->push(Op::fma_f32, fProgram[x.id].x, fProgram[x.id].y, y.id)};
            }
            if (fProgram[y.id].op == Op::mul_f32) {
                return {this, this->push(Op::fma_f32, fProgram[y.id].x, fProgram[y.id].y, x.id)};
            }
        }
        return {this, this->push(Op::add_f32, x.id, y.id)};
    }

    F32 Builder::sub(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X-Y); }
        if (this->isImm(y.id, 0.0f)) { return x; }   // x-0 == x
        if (fFeatures.fma) {
            if (fProgram[x.id].op == Op::mul_f32) {
                return {this, this->push(Op::fms_f32, fProgram[x.id].x, fProgram[x.id].y, y.id)};
            }
            if (fProgram[y.id].op == Op::mul_f32) {
                return {this, this->push(Op::fnma_f32, fProgram[y.id].x, fProgram[y.id].y, x.id)};
            }
        }
        return {this, this->push(Op::sub_f32, x.id, y.id)};
    }

    F32 Builder::mul(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X*Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 1.0f)) { return x; }  // x*1 == x
        return {this, this->push(Op::mul_f32, x.id, y.id)};
    }

    F32 Builder::fast_mul(F32 x, F32 y) {
        if (this->isImm(x.id, 0.0f) || this->isImm(y.id, 0.0f)) { return splat(0.0f); }
        return mul(x,y);
    }

    F32 Builder::div(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(sk_ieee_float_divide(X,Y)); }
        if (this->isImm(y.id, 1.0f)) { return x; }  // x/1 == x
        return {this, this->push(Op::div_f32, x.id, y.id)};
    }

    F32 Builder::sqrt(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat(std::sqrt(X)); }
        return {this, this->push(Op::sqrt_f32, x.id)};
    }

    // See http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html.
    F32 Builder::approx_log2(F32 x) {
        // e - 127 is a fair approximation of log2(x) in its own right...
        F32 e = mul(to_F32(pun_to_I32(x)), splat(1.0f / (1<<23)));

        // ... but using the mantissa to refine its error is _much_ better.
        F32 m = pun_to_F32(bit_or(bit_and(pun_to_I32(x), 0x007fffff),
                                0x3f000000));
        F32 approx = sub(e,        124.225514990f);
            approx = sub(approx, mul(1.498030302f, m));
            approx = sub(approx, div(1.725879990f, add(0.3520887068f, m)));

        return approx;
    }

    F32 Builder::approx_pow2(F32 x) {
        constexpr float kInfinityBits = 0x7f800000;

        F32 f = fract(x);
        F32 approx = add(x,         121.274057500f);
            approx = sub(approx, mul( 1.490129070f, f));
            approx = add(approx, div(27.728023300f, sub(4.84252568f, f)));
            approx = mul(1.0f * (1<<23), approx);
            approx = clamp(approx, 0, kInfinityBits);  // guard against underflow/overflow

        return pun_to_F32(round(approx));
    }

    F32 Builder::approx_powf(F32 x, F32 y) {
        // TODO: assert this instead?  Sometimes x is very slightly negative.  See skia:10210.
        x = max(0.0f, x);

        if (this->isImm(x.id, 1.0f)) { return x; }                    // 1^y is one
        if (this->isImm(x.id, 2.0f)) { return this->approx_pow2(y); } // 2^y is pow2(y)
        if (this->isImm(y.id, 0.5f)) { return this->sqrt(x); }        // x^0.5 is sqrt(x)
        if (this->isImm(y.id, 1.0f)) { return x; }                    // x^1 is x
        if (this->isImm(y.id, 2.0f)) { return x * x; }                // x^2 is x*x

        auto is_x = bit_or(eq(x, 0.0f),
                           eq(x, 1.0f));
        return select(is_x, x, approx_pow2(mul(approx_log2(x), y)));
    }

    // Bhaskara I's sine approximation
    // 16x(pi - x) / (5*pi^2 - 4x(pi - x)
    // ... divide by 4
    // 4x(pi - x) / 5*pi^2/4 - x(pi - x)
    //
    // This is a good approximation only for 0 <= x <= pi, so we use symmetries to get
    // radians into that range first.
    //
    F32 Builder::approx_sin(F32 radians) {
        constexpr float Pi = SK_ScalarPI;
        // x = radians mod 2pi
        F32 x = fract(radians * (0.5f/Pi)) * (2*Pi);
        I32 neg = x > Pi;   // are we pi < x < 2pi --> need to negate result
        x = select(neg, x - Pi, x);

        F32 pair = x * (Pi - x);
        x = 4.0f * pair / ((5*Pi*Pi/4) - pair);
        x = select(neg, -x, x);
        return x;
    }

    /*  "GENERATING ACCURATE VALUES FOR THE TANGENT FUNCTION"
         https://mae.ufl.edu/~uhk/ACCURATE-TANGENT.pdf

        approx = x + (1/3)x^3 + (2/15)x^5 + (17/315)x^7 + (62/2835)x^9

        Some simplifications:
        1. tan(x) is periodic, -PI/2 < x < PI/2
        2. tan(x) is odd, so tan(-x) = -tan(x)
        3. Our polynomial approximation is best near zero, so we use the following identity
                        tan(x) + tan(y)
           tan(x + y) = -----------------
                       1 - tan(x)*tan(y)
           tan(PI/4) = 1

           So for x > PI/8, we do the following refactor:
           x' = x - PI/4

                    1 + tan(x')
           tan(x) = ------------
                    1 - tan(x')
     */
    F32 Builder::approx_tan(F32 x) {
        constexpr float Pi = SK_ScalarPI;
        // periodic between -pi/2 ... pi/2
        // shift to 0...Pi, scale 1/Pi to get into 0...1, then fract, scale-up, shift-back
        x = fract((1/Pi)*x + 0.5f) * Pi - (Pi/2);

        I32 neg = (x < 0.0f);
        x = select(neg, -x, x);

        // minimize total error by shifting if x > pi/8
        I32 use_quotient = (x > (Pi/8));
        x = select(use_quotient, x - (Pi/4), x);

        // 9th order poly = 4th order(x^2) * x
        x = poly(x*x, 62/2835.0f, 17/315.0f, 2/15.0f, 1/3.0f, 1.0f) * x;
        x = select(use_quotient, (1+x)/(1-x), x);
        x = select(neg, -x, x);
        return x;
    }

    // http://mathforum.org/library/drmath/view/54137.html
    // referencing Handbook of Mathematical Functions,
    //             by Milton Abramowitz and Irene Stegun
    F32 Builder::approx_asin(F32 x) {
         I32 neg = (x < 0.0f);
         x = select(neg, -x, x);
         x = SK_ScalarPI/2 - sqrt(1-x) * poly(x, -0.0187293f, 0.0742610f, -0.2121144f, 1.5707288f);
         x = select(neg, -x, x);
         return x;
    }

    /*  Use 4th order polynomial approximation from https://arachnoid.com/polysolve/
     *      with 129 values of x,atan(x) for x:[0...1]
     *  This only works for 0 <= x <= 1
     */
    static F32 approx_atan_unit(F32 x) {
        // for now we might be given NaN... let that through
        x->assert_true((x != x) | ((x >= 0) & (x <= 1)));
        return poly(x, 0.14130025741326729f,
                      -0.34312835980675116f,
                      -0.016172900528248768f,
                       1.0037696976200385f,
                      -0.00014758242182738969f);
    }

    /*  Use identity atan(x) = pi/2 - atan(1/x) for x > 1
     */
    F32 Builder::approx_atan(F32 x) {
        I32 neg = (x < 0.0f);
        x = select(neg, -x, x);
        I32 flip = (x > 1.0f);
        x = select(flip, 1/x, x);
        x = approx_atan_unit(x);
        x = select(flip, SK_ScalarPI/2 - x, x);
        x = select(neg, -x, x);
        return x;
    }

    /*  Use identity atan(x) = pi/2 - atan(1/x) for x > 1
     *  By swapping y,x to ensure the ratio is <= 1, we can safely call atan_unit()
     *  which avoids a 2nd divide instruction if we had instead called atan().
     */
    F32 Builder::approx_atan2(F32 y0, F32 x0) {

        I32 flip = (abs(y0) > abs(x0));
        F32 y = select(flip, x0, y0);
        F32 x = select(flip, y0, x0);
        F32 arg = y/x;

        I32 neg = (arg < 0.0f);
        arg = select(neg, -arg, arg);

        F32 r = approx_atan_unit(arg);
        r = select(flip, SK_ScalarPI/2 - r, r);
        r = select(neg, -r, r);

        // handle quadrant distinctions
        r = select((y0 >= 0) & (x0  < 0), r + SK_ScalarPI, r);
        r = select((y0  < 0) & (x0 <= 0), r - SK_ScalarPI, r);
        // Note: we don't try to handle 0,0 or infinities
        return r;
    }

    F32 Builder::min(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(std::min(X,Y)); }
        return {this, this->push(Op::min_f32, x.id, y.id)};
    }
    F32 Builder::max(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(std::max(X,Y)); }
        return {this, this->push(Op::max_f32, x.id, y.id)};
    }

    SK_NO_SANITIZE("signed-integer-overflow")
    I32 Builder::add(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X+Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return x; }  // x+0 == x
        return {this, this->push(Op::add_i32, x.id, y.id)};
    }
    SK_NO_SANITIZE("signed-integer-overflow")
    I32 Builder::sub(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X-Y); }
        if (this->isImm(y.id, 0)) { return x; }
        return {this, this->push(Op::sub_i32, x.id, y.id)};
    }
    SK_NO_SANITIZE("signed-integer-overflow")
    I32 Builder::mul(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X*Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return splat(0); }  // x*0 == 0
        if (this->isImm(y.id, 1)) { return x; }         // x*1 == x
        return {this, this->push(Op::mul_i32, x.id, y.id)};
    }

    SK_NO_SANITIZE("shift")
    I32 Builder::shl(I32 x, int bits) {
        if (bits == 0) { return x; }
        if (int X; this->allImm(x.id,&X)) { return splat(X << bits); }
        return {this, this->push(Op::shl_i32, x.id,NA,NA,NA, bits)};
    }
    I32 Builder::shr(I32 x, int bits) {
        if (bits == 0) { return x; }
        if (int X; this->allImm(x.id,&X)) { return splat(unsigned(X) >> bits); }
        return {this, this->push(Op::shr_i32, x.id,NA,NA,NA, bits)};
    }
    I32 Builder::sra(I32 x, int bits) {
        if (bits == 0) { return x; }
        if (int X; this->allImm(x.id,&X)) { return splat(X >> bits); }
        return {this, this->push(Op::sra_i32, x.id,NA,NA,NA, bits)};
    }

    I32 Builder:: eq(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X==Y ? ~0 : 0); }
        this->canonicalizeIdOrder(x, y);
        return {this, this->push(Op::eq_f32, x.id, y.id)};
    }
    I32 Builder::neq(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X!=Y ? ~0 : 0); }
        this->canonicalizeIdOrder(x, y);
        return {this, this->push(Op::neq_f32, x.id, y.id)};
    }
    I32 Builder::lt(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(Y> X ? ~0 : 0); }
        return {this, this->push(Op::gt_f32, y.id, x.id)};
    }
    I32 Builder::lte(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(Y>=X ? ~0 : 0); }
        return {this, this->push(Op::gte_f32, y.id, x.id)};
    }
    I32 Builder::gt(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X> Y ? ~0 : 0); }
        return {this, this->push(Op::gt_f32, x.id, y.id)};
    }
    I32 Builder::gte(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X>=Y ? ~0 : 0); }
        return {this, this->push(Op::gte_f32, x.id, y.id)};
    }

    I32 Builder:: eq(I32 x, I32 y) {
        if (x.id == y.id) { return splat(~0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X==Y ? ~0 : 0); }
        this->canonicalizeIdOrder(x, y);
        return {this, this->push(Op:: eq_i32, x.id, y.id)};
    }
    I32 Builder::neq(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X!=Y ? ~0 : 0); }
        return ~(x == y);
    }
    I32 Builder:: gt(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X> Y ? ~0 : 0); }
        return {this, this->push(Op:: gt_i32, x.id, y.id)};
    }
    I32 Builder::gte(I32 x, I32 y) {
        if (x.id == y.id) { return splat(~0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X>=Y ? ~0 : 0); }
        return ~(x < y);
    }
    I32 Builder:: lt(I32 x, I32 y) { return y>x; }
    I32 Builder::lte(I32 x, I32 y) { return y>=x; }

    Val Builder::holdsBitNot(Val id) {
        // We represent `~x` as `x ^ ~0`.
        if (fProgram[id].op == Op::bit_xor && this->isImm(fProgram[id].y, ~0)) {
            return fProgram[id].x;
        }
        return NA;
    }

    I32 Builder::bit_and(I32 x, I32 y) {
        if (x.id == y.id) { return x; }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X&Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return splat(0); }         // (x & false) == false
        if (this->isImm(y.id,~0)) { return x; }                // (x & true) == x
        if (Val notX = this->holdsBitNot(x.id); notX != NA) {  // (~x & y) == bit_clear(y, ~x)
            return bit_clear(y, {this, notX});
        }
        if (Val notY = this->holdsBitNot(y.id); notY != NA) {  // (x & ~y) == bit_clear(x, ~y)
            return bit_clear(x, {this, notY});
        }
        return {this, this->push(Op::bit_and, x.id, y.id)};
    }
    I32 Builder::bit_or(I32 x, I32 y) {
        if (x.id == y.id) { return x; }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X|Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return x; }           // (x | false) == x
        if (this->isImm(y.id,~0)) { return splat(~0); }   // (x | true) == true
        return {this, this->push(Op::bit_or, x.id, y.id)};
    }
    I32 Builder::bit_xor(I32 x, I32 y) {
        if (x.id == y.id) { return splat(0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X^Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return x; }   // (x ^ false) == x
        return {this, this->push(Op::bit_xor, x.id, y.id)};
    }

    I32 Builder::bit_clear(I32 x, I32 y) {
        if (x.id == y.id) { return splat(0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X&~Y); }
        if (this->isImm(y.id, 0)) { return x; }          // (x & ~false) == x
        if (this->isImm(y.id,~0)) { return splat(0); }   // (x & ~true) == false
        if (this->isImm(x.id, 0)) { return splat(0); }   // (false & ~y) == false
        return {this, this->push(Op::bit_clear, x.id, y.id)};
    }

    I32 Builder::select(I32 x, I32 y, I32 z) {
        if (y.id == z.id) { return y; }
        if (int X,Y,Z; this->allImm(x.id,&X, y.id,&Y, z.id,&Z)) { return splat(X?Y:Z); }
        if (this->isImm(x.id,~0)) { return y; }                // (true  ? y : z) == y
        if (this->isImm(x.id, 0)) { return z; }                // (false ? y : z) == z
        if (this->isImm(y.id, 0)) { return bit_clear(z,x); }   //     (x ? 0 : z) == ~x&z
        if (this->isImm(z.id, 0)) { return bit_and  (y,x); }   //     (x ? y : 0) ==  x&y
        if (Val notX = this->holdsBitNot(x.id); notX != NA) {  //    (!x ? y : z) == (x ? z : y)
            x.id = notX;
            std::swap(y, z);
        }
        return {this, this->push(Op::select, x.id, y.id, z.id)};
    }

    I32 Builder::extract(I32 x, int bits, I32 z) {
        if (unsigned Z; this->allImm(z.id,&Z) && (~0u>>bits) == Z) { return shr(x, bits); }
        return bit_and(z, shr(x, bits));
    }

    I32 Builder::pack(I32 x, I32 y, int bits) {
        return bit_or(x, shl(y, bits));
    }

    F32 Builder::ceil(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat(ceilf(X)); }
        return {this, this->push(Op::ceil, x.id)};
    }
    F32 Builder::floor(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat(floorf(X)); }
        return {this, this->push(Op::floor, x.id)};
    }
    F32 Builder::to_F32(I32 x) {
        if (int X; this->allImm(x.id,&X)) { return splat((float)X); }
        return {this, this->push(Op::to_f32, x.id)};
    }
    I32 Builder::trunc(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat((int)X); }
        return {this, this->push(Op::trunc, x.id)};
    }
    I32 Builder::round(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat((int)lrintf(X)); }
        return {this, this->push(Op::round, x.id)};
    }

    I32 Builder::to_fp16(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat((int)SkFloatToHalf(X)); }
        return {this, this->push(Op::to_fp16, x.id)};
    }
    F32 Builder::from_fp16(I32 x) {
        if (int X; this->allImm(x.id,&X)) { return splat(SkHalfToFloat(X)); }
        return {this, this->push(Op::from_fp16, x.id)};
    }

    F32 Builder::from_unorm(int bits, I32 x) {
        F32 limit = splat(1 / ((1<<bits)-1.0f));
        return mul(to_F32(x), limit);
    }
    I32 Builder::to_unorm(int bits, F32 x) {
        F32 limit = splat((1<<bits)-1.0f);
        return round(mul(x, limit));
    }

    PixelFormat SkColorType_to_PixelFormat(SkColorType ct) {
        auto UNORM = PixelFormat::UNORM,
             SRGB  = PixelFormat::SRGB,
             FLOAT = PixelFormat::FLOAT,
             XRNG  = PixelFormat::XRNG;
        switch (ct) {
            case kUnknown_SkColorType: break;

            case kRGBA_F32_SkColorType: return {FLOAT,32,32,32,32, 0,32,64,96};

            case kRGBA_F16Norm_SkColorType:       return {FLOAT,16,16,16,16, 0,16,32,48};
            case kRGBA_F16_SkColorType:           return {FLOAT,16,16,16,16, 0,16,32,48};
            case kR16G16B16A16_unorm_SkColorType: return {UNORM,16,16,16,16, 0,16,32,48};

            case kA16_float_SkColorType:    return {FLOAT,  0, 0,0,16, 0, 0,0,0};
            case kR16G16_float_SkColorType: return {FLOAT, 16,16,0, 0, 0,16,0,0};

            case kAlpha_8_SkColorType:  return {UNORM, 0,0,0,8, 0,0,0,0};
            case kGray_8_SkColorType:   return {UNORM, 8,8,8,0, 0,0,0,0};  // Subtle.
            case kR8_unorm_SkColorType: return {UNORM, 8,0,0,0, 0,0,0,0};

            case kRGB_565_SkColorType:   return {UNORM, 5,6,5,0, 11,5,0,0};  // (BGR)
            case kARGB_4444_SkColorType: return {UNORM, 4,4,4,4, 12,8,4,0};  // (ABGR)

            case kRGBA_8888_SkColorType:  return {UNORM, 8,8,8,8,  0,8,16,24};
            case kRGB_888x_SkColorType:   return {UNORM, 8,8,8,0,  0,8,16,32};  // 32-bit
            case kBGRA_8888_SkColorType:  return {UNORM, 8,8,8,8, 16,8, 0,24};
            case kSRGBA_8888_SkColorType: return { SRGB, 8,8,8,8,  0,8,16,24};

            case kRGBA_1010102_SkColorType:   return {UNORM, 10,10,10,2,  0,10,20,30};
            case kBGRA_1010102_SkColorType:   return {UNORM, 10,10,10,2, 20,10, 0,30};
            case kRGB_101010x_SkColorType:    return {UNORM, 10,10,10,0,  0,10,20, 0};
            case kBGR_101010x_SkColorType:    return {UNORM, 10,10,10,0, 20,10, 0, 0};
            case kBGR_101010x_XR_SkColorType: return { XRNG, 10,10,10,0, 20,10, 0, 0};

            case kR8G8_unorm_SkColorType:   return {UNORM,  8, 8,0, 0, 0, 8,0,0};
            case kR16G16_unorm_SkColorType: return {UNORM, 16,16,0, 0, 0,16,0,0};
            case kA16_unorm_SkColorType:    return {UNORM,  0, 0,0,16, 0, 0,0,0};
        }
        SkASSERT(false);
        return {UNORM, 0,0,0,0, 0,0,0,0};
    }

    static int byte_size(PixelFormat f) {
        // What's the highest bit we read?
        int bits = std::max(f.r_bits + f.r_shift,
                   std::max(f.g_bits + f.g_shift,
                   std::max(f.b_bits + f.b_shift,
                            f.a_bits + f.a_shift)));
        // Round up to bytes.
        return (bits + 7) / 8;
    }

    static Color unpack(PixelFormat f, I32 x) {
        SkASSERT(byte_size(f) <= 4);

        auto from_srgb = [](int bits, I32 channel) -> F32 {
            const skcms_TransferFunction* tf = skcms_sRGB_TransferFunction();
            F32 v = from_unorm(bits, channel);
            return sk_program_transfer_fn(v, skcms_TFType_sRGBish,
                                          v->splat(tf->g),
                                          v->splat(tf->a),
                                          v->splat(tf->b),
                                          v->splat(tf->c),
                                          v->splat(tf->d),
                                          v->splat(tf->e),
                                          v->splat(tf->f));
        };
        auto from_xr = [](int bits, I32 channel) -> F32 {
            static constexpr float min = -0.752941f;
            static constexpr float max = 1.25098f;
            static constexpr float range = max - min;
            F32 v = from_unorm(bits, channel);
            return v * range + min;
        };

        auto unpack_rgb = [=](int bits, int shift) -> F32 {
            I32 channel = extract(x, shift, (1<<bits)-1);
            switch (f.encoding) {
                case PixelFormat::UNORM: return from_unorm(bits, channel);
                case PixelFormat:: SRGB: return from_srgb (bits, channel);
                case PixelFormat::FLOAT: return from_fp16 (      channel);
                case PixelFormat:: XRNG: return from_xr   (bits, channel);
            }
            SkUNREACHABLE;
        };
        auto unpack_alpha = [=](int bits, int shift) -> F32 {
            I32 channel = extract(x, shift, (1<<bits)-1);
            switch (f.encoding) {
                case PixelFormat::UNORM:
                case PixelFormat:: SRGB: return from_unorm(bits, channel);
                case PixelFormat::FLOAT: return from_fp16 (      channel);
                case PixelFormat:: XRNG: return from_xr   (bits, channel);
            }
            SkUNREACHABLE;
        };
        return {
            f.r_bits ? unpack_rgb  (f.r_bits, f.r_shift) : x->splat(0.0f),
            f.g_bits ? unpack_rgb  (f.g_bits, f.g_shift) : x->splat(0.0f),
            f.b_bits ? unpack_rgb  (f.b_bits, f.b_shift) : x->splat(0.0f),
            f.a_bits ? unpack_alpha(f.a_bits, f.a_shift) : x->splat(1.0f),
        };
    }

    static void split_disjoint_8byte_format(PixelFormat f, PixelFormat* lo, PixelFormat* hi) {
        SkASSERT(byte_size(f) == 8);
        // We assume some of the channels are in the low 32 bits, some in the high 32 bits.
        // The assert on byte_size(lo) will trigger if this assumption is violated.
        *lo = f;
        if (f.r_shift >= 32) { lo->r_bits = 0; lo->r_shift = 32; }
        if (f.g_shift >= 32) { lo->g_bits = 0; lo->g_shift = 32; }
        if (f.b_shift >= 32) { lo->b_bits = 0; lo->b_shift = 32; }
        if (f.a_shift >= 32) { lo->a_bits = 0; lo->a_shift = 32; }
        SkASSERT(byte_size(*lo) == 4);

        *hi = f;
        if (f.r_shift < 32) { hi->r_bits = 0; hi->r_shift = 32; } else { hi->r_shift -= 32; }
        if (f.g_shift < 32) { hi->g_bits = 0; hi->g_shift = 32; } else { hi->g_shift -= 32; }
        if (f.b_shift < 32) { hi->b_bits = 0; hi->b_shift = 32; } else { hi->b_shift -= 32; }
        if (f.a_shift < 32) { hi->a_bits = 0; hi->a_shift = 32; } else { hi->a_shift -= 32; }
        SkASSERT(byte_size(*hi) == 4);
    }

    // The only 16-byte format we support today is RGBA F32,
    // though, TODO, we could generalize that to any swizzle, and to allow UNORM too.
    static void assert_16byte_is_rgba_f32(PixelFormat f) {
    #if defined(SK_DEBUG)
        SkASSERT(byte_size(f) == 16);
        PixelFormat rgba_f32 = SkColorType_to_PixelFormat(kRGBA_F32_SkColorType);

        SkASSERT(f.encoding == rgba_f32.encoding);

        SkASSERT(f.r_bits == rgba_f32.r_bits);
        SkASSERT(f.g_bits == rgba_f32.g_bits);
        SkASSERT(f.b_bits == rgba_f32.b_bits);
        SkASSERT(f.a_bits == rgba_f32.a_bits);

        SkASSERT(f.r_shift == rgba_f32.r_shift);
        SkASSERT(f.g_shift == rgba_f32.g_shift);
        SkASSERT(f.b_shift == rgba_f32.b_shift);
        SkASSERT(f.a_shift == rgba_f32.a_shift);
    #endif
    }

    Color Builder::load(PixelFormat f, Ptr ptr) {
        switch (byte_size(f)) {
            case 1: return unpack(f, load8 (ptr));
            case 2: return unpack(f, load16(ptr));
            case 4: return unpack(f, load32(ptr));
            case 8: {
                PixelFormat lo,hi;
                split_disjoint_8byte_format(f, &lo,&hi);
                Color l = unpack(lo, load64(ptr, 0)),
                      h = unpack(hi, load64(ptr, 1));
                return {
                    lo.r_bits ? l.r : h.r,
                    lo.g_bits ? l.g : h.g,
                    lo.b_bits ? l.b : h.b,
                    lo.a_bits ? l.a : h.a,
                };
            }
            case 16: {
                assert_16byte_is_rgba_f32(f);
                return {
                    pun_to_F32(load128(ptr, 0)),
                    pun_to_F32(load128(ptr, 1)),
                    pun_to_F32(load128(ptr, 2)),
                    pun_to_F32(load128(ptr, 3)),
                };
            }
            default: SkUNREACHABLE;
        }
    }

    Color Builder::gather(PixelFormat f, UPtr ptr, int offset, I32 index) {
        switch (byte_size(f)) {
            case 1: return unpack(f, gather8 (ptr, offset, index));
            case 2: return unpack(f, gather16(ptr, offset, index));
            case 4: return unpack(f, gather32(ptr, offset, index));
            case 8: {
                PixelFormat lo,hi;
                split_disjoint_8byte_format(f, &lo,&hi);
                Color l = unpack(lo, gather32(ptr, offset, (index<<1)+0)),
                      h = unpack(hi, gather32(ptr, offset, (index<<1)+1));
                return {
                    lo.r_bits ? l.r : h.r,
                    lo.g_bits ? l.g : h.g,
                    lo.b_bits ? l.b : h.b,
                    lo.a_bits ? l.a : h.a,
                };
            }
            case 16: {
                assert_16byte_is_rgba_f32(f);
                return {
                    gatherF(ptr, offset, (index<<2)+0),
                    gatherF(ptr, offset, (index<<2)+1),
                    gatherF(ptr, offset, (index<<2)+2),
                    gatherF(ptr, offset, (index<<2)+3),
                };
            }
            default: SkUNREACHABLE;
        }
    }

    static I32 pack32(PixelFormat f, Color c) {
        SkASSERT(byte_size(f) <= 4);

        auto to_srgb = [](int bits, F32 v) {
            const skcms_TransferFunction* tf = skcms_sRGB_Inverse_TransferFunction();
            return to_unorm(bits, sk_program_transfer_fn(v, skcms_TFType_sRGBish,
                                                         v->splat(tf->g),
                                                         v->splat(tf->a),
                                                         v->splat(tf->b),
                                                         v->splat(tf->c),
                                                         v->splat(tf->d),
                                                         v->splat(tf->e),
                                                         v->splat(tf->f)));
        };
        auto to_xr = [](int bits, F32 v) {
            static constexpr float min = -0.752941f;
            static constexpr float max = 1.25098f;
            static constexpr float range = max - min;
            return to_unorm(bits, (v - min) * (1.0f / range));
        };

        I32 packed = c->splat(0);
        auto pack_rgb = [&](F32 channel, int bits, int shift) {
            I32 encoded;
            switch (f.encoding) {
                case PixelFormat::UNORM: encoded = to_unorm(bits, channel); break;
                case PixelFormat:: SRGB: encoded = to_srgb (bits, channel); break;
                case PixelFormat::FLOAT: encoded = to_fp16 (      channel); break;
                case PixelFormat:: XRNG: encoded = to_xr   (bits, channel); break;
            }
            packed = pack(packed, encoded, shift);
        };
        auto pack_alpha = [&](F32 channel, int bits, int shift) {
            I32 encoded;
            switch (f.encoding) {
                case PixelFormat::UNORM:
                case PixelFormat:: SRGB: encoded = to_unorm(bits, channel); break;
                case PixelFormat::FLOAT: encoded = to_fp16 (      channel); break;
                case PixelFormat:: XRNG: encoded = to_xr   (bits, channel); break;
            }
            packed = pack(packed, encoded, shift);
        };
        if (f.r_bits) { pack_rgb  (c.r, f.r_bits, f.r_shift); }
        if (f.g_bits) { pack_rgb  (c.g, f.g_bits, f.g_shift); }
        if (f.b_bits) { pack_rgb  (c.b, f.b_bits, f.b_shift); }
        if (f.a_bits) { pack_alpha(c.a, f.a_bits, f.a_shift); }
        return packed;
    }

    void Builder::store(PixelFormat f, Ptr ptr, Color c) {
        // Detect a grayscale PixelFormat: r,g,b bit counts and shifts all equal.
        if (f.r_bits  == f.g_bits  && f.g_bits  == f.b_bits &&
            f.r_shift == f.g_shift && f.g_shift == f.b_shift) {

            // TODO: pull these coefficients from an SkColorSpace?  This is sRGB luma/luminance.
            c.r = c.r * 0.2126f
                + c.g * 0.7152f
                + c.b * 0.0722f;
            f.g_bits = f.b_bits = 0;
        }

        switch (byte_size(f)) {
            case 1: store8 (ptr, pack32(f,c)); break;
            case 2: store16(ptr, pack32(f,c)); break;
            case 4: store32(ptr, pack32(f,c)); break;
            case 8: {
                PixelFormat lo,hi;
                split_disjoint_8byte_format(f, &lo,&hi);
                store64(ptr, pack32(lo,c)
                           , pack32(hi,c));
                break;
            }
            case 16: {
                assert_16byte_is_rgba_f32(f);
                store128(ptr, pun_to_I32(c.r), pun_to_I32(c.g), pun_to_I32(c.b), pun_to_I32(c.a));
                break;
            }
            default: SkUNREACHABLE;
        }
    }

    void Builder::unpremul(F32* r, F32* g, F32* b, F32 a) {
        skvm::F32 invA = 1.0f / a,
                  inf  = pun_to_F32(splat(0x7f800000));
        // If a is 0, so are *r,*g,*b, so set invA to 0 to avoid 0*inf=NaN (instead 0*0 = 0).
        invA = select(invA < inf, invA
                                , 0.0f);
        *r *= invA;
        *g *= invA;
        *b *= invA;
    }

    void Builder::premul(F32* r, F32* g, F32* b, F32 a) {
        *r *= a;
        *g *= a;
        *b *= a;
    }

    Color Builder::uniformColor(SkColor4f color, Uniforms* uniforms) {
        auto [r,g,b,a] = color;
        return {
            uniformF(uniforms->pushF(r)),
            uniformF(uniforms->pushF(g)),
            uniformF(uniforms->pushF(b)),
            uniformF(uniforms->pushF(a)),
        };
    }

    F32 Builder::lerp(F32 lo, F32 hi, F32 t) {
        if (this->isImm(t.id, 0.0f)) { return lo; }
        if (this->isImm(t.id, 1.0f)) { return hi; }
        return mad(sub(hi, lo), t, lo);
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
          invd = 1.0f / d,
        g_lt_b = select(c.g < c.b, splat(6.0f)
                                 , splat(0.0f));

        F32 h = (1/6.0f) * select(mx == mn,  0.0f,
                           select(mx == c.r, invd * (c.g - c.b) + g_lt_b,
                           select(mx == c.g, invd * (c.b - c.r) + 2.0f
                                           , invd * (c.r - c.g) + 4.0f)));

        F32 sum = mx + mn,
              l = sum * 0.5f,
              s = select(mx == mn, 0.0f
                                 , d / select(l > 0.5f, 2.0f - sum
                                                      , sum));
        return {h, s, l, c.a};
    }

    Color Builder::to_rgba(HSLA c) {
        // See GrRGBToHSLFilterEffect.fp

        auto [h,s,l,a] = c;
        F32 x = s * (1.0f - abs(l + l - 1.0f));

        auto hue_to_rgb = [&,l=l](auto hue) {
            auto q = abs(6.0f * fract(hue) - 3.0f) - 1.0f;
            return x * (clamp01(q) - 0.5f) + l;
        };

        return {
            hue_to_rgb(h + 0/3.0f),
            hue_to_rgb(h + 2/3.0f),
            hue_to_rgb(h + 1/3.0f),
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

    static skvm::F32 saturation(skvm::F32 r, skvm::F32 g, skvm::F32 b) {
        return max(r, max(g, b))
             - min(r, min(g, b));
    }

    static skvm::F32 luminance(skvm::F32 r, skvm::F32 g, skvm::F32 b) {
        return r*0.30f + g*0.59f + b*0.11f;
    }

    static void set_sat(skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 s) {
        F32 mn  = min(*r, min(*g, *b)),
            mx  = max(*r, max(*g, *b)),
            sat = mx - mn;

        // Map min channel to 0, max channel to s, and scale the middle proportionally.
        auto scale = [&](skvm::F32 c) {
            auto scaled = ((c - mn) * s) / sat;
            return select(is_finite(scaled), scaled, 0.0f);
        };
        *r = scale(*r);
        *g = scale(*g);
        *b = scale(*b);
    }

    static void set_lum(skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 lu) {
        auto diff = lu - luminance(*r, *g, *b);
        *r += diff;
        *g += diff;
        *b += diff;
    }

    static void clip_color(skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 a) {
        F32 mn  = min(*r, min(*g, *b)),
            mx  = max(*r, max(*g, *b)),
            lu = luminance(*r, *g, *b);

        auto clip = [&](auto c) {
            c = select(mn < 0 & lu != mn, lu + ((c-lu)*(  lu)) / (lu-mn), c);
            c = select(mx > a & lu != mx, lu + ((c-lu)*(a-lu)) / (mx-lu), c);
            return clamp01(c);  // May be a little negative, or worse, NaN.
        };
        *r = clip(*r);
        *g = clip(*g);
        *b = clip(*b);
    }

    Color Builder::blend(SkBlendMode mode, Color src, Color dst) {
        auto mma = [](skvm::F32 x, skvm::F32 y, skvm::F32 z, skvm::F32 w) {
            return x*y + z*w;
        };

        auto two = [](skvm::F32 x) { return x+x; };

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
                R + mma(src.r, 1-dst.a,  dst.r, 1-src.a),
                G + mma(src.g, 1-dst.a,  dst.g, 1-src.a),
                B + mma(src.b, 1-dst.a,  dst.b, 1-src.a),
                mad(dst.a, 1-src.a, src.a),   // srcover for alpha
            };
        };

        switch (mode) {
            default:
                SkASSERT(false);
                [[fallthrough]]; /*but also, for safety, fallthrough*/

            case SkBlendMode::kClear: return { splat(0.0f), splat(0.0f), splat(0.0f), splat(0.0f) };

            case SkBlendMode::kSrc: return src;
            case SkBlendMode::kDst: return dst;

            case SkBlendMode::kDstOver: std::swap(src, dst); [[fallthrough]];
            case SkBlendMode::kSrcOver:
                return apply_rgba([&](auto s, auto d) {
                    return mad(d,1-src.a, s);
                });

            case SkBlendMode::kDstIn: std::swap(src, dst); [[fallthrough]];
            case SkBlendMode::kSrcIn:
                return apply_rgba([&](auto s, auto d) {
                    return s * dst.a;
                });

            case SkBlendMode::kDstOut: std::swap(src, dst); [[fallthrough]];

            case SkBlendMode::kSrcOut:
                return apply_rgba([&](auto s, auto d) {
                    return s * (1-dst.a);
                });

            case SkBlendMode::kDstATop: std::swap(src, dst); [[fallthrough]];
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
                    return min(s+d, 1.0f);
                });

            case SkBlendMode::kModulate:
                return apply_rgba([&](auto s, auto d) {
                    return s * d;
                });

            case SkBlendMode::kScreen:
                // (s+d)-(s*d) gave us trouble with our "r,g,b <= after blending" asserts.
                // It's kind of plausible that s + (d - sd) keeps more precision?
                return apply_rgba([&](auto s, auto d) {
                    return s + (d - s*d);
                });

            case SkBlendMode::kDarken:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - max(s * dst.a,
                                        d * src.a));
                });

            case SkBlendMode::kLighten:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - min(s * dst.a,
                                        d * src.a));
                });

            case SkBlendMode::kDifference:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - two(min(s * dst.a,
                                            d * src.a)));
                });

            case SkBlendMode::kExclusion:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - two(s * d));
                });

            case SkBlendMode::kColorBurn:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    auto mn   = min(dst.a,
                                    src.a * (dst.a - d) / s),
                         burn = src.a * (dst.a - mn) + mma(s, 1-dst.a, d, 1-src.a);
                    return select(d == dst.a     , s * (1-dst.a) + d,
                           select(is_finite(burn), burn
                                                 , d * (1-src.a) + s));
                });

            case SkBlendMode::kColorDodge:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    auto dodge = src.a * min(dst.a,
                                             d * src.a / (src.a - s))
                                       + mma(s, 1-dst.a, d, 1-src.a);
                    return select(d == 0.0f       , s * (1-dst.a) + d,
                           select(is_finite(dodge), dodge
                                                  , d * (1-src.a) + s));
                });

            case SkBlendMode::kHardLight:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return mma(s, 1-dst.a, d, 1-src.a) +
                           select(two(s) <= src.a,
                                  two(s * d),
                                  src.a * dst.a - two((dst.a - d) * (src.a - s)));
                });

            case SkBlendMode::kOverlay:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return mma(s, 1-dst.a, d, 1-src.a) +
                           select(two(d) <= dst.a,
                                  two(s * d),
                                  src.a * dst.a - two((dst.a - d) * (src.a - s)));
                });

            case SkBlendMode::kMultiply:
                return apply_rgba([&](auto s, auto d) {
                    return mma(s, 1-dst.a, d, 1-src.a) + s * d;
                });

            case SkBlendMode::kSoftLight:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    auto  m = select(dst.a > 0.0f, d / dst.a
                                                 , 0.0f),
                         s2 = two(s),
                         m4 = 4*m;

                         // The logic forks three ways:
                         //    1. dark src?
                         //    2. light src, dark dst?
                         //    3. light src, light dst?

                         // Used in case 1
                    auto darkSrc = d * ((s2-src.a) * (1-m) + src.a),
                         // Used in case 2
                         darkDst = (m4 * m4 + m4) * (m-1) + 7*m,
                         // Used in case 3.
                         liteDst = sqrt(m) - m,
                         // Used in 2 or 3?
                         liteSrc = dst.a * (s2 - src.a) * select(4*d <= dst.a, darkDst
                                                                             , liteDst)
                                   + d * src.a;
                    return s * (1-dst.a) + d * (1-src.a) + select(s2 <= src.a, darkSrc
                                                                             , liteSrc);
                });

            case SkBlendMode::kHue: {
                skvm::F32 R = src.r * src.a,
                          G = src.g * src.a,
                          B = src.b * src.a;

                set_sat   (&R, &G, &B, src.a * saturation(dst.r, dst.g, dst.b));
                set_lum   (&R, &G, &B, src.a * luminance (dst.r, dst.g, dst.b));
                clip_color(&R, &G, &B, src.a * dst.a);

                return non_sep(R, G, B);
            }

            case SkBlendMode::kSaturation: {
                skvm::F32 R = dst.r * src.a,
                          G = dst.g * src.a,
                          B = dst.b * src.a;

                set_sat   (&R, &G, &B, dst.a * saturation(src.r, src.g, src.b));
                set_lum   (&R, &G, &B, src.a * luminance (dst.r, dst.g, dst.b));
                clip_color(&R, &G, &B, src.a * dst.a);

                return non_sep(R, G, B);
            }

            case SkBlendMode::kColor: {
                skvm::F32 R = src.r * dst.a,
                          G = src.g * dst.a,
                          B = src.b * dst.a;

                set_lum   (&R, &G, &B, src.a * luminance(dst.r, dst.g, dst.b));
                clip_color(&R, &G, &B, src.a * dst.a);

                return non_sep(R, G, B);
            }

            case SkBlendMode::kLuminosity: {
                skvm::F32 R = dst.r * src.a,
                          G = dst.g * src.a,
                          B = dst.b * src.a;

                set_lum   (&R, &G, &B, dst.a * luminance(src.r, src.g, src.b));
                clip_color(&R, &G, &B, dst.a * src.a);

                return non_sep(R, G, B);
            }
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

    Assembler::Assembler(void* buf) : fCode((uint8_t*)buf), fSize(0) {}

    size_t Assembler::size() const { return fSize; }

    void Assembler::bytes(const void* p, int n) {
        if (fCode) {
            memcpy(fCode+fSize, p, n);
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

    void Assembler::op(int opcode, Operand dst, GP64 x) {
        if (dst.kind == Operand::REG) {
            this->byte(rex(W1,x>>3,0,dst.reg>>3));
            this->bytes(&opcode, SkTFitsIn<uint8_t>(opcode) ? 1 : 2);
            this->byte(mod_rm(Mod::Direct, x, dst.reg&7));
        } else {
            SkASSERT(dst.kind == Operand::MEM);
            const Mem& m = dst.mem;
            const bool need_SIB = (m.base&7) == rsp
                               || m.index != rsp;

            this->byte(rex(W1,x>>3,m.index>>3,m.base>>3));
            this->bytes(&opcode, SkTFitsIn<uint8_t>(opcode) ? 1 : 2);
            this->byte(mod_rm(mod(m.disp), x&7, (need_SIB ? rsp : m.base)&7));
            if (need_SIB) {
                this->byte(sib(m.scale, m.index&7, m.base&7));
            }
            this->bytes(&m.disp, imm_bytes(mod(m.disp)));
        }
    }

    void Assembler::op(int opcode, int opcode_ext, Operand dst, int imm) {
        opcode |= 0b1000'0000;   // top bit set for instructions with any immediate

        int imm_bytes = 4;
        if (SkTFitsIn<int8_t>(imm)) {
            imm_bytes = 1;
            opcode |= 0b0000'0010;  // second bit set for 8-bit immediate, else 32-bit.
        }

        this->op(opcode, dst, (GP64)opcode_ext);
        this->bytes(&imm, imm_bytes);
    }

    void Assembler::add(Operand dst, int imm) { this->op(0x01,0b000, dst,imm); }
    void Assembler::sub(Operand dst, int imm) { this->op(0x01,0b101, dst,imm); }
    void Assembler::cmp(Operand dst, int imm) { this->op(0x01,0b111, dst,imm); }

    // These don't work quite like the other instructions with immediates:
    // these immediates are always fixed size at 4 bytes or 1 byte.
    void Assembler::mov(Operand dst, int imm) {
        this->op(0xC7,dst,(GP64)0b000);
        this->word(imm);
    }
    void Assembler::movb(Operand dst, int imm) {
        this->op(0xC6,dst,(GP64)0b000);
        this->byte(imm);
    }

    void Assembler::add (Operand dst, GP64 x) { this->op(0x01, dst,x); }
    void Assembler::sub (Operand dst, GP64 x) { this->op(0x29, dst,x); }
    void Assembler::cmp (Operand dst, GP64 x) { this->op(0x39, dst,x); }
    void Assembler::mov (Operand dst, GP64 x) { this->op(0x89, dst,x); }
    void Assembler::movb(Operand dst, GP64 x) { this->op(0x88, dst,x); }

    void Assembler::add (GP64 dst, Operand x) { this->op(0x03, x,dst); }
    void Assembler::sub (GP64 dst, Operand x) { this->op(0x2B, x,dst); }
    void Assembler::cmp (GP64 dst, Operand x) { this->op(0x3B, x,dst); }
    void Assembler::mov (GP64 dst, Operand x) { this->op(0x8B, x,dst); }
    void Assembler::movb(GP64 dst, Operand x) { this->op(0x8A, x,dst); }

    void Assembler::movzbq(GP64 dst, Operand x) { this->op(0xB60F, x,dst); }
    void Assembler::movzwq(GP64 dst, Operand x) { this->op(0xB70F, x,dst); }

    void Assembler::vpaddd (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xfe, dst,x,y); }
    void Assembler::vpsubd (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xfa, dst,x,y); }
    void Assembler::vpmulld(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x40, dst,x,y); }

    void Assembler::vpaddw   (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xfd, dst,x,y); }
    void Assembler::vpsubw   (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xf9, dst,x,y); }
    void Assembler::vpmullw  (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xd5, dst,x,y); }
    void Assembler::vpavgw   (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xe3, dst,x,y); }
    void Assembler::vpmulhrsw(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x0b, dst,x,y); }
    void Assembler::vpminsw  (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xea, dst,x,y); }
    void Assembler::vpmaxsw  (Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0xee, dst,x,y); }
    void Assembler::vpminuw  (Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x3a, dst,x,y); }
    void Assembler::vpmaxuw  (Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x3e, dst,x,y); }

    void Assembler::vpabsw(Ymm dst, Operand x) { this->op(0x66,0x380f,0x1d, dst,x); }


    void Assembler::vpand (Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0xdb, dst,x,y); }
    void Assembler::vpor  (Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0xeb, dst,x,y); }
    void Assembler::vpxor (Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0xef, dst,x,y); }
    void Assembler::vpandn(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0xdf, dst,x,y); }

    void Assembler::vaddps(Ymm dst, Ymm x, Operand y) { this->op(0,0x0f,0x58, dst,x,y); }
    void Assembler::vsubps(Ymm dst, Ymm x, Operand y) { this->op(0,0x0f,0x5c, dst,x,y); }
    void Assembler::vmulps(Ymm dst, Ymm x, Operand y) { this->op(0,0x0f,0x59, dst,x,y); }
    void Assembler::vdivps(Ymm dst, Ymm x, Operand y) { this->op(0,0x0f,0x5e, dst,x,y); }
    void Assembler::vminps(Ymm dst, Ymm x, Operand y) { this->op(0,0x0f,0x5d, dst,x,y); }
    void Assembler::vmaxps(Ymm dst, Ymm x, Operand y) { this->op(0,0x0f,0x5f, dst,x,y); }

    void Assembler::vfmadd132ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x98, dst,x,y); }
    void Assembler::vfmadd213ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0xa8, dst,x,y); }
    void Assembler::vfmadd231ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0xb8, dst,x,y); }

    void Assembler::vfmsub132ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x9a, dst,x,y); }
    void Assembler::vfmsub213ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0xaa, dst,x,y); }
    void Assembler::vfmsub231ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0xba, dst,x,y); }

    void Assembler::vfnmadd132ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x9c, dst,x,y); }
    void Assembler::vfnmadd213ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0xac, dst,x,y); }
    void Assembler::vfnmadd231ps(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0xbc, dst,x,y); }

    void Assembler::vpackusdw(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x2b, dst,x,y); }
    void Assembler::vpackuswb(Ymm dst, Ymm x, Operand y) { this->op(0x66,  0x0f,0x67, dst,x,y); }

    void Assembler::vpunpckldq(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0x62, dst,x,y); }
    void Assembler::vpunpckhdq(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0x6a, dst,x,y); }

    void Assembler::vpcmpeqd(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0x76, dst,x,y); }
    void Assembler::vpcmpeqw(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0x75, dst,x,y); }
    void Assembler::vpcmpgtd(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0x66, dst,x,y); }
    void Assembler::vpcmpgtw(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x0f,0x65, dst,x,y); }


    void Assembler::imm_byte_after_operand(const Operand& operand, int imm) {
        // When we've embedded a label displacement in the middle of an instruction,
        // we need to tweak it a little so that the resolved displacement starts
        // from the end of the instruction and not the end of the displacement.
        if (operand.kind == Operand::LABEL && fCode) {
            int disp;
            memcpy(&disp, fCode+fSize-4, 4);
            disp--;
            memcpy(fCode+fSize-4, &disp, 4);
        }
        this->byte(imm);
    }

    void Assembler::vcmpps(Ymm dst, Ymm x, Operand y, int imm) {
        this->op(0,0x0f,0xc2, dst,x,y);
        this->imm_byte_after_operand(y, imm);
    }

    void Assembler::vpblendvb(Ymm dst, Ymm x, Operand y, Ymm z) {
        this->op(0x66,0x3a0f,0x4c, dst,x,y);
        this->imm_byte_after_operand(y, z << 4);
    }

    // Shift instructions encode their opcode extension as "dst", dst as x, and x as y.
    void Assembler::vpslld(Ymm dst, Ymm x, int imm) {
        this->op(0x66,0x0f,0x72,(Ymm)6, dst,x);
        this->byte(imm);
    }
    void Assembler::vpsrld(Ymm dst, Ymm x, int imm) {
        this->op(0x66,0x0f,0x72,(Ymm)2, dst,x);
        this->byte(imm);
    }
    void Assembler::vpsrad(Ymm dst, Ymm x, int imm) {
        this->op(0x66,0x0f,0x72,(Ymm)4, dst,x);
        this->byte(imm);
    }
    void Assembler::vpsllw(Ymm dst, Ymm x, int imm) {
        this->op(0x66,0x0f,0x71,(Ymm)6, dst,x);
        this->byte(imm);
    }
    void Assembler::vpsrlw(Ymm dst, Ymm x, int imm) {
        this->op(0x66,0x0f,0x71,(Ymm)2, dst,x);
        this->byte(imm);
    }
    void Assembler::vpsraw(Ymm dst, Ymm x, int imm) {
        this->op(0x66,0x0f,0x71,(Ymm)4, dst,x);
        this->byte(imm);
    }

    void Assembler::vpermq(Ymm dst, Operand x, int imm) {
        // A bit unusual among the instructions we use, this is 64-bit operation, so we set W.
        this->op(0x66,0x3a0f,0x00, dst,x,W1);
        this->imm_byte_after_operand(x, imm);
    }

    void Assembler::vperm2f128(Ymm dst, Ymm x, Operand y, int imm) {
        this->op(0x66,0x3a0f,0x06, dst,x,y);
        this->imm_byte_after_operand(y, imm);
    }

    void Assembler::vpermps(Ymm dst, Ymm ix, Operand src) {
        this->op(0x66,0x380f,0x16, dst,ix,src);
    }

    void Assembler::vroundps(Ymm dst, Operand x, Rounding imm) {
        this->op(0x66,0x3a0f,0x08, dst,x);
        this->imm_byte_after_operand(x, imm);
    }

    void Assembler::vmovdqa(Ymm dst, Operand src) { this->op(0x66,0x0f,0x6f, dst,src); }
    void Assembler::vmovups(Ymm dst, Operand src) { this->op(   0,0x0f,0x10, dst,src); }
    void Assembler::vmovups(Xmm dst, Operand src) { this->op(   0,0x0f,0x10, dst,src); }
    void Assembler::vmovups(Operand dst, Ymm src) { this->op(   0,0x0f,0x11, src,dst); }
    void Assembler::vmovups(Operand dst, Xmm src) { this->op(   0,0x0f,0x11, src,dst); }

    void Assembler::vcvtdq2ps (Ymm dst, Operand x) { this->op(   0,0x0f,0x5b, dst,x); }
    void Assembler::vcvttps2dq(Ymm dst, Operand x) { this->op(0xf3,0x0f,0x5b, dst,x); }
    void Assembler::vcvtps2dq (Ymm dst, Operand x) { this->op(0x66,0x0f,0x5b, dst,x); }
    void Assembler::vsqrtps   (Ymm dst, Operand x) { this->op(   0,0x0f,0x51, dst,x); }

    void Assembler::vcvtps2ph(Operand dst, Ymm x, Rounding imm) {
        this->op(0x66,0x3a0f,0x1d, x,dst);
        this->imm_byte_after_operand(dst, imm);
    }
    void Assembler::vcvtph2ps(Ymm dst, Operand x) {
        this->op(0x66,0x380f,0x13, dst,x);
    }

    int Assembler::disp19(Label* l) {
        SkASSERT(l->kind == Label::NotYetSet ||
                 l->kind == Label::ARMDisp19);
        int here = (int)this->size();
        l->kind = Label::ARMDisp19;
        l->references.push_back(here);
        // ARM 19-bit instruction count, from the beginning of this instruction.
        return (l->offset - here) / 4;
    }

    int Assembler::disp32(Label* l) {
        SkASSERT(l->kind == Label::NotYetSet ||
                 l->kind == Label::X86Disp32);
        int here = (int)this->size();
        l->kind = Label::X86Disp32;
        l->references.push_back(here);
        // x86 32-bit byte count, from the end of this instruction.
        return l->offset - (here + 4);
    }

    void Assembler::op(int prefix, int map, int opcode, int dst, int x, Operand y, W w, L l) {
        switch (y.kind) {
            case Operand::REG: {
                VEX v = vex(w, dst>>3, 0, y.reg>>3,
                            map, x, l, prefix);
                this->bytes(v.bytes, v.len);
                this->byte(opcode);
                this->byte(mod_rm(Mod::Direct, dst&7, y.reg&7));
            } return;

            case Operand::MEM: {
                // Passing rsp as the rm argument to mod_rm() signals an SIB byte follows;
                // without an SIB byte, that's where the base register would usually go.
                // This means we have to use an SIB byte if we want to use rsp as a base register.
                const Mem& m = y.mem;
                const bool need_SIB = m.base  == rsp
                                   || m.index != rsp;

                VEX v = vex(w, dst>>3, m.index>>3, m.base>>3,
                            map, x, l, prefix);
                this->bytes(v.bytes, v.len);
                this->byte(opcode);
                this->byte(mod_rm(mod(m.disp), dst&7, (need_SIB ? rsp : m.base)&7));
                if (need_SIB) {
                    this->byte(sib(m.scale, m.index&7, m.base&7));
                }
                this->bytes(&m.disp, imm_bytes(mod(m.disp)));
            } return;

            case Operand::LABEL: {
                // IP-relative addressing uses Mod::Indirect with the R/M encoded as-if rbp or r13.
                const int rip = rbp;

                VEX v = vex(w, dst>>3, 0, rip>>3,
                            map, x, l, prefix);
                this->bytes(v.bytes, v.len);
                this->byte(opcode);
                this->byte(mod_rm(Mod::Indirect, dst&7, rip&7));
                this->word(this->disp32(y.label));
            } return;
        }
    }

    void Assembler::vpshufb(Ymm dst, Ymm x, Operand y) { this->op(0x66,0x380f,0x00, dst,x,y); }

    void Assembler::vptest(Ymm x, Operand y) { this->op(0x66, 0x380f, 0x17, x,y); }

    void Assembler::vbroadcastss(Ymm dst, Operand y) { this->op(0x66,0x380f,0x18, dst,y); }

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

    void Assembler::vpmovzxwd(Ymm dst, Operand src) { this->op(0x66,0x380f,0x33, dst,src); }
    void Assembler::vpmovzxbd(Ymm dst, Operand src) { this->op(0x66,0x380f,0x31, dst,src); }

    void Assembler::vmovq(Operand dst, Xmm src) { this->op(0x66,0x0f,0xd6, src,dst); }

    void Assembler::vmovd(Operand dst, Xmm src) { this->op(0x66,0x0f,0x7e, src,dst); }
    void Assembler::vmovd(Xmm dst, Operand src) { this->op(0x66,0x0f,0x6e, dst,src); }

    void Assembler::vpinsrd(Xmm dst, Xmm src, Operand y, int imm) {
        this->op(0x66,0x3a0f,0x22, dst,src,y);
        this->imm_byte_after_operand(y, imm);
    }
    void Assembler::vpinsrw(Xmm dst, Xmm src, Operand y, int imm) {
        this->op(0x66,0x0f,0xc4, dst,src,y);
        this->imm_byte_after_operand(y, imm);
    }
    void Assembler::vpinsrb(Xmm dst, Xmm src, Operand y, int imm) {
        this->op(0x66,0x3a0f,0x20, dst,src,y);
        this->imm_byte_after_operand(y, imm);
    }

    void Assembler::vextracti128(Operand dst, Ymm src, int imm) {
        this->op(0x66,0x3a0f,0x39, src,dst);
        SkASSERT(dst.kind != Operand::LABEL);
        this->byte(imm);
    }
    void Assembler::vpextrd(Operand dst, Xmm src, int imm) {
        this->op(0x66,0x3a0f,0x16, src,dst);
        SkASSERT(dst.kind != Operand::LABEL);
        this->byte(imm);
    }
    void Assembler::vpextrw(Operand dst, Xmm src, int imm) {
        this->op(0x66,0x3a0f,0x15, src,dst);
        SkASSERT(dst.kind != Operand::LABEL);
        this->byte(imm);
    }
    void Assembler::vpextrb(Operand dst, Xmm src, int imm) {
        this->op(0x66,0x3a0f,0x14, src,dst);
        SkASSERT(dst.kind != Operand::LABEL);
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
        this->byte(mod_rm(Mod::Indirect, dst&7, rsp/*use SIB*/));
        this->byte(sib(scale, ix&7, base&7));
    }

    // https://static.docs.arm.com/ddi0596/a/DDI_0596_ARM_a64_instruction_set_architecture.pdf

    static int mask(unsigned long long bits) { return (1<<(int)bits)-1; }

    void Assembler::op(uint32_t hi, V m, uint32_t lo, V n, V d) {
        this->word( (hi & mask(11)) << 21
                  | (m  & mask(5)) << 16
                  | (lo & mask(6)) << 10
                  | (n  & mask(5)) <<  5
                  | (d  & mask(5)) <<  0);
    }
    void Assembler::op(uint32_t op22, V n, V d, int imm) {
        this->word( (op22 & mask(22)) << 10
                  | imm  // size and location depends on the instruction
                  | (n    & mask(5)) <<  5
                  | (d    & mask(5)) <<  0);
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

    void Assembler::fneg4s (V d, V n) { this->op(0b0'1'1'01110'1'0'10000'01111'10, n,d); }
    void Assembler::fsqrt4s(V d, V n) { this->op(0b0'1'1'01110'1'0'10000'11111'10, n,d); }

    void Assembler::fcmeq4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b1110'0'1, n, d); }
    void Assembler::fcmgt4s(V d, V n, V m) { this->op(0b0'1'1'01110'1'0'1, m, 0b1110'0'1, n, d); }
    void Assembler::fcmge4s(V d, V n, V m) { this->op(0b0'1'1'01110'0'0'1, m, 0b1110'0'1, n, d); }

    void Assembler::fmla4s(V d, V n, V m) { this->op(0b0'1'0'01110'0'0'1, m, 0b11001'1, n, d); }
    void Assembler::fmls4s(V d, V n, V m) { this->op(0b0'1'0'01110'1'0'1, m, 0b11001'1, n, d); }

    void Assembler::tbl(V d, V n, V m) { this->op(0b0'1'001110'00'0, m, 0b0'00'0'00, n, d); }

    void Assembler::uzp14s(V d, V n, V m) { this->op(0b0'1'001110'10'0, m, 0b0'0'01'10, n, d); }
    void Assembler::uzp24s(V d, V n, V m) { this->op(0b0'1'001110'10'0, m, 0b0'1'01'10, n, d); }
    void Assembler::zip14s(V d, V n, V m) { this->op(0b0'1'001110'10'0, m, 0b0'0'11'10, n, d); }
    void Assembler::zip24s(V d, V n, V m) { this->op(0b0'1'001110'10'0, m, 0b0'1'11'10, n, d); }

    void Assembler::sli4s(V d, V n, int imm5) {
        this->op(0b0'1'1'011110'0100'000'01010'1,    n, d, ( imm5 & mask(5))<<16);
    }
    void Assembler::shl4s(V d, V n, int imm5) {
        this->op(0b0'1'0'011110'0100'000'01010'1,    n, d, ( imm5 & mask(5))<<16);
    }
    void Assembler::sshr4s(V d, V n, int imm5) {
        this->op(0b0'1'0'011110'0100'000'00'0'0'0'1, n, d, (-imm5 & mask(5))<<16);
    }
    void Assembler::ushr4s(V d, V n, int imm5) {
        this->op(0b0'1'1'011110'0100'000'00'0'0'0'1, n, d, (-imm5 & mask(5))<<16);
    }
    void Assembler::ushr8h(V d, V n, int imm4) {
        this->op(0b0'1'1'011110'0010'000'00'0'0'0'1, n, d, (-imm4 & mask(4))<<16);
    }

    void Assembler::scvtf4s (V d, V n) { this->op(0b0'1'0'01110'0'0'10000'11101'10, n,d); }
    void Assembler::fcvtzs4s(V d, V n) { this->op(0b0'1'0'01110'1'0'10000'1101'1'10, n,d); }
    void Assembler::fcvtns4s(V d, V n) { this->op(0b0'1'0'01110'0'0'10000'1101'0'10, n,d); }
    void Assembler::frintp4s(V d, V n) { this->op(0b0'1'0'01110'1'0'10000'1100'0'10, n,d); }
    void Assembler::frintm4s(V d, V n) { this->op(0b0'1'0'01110'0'0'10000'1100'1'10, n,d); }

    void Assembler::fcvtn(V d, V n) { this->op(0b0'0'0'01110'0'0'10000'10110'10, n,d); }
    void Assembler::fcvtl(V d, V n) { this->op(0b0'0'0'01110'0'0'10000'10111'10, n,d); }

    void Assembler::xtns2h(V d, V n) { this->op(0b0'0'0'01110'01'10000'10010'10, n,d); }
    void Assembler::xtnh2b(V d, V n) { this->op(0b0'0'0'01110'00'10000'10010'10, n,d); }

    void Assembler::uxtlb2h(V d, V n) { this->op(0b0'0'1'011110'0001'000'10100'1, n,d); }
    void Assembler::uxtlh2s(V d, V n) { this->op(0b0'0'1'011110'0010'000'10100'1, n,d); }

    void Assembler::uminv4s(V d, V n) { this->op(0b0'1'1'01110'10'11000'1'1010'10, n,d); }

    void Assembler::brk(int imm16) {
        this->op(0b11010100'001'00000000000, (imm16 & mask(16)) << 5);
    }

    void Assembler::ret(X n) { this->op(0b1101011'0'0'10'11111'0000'0'0, n, (X)0); }

    void Assembler::add(X d, X n, int imm12) {
        this->op(0b1'0'0'10001'00'000000000000, n,d, (imm12 & mask(12)) << 10);
    }
    void Assembler::sub(X d, X n, int imm12) {
        this->op(0b1'1'0'10001'00'000000000000, n,d, (imm12 & mask(12)) << 10);
    }
    void Assembler::subs(X d, X n, int imm12) {
        this->op(0b1'1'1'10001'00'000000000000, n,d, (imm12 & mask(12)) << 10);
    }

    void Assembler::add(X d, X n, X m, Shift shift, int imm6) {
        SkASSERT(shift != ROR);

        int imm = (imm6  & mask(6)) << 0
                | (m     & mask(5)) << 6
                | (0     & mask(1)) << 11
                | (shift & mask(2)) << 12;
        this->op(0b1'0'0'01011'00'0'00000'000000, n,d, imm << 10);
    }

    void Assembler::b(Condition cond, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b0101010'0'00000000000000, (X)0, (V)cond, (imm19 & mask(19)) << 5);
    }
    void Assembler::cbz(X t, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b1'011010'0'00000000000000, (X)0, t, (imm19 & mask(19)) << 5);
    }
    void Assembler::cbnz(X t, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b1'011010'1'00000000000000, (X)0, t, (imm19 & mask(19)) << 5);
    }

    void Assembler::ldrd(X dst, X src, int imm12) {
        this->op(0b11'111'0'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }
    void Assembler::ldrs(X dst, X src, int imm12) {
        this->op(0b10'111'0'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }
    void Assembler::ldrh(X dst, X src, int imm12) {
        this->op(0b01'111'0'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }
    void Assembler::ldrb(X dst, X src, int imm12) {
        this->op(0b00'111'0'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }

    void Assembler::ldrq(V dst, X src, int imm12) {
        this->op(0b00'111'1'01'11'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }
    void Assembler::ldrd(V dst, X src, int imm12) {
        this->op(0b11'111'1'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }
    void Assembler::ldrs(V dst, X src, int imm12) {
        this->op(0b10'111'1'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }
    void Assembler::ldrh(V dst, X src, int imm12) {
        this->op(0b01'111'1'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }
    void Assembler::ldrb(V dst, X src, int imm12) {
        this->op(0b00'111'1'01'01'000000000000, src, dst, (imm12 & mask(12)) << 10);
    }

    void Assembler::strs(X src, X dst, int imm12) {
        this->op(0b10'111'0'01'00'000000000000, dst, src, (imm12 & mask(12)) << 10);
    }

    void Assembler::strq(V src, X dst, int imm12) {
        this->op(0b00'111'1'01'10'000000000000, dst, src, (imm12 & mask(12)) << 10);
    }
    void Assembler::strd(V src, X dst, int imm12) {
        this->op(0b11'111'1'01'00'000000000000, dst, src, (imm12 & mask(12)) << 10);
    }
    void Assembler::strs(V src, X dst, int imm12) {
        this->op(0b10'111'1'01'00'000000000000, dst, src, (imm12 & mask(12)) << 10);
    }
    void Assembler::strh(V src, X dst, int imm12) {
        this->op(0b01'111'1'01'00'000000000000, dst, src, (imm12 & mask(12)) << 10);
    }
    void Assembler::strb(V src, X dst, int imm12) {
        this->op(0b00'111'1'01'00'000000000000, dst, src, (imm12 & mask(12)) << 10);
    }

    void Assembler::movs(X dst, V src, int lane) {
        int imm5 = (lane << 3) | 0b100;
        this->op(0b0'0'0'01110000'00000'0'01'1'1'1, src, dst, (imm5 & mask(5)) << 16);
    }
    void Assembler::inss(V dst, X src, int lane) {
        int imm5 = (lane << 3) | 0b100;
        this->op(0b0'1'0'01110000'00000'0'0011'1, src, dst, (imm5 & mask(5)) << 16);
    }


    void Assembler::ldrq(V dst, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b10'011'1'00'00000000000000, (V)0, dst, (imm19 & mask(19)) << 5);
    }

    void Assembler::dup4s(V dst, X src) {
        this->op(0b0'1'0'01110000'00100'0'0001'1, src, dst);
    }

    void Assembler::ld1r4s(V dst, X src) {
        this->op(0b0'1'0011010'1'0'00000'110'0'10, src, dst);
    }
    void Assembler::ld1r8h(V dst, X src) {
        this->op(0b0'1'0011010'1'0'00000'110'0'01, src, dst);
    }
    void Assembler::ld1r16b(V dst, X src) {
        this->op(0b0'1'0011010'1'0'00000'110'0'00, src, dst);
    }

    void Assembler::ld24s(V dst, X src) { this->op(0b0'1'0011000'1'000000'1000'10, src, dst); }
    void Assembler::ld44s(V dst, X src) { this->op(0b0'1'0011000'1'000000'0000'10, src, dst); }
    void Assembler::st24s(V src, X dst) { this->op(0b0'1'0011000'0'000000'1000'10, dst, src); }
    void Assembler::st44s(V src, X dst) { this->op(0b0'1'0011000'0'000000'0000'10, dst, src); }

    void Assembler::ld24s(V dst, X src, int lane) {
        int Q = (lane & 2)>>1,
            S = (lane & 1);
                 /*  Q                       S */
        this->op(0b0'0'0011010'1'1'00000'100'0'00, src, dst, (Q<<30)|(S<<12));
    }
    void Assembler::ld44s(V dst, X src, int lane) {
        int Q = (lane & 2)>>1,
            S = (lane & 1);
        this->op(0b0'0'0011010'1'1'00000'101'0'00, src, dst, (Q<<30)|(S<<12));
    }

    void Assembler::label(Label* l) {
        if (fCode) {
            // The instructions all currently point to l->offset.
            // We'll want to add a delta to point them to here.
            int here = (int)this->size();
            int delta = here - l->offset;
            l->offset = here;

            if (l->kind == Label::ARMDisp19) {
                for (int ref : l->references) {
                    // ref points to a 32-bit instruction with 19-bit displacement in instructions.
                    uint32_t inst;
                    memcpy(&inst, fCode + ref, 4);

                    // [ 8 bits to preserve] [ 19 bit signed displacement ] [ 5 bits to preserve ]
                    int disp = (int)(inst << 8) >> 13;

                    disp += delta/4;  // delta is in bytes, we want instructions.

                    // Put it all back together, preserving the high 8 bits and low 5.
                    inst = ((disp << 5) &  (mask(19) << 5))
                         | ((inst     ) & ~(mask(19) << 5));
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
        // So we'll sometimes use the interpreter here even if later calls will use the JIT.
        SkOpts::interpret_skvm(fImpl->instructions.data(), (int)fImpl->instructions.size(),
                               this->nregs(), this->loop(), fImpl->strides.data(),
                               fImpl->traceHooks.data(), fImpl->traceHooks.size(),
                               this->nargs(), n, args);
    }

    bool Program::hasTraceHooks() const {
        // Identifies a program which has been instrumented for debugging.
        return !fImpl->traceHooks.empty();
    }

    Program::Program() : fImpl(std::make_unique<Impl>()) {}

    Program::~Program() {}

    Program::Program(Program&& other) : fImpl(std::move(other.fImpl)) {}

    Program& Program::operator=(Program&& other) {
        fImpl = std::move(other.fImpl);
        return *this;
    }

    Program::Program(const std::vector<OptimizedInstruction>& instructions,
                     const std::vector<int>& strides,
                     const std::vector<SkSL::TraceHook*>& traceHooks,
                     const char* debug_name, bool) : Program() {
        fImpl->strides = strides;
        fImpl->traceHooks = traceHooks;

        this->setupInterpreter(instructions);
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
        // We have effectively infinite registers, so we hoist any value we can.
        // (The JIT may choose a more complex policy to reduce register pressure.)

        fImpl->regs = 0;
        std::vector<Reg> avail;

        // Assign this value to a register, recycling them where we can.
        auto assign_register = [&](Val id) {
            const OptimizedInstruction& inst = instructions[id];

            // If this is a real input and it's lifetime ends at this instruction,
            // we can recycle the register it's occupying.
            auto maybe_recycle_register = [&](Val input) {
                if (input != NA && instructions[input].death == id) {
                    avail.push_back(reg[input]);
                }
            };

            // Take care to not recycle the same register twice.
            const Val x = inst.x, y = inst.y, z = inst.z, w = inst.w;
            if (true                      ) { maybe_recycle_register(x); }
            if (y != x                    ) { maybe_recycle_register(y); }
            if (z != x && z != y          ) { maybe_recycle_register(z); }
            if (w != x && w != y && w != z) { maybe_recycle_register(w); }

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
            if ( instructions[id].can_hoist) { assign_register(id); }
        }
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            if (!instructions[id].can_hoist) { assign_register(id); }
        }

        // Translate OptimizedInstructions to InterpreterIstructions by mapping values to
        // registers.  This will be two passes, first hoisted instructions, then inside the loop.

        // The loop begins at the fImpl->loop'th Instruction.
        fImpl->loop = 0;
        fImpl->instructions.reserve(instructions.size());

        // Add a mapping for the N/A sentinel Val to any arbitrary register
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
                lookup_register(inst.y),
                lookup_register(inst.z),
                lookup_register(inst.w),
                inst.immA,
                inst.immB,
                inst.immC,
            };
            fImpl->instructions.push_back(pinst);
        };

        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const OptimizedInstruction& inst = instructions[id];
            if (inst.can_hoist) {
                push_instruction(id, inst);
                fImpl->loop++;
            }
        }
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const OptimizedInstruction& inst = instructions[id];
            if (!inst.can_hoist) {
                push_instruction(id, inst);
            }
        }
    }

}  // namespace skvm

#endif  // defined(SK_ENABLE_SKVM)
