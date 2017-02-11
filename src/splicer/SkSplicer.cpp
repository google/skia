/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCpu.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"
#include "SkStream.h"
#if defined(_MSC_VER)
    #include <windows.h>
#else
    #include <sys/mman.h>
#endif

#include "SkSplicer_generated.h"
#include "SkSplicer_shared.h"

// Uncomment to dump output JIT'd pipeline.
//#define DUMP "/tmp/dump.bin"
//#define DUMP "/data/local/tmp/dump.bin"
//
// On x86, we'll include IACA markers too.
//   https://software.intel.com/en-us/articles/intel-architecture-code-analyzer
// Running IACA will disassemble, and more.
//   $ ./iaca.sh -arch HSW -64 -mark 0 /tmp/dump.bin | less
//
// To disassemble an aarch64 dump,
//   $ adb pull /data/local/tmp/dump.bin; gobjdump -b binary -D dump.bin -m aarch64 | less
//
// To disassemble an armv7 dump,
//   $ adb pull /data/local/tmp/dump.bin; gobjdump -b binary -D dump.bin -m arm | less

namespace {

    // Stages expect these constants to be set to these values.
    // It's fine to rearrange and add new ones if you update SkSplicer_constants.
    static const SkSplicer_constants kConstants = {
        1.0f, 255.0f, 1/255.0f, 0x000000ff,
        0.0025f, 0.6975f, 0.3000f, 1/12.92f, 0.055f,       // from_srgb
        12.46f, 0.411192f, 0.689206f, -0.0988f, 0.0043f,   //   to_srgb
        0x77800000, 0x07800000,                            // fp16 <-> fp32
    };

    // We do this a lot, so it's nice to infer the correct size.  Works fine with arrays.
    template <typename T>
    static void splice(SkWStream* buf, const T& val) {
        buf->write(&val, sizeof(val));
    }

