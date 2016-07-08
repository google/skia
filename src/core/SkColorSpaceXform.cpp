/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"
#include "SkOpts.h"

static inline bool compute_gamut_xform(SkMatrix44* srcToDst, const SkMatrix44& srcToXYZ,
                                       const SkMatrix44& dstToXYZ) {
    if (!dstToXYZ.invert(srcToDst)) {
        return false;
    }

    srcToDst->postConcat(srcToXYZ);
    return true;
}

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(const sk_sp<SkColorSpace>& srcSpace,
                                                          const sk_sp<SkColorSpace>& dstSpace) {
    if (!srcSpace || !dstSpace) {
        // Invalid input
        return nullptr;
    }

    if (as_CSB(dstSpace)->colorLUT()) {
        // It would be really weird for a dst profile to have a color LUT.  I don't think
        // we need to support this.
        return nullptr;
    }

    SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
    if (!compute_gamut_xform(&srcToDst, srcSpace->xyz(), dstSpace->xyz())) {
        return nullptr;
    }

    if (0.0f == srcToDst.getFloat(3, 0) &&
        0.0f == srcToDst.getFloat(3, 1) &&
        0.0f == srcToDst.getFloat(3, 2) &&
        !as_CSB(srcSpace)->colorLUT())
    {
        switch (srcSpace->gammaNamed()) {
            case SkColorSpace::kSRGB_GammaNamed:
                if (SkColorSpace::kSRGB_GammaNamed == dstSpace->gammaNamed()) {
                    return std::unique_ptr<SkColorSpaceXform>(
                            new SkFastXform<SkColorSpace::kSRGB_GammaNamed,
                                            SkColorSpace::kSRGB_GammaNamed>(srcToDst));
                } else if (SkColorSpace::k2Dot2Curve_GammaNamed == dstSpace->gammaNamed()) {
                    return std::unique_ptr<SkColorSpaceXform>(
                            new SkFastXform<SkColorSpace::kSRGB_GammaNamed,
                                            SkColorSpace::k2Dot2Curve_GammaNamed>(srcToDst));
                }
                break;
            case SkColorSpace::k2Dot2Curve_GammaNamed:
                if (SkColorSpace::kSRGB_GammaNamed == dstSpace->gammaNamed()) {
                    return std::unique_ptr<SkColorSpaceXform>(
                            new SkFastXform<SkColorSpace::k2Dot2Curve_GammaNamed,
                                            SkColorSpace::kSRGB_GammaNamed>(srcToDst));
                } else if (SkColorSpace::k2Dot2Curve_GammaNamed == dstSpace->gammaNamed()) {
                    return std::unique_ptr<SkColorSpaceXform>(
                            new SkFastXform<SkColorSpace::k2Dot2Curve_GammaNamed,
                                            SkColorSpace::k2Dot2Curve_GammaNamed>(srcToDst));
                }
                break;
            default:
                break;
        }
    }

    return std::unique_ptr<SkColorSpaceXform>(new SkDefaultXform(srcSpace, srcToDst, dstSpace));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void build_src_to_dst(float srcToDstArray[12], const SkMatrix44& srcToDstMatrix) {
    // Build the following row major matrix:
    //   rX gX bX 0
    //   rY gY bY 0
    //   rZ gZ bZ 0
    // Swap R and B if necessary to make sure that we output SkPMColor order.
#ifdef SK_PMCOLOR_IS_BGRA
    srcToDstArray[0] = srcToDstMatrix.getFloat(0, 2);
    srcToDstArray[1] = srcToDstMatrix.getFloat(0, 1);
    srcToDstArray[2] = srcToDstMatrix.getFloat(0, 0);
    srcToDstArray[3] = 0.0f;
    srcToDstArray[4] = srcToDstMatrix.getFloat(1, 2);
    srcToDstArray[5] = srcToDstMatrix.getFloat(1, 1);
    srcToDstArray[6] = srcToDstMatrix.getFloat(1, 0);
    srcToDstArray[7] = 0.0f;
    srcToDstArray[8] = srcToDstMatrix.getFloat(2, 2);
    srcToDstArray[9] = srcToDstMatrix.getFloat(2, 1);
    srcToDstArray[10] = srcToDstMatrix.getFloat(2, 0);
    srcToDstArray[11] = 0.0f;
#else
    srcToDstArray[0] = srcToDstMatrix.getFloat(0, 0);
    srcToDstArray[1] = srcToDstMatrix.getFloat(0, 1);
    srcToDstArray[2] = srcToDstMatrix.getFloat(0, 2);
    srcToDstArray[3] = 0.0f;
    srcToDstArray[4] = srcToDstMatrix.getFloat(1, 0);
    srcToDstArray[5] = srcToDstMatrix.getFloat(1, 1);
    srcToDstArray[6] = srcToDstMatrix.getFloat(1, 2);
    srcToDstArray[7] = 0.0f;
    srcToDstArray[8] = srcToDstMatrix.getFloat(2, 0);
    srcToDstArray[9] = srcToDstMatrix.getFloat(2, 1);
    srcToDstArray[10] = srcToDstMatrix.getFloat(2, 2);
    srcToDstArray[11] = 0.0f;
#endif
}

template <SkColorSpace::GammaNamed Src, SkColorSpace::GammaNamed Dst>
SkFastXform<Src, Dst>::SkFastXform(const SkMatrix44& srcToDst)
{
    build_src_to_dst(fSrcToDst, srcToDst);
}

template <>
void SkFastXform<SkColorSpace::kSRGB_GammaNamed, SkColorSpace::kSRGB_GammaNamed>
::xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const
{
    SkOpts::color_xform_RGB1_srgb_to_srgb(dst, src, len, fSrcToDst);
}

template <>
void SkFastXform<SkColorSpace::kSRGB_GammaNamed, SkColorSpace::k2Dot2Curve_GammaNamed>
::xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const
{
    SkOpts::color_xform_RGB1_srgb_to_2dot2(dst, src, len, fSrcToDst);
}

template <>
void SkFastXform<SkColorSpace::k2Dot2Curve_GammaNamed, SkColorSpace::kSRGB_GammaNamed>
::xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const
{
    SkOpts::color_xform_RGB1_2dot2_to_srgb(dst, src, len, fSrcToDst);
}

template <>
void SkFastXform<SkColorSpace::k2Dot2Curve_GammaNamed, SkColorSpace::k2Dot2Curve_GammaNamed>
::xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const
{
    SkOpts::color_xform_RGB1_2dot2_to_2dot2(dst, src, len, fSrcToDst);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

extern const float sk_linear_from_srgb[256] = {
        0.000000000000000000f, 0.000303526983548838f, 0.000607053967097675f, 0.000910580950646513f,
        0.001214107934195350f, 0.001517634917744190f, 0.001821161901293030f, 0.002124688884841860f,
        0.002428215868390700f, 0.002731742851939540f, 0.003034518678424960f, 0.003346535763899160f,
        0.003676507324047440f, 0.004024717018496310f, 0.004391442037410290f, 0.004776953480693730f,
        0.005181516702338390f, 0.005605391624202720f, 0.006048833022857060f, 0.006512090792594470f,
        0.006995410187265390f, 0.007499032043226180f, 0.008023192985384990f, 0.008568125618069310f,
        0.009134058702220790f, 0.009721217320237850f, 0.010329823029626900f, 0.010960094006488200f,
        0.011612245179743900f, 0.012286488356915900f, 0.012983032342173000f, 0.013702083047289700f,
        0.014443843596092500f, 0.015208514422912700f, 0.015996293365509600f, 0.016807375752887400f,
        0.017641954488384100f, 0.018500220128379700f, 0.019382360956935700f, 0.020288563056652400f,
        0.021219010376003600f, 0.022173884793387400f, 0.023153366178110400f, 0.024157632448504800f,
        0.025186859627361600f, 0.026241221894849900f, 0.027320891639074900f, 0.028426039504420800f,
        0.029556834437808800f, 0.030713443732993600f, 0.031896033073011500f, 0.033104766570885100f,
        0.034339806808682200f, 0.035601314875020300f, 0.036889450401100000f, 0.038204371595346500f,
        0.039546235276732800f, 0.040915196906853200f, 0.042311410620809700f, 0.043735029256973500f,
        0.045186204385675500f, 0.046665086336880100f, 0.048171824226889400f, 0.049706565984127200f,
        0.051269458374043200f, 0.052860647023180200f, 0.054480276442442400f, 0.056128490049600100f,
        0.057805430191067200f, 0.059511238162981200f, 0.061246054231617600f, 0.063010017653167700f,
        0.064803266692905800f, 0.066625938643772900f, 0.068478169844400200f, 0.070360095696595900f,
        0.072271850682317500f, 0.074213568380149600f, 0.076185381481307900f, 0.078187421805186300f,
        0.080219820314468300f, 0.082282707129814800f, 0.084376211544148800f, 0.086500462036549800f,
        0.088655586285772900f, 0.090841711183407700f, 0.093058962846687500f, 0.095307466630964700f,
        0.097587347141862500f, 0.099898728247113900f, 0.102241733088101000f, 0.104616484091104000f,
        0.107023102978268000f, 0.109461710778299000f, 0.111932427836906000f, 0.114435373826974000f,
        0.116970667758511000f, 0.119538427988346000f, 0.122138772229602000f, 0.124771817560950000f,
        0.127437680435647000f, 0.130136476690364000f, 0.132868321553818000f, 0.135633329655206000f,
        0.138431615032452000f, 0.141263291140272000f, 0.144128470858058000f, 0.147027266497595000f,
        0.149959789810609000f, 0.152926151996150000f, 0.155926463707827000f, 0.158960835060880000f,
        0.162029375639111000f, 0.165132194501668000f, 0.168269400189691000f, 0.171441100732823000f,
        0.174647403655585000f, 0.177888415983629000f, 0.181164244249860000f, 0.184474994500441000f,
        0.187820772300678000f, 0.191201682740791000f, 0.194617830441576000f, 0.198069319559949000f,
        0.201556253794397000f, 0.205078736390317000f, 0.208636870145256000f, 0.212230757414055000f,
        0.215860500113899000f, 0.219526199729269000f, 0.223227957316809000f, 0.226965873510098000f,
        0.230740048524349000f, 0.234550582161005000f, 0.238397573812271000f, 0.242281122465555000f,
        0.246201326707835000f, 0.250158284729953000f, 0.254152094330827000f, 0.258182852921596000f,
        0.262250657529696000f, 0.266355604802862000f, 0.270497791013066000f, 0.274677312060385000f,
        0.278894263476810000f, 0.283148740429992000f, 0.287440837726918000f, 0.291770649817536000f,
        0.296138270798321000f, 0.300543794415777000f, 0.304987314069886000f, 0.309468922817509000f,
        0.313988713375718000f, 0.318546778125092000f, 0.323143209112951000f, 0.327778098056542000f,
        0.332451536346179000f, 0.337163615048330000f, 0.341914424908661000f, 0.346704056355030000f,
        0.351532599500439000f, 0.356400144145944000f, 0.361306779783510000f, 0.366252595598840000f,
        0.371237680474149000f, 0.376262122990906000f, 0.381326011432530000f, 0.386429433787049000f,
        0.391572477749723000f, 0.396755230725627000f, 0.401977779832196000f, 0.407240211901737000f,
        0.412542613483904000f, 0.417885070848138000f, 0.423267669986072000f, 0.428690496613907000f,
        0.434153636174749000f, 0.439657173840919000f, 0.445201194516228000f, 0.450785782838223000f,
        0.456411023180405000f, 0.462076999654407000f, 0.467783796112159000f, 0.473531496148010000f,
        0.479320183100827000f, 0.485149940056070000f, 0.491020849847836000f, 0.496932995060870000f,
        0.502886458032569000f, 0.508881320854934000f, 0.514917665376521000f, 0.520995573204354000f,
        0.527115125705813000f, 0.533276404010505000f, 0.539479489012107000f, 0.545724461370187000f,
        0.552011401512000000f, 0.558340389634268000f, 0.564711505704929000f, 0.571124829464873000f,
        0.577580440429651000f, 0.584078417891164000f, 0.590618840919337000f, 0.597201788363763000f,
        0.603827338855338000f, 0.610495570807865000f, 0.617206562419651000f, 0.623960391675076000f,
        0.630757136346147000f, 0.637596873994033000f, 0.644479681970582000f, 0.651405637419824000f,
        0.658374817279448000f, 0.665387298282272000f, 0.672443156957688000f, 0.679542469633094000f,
        0.686685312435314000f, 0.693871761291990000f, 0.701101891932973000f, 0.708375779891687000f,
        0.715693500506481000f, 0.723055128921969000f, 0.730460740090354000f, 0.737910408772731000f,
        0.745404209540387000f, 0.752942216776078000f, 0.760524504675292000f, 0.768151147247507000f,
        0.775822218317423000f, 0.783537791526194000f, 0.791297940332630000f, 0.799102738014409000f,
        0.806952257669252000f, 0.814846572216101000f, 0.822785754396284000f, 0.830769876774655000f,
        0.838799011740740000f, 0.846873231509858000f, 0.854992608124234000f, 0.863157213454102000f,
        0.871367119198797000f, 0.879622396887832000f, 0.887923117881966000f, 0.896269353374266000f,
        0.904661174391149000f, 0.913098651793419000f, 0.921581856277295000f, 0.930110858375424000f,
        0.938685728457888000f, 0.947306536733200000f, 0.955973353249286000f, 0.964686247894465000f,
        0.973445290398413000f, 0.982250550333117000f, 0.991102097113830000f, 1.000000000000000000f,
};

extern const float sk_linear_from_2dot2[256] = {
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

static constexpr uint8_t linear_to_srgb[1024] = {
          0,   3,   6,  10,  13,  15,  18,  20,  22,  23,  25,  27,  28,  30,  31,  32,  34,  35,
         36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  49,  50,  51,  52,
         53,  53,  54,  55,  56,  56,  57,  58,  58,  59,  60,  61,  61,  62,  62,  63,  64,  64,
         65,  66,  66,  67,  67,  68,  68,  69,  70,  70,  71,  71,  72,  72,  73,  73,  74,  74,
         75,  76,  76,  77,  77,  78,  78,  79,  79,  79,  80,  80,  81,  81,  82,  82,  83,  83,
         84,  84,  85,  85,  85,  86,  86,  87,  87,  88,  88,  88,  89,  89,  90,  90,  91,  91,
         91,  92,  92,  93,  93,  93,  94,  94,  95,  95,  95,  96,  96,  97,  97,  97,  98,  98,
         98,  99,  99,  99, 100, 100, 101, 101, 101, 102, 102, 102, 103, 103, 103, 104, 104, 104,
        105, 105, 106, 106, 106, 107, 107, 107, 108, 108, 108, 109, 109, 109, 110, 110, 110, 110,
        111, 111, 111, 112, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 115, 116, 116,
        116, 117, 117, 117, 118, 118, 118, 118, 119, 119, 119, 120, 120, 120, 121, 121, 121, 121,
        122, 122, 122, 123, 123, 123, 123, 124, 124, 124, 125, 125, 125, 125, 126, 126, 126, 126,
        127, 127, 127, 128, 128, 128, 128, 129, 129, 129, 129, 130, 130, 130, 130, 131, 131, 131,
        131, 132, 132, 132, 133, 133, 133, 133, 134, 134, 134, 134, 135, 135, 135, 135, 136, 136,
        136, 136, 137, 137, 137, 137, 138, 138, 138, 138, 138, 139, 139, 139, 139, 140, 140, 140,
        140, 141, 141, 141, 141, 142, 142, 142, 142, 143, 143, 143, 143, 143, 144, 144, 144, 144,
        145, 145, 145, 145, 146, 146, 146, 146, 146, 147, 147, 147, 147, 148, 148, 148, 148, 148,
        149, 149, 149, 149, 150, 150, 150, 150, 150, 151, 151, 151, 151, 152, 152, 152, 152, 152,
        153, 153, 153, 153, 153, 154, 154, 154, 154, 155, 155, 155, 155, 155, 156, 156, 156, 156,
        156, 157, 157, 157, 157, 157, 158, 158, 158, 158, 158, 159, 159, 159, 159, 159, 160, 160,
        160, 160, 160, 161, 161, 161, 161, 161, 162, 162, 162, 162, 162, 163, 163, 163, 163, 163,
        164, 164, 164, 164, 164, 165, 165, 165, 165, 165, 166, 166, 166, 166, 166, 167, 167, 167,
        167, 167, 168, 168, 168, 168, 168, 168, 169, 169, 169, 169, 169, 170, 170, 170, 170, 170,
        171, 171, 171, 171, 171, 171, 172, 172, 172, 172, 172, 173, 173, 173, 173, 173, 173, 174,
        174, 174, 174, 174, 175, 175, 175, 175, 175, 175, 176, 176, 176, 176, 176, 177, 177, 177,
        177, 177, 177, 178, 178, 178, 178, 178, 178, 179, 179, 179, 179, 179, 179, 180, 180, 180,
        180, 180, 181, 181, 181, 181, 181, 181, 182, 182, 182, 182, 182, 182, 183, 183, 183, 183,
        183, 183, 184, 184, 184, 184, 184, 184, 185, 185, 185, 185, 185, 185, 186, 186, 186, 186,
        186, 186, 187, 187, 187, 187, 187, 187, 188, 188, 188, 188, 188, 188, 189, 189, 189, 189,
        189, 189, 190, 190, 190, 190, 190, 190, 191, 191, 191, 191, 191, 191, 191, 192, 192, 192,
        192, 192, 192, 193, 193, 193, 193, 193, 193, 194, 194, 194, 194, 194, 194, 194, 195, 195,
        195, 195, 195, 195, 196, 196, 196, 196, 196, 196, 197, 197, 197, 197, 197, 197, 197, 198,
        198, 198, 198, 198, 198, 199, 199, 199, 199, 199, 199, 199, 200, 200, 200, 200, 200, 200,
        200, 201, 201, 201, 201, 201, 201, 202, 202, 202, 202, 202, 202, 202, 203, 203, 203, 203,
        203, 203, 203, 204, 204, 204, 204, 204, 204, 204, 205, 205, 205, 205, 205, 205, 206, 206,
        206, 206, 206, 206, 206, 207, 207, 207, 207, 207, 207, 207, 208, 208, 208, 208, 208, 208,
        208, 209, 209, 209, 209, 209, 209, 209, 210, 210, 210, 210, 210, 210, 210, 211, 211, 211,
        211, 211, 211, 211, 212, 212, 212, 212, 212, 212, 212, 212, 213, 213, 213, 213, 213, 213,
        213, 214, 214, 214, 214, 214, 214, 214, 215, 215, 215, 215, 215, 215, 215, 216, 216, 216,
        216, 216, 216, 216, 216, 217, 217, 217, 217, 217, 217, 217, 218, 218, 218, 218, 218, 218,
        218, 219, 219, 219, 219, 219, 219, 219, 219, 220, 220, 220, 220, 220, 220, 220, 221, 221,
        221, 221, 221, 221, 221, 221, 222, 222, 222, 222, 222, 222, 222, 222, 223, 223, 223, 223,
        223, 223, 223, 224, 224, 224, 224, 224, 224, 224, 224, 225, 225, 225, 225, 225, 225, 225,
        225, 226, 226, 226, 226, 226, 226, 226, 227, 227, 227, 227, 227, 227, 227, 227, 228, 228,
        228, 228, 228, 228, 228, 228, 229, 229, 229, 229, 229, 229, 229, 229, 230, 230, 230, 230,
        230, 230, 230, 230, 231, 231, 231, 231, 231, 231, 231, 231, 232, 232, 232, 232, 232, 232,
        232, 232, 233, 233, 233, 233, 233, 233, 233, 233, 234, 234, 234, 234, 234, 234, 234, 234,
        235, 235, 235, 235, 235, 235, 235, 235, 236, 236, 236, 236, 236, 236, 236, 236, 236, 237,
        237, 237, 237, 237, 237, 237, 237, 238, 238, 238, 238, 238, 238, 238, 238, 239, 239, 239,
        239, 239, 239, 239, 239, 239, 240, 240, 240, 240, 240, 240, 240, 240, 241, 241, 241, 241,
        241, 241, 241, 241, 241, 242, 242, 242, 242, 242, 242, 242, 242, 243, 243, 243, 243, 243,
        243, 243, 243, 243, 244, 244, 244, 244, 244, 244, 244, 244, 245, 245, 245, 245, 245, 245,
        245, 245, 245, 246, 246, 246, 246, 246, 246, 246, 246, 246, 247, 247, 247, 247, 247, 247,
        247, 247, 248, 248, 248, 248, 248, 248, 248, 248, 248, 249, 249, 249, 249, 249, 249, 249,
        249, 249, 250, 250, 250, 250, 250, 250, 250, 250, 250, 251, 251, 251, 251, 251, 251, 251,
        251, 251, 252, 252, 252, 252, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253, 253, 253,
        253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255
};

static constexpr uint8_t linear_to_2dot2[1024] = {
          0,  11,  15,  18,  21,  23,  25,  26,  28,  30,  31,  32,  34,  35,  36,  37,  39,  40,
         41,  42,  43,  44,  45,  45,  46,  47,  48,  49,  50,  50,  51,  52,  53,  54,  54,  55,
         56,  56,  57,  58,  58,  59,  60,  60,  61,  62,  62,  63,  63,  64,  65,  65,  66,  66,
         67,  68,  68,  69,  69,  70,  70,  71,  71,  72,  72,  73,  73,  74,  74,  75,  75,  76,
         76,  77,  77,  78,  78,  79,  79,  80,  80,  81,  81,  81,  82,  82,  83,  83,  84,  84,
         84,  85,  85,  86,  86,  87,  87,  87,  88,  88,  89,  89,  89,  90,  90,  91,  91,  91,
         92,  92,  93,  93,  93,  94,  94,  94,  95,  95,  96,  96,  96,  97,  97,  97,  98,  98,
         98,  99,  99,  99, 100, 100, 101, 101, 101, 102, 102, 102, 103, 103, 103, 104, 104, 104,
        105, 105, 105, 106, 106, 106, 107, 107, 107, 108, 108, 108, 108, 109, 109, 109, 110, 110,
        110, 111, 111, 111, 112, 112, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 115,
        116, 116, 116, 117, 117, 117, 117, 118, 118, 118, 119, 119, 119, 119, 120, 120, 120, 121,
        121, 121, 121, 122, 122, 122, 123, 123, 123, 123, 124, 124, 124, 124, 125, 125, 125, 125,
        126, 126, 126, 127, 127, 127, 127, 128, 128, 128, 128, 129, 129, 129, 129, 130, 130, 130,
        130, 131, 131, 131, 131, 132, 132, 132, 132, 133, 133, 133, 133, 134, 134, 134, 134, 135,
        135, 135, 135, 136, 136, 136, 136, 137, 137, 137, 137, 138, 138, 138, 138, 138, 139, 139,
        139, 139, 140, 140, 140, 140, 141, 141, 141, 141, 142, 142, 142, 142, 142, 143, 143, 143,
        143, 144, 144, 144, 144, 144, 145, 145, 145, 145, 146, 146, 146, 146, 146, 147, 147, 147,
        147, 148, 148, 148, 148, 148, 149, 149, 149, 149, 149, 150, 150, 150, 150, 151, 151, 151,
        151, 151, 152, 152, 152, 152, 152, 153, 153, 153, 153, 154, 154, 154, 154, 154, 155, 155,
        155, 155, 155, 156, 156, 156, 156, 156, 157, 157, 157, 157, 157, 158, 158, 158, 158, 158,
        159, 159, 159, 159, 159, 160, 160, 160, 160, 160, 161, 161, 161, 161, 161, 162, 162, 162,
        162, 162, 163, 163, 163, 163, 163, 164, 164, 164, 164, 164, 165, 165, 165, 165, 165, 165,
        166, 166, 166, 166, 166, 167, 167, 167, 167, 167, 168, 168, 168, 168, 168, 168, 169, 169,
        169, 169, 169, 170, 170, 170, 170, 170, 171, 171, 171, 171, 171, 171, 172, 172, 172, 172,
        172, 173, 173, 173, 173, 173, 173, 174, 174, 174, 174, 174, 174, 175, 175, 175, 175, 175,
        176, 176, 176, 176, 176, 176, 177, 177, 177, 177, 177, 177, 178, 178, 178, 178, 178, 179,
        179, 179, 179, 179, 179, 180, 180, 180, 180, 180, 180, 181, 181, 181, 181, 181, 181, 182,
        182, 182, 182, 182, 182, 183, 183, 183, 183, 183, 183, 184, 184, 184, 184, 184, 185, 185,
        185, 185, 185, 185, 186, 186, 186, 186, 186, 186, 186, 187, 187, 187, 187, 187, 187, 188,
        188, 188, 188, 188, 188, 189, 189, 189, 189, 189, 189, 190, 190, 190, 190, 190, 190, 191,
        191, 191, 191, 191, 191, 192, 192, 192, 192, 192, 192, 192, 193, 193, 193, 193, 193, 193,
        194, 194, 194, 194, 194, 194, 195, 195, 195, 195, 195, 195, 195, 196, 196, 196, 196, 196,
        196, 197, 197, 197, 197, 197, 197, 197, 198, 198, 198, 198, 198, 198, 199, 199, 199, 199,
        199, 199, 199, 200, 200, 200, 200, 200, 200, 201, 201, 201, 201, 201, 201, 201, 202, 202,
        202, 202, 202, 202, 202, 203, 203, 203, 203, 203, 203, 204, 204, 204, 204, 204, 204, 204,
        205, 205, 205, 205, 205, 205, 205, 206, 206, 206, 206, 206, 206, 206, 207, 207, 207, 207,
        207, 207, 207, 208, 208, 208, 208, 208, 208, 209, 209, 209, 209, 209, 209, 209, 210, 210,
        210, 210, 210, 210, 210, 211, 211, 211, 211, 211, 211, 211, 212, 212, 212, 212, 212, 212,
        212, 213, 213, 213, 213, 213, 213, 213, 213, 214, 214, 214, 214, 214, 214, 214, 215, 215,
        215, 215, 215, 215, 215, 216, 216, 216, 216, 216, 216, 216, 217, 217, 217, 217, 217, 217,
        217, 218, 218, 218, 218, 218, 218, 218, 218, 219, 219, 219, 219, 219, 219, 219, 220, 220,
        220, 220, 220, 220, 220, 221, 221, 221, 221, 221, 221, 221, 221, 222, 222, 222, 222, 222,
        222, 222, 223, 223, 223, 223, 223, 223, 223, 223, 224, 224, 224, 224, 224, 224, 224, 225,
        225, 225, 225, 225, 225, 225, 225, 226, 226, 226, 226, 226, 226, 226, 226, 227, 227, 227,
        227, 227, 227, 227, 228, 228, 228, 228, 228, 228, 228, 228, 229, 229, 229, 229, 229, 229,
        229, 229, 230, 230, 230, 230, 230, 230, 230, 230, 231, 231, 231, 231, 231, 231, 231, 232,
        232, 232, 232, 232, 232, 232, 232, 233, 233, 233, 233, 233, 233, 233, 233, 234, 234, 234,
        234, 234, 234, 234, 234, 235, 235, 235, 235, 235, 235, 235, 235, 236, 236, 236, 236, 236,
        236, 236, 236, 237, 237, 237, 237, 237, 237, 237, 237, 238, 238, 238, 238, 238, 238, 238,
        238, 238, 239, 239, 239, 239, 239, 239, 239, 239, 240, 240, 240, 240, 240, 240, 240, 240,
        241, 241, 241, 241, 241, 241, 241, 241, 242, 242, 242, 242, 242, 242, 242, 242, 243, 243,
        243, 243, 243, 243, 243, 243, 243, 244, 244, 244, 244, 244, 244, 244, 244, 245, 245, 245,
        245, 245, 245, 245, 245, 245, 246, 246, 246, 246, 246, 246, 246, 246, 247, 247, 247, 247,
        247, 247, 247, 247, 248, 248, 248, 248, 248, 248, 248, 248, 248, 249, 249, 249, 249, 249,
        249, 249, 249, 249, 250, 250, 250, 250, 250, 250, 250, 250, 251, 251, 251, 251, 251, 251,
        251, 251, 251, 252, 252, 252, 252, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253, 253,
        253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255,
};

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

static void build_table_linear_to_gamma(uint8_t* outTable, int outTableSize, float exponent) {
    float toGammaExp = 1.0f / exponent;

    for (int i = 0; i < outTableSize; i++) {
        float x = ((float) i) * (1.0f / ((float) (outTableSize - 1)));
        outTable[i] = clamp_normalized_float_to_byte(powf(x, toGammaExp));
    }
}

// Inverse table lookup.  Ex: what index corresponds to the input value?  This will
// have strange results when the table is non-increasing.  But any sane gamma
// function will be increasing.
static float inverse_interp_lut(float input, float* table, int tableSize) {
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

static void build_table_linear_to_gamma(uint8_t* outTable, int outTableSize, float* inTable,
                                        int inTableSize) {
    for (int i = 0; i < outTableSize; i++) {
        float x = ((float) i) * (1.0f / ((float) (outTableSize - 1)));
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

static void build_table_linear_to_gamma(uint8_t* outTable, int outTableSize, float g, float a,
                                        float b, float c, float d, float e, float f) {
    for (int i = 0; i < outTableSize; i++) {
        float x = ((float) i) * (1.0f / ((float) (outTableSize - 1)));
        float y = inverse_parametric(x, g, a, b, c, d, e, f);
        outTable[i] = clamp_normalized_float_to_byte(y);
    }
}

SkDefaultXform::SkDefaultXform(const sk_sp<SkColorSpace>& srcSpace, const SkMatrix44& srcToDst,
                               const sk_sp<SkColorSpace>& dstSpace)
    : fColorLUT(sk_ref_sp((SkColorLookUpTable*) as_CSB(srcSpace)->colorLUT()))
    , fSrcToDst(srcToDst)
{
    // Build tables to transform src gamma to linear.
    switch (srcSpace->gammaNamed()) {
        case SkColorSpace::kSRGB_GammaNamed:
            fSrcGammaTables[0] = fSrcGammaTables[1] = fSrcGammaTables[2] = sk_linear_from_srgb;
            break;
        case SkColorSpace::k2Dot2Curve_GammaNamed:
            fSrcGammaTables[0] = fSrcGammaTables[1] = fSrcGammaTables[2] = sk_linear_from_2dot2;
            break;
        case SkColorSpace::kLinear_GammaNamed:
            build_table_linear_from_gamma(fSrcGammaTableStorage, 1.0f);
            fSrcGammaTables[0] = fSrcGammaTables[1] = fSrcGammaTables[2] = fSrcGammaTableStorage;
            break;
        default: {
            const SkGammas* gammas = as_CSB(srcSpace)->gammas();
            SkASSERT(gammas);

            for (int i = 0; i < 3; i++) {
                const SkGammaCurve& curve = (*gammas)[i];

                if (i > 0) {
                    // Check if this curve matches the first curve.  In this case, we can
                    // share the same table pointer.  Logically, this should almost always
                    // be true.  I've never seen a profile where all three gamma curves
                    // didn't match.  But it is possible that they won't.
                    // TODO (msarett):
                    // This comparison won't catch the case where each gamma curve has a
                    // pointer to its own look-up table, but the tables actually match.
                    // Should we perform a deep compare of gamma tables here?  Or should
                    // we catch this when parsing the profile?  Or should we not worry
                    // about a bit of redundant work?
                    if (curve.quickEquals((*gammas)[0])) {
                        fSrcGammaTables[i] = fSrcGammaTables[0];
                        continue;
                    }
                }

                if (curve.isNamed()) {
                    switch (curve.fNamed) {
                        case SkColorSpace::kSRGB_GammaNamed:
                            fSrcGammaTables[i] = sk_linear_from_srgb;
                            break;
                        case SkColorSpace::k2Dot2Curve_GammaNamed:
                            fSrcGammaTables[i] = sk_linear_from_2dot2;
                            break;
                        case SkColorSpace::kLinear_GammaNamed:
                            build_table_linear_from_gamma(&fSrcGammaTableStorage[i * 256], 1.0f);
                            fSrcGammaTables[i] = &fSrcGammaTableStorage[i * 256];
                            break;
                        default:
                            SkASSERT(false);
                            break;
                    }
                } else if (curve.isValue()) {
                    build_table_linear_from_gamma(&fSrcGammaTableStorage[i * 256], curve.fValue);
                    fSrcGammaTables[i] = &fSrcGammaTableStorage[i * 256];
                } else if (curve.isTable()) {
                    build_table_linear_from_gamma(&fSrcGammaTableStorage[i * 256],
                                                  curve.fTable.get(), curve.fTableSize);
                    fSrcGammaTables[i] = &fSrcGammaTableStorage[i * 256];
                } else {
                    SkASSERT(curve.isParametric());
                    build_table_linear_from_gamma(&fSrcGammaTableStorage[i * 256], curve.fG,
                                                  curve.fA, curve.fB, curve.fC, curve.fD, curve.fE,
                                                  curve.fF);
                    fSrcGammaTables[i] = &fSrcGammaTableStorage[i * 256];
                }
            }
        }
    }

    // Build tables to transform linear to dst gamma.
    switch (dstSpace->gammaNamed()) {
        case SkColorSpace::kSRGB_GammaNamed:
            fDstGammaTables[0] = fDstGammaTables[1] = fDstGammaTables[2] = linear_to_srgb;
            break;
        case SkColorSpace::k2Dot2Curve_GammaNamed:
            fDstGammaTables[0] = fDstGammaTables[1] = fDstGammaTables[2] = linear_to_2dot2;
            break;
        case SkColorSpace::kLinear_GammaNamed:
            build_table_linear_to_gamma(fDstGammaTableStorage, kDstGammaTableSize, 1.0f);
            fDstGammaTables[0] = fDstGammaTables[1] = fDstGammaTables[2] = fDstGammaTableStorage;
            break;
        default: {
            const SkGammas* gammas = as_CSB(dstSpace)->gammas();
            SkASSERT(gammas);

            for (int i = 0; i < 3; i++) {
                const SkGammaCurve& curve = (*gammas)[i];

                if (i > 0) {
                    // Check if this curve matches the first curve.  In this case, we can
                    // share the same table pointer.  Logically, this should almost always
                    // be true.  I've never seen a profile where all three gamma curves
                    // didn't match.  But it is possible that they won't.
                    // TODO (msarett):
                    // This comparison won't catch the case where each gamma curve has a
                    // pointer to its own look-up table (but the tables actually match).
                    // Should we perform a deep compare of gamma tables here?  Or should
                    // we catch this when parsing the profile?  Or should we not worry
                    // about a bit of redundant work?
                    if (curve.quickEquals((*gammas)[0])) {
                        fDstGammaTables[i] = fDstGammaTables[0];
                        continue;
                    }
                }

                if (curve.isNamed()) {
                    switch (curve.fNamed) {
                        case SkColorSpace::kSRGB_GammaNamed:
                            fDstGammaTables[i] = linear_to_srgb;
                            break;
                        case SkColorSpace::k2Dot2Curve_GammaNamed:
                            fDstGammaTables[i] = linear_to_2dot2;
                            break;
                        case SkColorSpace::kLinear_GammaNamed:
                            build_table_linear_to_gamma(
                                    &fDstGammaTableStorage[i * kDstGammaTableSize],
                                    kDstGammaTableSize, 1.0f);
                            fDstGammaTables[i] = &fDstGammaTableStorage[i * kDstGammaTableSize];
                            break;
                        default:
                            SkASSERT(false);
                            break;
                    }
                } else if (curve.isValue()) {
                    build_table_linear_to_gamma(&fDstGammaTableStorage[i * kDstGammaTableSize],
                                                kDstGammaTableSize, curve.fValue);
                    fDstGammaTables[i] = &fDstGammaTableStorage[i * kDstGammaTableSize];
                } else if (curve.isTable()) {
                    build_table_linear_to_gamma(&fDstGammaTableStorage[i * kDstGammaTableSize],
                                                kDstGammaTableSize, curve.fTable.get(),
                                                curve.fTableSize);
                    fDstGammaTables[i] = &fDstGammaTableStorage[i * kDstGammaTableSize];
                } else {
                    SkASSERT(curve.isParametric());
                    build_table_linear_to_gamma(&fDstGammaTableStorage[i * kDstGammaTableSize],
                                                kDstGammaTableSize, curve.fG, curve.fA, curve.fB,
                                                curve.fC, curve.fD, curve.fE, curve.fF);
                    fDstGammaTables[i] = &fDstGammaTableStorage[i * kDstGammaTableSize];
                }
            }
        }
    }
}

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
    float* ptr = &colorLUT->fTable[ix*n001 + iy*n010 + iz*n100];

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

void SkDefaultXform::xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const {
    while (len-- > 0) {
        uint8_t r = (*src >>  0) & 0xFF,
                g = (*src >>  8) & 0xFF,
                b = (*src >> 16) & 0xFF;

        if (fColorLUT) {
            float in[3];
            float out[3];

            in[0] = byte_to_float(r);
            in[1] = byte_to_float(g);
            in[2] = byte_to_float(b);

            interp_3d_clut(out, in, fColorLUT.get());

            r = sk_float_round2int(255.0f * clamp_normalized_float(out[0]));
            g = sk_float_round2int(255.0f * clamp_normalized_float(out[1]));
            b = sk_float_round2int(255.0f * clamp_normalized_float(out[2]));
        }

        // Convert to linear.
        float srcFloats[3];
        srcFloats[0] = fSrcGammaTables[0][r];
        srcFloats[1] = fSrcGammaTables[1][g];
        srcFloats[2] = fSrcGammaTables[2][b];

        // Convert to dst gamut.
        float dstFloats[3];
        dstFloats[0] = srcFloats[0] * fSrcToDst.getFloat(0, 0) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 0) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 0) + fSrcToDst.getFloat(3, 0);
        dstFloats[1] = srcFloats[0] * fSrcToDst.getFloat(0, 1) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 1) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 1) + fSrcToDst.getFloat(3, 1);
        dstFloats[2] = srcFloats[0] * fSrcToDst.getFloat(0, 2) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 2) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 2) + fSrcToDst.getFloat(3, 2);

        // Clamp to 0-1.
        dstFloats[0] = clamp_normalized_float(dstFloats[0]);
        dstFloats[1] = clamp_normalized_float(dstFloats[1]);
        dstFloats[2] = clamp_normalized_float(dstFloats[2]);

        // Convert to dst gamma.
        r = fDstGammaTables[0][sk_float_round2int((kDstGammaTableSize - 1) * dstFloats[0])];
        g = fDstGammaTables[1][sk_float_round2int((kDstGammaTableSize - 1) * dstFloats[1])];
        b = fDstGammaTables[2][sk_float_round2int((kDstGammaTableSize - 1) * dstFloats[2])];

        *dst = SkPackARGB32NoCheck(0xFF, r, g, b);

        dst++;
        src++;
    }
}
