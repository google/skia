/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCpu.h"
#include "SkRasterPipeline.h"
#include <memory>

#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wduplicate-enum"
#endif
#define XBYAK_NO_OP_NAMES   // xor(), not(), etc. -> xor_(), not_(), etc.
#include "xbyak/xbyak.h"

namespace {

    struct Pipeline : public Xbyak::CodeGenerator {

        static std::shared_ptr<Pipeline> Create(const SkRasterPipeline::Stage* stages, int n) {
            if (!SkCpu::Supports(SkCpu::HSW)) {
                // TODO: other targets?
                return nullptr;
            }

            for (int i = 0; i < n; i++) {
                switch(stages[i].stage) {

                    default:
                        return nullptr;
                }
            }

            return std::make_shared<Pipeline>(stages, n);
        }

        Pipeline(const SkRasterPipeline::Stage* stages, int n) {
        #if 0
            // Set up some register name aliases.
            auto r = ymm0,  g = ymm1,  b = ymm2,  a = ymm3,
                dr = ymm4, dg = ymm5, db = ymm6, da = ymm7;
        #endif

            Xbyak::Label floatOneStorage;
            vbroadcastss(ymm8, ptr[rip + floatOneStorage]);

            for (int i = 0; i < n; i++) {
                switch(stages[i].stage) {

                    default: SkASSERT(false);
                }
            }

            ret();
            L(floatOneStorage); df(1.0f);
        }

        void df(float f) {
            union { float f; uint32_t x; } pun = {f};
            dd(pun.x);
        }
    };

}  // namespace

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::jit() const {
    try {
        if (auto pipeline = Pipeline::Create(fStages.data(), SkToInt(fStages.size()))) {
            return [pipeline] (size_t x, size_t y, size_t n) {
                auto call = pipeline->getCode<void(*)(size_t, size_t, size_t)>();
                call(x,y,n);
            };
        }
        SkDebugf("Cannot yet JIT with xbyak:\n");
        this->dump();
        return nullptr;
    } catch(...) {
        return nullptr;
    }
}
