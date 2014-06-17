
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPathOpsDebug.h"
#include "SkPicture.h"
#include "SkRTConf.h"
#include "SkTSort.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkThreadPool.h"
#include "SkTime.h"
#include "Test.h"

#ifdef SK_BUILD_FOR_WIN
    #define PATH_SLASH "\\"
    #define IN_DIR "D:\\skp\\slave"
    #define OUT_DIR "D:\\skpOut\\1\\"
#else
    #define PATH_SLASH "/"
    #define IN_DIR "/skp/2311328-7fc2228/slave"
    #define OUT_DIR "/skpOut/4/"
#endif

const struct {
    int directory;
    const char* filename;
} skipOverSept[] = {
    { 3, "http___www_americascup_com_.skp"},  // !simple->closed()
    {18, "http___www_argus_presse_fr_.skp"},  // can't find winding of remaining vertical edge
    {31, "http___www_narayana_verlag_de_.skp"},  // !simple->closed()
    {36, "http___www_educationalcraft_com_.skp"},  // cubic / cubic near end / assert in SkIntersections::insert
    {44, "http___www_cooksnaps_com_.skp"},  // !simple->isClosed()
    {48, "http___www_narayana_publishers_com_.skp"},  // !simple->isClosed()
    {51, "http___www_freedominthe50states_org_.skp"},  // corrupt dash data
    {52, "http___www_aceinfographics_com_.skp"},  // right angle winding assert
    {53, "http___www_lojaanabotafogo_com_br_.skp"},  // rrect validate assert
    {57, "http___www_vantageproduction_com_.skp"},  // !isClosed()
    {64, "http___www_etiqadd_com_.skp"},  // !simple->closed()
    {84, "http___www_swapspacesystems_com_.skp"},  // !simple->closed()
    {90, "http___www_tcmevents_org_.skp"},  // !simple->closed()
    {96, "http___www_paseoitaigara_com_br_.skp"},  // !simple->closed()
    {98, "http___www_mortgagemarketguide_com_.skp"},  // !simple->closed()
    {99, "http___www_kitcheninspirations_wordpress_com_.skp"},  // checkSmall / bumpSpan
};

