/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXform_A2B.h"
#include "SkColorSpaceXform_Base.h"
#include "SkColorSpaceXformPriv.h"
#include "SkHalf.h"
#include "SkOpts.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
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
    // Y = (aX + b)^g + c  for X >= d
    // Y = eX + f          otherwise

    // Assume that the gamma function is continuous, or this won't make much sense anyway.
    // Plug in |d| to the first equation to calculate the new piecewise interval.
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
                            (*fns.fBuildFromParam)(&gammaTableStorage[i * gammaTableSize], 2.4f,
                                                   (1.0f / 1.055f), (0.055f / 1.055f),
                                                   (1.0f / 12.92f), 0.04045f, 0.0f, 0.0f);
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

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(SkColorSpace* srcSpace,
                                                          SkColorSpace* dstSpace) {
    return SkColorSpaceXform_Base::New(srcSpace, dstSpace, SkTransferFunctionBehavior::kRespect);
}

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform_Base::New(SkColorSpace* srcSpace,
        SkColorSpace* dstSpace, SkTransferFunctionBehavior premulBehavior) {

    if (!srcSpace || !dstSpace) {
        // Invalid input
        return nullptr;
    }

    if (SkColorSpace_Base::Type::kA2B == as_CSB(dstSpace)->type()) {
        SkCSXformPrintf("A2B destinations not supported\n");
        return nullptr;
    }

    if (SkColorSpace_Base::Type::kA2B == as_CSB(srcSpace)->type()) {
        SkColorSpace_A2B* src = static_cast<SkColorSpace_A2B*>(srcSpace);
        SkColorSpace_XYZ* dst = static_cast<SkColorSpace_XYZ*>(dstSpace);
        return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_A2B(src, dst));
    }
    SkColorSpace_XYZ* srcSpaceXYZ = static_cast<SkColorSpace_XYZ*>(srcSpace);
    SkColorSpace_XYZ* dstSpaceXYZ = static_cast<SkColorSpace_XYZ*>(dstSpace);

    ColorSpaceMatch csm = kNone_ColorSpaceMatch;
    SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
    if (SkColorSpace::Equals(srcSpace, dstSpace)) {
        srcToDst.setIdentity();
        csm = kFull_ColorSpaceMatch;
    } else {
        if (srcSpaceXYZ->toXYZD50Hash() == dstSpaceXYZ->toXYZD50Hash()) {
            SkASSERT(*srcSpaceXYZ->toXYZD50() == *dstSpaceXYZ->toXYZD50() && "Hash collision");
            srcToDst.setIdentity();
            csm = kGamut_ColorSpaceMatch;
        } else {
            srcToDst.setConcat(*dstSpaceXYZ->fromXYZD50(), *srcSpaceXYZ->toXYZD50());
        }
    }

    switch (csm) {
        case kNone_ColorSpaceMatch:
            return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_XYZ
                    <kNone_ColorSpaceMatch>(srcSpaceXYZ, srcToDst, dstSpaceXYZ, premulBehavior));
        case kGamut_ColorSpaceMatch:
            return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_XYZ
                    <kGamut_ColorSpaceMatch>(srcSpaceXYZ, srcToDst, dstSpaceXYZ, premulBehavior));
        case kFull_ColorSpaceMatch:
            return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_XYZ
                    <kFull_ColorSpaceMatch>(srcSpaceXYZ, srcToDst, dstSpaceXYZ, premulBehavior));
        default:
            SkASSERT(false);
            return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define AI SK_ALWAYS_INLINE

static AI void load_matrix(const float matrix[13],
                           Sk4f& rXgXbX, Sk4f& rYgYbY, Sk4f& rZgZbZ, Sk4f& rTgTbT) {
    rXgXbX = Sk4f::Load(matrix + 0);
    rYgYbY = Sk4f::Load(matrix + 3);
    rZgZbZ = Sk4f::Load(matrix + 6);
    rTgTbT = Sk4f::Load(matrix + 9);
}

