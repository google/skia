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

    // Short x86-64 instruction sequences that we'll use as glue to splice together Stages.
    static const uint8_t   vzeroupper[] = { 0xc5, 0xf8, 0x77 };        // clear top half of all ymm
    static const uint8_t          ret[] = { 0xc3 };                    // return
    static const uint8_t  movabsq_rcx[] = { 0x48, 0xb9 };              // move next 8 bytes into rcx
    static const uint8_t  movabsq_rdx[] = { 0x48, 0xba };              // move next 8 bytes into rdx
    static const uint8_t   addq_8_rdi[] = { 0x48, 0x83, 0xc7, 0x08 };  // rdi += 8
    static const uint8_t cmpq_rsi_rdi[] = { 0x48, 0x39, 0xf7 };        // rdi cmp? rsi
    static const uint8_t      jb_near[] = { 0x0f, 0x8c };              // jump relative next 4 bytes
                                                                       //  if cmp set unsigned < bit

    // We do this a lot, so it's nice to infer the correct size.  Works fine with arrays.
    template <typename T>
    void splice(SkWStream* stream, const T& val) {
        stream->write(&val, sizeof(val));
    }

#ifdef IACA_DUMP
    static const uint8_t      ud2[] = { 0x0f, 0x0b };         // undefined... crashes when run
    static const uint8_t     nop3[] = { 0x64, 0x67, 0x90 };   // 3 byte no-op
    static const uint8_t movl_ebx[] = { 0xbb };               // move next 4 bytes into ebx

    static void iaca_start(SkWStream* stream) {
        splice(stream, ud2);
        splice(stream, movl_ebx);
        splice(stream, 111);
        splice(stream, nop3);
    }
    static void iaca_end(SkWStream* stream) {
        splice(stream, movl_ebx);
        splice(stream, 222);
        splice(stream, nop3);
        splice(stream, ud2);
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
            //   - to handle the n < 8 tails.
            fBackup     = SkOpts::compile_pipeline(stages, nstages);
            fSplicedLen = 0;
            fSpliced    = nullptr;
            // If we return early anywhere in here, !fSpliced means we'll use fBackup instead.

            // To keep things simple, only one target supported: Haswell+ x86-64.
            if (!SkCpu::Supports(SkCpu::HSW) || sizeof(void*) != 8) {
                return;
            }

            SkDynamicMemoryWStream buf;

            // Put the address of kConstants in rcx, Stage argument 4 "k".
            splice(&buf, movabsq_rcx);
            splice(&buf, &kConstants);

            // We'll loop back to here as long as x<n after x+=8.
            iaca_start(&buf);
            auto loop_start = buf.bytesWritten();  // Think of this like a label, loop_start:

            for (int i = 0; i < nstages; i++) {
                // If a stage has a context pointer, load it into rdx, Stage argument 3 "ctx".
                if (stages[i].ctx) {
                    splice(&buf, movabsq_rdx);
                    splice(&buf, stages[i].ctx);
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

            // See if we should loop back to handle more pixels.
            splice(&buf, addq_8_rdi);    // x += 8
            splice(&buf, cmpq_rsi_rdi);  // if (x < n)
            splice(&buf, jb_near);       //     goto loop_start;
            splice(&buf, (int)loop_start - (int)(buf.bytesWritten() + 4));
            iaca_end(&buf);

            // Nope!  We're done.
            splice(&buf, vzeroupper);
            splice(&buf, ret);

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
            // TODO: The looping logic is probably not correct for handling n<8 tails.
            if (fSpliced) {
                // TODO: At some point we will want to pass in y...
                using Fn = void(size_t x, size_t n);
                ((Fn*)fSpliced)(x,n);

                // Fall through to fBackup for any n<8 last pixels.
                size_t body = n/8*8;
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