/* stats
97 http___www_brandyandvinca_com_.skp pixelError=3
95 http___www_into_asia_com_.skp pixelError=12
93 http___www_lunarplanner_com_.skp pixelError=14
98 http___www_lovelyitalia_com_.skp pixelError=17
90 http___www_inter_partner_blogspot_com_.skp pixelError=18
99 http___www_maxarea_com_.skp pixelError=26
98 http___www_maroonsnet_org_.skp pixelError=33
92 http___www_belinaart_ru_.skp pixelError=50
100 http___www_chroot_ro_.skp pixelError=62
99 http___www_hsbrands_com_.skp pixelError=98
95 http___www_tournamentindicator_com_.skp pixelError=122
93 http___www_businesses_com_au_.skp pixelError=162
90 http___www_regenesys_net_.skp pixelError=182
88 http___www_1863544208148625103_c18eac63985503fa85b06358959c1ba27fc36f82_blogspot_com_.skp pixelError=186
97 http___www_pregacoesevangelica_com_br_.skp pixelError=240
77 http___www_zhenggang_org_.skp pixelError=284
96 http___slidesharemailer_com_.skp pixelError=522
94 http___www_gensteel_com_.skp pixelError=555
68 http___www_jf_eti_br_.skp pixelError=610
83 http___www_swishiat_com_.skp pixelError=706
96 http___www_matusikmissive_com_au_.skp pixelError=2580
95 http___www_momentumnation_com_.skp pixelError=3938
92 http___www_rssowl_com_.skp pixelError=5113
96 http___www_sexxygirl_tv_.skp pixelError=7605
99 http___www_georgevalah_wordpress_com_.skp pixelError=8386
78 http___www_furbo_org_.skp pixelError=8656
78 http___www_djxhemary_wordpress_com_.skp pixelError=8976
100 http___www_mindcontrolblackassassins_com_.skp pixelError=31950
98 http___bababillgates_free_fr_.skp pixelError=40237
98 http___hepatite_ro_.skp pixelError=44370
86 http___www_somethingwagging_com_.skp pixelError=47794
84 http___www_beverageuniverse_com_.skp pixelError=65450
50 http___www_aveksa_com_.skp pixelError=68194
10 http___www_publiker_pl_.skp pixelError=89997
61 http___www_dominos_co_id_.skp pixelError=476868
87 http___www_du_edu_om_.skp time=46
87 http___www_bigload_de_.skp time=46
100 http___www_home_forum_com_.skp time=48
97 http___www_hotamateurchat_com_.skp time=48
97 http___www_myrsky_com_cn_.skp time=48
98 http___www_techiegeex_com_.skp time=49
82 http___www_fashionoutletsofchicago_com_.skp time=50
77 http___www_dynamischbureau_nl_.skp time=50
82 http___www_mayihelpu_co_in_.skp time=50
84 http___www_vbox7_com_user_history_viewers_.skp time=50
85 http___www_ktokogda_com_.skp time=50
85 http___www_propertyturkeysale_com_.skp time=50
85 http___www_51play_com_.skp time=50
86 http___www_bayalarm_com_.skp time=50
87 http___www_eaglepictures_com_.skp time=50
88 http___www_atlasakvaryum_com_.skp time=50
91 http___www_pioneerchryslerjeep_com_.skp time=50
94 http___www_thepulsemag_com_.skp time=50
95 http___www_dcshoes_com_ph_.skp time=50
96 http___www_montrealmassage_ca_.skp time=50
96 http___www_jkshahclasses_com_.skp time=50
96 http___www_webcamconsult_com_.skp time=51
100 http___www_bsoscblog_com_.skp time=52
95 http___www_flaktwoods_com_.skp time=53
91 http___www_qivivo_com_.skp time=54
90 http___www_unitender_com_.skp time=56
97 http___www_casinogaming_com_.skp time=56
97 http___www_rootdownload_com_.skp time=56
94 http___www_aspa_ev_de_.skp time=57
98 http___www_tenpieknyswiat_pl_.skp time=57
93 http___www_transocean_de_.skp time=58
94 http___www_vdo2_blogspot_com_.skp time=58
94 http___www_asmaissexy_com_br_.skp time=58
100 http___www_prefeiturasjm_com_br_.skp time=60
100 http___www_eduinsuranceclick_blogspot_com_.skp time=60
96 http___www_bobdunsire_com_.skp time=61
96 http___www_omgkettlecorn_com_.skp time=61
85 http___www_fbbsessions_com_.skp time=62
86 http___www_hector_ru_.skp time=62
87 http___www_wereldsupporter_nl_.skp time=62
90 http___www_arello_com_.skp time=62
93 http___www_bayerplastics_com_.skp time=62
93 http___www_superandolamovida_com_ar_.skp time=62
96 http___www_med_rbf_ru_.skp time=62
81 http___www_carnegiescience_edu_.skp time=65
87 http___www_asanewengland_com_.skp time=65
92 http___www_turkce_karakter_appspot_com_.skp time=65
94 http___www_k3a_org_.skp time=65
96 http___www_powermaccenter_com_.skp time=65
98 http___www_avto49_ru_.skp time=67
100 http___www_hetoldeambaecht_nl_.skp time=68
95 http___www_marine_ie_.skp time=69
96 http___www_quebecvapeboutique_com_.skp time=69
95 http___www_brays_ingles_com_.skp time=70
100 http___www_lacondesa_com_.skp time=72
95 http___www_timbarrathai_com_au_.skp time=76
95 http___www_cuissedegrenouille_com_.skp time=76
95 http___www_iwama51_ru_.skp time=76
99 http___www_fotoantologia_it_.skp time=76
92 http___www_indian_architects_com_.skp time=78
92 http___www_totalwomanspa_com_.skp time=78
100 http___www_fachverband_spielhallen_de_.skp time=83
93 http___www_golshanemehr_ir_.skp time=84
95 http___www_maryesses_com_.skp time=84
99 http___www_ddcorp_ca_.skp time=89
90 http___www_brontops_com_.skp time=89
94 http___www_robgolding_com_.skp time=89
91 http___www_tecban_com_br_.skp time=91
98 http___www_costamesakarate_com_.skp time=100
95 http___www_monsexyblog_com_.skp time=103
97 http___www_stornowaygazette_co_uk_.skp time=103
93 http___www_fitforaframe_com_.skp time=104
98 http___www_intentionoftheday_com_.skp time=113
100 http___www_tailgateclothing_com_.skp time=117
95 http___www_senbros_com_.skp time=118
93 http___www_lettoblog_com_.skp time=121
94 http___www_maxineschallenge_com_au_.skp time=125
95 http___www_savvycard_net_.skp time=127
95 http___www_open_ac_mu_.skp time=129
96 http___www_avgindia_in_.skp time=135
97 http___www_stocktonseaview_com_.skp time=135
96 http___www_distroller_com_.skp time=142
94 http___www_travoggalop_dk_.skp time=144
100 http___www_history_im_.skp time=144
94 http___www_playradio_sk_.skp time=145
92 http___www_linglongglass_com_.skp time=151
97 http___www_bizzna_com_.skp time=151
96 http___www_spiros_ws_.skp time=154
91 http___www_rosen_meents_co_il_.skp time=156
81 http___www_hoteldeluxeportland_com_.skp time=158
92 http___www_freetennis_org_.skp time=161
93 http___www_aircharternetwork_com_au_.skp time=161
94 http___www_austinparks_org_.skp time=165
89 http___www_bevvy_co_.skp time=168
91 http___www_sosyalhile_net_.skp time=168
98 http___www_minvih_gob_ve_.skp time=171
89 http___www_streetfoodmtl_com_.skp time=172
92 http___www_loveslatinas_tumblr_com_.skp time=178
93 http___www_madbites_co_in_.skp time=180
94 http___www_rocktarah_ir_.skp time=185
97 http___www_penthouselife_com_.skp time=185
96 http___www_appymonkey_com_.skp time=196
92 http___www_pasargadhotels_com_.skp time=203
99 http___www_marina_mil_pe_.skp time=203
89 http___www_kays_co_uk_.skp time=205
77 http___www_334588_com_.skp time=211
83 http___www_trendbad24_de_.skp time=211
81 http___www_cdnetworks_co_kr_.skp time=216
94 http___www_schellgames_com_.skp time=223
95 http___www_juliaweddingnews_cn_.skp time=230
92 http___www_xcrafters_pl_.skp time=253
93 http___www_pondoo_com_.skp time=253
96 http___www_helsinkicapitalpartners_fi_.skp time=255
88 http___www_nadtexican_com_.skp time=259
85 http___www_canstockphoto_hu_.skp time=266
78 http___www_ecovacs_com_cn_.skp time=271
93 http___www_brookfieldplaceny_com_.skp time=334
93 http___www_fmastrengthtraining_com_.skp time=337
94 http___www_turtleonthebeach_com_.skp time=394
90 http___www_temptationthemovie_com_.skp time=413
95 http___www_patongsawaddi_com_.skp time=491
91 http___www_online_radio_appspot_com_.skp time=511
68 http___www_richardmiller_co_uk_.skp time=528
63 http___www_eschrade_com_.skp time=543
55 http___www_interaction_inf_br_.skp time=625
38 http___www_huskyliners_com_.skp time=632
86 http___granda_net_.skp time=1067
24 http___www_cocacolafm_com_br_.skp time=1081
*/

