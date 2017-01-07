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

        Pipeline(const SkRasterPipeline::Stage* stages, int nstages, bool* supported) {
            // Set up some register name aliases.
            // y = rsi
            auto x = rdi, n = rdx;
            auto r = ymm0,  g = ymm1,  b = ymm2,  a = ymm3,
                dr = ymm4, dg = ymm5, db = ymm6, da = ymm7;

            Xbyak::Label floatOneStorage,
                         start;

            // TODO: set up (x+0.5,y+0.5) in (r,g)
            vxorps(r,r);
            vxorps(g,g);
            vxorps(b,b);
            vxorps(a,a);
            vxorps(dr,dr);
            vxorps(dg,dg);
            vxorps(db,db);
            vxorps(da,da);

            auto zero = ymm14,
                  one = ymm15;
            vxorps(zero, zero);
            vbroadcastss(one, ptr[rip + floatOneStorage]);

            L(start);
            //trap();
            for (int i = 0; i < nstages; i++) {
                switch(stages[i].stage) {
                    case SkRasterPipeline::load_f16:
                        mov(rax, (size_t)stages[i].ctx);
                        mov(rax, ptr[rax]);

                        vmovdqu(xmm0, ptr[rax+x*8+ 0]);
                        vmovdqu(xmm1, ptr[rax+x*8+16]);
                        vmovdqu(xmm2, ptr[rax+x*8+32]);
                        vmovdqu(xmm3, ptr[rax+x*8+48]);

                        vpunpcklwd(xmm8, xmm1, xmm0); vpunpckhwd(xmm0 , xmm1, xmm0);
                        vpunpcklwd(xmm1, xmm3, xmm2); vpunpckhwd(xmm2 , xmm3, xmm2);
                        vpunpcklwd(xmm9, xmm0, xmm8); vpunpckhwd(xmm8 , xmm0, xmm8);
                        vpunpcklwd(xmm3, xmm2, xmm1); vpunpckhwd(xmm10, xmm2, xmm1);

                        vpunpcklqdq(xmm0,  xmm3, xmm9); vcvtph2ps(ymm0, xmm0);
                        vpunpckhqdq(xmm1,  xmm3, xmm9); vcvtph2ps(ymm1, xmm1);
                        vpunpcklqdq(xmm2, xmm10, xmm8); vcvtph2ps(ymm2, xmm2);
                        vpunpckhqdq(xmm3, xmm10, xmm8); vcvtph2ps(ymm3, xmm3);
                        break;

                    case SkRasterPipeline::unpremul:
                        vcmpeqps(ymm10, zero, a);              // ymm10: a == 0
                        vdivps(ymm11, one, a);                 // ymm11: 1/a
                        vblendvps(ymm10, ymm10, zero, ymm11);  // ymm10: (a==0) ? 0 : 1/a
                        vmulps(r, r, ymm10);
                        vmulps(g, g, ymm10);
                        vmulps(b, b, ymm10);
                        break;

                    case SkRasterPipeline::store_f16:
                        mov(rax, (size_t)stages[i].ctx);
                        mov(rax, ptr[rax]);

                        vcvtps2ph(xmm8 , ymm0, 4);
                        vcvtps2ph(xmm9 , ymm1, 4);
                        vcvtps2ph(xmm10, ymm2, 4);
                        vcvtps2ph(xmm11, ymm3, 4);

                        vpunpcklwd(xmm12, xmm9 , xmm8 );
                        vpunpckhwd(xmm8 , xmm9 , xmm8 );
                        vpunpcklwd(xmm9 , xmm11, xmm10);
                        vpunpckhwd(xmm10, xmm11, xmm10);

                        vpunpckldq(xmm11, xmm9 , xmm12); vmovdqu(ptr[rax+x*8+ 0], xmm11);
                        vpunpckhdq(xmm9 , xmm9 , xmm12); vmovdqu(ptr[rax+x*8+16], xmm9 );
                        vpunpckldq(xmm9 , xmm10, xmm8 ); vmovdqu(ptr[rax+x*8+32], xmm9 );
                        vpunpckhdq(xmm8 , xmm10, xmm8 ); vmovdqu(ptr[rax+x*8+48], xmm8 );
                        break;

                    default:
                        *supported = false;
                        return;
                }
            }
            add(x, 8);
            cmp(x, n);
            jl(start);

            vzeroupper();
            ret();
            L(floatOneStorage); df(1.0f);
        }

        void df(float f) {
            union { float f; uint32_t x; } pun = {f};
            dd(pun.x);
        }
        void dp(void* p) {
            union { void* p; uint64_t x; } pun = {p};
            dq(pun.x);
        }

        void trap() {
            dw(0x0b0f);
        }
    };

}  // namespace

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::jit() const {
    try {
        if (auto pipeline = Pipeline::Create(fStages.data(), SkToInt(fStages.size()))) {
            return [pipeline] (size_t x, size_t y, size_t n) {
                auto call = pipeline->getCode<void(*)(size_t, size_t, size_t)>();
                //printf("fn addr: %p\n", (void*)call);
                call(x,y,n);
            };
        }
#if 0
        SkDebugf("Cannot yet JIT with xbyak:\n");
        this->dump();
#endif
        return nullptr;
    } catch(...) {
        SkDebugf("caught exception\n");
        return nullptr;
    }
}
