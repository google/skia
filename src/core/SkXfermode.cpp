/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/SkOnce.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/core/SkXfermodePriv.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrCustomXfermode.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
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

    using INHERITED = SkXfermode;
};

const char* SkBlendMode_Name(SkBlendMode bm) {
    switch (bm) {
        case SkBlendMode::kClear:      return "Clear";
        case SkBlendMode::kSrc:        return "Src";
        case SkBlendMode::kDst:        return "Dst";
        case SkBlendMode::kSrcOver:    return "SrcOver";
        case SkBlendMode::kDstOver:    return "DstOver";
        case SkBlendMode::kSrcIn:      return "SrcIn";
        case SkBlendMode::kDstIn:      return "DstIn";
        case SkBlendMode::kSrcOut:     return "SrcOut";
        case SkBlendMode::kDstOut:     return "DstOut";
        case SkBlendMode::kSrcATop:    return "SrcATop";
        case SkBlendMode::kDstATop:    return "DstATop";
        case SkBlendMode::kXor:        return "Xor";
        case SkBlendMode::kPlus:       return "Plus";
        case SkBlendMode::kModulate:   return "Modulate";
        case SkBlendMode::kScreen:     return "Screen";

        case SkBlendMode::kOverlay:    return "Overlay";
        case SkBlendMode::kDarken:     return "Darken";
        case SkBlendMode::kLighten:    return "Lighten";
        case SkBlendMode::kColorDodge: return "ColorDodge";
        case SkBlendMode::kColorBurn:  return "ColorBurn";
        case SkBlendMode::kHardLight:  return "HardLight";
        case SkBlendMode::kSoftLight:  return "SoftLight";
        case SkBlendMode::kDifference: return "Difference";
        case SkBlendMode::kExclusion:  return "Exclusion";
        case SkBlendMode::kMultiply:   return "Multiply";

        case SkBlendMode::kHue:        return "Hue";
        case SkBlendMode::kSaturation: return "Saturation";
        case SkBlendMode::kColor:      return "Color";
        case SkBlendMode::kLuminosity: return "Luminosity";
    }
    SkUNREACHABLE;
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