size_t skipOverSeptCount = sizeof(skipOverSept) / sizeof(skipOverSept[0]);

enum TestStep {
    kCompareBits,
    kEncodeFiles,
};

enum {
    kMaxLength = 256,
    kMaxFiles = 128,
    kSmallLimit = 1000,
};

struct TestResult {
    void init(int dirNo) {
        fDirNo = dirNo;
        sk_bzero(fFilename, sizeof(fFilename));
        fTestStep = kCompareBits;
        fScale = 1;
    }

    SkString status() {
        SkString outStr;
        outStr.printf("%s %d %d\n", fFilename, fPixelError, fTime);
        return outStr;
    }

    SkString progress() {
        SkString outStr;
        outStr.printf("dir=%d %s ", fDirNo, fFilename);
        if (fPixelError) {
            outStr.appendf(" err=%d", fPixelError);
        }
        if (fTime) {
            outStr.appendf(" time=%d", fTime);
        }
        if (fScale != 1) {
            outStr.appendf(" scale=%d", fScale);
        }
        outStr.appendf("\n");
        return outStr;

    }

    static void Test(int dirNo, const char* filename, TestStep testStep) {
        TestResult test;
        test.init(dirNo);
        test.fTestStep = testStep;
        strcpy(test.fFilename, filename);
        test.testOne();
    }

    void test(int dirNo, const SkString& filename) {
        init(dirNo);
        strcpy(fFilename, filename.c_str());
        testOne();
    }

    void testOne();

    char fFilename[kMaxLength];
    TestStep fTestStep;
    int fDirNo;
    int fPixelError;
    int fTime;
    int fScale;
};

class SortByPixel : public TestResult {
public:
    bool operator<(const SortByPixel& rh) const {
        return fPixelError < rh.fPixelError;
    }
};

class SortByTime : public TestResult {
public:
    bool operator<(const SortByTime& rh) const {
        return fTime < rh.fTime;
    }
};

class SortByName : public TestResult {
public:
    bool operator<(const SortByName& rh) const {
        return strcmp(fFilename, rh.fFilename) < 0;
    }
};

struct TestState {
    void init(int dirNo, skiatest::Reporter* reporter) {
        fReporter = reporter;
        fResult.init(dirNo);
    }

    SkTDArray<SortByPixel> fPixelWorst;
    SkTDArray<SortByTime> fSlowest;
    skiatest::Reporter* fReporter;
    TestResult fResult;
};

struct TestRunner {
    TestRunner(skiatest::Reporter* reporter, int threadCount)
        : fNumThreads(threadCount)
        , fReporter(reporter) {
    }

    ~TestRunner();
    void render();
    int fNumThreads;
    SkTDArray<class TestRunnable*> fRunnables;
    skiatest::Reporter* fReporter;
};

class TestRunnable : public SkRunnable {
public:
    virtual void run() SK_OVERRIDE {
        SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);
        (*fTestFun)(&fState);
    }

    TestState fState;
    void (*fTestFun)(TestState*);
};


class TestRunnableDir : public TestRunnable {
public:
    TestRunnableDir(void (*testFun)(TestState*), int dirNo, TestRunner* runner) {
        fState.init(dirNo, runner->fReporter);
        fTestFun = testFun;
    }

};

class TestRunnableFile : public TestRunnable {
public:
    TestRunnableFile(void (*testFun)(TestState*), int dirNo, const char* name, TestRunner* runner) {
        fState.init(dirNo, runner->fReporter);
        strcpy(fState.fResult.fFilename, name);
        fTestFun = testFun;
    }
};

class TestRunnableEncode : public TestRunnableFile {
public:
    TestRunnableEncode(void (*testFun)(TestState*), int dirNo, const char* name, TestRunner* runner)
        : TestRunnableFile(testFun, dirNo, name, runner) {
        fState.fResult.fTestStep = kEncodeFiles;
    }
};

TestRunner::~TestRunner() {
    for (int index = 0; index < fRunnables.count(); index++) {
        SkDELETE(fRunnables[index]);
    }
}

void TestRunner::render() {
    SkThreadPool pool(fNumThreads);
    for (int index = 0; index < fRunnables.count(); ++ index) {
        pool.add(fRunnables[index]);
    }
}

////////////////////////////////////////////////