    // Splice up to (but not including) the final return instruction in code.
    template <typename T, size_t N>
    static void splice_until_ret(SkWStream* buf, const T (&code)[N]) {
        // On all platforms we splice today, return is a single T (byte on x86, u32 on ARM).
        buf->write(&code, sizeof(T) * (N-1));
    }

#if defined(__aarch64__)
    static constexpr int kStride = 4;
    static void set_ctx(SkWStream* buf, void* ctx) {
        uint16_t parts[4];
        memcpy(parts, &ctx, 8);
        splice(buf, 0xd2f00000 | (parts[3] << 5) | 0x2);  // move  16-bit intermediate << 48 into x2
        splice(buf, 0xf2c00000 | (parts[2] << 5) | 0x2);  // merge 16-bit intermediate << 32 into x2
        splice(buf, 0xf2a00000 | (parts[1] << 5) | 0x2);  // merge 16-bit intermediate << 16 into x2
        splice(buf, 0xf2800000 | (parts[0] << 5) | 0x2);  // merge 16-bit intermediate <<  0 into x2
    }
    static void loop(SkWStream* buf, int loop_start) {
        splice(buf, 0xeb01001f);        // cmp x0, x1
        int off = loop_start - (int)buf->bytesWritten();
        off /= 4;   // bytes -> instructions, still signed
        off = (off & 0x7ffff) << 5;  // 19 bit maximum range (+- 256K instructions)
        splice(buf, 0x54000003 | off); // b.cc loop_start  (cc == "carry clear", unsigned less than)
    }
    static void ret(SkWStream* buf) {
        splice(buf, 0xd65f03c0);  // ret
    }
#elif defined(__ARM_NEON__)
    static constexpr int kStride = 2;
    static void set_ctx(SkWStream* buf, void* ctx) {
        uint16_t parts[2];
        auto encode = [](uint16_t part) -> uint32_t {
            return (part & 0xf000) << 4 | (part & 0xfff);
        };
        memcpy(parts, &ctx, 4);
        splice(buf, 0xe3002000 | encode(parts[0]));  // mov  r2, <bottom 16 bits>
        splice(buf, 0xe3402000 | encode(parts[1]));  // movt r2,    <top 16 bits>
    }
    static void loop(SkWStream* buf, int loop_start) {
        splice(buf, 0xe1500001);  // cmp r0, r1
        int off = loop_start - ((int)buf->bytesWritten() + 8 /*ARM is weird*/);
        off /= 4;   // bytes -> instructions, still signed
        off = (off & 0x00ffffff);
        splice(buf,  0x3a000000 | off);  // bcc loop_start
    }
    static void ret(SkWStream* buf) {
        splice(buf, 0xe12fff1e);  // bx lr
    }
#else
    static constexpr int kStride = 8;
    static void set_ctx(SkWStream* buf, void* ctx) {
        static const uint8_t movabsq_rdx[] = { 0x48, 0xba };
        splice(buf, movabsq_rdx);  // movabsq <next 8 bytes>, %rdx
        splice(buf, ctx);
    }
    static void loop(SkWStream* buf, int loop_start) {
        static const uint8_t cmp_rsi_rdi[] = { 0x48, 0x39, 0xf7 };
        static const uint8_t     jb_near[] = { 0x0f, 0x8c };
        splice(buf, cmp_rsi_rdi);  // cmp %rsi, %rdi
        splice(buf, jb_near);      // jb <next 4 bytes>  (b == "before", unsigned less than)
        splice(buf, loop_start - (int)(buf->bytesWritten() + 4));
    }
    static void ret(SkWStream* buf) {
        static const uint8_t vzeroupper[] = { 0xc5, 0xf8, 0x77 };
        static const uint8_t        ret[] = { 0xc3 };
        splice(buf, vzeroupper);
        splice(buf, ret);
    }
#endif

#if defined(_MSC_VER)
    // Adapt from MS ABI to System V ABI used by stages.
    static void before_loop(SkWStream* buf) {
        static const uint8_t ms_to_system_v[] = {
            0x56,                                         // push   %rsi
            0x57,                                         // push   %rdi
            0x48,0x81,0xec,0xa8,0x00,0x00,0x00,           // sub    $0xa8,%rsp
            0x44,0x0f,0x29,0xbc,0x24,0x90,0x00,0x00,0x00, // movaps %xmm15,0x90(%rsp)
            0x44,0x0f,0x29,0xb4,0x24,0x80,0x00,0x00,0x00, // movaps %xmm14,0x80(%rsp)
            0x44,0x0f,0x29,0x6c,0x24,0x70,                // movaps %xmm13,0x70(%rsp)
            0x44,0x0f,0x29,0x64,0x24,0x60,                // movaps %xmm12,0x60(%rsp)
            0x44,0x0f,0x29,0x5c,0x24,0x50,                // movaps %xmm11,0x50(%rsp)
            0x44,0x0f,0x29,0x54,0x24,0x40,                // movaps %xmm10,0x40(%rsp)
            0x44,0x0f,0x29,0x4c,0x24,0x30,                // movaps %xmm9,0x30(%rsp)
            0x44,0x0f,0x29,0x44,0x24,0x20,                // movaps %xmm8,0x20(%rsp)
            0x0f,0x29,0x7c,0x24,0x10,                     // movaps %xmm7,0x10(%rsp)
            0x0f,0x29,0x34,0x24,                          // movaps %xmm6,(%rsp)
            0x48,0x89,0xcf,                               // mov    %rcx,%rdi
            0x48,0x89,0xd6,                               // mov    %rdx,%rsi
            0x4c,0x89,0xc2,                               // mov    %r8,%rdx
            0x4c,0x89,0xc9,                               // mov    %r9,%rcx
        };
        splice(buf, ms_to_system_v);
    }
    static void after_loop(SkWStream* buf) {
        static const uint8_t system_v_to_ms[] = {
            // TODO: vzeroupper here?
            0x0f,0x28,0x34,0x24,                          // movaps (%rsp),%xmm6
            0x0f,0x28,0x7c,0x24,0x10,                     // movaps 0x10(%rsp),%xmm7
            0x44,0x0f,0x28,0x44,0x24,0x20,                // movaps 0x20(%rsp),%xmm8
            0x44,0x0f,0x28,0x4c,0x24,0x30,                // movaps 0x30(%rsp),%xmm9
            0x44,0x0f,0x28,0x54,0x24,0x40,                // movaps 0x40(%rsp),%xmm10
            0x44,0x0f,0x28,0x5c,0x24,0x50,                // movaps 0x50(%rsp),%xmm11
            0x44,0x0f,0x28,0x64,0x24,0x60,                // movaps 0x60(%rsp),%xmm12
            0x44,0x0f,0x28,0x6c,0x24,0x70,                // movaps 0x70(%rsp),%xmm13
            0x44,0x0f,0x28,0xb4,0x24,0x80,0x00,0x00,0x00, // movaps 0x80(%rsp),%xmm14
            0x44,0x0f,0x28,0xbc,0x24,0x90,0x00,0x00,0x00, // movaps 0x90(%rsp),%xmm15
            0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,           // add    $0xa8,%rsp
            0x5f,                                         // pop    %rdi
            0x5e,                                         // pop    %rsi
        };
        splice(buf, system_v_to_ms);
    }
#elif !defined(__aarch64__) && !defined(__ARM_NEON__) && defined(DUMP)
    // IACA start and end markers.
    static const uint8_t      ud2[] = { 0x0f, 0x0b };         // undefined... crashes when run
    static const uint8_t     nop3[] = { 0x64, 0x67, 0x90 };   // 3 byte no-op
    static const uint8_t movl_ebx[] = { 0xbb };               // move next 4 bytes into ebx

