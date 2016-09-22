/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXform.h"
#include "SkHalf.h"
#include "SkOpts.h"
#include "SkSRGB.h"

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

// Interpolating lookup in a variably sized table.
static float interp_lut(float input, const float* table, int tableSize) {
    float index = input * (tableSize - 1);
    float diff = index - sk_float_floor2int(index);
    return table[(int) sk_float_floor2int(index)] * (1.0f - diff) +
            table[(int) sk_float_ceil2int(index)] * diff;
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
    // Y = (aX + b)^g + c  for X >= d
    // Y = eX + f          otherwise
    for (float x = 0.0f; x <= 1.0f; x += (1.0f/255.0f)) {
        if (x >= d) {
            *outTable++ = powf(a * x + b, g) + c;
        } else {
            *outTable++ = e * x + f;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Expand range from 0-1 to 0-255, then convert.
static uint8_t clamp_normalized_float_to_byte(float v) {
    // The ordering of the logic is a little strange here in order
    // to make sure we convert NaNs to 0.
    v = v * 255.0f;
    if (v >= 254.5f) {
        return 255;
    } else if (v >= 0.5f) {
        return (uint8_t) (v + 0.5f);
    } else {
        return 0;
    }
}

static const int kDstGammaTableSize =
        SkColorSpaceXform_Base<kTable_SrcGamma, kTable_DstGamma, kNone_ColorSpaceMatch>
        ::kDstGammaTableSize;

static void build_table_linear_to_gamma(uint8_t* outTable, float exponent) {
    float toGammaExp = 1.0f / exponent;

    for (int i = 0; i < kDstGammaTableSize; i++) {
        float x = ((float) i) * (1.0f / ((float) (kDstGammaTableSize - 1)));
        outTable[i] = clamp_normalized_float_to_byte(powf(x, toGammaExp));
    }
}

// Inverse table lookup.  Ex: what index corresponds to the input value?  This will
// have strange results when the table is non-increasing.  But any sane gamma
// function will be increasing.
static float inverse_interp_lut(float input, const float* table, int tableSize) {
    if (input <= table[0]) {
        return table[0];
    } else if (input >= table[tableSize - 1]) {
        return 1.0f;
    }

    for (int i = 1; i < tableSize; i++) {
        if (table[i] >= input) {
            // We are guaranteed that input is greater than table[i - 1].
            float diff = input - table[i - 1];
            float distance = table[i] - table[i - 1];
            float index = (i - 1) + diff / distance;
            return index / (tableSize - 1);
        }
    }

    // Should be unreachable, since we'll return before the loop if input is
    // larger than the last entry.
    SkASSERT(false);
    return 0.0f;
}

static void build_table_linear_to_gamma(uint8_t* outTable, const float* inTable,
                                        int inTableSize) {
    for (int i = 0; i < kDstGammaTableSize; i++) {
        float x = ((float) i) * (1.0f / ((float) (kDstGammaTableSize - 1)));
        float y = inverse_interp_lut(x, inTable, inTableSize);
        outTable[i] = clamp_normalized_float_to_byte(y);
    }
}

static float inverse_parametric(float x, float g, float a, float b, float c, float d, float e,
                                float f) {
    // We need to take the inverse of the following piecewise function.
    // Y = (aX + b)^g + c  for X >= d
    // Y = eX + f          otherwise

    // Assume that the gamma function is continuous, or this won't make much sense anyway.
    // Plug in |d| to the first equation to calculate the new piecewise interval.
    // Then simply use the inverse of the original functions.
    float interval = e * d + f;
    if (x < interval) {
        // X = (Y - F) / E
        if (0.0f == e) {
            // The gamma curve for this segment is constant, so the inverse is undefined.
            // Since this is the lower segment, guess zero.
            return 0.0f;
        }

        return (x - f) / e;
    }

    // X = ((Y - C)^(1 / G) - B) / A
    if (0.0f == a || 0.0f == g) {
        // The gamma curve for this segment is constant, so the inverse is undefined.
        // Since this is the upper segment, guess one.
        return 1.0f;
    }

    return (powf(x - c, 1.0f / g) - b) / a;
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
                               SkColorSpace* space, const GammaFns<T>& fns, bool gammasAreMatching)
{
    switch (as_CSB(space)->gammaNamed()) {
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
            const SkGammas* gammas = as_CSB(space)->gammas();
            SkASSERT(gammas);

            auto build_table = [=](int i) {
                if (gammas->isNamed(i)) {
                    switch (gammas->data(i).fNamed) {
                        case kSRGB_SkGammaNamed:
                            (*fns.fBuildFromParam)(&gammaTableStorage[i * gammaTableSize], 2.4f,
                                                   (1.0f / 1.055f), (0.055f / 1.055f), 0.0f,
                                                   0.04045f, (1.0f / 12.92f), 0.0f);
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
                    const SkGammas::Params& params = gammas->params(i);
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

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool is_almost_identity(const SkMatrix44& srcToDst) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float expected = (i == j) ? 1.0f : 0.0f;
            if (!color_space_almost_equal(srcToDst.getFloat(i,j), expected)) {
                return false;
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(SkColorSpace* srcSpace,
                                                          SkColorSpace* dstSpace) {
    if (!srcSpace || !dstSpace) {
        // Invalid input
        return nullptr;
    }

    ColorSpaceMatch csm = kNone_ColorSpaceMatch;
    SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
    if (SkColorSpace::Equals(srcSpace, dstSpace)) {
        srcToDst.setIdentity();
        csm = kFull_ColorSpaceMatch;
    } else {
        srcToDst.setConcat(as_CSB(dstSpace)->fromXYZD50(), srcSpace->toXYZD50());

        if (is_almost_identity(srcToDst)) {
            srcToDst.setIdentity();
            csm = kGamut_ColorSpaceMatch;
        }
    }

    switch (csm) {
        case kNone_ColorSpaceMatch:
            switch (as_CSB(dstSpace)->gammaNamed()) {
                case kSRGB_SkGammaNamed:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, kSRGB_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, kSRGB_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
                case k2Dot2Curve_SkGammaNamed:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, k2Dot2_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, k2Dot2_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
                case kLinear_SkGammaNamed:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, kLinear_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, kLinear_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
                default:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, kTable_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, kTable_DstGamma, kNone_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
            }
        case kGamut_ColorSpaceMatch:
            switch (as_CSB(dstSpace)->gammaNamed()) {
                case kSRGB_SkGammaNamed:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, kSRGB_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, kSRGB_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
                case k2Dot2Curve_SkGammaNamed:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, k2Dot2_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, k2Dot2_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
                case kLinear_SkGammaNamed:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, kLinear_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, kLinear_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
                default:
                    if (srcSpace->gammaIsLinear()) {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kLinear_SrcGamma, kTable_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    } else {
                        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                                <kTable_SrcGamma, kTable_DstGamma, kGamut_ColorSpaceMatch>
                                (srcSpace, srcToDst, dstSpace));
                    }
            }
        case kFull_ColorSpaceMatch:
            switch (as_CSB(dstSpace)->gammaNamed()) {
                case kSRGB_SkGammaNamed:
                    return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                            <kTable_SrcGamma, kSRGB_DstGamma, kFull_ColorSpaceMatch>
                            (srcSpace, srcToDst, dstSpace));
                case k2Dot2Curve_SkGammaNamed:
                    return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                            <kTable_SrcGamma, k2Dot2_DstGamma, kFull_ColorSpaceMatch>
                            (srcSpace, srcToDst, dstSpace));
                case kLinear_SkGammaNamed:
                    return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                            <kLinear_SrcGamma, kLinear_DstGamma, kFull_ColorSpaceMatch>
                            (srcSpace, srcToDst, dstSpace));
                default:
                    return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                            <kTable_SrcGamma, kTable_DstGamma, kFull_ColorSpaceMatch>
                            (srcSpace, srcToDst, dstSpace));
            }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static float byte_to_float(uint8_t byte) {
    return ((float) byte) * (1.0f / 255.0f);
}

// Clamp to the 0-1 range.
static float clamp_normalized_float(float v) {
    if (v > 1.0f) {
        return 1.0f;
    } else if ((v < 0.0f) || (v != v)) {
        return 0.0f;
    } else {
        return v;
    }
}

static void interp_3d_clut(float dst[3], float src[3], const SkColorLookUpTable* colorLUT) {
    // Call the src components x, y, and z.
    uint8_t maxX = colorLUT->fGridPoints[0] - 1;
    uint8_t maxY = colorLUT->fGridPoints[1] - 1;
    uint8_t maxZ = colorLUT->fGridPoints[2] - 1;

    // An approximate index into each of the three dimensions of the table.
    float x = src[0] * maxX;
    float y = src[1] * maxY;
    float z = src[2] * maxZ;

    // This gives us the low index for our interpolation.
    int ix = sk_float_floor2int(x);
    int iy = sk_float_floor2int(y);
    int iz = sk_float_floor2int(z);

    // Make sure the low index is not also the max index.
    ix = (maxX == ix) ? ix - 1 : ix;
    iy = (maxY == iy) ? iy - 1 : iy;
    iz = (maxZ == iz) ? iz - 1 : iz;

    // Weighting factors for the interpolation.
    float diffX = x - ix;
    float diffY = y - iy;
    float diffZ = z - iz;

    // Constants to help us navigate the 3D table.
    // Ex: Assume x = a, y = b, z = c.
    //     table[a * n001 + b * n010 + c * n100] logically equals table[a][b][c].
    const int n000 = 0;
    const int n001 = 3 * colorLUT->fGridPoints[1] * colorLUT->fGridPoints[2];
    const int n010 = 3 * colorLUT->fGridPoints[2];
    const int n011 = n001 + n010;
    const int n100 = 3;
    const int n101 = n100 + n001;
    const int n110 = n100 + n010;
    const int n111 = n110 + n001;

    // Base ptr into the table.
    const float* ptr = &(colorLUT->table()[ix*n001 + iy*n010 + iz*n100]);

    // The code below performs a tetrahedral interpolation for each of the three
    // dst components.  Once the tetrahedron containing the interpolation point is
    // identified, the interpolation is a weighted sum of grid values at the
    // vertices of the tetrahedron.  The claim is that tetrahedral interpolation
    // provides a more accurate color conversion.
    // blogs.mathworks.com/steve/2006/11/24/tetrahedral-interpolation-for-colorspace-conversion/
    //
    // I have one test image, and visually I can't tell the difference between
    // tetrahedral and trilinear interpolation.  In terms of computation, the
    // tetrahedral code requires more branches but less computation.  The
    // SampleICC library provides an option for the client to choose either
    // tetrahedral or trilinear.
    for (int i = 0; i < 3; i++) {
        if (diffZ < diffY) {
            if (diffZ < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n110] - ptr[n010]) +
                                      diffY * (ptr[n010] - ptr[n000]) +
                                      diffX * (ptr[n111] - ptr[n110]));
            } else if (diffY < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n111] - ptr[n011]) +
                                      diffY * (ptr[n011] - ptr[n001]) +
                                      diffX * (ptr[n001] - ptr[n000]));
            } else {
                dst[i] = (ptr[n000] + diffZ * (ptr[n111] - ptr[n011]) +
                                      diffY * (ptr[n010] - ptr[n000]) +
                                      diffX * (ptr[n011] - ptr[n010]));
            }
        } else {
            if (diffZ < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n101] - ptr[n001]) +
                                      diffY * (ptr[n111] - ptr[n101]) +
                                      diffX * (ptr[n001] - ptr[n000]));
            } else if (diffY < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n100] - ptr[n000]) +
                                      diffY * (ptr[n111] - ptr[n101]) +
                                      diffX * (ptr[n101] - ptr[n100]));
            } else {
                dst[i] = (ptr[n000] + diffZ * (ptr[n100] - ptr[n000]) +
                                      diffY * (ptr[n110] - ptr[n100]) +
                                      diffX * (ptr[n111] - ptr[n110]));
            }
        }

        // Increment the table ptr in order to handle the next component.
        // Note that this is the how table is designed: all of nXXX
        // variables are multiples of 3 because there are 3 output
        // components.
        ptr++;
    }
}