enum Order {
    kRGBA_Order,
    kBGRA_Order,
};

static AI void set_rb_shifts(Order kOrder, int* kRShift, int* kBShift) {
    if (kRGBA_Order == kOrder) {
        *kRShift = 0;
        *kBShift = 16;
    } else {
        *kRShift = 16;
        *kBShift = 0;
    }
}

template <Order kOrder>
static AI void load_rgb_from_tables(const uint32_t* src,
                                    Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                    const float* const srcTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = { srcTables[0][(src[0] >> kRShift) & 0xFF],
          srcTables[0][(src[1] >> kRShift) & 0xFF],
          srcTables[0][(src[2] >> kRShift) & 0xFF],
          srcTables[0][(src[3] >> kRShift) & 0xFF], };
    g = { srcTables[1][(src[0] >> kGShift) & 0xFF],
          srcTables[1][(src[1] >> kGShift) & 0xFF],
          srcTables[1][(src[2] >> kGShift) & 0xFF],
          srcTables[1][(src[3] >> kGShift) & 0xFF], };
    b = { srcTables[2][(src[0] >> kBShift) & 0xFF],
          srcTables[2][(src[1] >> kBShift) & 0xFF],
          srcTables[2][(src[2] >> kBShift) & 0xFF],
          srcTables[2][(src[3] >> kBShift) & 0xFF], };
    a = 0.0f; // Don't let the compiler complain that |a| is uninitialized.
}

template <Order kOrder>
static AI void load_rgba_from_tables(const uint32_t* src,
                                     Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                     const float* const srcTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = { srcTables[0][(src[0] >> kRShift) & 0xFF],
          srcTables[0][(src[1] >> kRShift) & 0xFF],
          srcTables[0][(src[2] >> kRShift) & 0xFF],
          srcTables[0][(src[3] >> kRShift) & 0xFF], };
    g = { srcTables[1][(src[0] >> kGShift) & 0xFF],
          srcTables[1][(src[1] >> kGShift) & 0xFF],
          srcTables[1][(src[2] >> kGShift) & 0xFF],
          srcTables[1][(src[3] >> kGShift) & 0xFF], };
    b = { srcTables[2][(src[0] >> kBShift) & 0xFF],
          srcTables[2][(src[1] >> kBShift) & 0xFF],
          srcTables[2][(src[2] >> kBShift) & 0xFF],
          srcTables[2][(src[3] >> kBShift) & 0xFF], };
    a = (1.0f / 255.0f) * SkNx_cast<float>(Sk4u::Load(src) >> 24);
}

template <Order kOrder>
static AI void load_rgb_linear(const uint32_t* src, Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                               const float* const[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> kRShift) & 0xFF);
    g = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> kGShift) & 0xFF);
    b = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> kBShift) & 0xFF);
    a = 0.0f; // Don't let the compiler complain that |a| is uninitialized.
}

template <Order kOrder>
static AI void load_rgba_linear(const uint32_t* src, Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                const float* const[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> kRShift) & 0xFF);
    g = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> kGShift) & 0xFF);
    b = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> kBShift) & 0xFF);
    a = (1.0f / 255.0f) * SkNx_cast<float>((Sk4u::Load(src) >> 24));
}

template <Order kOrder>
static AI void load_rgb_from_tables_1(const uint32_t* src,
                                      Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                      const float* const srcTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = Sk4f(srcTables[0][(*src >> kRShift) & 0xFF]);
    g = Sk4f(srcTables[1][(*src >> kGShift) & 0xFF]);
    b = Sk4f(srcTables[2][(*src >> kBShift) & 0xFF]);
    a = 0.0f; // Don't let MSAN complain that |a| is uninitialized.
}

