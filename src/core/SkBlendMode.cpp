/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlendModePriv.h"
#include "src/core/SkCoverageModePriv.h"
#include "src/core/SkRasterPipeline.h"

bool SkBlendMode_ShouldPreScaleCoverage(SkBlendMode mode, bool rgb_coverage) {
    // The most important things we do here are:
    //   1) never pre-scale with rgb coverage if the blend mode involves a source-alpha term;
    //   2) always pre-scale Plus.
    //
    // When we pre-scale with rgb coverage, we scale each of source r,g,b, with a distinct value,
    // and source alpha with one of those three values.  This process destructively updates the
    // source-alpha term, so we can't evaluate blend modes that need its original value.
    //
    // Plus always requires pre-scaling as a specific quirk of its implementation in
    // SkRasterPipeline.  This lets us put the clamp inside the blend mode itself rather
    // than as a separate stage that'd come after the lerp.
    //
    // This function is a finer-grained breakdown of SkBlendMode_SupportsCoverageAsAlpha().
    switch (mode) {
        case SkBlendMode::kDst:        // d              --> no sa term, ok!
        case SkBlendMode::kDstOver:    // d + s*inv(da)  --> no sa term, ok!
        case SkBlendMode::kPlus:       // clamp(s+d)     --> no sa term, ok!
            return true;

        case SkBlendMode::kDstOut:     // d * inv(sa)
        case SkBlendMode::kSrcATop:    // s*da + d*inv(sa)
        case SkBlendMode::kSrcOver:    // s + d*inv(sa)
        case SkBlendMode::kXor:        // s*inv(da) + d*inv(sa)
            return !rgb_coverage;

        default: break;
    }
    return false;
}

// Users of this function may want to switch to the rgb-coverage aware version above.
bool SkBlendMode_SupportsCoverageAsAlpha(SkBlendMode mode) {
    return SkBlendMode_ShouldPreScaleCoverage(mode, false);
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

SkPMColor4f SkBlendMode_Apply(SkBlendMode mode, const SkPMColor4f& src, const SkPMColor4f& dst) {
    // special-case simple/common modes...
    switch (mode) {
        case SkBlendMode::kClear:   return SK_PMColor4fTRANSPARENT;
        case SkBlendMode::kSrc:     return src;
        case SkBlendMode::kDst:     return dst;
        case SkBlendMode::kSrcOver: {
            Sk4f r = Sk4f::Load(src.vec()) + Sk4f::Load(dst.vec()) * Sk4f(1 - src.fA);
            return { r[0], r[1], r[2], r[3] };
        }
        default:
            break;
    }

    SkRasterPipeline_<256> p;
    SkPMColor4f            src_storage = src,
                           dst_storage = dst,
                           res_storage;
    SkRasterPipeline_MemoryCtx src_ctx = { &src_storage, 0 },
                               dst_ctx = { &dst_storage, 0 },
                               res_ctx = { &res_storage, 0 };

    p.append(SkRasterPipeline::load_f32, &dst_ctx);
    p.append(SkRasterPipeline::move_src_dst);
    p.append(SkRasterPipeline::load_f32, &src_ctx);
    SkBlendMode_AppendStages(mode, &p);
    p.append(SkRasterPipeline::store_f32, &res_ctx);
    p.run(0,0, 1,1);
    return res_storage;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const SkBlendMode gUncorrelatedCoverageToBlend[] = {
    SkBlendMode::kSrcOver,  // or DstOver
    SkBlendMode::kSrcIn,    // or kDstIn
    SkBlendMode::kSrcOut,
    SkBlendMode::kDstOut,
    SkBlendMode::kXor,
};

SkBlendMode SkUncorrelatedCoverageModeToBlendMode(SkCoverageMode cm) {
    unsigned index = static_cast<unsigned>(cm);
    SkASSERT(index < SK_ARRAY_COUNT(gUncorrelatedCoverageToBlend));
    return gUncorrelatedCoverageToBlend[index];
}