static const char outOpDir[] = OUT_DIR "opClip";
static const char outOldDir[] = OUT_DIR "oldClip";
static const char outSkpDir[] = OUT_DIR "skpTest";
static const char outDiffDir[] = OUT_DIR "outTest";
static const char outStatusDir[] = OUT_DIR "statusTest";

static SkString make_filepath(int dirNo, const char* dir, const char* name) {
    SkString path(dir);
    if (dirNo) {
        path.appendf("%d", dirNo);
    }
    path.append(PATH_SLASH);
    path.append(name);
    return path;
}

static SkString make_in_dir_name(int dirNo) {
    SkString dirName(IN_DIR);
    dirName.appendf("%d", dirNo);
    if (!sk_exists(dirName.c_str())) {
        SkDebugf("could not read dir %s\n", dirName.c_str());
        return SkString();
    }
    return dirName;
}

static SkString make_stat_dir_name(int dirNo) {
    SkString dirName(outStatusDir);
    dirName.appendf("%d", dirNo);
    if (!sk_exists(dirName.c_str())) {
        SkDebugf("could not read dir %s\n", dirName.c_str());
        return SkString();
    }
    return dirName;
}

static bool make_one_out_dir(const char* outDirStr) {
    SkString outDir = make_filepath(0, outDirStr, "");
    if (!sk_exists(outDir.c_str())) {
        if (!sk_mkdir(outDir.c_str())) {
            SkDebugf("could not create dir %s\n", outDir.c_str());
            return false;
        }
    }
    return true;
}

static bool make_out_dirs() {
    SkString outDir = make_filepath(0, OUT_DIR, "");
    if (!sk_exists(outDir.c_str())) {
        if (!sk_mkdir(outDir.c_str())) {
            SkDebugf("could not create dir %s\n", outDir.c_str());
            return false;
        }
    }
    return make_one_out_dir(outOldDir)
            && make_one_out_dir(outOpDir)
            && make_one_out_dir(outSkpDir)
            && make_one_out_dir(outDiffDir)
            && make_one_out_dir(outStatusDir);
}

static SkString make_png_name(const char* filename) {
    SkString pngName = SkString(filename);
    pngName.remove(pngName.size() - 3, 3);
    pngName.append("png");
    return pngName;
}

static int similarBits(const SkBitmap& gr, const SkBitmap& sk) {
    const int kRowCount = 3;
    const int kThreshold = 3;
    int width = SkTMin(gr.width(), sk.width());
    if (width < kRowCount) {
        return true;
    }
    int height = SkTMin(gr.height(), sk.height());
    if (height < kRowCount) {
        return true;
    }
    int errorTotal = 0;
    SkTArray<int, true> errorRows;
    errorRows.push_back_n(width * kRowCount);
    SkAutoLockPixels autoGr(gr);
    SkAutoLockPixels autoSk(sk);
    for (int y = 0; y < height; ++y) {
        SkPMColor* grRow = gr.getAddr32(0, y);
        SkPMColor* skRow = sk.getAddr32(0, y);
        int* base = &errorRows[0];
        int* cOut = &errorRows[y % kRowCount];
        for (int x = 0; x < width; ++x) {
            SkPMColor grColor = grRow[x];
            SkPMColor skColor = skRow[x];
            int dr = SkGetPackedR32(grColor) - SkGetPackedR32(skColor);
            int dg = SkGetPackedG32(grColor) - SkGetPackedG32(skColor);
            int db = SkGetPackedB32(grColor) - SkGetPackedB32(skColor);
            int error = cOut[x] = SkTMax(SkAbs32(dr), SkTMax(SkAbs32(dg), SkAbs32(db)));
            if (error < kThreshold || x < 2) {
                continue;
            }
            if (base[x - 2] < kThreshold
                    || base[width + x - 2] < kThreshold
                    || base[width * 2 + x - 2] < kThreshold
                    || base[x - 1] < kThreshold
                    || base[width + x - 1] < kThreshold
                    || base[width * 2 + x - 1] < kThreshold
                    || base[x] < kThreshold
                    || base[width + x] < kThreshold
                    || base[width * 2 + x] < kThreshold) {
                continue;
            }
            errorTotal += error;
        }
    }
    return errorTotal;
}

static bool addError(TestState* data, const TestResult& testResult) {
    if (testResult.fPixelError <= 0 && testResult.fTime <= 0) {
        return false;
    }
    int worstCount = data->fPixelWorst.count();
    int pixelError = testResult.fPixelError;
    if (pixelError > 0) {
        for (int index = 0; index < worstCount; ++index) {
            if (pixelError > data->fPixelWorst[index].fPixelError) {
                data->fPixelWorst[index] = *(SortByPixel*) &testResult;
                return true;
            }
        }
    }
    int slowCount = data->fSlowest.count();
    int time = testResult.fTime;
    if (time > 0) {
        for (int index = 0; index < slowCount; ++index) {
            if (time > data->fSlowest[index].fTime) {
                data->fSlowest[index] = *(SortByTime*) &testResult;
                return true;
            }
        }
    }
    if (pixelError > 0 && worstCount < kMaxFiles) {
        *data->fPixelWorst.append() = *(SortByPixel*) &testResult;
        return true;
    }
    if (time > 0 && slowCount < kMaxFiles) {
        *data->fSlowest.append() = *(SortByTime*) &testResult;
        return true;
    }
    return false;
}

