/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/private/SkColorData.h"
#include "src/base/SkVx.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <optional>

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

bool SkBlendMode_AsCoeff(SkBlendMode mode, SkBlendModeCoeff* src, SkBlendModeCoeff* dst) {
    struct CoeffRec {
        SkBlendModeCoeff    fSrc;
        SkBlendModeCoeff    fDst;
    };

    static constexpr CoeffRec kCoeffs[] = {
        // For Porter-Duff blend functions, color = src * src coeff + dst * dst coeff
        // src coeff                  dst coeff                     blend func
        // ----------------------     -----------------------       ----------
        { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kZero }, // clear
        { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kZero }, // src
        { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kOne  }, // dst
        { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kISA  }, // src-over
        { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kOne  }, // dst-over
        { SkBlendModeCoeff::kDA,      SkBlendModeCoeff::kZero }, // src-in
        { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kSA   }, // dst-in
        { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kZero }, // src-out
        { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kISA  }, // dst-out
        { SkBlendModeCoeff::kDA,      SkBlendModeCoeff::kISA  }, // src-atop
        { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kSA   }, // dst-atop
        { SkBlendModeCoeff::kIDA,     SkBlendModeCoeff::kISA  }, // xor

        { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kOne  }, // plus
        { SkBlendModeCoeff::kZero,    SkBlendModeCoeff::kSC   }, // modulate
        { SkBlendModeCoeff::kOne,     SkBlendModeCoeff::kISC  }, // screen
    };

    if (mode > SkBlendMode::kScreen) {
        return false;
    }
    if (src) {
        *src = kCoeffs[static_cast<int>(mode)].fSrc;
    }
    if (dst) {
        *dst = kCoeffs[static_cast<int>(mode)].fDst;
    }
    return true;
}

void SkBlendMode_AppendStages(SkBlendMode mode, SkRasterPipeline* p) {
    auto stage = SkRasterPipelineOp::srcover;
    switch (mode) {
        case SkBlendMode::kClear:    stage = SkRasterPipelineOp::clear; break;
        case SkBlendMode::kSrc:      return;  // This stage is a no-op.
        case SkBlendMode::kDst:      stage = SkRasterPipelineOp::move_dst_src; break;
        case SkBlendMode::kSrcOver:  stage = SkRasterPipelineOp::srcover; break;
        case SkBlendMode::kDstOver:  stage = SkRasterPipelineOp::dstover; break;
        case SkBlendMode::kSrcIn:    stage = SkRasterPipelineOp::srcin; break;
        case SkBlendMode::kDstIn:    stage = SkRasterPipelineOp::dstin; break;
        case SkBlendMode::kSrcOut:   stage = SkRasterPipelineOp::srcout; break;
        case SkBlendMode::kDstOut:   stage = SkRasterPipelineOp::dstout; break;
        case SkBlendMode::kSrcATop:  stage = SkRasterPipelineOp::srcatop; break;
        case SkBlendMode::kDstATop:  stage = SkRasterPipelineOp::dstatop; break;
        case SkBlendMode::kXor:      stage = SkRasterPipelineOp::xor_; break;
        case SkBlendMode::kPlus:     stage = SkRasterPipelineOp::plus_; break;
        case SkBlendMode::kModulate: stage = SkRasterPipelineOp::modulate; break;

        case SkBlendMode::kScreen:     stage = SkRasterPipelineOp::screen; break;
        case SkBlendMode::kOverlay:    stage = SkRasterPipelineOp::overlay; break;
        case SkBlendMode::kDarken:     stage = SkRasterPipelineOp::darken; break;
        case SkBlendMode::kLighten:    stage = SkRasterPipelineOp::lighten; break;
        case SkBlendMode::kColorDodge: stage = SkRasterPipelineOp::colordodge; break;
        case SkBlendMode::kColorBurn:  stage = SkRasterPipelineOp::colorburn; break;
        case SkBlendMode::kHardLight:  stage = SkRasterPipelineOp::hardlight; break;
        case SkBlendMode::kSoftLight:  stage = SkRasterPipelineOp::softlight; break;
        case SkBlendMode::kDifference: stage = SkRasterPipelineOp::difference; break;
        case SkBlendMode::kExclusion:  stage = SkRasterPipelineOp::exclusion; break;
        case SkBlendMode::kMultiply:   stage = SkRasterPipelineOp::multiply; break;

        case SkBlendMode::kHue:        stage = SkRasterPipelineOp::hue; break;
        case SkBlendMode::kSaturation: stage = SkRasterPipelineOp::saturation; break;
        case SkBlendMode::kColor:      stage = SkRasterPipelineOp::color; break;
        case SkBlendMode::kLuminosity: stage = SkRasterPipelineOp::luminosity; break;
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
            SkPMColor4f r;
            (skvx::float4::Load(src.vec()) + skvx::float4::Load(dst.vec()) * (1-src.fA)).store(&r);
            return r;
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

    p.append(SkRasterPipelineOp::load_f32, &dst_ctx);
    p.append(SkRasterPipelineOp::move_src_dst);
    p.append(SkRasterPipelineOp::load_f32, &src_ctx);
    SkBlendMode_AppendStages(mode, &p);
    p.append(SkRasterPipelineOp::store_f32, &res_ctx);
    p.run(0,0, 1,1);
    return res_storage;
}

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

static bool just_solid_color(const SkPaint& p) {
    return SK_AlphaOPAQUE == p.getAlpha() && !p.getColorFilter() && !p.getShader();
}

SkBlendFastPath CheckFastPath(const SkPaint& paint, bool dstIsOpaque) {
    const auto bm = paint.asBlendMode();
    if (!bm) {
        return SkBlendFastPath::kNormal;
    }
    switch (bm.value()) {
        case SkBlendMode::kSrcOver:
            return SkBlendFastPath::kSrcOver;
        case SkBlendMode::kSrc:
            if (just_solid_color(paint)) {
                return SkBlendFastPath::kSrcOver;
            }
            return SkBlendFastPath::kNormal;
        case SkBlendMode::kDst:
            return SkBlendFastPath::kSkipDrawing;
        case SkBlendMode::kDstOver:
            if (dstIsOpaque) {
                return SkBlendFastPath::kSkipDrawing;
            }
            return SkBlendFastPath::kNormal;
        case SkBlendMode::kSrcIn:
            if (dstIsOpaque && just_solid_color(paint)) {
                return SkBlendFastPath::kSrcOver;
            }
            return SkBlendFastPath::kNormal;
        case SkBlendMode::kDstIn:
            if (just_solid_color(paint)) {
                return SkBlendFastPath::kSkipDrawing;
            }
            return SkBlendFastPath::kNormal;
        default:
            return SkBlendFastPath::kNormal;
    }
}
