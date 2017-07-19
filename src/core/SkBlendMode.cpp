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
    switch (mode) {
        case SkBlendMode::kClear:    p->append_clear(); break;
        case SkBlendMode::kSrc:      break;  // This stage is a no-op.
        case SkBlendMode::kDst:      p->append_move_dst_src(); break;
        case SkBlendMode::kSrcOver:  p->append_srcover(); break;
        case SkBlendMode::kDstOver:  p->append_dstover(); break;
        case SkBlendMode::kSrcIn:    p->append_srcin(); break;
        case SkBlendMode::kDstIn:    p->append_dstin(); break;
        case SkBlendMode::kSrcOut:   p->append_srcout(); break;
        case SkBlendMode::kDstOut:   p->append_dstout(); break;
        case SkBlendMode::kSrcATop:  p->append_srcatop(); break;
        case SkBlendMode::kDstATop:  p->append_dstatop(); break;
        case SkBlendMode::kXor:      p->append_xor_(); break;
        case SkBlendMode::kPlus:     p->append_plus_(); break;
        case SkBlendMode::kModulate: p->append_modulate(); break;

        case SkBlendMode::kScreen:     p->append_screen(); break;
        case SkBlendMode::kOverlay:    p->append_overlay(); break;
        case SkBlendMode::kDarken:     p->append_darken(); break;
        case SkBlendMode::kLighten:    p->append_lighten(); break;
        case SkBlendMode::kColorDodge: p->append_colordodge(); break;
        case SkBlendMode::kColorBurn:  p->append_colorburn(); break;
        case SkBlendMode::kHardLight:  p->append_hardlight(); break;
        case SkBlendMode::kSoftLight:  p->append_softlight(); break;
        case SkBlendMode::kDifference: p->append_difference(); break;
        case SkBlendMode::kExclusion:  p->append_exclusion(); break;
        case SkBlendMode::kMultiply:   p->append_multiply(); break;

        case SkBlendMode::kHue:        p->append_hue(); break;
        case SkBlendMode::kSaturation: p->append_saturation(); break;
        case SkBlendMode::kColor:      p->append_color(); break;
        case SkBlendMode::kLuminosity: p->append_luminosity(); break;
    }
}

void SkBlendMode_AppendClampIfNeeded(SkBlendMode mode, SkRasterPipeline* p) {
    if (mode == SkBlendMode::kPlus) {
        // Both clamp_a and clamp_1 would preserve premultiplication invariants here,
        // so we pick clamp_1 for being a smidge faster.
        p->append_clamp_1();
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
                           res_storage;

    float *src_ctx = src_storage.fVec,
          *dst_ctx = dst_storage.fVec,
          *res_ctx = res_storage.fVec;

    p.append_load_f32    ((const float**)&src_ctx);
    p.append_load_f32_dst((const float**)&dst_ctx);
    SkBlendMode_AppendStages(mode, &p);
    p.append_store_f32   (&res_ctx);
    p.run(0, 0, 1);
    return res_storage;
}
