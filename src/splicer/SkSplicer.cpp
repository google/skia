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

#include "SkSplicer_shared.h"
#include "SkSplicer_generated.h"


namespace {

    static const SkSplicer_constants kConstants = { 0x000000ff, 1.0f, 255.0f, 1/255.0f };

    static const uint16_t movabsq_rcx = 0xb948;
    static const uint16_t movabsq_rdx = 0xba48;
    static const uint8_t          ret = 0xc3;

    template <typename T>
    void splice(SkWStream* stream, const T& val) {
        stream->write(&val, sizeof(val));
    }

    static void* copy_from(const void* src, size_t len) {
        // TODO: w^x
        auto fn = mmap(nullptr, len, PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE, -1, 0);
        return memcpy(fn, src, len);
    }

    struct Spliced {

        Spliced(const SkRasterPipeline::Stage* stages, int nstages) {
            fBackup = SkOpts::compile_pipeline(stages, nstages);
            if (!SkCpu::Supports(SkCpu::HSW) || sizeof(void*) != 8) {
                return;
            }

            SkDynamicMemoryWStream buf;
            splice(&buf, movabsq_rcx);
            splice(&buf, &kConstants);

            for (int i = 0; i < nstages; i++) {
                if (stages[i].ctx) {
                    splice(&buf, movabsq_rdx);
                    splice(&buf, stages[i].ctx);
                }
                switch(stages[i].stage) {
                    case SkRasterPipeline::clear:      splice(&buf, kSplice_clear     ); break;
                    case SkRasterPipeline::plus_:      splice(&buf, kSplice_plus      ); break;
                    case SkRasterPipeline::srcover:    splice(&buf, kSplice_srcover   ); break;
                    case SkRasterPipeline::clamp_0:    splice(&buf, kSplice_clamp_0   ); break;
                    case SkRasterPipeline::clamp_1:    splice(&buf, kSplice_clamp_1   ); break;
                    case SkRasterPipeline::clamp_a:    splice(&buf, kSplice_clamp_a   ); break;
                    case SkRasterPipeline::swap:       splice(&buf, kSplice_swap      ); break;
                    case SkRasterPipeline::premul:     splice(&buf, kSplice_premul    ); break;
                    case SkRasterPipeline::unpremul:   splice(&buf, kSplice_unpremul  ); break;
                    case SkRasterPipeline::load_8888:  splice(&buf, kSplice_load_8888 ); break;
                    case SkRasterPipeline::store_8888: splice(&buf, kSplice_store_8888); break;
                    case SkRasterPipeline::load_f16:   splice(&buf, kSplice_load_f16  ); break;
                    case SkRasterPipeline::store_f16:  splice(&buf, kSplice_store_f16 ); break;

                    default: return;
                }
            }
            splice(&buf, kSplice_done);
            splice(&buf, ret);

            auto data = buf.detachAsData();
            fSplicedLen = data->size();
            fSpliced    = copy_from(data->data(), fSplicedLen);
        }

        Spliced(const Spliced& o) : fBackup    (o.fBackup)
                                  , fSplicedLen(o.fSplicedLen)
                                  , fSpliced   (copy_from(o.fSpliced, fSplicedLen)) {}

        ~Spliced() {
            if (fSpliced) {
                munmap((void*)fSpliced, fSplicedLen);
            }
        }

        void operator()(size_t x, size_t y, size_t n) const {
            while (fSpliced && n >= 8) {
                using Fn = void(size_t x, size_t y);
                ((Fn*)fSpliced)(x,y);
                x += 8;
                n -= 8;
            }
            fBackup(x,y,n);
        }

        std::function<void(size_t, size_t, size_t)> fBackup;
        size_t fSplicedLen = 0;
        void*  fSpliced    = nullptr;
    };

}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::jit() const {
    return Spliced(fStages.data(), SkToInt(fStages.size()));
}