static void handle_color_lut(uint32_t* dst, const uint32_t* src, int len,
                             SkColorLookUpTable* colorLUT) {
    while (len-- > 0) {
        uint8_t r = (*src >>  0) & 0xFF,
                g = (*src >>  8) & 0xFF,
                b = (*src >> 16) & 0xFF;

        float in[3];
        float out[3];
        in[0] = byte_to_float(r);
        in[1] = byte_to_float(g);
        in[2] = byte_to_float(b);
        interp_3d_clut(out, in, colorLUT);

        r = sk_float_round2int(255.0f * clamp_normalized_float(out[0]));
        g = sk_float_round2int(255.0f * clamp_normalized_float(out[1]));
        b = sk_float_round2int(255.0f * clamp_normalized_float(out[2]));
        *dst = SkPackARGB_as_RGBA(0xFF, r, g, b);

        src++;
        dst++;
    }
}

static inline void load_matrix(const float matrix[16],
                               Sk4f& rXgXbX, Sk4f& rYgYbY, Sk4f& rZgZbZ, Sk4f& rTgTbT) {
    rXgXbX = Sk4f::Load(matrix +  0);
    rYgYbY = Sk4f::Load(matrix +  4);
    rZgZbZ = Sk4f::Load(matrix +  8);
    rTgTbT = Sk4f::Load(matrix + 12);
}

