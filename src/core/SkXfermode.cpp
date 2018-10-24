/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlendModePriv.h"
#include "SkColorData.h"
#include "SkMathPriv.h"
#include "SkOnce.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkWriteBuffer.h"
#include "SkXfermodePriv.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "effects/GrCustomXfermode.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

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
                p.append(SkRasterPipeline::scale_u8, &aa_ctx);
            }
            SkBlendMode_AppendStages(fMode, &p);
        } else {
            SkBlendMode_AppendStages(fMode, &p);
            if (aa) {
                p.append(SkRasterPipeline::lerp_u8, &aa_ctx);
            }
        }

        p.append_store(kN32_SkColorType, &dst_ctx);
        p.run(0, 0, count,1);
    }

private:
    const SkBlendMode fMode;

    typedef SkXfermode INHERITED;
};

const char* SkBlendMode_Name(SkBlendMode mode) {
    SkASSERT((unsigned) mode <= (unsigned)SkBlendMode::kLastMode);
    const char* gModeStrings[] = {
        "Clear", "Src", "Dst", "SrcOver", "DstOver", "SrcIn", "DstIn",
        "SrcOut", "DstOut", "SrcATop", "DstATop", "Xor", "Plus",
        "Modulate", "Screen", "Overlay", "Darken", "Lighten", "ColorDodge",
        "ColorBurn", "HardLight", "SoftLight", "Difference", "Exclusion",
        "Multiply", "Hue", "Saturation", "Color",  "Luminosity"
    };
    return gModeStrings[(int)mode];
    static_assert(SK_ARRAY_COUNT(gModeStrings) == (size_t)SkBlendMode::kLastMode + 1, "mode_count");
}

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

    const int COUNT_BLENDMODES = (int)SkBlendMode::kLastMode + 1;

    static SkOnce        once[COUNT_BLENDMODES];
    static SkXfermode* cached[COUNT_BLENDMODES];

    once[(int)mode]([mode] {
        if (auto xfermode = SkOpts::create_xfermode(mode)) {
            cached[(int)mode] = xfermode;
        } else {
            cached[(int)mode] = new SkProcCoeffXfermode(mode);
        }
    });
    return sk_ref_sp(cached[(int)mode]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkXfermode::IsOpaque(SkBlendMode mode, SrcColorOpacity opacityType) {
    SkBlendModeCoeff src, dst;
    if (!SkBlendMode_AsCoeff(mode, &src, &dst)) {
        return false;
    }

    switch (src) {
        case SkBlendModeCoeff::kDA:
        case SkBlendModeCoeff::kDC:
        case SkBlendModeCoeff::kIDA:
        case SkBlendModeCoeff::kIDC:
            return false;
        default:
            break;
    }

    switch (dst) {
        case SkBlendModeCoeff::kZero:
            return true;
        case SkBlendModeCoeff::kISA:
            return kOpaque_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSA:
            return kTransparentBlack_SrcColorOpacity == opacityType ||
            kTransparentAlpha_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSC:
            return kTransparentBlack_SrcColorOpacity == opacityType;
        default:
            return false;
    }
    return false;
}

#if SK_SUPPORT_GPU
const GrXPFactory* SkBlendMode_AsXPFactory(SkBlendMode mode) {
    if (SkBlendMode_AsCoeff(mode, nullptr, nullptr)) {
        const GrXPFactory* result = GrPorterDuffXPFactory::Get(mode);
        SkASSERT(result);
        return result;
    }

    SkASSERT(GrCustomXfermode::IsSupportedMode(mode));
    return GrCustomXfermode::Get(mode);
}
#endif

