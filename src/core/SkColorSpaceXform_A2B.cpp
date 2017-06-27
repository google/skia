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
#include "../jumper/SkJumper.h"

bool SkColorSpaceXform_A2B::onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat,
                                    const void* src, int count, SkAlphaType alphaType) const {
    SkRasterPipeline_<256> pipeline;
    switch (srcFormat) {
        case kBGRA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::load_bgra, &src);
            break;
        case kRGBA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::load_8888, &src);
            break;
        case kRGBA_U16_BE_ColorFormat:
            pipeline.append(SkRasterPipeline::load_u16_be, &src);
            break;
        case kRGB_U16_BE_ColorFormat:
            pipeline.append(SkRasterPipeline::load_rgb_u16_be, &src);
            break;
        default:
            SkCSXformPrintf("F16/F32 sources must be linear.\n");
            return false;
    }

    pipeline.extend(fElementsPipeline);

    if (kPremul_SkAlphaType == alphaType) {
        pipeline.append(SkRasterPipeline::premul);
    }

    switch (dstFormat) {
        case kBGRA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::store_bgra, &dst);
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
        case kBGR_565_ColorFormat:
            if (kOpaque_SkAlphaType != alphaType) {
                return false;
            }
            pipeline.append(SkRasterPipeline::store_565, &dst);
            break;
        default:
            return false;
    }
    pipeline.run(0,0, count);

    return true;
}

static inline bool gamma_to_parametric(SkColorSpaceTransferFn* coeffs, const SkGammas& gammas,
                                       int channel) {
    switch (gammas.type(channel)) {
        case SkGammas::Type::kNamed_Type:
            return named_to_parametric(coeffs, gammas.data(channel).fNamed);
        case SkGammas::Type::kValue_Type:
            value_to_parametric(coeffs, gammas.data(channel).fValue);
            return true;
        case SkGammas::Type::kParam_Type:
            *coeffs = gammas.params(channel);
            return true;
        default:
            return false;
    }
}