static inline void load_rgb_from_tables(const uint32_t* src,
                                        Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                        const float* const srcTables[3]) {
    r = { srcTables[0][(src[0] >>  0) & 0xFF],
          srcTables[0][(src[1] >>  0) & 0xFF],
          srcTables[0][(src[2] >>  0) & 0xFF],
          srcTables[0][(src[3] >>  0) & 0xFF], };
    g = { srcTables[1][(src[0] >>  8) & 0xFF],
          srcTables[1][(src[1] >>  8) & 0xFF],
          srcTables[1][(src[2] >>  8) & 0xFF],
          srcTables[1][(src[3] >>  8) & 0xFF], };
    b = { srcTables[2][(src[0] >> 16) & 0xFF],
          srcTables[2][(src[1] >> 16) & 0xFF],
          srcTables[2][(src[2] >> 16) & 0xFF],
          srcTables[2][(src[3] >> 16) & 0xFF], };
    a = 0.0f; // Don't let the compiler complain that |a| is uninitialized.
}

static inline void load_rgba_from_tables(const uint32_t* src,
                                         Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                         const float* const srcTables[3]) {
    r = { srcTables[0][(src[0] >>  0) & 0xFF],
          srcTables[0][(src[1] >>  0) & 0xFF],
          srcTables[0][(src[2] >>  0) & 0xFF],
          srcTables[0][(src[3] >>  0) & 0xFF], };
    g = { srcTables[1][(src[0] >>  8) & 0xFF],
          srcTables[1][(src[1] >>  8) & 0xFF],
          srcTables[1][(src[2] >>  8) & 0xFF],
          srcTables[1][(src[3] >>  8) & 0xFF], };
    b = { srcTables[2][(src[0] >> 16) & 0xFF],
          srcTables[2][(src[1] >> 16) & 0xFF],
          srcTables[2][(src[2] >> 16) & 0xFF],
          srcTables[2][(src[3] >> 16) & 0xFF], };
    a = (1.0f / 255.0f) * SkNx_cast<float>(Sk4u::Load(src) >> 24);
}

static inline void load_rgb_linear(const uint32_t* src,
                                   Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                   const float* const[3]) {
    r = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src)      ) & 0xFF);
    g = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >>  8) & 0xFF);
    b = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> 16) & 0xFF);
    a = 0.0f; // Don't let the compiler complain that |a| is uninitialized.
}

