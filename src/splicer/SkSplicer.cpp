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
#include <sys/mman.h>

#include "SkSplicer_generated.h"
#include "SkSplicer_shared.h"

// Uncomment to dump output with IACA markers.
// #define IACA_DUMP "/tmp/dump.o"
// https://software.intel.com/en-us/articles/intel-architecture-code-analyzer
// $ ./iaca.sh -arch HSW -64 -mark 0 /tmp/dump.o | less

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
    static void set_k(SkWStream* buf, const SkSplicer_constants* k) {
        uint16_t parts[4];
        memcpy(parts, &k, 8);
        splice(buf, 0xd2f00000 | (parts[3] << 5) | 0x3);  // move  16-bit intermediate << 48 into x3
        splice(buf, 0xf2c00000 | (parts[2] << 5) | 0x3);  // merge 16-bit intermediate << 32 into x3
        splice(buf, 0xf2a00000 | (parts[1] << 5) | 0x3);  // merge 16-bit intermediate << 16 into x3
        splice(buf, 0xf2800000 | (parts[0] << 5) | 0x3);  // merge 16-bit intermediate <<  0 into x3
    }
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
        int off = loop_start - (int)(buf->bytesWritten() + 4);  // TODO: check that this is right
        off /= 4;   // bytes -> instructions, still signed
        off = (off & 0x7ffff) << 5;  // 19 bit maximum range (+- 256K instructions)
        splice(buf, 0x54000003 | off); // b.cc loop_start  (cc == "carry clear", unsigned less than)
    }
    static void ret(SkWStream* buf) {
        splice(buf, 0xd65f03c0);  // ret
    }
#else
    static constexpr int kStride = 8;
    static void set_k(SkWStream* buf, const SkSplicer_constants* k) {
        static const uint8_t movabsq_rcx[] = { 0x48, 0xb9 };
        splice(buf, movabsq_rcx);  // movabsq <next 8 bytes>, %rcx
        splice(buf, k);
    }
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

#ifdef IACA_DUMP
    static const uint8_t      ud2[] = { 0x0f, 0x0b };         // undefined... crashes when run
    static const uint8_t     nop3[] = { 0x64, 0x67, 0x90 };   // 3 byte no-op
    static const uint8_t movl_ebx[] = { 0xbb };               // move next 4 bytes into ebx

    static void iaca_start(SkWStream* buf) {
        splice(buf, ud2);
        splice(buf, movl_ebx);
        splice(buf, 111);
        splice(buf, nop3);
    }
    static void iaca_end(SkWStream* buf) {
        splice(buf, movl_ebx);
        splice(buf, 222);
        splice(buf, nop3);
        splice(buf, ud2);
    }
#else
    static void iaca_start(SkWStream*) {}
    static void iaca_end  (SkWStream*) {}
#endif

    // Copy len bytes from src to memory that's executable.  cleanup with cleanup_executable_mem().
    static void* copy_to_executable_mem(const void* src, size_t len) {
        if (src && len) {
            // TODO: w^x
            auto fn = mmap(nullptr, len, PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE, -1, 0);
            return memcpy(fn, src, len);
        }
        return nullptr;
    }
    static void cleanup_executable_mem(void* fn, size_t len) {
        if (fn) {
            munmap(fn, len);
        }
    }

    struct Spliced {

        Spliced(const SkRasterPipeline::Stage* stages, int nstages) {
            // We always create a backup interpreter pipeline,
            //   - to handle any program we can't, and
            //   - to handle the n < kStride tails.
            fBackup     = SkOpts::compile_pipeline(stages, nstages);
            fSplicedLen = 0;
            fSpliced    = nullptr;
            // If we return early anywhere in here, !fSpliced means we'll use fBackup instead.

        #if !defined(__aarch64__)
            // To keep things simple, only one target supported: Haswell+ x86-64.
            if (!SkCpu::Supports(SkCpu::HSW) || sizeof(void*) != 8) {
                return;
            }
        #endif

            SkDynamicMemoryWStream buf;

            // Put the address of kConstants in rcx/x3, Stage argument 4 "k".
            set_k(&buf, &kConstants);

            // We'll loop back to here as long as x<n after x += kStride.
            iaca_start(&buf);
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
                    case SkRasterPipeline::load_8888:    splice(&buf, kSplice_load_8888   ); break;
                    case SkRasterPipeline::store_8888:   splice(&buf, kSplice_store_8888  ); break;
                    case SkRasterPipeline::load_f16:     splice(&buf, kSplice_load_f16    ); break;
                    case SkRasterPipeline::store_f16:    splice(&buf, kSplice_store_f16   ); break;

                    // No joy (probably just not yet implemented).
                    default:
                        //SkDebugf("SkSplicer can't yet handle stage %d.\n", stages[i].stage);
                        return;
                }
            }

            loop(&buf, loop_start);  // Loop back to handle more pixels if not done.
            iaca_end(&buf);
            ret(&buf);  // We're done.

            auto data = buf.detachAsData();
            fSplicedLen = data->size();
            fSpliced    = copy_to_executable_mem(data->data(), fSplicedLen);

        #ifdef IACA_DUMP
            SkFILEWStream(IACA_DUMP).write(data->data(), data->size());
        #endif
        }

        // Spliced is stored in a std::function, so it needs to be copyable.
        Spliced(const Spliced& o) : fBackup    (o.fBackup)
                                  , fSplicedLen(o.fSplicedLen)
                                  , fSpliced   (copy_to_executable_mem(o.fSpliced, fSplicedLen)) {}

        ~Spliced() {
            cleanup_executable_mem(fSpliced, fSplicedLen);
        }

        // Here's where we call fSpliced if we created it, fBackup if not.
        void operator()(size_t x, size_t y, size_t n) const {
            // TODO: The looping logic is probably not correct for n < kStride tails or x != 0.

            size_t body = n/kStride*kStride;   // Largest multiple of kStride (4 or 8) <= n.
            if (fSpliced && body) {            // Can we run fSpliced for at least one kStride?
                // TODO: At some point we will want to pass in y...
                using Fn = void(size_t x, size_t n);
                ((Fn*)fSpliced)(x,body);

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