template <Order kOrder>
static AI void load_rgba_from_tables_1(const uint32_t* src,
                                       Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                       const float* const srcTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = Sk4f(srcTables[0][(*src >> kRShift) & 0xFF]);
    g = Sk4f(srcTables[1][(*src >> kGShift) & 0xFF]);
    b = Sk4f(srcTables[2][(*src >> kBShift) & 0xFF]);
    a = (1.0f / 255.0f) * Sk4f(*src >> 24);
}

template <Order kOrder>
static AI void load_rgb_linear_1(const uint32_t* src,
                                 Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                 const float* const srcTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = Sk4f((1.0f / 255.0f) * ((*src >> kRShift) & 0xFF));
    g = Sk4f((1.0f / 255.0f) * ((*src >> kGShift) & 0xFF));
    b = Sk4f((1.0f / 255.0f) * ((*src >> kBShift) & 0xFF));
    a = 0.0f; // Don't let MSAN complain that |a| is uninitialized.
}

template <Order kOrder>
static AI void load_rgba_linear_1(const uint32_t* src,
                                  Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                  const float* const srcTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    r = Sk4f((1.0f / 255.0f) * ((*src >> kRShift) & 0xFF));
    g = Sk4f((1.0f / 255.0f) * ((*src >> kGShift) & 0xFF));
    b = Sk4f((1.0f / 255.0f) * ((*src >> kBShift) & 0xFF));
    a = Sk4f((1.0f / 255.0f) * ((*src >> 24)));
}

static AI void transform_gamut(const Sk4f& r, const Sk4f& g, const Sk4f& b, const Sk4f& a,
                               const Sk4f& rXgXbX, const Sk4f& rYgYbY, const Sk4f& rZgZbZ,
                               Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da) {
    dr = rXgXbX[0]*r + rYgYbY[0]*g + rZgZbZ[0]*b;
    dg = rXgXbX[1]*r + rYgYbY[1]*g + rZgZbZ[1]*b;
    db = rXgXbX[2]*r + rYgYbY[2]*g + rZgZbZ[2]*b;
    da = a;
}

static AI void transform_gamut_1(const Sk4f& r, const Sk4f& g, const Sk4f& b,
                                 const Sk4f& rXgXbX, const Sk4f& rYgYbY, const Sk4f& rZgZbZ,
                                 Sk4f& rgba) {
    rgba = rXgXbX*r + rYgYbY*g + rZgZbZ*b;
}

static AI void translate_gamut(const Sk4f& rTgTbT, Sk4f& dr, Sk4f& dg, Sk4f& db) {
    dr = dr + rTgTbT[0];
    dg = dg + rTgTbT[1];
    db = db + rTgTbT[2];
}

static AI void translate_gamut_1(const Sk4f& rTgTbT, Sk4f& rgba) {
    rgba = rgba + rTgTbT;
}

