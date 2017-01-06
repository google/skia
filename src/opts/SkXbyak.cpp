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

            bool supported = true;
            auto pipeline = std::make_shared<Pipeline>(stages, n, &supported);
            if (supported) {
                return pipeline;
            }
            return nullptr;
        }

        Pipeline(const SkRasterPipeline::Stage* stages, int n, bool* supported) {
            // Set up some register name aliases.
            //auto x = rdi, y = rsi, tail = rdx;
            auto r = ymm0,  g = ymm1,  b = ymm2,  a = ymm3,
                dr = ymm4, dg = ymm5, db = ymm6, da = ymm7;

            Xbyak::Label floatOneStorage;
            vbroadcastss(ymm8, ptr[rip + floatOneStorage]);

            // TODO: set up (x+0.5,y+0.5) in (r,g)
            vxorps(r,r);
            vxorps(g,g);
            vxorps(b,b);
            vxorps(a,a);
            vxorps(dr,dr);
            vxorps(dg,dg);
            vxorps(db,db);
            vxorps(da,da);

            for (int i = 0; i < n; i++) {
                switch(stages[i].stage) {

                    default:
                        *supported = false;
                        return;
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
                while (n >= 8) {
                    call(x,y,0);
                    x += 8;
                    n -= 8;
                }
                if (n) {
                    call(x,y,n);
                }
            };
        }
        SkDebugf("Cannot yet JIT with xbyak:\n");
        this->dump();
        return nullptr;
    } catch(...) {
        return nullptr;
    }
}
