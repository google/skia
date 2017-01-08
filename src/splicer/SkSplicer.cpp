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


namespace {

    static const uint16_t movabsq = 0xba48;
    static const uint8_t      ret = 0xc3;

    template <typename T>
    void splice(SkWStream* stream, const T& val) {
        stream->write(&val, sizeof(val));
    }

    using Fn = void(size_t x, size_t y);

    static Fn* copy_from(const void* src, size_t len) {
        // TODO: w^x
        auto fn = mmap(nullptr, len, PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE, -1, 0);
        memcpy(fn, src, len);
        return (Fn*)fn;
    }

    struct Spliced {

        Spliced(const SkRasterPipeline::Stage* stages, int nstages) {
            fBackup = SkOpts::compile_pipeline(stages, nstages);

            SkDynamicMemoryWStream buf;

        if (false)
            for (int i = 0; i < nstages; i++) {
                if (stages[i].ctx) {
                    splice(&buf, movabsq);
                    splice(&buf, stages[i].ctx);
                }
                switch(stages[i].stage) {
                    case SkRasterPipeline::load_f16:  splice(&buf, kSplice_load_f16 ); break;
                    case SkRasterPipeline::store_f16: splice(&buf, kSplice_store_f16); break;
                    case SkRasterPipeline::unpremul:  splice(&buf, kSplice_unpremul ); break;

                    default: return;
                }
            }
            splice(&buf, ret);

            auto data = buf.detachAsData();
            fSpliced    = copy_from(data->data(), data->size());
            fSplicedLen = data->size();
        }

        Spliced(const Spliced& o) : fSpliced   (copy_from((const void*)o.fSpliced, o.fSplicedLen))
                                  , fSplicedLen(o.fSplicedLen)
                                  , fBackup    (o.fBackup) {}

        ~Spliced() {
            if (fSpliced) {
                munmap((void*)fSpliced, fSplicedLen);
            }
        }

        void operator()(size_t x, size_t y, size_t n) const {
            while (fSpliced && n >= 8) {
                fSpliced(x,y);
                x += 8;
                n -= 8;
            }
            fBackup(x,y,n);
        }

        Fn*    fSpliced    = nullptr;
        size_t fSplicedLen = 0;
        std::function<void(size_t, size_t, size_t)> fBackup;
    };

}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::jit() const {
    if (!SkCpu::Supports(SkCpu::HSW) || sizeof(void*) != 8) {
        return nullptr;
    }
    return Spliced(fStages.data(), SkToInt(fStages.size()));
}
