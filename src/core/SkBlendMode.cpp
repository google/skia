/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlendModePriv.h"
#include "SkRasterPipeline.h"

bool SkBlendMode_SupportsCoverageAsAlpha(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kDst:
        case SkBlendMode::kSrcOver:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kXor:
        case SkBlendMode::kPlus:
            return true;
        default:
            break;
    }
    return false;
}

struct CoeffRec {
    SkBlendModeCoeff    fSrc;
    SkBlendModeCoeff    fDst;
};

const CoeffRec gCoeffs[] = {
    { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kZero },
    { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kZero },
    { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kOne  },
    { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kISA  },
    { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kOne  },
    { SkBlendModeCoeff::kDA,      SkBlendModeCoeff::kZero },
    { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kSA   },
    { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kZero },
    { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kISA  },
    { SkBlendModeCoeff::kDA,      SkBlendModeCoeff::kISA  },
    { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kSA   },
    { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kISA  },

    { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kOne  },
    { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kSC   },
    { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kISC  },    // screen
};

bool SkBlendMode_AsCoeff(SkBlendMode mode, SkBlendModeCoeff* src, SkBlendModeCoeff* dst) {
    if (mode > SkBlendMode::kScreen) {
        return false;
    }
    if (src) {
        *src = gCoeffs[static_cast<int>(mode)].fSrc;
    }
    if (dst) {
        *dst = gCoeffs[static_cast<int>(mode)].fDst;
    }
    return true;
}

void SkBlendMode_AppendStagesNoClamp(SkBlendMode mode, SkRasterPipeline* p) {
    auto stage = SkRasterPipeline::srcover;
    switch (mode) {
        case SkBlendMode::kClear:    stage = SkRasterPipeline::clear; break;
        case SkBlendMode::kSrc:      return;  // This stage is a no-op.
        case SkBlendMode::kDst:      stage = SkRasterPipeline::move_dst_src; break;
        case SkBlendMode::kSrcOver:  stage = SkRasterPipeline::srcover; break;
        case SkBlendMode::kDstOver:  stage = SkRasterPipeline::dstover; break;
        case SkBlendMode::kSrcIn:    stage = SkRasterPipeline::srcin; break;
        case SkBlendMode::kDstIn:    stage = SkRasterPipeline::dstin; break;
        case SkBlendMode::kSrcOut:   stage = SkRasterPipeline::srcout; break;
        case SkBlendMode::kDstOut:   stage = SkRasterPipeline::dstout; break;
        case SkBlendMode::kSrcATop:  stage = SkRasterPipeline::srcatop; break;
        case SkBlendMode::kDstATop:  stage = SkRasterPipeline::dstatop; break;
        case SkBlendMode::kXor:      stage = SkRasterPipeline::xor_; break;
        case SkBlendMode::kPlus:     stage = SkRasterPipeline::plus_; break;
        case SkBlendMode::kModulate: stage = SkRasterPipeline::modulate; break;

        case SkBlendMode::kScreen:     stage = SkRasterPipeline::screen; break;
        case SkBlendMode::kOverlay:    stage = SkRasterPipeline::overlay; break;
        case SkBlendMode::kDarken:     stage = SkRasterPipeline::darken; break;
        case SkBlendMode::kLighten:    stage = SkRasterPipeline::lighten; break;
        case SkBlendMode::kColorDodge: stage = SkRasterPipeline::colordodge; break;
        case SkBlendMode::kColorBurn:  stage = SkRasterPipeline::colorburn; break;
        case SkBlendMode::kHardLight:  stage = SkRasterPipeline::hardlight; break;
        case SkBlendMode::kSoftLight:  stage = SkRasterPipeline::softlight; break;
        case SkBlendMode::kDifference: stage = SkRasterPipeline::difference; break;
        case SkBlendMode::kExclusion:  stage = SkRasterPipeline::exclusion; break;
        case SkBlendMode::kMultiply:   stage = SkRasterPipeline::multiply; break;

        case SkBlendMode::kHue:        stage = SkRasterPipeline::hue; break;
        case SkBlendMode::kSaturation: stage = SkRasterPipeline::saturation; break;
        case SkBlendMode::kColor:      stage = SkRasterPipeline::color; break;
        case SkBlendMode::kLuminosity: stage = SkRasterPipeline::luminosity; break;
    }
    p->append(stage);
}

void SkBlendMode_AppendClampIfNeeded(SkBlendMode mode, SkRasterPipeline* p) {
    if (mode == SkBlendMode::kPlus) {
        // Both clamp_a and clamp_1 would preserve premultiplication invariants here,
        // so we pick clamp_1 for being a smidge faster.
        p->append(SkRasterPipeline::clamp_1);
    }
}

SkPM4f SkBlendMode_Apply(SkBlendMode mode, const SkPM4f& src, const SkPM4f& dst) {
    // special-case simple/common modes...
    switch (mode) {
        case SkBlendMode::kClear:   return {{ 0, 0, 0, 0 }};
        case SkBlendMode::kSrc:     return src;
        case SkBlendMode::kDst:     return dst;
        case SkBlendMode::kSrcOver:
            return SkPM4f::From4f(src.to4f() + dst.to4f() * Sk4f(1 - src.a()));
        default:
            break;
    }

    SkRasterPipeline_<256> p;
    SkPM4f                 src_storage = src,
                           dst_storage = dst,
                           result_storage,
                           *src_ctx = &src_storage,
                           *dst_ctx = &dst_storage,
                           *res_ctx = &result_storage;

    p.append(SkRasterPipeline::load_f32, &dst_ctx);
    p.append(SkRasterPipeline::move_src_dst);
    p.append(SkRasterPipeline::load_f32, &src_ctx);
    SkBlendMode_AppendStages(mode, &p);
    p.append(SkRasterPipeline::store_f32, &res_ctx);
    p.run(0, 0, 1);
    return result_storage;
}