static SkMSec timePict(SkPicture* pic, SkCanvas* canvas) {
    canvas->save();
    int pWidth = pic->width();
    int pHeight = pic->height();
    const int maxDimension = 1000;
    const int slices = 3;
    int xInterval = SkTMax(pWidth - maxDimension, 0) / (slices - 1);
    int yInterval = SkTMax(pHeight - maxDimension, 0) / (slices - 1);
    SkRect rect = {0, 0, SkIntToScalar(SkTMin(maxDimension, pWidth)),
            SkIntToScalar(SkTMin(maxDimension, pHeight))};
    canvas->clipRect(rect);
    SkMSec start = SkTime::GetMSecs();
    for (int x = 0; x < slices; ++x) {
        for (int y = 0; y < slices; ++y) {
            pic->draw(canvas);
            canvas->translate(0, SkIntToScalar(yInterval));
        }
        canvas->translate(SkIntToScalar(xInterval), SkIntToScalar(-yInterval * slices));
    }
    SkMSec end = SkTime::GetMSecs();
    canvas->restore();
    return end - start;
}

static void drawPict(SkPicture* pic, SkCanvas* canvas, int scale) {
    canvas->clear(SK_ColorWHITE);
    if (scale != 1) {
        canvas->save();
        canvas->scale(1.0f / scale, 1.0f / scale);
    }
    pic->draw(canvas);
    if (scale != 1) {
        canvas->restore();
    }
}

static void writePict(const SkBitmap& bitmap, const char* outDir, const char* pngName) {
    SkString outFile = make_filepath(0, outDir, pngName);
    if (!SkImageEncoder::EncodeFile(outFile.c_str(), bitmap,
            SkImageEncoder::kPNG_Type, 100)) {
        SkDebugf("unable to encode gr %s (width=%d height=%d)\n", pngName,
                    bitmap.width(), bitmap.height());
    }
}

void TestResult::testOne() {
    SkPicture* pic = NULL;
    {
    #if DEBUG_SHOW_TEST_NAME
        if (fTestStep == kCompareBits) {
            SkString testName(fFilename);
            const char http[] = "http";
            if (testName.startsWith(http)) {
                testName.remove(0, sizeof(http) - 1);
            }
            while (testName.startsWith("_")) {
                testName.remove(0, 1);
            }
            const char dotSkp[] = ".skp";
            if (testName.endsWith(dotSkp)) {
                size_t len = testName.size();
                testName.remove(len - (sizeof(dotSkp) - 1), sizeof(dotSkp) - 1);
            }
            testName.prepend("skp");
            testName.append("1");
            strncpy(DEBUG_FILENAME_STRING, testName.c_str(), DEBUG_FILENAME_STRING_LENGTH);
        } else if (fTestStep == kEncodeFiles) {
            strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
        }
    #endif
        SkString path = make_filepath(fDirNo, IN_DIR, fFilename);
        SkFILEStream stream(path.c_str());
        if (!stream.isValid()) {
            SkDebugf("invalid stream %s\n", path.c_str());
            goto finish;
        }
        pic = SkPicture::CreateFromStream(&stream, &SkImageDecoder::DecodeMemory);
        if (!pic) {
            SkDebugf("unable to decode %s\n", fFilename);
            goto finish;
        }
        int width = pic->width();
        int height = pic->height();
        SkBitmap oldBitmap, opBitmap;
        fScale = 1;
        while (width / fScale > 32767 || height / fScale > 32767) {
            ++fScale;
        }
        do {
            int dimX = (width + fScale - 1) / fScale;
            int dimY = (height + fScale - 1) / fScale;
            if (oldBitmap.allocN32Pixels(dimX, dimY) &&
                opBitmap.allocN32Pixels(dimX, dimY)) {
                break;
            }
            SkDebugf("-%d-", fScale);
        } while (++fScale < 256);
        if (fScale >= 256) {
            SkDebugf("unable to allocate bitmap for %s (w=%d h=%d)\n", fFilename,
                    width, height);
            goto finish;
        }
        oldBitmap.eraseColor(SK_ColorWHITE);
        SkCanvas oldCanvas(oldBitmap);
        oldCanvas.setAllowSimplifyClip(false);
        opBitmap.eraseColor(SK_ColorWHITE);
        SkCanvas opCanvas(opBitmap);
        opCanvas.setAllowSimplifyClip(true);
        drawPict(pic, &oldCanvas, fScale);
        drawPict(pic, &opCanvas, fScale);
        if (fTestStep == kCompareBits) {
            fPixelError = similarBits(oldBitmap, opBitmap);
            int oldTime = timePict(pic, &oldCanvas);
            int opTime = timePict(pic, &opCanvas);
            fTime = SkTMax(0, oldTime - opTime);
        } else if (fTestStep == kEncodeFiles) {
            SkString pngStr = make_png_name(fFilename);
            const char* pngName = pngStr.c_str();
            writePict(oldBitmap, outOldDir, pngName);
            writePict(opBitmap, outOpDir, pngName);
        }
    }
finish:
    if (pic) {
        pic->unref();
    }
}

static SkString makeStatusString(int dirNo) {
    SkString statName;
    statName.printf("stats%d.txt", dirNo);
    SkString statusFile = make_filepath(0, outStatusDir, statName.c_str());
    return statusFile;
}

