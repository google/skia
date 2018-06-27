/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorData.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformPriv.h"
#include "SkColorSpaceXform_A2B.h"
#include "SkColorSpaceXform_Base.h"
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_XYZ.h"
#include "SkHalf.h"
#include "SkMakeUnique.h"
#include "SkOpts.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"
#include "../jumper/SkJumper.h"

static constexpr float sk_linear_from_2dot2[256] = {
        0.000000000000000000f, 0.000005077051900662f, 0.000023328004666099f, 0.000056921765712193f,
        0.000107187362341244f, 0.000175123977503027f, 0.000261543754548491f, 0.000367136269815943f,
        0.000492503787191433f, 0.000638182842167022f, 0.000804658499513058f, 0.000992374304074325f,
        0.001201739522438400f, 0.001433134589671860f, 0.001686915316789280f, 0.001963416213396470f,
        0.002262953160706430f, 0.002585825596234170f, 0.002932318323938360f, 0.003302703032003640f,
        0.003697239578900130f, 0.004116177093282750f, 0.004559754922526020f, 0.005028203456855540f,
        0.005521744850239660f, 0.006040593654849810f, 0.006584957382581690f, 0.007155037004573030f,
        0.007751027397660610f, 0.008373117745148580f, 0.009021491898012130f, 0.009696328701658230f,
        0.010397802292555300f, 0.011126082368383200f, 0.011881334434813700f, 0.012663720031582100f,
        0.013473396940142600f, 0.014310519374884100f, 0.015175238159625200f, 0.016067700890886900f,
        0.016988052089250000f, 0.017936433339950200f, 0.018912983423721500f, 0.019917838438785700f,
        0.020951131914781100f, 0.022012994919336500f, 0.023103556157921400f, 0.024222942067534200f,
        0.025371276904734600f, 0.026548682828472900f, 0.027755279978126000f, 0.028991186547107800f,
        0.030256518852388700f, 0.031551391400226400f, 0.032875916948383800f, 0.034230206565082000f,
        0.035614369684918800f, 0.037028514161960200f, 0.038472746320194600f, 0.039947171001525600f,
        0.041451891611462500f, 0.042987010162657100f, 0.044552627316421400f, 0.046148842422351000f,
        0.047775753556170600f, 0.049433457555908000f, 0.051122050056493400f, 0.052841625522879000f,
        0.054592277281760300f, 0.056374097551979800f, 0.058187177473685400f, 0.060031607136313200f,
        0.061907475605455800f, 0.063814870948677200f, 0.065753880260330100f, 0.067724589685424300f,
        0.069727084442598800f, 0.071761448846239100f, 0.073827766327784600f, 0.075926119456264800f,
        0.078056589958101900f, 0.080219258736215100f, 0.082414205888459200f, 0.084641510725429500f,
        0.086901251787660300f, 0.089193506862247800f, 0.091518352998919500f, 0.093875866525577800f,
        0.096266123063339700f, 0.098689197541094500f, 0.101145164209600000f, 0.103634096655137000f,
        0.106156067812744000f, 0.108711149979039000f, 0.111299414824660000f, 0.113920933406333000f,
        0.116575776178572000f, 0.119264013005047000f, 0.121985713169619000f, 0.124740945387051000f,
        0.127529777813422000f, 0.130352278056244000f, 0.133208513184300000f, 0.136098549737202000f,
        0.139022453734703000f, 0.141980290685736000f, 0.144972125597231000f, 0.147998022982685000f,
        0.151058046870511000f, 0.154152260812165000f, 0.157280727890073000f, 0.160443510725344000f,
        0.163640671485290000f, 0.166872271890766000f, 0.170138373223312000f, 0.173439036332135000f,
        0.176774321640903000f, 0.180144289154390000f, 0.183548998464951000f, 0.186988508758844000f,
        0.190462878822409000f, 0.193972167048093000f, 0.197516431440340000f, 0.201095729621346000f,
        0.204710118836677000f, 0.208359655960767000f, 0.212044397502288000f, 0.215764399609395000f,
        0.219519718074868000f, 0.223310408341127000f, 0.227136525505149000f, 0.230998124323267000f,
        0.234895259215880000f, 0.238827984272048000f, 0.242796353254002000f, 0.246800419601550000f,
        0.250840236436400000f, 0.254915856566385000f, 0.259027332489606000f, 0.263174716398492000f,
        0.267358060183772000f, 0.271577415438375000f, 0.275832833461245000f, 0.280124365261085000f,
        0.284452061560024000f, 0.288815972797219000f, 0.293216149132375000f, 0.297652640449211000f,
        0.302125496358853000f, 0.306634766203158000f, 0.311180499057984000f, 0.315762743736397000f,
        0.320381548791810000f, 0.325036962521076000f, 0.329729032967515000f, 0.334457807923889000f,
        0.339223334935327000f, 0.344025661302187000f, 0.348864834082879000f, 0.353740900096629000f,
        0.358653905926199000f, 0.363603897920553000f, 0.368590922197487000f, 0.373615024646202000f,
        0.378676250929840000f, 0.383774646487975000f, 0.388910256539059000f, 0.394083126082829000f,
        0.399293299902674000f, 0.404540822567962000f, 0.409825738436323000f, 0.415148091655907000f,
        0.420507926167587000f, 0.425905285707146000f, 0.431340213807410000f, 0.436812753800359000f,
        0.442322948819202000f, 0.447870841800410000f, 0.453456475485731000f, 0.459079892424160000f,
        0.464741134973889000f, 0.470440245304218000f, 0.476177265397440000f, 0.481952237050698000f,
        0.487765201877811000f, 0.493616201311074000f, 0.499505276603030000f, 0.505432468828216000f,
        0.511397818884880000f, 0.517401367496673000f, 0.523443155214325000f, 0.529523222417277000f,
        0.535641609315311000f, 0.541798355950137000f, 0.547993502196972000f, 0.554227087766085000f,
        0.560499152204328000f, 0.566809734896638000f, 0.573158875067523000f, 0.579546611782525000f,
        0.585972983949661000f, 0.592438030320847000f, 0.598941789493296000f, 0.605484299910907000f,
        0.612065599865624000f, 0.618685727498780000f, 0.625344720802427000f, 0.632042617620641000f,
        0.638779455650817000f, 0.645555272444935000f, 0.652370105410821000f, 0.659223991813387000f,
        0.666116968775851000f, 0.673049073280942000f, 0.680020342172095000f, 0.687030812154625000f,
        0.694080519796882000f, 0.701169501531402000f, 0.708297793656032000f, 0.715465432335048000f,
        0.722672453600255000f, 0.729918893352071000f, 0.737204787360605000f, 0.744530171266715000f,
        0.751895080583051000f, 0.759299550695091000f, 0.766743616862161000f, 0.774227314218442000f,
        0.781750677773962000f, 0.789313742415586000f, 0.796916542907978000f, 0.804559113894567000f,
        0.812241489898490000f, 0.819963705323528000f, 0.827725794455034000f, 0.835527791460841000f,
        0.843369730392169000f, 0.851251645184515000f, 0.859173569658532000f, 0.867135537520905000f,
        0.875137582365205000f, 0.883179737672745000f, 0.891262036813419000f, 0.899384513046529000f,
        0.907547199521614000f, 0.915750129279253000f, 0.923993335251873000f, 0.932276850264543000f,
        0.940600707035753000f, 0.948964938178195000f, 0.957369576199527000f, 0.965814653503130000f,
        0.974300202388861000f, 0.982826255053791000f, 0.991392843592940000f, 1.000000000000000000f,
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static void build_table_linear_from_gamma(float* outTable, float exponent) {
    for (float x = 0.0f; x <= 1.0f; x += (1.0f/255.0f)) {
        *outTable++ = powf(x, exponent);
    }
}

// outTable is always 256 entries, inTable may be larger or smaller.
static void build_table_linear_from_gamma(float* outTable, const float* inTable,
                                          int inTableSize) {
    if (256 == inTableSize) {
        memcpy(outTable, inTable, sizeof(float) * 256);
        return;
    }

    for (float x = 0.0f; x <= 1.0f; x += (1.0f/255.0f)) {
        *outTable++ = interp_lut(x, inTable, inTableSize);
    }
}


static void build_table_linear_from_gamma(float* outTable, float g, float a, float b, float c,
                                          float d, float e, float f) {
    // Y = (aX + b)^g + e  for X >= d
    // Y = cX + f          otherwise
    for (float x = 0.0f; x <= 1.0f; x += (1.0f/255.0f)) {
        if (x >= d) {
            *outTable++ = clamp_0_1(powf(a * x + b, g) + e);
        } else {
            *outTable++ = clamp_0_1(c * x + f);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static const int kDstGammaTableSize = SkColorSpaceXform_Base::kDstGammaTableSize;

static void build_table_linear_to_gamma(uint8_t* outTable, float exponent) {
    float toGammaExp = 1.0f / exponent;

    for (int i = 0; i < kDstGammaTableSize; i++) {
        float x = ((float) i) * (1.0f / ((float) (kDstGammaTableSize - 1)));
        outTable[i] = clamp_normalized_float_to_byte(powf(x, toGammaExp));
    }
}

static void build_table_linear_to_gamma(uint8_t* outTable, const float* inTable,
                                        int inTableSize) {
    invert_table_gamma(nullptr, outTable, kDstGammaTableSize, inTable, inTableSize);
}

static float inverse_parametric(float x, float g, float a, float b, float c, float d, float e,
                                float f) {
    // We need to take the inverse of the following piecewise function.
    // Y = (aX + b)^g + e  for X >= d
    // Y = cX + f          otherwise

    // Assume that the gamma function is continuous, or this won't make much sense anyway.
    // Plug in |d| to the second equation to calculate the new piecewise interval.
    // Then simply use the inverse of the original functions.
    float interval = c * d + f;
    if (x < interval) {
        // X = (Y - F) / C
        if (0.0f == c) {
            // The gamma curve for this segment is constant, so the inverse is undefined.
            // Since this is the lower segment, guess zero.
            return 0.0f;
        }

        return (x - f) / c;
    }

    // X = ((Y - E)^(1 / G) - B) / A
    if (0.0f == a || 0.0f == g) {
        // The gamma curve for this segment is constant, so the inverse is undefined.
        // Since this is the upper segment, guess one.
        return 1.0f;
    }

    return (powf(x - e, 1.0f / g) - b) / a;
}

static void build_table_linear_to_gamma(uint8_t* outTable, float g, float a,
                                        float b, float c, float d, float e, float f) {
    for (int i = 0; i < kDstGammaTableSize; i++) {
        float x = ((float) i) * (1.0f / ((float) (kDstGammaTableSize - 1)));
        float y = inverse_parametric(x, g, a, b, c, d, e, f);
        outTable[i] = clamp_normalized_float_to_byte(y);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct GammaFns {
    const T* fSRGBTable;
    const T* f2Dot2Table;
    void (*fBuildFromValue)(T*, float);
    void (*fBuildFromTable)(T*, const float*, int);
    void (*fBuildFromParam)(T*, float, float, float, float, float, float, float);
};

static const GammaFns<float> kToLinear {
    sk_linear_from_srgb,
    sk_linear_from_2dot2,
    &build_table_linear_from_gamma,
    &build_table_linear_from_gamma,
    &build_table_linear_from_gamma,
};

static const GammaFns<uint8_t> kFromLinear {
    nullptr,
    nullptr,
    &build_table_linear_to_gamma,
    &build_table_linear_to_gamma,
    &build_table_linear_to_gamma,
};

// Build tables to transform src gamma to linear.
template <typename T>
static void build_gamma_tables(const T* outGammaTables[3], T* gammaTableStorage, int gammaTableSize,
                               const SkColorSpace_XYZ* space, const GammaFns<T>& fns,
                               bool gammasAreMatching)
{
    switch (space->gammaNamed()) {
        case kSRGB_SkGammaNamed:
            outGammaTables[0] = outGammaTables[1] = outGammaTables[2] = fns.fSRGBTable;
            break;
        case k2Dot2Curve_SkGammaNamed:
            outGammaTables[0] = outGammaTables[1] = outGammaTables[2] = fns.f2Dot2Table;
            break;
        case kLinear_SkGammaNamed:
            outGammaTables[0] = outGammaTables[1] = outGammaTables[2] = nullptr;
            break;
        default: {
            const SkGammas* gammas = space->gammas();
            SkASSERT(gammas);

            auto build_table = [=](int i) {
                if (gammas->isNamed(i)) {
                    switch (gammas->data(i).fNamed) {
                        case kSRGB_SkGammaNamed:
                            (*fns.fBuildFromParam)(&gammaTableStorage[i * gammaTableSize],
                                                   gSRGB_TransferFn.fG,
                                                   gSRGB_TransferFn.fA,
                                                   gSRGB_TransferFn.fB,
                                                   gSRGB_TransferFn.fC,
                                                   gSRGB_TransferFn.fD,
                                                   gSRGB_TransferFn.fE,
                                                   gSRGB_TransferFn.fF);
                            outGammaTables[i] = &gammaTableStorage[i * gammaTableSize];
                            break;
                        case k2Dot2Curve_SkGammaNamed:
                            (*fns.fBuildFromValue)(&gammaTableStorage[i * gammaTableSize], 2.2f);
                            outGammaTables[i] = &gammaTableStorage[i * gammaTableSize];
                            break;
                        case kLinear_SkGammaNamed:
                            (*fns.fBuildFromValue)(&gammaTableStorage[i * gammaTableSize], 1.0f);
                            outGammaTables[i] = &gammaTableStorage[i * gammaTableSize];
                            break;
                        default:
                            SkASSERT(false);
                            break;
                    }
                } else if (gammas->isValue(i)) {
                    (*fns.fBuildFromValue)(&gammaTableStorage[i * gammaTableSize],
                                           gammas->data(i).fValue);
                    outGammaTables[i] = &gammaTableStorage[i * gammaTableSize];
                } else if (gammas->isTable(i)) {
                    (*fns.fBuildFromTable)(&gammaTableStorage[i * gammaTableSize], gammas->table(i),
                                           gammas->data(i).fTable.fSize);
                    outGammaTables[i] = &gammaTableStorage[i * gammaTableSize];
                } else {
                    SkASSERT(gammas->isParametric(i));
                    const SkColorSpaceTransferFn& params = gammas->params(i);
                    (*fns.fBuildFromParam)(&gammaTableStorage[i * gammaTableSize], params.fG,
                                           params.fA, params.fB, params.fC, params.fD, params.fE,
                                           params.fF);
                    outGammaTables[i] = &gammaTableStorage[i * gammaTableSize];
                }
            };

            if (gammasAreMatching) {
                build_table(0);
                outGammaTables[1] = outGammaTables[0];
                outGammaTables[2] = outGammaTables[0];
            } else {
                build_table(0);
                build_table(1);
                build_table(2);
            }

            break;
        }
    }
}

void SkColorSpaceXform_Base::BuildDstGammaTables(const uint8_t* dstGammaTables[3],
                                                 uint8_t* dstStorage,
                                                 const SkColorSpace_XYZ* space,
                                                 bool gammasAreMatching) {
    build_gamma_tables(dstGammaTables, dstStorage, kDstGammaTableSize, space, kFromLinear,
                       gammasAreMatching);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(SkColorSpace* src,
                                                          SkColorSpace* dst) {
    return SkColorSpaceXform_Base::New(src, dst, SkTransferFunctionBehavior::kRespect);
}

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform_Base::New(
        SkColorSpace* src,
        SkColorSpace* dst,
        SkTransferFunctionBehavior premulBehavior) {

    if (!src || !dst) {
        // Invalid input
        return nullptr;
    }

    if (!dst->toXYZD50()) {
        SkCSXformPrintf("only XYZ destinations supported\n");
        return nullptr;
    }

    if (src->toXYZD50()) {
        return skstd::make_unique<SkColorSpaceXform_XYZ>(static_cast<SkColorSpace_XYZ*>(src),
                                                         static_cast<SkColorSpace_XYZ*>(dst),
                                                         premulBehavior);
    }
    return skstd::make_unique<SkColorSpaceXform_A2B>(static_cast<SkColorSpace_A2B*>(src),
                                                     static_cast<SkColorSpace_XYZ*>(dst));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline int num_tables(SkColorSpace_XYZ* space) {
    switch (space->gammaNamed()) {
        case kSRGB_SkGammaNamed:
        case k2Dot2Curve_SkGammaNamed:
        case kLinear_SkGammaNamed:
            return 0;
        default: {
            const SkGammas* gammas = space->gammas();
            SkASSERT(gammas);

            // It's likely that each component will have the same gamma.  In this case,
            // we only need to build one table.
            return gammas->allChannelsSame() ? 1 : 3;
        }
    }
}

SkColorSpaceXform_XYZ::SkColorSpaceXform_XYZ(SkColorSpace_XYZ* src,
                                             SkColorSpace_XYZ* dst,
                                             SkTransferFunctionBehavior premulBehavior)
    : fPremulBehavior(premulBehavior)
{
    fColorSpacesAreIdentical = SkColorSpace::Equals(src, dst);

    SkMatrix44 srcToDst(SkMatrix44::kIdentity_Constructor);
    if (!fColorSpacesAreIdentical && *src->toXYZD50() != *dst->toXYZD50()) {
        srcToDst.setConcat(*dst->fromXYZD50(), *src->toXYZD50());
    }

    fSrcToDst[ 0] = srcToDst.get(0, 0);
    fSrcToDst[ 1] = srcToDst.get(1, 0);
    fSrcToDst[ 2] = srcToDst.get(2, 0);
    fSrcToDst[ 3] = srcToDst.get(0, 1);
    fSrcToDst[ 4] = srcToDst.get(1, 1);
    fSrcToDst[ 5] = srcToDst.get(2, 1);
    fSrcToDst[ 6] = srcToDst.get(0, 2);
    fSrcToDst[ 7] = srcToDst.get(1, 2);
    fSrcToDst[ 8] = srcToDst.get(2, 2);
    fSrcToDst[ 9] = srcToDst.get(0, 3);
    fSrcToDst[10] = srcToDst.get(1, 3);
    fSrcToDst[11] = srcToDst.get(2, 3);
    fSrcToDstIsIdentity = srcToDst.isIdentity();

    const int numSrcTables = num_tables(src);
    const size_t srcEntries = numSrcTables * 256;
    const bool srcGammasAreMatching = (1 >= numSrcTables);
    fSrcStorage.reset(srcEntries);
    build_gamma_tables(fSrcGammaTables, fSrcStorage.get(), 256, src, kToLinear,
                       srcGammasAreMatching);

    const int numDstTables = num_tables(dst);
    dst->toDstGammaTables(fDstGammaTables, &fDstStorage, numDstTables);

    if (src->gammaIsLinear()) {
        fSrcGamma = kLinear_SrcGamma;
    } else if (kSRGB_SkGammaNamed == src->gammaNamed()) {
        fSrcGamma = kSRGB_SrcGamma;
    } else {
        fSrcGamma = kTable_SrcGamma;
    }

    switch (dst->gammaNamed()) {
        case kSRGB_SkGammaNamed:
            fDstGamma = kSRGB_DstGamma;
            break;
        case k2Dot2Curve_SkGammaNamed:
            fDstGamma = k2Dot2_DstGamma;
            break;
        case kLinear_SkGammaNamed:
            fDstGamma = kLinear_DstGamma;
            break;
        default:
            fDstGamma = kTable_DstGamma;
            break;
    }
}


bool SkColorSpaceXform_XYZ::onApply(ColorFormat dstColorFormat, void* dst,
                                    ColorFormat srcColorFormat, const void* src,
                                    int len, SkAlphaType alphaType) const {
    if (fColorSpacesAreIdentical && kPremul_SkAlphaType != alphaType) {
        if ((kRGBA_8888_ColorFormat == dstColorFormat &&
             kRGBA_8888_ColorFormat == srcColorFormat) ||
            (kBGRA_8888_ColorFormat == dstColorFormat &&
             kBGRA_8888_ColorFormat == srcColorFormat))
        {
            memcpy(dst, src, len * sizeof(uint32_t));
            return true;
        }

        if ((kRGBA_8888_ColorFormat == dstColorFormat &&
             kBGRA_8888_ColorFormat == srcColorFormat) ||
            (kBGRA_8888_ColorFormat == dstColorFormat &&
             kRGBA_8888_ColorFormat == srcColorFormat))
        {
            SkOpts::RGBA_to_BGRA((uint32_t*)dst, src, len);
            return true;
        }
    }

    SkRasterPipeline_<256> pipeline;

    SkJumper_MemoryCtx src_ctx = { (void*)src, 0 },
                       dst_ctx = { (void*)dst, 0 };

    LoadTablesContext loadTables;
    switch (srcColorFormat) {
        case kRGBA_8888_ColorFormat:
            if (kLinear_SrcGamma == fSrcGamma) {
                pipeline.append(SkRasterPipeline::load_8888, &src_ctx);
            } else {
                loadTables.fSrc = src;
                loadTables.fR = fSrcGammaTables[0];
                loadTables.fG = fSrcGammaTables[1];
                loadTables.fB = fSrcGammaTables[2];
                pipeline.append(SkRasterPipeline::load_tables, &loadTables);
            }

            break;
        case kBGRA_8888_ColorFormat:
            if (kLinear_SrcGamma == fSrcGamma) {
                pipeline.append(SkRasterPipeline::load_bgra, &src_ctx);
            } else {
                loadTables.fSrc = src;
                loadTables.fR = fSrcGammaTables[2];
                loadTables.fG = fSrcGammaTables[1];
                loadTables.fB = fSrcGammaTables[0];
                pipeline.append(SkRasterPipeline::load_tables, &loadTables);
                pipeline.append(SkRasterPipeline::swap_rb);
            }

            break;
        case kRGBA_F16_ColorFormat:
            if (kLinear_SrcGamma != fSrcGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::load_f16, &src_ctx);
            break;
        case kRGBA_F32_ColorFormat:
            if (kLinear_SrcGamma != fSrcGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::load_f32, &src_ctx);
            break;
        case kRGBA_U16_BE_ColorFormat:
            switch (fSrcGamma) {
                case kLinear_SrcGamma:
                    pipeline.append(SkRasterPipeline::load_u16_be, &src_ctx);
                    break;
                case kSRGB_SrcGamma:
                    pipeline.append(SkRasterPipeline::load_u16_be, &src_ctx);
                    pipeline.append(SkRasterPipeline::from_srgb);
                    break;
                case kTable_SrcGamma:
                    loadTables.fSrc = src;
                    loadTables.fR = fSrcGammaTables[0];
                    loadTables.fG = fSrcGammaTables[1];
                    loadTables.fB = fSrcGammaTables[2];
                    pipeline.append(SkRasterPipeline::load_tables_u16_be, &loadTables);
                    break;
            }
            break;
        case kRGB_U16_BE_ColorFormat:
            switch (fSrcGamma) {
                case kLinear_SrcGamma:
                    pipeline.append(SkRasterPipeline::load_rgb_u16_be, &src_ctx);
                    break;
                case kSRGB_SrcGamma:
                    pipeline.append(SkRasterPipeline::load_rgb_u16_be, &src_ctx);
                    pipeline.append(SkRasterPipeline::from_srgb);
                    break;
                case kTable_SrcGamma:
                    loadTables.fSrc = src;
                    loadTables.fR = fSrcGammaTables[0];
                    loadTables.fG = fSrcGammaTables[1];
                    loadTables.fB = fSrcGammaTables[2];
                    pipeline.append(SkRasterPipeline::load_tables_rgb_u16_be, &loadTables);
                    break;
            }
            break;
        default:
            return false;
    }

    if (!fSrcToDstIsIdentity) {
        pipeline.append(SkRasterPipeline::matrix_3x4, fSrcToDst);

        if (kRGBA_F16_ColorFormat != dstColorFormat &&
            kRGBA_F32_ColorFormat != dstColorFormat)
        {
            bool need_clamp_0, need_clamp_1;
            analyze_3x4_matrix(fSrcToDst, &need_clamp_0, &need_clamp_1);

            if (need_clamp_0) { pipeline.append(SkRasterPipeline::clamp_0); }
            if (need_clamp_1) { pipeline.append(SkRasterPipeline::clamp_1); }
        }
    }

    if (kPremul_SkAlphaType == alphaType && SkTransferFunctionBehavior::kRespect == fPremulBehavior)
    {
        pipeline.append(SkRasterPipeline::premul);
    }

    TablesContext tables;
    float to_2dot2 = 1/2.2f;
    switch (fDstGamma) {
        case kSRGB_DstGamma:
            pipeline.append(SkRasterPipeline::to_srgb);
            break;
        case k2Dot2_DstGamma:
            pipeline.append(SkRasterPipeline::gamma, &to_2dot2);
            break;
        case kTable_DstGamma:
            tables.fR = fDstGammaTables[0];
            tables.fG = fDstGammaTables[1];
            tables.fB = fDstGammaTables[2];
            tables.fCount = SkColorSpaceXform_Base::kDstGammaTableSize;
            pipeline.append(SkRasterPipeline::byte_tables_rgb, &tables);
        default:
            break;
    }

    if (kPremul_SkAlphaType == alphaType && SkTransferFunctionBehavior::kIgnore == fPremulBehavior)
    {
        pipeline.append(SkRasterPipeline::premul);
    }

    switch (dstColorFormat) {
        case kRGBA_8888_ColorFormat:
             pipeline.append(SkRasterPipeline::store_8888, &dst_ctx);
            break;
        case kBGRA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::store_bgra, &dst_ctx);
            break;
        case kRGBA_F16_ColorFormat:
            if (kLinear_DstGamma != fDstGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::store_f16, &dst_ctx);
            break;
        case kRGBA_F32_ColorFormat:
            if (kLinear_DstGamma != fDstGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::store_f32, &dst_ctx);
            break;
        case kBGR_565_ColorFormat:
            if (kOpaque_SkAlphaType != alphaType) {
                return false;
            }
            pipeline.append(SkRasterPipeline::store_565, &dst_ctx);
            break;
        default:
            return false;
    }
    pipeline.run(0,0, len,1);
    return true;
}

std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace_XYZ* space) {
    auto xform = skstd::make_unique<SkColorSpaceXform_XYZ>(space, space,
                                                           SkTransferFunctionBehavior::kRespect);
    xform->pretendNotToBeIdentityForTesting();
    return std::move(xform);
}

bool SkColorSpaceXform::apply(ColorFormat dstColorFormat, void* dst,
                              ColorFormat srcColorFormat, const void* src,
                              int len, SkAlphaType alphaType) const {
    return ((SkColorSpaceXform_Base*) this)->onApply(dstColorFormat, dst,
                                                     srcColorFormat, src,
                                                     len, alphaType);
}

bool SkColorSpaceXform::Apply(SkColorSpace* dstCS, ColorFormat dstFormat, void* dst,
                              SkColorSpace* srcCS, ColorFormat srcFormat, const void* src,
                              int len, AlphaOp op) {
    SkAlphaType at;
    switch (op) {
        case kPreserve_AlphaOp:    at = kUnpremul_SkAlphaType; break;
        case kPremul_AlphaOp:      at = kPremul_SkAlphaType;   break;
        case kSrcIsOpaque_AlphaOp: at = kOpaque_SkAlphaType;   break;
    }
    return New(srcCS, dstCS)->apply(dstFormat, dst, srcFormat, src, len, at);
}