    static void before_loop(SkWStream* buf) {
        splice(buf, ud2);
        splice(buf, movl_ebx);
        splice(buf, 111);
        splice(buf, nop3);
    }
    static void after_loop(SkWStream* buf) {
        splice(buf, movl_ebx);
        splice(buf, 222);
        splice(buf, nop3);
        splice(buf, ud2);
    }
#else
    static void before_loop(SkWStream*) {}
    static void after_loop (SkWStream*) {}
#endif

    // We can only mprotect / VirtualProtect at 4K page granularity.
    static size_t round_up_to_full_pages(size_t len) {
        size_t size = 0;
        while (size < len) {
            size += 4096;
        }
        return size;
    }

#if defined(_MSC_VER)
    // Copy len bytes from src to memory that's executable.  cleanup with cleanup_executable_mem().
    static void* copy_to_executable_mem(const void* src, size_t* len) {
        if (!src || !*len) {
            return nullptr;
        }

        size_t alloc = round_up_to_full_pages(*len);

        auto fn = VirtualAlloc(nullptr, alloc, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        memcpy(fn, src, *len);

        DWORD dont_care;
        VirtualProtect(fn, alloc, PAGE_EXECUTE_READ, &dont_care);

        *len = alloc;
        return fn;
    }
    static void cleanup_executable_mem(void* fn, size_t len) {
        if (fn) {
            VirtualFree(fn, 0, MEM_RELEASE);
        }
    }
#else
    static void* copy_to_executable_mem(const void* src, size_t* len) {
        if (!src || !*len) {
            return nullptr;
        }

        size_t alloc = round_up_to_full_pages(*len);

        auto fn = mmap(nullptr, alloc, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        memcpy(fn, src, *len);

        mprotect(fn, alloc, PROT_READ|PROT_EXEC);
        __builtin___clear_cache((char*)fn, (char*)fn + *len);  // Essential on ARM; no-op on x86.

        *len = alloc;
        return fn;
    }
    static void cleanup_executable_mem(void* fn, size_t len) {
        if (fn) {
            munmap(fn, len);
        }
    }
#endif

    static bool splice(SkWStream* buf, SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return false;
        #define CASE(st) case SkRasterPipeline::st: splice_until_ret(buf, kSplice_##st); break
            CASE(clear);
            CASE(plus_);
            CASE(srcover);
            CASE(dstover);
            CASE(clamp_0);
            CASE(clamp_1);
            CASE(clamp_a);
            CASE(swap);
            CASE(move_src_dst);
            CASE(move_dst_src);
            CASE(premul);
            CASE(unpremul);
            CASE(from_srgb);
            CASE(to_srgb);
            CASE(scale_u8);
            CASE(load_tables);
            CASE(load_8888);
            CASE(store_8888);
            CASE(load_f16);
            CASE(store_f16);
            CASE(matrix_3x4);
        #undef CASE
        }
        return true;
    }

    struct Spliced {

        Spliced(const SkRasterPipeline::Stage* stages, int nstages) {
            // We always create a backup interpreter pipeline,
            //   - to handle any program we can't, and
            //   - to handle the n < stride tails.
            fBackup     = SkOpts::compile_pipeline(stages, nstages);
            fSplicedLen = 0;
            fSpliced    = nullptr;
            // If we return early anywhere in here, !fSpliced means we'll use fBackup instead.

        #if defined(__aarch64__)
        #elif defined(__ARM_NEON__)
            // Late generation ARMv7, e.g. Cortex A15 or Krait.
            if (!SkCpu::Supports(SkCpu::NEON|SkCpu::NEON_FMA|SkCpu::VFP_FP16)) {
                return;
            }
        #else
            // To keep things simple, only one x86 target supported: Haswell+ x86-64.
            if (!SkCpu::Supports(SkCpu::HSW) || sizeof(void*) != 8) {
                return;
            }
        #endif

            SkDynamicMemoryWStream buf;

            // Our loop is the equivalent of this C++ code:
            //    do {
            //        ... run spliced stages...
            //        x += stride;
            //    } while(x < limit);
            before_loop(&buf);
            auto loop_start = buf.bytesWritten();  // Think of this like a label, loop_start:

            for (int i = 0; i < nstages; i++) {
                // If a stage has a context pointer, load it into rdx/x2, Stage argument 3 "ctx".
                if (stages[i].ctx) {
                    set_ctx(&buf, stages[i].ctx);
                }

                // Splice in the code for the Stages, generated offline into SkSplicer_generated.h.
                if (!splice(&buf, stages[i].stage)) {
                    //SkDebugf("SkSplicer can't yet handle stage %d.\n", stages[i].stage);
                    return;
                }
            }

            splice_until_ret(&buf, kSplice_inc_x);
            loop(&buf, loop_start);  // Loop back to handle more pixels if not done.
            after_loop(&buf);
            ret(&buf);  // We're done.

            auto data = buf.detachAsData();
            fSplicedLen = data->size();
            fSpliced    = copy_to_executable_mem(data->data(), &fSplicedLen);

        #if defined(DUMP)
            SkFILEWStream(DUMP).write(data->data(), data->size());
        #endif
        }

        // Spliced is stored in a std::function, so it needs to be copyable.
        Spliced(const Spliced& o) : fBackup    (o.fBackup)
                                  , fSplicedLen(o.fSplicedLen)
                                  , fSpliced   (copy_to_executable_mem(o.fSpliced, &fSplicedLen)) {}

        ~Spliced() {
            cleanup_executable_mem(fSpliced, fSplicedLen);
        }

        // Here's where we call fSpliced if we created it, fBackup if not.
        void operator()(size_t x, size_t n) const {
            size_t body = n/kStride*kStride;   // Largest multiple of kStride (2, 4, 8, or 16) <= n.
            if (fSpliced && body) {            // Can we run fSpliced for at least one stride?
                using Fn = void(size_t x, size_t limit, void* ctx, const void* k);
                ((Fn*)fSpliced)(x, x+body, nullptr, &kConstants);

                // Fall through to fBackup for any n<stride last pixels.
                x += body;
                n -= body;
            }
            fBackup(x,n);
        }

        std::function<void(size_t, size_t)> fBackup;
        size_t                              fSplicedLen;
        void*                               fSpliced;
    };

}

std::function<void(size_t, size_t)> SkRasterPipeline::jit() const {
    return Spliced(fStages.data(), SkToInt(fStages.size()));
}