class PreParser {
public:
    PreParser(int dirNo, bool threaded)
        : fDirNo(dirNo)
        , fIndex(0)
        , fThreaded(threaded) {
        SkString statusPath = makeStatusString(dirNo);
        if (!sk_exists(statusPath.c_str())) {
            return;
        }
        SkFILEStream reader;
        reader.setPath(statusPath.c_str());
        while (fetch(reader, &fResults.push_back()))
            ;
        fResults.pop_back();
    }

    bool fetch(SkFILEStream& reader, TestResult* result) {
        char c;
        int i = 0;
        result->init(fDirNo);
        result->fPixelError = 0;
        result->fTime = 0;
        do {
            bool readOne = reader.read(&c, 1) != 0;
            if (!readOne) {
//                SkASSERT(i == 0);   // the current text may be incomplete -- if so, ignore it
                return false;
            }
            if (c == ' ') {
                result->fFilename[i++] = '\0';
                break;
            }
            result->fFilename[i++] = c;
            SkASSERT(i < kMaxLength);
        } while (true);
        do {
            if (!reader.read(&c, 1)) {
                return false;
            }
            if (c == ' ') {
                break;
            }
            SkASSERT(c >= '0' && c <= '9');
            result->fPixelError = result->fPixelError * 10 + (c - '0');
        } while (true);
        bool minus = false;
        do {
            if (!reader.read(&c, 1)) {
                return false;
            }
            if (c == '\n') {
                break;
            }
            if (c == '-') {
                minus = true;
                continue;
            }
            SkASSERT(c >= '0' && c <= '9');
            result->fTime = result->fTime * 10 + (c - '0');
        } while (true);
        if (minus) {
            result->fTime = -result->fTime;
        }
        return true;
    }

    bool match(const SkString& filename, SkFILEWStream* stream, TestResult* result) {
        if (fThreaded) {
            for (int index = 0; index < fResults.count(); ++index) {
                const TestResult& test = fResults[index];
                if (filename.equals(test.fFilename)) {
                    *result = test;
                    SkString outStr(result->status());
                    stream->write(outStr.c_str(), outStr.size());
                    return true;
                }
            }
        } else if (fIndex < fResults.count()) {
            *result = fResults[fIndex++];
            SkASSERT(filename.equals(result->fFilename));
            SkString outStr(result->status());
            stream->write(outStr.c_str(), outStr.size());
            return true;
        }
        return false;
    }

private:
    int fDirNo;
    int fIndex;
    SkTArray<TestResult, true> fResults;
    bool fThreaded;
};

static bool doOneDir(TestState* state, bool threaded) {
    int dirNo = state->fResult.fDirNo;
    skiatest::Reporter* reporter = state->fReporter;
    SkString dirName = make_in_dir_name(dirNo);
    if (!dirName.size()) {
        return false;
    }
    SkOSFile::Iter iter(dirName.c_str(), "skp");
    SkString filename;
    int testCount = 0;
    PreParser preParser(dirNo, threaded);
    SkFILEWStream statusStream(makeStatusString(dirNo).c_str());
    while (iter.next(&filename)) {
        for (size_t index = 0; index < skipOverSeptCount; ++index) {
            if (skipOverSept[index].directory == dirNo
                    && strcmp(filename.c_str(), skipOverSept[index].filename) == 0) {
                goto checkEarlyExit;
            }
        }
        if (preParser.match(filename, &statusStream, &state->fResult)) {
            (void) addError(state, state->fResult);
            ++testCount;
            goto checkEarlyExit;
        }
        {
            TestResult& result = state->fResult;
            result.test(dirNo, filename);
            SkString outStr(result.status());
            statusStream.write(outStr.c_str(), outStr.size());
            statusStream.flush();
            if (addError(state, result)) {
                SkDebugf("%s", result.progress().c_str());
            }
        }
        ++testCount;
        if (reporter->verbose()) {
            SkDebugf(".");
            if (++testCount % 100 == 0) {
                SkDebugf("%d\n", testCount);
            }
        }
checkEarlyExit:
        if (0 && testCount >= 1) {
            return true;
        }
    }
    return true;
}

static bool initTest() {
#if !defined SK_BUILD_FOR_WIN && !defined SK_BUILD_FOR_MAC
    SK_CONF_SET("images.jpeg.suppressDecoderWarnings", true);
    SK_CONF_SET("images.png.suppressDecoderWarnings", true);
#endif
    return make_out_dirs();
}

static bool initUberTest(int firstDirNo, int lastDirNo) {
    if (!initTest()) {
        return false;
    }
    for (int index = firstDirNo; index <= lastDirNo; ++index) {
        SkString statusDir(outStatusDir);
        statusDir.appendf("%d", index);
        if (!make_one_out_dir(statusDir.c_str())) {
            return false;
        }
    }
    return true;
}


static void testSkpClipEncode(TestState* data) {
    data->fResult.testOne();
    if (data->fReporter->verbose()) {
       SkDebugf("+");
    }
}

