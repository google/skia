/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkHalf.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkThreadID.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCpu.h"
#include "src/core/SkEnumerate.h"
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
    #include <llvm/Support/Host.h>

    // Platform-specific intrinsics got their own files in LLVM 10.
    #if __has_include(<llvm/IR/IntrinsicsX86.h>)
        #include <llvm/IR/IntrinsicsX86.h>
    #endif
#endif

// #define SKVM_LLVM_WAIT_FOR_COMPILATION

bool gSkVMAllowJIT{false};
bool gSkVMJITViaDylib{false};

#if defined(SKVM_JIT)
    #if defined(SK_BUILD_FOR_WIN)
        #include "src/core/SkLeanWindows.h"
        #include <memoryapi.h>

        static void* alloc_jit_buffer(size_t* len) {
            return VirtualAlloc(NULL, *len, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        }
        static void remap_as_executable(void* ptr, size_t len) {
            DWORD old;
            VirtualProtect(ptr, len, PAGE_EXECUTE_READ, &old);
            SkASSERT(old == PAGE_READWRITE);
        }
        #if !defined(SKVM_LLVM)
        static void unmap_jit_buffer(void* ptr, size_t len) {
            VirtualFree(ptr, 0, MEM_RELEASE);
        }
        static void close_dylib(void* dylib) {
            SkASSERT(false);  // TODO?  For now just assert we never make one.
        }
        #endif
    #else
        #include <dlfcn.h>
        #include <sys/mman.h>

        static void* alloc_jit_buffer(size_t* len) {
            // While mprotect and VirtualAlloc both work at page granularity,
            // mprotect doesn't round up for you, and instead requires *len is at page granularity.
            const size_t page = sysconf(_SC_PAGESIZE);
            *len = ((*len + page - 1) / page) * page;
            return mmap(nullptr,*len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1,0);
        }
        static void remap_as_executable(void* ptr, size_t len) {
            mprotect(ptr, len, PROT_READ|PROT_EXEC);
            __builtin___clear_cache((char*)ptr,
                                    (char*)ptr + len);
        }
        #if !defined(SKVM_LLVM)
        static void unmap_jit_buffer(void* ptr, size_t len) {
            munmap(ptr, len);
        }
        static void close_dylib(void* dylib) {
            dlclose(dylib);
        }
        #endif
    #endif

    #if defined(SKVM_JIT_VTUNE)
        #include <jitprofiling.h>
        static void notify_vtune(const char* name, void* addr, size_t len) {
            if (iJIT_IsProfilingActive() == iJIT_SAMPLING_ON) {
                iJIT_Method_Load event;
                memset(&event, 0, sizeof(event));
                event.method_id           = iJIT_GetNewMethodID();
                event.method_name         = const_cast<char*>(name);
                event.method_load_address = addr;
                event.method_size         = len;
                iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &event);
            }
        }
    #else
        static void notify_vtune(const char* name, void* addr, size_t len) {}
    #endif
#endif

// JIT code isn't MSAN-instrumented, so we won't see when it uses
// uninitialized memory, and we'll not see the writes it makes as properly
// initializing memory.  Instead force the interpreter, which should let
// MSAN see everything our programs do properly.
//
// Similarly, we can't get ASAN's checks unless we let it instrument our interpreter.
#if defined(__has_feature)
    #if __has_feature(memory_sanitizer) || __has_feature(address_sanitizer)
        #define SKVM_JIT_BUT_IGNORE_IT
    #endif
#endif

