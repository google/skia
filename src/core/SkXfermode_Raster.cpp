/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorType.h"
#include "include/private/base/SkOnce.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkXfermodePriv.h"

class SkProcCoeffXfermode : public SkXfermode {
public:
    SkProcCoeffXfermode(SkBlendMode mode) : fMode(mode) {}

    void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                const SkAlpha aa[]) const override {
        SkASSERT(dst && src && count >= 0);

        SkRasterPipeline_<256> p;

        SkRasterPipeline_MemoryCtx dst_ctx = { (void*)dst, 0 },
                                   src_ctx = { (void*)src, 0 },
                                    aa_ctx = { (void*)aa,  0 };

        p.append_load    (kN32_SkColorType, &src_ctx);
        p.append_load_dst(kN32_SkColorType, &dst_ctx);

        if (SkBlendMode_ShouldPreScaleCoverage(fMode, /*rgb_coverage=*/false)) {
            if (aa) {
                p.append(SkRasterPipelineOp::scale_u8, &aa_ctx);
            }
            SkBlendMode_AppendStages(fMode, &p);
        } else {
            SkBlendMode_AppendStages(fMode, &p);
            if (aa) {
                p.append(SkRasterPipelineOp::lerp_u8, &aa_ctx);
            }
        }

        p.append_store(kN32_SkColorType, &dst_ctx);
        p.run(0, 0, count,1);
    }

private:
    const SkBlendMode fMode;

    using INHERITED = SkXfermode;
};

sk_sp<SkXfermode> SkXfermode::Make(SkBlendMode mode) {
    if ((unsigned)mode > (unsigned)SkBlendMode::kLastMode) {
        // report error
        return nullptr;
    }

    // Skia's "default" mode is srcover. nullptr in SkPaint is interpreted as srcover
    // so we can just return nullptr from the factory.
    if (SkBlendMode::kSrcOver == mode) {
        return nullptr;
    }

    static SkOnce        once[kSkBlendModeCount];
    static SkXfermode* cached[kSkBlendModeCount];

    once[(int)mode]([mode] {
        if (auto xfermode = SkOpts::create_xfermode(mode)) {
            cached[(int)mode] = xfermode;
        } else {
            cached[(int)mode] = new SkProcCoeffXfermode(mode);
        }
    });
    return sk_ref_sp(cached[(int)mode]);
}