static void encodeFound(skiatest::Reporter* reporter, TestState& state) {
    if (reporter->verbose()) {
        if (state.fPixelWorst.count()) {
            SkTDArray<SortByPixel*> worst;
            for (int index = 0; index < state.fPixelWorst.count(); ++index) {
                *worst.append() = &state.fPixelWorst[index];
            }
            SkTQSort<SortByPixel>(worst.begin(), worst.end() - 1);
            for (int index = 0; index < state.fPixelWorst.count(); ++index) {
                const TestResult& result = *worst[index];
                SkDebugf("%d %s pixelError=%d\n", result.fDirNo, result.fFilename, result.fPixelError);
            }
        }
        if (state.fSlowest.count()) {
            SkTDArray<SortByTime*> slowest;
            for (int index = 0; index < state.fSlowest.count(); ++index) {
                *slowest.append() = &state.fSlowest[index];
            }
            if (slowest.count() > 0) {
                SkTQSort<SortByTime>(slowest.begin(), slowest.end() - 1);
                for (int index = 0; index < slowest.count(); ++index) {
                    const TestResult& result = *slowest[index];
                    SkDebugf("%d %s time=%d\n", result.fDirNo, result.fFilename, result.fTime);
                }
            }
        }
    }

    int threadCount = reporter->allowThreaded() ? SkThreadPool::kThreadPerCore : 1;
    TestRunner testRunner(reporter, threadCount);
    for (int index = 0; index < state.fPixelWorst.count(); ++index) {
        const TestResult& result = state.fPixelWorst[index];
        SkString filename(result.fFilename);
        if (!filename.endsWith(".skp")) {
            filename.append(".skp");
        }
        *testRunner.fRunnables.append() = SkNEW_ARGS(TestRunnableEncode,
                (&testSkpClipEncode, result.fDirNo, filename.c_str(), &testRunner));
    }
    testRunner.render();
#if 0
    for (int index = 0; index < state.fPixelWorst.count(); ++index) {
        const TestResult& result = state.fPixelWorst[index];
        SkString filename(result.fFilename);
        if (!filename.endsWith(".skp")) {
            filename.append(".skp");
        }
        TestResult::Test(result.fDirNo, filename.c_str(), kEncodeFiles);
        if (reporter->verbose()) SkDebugf("+");
    }
#endif
}

DEF_TEST(PathOpsSkpClip, reporter) {
    if (!initTest()) {
        return;
    }
    SkTArray<TestResult, true> errors;
    TestState state;
    state.init(0, reporter);
    for (int dirNo = 1; dirNo <= 100; ++dirNo) {
        if (reporter->verbose()) {
            SkDebugf("dirNo=%d\n", dirNo);
        }
        state.fResult.fDirNo = dirNo;
        if (!doOneDir(&state, false)) {
            break;
        }
    }
    encodeFound(reporter, state);
}

static void testSkpClipMain(TestState* data) {
        (void) doOneDir(data, true);
}

DEF_TEST(PathOpsSkpClipThreaded, reporter) {
    if (!initTest()) {
        return;
    }
    int threadCount = reporter->allowThreaded() ? SkThreadPool::kThreadPerCore : 1;
    TestRunner testRunner(reporter, threadCount);
    const int firstDirNo = 1;
    for (int dirNo = firstDirNo; dirNo <= 100; ++dirNo) {
        *testRunner.fRunnables.append() = SkNEW_ARGS(TestRunnableDir,
                (&testSkpClipMain, dirNo, &testRunner));
    }
    testRunner.render();
    TestState state;
    state.init(0, reporter);
    for (int dirNo = firstDirNo; dirNo <= 100; ++dirNo) {
        TestState& testState = testRunner.fRunnables[dirNo - 1]->fState;
        SkASSERT(testState.fResult.fDirNo == dirNo);
        for (int inner = 0; inner < testState.fPixelWorst.count(); ++inner) {
            addError(&state, testState.fPixelWorst[inner]);
        }
        for (int inner = 0; inner < testState.fSlowest.count(); ++inner) {
            addError(&state, testState.fSlowest[inner]);
        }
    }
    encodeFound(reporter, state);
}

static void testSkpClipUber(TestState* data) {
    data->fResult.testOne();
    SkString dirName = make_stat_dir_name(data->fResult.fDirNo);
    if (!dirName.size()) {
        return;
    }
    SkString statName(data->fResult.fFilename);
    SkASSERT(statName.endsWith(".skp"));
    statName.remove(statName.size() - 4, 4);
    statName.appendf(".%d.%d.skp", data->fResult.fPixelError, data->fResult.fTime);
    SkString statusFile = make_filepath(data->fResult.fDirNo, outStatusDir, statName.c_str());
    SkFILE* file = sk_fopen(statusFile.c_str(), kWrite_SkFILE_Flag);
    if (!file) {
            SkDebugf("failed to create %s", statusFile.c_str());
            return;
    }
    sk_fclose(file);
    if (data->fReporter->verbose()) {
        if (data->fResult.fPixelError || data->fResult.fTime) {
            SkDebugf("%s", data->fResult.progress().c_str());
        } else {
            SkDebugf(".");
        }
    }
}