#if defined(SKSL_STANDALONE)
    // skslc needs to link against this module (for the VM code generator). This module pulls in
    // color-space code, but attempting to add those transitive dependencies to skslc gets out of
    // hand. So we terminate the chain here with stub functions. Note that skslc's usage of SkVM
    // never cares about color management.
    skvm::F32 sk_program_transfer_fn(
        skvm::F32 v, TFKind tf_kind,
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

    Builder::Builder()                  : fFeatures(detect_features()) {}
    Builder::Builder(Features features) : fFeatures(features         ) {}


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
                SkDebugf("%.*s", (int)size, (const char*)buffer);
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
        // For op `trace_line` or `trace_call`
        struct Line  { int bits; };
        // For op `trace_var`
        struct VarSlot { int bits; };
        struct VarType { int bits; };
        static constexpr VarType kVarTypeInt{0};
        static constexpr VarType kVarTypeFloat{1};
        static constexpr VarType kVarTypeBool{2};
        // For op `trace_call`
        struct CallType { int bits; };
        static constexpr CallType kCallTypeEnter{1};
        static constexpr CallType kCallTypeExit{0};

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
        static void write(SkWStream* o, Line d) {
            write(o, "L");
            o->writeDecAsText(d.bits);
        }
        static void write(SkWStream* o, VarSlot s) {
            write(o, "$");
            o->writeDecAsText(s.bits);
        }
        static void write(SkWStream* o, VarType n) {
            if (n.bits == kVarTypeFloat.bits) {
                write(o, "(F32)");
            } else if (n.bits == kVarTypeInt.bits) {
                write(o, "(I32)");
            } else if (n.bits == kVarTypeBool.bits) {
                write(o, "(bool)");
            } else {
                write(o, "???");
            }
        }
        static void write(SkWStream* o, CallType n) {
            if (n.bits == kCallTypeEnter.bits) {
                write(o, "(enter)");
            } else if (n.bits == kCallTypeExit.bits) {
                write(o, "(exit)");
            } else {
                write(o, "???");
            }
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

            case Op::trace_line: write(o, op, V{x}, Line{immA}); break;
            case Op::trace_var:  write(o, op, V{x}, VarSlot{immA}, "=", V{y}, VarType{immB}); break;
            case Op::trace_call: write(o, op, V{x}, Line{immA}, CallType{immB}); break;

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

                case Op::trace_line: write(o, op, R{x}, Line{immA}); break;
                case Op::trace_var: write(o, op, R{x}, VarSlot{immA}, "=", R{y}, VarType{immB});
                                    break;
                case Op::trace_call: write(o, op, R{x}, Line{immA}, CallType{immB}); break;

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

        // After removing non-live instructions, we can be left with redundant back-to-back
        // trace_line instructions. (e.g. one line could have multiple statements on it.)
        // Eliminate any duplicate ops.
        int lastId = -1;
        for (Val id = 0; id < (Val)program.size(); id++) {
            if (!live[id]) {
                continue;
            }
            const Instruction& inst = program[id];
            if (inst.op != Op::trace_line) {
                lastId = -1;
                continue;
            }
            if (lastId >= 0) {
                const Instruction& last = program[lastId];
                if (inst.immA == last.immA && inst.x == last.x) {
                    // Found two matching trace_lines in a row. Mark the first one as dead.
                    live[lastId] = false;
                }
            }
            lastId = id;
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

    Program Builder::done(const char* debug_name, bool allow_jit) const {
        char buf[64] = "skvm-jit-";
        if (!debug_name) {
            *SkStrAppendU32(buf+9, this->hash()) = '\0';
            debug_name = buf;
        }

        return {this->optimize(), fStrides, debug_name, allow_jit};
    }

    uint64_t Builder::hash() const {
        uint32_t lo = SkOpts::hash(fProgram.data(), fProgram.size() * sizeof(Instruction), 0),
                 hi = SkOpts::hash(fProgram.data(), fProgram.size() * sizeof(Instruction), 1);
        return (uint64_t)lo | (uint64_t)hi << 32;
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
        return SkOpts::hash(&inst, sizeof(inst), seed);
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

    void Builder::trace_line(I32 mask, int line) {
        if (this->isImm(mask.id, 0)) { return; }
        (void)push(Op::trace_line, mask.id,NA,NA,NA, line);
    }
    void Builder::trace_var(I32 mask, int slot, I32 val) {
        if (this->isImm(mask.id, 0)) { return; }
        (void)push(Op::trace_var, mask.id,val.id,NA,NA, slot, kVarTypeInt.bits);
    }
    void Builder::trace_var(I32 mask, int slot, F32 val) {
        if (this->isImm(mask.id, 0)) { return; }
        (void)push(Op::trace_var, mask.id,val.id,NA,NA, slot, kVarTypeFloat.bits);
    }
    void Builder::trace_var(I32 mask, int slot, bool b) {
        if (this->isImm(mask.id, 0)) { return; }
        I32 val = b ? this->splat(1) : this->splat(0);
        (void)push(Op::trace_var, mask.id,val.id,NA,NA, slot, kVarTypeBool.bits);
    }
    void Builder::trace_call_enter(I32 mask, int line) {
        if (this->isImm(mask.id, 0)) { return; }
        (void)push(Op::trace_call, mask.id,NA,NA,NA, line, kCallTypeEnter.bits);
    }
    void Builder::trace_call_exit(I32 mask, int line) {
        if (this->isImm(mask.id, 0)) { return; }
        (void)push(Op::trace_call, mask.id,NA,NA,NA, line, kCallTypeExit.bits);
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
        if (this->isImm(y.id, 0.0f)) { return x; }   // x+0 == x
        if (this->isImm(x.id, 0.0f)) { return y; }   // 0+y == y

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
        if (this->isImm(y.id, 1.0f)) { return x; }  // x*1 == x
        if (this->isImm(x.id, 1.0f)) { return y; }  // 1*y == y
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
        F32 f = fract(x);
        F32 approx = add(x,         121.274057500f);
            approx = sub(approx, mul( 1.490129070f, f));
            approx = add(approx, div(27.728023300f, sub(4.84252568f, f)));

        return pun_to_F32(round(mul(1.0f * (1<<23), approx)));
    }

    F32 Builder::approx_powf(F32 x, F32 y) {
        // TODO: assert this instead?  Sometimes x is very slightly negative.  See skia:10210.
        x = max(0.0f, x);

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
        // Note: we don't try to handle 0,0 or infinities (yet)
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

    SK_ATTRIBUTE(no_sanitize("signed-integer-overflow"))
    I32 Builder::add(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X+Y); }
        if (this->isImm(x.id, 0)) { return y; }
        if (this->isImm(y.id, 0)) { return x; }
        return {this, this->push(Op::add_i32, x.id, y.id)};
    }
    SK_ATTRIBUTE(no_sanitize("signed-integer-overflow"))
    I32 Builder::sub(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X-Y); }
        if (this->isImm(y.id, 0)) { return x; }
        return {this, this->push(Op::sub_i32, x.id, y.id)};
    }
    SK_ATTRIBUTE(no_sanitize("signed-integer-overflow"))
    I32 Builder::mul(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X*Y); }
        if (this->isImm(x.id, 0)) { return splat(0); }
        if (this->isImm(y.id, 0)) { return splat(0); }
        if (this->isImm(x.id, 1)) { return y; }
        if (this->isImm(y.id, 1)) { return x; }
        return {this, this->push(Op::mul_i32, x.id, y.id)};
    }

    SK_ATTRIBUTE(no_sanitize("shift"))
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
        return {this, this->push(Op::eq_f32, x.id, y.id)};
    }
    I32 Builder::neq(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X!=Y ? ~0 : 0); }
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

    I32 Builder::bit_and(I32 x, I32 y) {
        if (x.id == y.id) { return x; }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X&Y); }
        if (this->isImm(y.id, 0)) { return splat(0); }   // (x & false) == false
        if (this->isImm(x.id, 0)) { return splat(0); }   // (false & y) == false
        if (this->isImm(y.id,~0)) { return x; }          // (x & true) == x
        if (this->isImm(x.id,~0)) { return y; }          // (true & y) == y
        return {this, this->push(Op::bit_and, x.id, y.id)};
    }
    I32 Builder::bit_or(I32 x, I32 y) {
        if (x.id == y.id) { return x; }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X|Y); }
        if (this->isImm(y.id, 0)) { return x; }           // (x | false) == x
        if (this->isImm(x.id, 0)) { return y; }           // (false | y) == y
        if (this->isImm(y.id,~0)) { return splat(~0); }   // (x | true) == true
        if (this->isImm(x.id,~0)) { return splat(~0); }   // (true | y) == true
        return {this, this->push(Op::bit_or, x.id, y.id)};
    }
    I32 Builder::bit_xor(I32 x, I32 y) {
        if (x.id == y.id) { return splat(0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X^Y); }
        if (this->isImm(y.id, 0)) { return x; }   // (x ^ false) == x
        if (this->isImm(x.id, 0)) { return y; }   // (false ^ y) == y
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
        if (this->isImm(x.id,~0)) { return y; }               // true  ? y : z == y
        if (this->isImm(x.id, 0)) { return z; }               // false ? y : z == z
        if (this->isImm(y.id, 0)) { return bit_clear(z,x); }  //     x ? 0 : z == ~x&z
        if (this->isImm(z.id, 0)) { return bit_and  (y,x); }  //     x ? y : 0 ==  x&y
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
             FLOAT = PixelFormat::FLOAT;
        switch (ct) {
            case kUnknown_SkColorType: break;

            case kRGBA_F32_SkColorType: return {FLOAT,32,32,32,32, 0,32,64,96};

            case kRGBA_F16Norm_SkColorType:       return {FLOAT,16,16,16,16, 0,16,32,48};
            case kRGBA_F16_SkColorType:           return {FLOAT,16,16,16,16, 0,16,32,48};
            case kR16G16B16A16_unorm_SkColorType: return {UNORM,16,16,16,16, 0,16,32,48};

            case kA16_float_SkColorType:    return {FLOAT,  0, 0,0,16, 0, 0,0,0};
            case kR16G16_float_SkColorType: return {FLOAT, 16,16,0, 0, 0,16,0,0};

            case kAlpha_8_SkColorType: return {UNORM, 0,0,0,8, 0,0,0,0};
            case kGray_8_SkColorType:  return {UNORM, 8,8,8,0, 0,0,0,0};  // Subtle.

            case kRGB_565_SkColorType:   return {UNORM, 5,6,5,0, 11,5,0,0};  // (BGR)
            case kARGB_4444_SkColorType: return {UNORM, 4,4,4,4, 12,8,4,0};  // (ABGR)

            case kRGBA_8888_SkColorType:  return {UNORM, 8,8,8,8,  0,8,16,24};
            case kRGB_888x_SkColorType:   return {UNORM, 8,8,8,0,  0,8,16,32};  // 32-bit
            case kBGRA_8888_SkColorType:  return {UNORM, 8,8,8,8, 16,8, 0,24};
            case kSRGBA_8888_SkColorType: return { SRGB, 8,8,8,8,  0,8,16,24};

            case kRGBA_1010102_SkColorType: return {UNORM, 10,10,10,2,  0,10,20,30};
            case kBGRA_1010102_SkColorType: return {UNORM, 10,10,10,2, 20,10, 0,30};
            case kRGB_101010x_SkColorType:  return {UNORM, 10,10,10,0,  0,10,20, 0};
            case kBGR_101010x_SkColorType:  return {UNORM, 10,10,10,0, 20,10, 0, 0};

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
            return sk_program_transfer_fn(v, sRGBish_TF,
                                          v->splat(tf->g),
                                          v->splat(tf->a),
                                          v->splat(tf->b),
                                          v->splat(tf->c),
                                          v->splat(tf->d),
                                          v->splat(tf->e),
                                          v->splat(tf->f));
        };

        auto unpack_rgb = [=](int bits, int shift) -> F32 {
            I32 channel = extract(x, shift, (1<<bits)-1);
            switch (f.encoding) {
                case PixelFormat::UNORM: return from_unorm(bits, channel);
                case PixelFormat:: SRGB: return from_srgb (bits, channel);
                case PixelFormat::FLOAT: return from_fp16 (      channel);
            }
            SkUNREACHABLE;
        };
        auto unpack_alpha = [=](int bits, int shift) -> F32 {
            I32 channel = extract(x, shift, (1<<bits)-1);
            switch (f.encoding) {
                case PixelFormat::UNORM:
                case PixelFormat:: SRGB: return from_unorm(bits, channel);
                case PixelFormat::FLOAT: return from_fp16 (      channel);
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
        return {};
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
        return {};
    }

    static I32 pack32(PixelFormat f, Color c) {
        SkASSERT(byte_size(f) <= 4);

        auto to_srgb = [](int bits, F32 v) {
            const skcms_TransferFunction* tf = skcms_sRGB_Inverse_TransferFunction();
            return to_unorm(bits, sk_program_transfer_fn(v, sRGBish_TF,
                                                         v->splat(tf->g),
                                                         v->splat(tf->a),
                                                         v->splat(tf->b),
                                                         v->splat(tf->c),
                                                         v->splat(tf->d),
                                                         v->splat(tf->e),
                                                         v->splat(tf->f)));
        };

        I32 packed = c->splat(0);
        auto pack_rgb = [&](F32 channel, int bits, int shift) {
            I32 encoded;
            switch (f.encoding) {
                case PixelFormat::UNORM: encoded = to_unorm(bits, channel); break;
                case PixelFormat:: SRGB: encoded = to_srgb (bits, channel); break;
                case PixelFormat::FLOAT: encoded = to_fp16 (      channel); break;
            }
            packed = pack(packed, encoded, shift);
        };
        auto pack_alpha = [&](F32 channel, int bits, int shift) {
            I32 encoded;
            switch (f.encoding) {
                case PixelFormat::UNORM:
                case PixelFormat:: SRGB: encoded = to_unorm(bits, channel); break;
                case PixelFormat::FLOAT: encoded = to_fp16 (      channel); break;
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
            c = select(mn >= 0, c
                              , lu + ((c-lu)*(  lu)) / (lu-mn));
            c = select(mx >  a, lu + ((c-lu)*(a-lu)) / (mx-lu)
                              , c);
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

    static int operator"" _mask(unsigned long long bits) { return (1<<(int)bits)-1; }

    void Assembler::op(uint32_t hi, V m, uint32_t lo, V n, V d) {
        this->word( (hi & 11_mask) << 21
                  | (m  &  5_mask) << 16
                  | (lo &  6_mask) << 10
                  | (n  &  5_mask) <<  5
                  | (d  &  5_mask) <<  0);
    }
    void Assembler::op(uint32_t op22, V n, V d, int imm) {
        this->word( (op22 & 22_mask) << 10
                  | imm  // size and location depends on the instruction
                  | (n    &  5_mask) <<  5
                  | (d    &  5_mask) <<  0);
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
        this->op(0b0'1'1'011110'0100'000'01010'1,    n, d, ( imm5 & 5_mask)<<16);
    }
    void Assembler::shl4s(V d, V n, int imm5) {
        this->op(0b0'1'0'011110'0100'000'01010'1,    n, d, ( imm5 & 5_mask)<<16);
    }
    void Assembler::sshr4s(V d, V n, int imm5) {
        this->op(0b0'1'0'011110'0100'000'00'0'0'0'1, n, d, (-imm5 & 5_mask)<<16);
    }
    void Assembler::ushr4s(V d, V n, int imm5) {
        this->op(0b0'1'1'011110'0100'000'00'0'0'0'1, n, d, (-imm5 & 5_mask)<<16);
    }
    void Assembler::ushr8h(V d, V n, int imm4) {
        this->op(0b0'1'1'011110'0010'000'00'0'0'0'1, n, d, (-imm4 & 4_mask)<<16);
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
        this->op(0b11010100'001'00000000000, (imm16 & 16_mask) << 5);
    }

    void Assembler::ret(X n) { this->op(0b1101011'0'0'10'11111'0000'0'0, n, (X)0); }

    void Assembler::add(X d, X n, int imm12) {
        this->op(0b1'0'0'10001'00'000000000000, n,d, (imm12 & 12_mask) << 10);
    }
    void Assembler::sub(X d, X n, int imm12) {
        this->op(0b1'1'0'10001'00'000000000000, n,d, (imm12 & 12_mask) << 10);
    }
    void Assembler::subs(X d, X n, int imm12) {
        this->op(0b1'1'1'10001'00'000000000000, n,d, (imm12 & 12_mask) << 10);
    }

    void Assembler::add(X d, X n, X m, Shift shift, int imm6) {
        SkASSERT(shift != ROR);

        int imm = (imm6  & 6_mask) << 0
                | (m     & 5_mask) << 6
                | (0     & 1_mask) << 11
                | (shift & 2_mask) << 12;
        this->op(0b1'0'0'01011'00'0'00000'000000, n,d, imm << 10);
    }

    void Assembler::b(Condition cond, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b0101010'0'00000000000000, (X)0, (V)cond, (imm19 & 19_mask) << 5);
    }
    void Assembler::cbz(X t, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b1'011010'0'00000000000000, (X)0, t, (imm19 & 19_mask) << 5);
    }
    void Assembler::cbnz(X t, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b1'011010'1'00000000000000, (X)0, t, (imm19 & 19_mask) << 5);
    }

    void Assembler::ldrd(X dst, X src, int imm12) {
        this->op(0b11'111'0'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }
    void Assembler::ldrs(X dst, X src, int imm12) {
        this->op(0b10'111'0'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }
    void Assembler::ldrh(X dst, X src, int imm12) {
        this->op(0b01'111'0'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }
    void Assembler::ldrb(X dst, X src, int imm12) {
        this->op(0b00'111'0'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }

    void Assembler::ldrq(V dst, X src, int imm12) {
        this->op(0b00'111'1'01'11'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }
    void Assembler::ldrd(V dst, X src, int imm12) {
        this->op(0b11'111'1'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }
    void Assembler::ldrs(V dst, X src, int imm12) {
        this->op(0b10'111'1'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }
    void Assembler::ldrh(V dst, X src, int imm12) {
        this->op(0b01'111'1'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }
    void Assembler::ldrb(V dst, X src, int imm12) {
        this->op(0b00'111'1'01'01'000000000000, src, dst, (imm12 & 12_mask) << 10);
    }

    void Assembler::strs(X src, X dst, int imm12) {
        this->op(0b10'111'0'01'00'000000000000, dst, src, (imm12 & 12_mask) << 10);
    }

    void Assembler::strq(V src, X dst, int imm12) {
        this->op(0b00'111'1'01'10'000000000000, dst, src, (imm12 & 12_mask) << 10);
    }
    void Assembler::strd(V src, X dst, int imm12) {
        this->op(0b11'111'1'01'00'000000000000, dst, src, (imm12 & 12_mask) << 10);
    }
    void Assembler::strs(V src, X dst, int imm12) {
        this->op(0b10'111'1'01'00'000000000000, dst, src, (imm12 & 12_mask) << 10);
    }
    void Assembler::strh(V src, X dst, int imm12) {
        this->op(0b01'111'1'01'00'000000000000, dst, src, (imm12 & 12_mask) << 10);
    }
    void Assembler::strb(V src, X dst, int imm12) {
        this->op(0b00'111'1'01'00'000000000000, dst, src, (imm12 & 12_mask) << 10);
    }

    void Assembler::movs(X dst, V src, int lane) {
        int imm5 = (lane << 3) | 0b100;
        this->op(0b0'0'0'01110000'00000'0'01'1'1'1, src, dst, (imm5 & 5_mask) << 16);
    }
    void Assembler::inss(V dst, X src, int lane) {
        int imm5 = (lane << 3) | 0b100;
        this->op(0b0'1'0'01110000'00000'0'0011'1, src, dst, (imm5 & 5_mask) << 16);
    }


    void Assembler::ldrq(V dst, Label* l) {
        const int imm19 = this->disp19(l);
        this->op(0b10'011'1'00'00000000000000, (V)0, dst, (imm19 & 19_mask) << 5);
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

    #if !defined(SKVM_JIT_BUT_IGNORE_IT)
        const void* jit_entry = fImpl->jit_entry.load();
        // jit_entry may be null either simply because we can't JIT, or when using LLVM
        // if the work represented by fImpl->llvm_compiling hasn't finished yet.
        //
        // Ordinarily we'd never find ourselves with non-null jit_entry and !gSkVMAllowJIT, but it
        // can happen during interactive programs like Viewer that toggle gSkVMAllowJIT on and off,
        // due to timing or program caching.
        if (jit_entry != nullptr && gSkVMAllowJIT) {
        #if SKVM_JIT_STATS
            jits++;
            fast += n;
        #endif
            void** a = args;
            switch (fImpl->strides.size()) {
                case 0: return ((void(*)(int                        ))jit_entry)(n               );
                case 1: return ((void(*)(int,void*                  ))jit_entry)(n,a[0]          );
                case 2: return ((void(*)(int,void*,void*            ))jit_entry)(n,a[0],a[1]     );
                case 3: return ((void(*)(int,void*,void*,void*      ))jit_entry)(n,a[0],a[1],a[2]);
                case 4: return ((void(*)(int,void*,void*,void*,void*))jit_entry)
                                (n,a[0],a[1],a[2],a[3]);
                case 5: return ((void(*)(int,void*,void*,void*,void*,void*))jit_entry)
                                (n,a[0],a[1],a[2],a[3],a[4]);
                case 6: return ((void(*)(int,void*,void*,void*,void*,void*,void*))jit_entry)
                                (n,a[0],a[1],a[2],a[3],a[4],a[5]);
                case 7: return ((void(*)(int,void*,void*,void*,void*,void*,void*,void*))jit_entry)
                                (n,a[0],a[1],a[2],a[3],a[4],a[5],a[6]);
                default: break; //SkASSERT(fImpl->strides.size() <= 7);
            }
        }
    #endif

        // So we'll sometimes use the interpreter here even if later calls will use the JIT.
        SkOpts::interpret_skvm(fImpl->instructions.data(), (int)fImpl->instructions.size(),
                               this->nregs(), this->loop(), fImpl->strides.data(), this->nargs(),
                               n, args);
    }

    #if defined(SKVM_LLVM)
    // -- SKVM_LLVM --------------------------------------------------------------------------------
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
            auto [op, x,y,z,w, immA,immB,immC, death,can_hoist] = instructions[i];

            llvm::Type *i1    = llvm::Type::getInt1Ty (*ctx),
                       *i8    = llvm::Type::getInt8Ty (*ctx),
                       *i16   = llvm::Type::getInt16Ty(*ctx),
                       *f32   = llvm::Type::getFloatTy(*ctx),
                       *I1    = scalar ? i1    : llvm::VectorType::get(i1 , K, false  ),
                       *I8    = scalar ? i8    : llvm::VectorType::get(i8 , K, false  ),
                       *I16   = scalar ? i16   : llvm::VectorType::get(i16, K, false  ),
                       *I32   = scalar ? i32   : llvm::VectorType::get(i32, K, false  ),
                       *F32   = scalar ? f32   : llvm::VectorType::get(f32, K, false  );

            auto I  = [&](llvm::Value* v) { return b->CreateBitCast(v, I32  ); };
            auto F  = [&](llvm::Value* v) { return b->CreateBitCast(v, F32  ); };

            auto S = [&](llvm::Type* dst, llvm::Value* v) { return b->CreateSExt(v, dst); };

            llvm::Type* vt = nullptr;
            switch (llvm::Type* t = nullptr; op) {
                default:
                    SkDebugf("can't llvm %s (%d)\n", name(op), op);
                    return false;

                case Op::assert_true: /*TODO*/ break;

                case Op::trace_line:
                case Op::trace_var:
                case Op::trace_call:
                    /* Only supported in the interpreter. */
                    break;

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
                    llvm::Value* ptr = b->CreateBitCast(args[immA], t->getPointerTo());
                    vals[i] = b->CreateZExt(
                            b->CreateAlignedLoad(t, ptr, llvm::MaybeAlign{1}), I32);
                } break;


                case Op::splat: vals[i] = llvm::ConstantInt::get(I32, immA); break;

                case Op::uniform32: {
                    llvm::Value* ptr = b->CreateBitCast(
                            b->CreateConstInBoundsGEP1_32(i8, args[immA], immB),
                            i32->getPointerTo());
                    llvm::Value* val = b->CreateZExt(
                            b->CreateAlignedLoad(i32, ptr, llvm::MaybeAlign{1}), i32);
                    vals[i] = I32->isVectorTy() ? b->CreateVectorSplat(K, val)
                                                : val;
                } break;

                case Op::gather8:  t = i8 ; vt = I8; goto gather;
                case Op::gather16: t = i16; vt = I16; goto gather;
                case Op::gather32: t = i32; vt = I32; goto gather;
                gather: {
                    // Our gather base pointer is immB bytes off of uniform immA.
                    llvm::Value* base =
                        b->CreateLoad(b->CreateBitCast(
                                b->CreateConstInBoundsGEP1_32(i8, args[immA],immB),
                                t->getPointerTo()->getPointerTo()));

                    llvm::Value* ptr = b->CreateInBoundsGEP(t, base, vals[x]);
                    llvm::Value* gathered;
                    if (ptr->getType()->isVectorTy()) {
                        gathered = b->CreateMaskedGather(
                                vt,
                                ptr,
                                llvm::Align{1});
                    } else {
                        gathered = b->CreateAlignedLoad(vt, ptr, llvm::MaybeAlign{1});
                    }
                    vals[i] = b->CreateZExt(gathered, I32);
                } break;

                case Op::store8:  t = I8 ; goto store;
                case Op::store16: t = I16; goto store;
                case Op::store32: t = I32; goto store;
                store: {
                    llvm::Value* val = b->CreateTrunc(vals[x], t);
                    llvm::Value* ptr = b->CreateBitCast(args[immA],
                                                        val->getType()->getPointerTo());
                    vals[i] = b->CreateAlignedStore(val, ptr, llvm::MaybeAlign{1});
                } break;

                case Op::bit_and:   vals[i] = b->CreateAnd(vals[x], vals[y]); break;
                case Op::bit_or :   vals[i] = b->CreateOr (vals[x], vals[y]); break;
                case Op::bit_xor:   vals[i] = b->CreateXor(vals[x], vals[y]); break;
                case Op::bit_clear: vals[i] = b->CreateAnd(vals[x], b->CreateNot(vals[y])); break;

                case Op::select:
                    vals[i] = b->CreateSelect(b->CreateTrunc(vals[x], I1), vals[y], vals[z]);
                    break;

                case Op::add_i32: vals[i] = b->CreateAdd(vals[x], vals[y]); break;
                case Op::sub_i32: vals[i] = b->CreateSub(vals[x], vals[y]); break;
                case Op::mul_i32: vals[i] = b->CreateMul(vals[x], vals[y]); break;

                case Op::shl_i32: vals[i] = b->CreateShl (vals[x], immA); break;
                case Op::sra_i32: vals[i] = b->CreateAShr(vals[x], immA); break;
                case Op::shr_i32: vals[i] = b->CreateLShr(vals[x], immA); break;

                case Op:: eq_i32: vals[i] = S(I32, b->CreateICmpEQ (vals[x], vals[y])); break;
                case Op:: gt_i32: vals[i] = S(I32, b->CreateICmpSGT(vals[x], vals[y])); break;

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

                case Op::ceil:
                    vals[i] = I(b->CreateUnaryIntrinsic(llvm::Intrinsic::ceil, F(vals[x])));
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
                        llvm::Value* v = llvm::UndefValue::get(
                                llvm::VectorType::get(f32, 4, false));
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
                    = b.CreateConstInBoundsGEP1_32(
                            llvm::Type::getInt8Ty (*ctx),
                            args[i],
                            K*fImpl->strides[i]);
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

            // Each arg ptr += 1
            for (size_t i = 0; i < fImpl->strides.size(); i++) {
                llvm::Value* arg_next
                    = b.CreateConstInBoundsGEP1_32(
                            llvm::Type::getInt8Ty (*ctx), args[i], fImpl->strides[i]);
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

            #if defined(SKVM_LLVM_WAIT_FOR_COMPILATION)
            // Wait for llvm to compile
            void* function = (void*)ee->getFunctionAddress(debug_name);
            fImpl->jit_entry.store(function);
            // We have to be careful here about what we close over and how, in case fImpl moves.
            // fImpl itself may change, but its pointee fields won't, so close over them by value.
            // Also, debug_name will almost certainly leave scope, so copy it.
            #else
            fImpl->llvm_compiling = std::async(std::launch::async, [dst  = &fImpl->jit_entry,
                                                                    ee   =  fImpl->llvm_ee.get(),
                                                                    name = std::string(debug_name)]{
                // std::atomic<void*>*    dst;
                // llvm::ExecutionEngine* ee;
                // std::string            name;
                dst->store( (void*)ee->getFunctionAddress(name.c_str()) );
            });
            #endif
        }
    }
    #endif  // SKVM_LLVM

    void Program::waitForLLVM() const {
    #if defined(SKVM_LLVM) && !defined(SKVM_LLVM_WAIT_FOR_COMPILATION)
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
            close_dylib(fImpl->dylib);
        } else if (auto jit_entry = fImpl->jit_entry.load()) {
            unmap_jit_buffer(jit_entry, fImpl->jit_size);
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

    Program::Program(const std::vector<OptimizedInstruction>& instructions,
                     const std::vector<int>& strides,
                     const char* debug_name, bool allow_jit) : Program() {
        fImpl->strides = strides;
        if (gSkVMAllowJIT && allow_jit) {
        #if 1 && defined(SKVM_LLVM)
            this->setupLLVM(instructions, debug_name);
        #elif 1 && defined(SKVM_JIT)
            this->setupJIT(instructions, debug_name);
        #endif
        }

        // Might as well do this after setupLLVM() to get a little more time to compile.
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

#if defined(SKVM_JIT)

    namespace SkVMJitTypes {
    #if defined(__x86_64__) || defined(_M_X64)
        using Reg = Assembler::Ymm;
    #elif defined(__aarch64__)
        using Reg = Assembler::V;
    #endif
    }  // namespace SkVMJitTypes

    bool Program::jit(const std::vector<OptimizedInstruction>& instructions,
                      int* stack_hint,
                      uint32_t* registers_used,
                      Assembler* a) const {
        using A = Assembler;
        using SkVMJitTypes::Reg;

        SkTHashMap<int, A::Label> constants;    // Constants (mostly splats) share the same pool.
        A::Label                  iota;         // Varies per lane, for Op::index.
        A::Label                  load64_index; // Used to load low or high half of 64-bit lanes.

        // The `regs` array tracks everything we know about each register's state:
        //   - NA:   empty
        //   - RES:  reserved by ABI
        //   - TMP:  holding a temporary
        //   - id:   holding Val id
        constexpr Val RES = NA-1,
                      TMP = RES-1;

        // Map val -> stack slot.
        std::vector<int> stack_slot(instructions.size(), NA);
        int next_stack_slot = 0;

        const int nstack_slots = *stack_hint >= 0 ? *stack_hint
                                                  : stack_slot.size();
    #if defined(__x86_64__) || defined(_M_X64)
        if (!SkCpu::Supports(SkCpu::HSW)) {
            return false;
        }
        const int K = 8;
        #if defined(_M_X64)  // Important to check this first; clang-cl defines both.
            const A::GP64 N = A::rcx,
                        GP0 = A::rax,
                        GP1 = A::r11,
                        arg[]    = { A::rdx, A::r8, A::r9, A::r10, A::rdi, A::rsi };

            // xmm6-15 need are callee-saved.
            std::array<Val,16> regs = {
                 NA, NA, NA, NA,  NA, NA,RES,RES,
                RES,RES,RES,RES, RES,RES,RES,RES,
            };
            const uint32_t incoming_registers_used = *registers_used;

            auto enter = [&]{
                // rcx,rdx,r8,r9 are all already holding their correct values.
                // Load caller-saved r10 from rsp+40 if there's a fourth arg.
                if (fImpl->strides.size() >= 4) {
                    a->mov(A::r10, A::Mem{A::rsp, 40});
                }
                // Load callee-saved rdi from rsp+48 if there's a fifth arg,
                // first saving it to ABI reserved shadow area rsp+8.
                if (fImpl->strides.size() >= 5) {
                    a->mov(A::Mem{A::rsp, 8}, A::rdi);
                    a->mov(A::rdi, A::Mem{A::rsp, 48});
                }
                // Load callee-saved rsi from rsp+56 if there's a sixth arg,
                // first saving it to ABI reserved shadow area rsp+16.
                if (fImpl->strides.size() >= 6) {
                    a->mov(A::Mem{A::rsp, 16}, A::rsi);
                    a->mov(A::rsi, A::Mem{A::rsp, 56});
                }

                // Allocate stack for our values and callee-saved xmm6-15.
                int stack_needed = nstack_slots*K*4;
                for (int r = 6; r < 16; r++) {
                    if (incoming_registers_used & (1<<r)) {
                        stack_needed += 16;
                    }
                }
                if (stack_needed) { a->sub(A::rsp, stack_needed); }

                int next_saved_xmm = nstack_slots*K*4;
                for (int r = 6; r < 16; r++) {
                    if (incoming_registers_used & (1<<r)) {
                        a->vmovups(A::Mem{A::rsp, next_saved_xmm}, (A::Xmm)r);
                        next_saved_xmm += 16;
                        regs[r] = NA;
                    }
                }
            };
            auto exit  = [&]{
                // The second pass of jit() shouldn't use any register it didn't in the first pass.
                SkASSERT((*registers_used & incoming_registers_used) == *registers_used);

                // Restore callee-saved xmm6-15 and the stack pointer.
                int stack_used = nstack_slots*K*4;
                for (int r = 6; r < 16; r++) {
                    if (incoming_registers_used & (1<<r)) {
                        a->vmovups((A::Xmm)r, A::Mem{A::rsp, stack_used});
                        stack_used += 16;
                    }
                }
                if (stack_used) { a->add(A::rsp, stack_used); }

                // Restore callee-saved rdi/rsi if we used them.
                if (fImpl->strides.size() >= 5) {
                    a->mov(A::rdi, A::Mem{A::rsp, 8});
                }
                if (fImpl->strides.size() >= 6) {
                    a->mov(A::rsi, A::Mem{A::rsp, 16});
                }

                a->vzeroupper();
                a->ret();
            };
        #elif defined(__x86_64__)
            const A::GP64 N = A::rdi,
                        GP0 = A::rax,
                        GP1 = A::r11,
                        arg[]    = { A::rsi, A::rdx, A::rcx, A::r8, A::r9, A::r10 };

            // All 16 ymm registers are available to use.
            std::array<Val,16> regs = {
                NA,NA,NA,NA, NA,NA,NA,NA,
                NA,NA,NA,NA, NA,NA,NA,NA,
            };

            auto enter = [&]{
                // Load caller-saved r10 from rsp+8 if there's a sixth arg.
                if (fImpl->strides.size() >= 6) {
                    a->mov(A::r10, A::Mem{A::rsp, 8});
                }
                if (nstack_slots) { a->sub(A::rsp, nstack_slots*K*4); }
            };
            auto exit  = [&]{
                if (nstack_slots) { a->add(A::rsp, nstack_slots*K*4); }
                a->vzeroupper();
                a->ret();
            };
        #endif

        auto load_from_memory = [&](Reg r, Val v) {
            if (instructions[v].op == Op::splat) {
                if (instructions[v].immA == 0) {
                    a->vpxor(r,r,r);
                } else {
                    a->vmovups(r, constants.find(instructions[v].immA));
                }
            } else {
                SkASSERT(stack_slot[v] != NA);
                a->vmovups(r, A::Mem{A::rsp, stack_slot[v]*K*4});
            }
        };
        auto store_to_stack = [&](Reg r, Val v) {
            SkASSERT(next_stack_slot < nstack_slots);
            stack_slot[v] = next_stack_slot++;
            a->vmovups(A::Mem{A::rsp, stack_slot[v]*K*4}, r);
        };
    #elif defined(__aarch64__)
        const int K = 4;
        const A::X N     = A::x0,
                   GP0   = A::x8,
                   GP1   = A::x9,
                   arg[] = { A::x1, A::x2, A::x3, A::x4, A::x5, A::x6, A::x7 };

        // We can use v0-v7 and v16-v31 freely; we'd need to preserve v8-v15 in enter/exit.
        std::array<Val,32> regs = {
             NA, NA, NA, NA,  NA, NA, NA, NA,
            RES,RES,RES,RES, RES,RES,RES,RES,
             NA, NA, NA, NA,  NA, NA, NA, NA,
             NA, NA, NA, NA,  NA, NA, NA, NA,
        };

        auto enter = [&]{ if (nstack_slots) { a->sub(A::sp, A::sp, nstack_slots*K*4); } };
        auto exit  = [&]{ if (nstack_slots) { a->add(A::sp, A::sp, nstack_slots*K*4); }
                          a->ret(A::x30); };

        auto load_from_memory = [&](Reg r, Val v) {
            if (instructions[v].op == Op::splat) {
                if (instructions[v].immA == 0) {
                    a->eor16b(r,r,r);
                } else {
                    a->ldrq(r, constants.find(instructions[v].immA));
                }
            } else {
                SkASSERT(stack_slot[v] != NA);
                a->ldrq(r, A::sp, stack_slot[v]);
            }
        };
        auto store_to_stack  = [&](Reg r, Val v) {
            SkASSERT(next_stack_slot < nstack_slots);
            stack_slot[v] = next_stack_slot++;
            a->strq(r, A::sp, stack_slot[v]);
        };
    #endif

        *registers_used = 0;  // We'll update this as we go.

        if (SK_ARRAY_COUNT(arg) < fImpl->strides.size()) {
            return false;
        }

        auto emit = [&](Val id, bool scalar) {
            const int active_lanes = scalar ? 1 : K;
            const OptimizedInstruction& inst = instructions[id];
            const Op op = inst.op;
            const Val x = inst.x,
                      y = inst.y,
                      z = inst.z,
                      w = inst.w;
            const int immA = inst.immA,
                      immB = inst.immB,
                      immC = inst.immC;

            // alloc_tmp() returns the first of N adjacent temporary registers,
            // each freed manually with free_tmp() or noted as our result with mark_tmp_as_dst().
            auto alloc_tmp = [&](int N=1) -> Reg {
                auto needs_spill = [&](Val v) -> bool {
                    SkASSERT(v >= 0);   // {NA,TMP,RES} need to be handled before calling this.
                    return stack_slot[v] == NA               // We haven't spilled it already?
                        && instructions[v].op != Op::splat;  // No need to spill constants.
                };

                // We want to find a block of N adjacent registers requiring the fewest spills.
                int best_block = -1,
                    min_spills = 0x7fff'ffff;
                for (int block = 0; block+N <= (int)regs.size(); block++) {
                    int spills = 0;
                    for (int r = block; r < block+N; r++) {
                        Val v = regs[r];
                        // Registers holding NA (nothing) are ideal, nothing to spill.
                        if (v == NA) {
                            continue;
                        }
                        // We can't spill anything REServed or that we'll need this instruction.
                        if (v == RES ||
                            v == TMP || v == id || v == x || v == y || v == z || v == w) {
                            spills = 0x7fff'ffff;
                            block  = r;   // (optimization) continue outer loop at next register.
                            break;
                        }
                        // Usually here we've got a value v that we'd have to spill to the stack
                        // before reusing its register, but sometimes even now we get a freebie.
                        spills += needs_spill(v) ? 1 : 0;
                    }

                    // TODO: non-arbitrary tie-breaking?
                    if (min_spills > spills) {
                        min_spills = spills;
                        best_block = block;
                    }
                    if (min_spills == 0) {
                        break;  // (optimization) stop early if we find an unbeatable block.
                    }
                }

                // TODO: our search's success isn't obviously guaranteed... it depends on N
                // and the number and relative position in regs of any unspillable values.
                // I think we should be able to get away with N≤2 on x86-64 and N≤4 on arm64;
                // we'll need to revisit this logic should this assert fire.
                SkASSERT(min_spills <= N);

                // Spill what needs spilling, and mark the block all as TMP.
                for (int r = best_block; r < best_block+N; r++) {
                    Val& v = regs[r];
                    *registers_used |= (1<<r);

                    SkASSERT(v == NA || v >= 0);
                    if (v >= 0 && needs_spill(v)) {
                        store_to_stack((Reg)r, v);
                        SkASSERT(!needs_spill(v));
                        min_spills--;
                    }

                    v = TMP;
                }
                SkASSERT(min_spills == 0);
                return (Reg)best_block;
            };

            auto free_tmp = [&](Reg r) {
                SkASSERT(regs[r] == TMP);
                regs[r] = NA;
            };

            // Which register holds dst,x,y,z,w for this instruction?  NA if none does yet.
            int rd = NA,
                rx = NA,
                ry = NA,
                rz = NA,
                rw = NA;

            auto update_regs = [&](Reg r, Val v) {
                if (v == id) { rd = r; }
                if (v ==  x) { rx = r; }
                if (v ==  y) { ry = r; }
                if (v ==  z) { rz = r; }
                if (v ==  w) { rw = r; }
                return r;
            };

            auto find_existing_reg = [&](Val v) -> int {
                // Quick-check our working registers.
                if (v == id && rd != NA) { return rd; }
                if (v ==  x && rx != NA) { return rx; }
                if (v ==  y && ry != NA) { return ry; }
                if (v ==  z && rz != NA) { return rz; }
                if (v ==  w && rw != NA) { return rw; }

                // Search inter-instruction register map.
                for (auto [r,val] : SkMakeEnumerate(regs)) {
                    if (val == v) {
                        return update_regs((Reg)r, v);
                    }
                }
                return NA;
            };

            // Return a register for Val, holding that value if it already exists.
            // During this instruction all calls to r(v) will return the same register.
            auto r = [&](Val v) -> Reg {
                SkASSERT(v >= 0);

                if (int found = find_existing_reg(v); found != NA) {
                    return (Reg)found;
                }

                Reg r = alloc_tmp();
                SkASSERT(regs[r] == TMP);

                SkASSERT(v <= id);
                if (v < id) {
                    // If v < id, we're loading one of this instruction's inputs.
                    // If v == id we're just allocating its destination register.
                    load_from_memory(r, v);
                }
                regs[r] = v;
                return update_regs(r, v);
            };

            auto dies_here = [&](Val v) -> bool {
                SkASSERT(v >= 0);
                return instructions[v].death == id;
            };

            // Alias dst() to r(v) if dies_here(v).
            auto try_alias = [&](Val v) -> bool {
                SkASSERT(v == x || v == y || v == z || v == w);
                if (dies_here(v)) {
                    rd = r(v);      // Vals v and id share a register for this instruction.
                    regs[rd] = id;  // Next instruction, Val id will be in the register, not Val v.
                    return true;
                }
                return false;
            };

            // Generally r(id),
            // but with a hint, try to alias dst() to r(v) if dies_here(v).
            auto dst = [&](Val hint1 = NA, Val hint2 = NA) -> Reg {
                if (hint1 != NA && try_alias(hint1)) { return r(id); }
                if (hint2 != NA && try_alias(hint2)) { return r(id); }
                return r(id);
            };

        #if defined(__aarch64__)  // Nothing sneaky, just unused on x86-64.
            auto mark_tmp_as_dst = [&](Reg tmp) {
                SkASSERT(regs[tmp] == TMP);
                rd = tmp;
                regs[rd] = id;
                SkASSERT(dst() == tmp);
            };
        #endif

        #if defined(__x86_64__) || defined(_M_X64)
            // On x86 we can work with many values directly from the stack or program constant pool.
            auto any = [&](Val v) -> A::Operand {
                SkASSERT(v >= 0);
                SkASSERT(v < id);

                if (int found = find_existing_reg(v); found != NA) {
                    return (Reg)found;
                }
                if (instructions[v].op == Op::splat) {
                    return constants.find(instructions[v].immA);
                }
                return A::Mem{A::rsp, stack_slot[v]*K*4};
            };

            // This is never really worth asking except when any() might be used;
            // if we need this value in ARM, might as well just call r(v) to get it into a register.
            auto in_reg = [&](Val v) -> bool {
                return find_existing_reg(v) != NA;
            };
        #endif

            switch (op) {
                // Make sure splat constants can be found by load_from_memory() or any().
                case Op::splat:
                    (void)constants[immA];
                    break;

            #if defined(__x86_64__) || defined(_M_X64)
                case Op::assert_true: {
                    a->vptest (r(x), &constants[0xffffffff]);
                    A::Label all_true;
                    a->jc(&all_true);
                    a->int3();
                    a->label(&all_true);
                } break;

                case Op::trace_line:
                case Op::trace_var:
                case Op::trace_call:
                    /* Only supported in the interpreter. */
                    break;

                case Op::store8:
                    if (scalar) {
                        a->vpextrb(A::Mem{arg[immA]}, (A::Xmm)r(x), 0);
                    } else {
                        a->vpackusdw(dst(x), r(x), r(x));
                        a->vpermq   (dst(), dst(), 0xd8);
                        a->vpackuswb(dst(), dst(), dst());
                        a->vmovq    (A::Mem{arg[immA]}, (A::Xmm)dst());
                    } break;

                case Op::store16:
                    if (scalar) {
                        a->vpextrw(A::Mem{arg[immA]}, (A::Xmm)r(x), 0);
                    } else {
                        a->vpackusdw(dst(x), r(x), r(x));
                        a->vpermq   (dst(), dst(), 0xd8);
                        a->vmovups  (A::Mem{arg[immA]}, (A::Xmm)dst());
                    } break;

                case Op::store32: if (scalar) { a->vmovd  (A::Mem{arg[immA]}, (A::Xmm)r(x)); }
                                  else        { a->vmovups(A::Mem{arg[immA]},         r(x)); }
                                  break;

                case Op::store64: if (scalar) {
                                      a->vmovd(A::Mem{arg[immA],0}, (A::Xmm)r(x));
                                      a->vmovd(A::Mem{arg[immA],4}, (A::Xmm)r(y));
                                  } else {
                                      // r(x) = {a,b,c,d|e,f,g,h}
                                      // r(y) = {i,j,k,l|m,n,o,p}
                                      // We want to write a,i,b,j,c,k,d,l,e,m...
                                      A::Ymm L = alloc_tmp(),
                                             H = alloc_tmp();
                                      a->vpunpckldq(L, r(x), any(y));  // L = {a,i,b,j|e,m,f,n}
                                      a->vpunpckhdq(H, r(x), any(y));  // H = {c,k,d,l|g,o,h,p}
                                      a->vperm2f128(dst(), L,H, 0x20); //   = {a,i,b,j|c,k,d,l}
                                      a->vmovups(A::Mem{arg[immA], 0}, dst());
                                      a->vperm2f128(dst(), L,H, 0x31); //   = {e,m,f,n|g,o,h,p}
                                      a->vmovups(A::Mem{arg[immA],32}, dst());
                                      free_tmp(L);
                                      free_tmp(H);
                                  } break;

                case Op::store128: {
                    // TODO: >32-bit stores
                    a->vmovd  (A::Mem{arg[immA], 0*16 +  0}, (A::Xmm)r(x)   );
                    a->vmovd  (A::Mem{arg[immA], 0*16 +  4}, (A::Xmm)r(y)   );
                    a->vmovd  (A::Mem{arg[immA], 0*16 +  8}, (A::Xmm)r(z)   );
                    a->vmovd  (A::Mem{arg[immA], 0*16 + 12}, (A::Xmm)r(w)   );
                    if (scalar) { break; }

                    a->vpextrd(A::Mem{arg[immA], 1*16 +  0}, (A::Xmm)r(x), 1);
                    a->vpextrd(A::Mem{arg[immA], 1*16 +  4}, (A::Xmm)r(y), 1);
                    a->vpextrd(A::Mem{arg[immA], 1*16 +  8}, (A::Xmm)r(z), 1);
                    a->vpextrd(A::Mem{arg[immA], 1*16 + 12}, (A::Xmm)r(w), 1);

                    a->vpextrd(A::Mem{arg[immA], 2*16 +  0}, (A::Xmm)r(x), 2);
                    a->vpextrd(A::Mem{arg[immA], 2*16 +  4}, (A::Xmm)r(y), 2);
                    a->vpextrd(A::Mem{arg[immA], 2*16 +  8}, (A::Xmm)r(z), 2);
                    a->vpextrd(A::Mem{arg[immA], 2*16 + 12}, (A::Xmm)r(w), 2);

                    a->vpextrd(A::Mem{arg[immA], 3*16 +  0}, (A::Xmm)r(x), 3);
                    a->vpextrd(A::Mem{arg[immA], 3*16 +  4}, (A::Xmm)r(y), 3);
                    a->vpextrd(A::Mem{arg[immA], 3*16 +  8}, (A::Xmm)r(z), 3);
                    a->vpextrd(A::Mem{arg[immA], 3*16 + 12}, (A::Xmm)r(w), 3);
                    // Now we need to store the upper 128 bits of x,y,z,w.
                    // Storing in this order rather than interlacing minimizes temporaries.
                    a->vextracti128(dst(), r(x), 1);
                    a->vmovd  (A::Mem{arg[immA], 4*16 +  0}, (A::Xmm)dst()   );
                    a->vpextrd(A::Mem{arg[immA], 5*16 +  0}, (A::Xmm)dst(), 1);
                    a->vpextrd(A::Mem{arg[immA], 6*16 +  0}, (A::Xmm)dst(), 2);
                    a->vpextrd(A::Mem{arg[immA], 7*16 +  0}, (A::Xmm)dst(), 3);

                    a->vextracti128(dst(), r(y), 1);
                    a->vmovd  (A::Mem{arg[immA], 4*16 +  4}, (A::Xmm)dst()   );
                    a->vpextrd(A::Mem{arg[immA], 5*16 +  4}, (A::Xmm)dst(), 1);
                    a->vpextrd(A::Mem{arg[immA], 6*16 +  4}, (A::Xmm)dst(), 2);
                    a->vpextrd(A::Mem{arg[immA], 7*16 +  4}, (A::Xmm)dst(), 3);

                    a->vextracti128(dst(), r(z), 1);
                    a->vmovd  (A::Mem{arg[immA], 4*16 +  8}, (A::Xmm)dst()   );
                    a->vpextrd(A::Mem{arg[immA], 5*16 +  8}, (A::Xmm)dst(), 1);
                    a->vpextrd(A::Mem{arg[immA], 6*16 +  8}, (A::Xmm)dst(), 2);
                    a->vpextrd(A::Mem{arg[immA], 7*16 +  8}, (A::Xmm)dst(), 3);

                    a->vextracti128(dst(), r(w), 1);
                    a->vmovd  (A::Mem{arg[immA], 4*16 + 12}, (A::Xmm)dst()   );
                    a->vpextrd(A::Mem{arg[immA], 5*16 + 12}, (A::Xmm)dst(), 1);
                    a->vpextrd(A::Mem{arg[immA], 6*16 + 12}, (A::Xmm)dst(), 2);
                    a->vpextrd(A::Mem{arg[immA], 7*16 + 12}, (A::Xmm)dst(), 3);
                } break;

                case Op::load8:  if (scalar) {
                                     a->vpxor  (dst(), dst(), dst());
                                     a->vpinsrb((A::Xmm)dst(), (A::Xmm)dst(), A::Mem{arg[immA]}, 0);
                                 } else {
                                     a->vpmovzxbd(dst(), A::Mem{arg[immA]});
                                 } break;

                case Op::load16: if (scalar) {
                                     a->vpxor  (dst(), dst(), dst());
                                     a->vpinsrw((A::Xmm)dst(), (A::Xmm)dst(), A::Mem{arg[immA]}, 0);
                                 } else {
                                     a->vpmovzxwd(dst(), A::Mem{arg[immA]});
                                 } break;

                case Op::load32: if (scalar) { a->vmovd  ((A::Xmm)dst(), A::Mem{arg[immA]}); }
                                 else        { a->vmovups(        dst(), A::Mem{arg[immA]}); }
                                 break;

                case Op::load64: if (scalar) {
                                    a->vmovd((A::Xmm)dst(), A::Mem{arg[immA], 4*immB});
                                 } else {
                                    A::Ymm tmp = alloc_tmp();
                                    a->vmovups(tmp, &load64_index);
                                    a->vpermps(dst(), tmp, A::Mem{arg[immA],  0});
                                    a->vpermps(  tmp, tmp, A::Mem{arg[immA], 32});
                                    // Low 128 bits holds immB=0 lanes, high 128 bits holds immB=1.
                                    a->vperm2f128(dst(), dst(),tmp, immB ? 0x31 : 0x20);
                                    free_tmp(tmp);
                                 } break;

                case Op::load128: if (scalar) {
                                      a->vmovd((A::Xmm)dst(), A::Mem{arg[immA], 4*immB});
                                  } else {
                                      // Load 4 low values into xmm tmp,
                                      A::Ymm tmp = alloc_tmp();
                                      A::Xmm t = (A::Xmm)tmp;
                                      a->vmovd  (t,   A::Mem{arg[immA], 0*16 + 4*immB}   );
                                      a->vpinsrd(t,t, A::Mem{arg[immA], 1*16 + 4*immB}, 1);
                                      a->vpinsrd(t,t, A::Mem{arg[immA], 2*16 + 4*immB}, 2);
                                      a->vpinsrd(t,t, A::Mem{arg[immA], 3*16 + 4*immB}, 3);

                                      // Load 4 high values into xmm dst(),
                                      A::Xmm d = (A::Xmm)dst();
                                      a->vmovd  (d,   A::Mem{arg[immA], 4*16 + 4*immB}   );
                                      a->vpinsrd(d,d, A::Mem{arg[immA], 5*16 + 4*immB}, 1);
                                      a->vpinsrd(d,d, A::Mem{arg[immA], 6*16 + 4*immB}, 2);
                                      a->vpinsrd(d,d, A::Mem{arg[immA], 7*16 + 4*immB}, 3);

                                      // Merge the two, ymm dst() = {xmm tmp|xmm dst()}
                                      a->vperm2f128(dst(), tmp,dst(), 0x20);
                                      free_tmp(tmp);
                                  } break;

                case Op::gather8: {
                    // As usual, the gather base pointer is immB bytes off of uniform immA.
                    a->mov(GP0, A::Mem{arg[immA], immB});

                    A::Ymm tmp = alloc_tmp();
                    a->vmovups(tmp, any(x));

                    for (int i = 0; i < active_lanes; i++) {
                        if (i == 4) {
                            // vpextrd can only pluck indices out from an Xmm register,
                            // so we manually swap over to the top when we're halfway through.
                            a->vextracti128((A::Xmm)tmp, tmp, 1);
                        }
                        a->vpextrd(GP1, (A::Xmm)tmp, i%4);
                        a->vpinsrb((A::Xmm)dst(), (A::Xmm)dst(), A::Mem{GP0,0,GP1,A::ONE}, i);
                    }
                    a->vpmovzxbd(dst(), dst());
                    free_tmp(tmp);
                } break;

                case Op::gather16: {
                    // Just as gather8 except vpinsrb->vpinsrw, ONE->TWO, and vpmovzxbd->vpmovzxwd.
                    a->mov(GP0, A::Mem{arg[immA], immB});

                    A::Ymm tmp = alloc_tmp();
                    a->vmovups(tmp, any(x));

                    for (int i = 0; i < active_lanes; i++) {
                        if (i == 4) {
                            a->vextracti128((A::Xmm)tmp, tmp, 1);
                        }
                        a->vpextrd(GP1, (A::Xmm)tmp, i%4);
                        a->vpinsrw((A::Xmm)dst(), (A::Xmm)dst(), A::Mem{GP0,0,GP1,A::TWO}, i);
                    }
                    a->vpmovzxwd(dst(), dst());
                    free_tmp(tmp);
                } break;

                case Op::gather32:
                if (scalar) {
                    // Our gather base pointer is immB bytes off of uniform immA.
                    a->mov(GP0, A::Mem{arg[immA], immB});

                    // Grab our index from lane 0 of the index argument.
                    a->vmovd(GP1, (A::Xmm)r(x));

                    // dst = *(base + 4*index)
                    a->vmovd((A::Xmm)dst(x), A::Mem{GP0, 0, GP1, A::FOUR});
                } else {
                    a->mov(GP0, A::Mem{arg[immA], immB});

                    A::Ymm mask = alloc_tmp();
                    a->vpcmpeqd(mask, mask, mask);   // (All lanes enabled.)

                    a->vgatherdps(dst(), A::FOUR, r(x), GP0, mask);
                    free_tmp(mask);
                }
                break;

                case Op::uniform32: a->vbroadcastss(dst(), A::Mem{arg[immA], immB});
                                    break;

                case Op::array32: a->mov(GP0, A::Mem{arg[immA], immB});
                                  a->vbroadcastss(dst(), A::Mem{GP0, immC});
                                  break;

                case Op::index: a->vmovd((A::Xmm)dst(), N);
                                a->vbroadcastss(dst(), dst());
                                a->vpsubd(dst(), dst(), &iota);
                                break;

                // We can swap the arguments of symmetric instructions to make better use of any().
                case Op::add_f32:
                    if (in_reg(x)) { a->vaddps(dst(x), r(x), any(y)); }
                    else           { a->vaddps(dst(y), r(y), any(x)); }
                                     break;

                case Op::mul_f32:
                    if (in_reg(x)) { a->vmulps(dst(x), r(x), any(y)); }
                    else           { a->vmulps(dst(y), r(y), any(x)); }
                                     break;

                case Op::sub_f32: a->vsubps(dst(x), r(x), any(y)); break;
                case Op::div_f32: a->vdivps(dst(x), r(x), any(y)); break;
                case Op::min_f32: a->vminps(dst(y), r(y), any(x)); break;  // Order matters,
                case Op::max_f32: a->vmaxps(dst(y), r(y), any(x)); break;  // see test SkVM_min_max.

                case Op::fma_f32:
                    if (try_alias(x)) { a->vfmadd132ps(dst(x), r(z), any(y)); } else
                    if (try_alias(y)) { a->vfmadd213ps(dst(y), r(x), any(z)); } else
                    if (try_alias(z)) { a->vfmadd231ps(dst(z), r(x), any(y)); } else
                                      { a->vmovups    (dst(), any(x));
                                        a->vfmadd132ps(dst(), r(z), any(y)); }
                                        break;

                case Op::fms_f32:
                    if (try_alias(x)) { a->vfmsub132ps(dst(x), r(z), any(y)); } else
                    if (try_alias(y)) { a->vfmsub213ps(dst(y), r(x), any(z)); } else
                    if (try_alias(z)) { a->vfmsub231ps(dst(z), r(x), any(y)); } else
                                      { a->vmovups    (dst(), any(x));
                                        a->vfmsub132ps(dst(), r(z), any(y)); }
                                        break;

                case Op::fnma_f32:
                    if (try_alias(x)) { a->vfnmadd132ps(dst(x), r(z), any(y)); } else
                    if (try_alias(y)) { a->vfnmadd213ps(dst(y), r(x), any(z)); } else
                    if (try_alias(z)) { a->vfnmadd231ps(dst(z), r(x), any(y)); } else
                                      { a->vmovups     (dst(), any(x));
                                        a->vfnmadd132ps(dst(), r(z), any(y)); }
                                        break;

                // In situations like this we want to try aliasing dst(x) when x is
                // already in a register, but not if we'd have to load it from the stack
                // just to alias it.  That's done better directly into the new register.
                case Op::sqrt_f32:
                    if (in_reg(x)) { a->vsqrtps(dst(x),  r(x)); }
                    else           { a->vsqrtps(dst(), any(x)); }
                                     break;

                case Op::add_i32:
                    if (in_reg(x)) { a->vpaddd(dst(x), r(x), any(y)); }
                    else           { a->vpaddd(dst(y), r(y), any(x)); }
                                     break;

                case Op::mul_i32:
                    if (in_reg(x)) { a->vpmulld(dst(x), r(x), any(y)); }
                    else           { a->vpmulld(dst(y), r(y), any(x)); }
                                     break;

                case Op::sub_i32: a->vpsubd(dst(x), r(x), any(y)); break;

                case Op::bit_and:
                    if (in_reg(x)) { a->vpand(dst(x), r(x), any(y)); }
                    else           { a->vpand(dst(y), r(y), any(x)); }
                                     break;
                case Op::bit_or:
                    if (in_reg(x)) { a->vpor(dst(x), r(x), any(y)); }
                    else           { a->vpor(dst(y), r(y), any(x)); }
                                     break;
                case Op::bit_xor:
                    if (in_reg(x)) { a->vpxor(dst(x), r(x), any(y)); }
                    else           { a->vpxor(dst(y), r(y), any(x)); }
                                     break;

                case Op::bit_clear: a->vpandn(dst(y), r(y), any(x)); break; // Notice, y then x.

                case Op::select:
                    if (try_alias(z)) { a->vpblendvb(dst(z), r(z), any(y), r(x)); }
                    else              { a->vpblendvb(dst(x), r(z), any(y), r(x)); }
                                        break;

                case Op::shl_i32: a->vpslld(dst(x), r(x), immA); break;
                case Op::shr_i32: a->vpsrld(dst(x), r(x), immA); break;
                case Op::sra_i32: a->vpsrad(dst(x), r(x), immA); break;

                case Op::eq_i32:
                    if (in_reg(x)) { a->vpcmpeqd(dst(x), r(x), any(y)); }
                    else           { a->vpcmpeqd(dst(y), r(y), any(x)); }
                                     break;

                case Op::gt_i32: a->vpcmpgtd(dst(), r(x), any(y)); break;

                case Op::eq_f32:
                    if (in_reg(x)) { a->vcmpeqps(dst(x), r(x), any(y)); }
                    else           { a->vcmpeqps(dst(y), r(y), any(x)); }
                                     break;
                case Op::neq_f32:
                    if (in_reg(x)) { a->vcmpneqps(dst(x), r(x), any(y)); }
                    else           { a->vcmpneqps(dst(y), r(y), any(x)); }
                                     break;

                case Op:: gt_f32: a->vcmpltps (dst(y), r(y), any(x)); break;
                case Op::gte_f32: a->vcmpleps (dst(y), r(y), any(x)); break;

                case Op::ceil:
                    if (in_reg(x)) { a->vroundps(dst(x),  r(x), Assembler::CEIL); }
                    else           { a->vroundps(dst(), any(x), Assembler::CEIL); }
                                     break;

                case Op::floor:
                    if (in_reg(x)) { a->vroundps(dst(x),  r(x), Assembler::FLOOR); }
                    else           { a->vroundps(dst(), any(x), Assembler::FLOOR); }
                                     break;

                case Op::to_f32:
                    if (in_reg(x)) { a->vcvtdq2ps(dst(x),  r(x)); }
                    else           { a->vcvtdq2ps(dst(), any(x)); }
                                     break;

                case Op::trunc:
                    if (in_reg(x)) { a->vcvttps2dq(dst(x),  r(x)); }
                    else           { a->vcvttps2dq(dst(), any(x)); }
                                     break;

                case Op::round:
                    if (in_reg(x)) { a->vcvtps2dq(dst(x),  r(x)); }
                    else           { a->vcvtps2dq(dst(), any(x)); }
                                     break;

                case Op::to_fp16:
                    a->vcvtps2ph(dst(x), r(x), A::CURRENT);  // f32 ymm -> f16 xmm
                    a->vpmovzxwd(dst(), dst());              // f16 xmm -> f16 ymm
                    break;

                case Op::from_fp16:
                    a->vpackusdw(dst(x), r(x), r(x));  // f16 ymm -> f16 xmm
                    a->vpermq   (dst(), dst(), 0xd8);  // swap middle two 64-bit lanes
                    a->vcvtph2ps(dst(), dst());        // f16 xmm -> f32 ymm
                    break;

            #elif defined(__aarch64__)
                case Op::assert_true: {
                    a->uminv4s(dst(), r(x));   // uminv acts like an all() across the vector.
                    a->movs(GP0, dst(), 0);
                    A::Label all_true;
                    a->cbnz(GP0, &all_true);
                    a->brk(0);
                    a->label(&all_true);
                } break;

                case Op::trace_line:
                case Op::trace_var:
                case Op::trace_call:
                    /* Only supported in the interpreter. */
                    break;

                case Op::index: {
                    A::V tmp = alloc_tmp();
                    a->ldrq (tmp, &iota);
                    a->dup4s(dst(), N);
                    a->sub4s(dst(), dst(), tmp);
                    free_tmp(tmp);
                } break;

                case Op::store8: a->xtns2h(dst(x), r(x));
                                 a->xtnh2b(dst(), dst());
                   if (scalar) { a->strb  (dst(), arg[immA]); }
                   else        { a->strs  (dst(), arg[immA]); }
                                 break;

                case Op::store16: a->xtns2h(dst(x), r(x));
                    if (scalar) { a->strh  (dst(), arg[immA]); }
                    else        { a->strd  (dst(), arg[immA]); }
                                  break;

                case Op::store32: if (scalar) { a->strs(r(x), arg[immA]); }
                                  else        { a->strq(r(x), arg[immA]); }
                                                break;

                case Op::store64: if (scalar) {
                                      a->strs(r(x), arg[immA], 0);
                                      a->strs(r(y), arg[immA], 1);
                                  } else if (r(y) == r(x)+1) {
                                      a->st24s(r(x), arg[immA]);
                                  } else {
                                      Reg tmp0 = alloc_tmp(2),
                                          tmp1 = (Reg)(tmp0+1);
                                      a->orr16b(tmp0, r(x), r(x));
                                      a->orr16b(tmp1, r(y), r(y));
                                      a-> st24s(tmp0, arg[immA]);
                                      free_tmp(tmp0);
                                      free_tmp(tmp1);
                                  } break;

                case Op::store128:
                    if (scalar) {
                        a->strs(r(x), arg[immA], 0);
                        a->strs(r(y), arg[immA], 1);
                        a->strs(r(z), arg[immA], 2);
                        a->strs(r(w), arg[immA], 3);
                    } else if (r(y) == r(x)+1 &&
                               r(z) == r(x)+2 &&
                               r(w) == r(x)+3) {
                        a->st44s(r(x), arg[immA]);
                    } else {
                        Reg tmp0 = alloc_tmp(4),
                            tmp1 = (Reg)(tmp0+1),
                            tmp2 = (Reg)(tmp0+2),
                            tmp3 = (Reg)(tmp0+3);
                        a->orr16b(tmp0, r(x), r(x));
                        a->orr16b(tmp1, r(y), r(y));
                        a->orr16b(tmp2, r(z), r(z));
                        a->orr16b(tmp3, r(w), r(w));
                        a-> st44s(tmp0, arg[immA]);
                        free_tmp(tmp0);
                        free_tmp(tmp1);
                        free_tmp(tmp2);
                        free_tmp(tmp3);
                    } break;


                case Op::load8: if (scalar) { a->ldrb(dst(), arg[immA]); }
                                else        { a->ldrs(dst(), arg[immA]); }
                                              a->uxtlb2h(dst(), dst());
                                              a->uxtlh2s(dst(), dst());
                                              break;

                case Op::load16: if (scalar) { a->ldrh(dst(), arg[immA]); }
                                 else        { a->ldrd(dst(), arg[immA]); }
                                               a->uxtlh2s(dst(), dst());
                                               break;

                case Op::load32: if (scalar) { a->ldrs(dst(), arg[immA]); }
                                 else        { a->ldrq(dst(), arg[immA]); }
                                               break;

                case Op::load64: if (scalar) {
                                    a->ldrs(dst(), arg[immA], immB);
                                 } else {
                                    Reg tmp0 = alloc_tmp(2),
                                        tmp1 = (Reg)(tmp0+1);
                                    a->ld24s(tmp0, arg[immA]);
                                    // TODO: return both
                                    switch (immB) {
                                        case 0: mark_tmp_as_dst(tmp0); free_tmp(tmp1); break;
                                        case 1: mark_tmp_as_dst(tmp1); free_tmp(tmp0); break;
                                    }
                                 } break;

                case Op::load128: if (scalar) {
                                      a->ldrs(dst(), arg[immA], immB);
                                  } else {
                                      Reg tmp0 = alloc_tmp(4),
                                          tmp1 = (Reg)(tmp0+1),
                                          tmp2 = (Reg)(tmp0+2),
                                          tmp3 = (Reg)(tmp0+3);
                                      a->ld44s(tmp0, arg[immA]);
                                      // TODO: return all four
                                      switch (immB) {
                                          case 0: mark_tmp_as_dst(tmp0); break;
                                          case 1: mark_tmp_as_dst(tmp1); break;
                                          case 2: mark_tmp_as_dst(tmp2); break;
                                          case 3: mark_tmp_as_dst(tmp3); break;
                                      }
                                      if (immB != 0) { free_tmp(tmp0); }
                                      if (immB != 1) { free_tmp(tmp1); }
                                      if (immB != 2) { free_tmp(tmp2); }
                                      if (immB != 3) { free_tmp(tmp3); }
                                  } break;

                case Op::uniform32: a->add(GP0, arg[immA], immB);
                                    a->ld1r4s(dst(), GP0);
                                    break;

                case Op::array32: a->add(GP0, arg[immA], immB);
                                  a->ldrd(GP0, GP0);
                                  a->add(GP0, GP0, immC);
                                  a->ld1r4s(dst(), GP0);
                                  break;

                case Op::gather8: {
                    // As usual, the gather base pointer is immB bytes off of uniform immA.
                    a->add (GP0, arg[immA], immB);  // GP0 = &(gather base pointer)
                    a->ldrd(GP0, GP0);              // GP0 =   gather base pointer

                    for (int i = 0; i < active_lanes; i++) {
                        a->movs(GP1, r(x), i);    // Extract index lane i into GP1.
                        a->add (GP1, GP0, GP1);   // Add the gather base pointer.
                        a->ldrb(GP1, GP1);        // Load that byte.
                        a->inss(dst(x), GP1, i);  // Insert it into dst() lane i.
                    }
                } break;

                // See gather8 for general idea; comments here only where gather16 differs.
                case Op::gather16: {
                    a->add (GP0, arg[immA], immB);
                    a->ldrd(GP0, GP0);
                    for (int i = 0; i < active_lanes; i++) {
                        a->movs(GP1, r(x), i);
                        a->add (GP1, GP0, GP1, A::LSL, 1);  // Scale index 2x into a byte offset.
                        a->ldrh(GP1, GP1);                  // 2-byte load.
                        a->inss(dst(x), GP1, i);
                    }
                } break;

                // See gather8 for general idea; comments here only where gather32 differs.
                case Op::gather32: {
                    a->add (GP0, arg[immA], immB);
                    a->ldrd(GP0, GP0);
                    for (int i = 0; i < active_lanes; i++) {
                        a->movs(GP1, r(x), i);
                        a->add (GP1, GP0, GP1, A::LSL, 2);  // Scale index 4x into a byte offset.
                        a->ldrs(GP1, GP1);                  // 4-byte load.
                        a->inss(dst(x), GP1, i);
                    }
                } break;

                case Op::add_f32: a->fadd4s(dst(x,y), r(x), r(y)); break;
                case Op::sub_f32: a->fsub4s(dst(x,y), r(x), r(y)); break;
                case Op::mul_f32: a->fmul4s(dst(x,y), r(x), r(y)); break;
                case Op::div_f32: a->fdiv4s(dst(x,y), r(x), r(y)); break;

                case Op::sqrt_f32: a->fsqrt4s(dst(x), r(x)); break;

                case Op::fma_f32: // fmla.4s is z += x*y
                    if (try_alias(z)) { a->fmla4s( r(z), r(x), r(y)); }
                    else              { a->orr16b(dst(), r(z), r(z));
                                        a->fmla4s(dst(), r(x), r(y)); }
                                        break;

                case Op::fnma_f32:  // fmls.4s is z -= x*y
                    if (try_alias(z)) { a->fmls4s( r(z), r(x), r(y)); }
                    else              { a->orr16b(dst(), r(z), r(z));
                                        a->fmls4s(dst(), r(x), r(y)); }
                                        break;

                case Op::fms_f32:   // calculate z - xy, then negate to xy - z
                    if (try_alias(z)) { a->fmls4s( r(z), r(x), r(y)); }
                    else              { a->orr16b(dst(), r(z), r(z));
                                        a->fmls4s(dst(), r(x), r(y)); }
                                        a->fneg4s(dst(), dst());
                                        break;

                case Op:: gt_f32: a->fcmgt4s (dst(x,y), r(x), r(y)); break;
                case Op::gte_f32: a->fcmge4s (dst(x,y), r(x), r(y)); break;
                case Op:: eq_f32: a->fcmeq4s (dst(x,y), r(x), r(y)); break;
                case Op::neq_f32: a->fcmeq4s (dst(x,y), r(x), r(y));
                                  a->not16b  (dst(), dst());         break;


                case Op::add_i32: a->add4s(dst(x,y), r(x), r(y)); break;
                case Op::sub_i32: a->sub4s(dst(x,y), r(x), r(y)); break;
                case Op::mul_i32: a->mul4s(dst(x,y), r(x), r(y)); break;

                case Op::bit_and  : a->and16b(dst(x,y), r(x), r(y)); break;
                case Op::bit_or   : a->orr16b(dst(x,y), r(x), r(y)); break;
                case Op::bit_xor  : a->eor16b(dst(x,y), r(x), r(y)); break;
                case Op::bit_clear: a->bic16b(dst(x,y), r(x), r(y)); break;

                case Op::select: // bsl16b is x = x ? y : z
                    if (try_alias(x)) { a->bsl16b( r(x), r(y), r(z)); }
                    else              { a->orr16b(dst(), r(x), r(x));
                                        a->bsl16b(dst(), r(y), r(z)); }
                                        break;

                // fmin4s and fmax4s don't work the way we want with NaN,
                // so we write them the long way:
                case Op::min_f32: // min(x,y) = y<x ? y : x
                                  a->fcmgt4s(dst(), r(x), r(y));
                                  a->bsl16b (dst(), r(y), r(x));
                                  break;

                case Op::max_f32: // max(x,y) = x<y ? y : x
                                  a->fcmgt4s(dst(), r(y), r(x));
                                  a->bsl16b (dst(), r(y), r(x));
                                  break;

                case Op::shl_i32: a-> shl4s(dst(x), r(x), immA); break;
                case Op::shr_i32: a->ushr4s(dst(x), r(x), immA); break;
                case Op::sra_i32: a->sshr4s(dst(x), r(x), immA); break;

                case Op::eq_i32: a->cmeq4s(dst(x,y), r(x), r(y)); break;
                case Op::gt_i32: a->cmgt4s(dst(x,y), r(x), r(y)); break;

                case Op::to_f32: a->scvtf4s (dst(x), r(x)); break;
                case Op::trunc:  a->fcvtzs4s(dst(x), r(x)); break;
                case Op::round:  a->fcvtns4s(dst(x), r(x)); break;
                case Op::ceil:   a->frintp4s(dst(x), r(x)); break;
                case Op::floor:  a->frintm4s(dst(x), r(x)); break;

                case Op::to_fp16:
                    a->fcvtn  (dst(x), r(x));    // 4x f32 -> 4x f16 in bottom four lanes
                    a->uxtlh2s(dst(), dst());    // expand to 4x f16 in even 16-bit lanes
                    break;

                case Op::from_fp16:
                    a->xtns2h(dst(x), r(x));     // pack even 16-bit lanes into bottom four lanes
                    a->fcvtl (dst(), dst());     // 4x f16 -> 4x f32
                    break;
            #endif
            }

            // Proactively free the registers holding any value that dies here.
            if (rd != NA &&                   dies_here(regs[rd])) { regs[rd] = NA; }
            if (rx != NA && regs[rx] != NA && dies_here(regs[rx])) { regs[rx] = NA; }
            if (ry != NA && regs[ry] != NA && dies_here(regs[ry])) { regs[ry] = NA; }
            if (rz != NA && regs[rz] != NA && dies_here(regs[rz])) { regs[rz] = NA; }
            if (rw != NA && regs[rw] != NA && dies_here(regs[rw])) { regs[rw] = NA; }
            return true;
        };

        #if defined(__x86_64__) || defined(_M_X64)
            auto jump_if_less = [&](A::Label* l) { a->jl (l); };
            auto jump         = [&](A::Label* l) { a->jmp(l); };

            auto add = [&](A::GP64 gp, int imm) { a->add(gp, imm); };
            auto sub = [&](A::GP64 gp, int imm) { a->sub(gp, imm); };
        #elif defined(__aarch64__)
            auto jump_if_less = [&](A::Label* l) { a->blt(l); };
            auto jump         = [&](A::Label* l) { a->b  (l); };

            auto add = [&](A::X gp, int imm) { a->add(gp, gp, imm); };
            auto sub = [&](A::X gp, int imm) { a->sub(gp, gp, imm); };
        #endif

        A::Label body,
                 tail,
                 done;

        enter();
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            if (instructions[id].can_hoist && !emit(id, /*scalar=*/false)) {
                return false;
            }
        }

        // This point marks a kind of canonical fixed point for register contents: if loop
        // code is generated as if these registers are holding these values, the next time
        // the loop comes around we'd better find those same registers holding those same values.
        auto restore_incoming_regs = [&,incoming=regs,saved_stack_slot=stack_slot,
                                      saved_next_stack_slot=next_stack_slot]{
            for (int r = 0; r < (int)regs.size(); r++) {
                if (regs[r] != incoming[r]) {
                    regs[r]  = incoming[r];
                    if (regs[r] >= 0) {
                        load_from_memory((Reg)r, regs[r]);
                    }
                }
            }
            *stack_hint = std::max(*stack_hint, next_stack_slot);
            stack_slot = saved_stack_slot;
            next_stack_slot = saved_next_stack_slot;
        };

        a->label(&body);
        {
            a->cmp(N, K);
            jump_if_less(&tail);
            for (Val id = 0; id < (Val)instructions.size(); id++) {
                if (!instructions[id].can_hoist && !emit(id, /*scalar=*/false)) {
                    return false;
                }
            }
            restore_incoming_regs();
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
                if (!instructions[id].can_hoist && !emit(id, /*scalar=*/true)) {
                    return false;
                }
            }
            restore_incoming_regs();
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

        constants.foreach([&](int imm, A::Label* label) {
            a->align(4);
            a->label(label);
            for (int i = 0; i < K; i++) {
                a->word(imm);
            }
        });

        if (!iota.references.empty()) {
            a->align(4);
            a->label(&iota);        // 0,1,2,3,4,...
            for (int i = 0; i < K; i++) {
                a->word(i);
            }
        }

        if (!load64_index.references.empty()) {
            a->align(4);
            a->label(&load64_index);  // {0,2,4,6|1,3,5,7}
            a->word(0); a->word(2); a->word(4); a->word(6);
            a->word(1); a->word(3); a->word(5); a->word(7);
        }

        return true;
    }

    void Program::setupJIT(const std::vector<OptimizedInstruction>& instructions,
                           const char* debug_name) {
        // Assemble with no buffer to determine a.size() (the number of bytes we'll assemble)
        // and stack_hint/registers_used to feed forward into the next jit() call.
        Assembler a{nullptr};
        int stack_hint = -1;
        uint32_t registers_used = 0xffff'ffff;  // Start conservatively with all.
        if (!this->jit(instructions, &stack_hint, &registers_used, &a)) {
            return;
        }

        fImpl->jit_size = a.size();
        void* jit_entry = alloc_jit_buffer(&fImpl->jit_size);
        fImpl->jit_entry.store(jit_entry);

        // Assemble the program for real with stack_hint/registers_used as feedback from first call.
        a = Assembler{jit_entry};
        SkAssertResult(this->jit(instructions, &stack_hint, &registers_used, &a));
        SkASSERT(a.size() <= fImpl->jit_size);

        // Remap as executable, and flush caches on platforms that need that.
        remap_as_executable(jit_entry, fImpl->jit_size);

        notify_vtune(debug_name, jit_entry, fImpl->jit_size);

    #if !defined(SK_BUILD_FOR_WIN)
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
            void* sym = nullptr;
            for (const char* name : {"skvm_jit", "_skvm_jit"} ) {
                if (!sym) { sym = dlsym(fImpl->dylib, name); }
            }
            fImpl->jit_entry.store(sym);
        }
    #endif
    }

    void Program::disassemble(SkWStream* o) const {
    #if !defined(SK_BUILD_FOR_WIN)
        SkDebugfStream debug;
        if (!o) { o = &debug; }

        const void* jit_entry = fImpl->jit_entry.load();
        size_t jit_size = fImpl->jit_size;

        if (!jit_entry) {
            o->writeText("Program not JIT'd. Did you pass --jit?\n");
            return;
        }

        char path[] = "/tmp/skvm-jit.XXXXXX";
        int fd = mkstemp(path);
        ::write(fd, jit_entry, jit_size);
        close(fd);

        // Convert it in-place to a dynamic library with a single symbol "skvm_jit":
        SkString cmd = SkStringPrintf(
                "echo '.global _skvm_jit\n_skvm_jit: .incbin \"%s\"'"
                " | clang -x assembler -shared - -o %s",
                path, path);
        system(cmd.c_str());

        // Now objdump to disassemble our function:
        // TODO: We could trim this down to just our code using '--disassemble=<symbol name>`,
        // but the symbol name varies with OS, and that option may be missing from objdump on some
        // machines? There also apears to be quite a bit of junk after the end of the JIT'd code.
        // Trimming that would let us pass '--visualize-jumps' and get the loop annotated.
        // With the junk, we tend to end up with a bunch of stray jumps that pollute the ASCII art.
        cmd = SkStringPrintf("objdump -D %s", path);
    #if defined(SK_BUILD_FOR_UNIX)
        cmd.append(" --section=.text");
    #endif
        FILE* fp = popen(cmd.c_str(), "r");
        if (!fp) {
            o->writeText("objdump failed\n");
            return;
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            o->writeText(line);
        }

        pclose(fp);
    #endif
    }

#endif

}  // namespace skvm