template <Order kOrder>
static AI void store_srgb(void* dst, const uint32_t* src, Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                          const uint8_t* const[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
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

template <Order kOrder>
static AI void store_srgb_1(void* dst, const uint32_t* src,
                            Sk4f& rgba, const Sk4f&,
                            const uint8_t* const[3]) {
    rgba = sk_clamp_0_255(sk_linear_to_srgb_needs_trunc(rgba));

    uint32_t tmp;
    SkNx_cast<uint8_t>(SkNx_cast<int32_t>(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kBGRA_Order == kOrder) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

static AI Sk4f linear_to_2dot2(const Sk4f& x) {
    // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
    auto x2  = x.rsqrt(),                            // x^(-1/2)
         x32 = x2.rsqrt().rsqrt().rsqrt().rsqrt(),   // x^(-1/32)
         x64 = x32.rsqrt();                          // x^(+1/64)

    // 29 = 32 - 2 - 1
    return 255.0f * x2.invert() * x32 * x64.invert();
}

template <Order kOrder>
static AI void store_2dot2(void* dst, const uint32_t* src, Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                           const uint8_t* const[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
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

template <Order kOrder>
static AI void store_2dot2_1(void* dst, const uint32_t* src,
                             Sk4f& rgba, const Sk4f&,
                             const uint8_t* const[3]) {
    rgba = sk_clamp_0_255(linear_to_2dot2(rgba));

    uint32_t tmp;
    SkNx_cast<uint8_t>(Sk4f_round(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kBGRA_Order == kOrder) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

template <Order kOrder>
static AI void store_linear(void* dst, const uint32_t* src, Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                            const uint8_t* const[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
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

template <Order kOrder>
static AI void store_linear_1(void* dst, const uint32_t* src,
                              Sk4f& rgba, const Sk4f&,
                              const uint8_t* const[3]) {
    rgba = sk_clamp_0_255(255.0f * rgba);

    uint32_t tmp;
    SkNx_cast<uint8_t>(Sk4f_round(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kBGRA_Order == kOrder) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

template <Order kOrder>
static AI void store_f16(void* dst, const uint32_t* src, Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da,
                         const uint8_t* const[3]) {
    Sk4h::Store4(dst, SkFloatToHalf_finite_ftz(dr),
                      SkFloatToHalf_finite_ftz(dg),
                      SkFloatToHalf_finite_ftz(db),
                      SkFloatToHalf_finite_ftz(da));
}

template <Order kOrder>
static AI void store_f16_1(void* dst, const uint32_t* src,
                           Sk4f& rgba, const Sk4f& a,
                           const uint8_t* const[3]) {
    rgba = Sk4f(rgba[0], rgba[1], rgba[2], a[3]);
    SkFloatToHalf_finite_ftz(rgba).store((uint64_t*) dst);
}

template <Order kOrder>
static AI void store_f16_opaque(void* dst, const uint32_t* src, Sk4f& dr, Sk4f& dg, Sk4f& db,
                                Sk4f&, const uint8_t* const[3]) {
    Sk4h::Store4(dst, SkFloatToHalf_finite_ftz(dr),
                      SkFloatToHalf_finite_ftz(dg),
                      SkFloatToHalf_finite_ftz(db),
                      SK_Half1);
}

template <Order kOrder>
static AI void store_f16_1_opaque(void* dst, const uint32_t* src,
                                  Sk4f& rgba, const Sk4f&,
                                  const uint8_t* const[3]) {
    uint64_t tmp;
    SkFloatToHalf_finite_ftz(rgba).store(&tmp);
    tmp &= 0x0000FFFFFFFFFFFF;
    tmp |= static_cast<uint64_t>(SK_Half1) << 48;
    *((uint64_t*) dst) = tmp;
}

template <Order kOrder>
static AI void store_generic(void* dst, const uint32_t* src, Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                             const uint8_t* const dstTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
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

template <Order kOrder>
static AI void store_generic_1(void* dst, const uint32_t* src,
                               Sk4f& rgba, const Sk4f&,
                               const uint8_t* const dstTables[3]) {
    int kRShift, kGShift = 8, kBShift;
    set_rb_shifts(kOrder, &kRShift, &kBShift);
    rgba = Sk4f::Min(Sk4f::Max(1023.0f * rgba, 0.0f), 1023.0f);

    Sk4i indices = Sk4f_round(rgba);

    *((uint32_t*) dst) = dstTables[0][indices[0]] << kRShift
                       | dstTables[1][indices[1]] << kGShift
                       | dstTables[2][indices[2]] << kBShift
                       | (*src & 0xFF000000);
}

typedef decltype(load_rgb_from_tables<kRGBA_Order>  )* LoadFn;
typedef decltype(load_rgb_from_tables_1<kRGBA_Order>)* Load1Fn;
typedef decltype(store_generic<kRGBA_Order>         )* StoreFn;
typedef decltype(store_generic_1<kRGBA_Order>       )* Store1Fn;

enum SrcFormat {
    kRGBA_8888_Linear_SrcFormat,
    kRGBA_8888_Table_SrcFormat,
    kBGRA_8888_Linear_SrcFormat,
    kBGRA_8888_Table_SrcFormat,
};

enum DstFormat {
    kRGBA_8888_Linear_DstFormat,
    kRGBA_8888_SRGB_DstFormat,
    kRGBA_8888_2Dot2_DstFormat,
    kRGBA_8888_Table_DstFormat,
    kBGRA_8888_Linear_DstFormat,
    kBGRA_8888_SRGB_DstFormat,
    kBGRA_8888_2Dot2_DstFormat,
    kBGRA_8888_Table_DstFormat,
    kF16_Linear_DstFormat,
};

template <SrcFormat kSrc,
          DstFormat kDst,
          SkAlphaType kAlphaType,
          ColorSpaceMatch kCSM>
static void color_xform_RGBA(void* dst, const void* vsrc, int len,
                             const float* const srcTables[3], const float matrix[13],
                             const uint8_t* const dstTables[3]) {
    LoadFn load;
    Load1Fn load_1;
    const bool kLoadAlpha = kF16_Linear_DstFormat == kDst && kOpaque_SkAlphaType != kAlphaType;
    switch (kSrc) {
        case kRGBA_8888_Linear_SrcFormat:
            if (kLoadAlpha) {
                load = load_rgba_linear<kRGBA_Order>;
                load_1 = load_rgba_linear_1<kRGBA_Order>;
            } else {
                load = load_rgb_linear<kRGBA_Order>;
                load_1 = load_rgb_linear_1<kRGBA_Order>;
            }
            break;
        case kRGBA_8888_Table_SrcFormat:
            if (kLoadAlpha) {
                load = load_rgba_from_tables<kRGBA_Order>;
                load_1 = load_rgba_from_tables_1<kRGBA_Order>;
            } else {
                load = load_rgb_from_tables<kRGBA_Order>;
                load_1 = load_rgb_from_tables_1<kRGBA_Order>;
            }
            break;
        case kBGRA_8888_Linear_SrcFormat:
            if (kLoadAlpha) {
                load = load_rgba_linear<kBGRA_Order>;
                load_1 = load_rgba_linear_1<kBGRA_Order>;
            } else {
                load = load_rgb_linear<kBGRA_Order>;
                load_1 = load_rgb_linear_1<kBGRA_Order>;
            }
            break;
        case kBGRA_8888_Table_SrcFormat:
            if (kLoadAlpha) {
                load = load_rgba_from_tables<kBGRA_Order>;
                load_1 = load_rgba_from_tables_1<kBGRA_Order>;
            } else {
                load = load_rgb_from_tables<kBGRA_Order>;
                load_1 = load_rgb_from_tables_1<kBGRA_Order>;
            }
            break;
    }

    StoreFn store;
    Store1Fn store_1;
    size_t sizeOfDstPixel;
    switch (kDst) {
        case kRGBA_8888_Linear_DstFormat:
            store   = store_linear<kRGBA_Order>;
            store_1 = store_linear_1<kRGBA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kRGBA_8888_SRGB_DstFormat:
            store   = store_srgb<kRGBA_Order>;
            store_1 = store_srgb_1<kRGBA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kRGBA_8888_2Dot2_DstFormat:
            store   = store_2dot2<kRGBA_Order>;
            store_1 = store_2dot2_1<kRGBA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kRGBA_8888_Table_DstFormat:
            store   = store_generic<kRGBA_Order>;
            store_1 = store_generic_1<kRGBA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kBGRA_8888_Linear_DstFormat:
            store   = store_linear<kBGRA_Order>;
            store_1 = store_linear_1<kBGRA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kBGRA_8888_SRGB_DstFormat:
            store   = store_srgb<kBGRA_Order>;
            store_1 = store_srgb_1<kBGRA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kBGRA_8888_2Dot2_DstFormat:
            store   = store_2dot2<kBGRA_Order>;
            store_1 = store_2dot2_1<kBGRA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kBGRA_8888_Table_DstFormat:
            store   = store_generic<kBGRA_Order>;
            store_1 = store_generic_1<kBGRA_Order>;
            sizeOfDstPixel = 4;
            break;
        case kF16_Linear_DstFormat:
            store   = (kOpaque_SkAlphaType == kAlphaType) ? store_f16_opaque<kRGBA_Order> :
                                                            store_f16<kRGBA_Order>;
            store_1 = (kOpaque_SkAlphaType == kAlphaType) ? store_f16_1_opaque<kRGBA_Order> :
                                                            store_f16_1<kRGBA_Order>;
            sizeOfDstPixel = 8;
            break;
    }

    const uint32_t* src = (const uint32_t*) vsrc;
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

        store_1(dst, src, rgba, a, dstTables);

        src += 1;
        len -= 1;
        dst = SkTAddOffset<void>(dst, sizeOfDstPixel);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static AI int num_tables(SkColorSpace_XYZ* space) {
    switch (space->gammaNamed()) {
        case kSRGB_SkGammaNamed:
        case k2Dot2Curve_SkGammaNamed:
        case kLinear_SkGammaNamed:
            return 0;
        default: {
            const SkGammas* gammas = space->gammas();
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

template <ColorSpaceMatch kCSM>
SkColorSpaceXform_XYZ<kCSM>
::SkColorSpaceXform_XYZ(SkColorSpace_XYZ* srcSpace, const SkMatrix44& srcToDst,
                        SkColorSpace_XYZ* dstSpace, SkTransferFunctionBehavior premulBehavior)
    : fPremulBehavior(premulBehavior)
{
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
    fSrcToDst[12] = 0.0f;

    const int numSrcTables = num_tables(srcSpace);
    const size_t srcEntries = numSrcTables * 256;
    const bool srcGammasAreMatching = (1 >= numSrcTables);
    fSrcStorage.reset(srcEntries);
    build_gamma_tables(fSrcGammaTables, fSrcStorage.get(), 256, srcSpace, kToLinear,
                       srcGammasAreMatching);

    const int numDstTables = num_tables(dstSpace);
    dstSpace->toDstGammaTables(fDstGammaTables, &fDstStorage, numDstTables);

    if (srcSpace->gammaIsLinear()) {
        fSrcGamma = kLinear_SrcGamma;
    } else if (kSRGB_SkGammaNamed == srcSpace->gammaNamed()) {
        fSrcGamma = kSRGB_SrcGamma;
    } else {
        fSrcGamma = kTable_SrcGamma;
    }

    switch (dstSpace->gammaNamed()) {
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

///////////////////////////////////////////////////////////////////////////////////////////////////

template <SrcFormat kSrc, DstFormat kDst, ColorSpaceMatch kCSM>
static AI bool apply_set_alpha(void* dst, const void* src, int len, SkAlphaType alphaType,
                               const float* const srcTables[3], const float matrix[13],
                               const uint8_t* const dstTables[3]) {
    switch (alphaType) {
        case kOpaque_SkAlphaType:
            color_xform_RGBA<kSrc, kDst, kOpaque_SkAlphaType, kCSM>
                    (dst, src, len, srcTables, matrix, dstTables);
            return true;
        case kUnpremul_SkAlphaType:
            color_xform_RGBA<kSrc, kDst, kUnpremul_SkAlphaType, kCSM>
                    (dst, src, len, srcTables, matrix, dstTables);
            return true;
        default:
            return false;
    }
}

template <DstFormat kDst, ColorSpaceMatch kCSM>
static AI bool apply_set_src(void* dst, const void* src, int len, SkAlphaType alphaType,
                             const float* const srcTables[3], const float matrix[13],
                             const uint8_t* const dstTables[3],
                             SkColorSpaceXform::ColorFormat srcColorFormat,
                             SrcGamma srcGamma) {
    switch (srcColorFormat) {
        case SkColorSpaceXform::kRGBA_8888_ColorFormat:
            switch (srcGamma) {
                case kLinear_SrcGamma:
                    return apply_set_alpha<kRGBA_8888_Linear_SrcFormat, kDst, kCSM>
                            (dst, src, len, alphaType, nullptr, matrix, dstTables);
                default:
                    return apply_set_alpha<kRGBA_8888_Table_SrcFormat, kDst, kCSM>
                            (dst, src, len, alphaType, srcTables, matrix, dstTables);
            }
        case SkColorSpaceXform::kBGRA_8888_ColorFormat:
            switch (srcGamma) {
                case kLinear_SrcGamma:
                    return apply_set_alpha<kBGRA_8888_Linear_SrcFormat, kDst, kCSM>
                            (dst, src, len, alphaType, nullptr, matrix, dstTables);
                default:
                    return apply_set_alpha<kBGRA_8888_Table_SrcFormat, kDst, kCSM>
                            (dst, src, len, alphaType, srcTables, matrix, dstTables);
            }
        default:
            return false;
    }
}

#undef AI

template <ColorSpaceMatch kCSM>
bool SkColorSpaceXform_XYZ<kCSM>
::onApply(ColorFormat dstColorFormat, void* dst, ColorFormat srcColorFormat, const void* src,
          int len, SkAlphaType alphaType) const
{
    if (kFull_ColorSpaceMatch == kCSM) {
        if (kPremul_SkAlphaType != alphaType) {
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
                SkOpts::RGBA_to_BGRA((uint32_t*) dst, src, len);
                return true;
            }
        }
    }

    if (kRGBA_F32_ColorFormat == dstColorFormat ||
        kBGR_565_ColorFormat == dstColorFormat ||
        kRGBA_F32_ColorFormat == srcColorFormat ||
        kRGBA_F16_ColorFormat == srcColorFormat ||
        kRGBA_U16_BE_ColorFormat == srcColorFormat ||
        kRGB_U16_BE_ColorFormat == srcColorFormat ||
        kPremul_SkAlphaType == alphaType)
    {
        return this->applyPipeline(dstColorFormat, dst, srcColorFormat, src, len, alphaType);
    }

    switch (dstColorFormat) {
        case kRGBA_8888_ColorFormat:
            switch (fDstGamma) {
                case kLinear_DstGamma:
                    return apply_set_src<kRGBA_8888_Linear_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr,
                             srcColorFormat, fSrcGamma);
                case kSRGB_DstGamma:
                    return apply_set_src<kRGBA_8888_SRGB_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr,
                             srcColorFormat, fSrcGamma);
                case k2Dot2_DstGamma:
                    return apply_set_src<kRGBA_8888_2Dot2_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr,
                             srcColorFormat, fSrcGamma);
                case kTable_DstGamma:
                    return apply_set_src<kRGBA_8888_Table_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, fDstGammaTables,
                             srcColorFormat, fSrcGamma);
            }
        case kBGRA_8888_ColorFormat:
            switch (fDstGamma) {
                case kLinear_DstGamma:
                    return apply_set_src<kBGRA_8888_Linear_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr,
                             srcColorFormat, fSrcGamma);
                case kSRGB_DstGamma:
                    return apply_set_src<kBGRA_8888_SRGB_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr,
                             srcColorFormat, fSrcGamma);
                case k2Dot2_DstGamma:
                    return apply_set_src<kBGRA_8888_2Dot2_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr,
                             srcColorFormat, fSrcGamma);
                case kTable_DstGamma:
                    return apply_set_src<kBGRA_8888_Table_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, fDstGammaTables,
                             srcColorFormat, fSrcGamma);
            }
        case kRGBA_F16_ColorFormat:
            switch (fDstGamma) {
                case kLinear_DstGamma:
                    return apply_set_src<kF16_Linear_DstFormat, kCSM>
                            (dst, src, len, alphaType, fSrcGammaTables, fSrcToDst, nullptr,
                             srcColorFormat, fSrcGamma);
                default:
                    return false;
            }
        default:
            SkASSERT(false);
            return false;
    }
}

bool SkColorSpaceXform::apply(ColorFormat dstColorFormat, void* dst, ColorFormat srcColorFormat,
                              const void* src, int len, SkAlphaType alphaType) const {
    return ((SkColorSpaceXform_Base*) this)->onApply(dstColorFormat, dst, srcColorFormat, src, len,
                                                     alphaType);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <ColorSpaceMatch kCSM>
bool SkColorSpaceXform_XYZ<kCSM>
::applyPipeline(ColorFormat dstColorFormat, void* dst, ColorFormat srcColorFormat,
                const void* src, int len, SkAlphaType alphaType) const {
    SkRasterPipeline pipeline;

    LoadTablesContext loadTables;
    switch (srcColorFormat) {
        case kRGBA_8888_ColorFormat:
            if (kLinear_SrcGamma == fSrcGamma) {
                pipeline.append(SkRasterPipeline::load_8888, &src);
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
                pipeline.append(SkRasterPipeline::load_8888, &src);
            } else {
                loadTables.fSrc = src;
                loadTables.fR = fSrcGammaTables[2];
                loadTables.fG = fSrcGammaTables[1];
                loadTables.fB = fSrcGammaTables[0];
                pipeline.append(SkRasterPipeline::load_tables, &loadTables);
            }

            pipeline.append(SkRasterPipeline::swap_rb);
            break;
        case kRGBA_F16_ColorFormat:
            if (kLinear_SrcGamma != fSrcGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::load_f16, &src);
            break;
        case kRGBA_F32_ColorFormat:
            if (kLinear_SrcGamma != fSrcGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::load_f32, &src);
            break;
        case kRGBA_U16_BE_ColorFormat:
            switch (fSrcGamma) {
                case kLinear_SrcGamma:
                    pipeline.append(SkRasterPipeline::load_u16_be, &src);
                    break;
                case kSRGB_SrcGamma:
                    pipeline.append(SkRasterPipeline::load_u16_be, &src);
                    pipeline.append_from_srgb(kUnpremul_SkAlphaType);
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
                    pipeline.append(SkRasterPipeline::load_rgb_u16_be, &src);
                    break;
                case kSRGB_SrcGamma:
                    pipeline.append(SkRasterPipeline::load_rgb_u16_be, &src);
                    pipeline.append_from_srgb(kUnpremul_SkAlphaType);
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

    if (kNone_ColorSpaceMatch == kCSM) {
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
    switch (fDstGamma) {
        case kSRGB_DstGamma:
            pipeline.append(SkRasterPipeline::to_srgb);
            break;
        case k2Dot2_DstGamma:
            pipeline.append(SkRasterPipeline::to_2dot2);
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
             pipeline.append(SkRasterPipeline::store_8888, &dst);
            break;
        case kBGRA_8888_ColorFormat:
            pipeline.append(SkRasterPipeline::swap_rb);
            pipeline.append(SkRasterPipeline::store_8888, &dst);
            break;
        case kRGBA_F16_ColorFormat:
            if (kLinear_DstGamma != fDstGamma) {
                return false;
            }
            pipeline.append(SkRasterPipeline::store_f16, &dst);
            break;
        case kRGBA_F32_ColorFormat:
            if (kLinear_DstGamma != fDstGamma) {
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

    pipeline.run(0, len);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace_XYZ* space) {
    return std::unique_ptr<SkColorSpaceXform>(new SkColorSpaceXform_XYZ<kNone_ColorSpaceMatch>
            (space, SkMatrix::I(), space, SkTransferFunctionBehavior::kRespect));
}
