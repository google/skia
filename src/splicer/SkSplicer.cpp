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
//
// On x86, we'll include IACA markers too.
//   https://software.intel.com/en-us/articles/intel-architecture-code-analyzer
// Running IACA will disassemble, and more.
//   $ ./iaca.sh -arch HSW -64 -mark 0 /tmp/dump.bin | less
//
// To disassemble an aarch64 dump,
//   $ gobjdump -b binary -D dump.bin -m aarch64
//
// To disassemble an armv7 dump,
//   $ gobjdump -b binary -D dump.bin -m arm

namespace {

    // Stages expect these constants to be set to these values.
    // It's fine to rearrange and add new ones if you update SkSplicer_constants.
    static const SkSplicer_constants kConstants = {
        0x000000ff, 1.0f, 255.0f, 1/255.0f,
        0.0025f, 0.6975f, 0.3000f, 1/12.92f, 0.055f,       // from_srgb
        12.46f, 0.411192f, 0.689206f, -0.0988f, 0.0043f,   //   to_srgb
    };

    // We do this a lot, so it's nice to infer the correct size.  Works fine with arrays.
    template <typename T>
    static void splice(SkWStream* buf, const T& val) {
        buf->write(&val, sizeof(val));
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
        splice(buf, 0x91001000);        // add x0, x0, #4
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
        splice(buf, 0xe2800002);  // add r0, r0, #2
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
        static const uint8_t  addq_8_rdi[] = { 0x48, 0x83, 0xc7, 0x08 };
        static const uint8_t cmp_rsi_rdi[] = { 0x48, 0x39, 0xf7 };
        static const uint8_t     jb_near[] = { 0x0f, 0x8c };
        splice(buf, addq_8_rdi);   // addq $8, %rdi
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
            0xc5,0x78,0x29,0xbc,0x24,0x90,0x00,0x00,0x00, // vmovaps %xmm15,0x90(%rsp)
            0xc5,0x78,0x29,0xb4,0x24,0x80,0x00,0x00,0x00, // vmovaps %xmm14,0x80(%rsp)
            0xc5,0x78,0x29,0x6c,0x24,0x70,                // vmovaps %xmm13,0x70(%rsp)
            0xc5,0x78,0x29,0x64,0x24,0x60,                // vmovaps %xmm12,0x60(%rsp)
            0xc5,0x78,0x29,0x5c,0x24,0x50,                // vmovaps %xmm11,0x50(%rsp)
            0xc5,0x78,0x29,0x54,0x24,0x40,                // vmovaps %xmm10,0x40(%rsp)
            0xc5,0x78,0x29,0x4c,0x24,0x30,                // vmovaps %xmm9,0x30(%rsp)
            0xc5,0x78,0x29,0x44,0x24,0x20,                // vmovaps %xmm8,0x20(%rsp)
            0xc5,0xf8,0x29,0x7c,0x24,0x10,                // vmovaps %xmm7,0x10(%rsp)
            0xc5,0xf8,0x29,0x34,0x24,                     // vmovaps %xmm6,(%rsp)
            0x48,0x89,0xcf,                               // mov    %rcx,%rdi
            0x48,0x89,0xd6,                               // mov    %rdx,%rsi
            0x4c,0x89,0xc2,                               // mov    %r8,%rdx
            0x4c,0x89,0xc9,                               // mov    %r9,%rcx
        };
        splice(buf, ms_to_system_v);
    }
    static void after_loop(SkWStream* buf) {
        static const uint8_t system_v_to_ms[] = {
            0xc5,0xf8,0x28,0x34,0x24,                     // vmovaps (%rsp),%xmm6
            0xc5,0xf8,0x28,0x7c,0x24,0x10,                // vmovaps 0x10(%rsp),%xmm7
            0xc5,0x78,0x28,0x44,0x24,0x20,                // vmovaps 0x20(%rsp),%xmm8
            0xc5,0x78,0x28,0x4c,0x24,0x30,                // vmovaps 0x30(%rsp),%xmm9
            0xc5,0x78,0x28,0x54,0x24,0x40,                // vmovaps 0x40(%rsp),%xmm10
            0xc5,0x78,0x28,0x5c,0x24,0x50,                // vmovaps 0x50(%rsp),%xmm11
            0xc5,0x78,0x28,0x64,0x24,0x60,                // vmovaps 0x60(%rsp),%xmm12
            0xc5,0x78,0x28,0x6c,0x24,0x70,                // vmovaps 0x70(%rsp),%xmm13
            0xc5,0x78,0x28,0xb4,0x24,0x80,0x00,0x00,0x00, // vmovaps 0x80(%rsp),%xmm14
            0xc5,0x78,0x28,0xbc,0x24,0x90,0x00,0x00,0x00, // vmovaps 0x90(%rsp),%xmm15
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

    struct Spliced {

        Spliced(const SkRasterPipeline::Stage* stages, int nstages) {
            // We always create a backup interpreter pipeline,
            //   - to handle any program we can't, and
            //   - to handle the n < kStride tails.
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
            //        x += kStride;
            //    } while(x < limit);
            before_loop(&buf);
            auto loop_start = buf.bytesWritten();  // Think of this like a label, loop_start:

            for (int i = 0; i < nstages; i++) {
                // If a stage has a context pointer, load it into rdx/x2, Stage argument 3 "ctx".
                if (stages[i].ctx) {
                    set_ctx(&buf, stages[i].ctx);
                }

                // Splice in the code for the Stages, generated offline into SkSplicer_generated.h.
                switch(stages[i].stage) {
                    case SkRasterPipeline::clear:        splice(&buf, kSplice_clear       ); break;
                    case SkRasterPipeline::plus_:        splice(&buf, kSplice_plus        ); break;
                    case SkRasterPipeline::srcover:      splice(&buf, kSplice_srcover     ); break;
                    case SkRasterPipeline::dstover:      splice(&buf, kSplice_dstover     ); break;
                    case SkRasterPipeline::clamp_0:      splice(&buf, kSplice_clamp_0     ); break;
                    case SkRasterPipeline::clamp_1:      splice(&buf, kSplice_clamp_1     ); break;
                    case SkRasterPipeline::clamp_a:      splice(&buf, kSplice_clamp_a     ); break;
                    case SkRasterPipeline::swap:         splice(&buf, kSplice_swap        ); break;
                    case SkRasterPipeline::move_src_dst: splice(&buf, kSplice_move_src_dst); break;
                    case SkRasterPipeline::move_dst_src: splice(&buf, kSplice_move_dst_src); break;
                    case SkRasterPipeline::premul:       splice(&buf, kSplice_premul      ); break;
                    case SkRasterPipeline::unpremul:     splice(&buf, kSplice_unpremul    ); break;
                    case SkRasterPipeline::from_srgb:    splice(&buf, kSplice_from_srgb   ); break;
                    case SkRasterPipeline::to_srgb:      splice(&buf, kSplice_to_srgb     ); break;
                    case SkRasterPipeline::scale_u8:     splice(&buf, kSplice_scale_u8    ); break;
                    case SkRasterPipeline::load_tables:  splice(&buf, kSplice_load_tables ); break;
                    case SkRasterPipeline::load_8888:    splice(&buf, kSplice_load_8888   ); break;
                    case SkRasterPipeline::store_8888:   splice(&buf, kSplice_store_8888  ); break;
                    case SkRasterPipeline::load_f16:     splice(&buf, kSplice_load_f16    ); break;
                    case SkRasterPipeline::store_f16:    splice(&buf, kSplice_store_f16   ); break;
                    case SkRasterPipeline::matrix_3x4:   splice(&buf, kSplice_matrix_3x4  ); break;

                    // No joy (probably just not yet implemented).
                    default:
                        //SkDebugf("SkSplicer can't yet handle stage %d.\n", stages[i].stage);
                        return;
                }
            }

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
        void operator()(size_t x, size_t y, size_t n) const {
            size_t body = n/kStride*kStride;   // Largest multiple of kStride (4 or 8) <= n.
            if (fSpliced && body) {            // Can we run fSpliced for at least one kStride?
                // TODO: At some point we will want to pass in y...
                using Fn = void(size_t x, size_t limit, void* ctx, const SkSplicer_constants* k);
                ((Fn*)fSpliced)(x, x+body, nullptr, &kConstants);

                // Fall through to fBackup for any n<kStride last pixels.
                x += body;
                n -= body;
            }
            fBackup(x,y,n);
        }

        std::function<void(size_t, size_t, size_t)> fBackup;
        size_t                                      fSplicedLen;
        void*                                       fSpliced;
    };

}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::jit() const {
    return Spliced(fStages.data(), SkToInt(fStages.size()));
}