static inline void load_rgba_linear(const uint32_t* src,
                                    Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                    const float* const[3]) {
    r = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src)      ) & 0xFF);
    g = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >>  8) & 0xFF);
    b = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> 16) & 0xFF);
    a = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> 24)       );
}

static inline void load_rgb_from_tables_1(const uint32_t* src,
                                          Sk4f& r, Sk4f& g, Sk4f& b, Sk4f&,
                                          const float* const srcTables[3]) {
    // Splat r,g,b across a register each.
    r = Sk4f(srcTables[0][(*src >>  0) & 0xFF]);
    g = Sk4f(srcTables[1][(*src >>  8) & 0xFF]);
    b = Sk4f(srcTables[2][(*src >> 16) & 0xFF]);
}

static inline void load_rgba_from_tables_1(const uint32_t* src,
                                           Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                           const float* const srcTables[3]) {
    // Splat r,g,b across a register each.
    r = Sk4f(srcTables[0][(*src >>  0) & 0xFF]);
    g = Sk4f(srcTables[1][(*src >>  8) & 0xFF]);
    b = Sk4f(srcTables[2][(*src >> 16) & 0xFF]);
    a = (1.0f / 255.0f) * Sk4f(*src >> 24);
}

static inline void load_rgb_linear_1(const uint32_t* src,
                                     Sk4f& r, Sk4f& g, Sk4f& b, Sk4f&,
                                     const float* const srcTables[3]) {
    // Splat r,g,b across a register each.
    r = Sk4f((1.0f / 255.0f) * ((*src      ) & 0xFF));
    g = Sk4f((1.0f / 255.0f) * ((*src >>  8) & 0xFF));
    b = Sk4f((1.0f / 255.0f) * ((*src >> 16) & 0xFF));
}

static inline void load_rgba_linear_1(const uint32_t* src,
                                      Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                      const float* const srcTables[3]) {
    // Splat r,g,b,a across a register each.
    r = Sk4f((1.0f / 255.0f) * ((*src      ) & 0xFF));
    g = Sk4f((1.0f / 255.0f) * ((*src >>  8) & 0xFF));
    b = Sk4f((1.0f / 255.0f) * ((*src >> 16) & 0xFF));
    a = Sk4f((1.0f / 255.0f) * ((*src >> 24)       ));
}

static inline void transform_gamut(const Sk4f& r, const Sk4f& g, const Sk4f& b, const Sk4f& a,
                                   const Sk4f& rXgXbX, const Sk4f& rYgYbY, const Sk4f& rZgZbZ,
                                   Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da) {
    dr = rXgXbX[0]*r + rYgYbY[0]*g + rZgZbZ[0]*b;
    dg = rXgXbX[1]*r + rYgYbY[1]*g + rZgZbZ[1]*b;
    db = rXgXbX[2]*r + rYgYbY[2]*g + rZgZbZ[2]*b;
    da = a;
}

static inline void transform_gamut_1(const Sk4f& r, const Sk4f& g, const Sk4f& b,
                                     const Sk4f& rXgXbX, const Sk4f& rYgYbY, const Sk4f& rZgZbZ,
                                     Sk4f& rgba) {
    rgba = rXgXbX*r + rYgYbY*g + rZgZbZ*b;
}

static inline void translate_gamut(const Sk4f& rTgTbT, Sk4f& dr, Sk4f& dg, Sk4f& db) {
    dr = dr + rTgTbT[0];
    dg = dg + rTgTbT[1];
    db = db + rTgTbT[2];
}

static inline void translate_gamut_1(const Sk4f& rTgTbT, Sk4f& rgba) {
    rgba = rgba + rTgTbT;
}

static inline void premultiply(Sk4f& dr, Sk4f& dg, Sk4f& db, const Sk4f& da) {
    dr = da * dr;
    dg = da * dg;
    db = da * db;
}

static inline void premultiply_1(const Sk4f& a, Sk4f& rgba) {
    rgba = a * rgba;
}

enum SwapRB {
    kNo_SwapRB,
    kYes_SwapRB,
};

template <SwapRB kSwapRB>
static inline void store_srgb(void* dst, const uint32_t* src,
                              Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                              const uint8_t* const[3]) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    dr = sk_linear_to_srgb_needs_trunc(dr);
    dg = sk_linear_to_srgb_needs_trunc(dg);
    db = sk_linear_to_srgb_needs_trunc(db);

    dr = sk_clamp_0_255(dr);
    dg = sk_clamp_0_255(dg);
    db = sk_clamp_0_255(db);

    Sk4i da = Sk4i::Load(src) & 0xFF000000;

    Sk4i rgba = (SkNx_cast<int>(dr) << kRShift)
              | (SkNx_cast<int>(dg) << kGShift)
              | (SkNx_cast<int>(db) << kBShift)
              | (da                           );
    rgba.store(dst);
}

