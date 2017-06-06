/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlendModePriv.h"
#include "SkRasterPipeline.h"

bool SkBlendMode_CanOverflow(SkBlendMode mode) {
    return mode == SkBlendMode::kPlus;
}

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

void SkBlendMode_AppendStages(SkBlendMode mode, SkRasterPipeline* p) {
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