SkColorSpaceXform_A2B::SkColorSpaceXform_A2B(SkColorSpace_A2B* srcSpace,
                                             SkColorSpace_XYZ* dstSpace)
    : fElementsPipeline(&fAlloc)
    , fLinearDstGamma(kLinear_SkGammaNamed == dstSpace->gammaNamed()) {
#if (SkCSXformPrintfDefined)
    static const char* debugGammaNamed[4] = {
        "Linear", "SRGB", "2.2", "NonStandard"
    };
    static const char* debugGammas[5] = {
        "None", "Named", "Value", "Table", "Param"
    };
#endif
    int currentChannels;
    switch (srcSpace->iccType()) {
        case SkColorSpace_Base::kRGB_ICCTypeFlag:
            currentChannels = 3;
            break;
        case SkColorSpace_Base::kCMYK_ICCTypeFlag: {
            currentChannels = 4;
            // CMYK images from JPEGs (the only format that supports it) are actually
            // inverted CMYK, so we need to invert every channel.
            // TransferFn is y = -x + 1 for x < 1.f, otherwise 0x + 0, ie y = 1 - x for x in [0,1]
            SkColorSpaceTransferFn fn = {0,0,0,0,0,0,0};
            fn.fG =  1;
            fn.fA =  0;
            fn.fB =  0;
            fn.fC = -1;
            fn.fD =  1;
            fn.fE =  0;
            fn.fF =  1;
            this->addTransferFns(fn,4);
            break;
        }
        default:
            currentChannels = 0;
            SkASSERT(false);
    }
    // add in all input color space -> PCS xforms
    for (int i = 0; i < srcSpace->count(); ++i) {
        const SkColorSpace_A2B::Element& e = srcSpace->element(i);
        SkASSERT(e.inputChannels() == currentChannels);
        currentChannels = e.outputChannels();
        switch (e.type()) {
            case SkColorSpace_A2B::Element::Type::kGammaNamed:
                if (kLinear_SkGammaNamed == e.gammaNamed()) {
                    break;
                }

                // Take the fast path for ordinary sRGB.
                if (3 == currentChannels && kSRGB_SkGammaNamed == e.gammaNamed()) {
                    SkCSXformPrintf("fast path from sRGB\n");
                    // Images should always start the pipeline as unpremul
                    fElementsPipeline.append_from_srgb(kUnpremul_SkAlphaType);
                    break;
                }

                SkCSXformPrintf("Gamma stage added: %s\n", debugGammaNamed[(int)e.gammaNamed()]);
                SkColorSpaceTransferFn fn;
                SkAssertResult(named_to_parametric(&fn, e.gammaNamed()));
                this->addTransferFns(fn, currentChannels);
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

                        gammaNeedsRef |= !this->buildTableFn(&table);
                        this->addTableFn(table, channel);
                    } else {
                        SkColorSpaceTransferFn fn;
                        SkAssertResult(gamma_to_parametric(&fn, gammas, channel));
                        this->addTransferFn(fn, channel);
                    }
                }
                if (gammaNeedsRef) {
                    this->copy(sk_ref_sp(&gammas));
                }
                break;
            }
            case SkColorSpace_A2B::Element::Type::kCLUT: {
                SkCSXformPrintf("CLUT (%d -> %d) stage added\n", e.colorLUT().inputChannels(),
                                                                 e.colorLUT().outputChannels());
                struct CallbackCtx : SkJumper_CallbackCtx {
                    sk_sp<const SkColorLookUpTable> clut;
                    // clut->interp() can't always safely alias its arguments,
                    // so we allocate a second buffer to hold our results.
                    float results[4*SkJumper_kMaxStride];
                };
                auto cb = fAlloc.make<CallbackCtx>();
                cb->clut      = sk_ref_sp(&e.colorLUT());
                cb->read_from = cb->results;
                cb->fn        = [](SkJumper_CallbackCtx* ctx, int active_pixels) {
                    auto c = (CallbackCtx*)ctx;
                    for (int i = 0; i < active_pixels; i++) {
                        // Look up red, green, and blue for this pixel using 3-4 values from rgba.
                        c->clut->interp(c->results+4*i, c->rgba+4*i);

                        // If we used 3 inputs (rgb) preserve the fourth as alpha.
                        // If we used 4 inputs (cmyk) force alpha to 1.
                        c->results[4*i+3] = (3 == c->clut->inputChannels()) ? c->rgba[4*i+3] : 1.0f;
                    }
                };
                fElementsPipeline.append(SkRasterPipeline::callback, cb);
                break;
            }
            case SkColorSpace_A2B::Element::Type::kMatrix:
                if (!e.matrix().isIdentity()) {
                    SkCSXformPrintf("Matrix stage added\n");
                    addMatrix(e.matrix());
                }
                break;
        }
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

    switch (dstSpace->gammaNamed()) {
        case kLinear_SkGammaNamed:
            // do nothing
            break;
        case k2Dot2Curve_SkGammaNamed: {
            SkColorSpaceTransferFn fn = {0,0,0,0,0,0,0};
            fn.fG = 1/2.2f;
            fn.fA = 1;
            auto to_2dot2 = this->copy(fn);
            fElementsPipeline.append(SkRasterPipeline::parametric_r, to_2dot2);
            fElementsPipeline.append(SkRasterPipeline::parametric_g, to_2dot2);
            fElementsPipeline.append(SkRasterPipeline::parametric_b, to_2dot2);
            break;
        }
        case kSRGB_SkGammaNamed:
            fElementsPipeline.append(SkRasterPipeline::to_srgb);
            break;
        case kNonStandard_SkGammaNamed: {
            for (int channel = 0; channel < 3; ++channel) {
                const SkGammas& gammas = *dstSpace->gammas();
                if (SkGammas::Type::kTable_Type == gammas.type(channel)) {
                    static constexpr int kInvTableSize = 256;
                    auto storage = fAlloc.makeArray<float>(kInvTableSize);
                    invert_table_gamma(storage, nullptr, kInvTableSize,
                                       gammas.table(channel),
                                       gammas.data(channel).fTable.fSize);
                    SkTableTransferFn table = { storage, kInvTableSize };
                    this->addTableFn(table, channel);
                } else {
                    SkColorSpaceTransferFn fn;
                    SkAssertResult(gamma_to_parametric(&fn, gammas, channel));
                    this->addTransferFn(fn.invert(), channel);
                }
            }
        }
        break;
    }
}