template <SwapRB kSwapRB>
static inline void store_srgb_1(void* dst, const uint32_t* src,
                                Sk4f& rgba, const Sk4f&,
                                const uint8_t* const[3]) {
    rgba = sk_clamp_0_255(sk_linear_to_srgb_needs_trunc(rgba));

    uint32_t tmp;
    SkNx_cast<uint8_t>(SkNx_cast<int32_t>(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kYes_SwapRB == kSwapRB) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

static inline Sk4f linear_to_2dot2(const Sk4f& x) {
    // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
    auto x2  = x.rsqrt(),                            // x^(-1/2)
         x32 = x2.rsqrt().rsqrt().rsqrt().rsqrt(),   // x^(-1/32)
         x64 = x32.rsqrt();                          // x^(+1/64)

    // 29 = 32 - 2 - 1
    return 255.0f * x2.invert() * x32 * x64.invert();
}

template <SwapRB kSwapRB>
static inline void store_2dot2(void* dst, const uint32_t* src,
                               Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                               const uint8_t* const[3]) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    dr = linear_to_2dot2(dr);
    dg = linear_to_2dot2(dg);
    db = linear_to_2dot2(db);

    dr = sk_clamp_0_255(dr);
    dg = sk_clamp_0_255(dg);
    db = sk_clamp_0_255(db);

    Sk4i da = Sk4i::Load(src) & 0xFF000000;

    Sk4i rgba = (Sk4f_round(dr) << kRShift)
              | (Sk4f_round(dg) << kGShift)
              | (Sk4f_round(db) << kBShift)
              | (da                       );
    rgba.store(dst);
}

template <SwapRB kSwapRB>
static inline void store_2dot2_1(void* dst, const uint32_t* src,
                                 Sk4f& rgba, const Sk4f&,
                                 const uint8_t* const[3]) {
    rgba = sk_clamp_0_255(linear_to_2dot2(rgba));

    uint32_t tmp;
    SkNx_cast<uint8_t>(Sk4f_round(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kYes_SwapRB == kSwapRB) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

template <SwapRB kSwapRB>
static inline void store_linear(void* dst, const uint32_t* src,
                               Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                               const uint8_t* const[3]) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    dr = sk_clamp_0_255(255.0f * dr);
    dg = sk_clamp_0_255(255.0f * dg);
    db = sk_clamp_0_255(255.0f * db);

    Sk4i da = Sk4i::Load(src) & 0xFF000000;

    Sk4i rgba = (Sk4f_round(dr) << kRShift)
              | (Sk4f_round(dg) << kGShift)
              | (Sk4f_round(db) << kBShift)
              | (da                       );
    rgba.store(dst);
}

template <SwapRB kSwapRB>
static inline void store_linear_1(void* dst, const uint32_t* src,
                                  Sk4f& rgba, const Sk4f&,
                                  const uint8_t* const[3]) {
    rgba = sk_clamp_0_255(255.0f * rgba);

    uint32_t tmp;
    SkNx_cast<uint8_t>(Sk4f_round(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kYes_SwapRB == kSwapRB) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

template <SwapRB kSwapRB>
static inline void store_f16(void* dst, const uint32_t* src,
                             Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da,
                             const uint8_t* const[3]) {
    Sk4h_store4(dst, SkFloatToHalf_finite_ftz(dr),
                     SkFloatToHalf_finite_ftz(dg),
                     SkFloatToHalf_finite_ftz(db),
                     SkFloatToHalf_finite_ftz(da));
}

template <SwapRB kSwapRB>
static inline void store_f16_1(void* dst, const uint32_t* src,
                               Sk4f& rgba, const Sk4f& a,
                               const uint8_t* const[3]) {
    rgba = Sk4f(rgba[0], rgba[1], rgba[2], a[3]);
    SkFloatToHalf_finite_ftz(rgba).store((uint64_t*) dst);
}

template <SwapRB kSwapRB>
static inline void store_f32(void* dst, const uint32_t* src,
                             Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da,
                             const uint8_t* const[3]) {
    Sk4f_store4(dst, dr, dg, db, da);
}

template <SwapRB kSwapRB>
static inline void store_f32_1(void* dst, const uint32_t* src,
                               Sk4f& rgba, const Sk4f& a,
                               const uint8_t* const[3]) {
    rgba = Sk4f(rgba[0], rgba[1], rgba[2], a[3]);
    rgba.store((float*) dst);
}

template <SwapRB kSwapRB>
static inline void store_f16_opaque(void* dst, const uint32_t* src,
                                    Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                                    const uint8_t* const[3]) {
    Sk4h_store4(dst, SkFloatToHalf_finite_ftz(dr),
                     SkFloatToHalf_finite_ftz(dg),
                     SkFloatToHalf_finite_ftz(db),
                     SK_Half1);
}

template <SwapRB kSwapRB>
static inline void store_f16_1_opaque(void* dst, const uint32_t* src,
                                      Sk4f& rgba, const Sk4f&,
                                      const uint8_t* const[3]) {
    uint64_t tmp;
    SkFloatToHalf_finite_ftz(rgba).store(&tmp);
    tmp |= static_cast<uint64_t>(SK_Half1) << 48;
    *((uint64_t*) dst) = tmp;
}

template <SwapRB kSwapRB>
static inline void store_generic(void* dst, const uint32_t* src,
                                 Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                                 const uint8_t* const dstTables[3]) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    dr = Sk4f::Min(Sk4f::Max(1023.0f * dr, 0.0f), 1023.0f);
    dg = Sk4f::Min(Sk4f::Max(1023.0f * dg, 0.0f), 1023.0f);
    db = Sk4f::Min(Sk4f::Max(1023.0f * db, 0.0f), 1023.0f);

    Sk4i ir = Sk4f_round(dr);
    Sk4i ig = Sk4f_round(dg);
    Sk4i ib = Sk4f_round(db);

    Sk4i da = Sk4i::Load(src) & 0xFF000000;

    uint32_t* dst32 = (uint32_t*) dst;
    dst32[0] = dstTables[0][ir[0]] << kRShift
             | dstTables[1][ig[0]] << kGShift
             | dstTables[2][ib[0]] << kBShift
             | da[0];
    dst32[1] = dstTables[0][ir[1]] << kRShift
             | dstTables[1][ig[1]] << kGShift
             | dstTables[2][ib[1]] << kBShift
             | da[1];
    dst32[2] = dstTables[0][ir[2]] << kRShift
             | dstTables[1][ig[2]] << kGShift
             | dstTables[2][ib[2]] << kBShift
             | da[2];
    dst32[3] = dstTables[0][ir[3]] << kRShift
             | dstTables[1][ig[3]] << kGShift
             | dstTables[2][ib[3]] << kBShift
             | da[3];
}

template <SwapRB kSwapRB>
static inline void store_generic_1(void* dst, const uint32_t* src,
                                   Sk4f& rgba, const Sk4f&,
                                   const uint8_t* const dstTables[3]) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    rgba = Sk4f::Min(Sk4f::Max(1023.0f * rgba, 0.0f), 1023.0f);

    Sk4i indices = Sk4f_round(rgba);

    *((uint32_t*) dst) = dstTables[0][indices[0]] << kRShift
                       | dstTables[1][indices[1]] << kGShift
                       | dstTables[2][indices[2]] << kBShift
                       | (*src & 0xFF000000);
}

typedef decltype(load_rgb_from_tables       )* LoadFn;
typedef decltype(load_rgb_from_tables_1     )* Load1Fn;
typedef decltype(store_generic<kNo_SwapRB>  )* StoreFn;
typedef decltype(store_generic_1<kNo_SwapRB>)* Store1Fn;

template <SkAlphaType kAlphaType,
          ColorSpaceMatch kCSM>
static inline void do_color_xform(void* dst, const uint32_t* src, int len,
                                  const float* const srcTables[3], const float matrix[16],
                                  const uint8_t* const dstTables[3], LoadFn load, Load1Fn load_1,
                                  StoreFn store, Store1Fn store_1, size_t sizeOfDstPixel) {
    Sk4f rXgXbX, rYgYbY, rZgZbZ, rTgTbT;
    load_matrix(matrix, rXgXbX, rYgYbY, rZgZbZ, rTgTbT);

    if (len >= 4) {
        // Naively this would be a loop of load-transform-store, but we found it faster to
        // move the N+1th load ahead of the Nth store.  We don't bother doing this for N<4.
        Sk4f r, g, b, a;
        load(src, r, g, b, a, srcTables);
        src += 4;
        len -= 4;

        Sk4f dr, dg, db, da;
        while (len >= 4) {
            if (kNone_ColorSpaceMatch == kCSM) {
                transform_gamut(r, g, b, a, rXgXbX, rYgYbY, rZgZbZ, dr, dg, db, da);
                translate_gamut(rTgTbT, dr, dg, db);
            } else {
                dr = r;
                dg = g;
                db = b;
                da = a;
            }

            if (kPremul_SkAlphaType == kAlphaType) {
                premultiply(dr, dg, db, da);
            }

            load(src, r, g, b, a, srcTables);

            store(dst, src - 4, dr, dg, db, da, dstTables);
            dst = SkTAddOffset<void>(dst, 4 * sizeOfDstPixel);
            src += 4;
            len -= 4;
        }

        if (kNone_ColorSpaceMatch == kCSM) {
            transform_gamut(r, g, b, a, rXgXbX, rYgYbY, rZgZbZ, dr, dg, db, da);
            translate_gamut(rTgTbT, dr, dg, db);
        } else {
            dr = r;
            dg = g;
            db = b;
            da = a;
        }

        if (kPremul_SkAlphaType == kAlphaType) {
            premultiply(dr, dg, db, da);
        }

        store(dst, src - 4, dr, dg, db, da, dstTables);
        dst = SkTAddOffset<void>(dst, 4 * sizeOfDstPixel);
    }

    while (len > 0) {
        Sk4f r, g, b, a;
        load_1(src, r, g, b, a, srcTables);

        Sk4f rgba;
        if (kNone_ColorSpaceMatch == kCSM) {
            transform_gamut_1(r, g, b, rXgXbX, rYgYbY, rZgZbZ, rgba);
            translate_gamut_1(rTgTbT, rgba);
        } else {
            rgba = Sk4f(r[0], g[0], b[0], a[0]);
        }

        if (kPremul_SkAlphaType == kAlphaType) {
            premultiply_1(a, rgba);
        }

        store_1(dst, src, rgba, a, dstTables);

        src += 1;
        len -= 1;
        dst = SkTAddOffset<void>(dst, sizeOfDstPixel);
    }
}

enum SrcFormat {
    kRGBA_8888_Linear_SrcFormat,
    kRGBA_8888_Table_SrcFormat,
};

enum DstFormat {
    k8888_Linear_DstFormat,
    k8888_SRGB_DstFormat,
    k8888_2Dot2_DstFormat,
    k8888_Table_DstFormat,
    kF16_Linear_DstFormat,
    kF32_Linear_DstFormat,
};

template <SrcFormat kSrc,
          DstFormat kDst,
          SkAlphaType kAlphaType,
          ColorSpaceMatch kCSM,
          SwapRB kSwapRB>
static void color_xform_RGBA(void* dst, const uint32_t* src, int len,
                             const float* const srcTables[3], const float matrix[16],
                             const uint8_t* const dstTables[3]) {
    LoadFn load;
    Load1Fn load_1;
    static constexpr bool loadAlpha = (kPremul_SkAlphaType == kAlphaType) ||
                                      (kF16_Linear_DstFormat == kDst) ||
                                      (kF32_Linear_DstFormat == kDst);
    switch (kSrc) {
        case kRGBA_8888_Linear_SrcFormat:
            if (loadAlpha) {
                load = load_rgba_linear;
                load_1 = load_rgba_linear_1;
            } else {
                load = load_rgb_linear;
                load_1 = load_rgb_linear_1;
            }
            break;
        case kRGBA_8888_Table_SrcFormat:
            if (loadAlpha) {
                load = load_rgba_from_tables;
                load_1 = load_rgba_from_tables_1;
            } else {
                load = load_rgb_from_tables;
                load_1 = load_rgb_from_tables_1;
            }
            break;
    }

    StoreFn store;
    Store1Fn store_1;
    size_t sizeOfDstPixel;
    switch (kDst) {
        case k8888_Linear_DstFormat:
            store   = store_linear<kSwapRB>;
            store_1 = store_linear_1<kSwapRB>;
            sizeOfDstPixel = 4;
            break;
        case k8888_SRGB_DstFormat:
            store   = store_srgb<kSwapRB>;
            store_1 = store_srgb_1<kSwapRB>;
            sizeOfDstPixel = 4;
            break;
        case k8888_2Dot2_DstFormat:
            store   = store_2dot2<kSwapRB>;
            store_1 = store_2dot2_1<kSwapRB>;
            sizeOfDstPixel = 4;
            break;
        case k8888_Table_DstFormat:
            store   = store_generic<kSwapRB>;
            store_1 = store_generic_1<kSwapRB>;
            sizeOfDstPixel = 4;
            break;
        case kF16_Linear_DstFormat:
            store   = (kOpaque_SkAlphaType == kAlphaType) ? store_f16_opaque<kSwapRB> :
                                                            store_f16<kSwapRB>;
            store_1 = (kOpaque_SkAlphaType == kAlphaType) ? store_f16_1_opaque<kSwapRB> :
                                                            store_f16_1<kSwapRB>;
            sizeOfDstPixel = 8;
            break;
        case kF32_Linear_DstFormat:
            store   = store_f32<kSwapRB>;
            store_1 = store_f32_1<kSwapRB>;
            sizeOfDstPixel = 16;
            break;
    }

    do_color_xform<kAlphaType, kCSM>
            (dst, src, len, srcTables, matrix, dstTables, load, load_1, store, store_1,
             sizeOfDstPixel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline int num_tables(SkColorSpace* space) {
    switch (as_CSB(space)->gammaNamed()) {
        case kSRGB_SkGammaNamed:
        case k2Dot2Curve_SkGammaNamed:
        case kLinear_SkGammaNamed:
            return 0;
        default: {
            const SkGammas* gammas = as_CSB(space)->gammas();
            SkASSERT(gammas);

            bool gammasAreMatching = (gammas->type(0) == gammas->type(1)) &&
                                     (gammas->data(0) == gammas->data(1)) &&
                                     (gammas->type(0) == gammas->type(2)) &&
                                     (gammas->data(0) == gammas->data(2));

            // It's likely that each component will have the same gamma.  In this case,
            // we only need to build one table.
            return gammasAreMatching ? 1 : 3;
        }
    }
}

template <SrcGamma kSrc, DstGamma kDst, ColorSpaceMatch kCSM>
SkColorSpaceXform_Base<kSrc, kDst, kCSM>
::SkColorSpaceXform_Base(SkColorSpace* srcSpace, const SkMatrix44& srcToDst, SkColorSpace* dstSpace)
    : fColorLUT(sk_ref_sp((SkColorLookUpTable*) as_CSB(srcSpace)->colorLUT()))
{
    srcToDst.asColMajorf(fSrcToDst);

    const int numSrcTables = num_tables(srcSpace);
    const int numDstTables = num_tables(dstSpace);
    const size_t srcTableBytes = numSrcTables * 256 * sizeof(float);
    const size_t dstTableBytes = numDstTables * kDstGammaTableSize * sizeof(uint8_t);
    fStorage.reset(srcTableBytes + dstTableBytes);
    float* srcStorage = (float*) fStorage.get();
    uint8_t* dstStorage = SkTAddOffset<uint8_t>(fStorage.get(), srcTableBytes);

    const bool srcGammasAreMatching = (1 >= numSrcTables);
    const bool dstGammasAreMatching = (1 >= numDstTables);
    build_gamma_tables(fSrcGammaTables, srcStorage, 256, srcSpace, kToLinear, srcGammasAreMatching);
    build_gamma_tables(fDstGammaTables, dstStorage, kDstGammaTableSize, dstSpace, kFromLinear,
                       dstGammasAreMatching);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <SrcFormat kSrc, DstFormat kDst, ColorSpaceMatch kCSM, SwapRB kSwap>
static inline void apply_set_alpha(void* dst, const uint32_t* src, int len, SkAlphaType alphaType,
                                   const float* const srcTables[3], const float matrix[16],
                                   const uint8_t* const dstTables[3]) {
    switch (alphaType) {
        case kOpaque_SkAlphaType:
            return color_xform_RGBA<kSrc, kDst, kOpaque_SkAlphaType, kCSM, kSwap>
                    (dst, src, len, srcTables, matrix, dstTables);
        case kPremul_SkAlphaType:
            return color_xform_RGBA<kSrc, kDst, kPremul_SkAlphaType, kCSM, kSwap>
                    (dst, src, len, srcTables, matrix, dstTables);
        case kUnpremul_SkAlphaType:
            return color_xform_RGBA<kSrc, kDst, kUnpremul_SkAlphaType, kCSM, kSwap>
                    (dst, src, len, srcTables, matrix, dstTables);
        default:
            SkASSERT(false);
            return;
    }
}

template <SrcGamma kSrc, DstFormat kDst, ColorSpaceMatch kCSM, SwapRB kSwap>
static inline void apply_set_src(void* dst, const uint32_t* src, int len, SkAlphaType alphaType,
                                 const float* const srcTables[3], const float matrix[16],
                                 const uint8_t* const dstTables[3]) {
    switch (kSrc) {
        case kLinear_SrcGamma:
            return apply_set_alpha<kRGBA_8888_Linear_SrcFormat, kDst, kCSM, kSwap>
                    (dst, src, len, alphaType, nullptr, matrix, dstTables);
        case kTable_SrcGamma:
            return apply_set_alpha<kRGBA_8888_Table_SrcFormat, kDst, kCSM, kSwap>
                    (dst, src, len, alphaType, srcTables, matrix, dstTables);
    }
}

template <SrcGamma kSrc, DstGamma kDst, ColorSpaceMatch kCSM>
void SkColorSpaceXform_Base<kSrc, kDst, kCSM>
::apply(void* dst, const uint32_t* src, int len, ColorFormat dstColorFormat, SkAlphaType alphaType)
const
{
    if (kFull_ColorSpaceMatch == kCSM) {
        switch (alphaType) {
            case kPremul_SkAlphaType:
                // We can't skip the xform since we need to perform a premultiply in the
                // linear space.
                break;
            default:
                switch (dstColorFormat) {
                    case kRGBA_8888_ColorFormat:
                        return (void) memcpy(dst, src, len * sizeof(uint32_t));
                    case kBGRA_8888_ColorFormat:
                        return SkOpts::RGBA_to_BGRA((uint32_t*) dst, src, len);
                    case kRGBA_F16_ColorFormat:
                    case kRGBA_F32_ColorFormat:
                        // There's still work to do to xform to linear floats.
                        break;
                    default:
                        SkASSERT(false);
                        return;
                }
        }
    }

    if (fColorLUT) {
        size_t storageBytes = len * sizeof(uint32_t);
#if defined(GOOGLE3)
        // Stack frame size is limited in GOOGLE3.
        SkAutoSMalloc<256 * sizeof(uint32_t)> storage(storageBytes);
#else
        SkAutoSMalloc<1024 * sizeof(uint32_t)> storage(storageBytes);
#endif

        handle_color_lut((uint32_t*) storage.get(), src, len, fColorLUT.get());
        src = (const uint32_t*) storage.get();
    }

    switch (dstColorFormat) {
        case kRGBA_8888_ColorFormat:
            switch (kDst) {
                case kLinear_DstGamma:
                    return apply_set_src<kSrc, k8888_Linear_DstFormat, kCSM, kNo_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                case kSRGB_DstGamma:
                    return apply_set_src<kSrc, k8888_SRGB_DstFormat, kCSM, kNo_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                case k2Dot2_DstGamma:
                    return apply_set_src<kSrc, k8888_2Dot2_DstFormat, kCSM, kNo_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                case kTable_DstGamma:
                    return apply_set_src<kSrc, k8888_Table_DstFormat, kCSM, kNo_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, fDstGammaTables);
            }
        case kBGRA_8888_ColorFormat:
            switch (kDst) {
                case kLinear_DstGamma:
                    return apply_set_src<kSrc, k8888_Linear_DstFormat, kCSM, kYes_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                case kSRGB_DstGamma:
                    return apply_set_src<kSrc, k8888_SRGB_DstFormat, kCSM, kYes_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                case k2Dot2_DstGamma:
                    return apply_set_src<kSrc, k8888_2Dot2_DstFormat, kCSM, kYes_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                case kTable_DstGamma:
                    return apply_set_src<kSrc, k8888_Table_DstFormat, kCSM, kYes_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, fDstGammaTables);
            }
        case kRGBA_F16_ColorFormat:
            switch (kDst) {
                case kLinear_DstGamma:
                    return apply_set_src<kSrc, kF16_Linear_DstFormat, kCSM, kNo_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                default:
                    SkASSERT(false);
                    return;
            }
        case kRGBA_F32_ColorFormat:
            switch (kDst) {
                case kLinear_DstGamma:
                    return apply_set_src<kSrc, kF32_Linear_DstFormat, kCSM, kNo_SwapRB>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr);
                default:
                    SkASSERT(false);
                    return;
            }
        default:
            SkASSERT(false);
            return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace* space) {
        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_Base
                <kTable_SrcGamma, kTable_DstGamma, kNone_ColorSpaceMatch>
                (space, SkMatrix::I(), space));
}
