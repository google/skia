/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform_A2B.h"

#include "SkColorPriv.h"
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformPriv.h"
#include "SkMakeUnique.h"
#include "SkNx.h"
#include "SkSRGB.h"
#include "SkTypes.h"

#include <algorithm>

#define AI SK_ALWAYS_INLINE

bool SkColorSpaceXform_A2B::onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat,
                                    const void* src, int count, SkAlphaType alphaType) const {
    SkRasterPipeline pipeline;
    switch (srcFormat) {
        case kBGRA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::load_8888, &src);
            pipeline.append(SkRasterPipeline::swap_rb);
            break;
        case kRGBA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::load_8888, &src);
            break;
        default:
            SkCSXformPrintf("F16/F32 source color format not supported\n");
            return false;
    }

    pipeline.extend(fElementsPipeline);

    if (kPremul_SkAlphaType == alphaType) {
        pipeline.append(SkRasterPipeline::premul);
    }

    switch (dstFormat) {
        case kBGRA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::swap_rb);
            pipeline.append(SkRasterPipeline::store_8888, &dst);
            break;
        case kRGBA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::store_8888, &dst);
            break;
        case kRGBA_F16_ColorFormat:
            if (!fLinearDstGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::store_f16, &dst);
            break;
        case kRGBA_F32_ColorFormat:
            if (!fLinearDstGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::store_f32, &dst);
            break;
    }
    pipeline.run(0,0, count);

    return true;
}

static inline SkColorSpaceTransferFn value_to_parametric(float exp) {
    return {exp, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f};
}