void SkColorSpaceXform_A2B::addTransferFns(const SkColorSpaceTransferFn& fn, int channelCount) {
    for (int i = 0; i < channelCount; ++i) {
        this->addTransferFn(fn, i);
    }
}

void SkColorSpaceXform_A2B::addTransferFn(const SkColorSpaceTransferFn& fn, int channelIndex) {
    switch (channelIndex) {
        case 0:
            fElementsPipeline.append(SkRasterPipeline::parametric_r, this->copy(fn));
            break;
        case 1:
            fElementsPipeline.append(SkRasterPipeline::parametric_g, this->copy(fn));
            break;
        case 2:
            fElementsPipeline.append(SkRasterPipeline::parametric_b, this->copy(fn));
            break;
        case 3:
            fElementsPipeline.append(SkRasterPipeline::parametric_a, this->copy(fn));
            break;
        default:
            SkASSERT(false);
    }
}

/**
 *  |fn| is an in-out parameter.  If the table is too small to perform reasonable table-lookups
 *  without interpolation, we will build a bigger table.
 *
 *  This returns false if we use the original table, meaning we do nothing here but need to keep
 *  a reference to the original table.  This returns true if we build a new table and the original
 *  table can be discarded.
 */
bool SkColorSpaceXform_A2B::buildTableFn(SkTableTransferFn* fn) {
    // Arbitrary, but seems like a reasonable guess.
    static constexpr int kMinTableSize = 256;

    if (fn->fSize >= kMinTableSize) {
        return false;
    }

    float* outTable = fAlloc.makeArray<float>(kMinTableSize);
    float step = 1.0f / (kMinTableSize - 1);
    for (int i = 0; i < kMinTableSize; i++) {
        outTable[i] = interp_lut(i * step, fn->fData, fn->fSize);
    }

    fn->fData = outTable;
    fn->fSize = kMinTableSize;
    return true;
}

void SkColorSpaceXform_A2B::addTableFn(const SkTableTransferFn& fn, int channelIndex) {
    switch (channelIndex) {
        case 0:
            fElementsPipeline.append(SkRasterPipeline::table_r, this->copy(fn));
            break;
        case 1:
            fElementsPipeline.append(SkRasterPipeline::table_g, this->copy(fn));
            break;
        case 2:
            fElementsPipeline.append(SkRasterPipeline::table_b, this->copy(fn));
            break;
        case 3:
            fElementsPipeline.append(SkRasterPipeline::table_a, this->copy(fn));
            break;
        default:
            SkASSERT(false);
    }
}

void SkColorSpaceXform_A2B::addMatrix(const SkMatrix44& m44) {
    auto m = fAlloc.makeArray<float>(12);
    m[0] = m44.get(0,0); m[ 1] = m44.get(1,0); m[ 2] = m44.get(2,0);
    m[3] = m44.get(0,1); m[ 4] = m44.get(1,1); m[ 5] = m44.get(2,1);
    m[6] = m44.get(0,2); m[ 7] = m44.get(1,2); m[ 8] = m44.get(2,2);
    m[9] = m44.get(0,3); m[10] = m44.get(1,3); m[11] = m44.get(2,3);

    SkASSERT(m44.get(3,0) == 0.0f);
    SkASSERT(m44.get(3,1) == 0.0f);
    SkASSERT(m44.get(3,2) == 0.0f);
    SkASSERT(m44.get(3,3) == 1.0f);

    fElementsPipeline.append(SkRasterPipeline::matrix_3x4, m);
    fElementsPipeline.append(SkRasterPipeline::clamp_0);
    fElementsPipeline.append(SkRasterPipeline::clamp_1);
}