static bool buildTests(skiatest::Reporter* reporter, int firstDirNo, int lastDirNo, SkTDArray<TestResult>* tests,
        SkTDArray<SortByName*>* sorted) {
    for (int dirNo = firstDirNo; dirNo <= lastDirNo; ++dirNo) {
        SkString dirName = make_stat_dir_name(dirNo);
        if (!dirName.size()) {
            return false;
        }
        SkOSFile::Iter iter(dirName.c_str(), "skp");
        SkString filename;
        while (iter.next(&filename)) {
            TestResult test;
            test.init(dirNo);
            SkString spaceFile(filename);
            char* spaces = spaceFile.writable_str();
            int spaceSize = (int) spaceFile.size();
            for (int index = 0; index < spaceSize; ++index) {
                if (spaces[index] == '.') {
                    spaces[index] = ' ';
                }
            }
            int success = sscanf(spaces, "%s %d %d skp", test.fFilename,
                    &test.fPixelError, &test.fTime);
            if (success < 3) {
                SkDebugf("failed to scan %s matched=%d\n", filename.c_str(), success);
                return false;
            }
            *tests[dirNo - firstDirNo].append() = test;
        }
        if (!sorted) {
            continue;
        }
        SkTDArray<TestResult>& testSet = tests[dirNo - firstDirNo];
        int count = testSet.count();
        for (int index = 0; index < count; ++index) {
            *sorted[dirNo - firstDirNo].append() = (SortByName*) &testSet[index];
        }
        if (sorted[dirNo - firstDirNo].count()) {
            SkTQSort<SortByName>(sorted[dirNo - firstDirNo].begin(),
                    sorted[dirNo - firstDirNo].end() - 1);
            if (reporter->verbose()) {
                SkDebugf("+");
            }
       }
    }
    return true;
}

bool Less(const SortByName& a, const SortByName& b);
bool Less(const SortByName& a, const SortByName& b) {
    return a < b;
}

DEF_TEST(PathOpsSkpClipUberThreaded, reporter) {
    const int firstDirNo = 1;
    const int lastDirNo = 100;
    if (!initUberTest(firstDirNo, lastDirNo)) {
        return;
    }
    const int dirCount = lastDirNo - firstDirNo + 1;
    SkTDArray<TestResult> tests[dirCount];
    SkTDArray<SortByName*> sorted[dirCount];
    if (!buildTests(reporter, firstDirNo, lastDirNo, tests, sorted)) {
        return;
    }
    int threadCount = reporter->allowThreaded() ? SkThreadPool::kThreadPerCore : 1;
    TestRunner testRunner(reporter, threadCount);
    for (int dirNo = firstDirNo; dirNo <= lastDirNo; ++dirNo) {
        SkString dirName = make_in_dir_name(dirNo);
        if (!dirName.size()) {
            continue;
        }
        SkOSFile::Iter iter(dirName.c_str(), "skp");
        SkString filename;
        while (iter.next(&filename)) {
            int count;
            SortByName name;
            for (size_t index = 0; index < skipOverSeptCount; ++index) {
                if (skipOverSept[index].directory == dirNo
                        && strcmp(filename.c_str(), skipOverSept[index].filename) == 0) {
                    goto checkEarlyExit;
                }
            }
            name.init(dirNo);
            strncpy(name.fFilename, filename.c_str(), filename.size() - 4);  // drop .skp
            count = sorted[dirNo - firstDirNo].count();
            if (SkTSearch<SortByName, Less>(sorted[dirNo - firstDirNo].begin(),
                    count, &name, sizeof(&name)) < 0) {
                *testRunner.fRunnables.append() = SkNEW_ARGS(TestRunnableFile,
                        (&testSkpClipUber, dirNo, filename.c_str(), &testRunner));
            }
    checkEarlyExit:
            ;
        }

    }
    testRunner.render();
    SkTDArray<TestResult> results[dirCount];
    if (!buildTests(reporter, firstDirNo, lastDirNo, results, NULL)) {
        return;
    }
    SkTDArray<TestResult> allResults;
    for (int dirNo = firstDirNo; dirNo <= lastDirNo; ++dirNo) {
        SkTDArray<TestResult>& array = results[dirNo - firstDirNo];
        allResults.append(array.count(), array.begin());
    }
    int allCount = allResults.count();
    SkTDArray<SortByPixel*> pixels;
    SkTDArray<SortByTime*> times;
    for (int index = 0; index < allCount; ++index) {
        *pixels.append() = (SortByPixel*) &allResults[index];
        *times.append() = (SortByTime*) &allResults[index];
    }
    TestState state;
    if (pixels.count()) {
        SkTQSort<SortByPixel>(pixels.begin(), pixels.end() - 1);
        for (int inner = 0; inner < kMaxFiles; ++inner) {
            *state.fPixelWorst.append() = *pixels[allCount - inner - 1];
        }
    }
    if (times.count()) {
        SkTQSort<SortByTime>(times.begin(), times.end() - 1);
        for (int inner = 0; inner < kMaxFiles; ++inner) {
            *state.fSlowest.append() = *times[allCount - inner - 1];
        }
    }
    encodeFound(reporter, state);
}

DEF_TEST(PathOpsSkpClipOneOff, reporter) {
    if (!initTest()) {
        return;
    }
    const int testIndex = 43 - 37;
    int dirNo = skipOverSept[testIndex].directory;
    SkAssertResult(make_in_dir_name(dirNo).size());
    SkString filename(skipOverSept[testIndex].filename);
    TestResult state;
    state.test(dirNo, filename);
    if (reporter->verbose()) {
        SkDebugf("%s", state.status().c_str());
    }
    state.fTestStep = kEncodeFiles;
    state.testOne();
}