static inline SkColorSpaceTransferFn gammanamed_to_parametric(SkGammaNamed gammaNamed) {
    switch (gammaNamed) {
        case kLinear_SkGammaNamed:
            return value_to_parametric(1.f);
        case kSRGB_SkGammaNamed:
            return {2.4f, (1.f / 1.055f), (0.055f / 1.055f), 0.f, 0.04045f, (1.f / 12.92f), 0.f};
        case k2Dot2Curve_SkGammaNamed:
            return value_to_parametric(2.2f);
        default:
            SkASSERT(false);
            return {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
    }
}

static inline SkColorSpaceTransferFn gamma_to_parametric(const SkGammas& gammas, int channel) {
    switch (gammas.type(channel)) {
        case SkGammas::Type::kNamed_Type:
            return gammanamed_to_parametric(gammas.data(channel).fNamed);
        case SkGammas::Type::kValue_Type:
            return value_to_parametric(gammas.data(channel).fValue);
        case SkGammas::Type::kParam_Type:
            return gammas.params(channel);
        default:
            SkASSERT(false);
            return {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
    }
}
static inline SkColorSpaceTransferFn invert_parametric(const SkColorSpaceTransferFn& fn) {
    // Original equation is:       y = (ax + b)^g + c   for x >= d
    //                             y = ex + f           otherwise
    //
    // so 1st inverse is:          (y - c)^(1/g) = ax + b
    //                             x = ((y - c)^(1/g) - b) / a
    //
    // which can be re-written as: x = (1/a)(y - c)^(1/g) - b/a
    //                             x = ((1/a)^g)^(1/g) * (y - c)^(1/g) - b/a
    //                             x = ([(1/a)^g]y + [-((1/a)^g)c]) ^ [1/g] + [-b/a]
    //
    // and 2nd inverse is:         x = (y - f) / e
    // which can be re-written as: x = [1/e]y + [-f/e]
    //
    // and now both can be expressed in terms of the same parametric form as the
    // original - parameters are enclosed in square brackets.

    // find inverse for linear segment (if possible)
    float e, f;
    if (0.f == fn.fE) {
        // otherwise assume it should be 0 as it is the lower segment
        // as y = f is a constant function
        e = 0.f;
        f = 0.f;
    } else {
        e = 1.f / fn.fE;
        f = -fn.fF / fn.fE;
    }
    // find inverse for the other segment (if possible)
    float g, a, b, c;
    if (0.f == fn.fA || 0.f == fn.fG) {
        // otherwise assume it should be 1 as it is the top segment
        // as you can't invert the constant functions y = b^g + c, or y = 1 + c
        g = 1.f;
        a = 0.f;
        b = 0.f;
        c = 1.f;
    } else {
        g = 1.f / fn.fG;
        a = powf(1.f / fn.fA, fn.fG);
        b = -a * fn.fC;
        c = -fn.fB / fn.fA;
    }
    const float d = fn.fE * fn.fD + fn.fF;
    return {g, a, b, c, d, e, f};
}

SkColorSpaceXform_A2B::SkColorSpaceXform_A2B(SkColorSpace_A2B* srcSpace,
                                             SkColorSpace_XYZ* dstSpace)
    : fLinearDstGamma(kLinear_SkGammaNamed == dstSpace->gammaNamed()) {
#if (SkCSXformPrintfDefined)
    static const char* debugGammaNamed[4] = {
        "Linear", "SRGB", "2.2", "NonStandard"
    };
    static const char* debugGammas[5] = {
        "None", "Named", "Value", "Table", "Param"
    };
#endif
    int currentChannels = -1;
    switch (srcSpace->inputColorFormat()) {
        case SkColorSpace_Base::InputColorFormat::kRGB:
            currentChannels = 3;
            break;
        case SkColorSpace_Base::InputColorFormat::kCMYK:
            currentChannels = 4;
            // CMYK images from JPEGs (the only format that supports it) are actually
            // inverted CMYK, so we need to invert every channel.
            // TransferFn is y = -x + 1 for x < 1.f, otherwise 0x + 0, ie y = 1 - x for x in [0,1]
            this->addTransferFns({1.f, 0.f, 0.f, 0.f, 1.f, -1.f, 1.f}, 4);
            break;
        case SkColorSpace_Base::InputColorFormat::kGray:
            currentChannels = 1;
            break;
        default:
            SkASSERT(false);
    }
    // add in all input color space -> PCS xforms
    for (int i = 0; i < srcSpace->count(); ++i) {
        const SkColorSpace_A2B::Element& e = srcSpace->element(i);
        SkASSERT(e.inputChannels() == currentChannels);
        currentChannels = e.outputChannels();
        switch (e.type()) {
            case SkColorSpace_A2B::Element::Type::kGammaNamed:
                if (kLinear_SkGammaNamed != e.gammaNamed()) {
                    SkCSXformPrintf("Gamma stage added: %s\n",
                                    debugGammaNamed[(int)e.gammaNamed()]);
                    SkColorSpaceTransferFn fn = gammanamed_to_parametric(e.gammaNamed());
                    this->addTransferFns(fn, currentChannels);
                }
                break;
            case SkColorSpace_A2B::Element::Type::kGammas: {
                const SkGammas& gammas = e.gammas();
                SkCSXformPrintf("Gamma stage added:");
                for (int channel = 0; channel < gammas.channels(); ++channel) {
                    SkCSXformPrintf("  %s", debugGammas[(int)gammas.type(channel)]);
                }
                SkCSXformPrintf("\n");
                bool gammaNeedsRef = false;
                for (int channel = 0; channel < gammas.channels(); ++channel) {
                    if (SkGammas::Type::kTable_Type == gammas.type(channel)) {
                        SkTableTransferFn table = {
                                gammas.table(channel),
                                gammas.data(channel).fTable.fSize,
                        };

                        this->addTableFn(table, channel);
                        gammaNeedsRef = true;
                    } else {
                        SkColorSpaceTransferFn fn = gamma_to_parametric(gammas, channel);
                        this->addTransferFn(fn, channel);
                    }
                }
                if (gammaNeedsRef) {
                    fGammaRefs.push_back(sk_ref_sp(&gammas));
                }
                break;
            }
            case SkColorSpace_A2B::Element::Type::kCLUT:
                SkCSXformPrintf("CLUT (%d -> %d) stage added\n", e.colorLUT().inputChannels(),
                                                                 e.colorLUT().outputChannels());
                fCLUTs.push_back(sk_ref_sp(&e.colorLUT()));
                fElementsPipeline.append(SkRasterPipeline::color_lookup_table,
                                         fCLUTs.back().get());
                break;
            case SkColorSpace_A2B::Element::Type::kMatrix:
                if (!e.matrix().isIdentity()) {
                    SkCSXformPrintf("Matrix stage added\n");
                    addMatrix(e.matrix());
                }
                break;
        }
    }

    // take care of monochrome ICC profiles (but not A2B with gray input color space!)
    if (1 == currentChannels) {
        // Gray color spaces must multiply their channel by the PCS whitepoint to convert to
        // the PCS however, PCSLAB profiles must be n-component LUT-based ones, which
        // need to have 3 (to match PCS) output channels, not 1
        SkASSERT(SkColorSpace_Base::InputColorFormat::kGray == srcSpace->inputColorFormat());
        SkASSERT(SkColorSpace_A2B::PCS::kXYZ == srcSpace->pcs());
        constexpr float PCSXYZWhitePoint[3] = {0.9642f, 1.f, 0.8249f};
        fMatrices.push_front(std::vector<float>(12, 0.f));
        std::copy_n(PCSXYZWhitePoint, 3, fMatrices.front().begin());
        fElementsPipeline.append(SkRasterPipeline::matrix_3x4, fMatrices.front().data());
        currentChannels = 3;
    }

    // Lab PCS -> XYZ PCS
    if (SkColorSpace_A2B::PCS::kLAB == srcSpace->pcs()) {
        SkCSXformPrintf("Lab -> XYZ element added\n");
        fElementsPipeline.append(SkRasterPipeline::lab_to_xyz);
    }

    // we should now be in XYZ PCS
    SkASSERT(3 == currentChannels);

    // and XYZ PCS -> output color space xforms
    if (!dstSpace->fromXYZD50()->isIdentity()) {
        addMatrix(*dstSpace->fromXYZD50());
    }

    if (kNonStandard_SkGammaNamed != dstSpace->gammaNamed()) {
        if (!fLinearDstGamma) {
            SkColorSpaceTransferFn fn =
                    invert_parametric(gammanamed_to_parametric(dstSpace->gammaNamed()));
            this->addTransferFns(fn, 3);
        }
    } else {
        for (int channel = 0; channel < 3; ++channel) {
            const SkGammas& gammas = *dstSpace->gammas();
            if (SkGammas::Type::kTable_Type == gammas.type(channel)) {
                static constexpr int kInvTableSize = 256;
                std::vector<float> storage(kInvTableSize);
                invert_table_gamma(storage.data(), nullptr, storage.size(), gammas.table(channel),
                                  gammas.data(channel).fTable.fSize);
                SkTableTransferFn table = {
                        storage.data(),
                        (int) storage.size(),
                };
                fTableStorage.push_front(std::move(storage));

                this->addTableFn(table, channel);
            } else {
                SkColorSpaceTransferFn fn = invert_parametric(gamma_to_parametric(gammas, channel));
                this->addTransferFn(fn, channel);
            }
        }
    }
}

void SkColorSpaceXform_A2B::addTransferFns(const SkColorSpaceTransferFn& fn, int channelCount) {
    for (int i = 0; i < channelCount; ++i) {
        this->addTransferFn(fn, i);
    }
}

void SkColorSpaceXform_A2B::addTransferFn(const SkColorSpaceTransferFn& fn, int channelIndex) {
    fTransferFns.push_front(fn);
    switch (channelIndex) {
        case 0:
            fElementsPipeline.append(SkRasterPipeline::parametric_r, &fTransferFns.front());
            break;
        case 1:
            fElementsPipeline.append(SkRasterPipeline::parametric_g, &fTransferFns.front());
            break;
        case 2:
            fElementsPipeline.append(SkRasterPipeline::parametric_b, &fTransferFns.front());
            break;
        case 3:
            fElementsPipeline.append(SkRasterPipeline::parametric_a, &fTransferFns.front());
            break;
        default:
            SkASSERT(false);
    }
}

void SkColorSpaceXform_A2B::addTableFn(const SkTableTransferFn& fn, int channelIndex) {
    fTableTransferFns.push_front(fn);
    switch (channelIndex) {
        case 0:
            fElementsPipeline.append(SkRasterPipeline::table_r, &fTableTransferFns.front());
            break;
        case 1:
            fElementsPipeline.append(SkRasterPipeline::table_g, &fTableTransferFns.front());
            break;
        case 2:
            fElementsPipeline.append(SkRasterPipeline::table_b, &fTableTransferFns.front());
            break;
        case 3:
            fElementsPipeline.append(SkRasterPipeline::table_a, &fTableTransferFns.front());
            break;
        default:
            SkASSERT(false);
    }
}

void SkColorSpaceXform_A2B::addMatrix(const SkMatrix44& matrix) {
    fMatrices.push_front(std::vector<float>(12));
    auto& m = fMatrices.front();
    m[ 0] = matrix.get(0, 0);
    m[ 1] = matrix.get(1, 0);
    m[ 2] = matrix.get(2, 0);
    m[ 3] = matrix.get(0, 1);
    m[ 4] = matrix.get(1, 1);
    m[ 5] = matrix.get(2, 1);
    m[ 6] = matrix.get(0, 2);
    m[ 7] = matrix.get(1, 2);
    m[ 8] = matrix.get(2, 2);
    m[ 9] = matrix.get(0, 3);
    m[10] = matrix.get(1, 3);
    m[11] = matrix.get(2, 3);
    SkASSERT(matrix.get(3, 0) == 0.f);
    SkASSERT(matrix.get(3, 1) == 0.f);
    SkASSERT(matrix.get(3, 2) == 0.f);
    SkASSERT(matrix.get(3, 3) == 1.f);
    fElementsPipeline.append(SkRasterPipeline::matrix_3x4, m.data());
    fElementsPipeline.append(SkRasterPipeline::clamp_0);
    fElementsPipeline.append(SkRasterPipeline::clamp_1);
}
