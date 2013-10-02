#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkBitmap.h"
#include "SkColor.h"
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkString.h"

#ifdef SK_BUILD_FOR_WIN
    #define PATH_SLASH "\\"
    #define IN_DIR "D:" PATH_SLASH "skp"
    #define OUT_DIR "D:" PATH_SLASH
#else
    #define PATH_SLASH "/"
    #if 1
        #define IN_DIR "/usr/local/google/home/caryclark/new10k" PATH_SLASH
        #define OUT_DIR "/usr/local/google/home/caryclark/out10k" PATH_SLASH
    #else
        #define IN_DIR "/usr/local/google/home/caryclark/6-18-13" PATH_SLASH
        #define OUT_DIR "/usr/local/google/home/caryclark" PATH_SLASH
    #endif
#endif

static const char pictDir[] = IN_DIR ;
static const char outSkpClipDir[] = OUT_DIR "skpClip";
static const char outOldClipDir[] = OUT_DIR "oldClip";

static SkString make_filepath(const char* dir, const SkString& name) {
    SkString path(dir);
    size_t len = strlen(dir);
    if (len > 0 && dir[len - 1] != PATH_SLASH[0]) {
        path.append(PATH_SLASH);
    }
    path.append(name);
    return path;
}

static SkString make_png_name(const SkString& filename) {
    SkString pngName = SkString(filename);
    pngName.remove(pngName.size() - 3, 3);
    pngName.append("png");
    return pngName;
}

static void testOne(const SkString& filename) {
    if (filename == SkString("http___migracioncolombia_gov_co.skp")
            || filename == SkString("http___miuki_info.skp")
    ) {
        return;
    }
#if DEBUG_SHOW_TEST_NAME
    SkString testName(filename);
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
#endif
    SkString path = make_filepath(pictDir, filename);
    SkFILEStream stream(path.c_str());
    if (!stream.isValid()) {
        return;
    }
    SkPicture* pic = SkPicture::CreateFromStream(&stream, &SkImageDecoder::DecodeMemory);
    if (!pic) {
        SkDebugf("unable to decode %s\n", filename.c_str());
        return;
    }
    int width = pic->width();
    int height = pic->height();

    SkBitmap bitmap;
    int scale = 1;
    do {
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, (width + scale - 1) / scale,
            (height + scale - 1) / scale);
        bool success = bitmap.allocPixels();
        bitmap.eraseColor(SK_ColorWHITE);
        if (success) {
            break;
        }
        SkDebugf("-%d-", scale);
    } while ((scale *= 2) < 32);
    if (scale >= 32) {
        SkDebugf("unable to allocate bitmap for %s (w=%d h=%d)\n", filename.c_str(),
                width, height);
        return;
    }
    SkCanvas canvas(bitmap);
    canvas.scale(1.0f / scale, 1.0f / scale);
    SkString pngName = make_png_name(filename);
    for (int i = 0; i < 2; ++i) {
        bool useOp = i ? true : false;
        canvas.setAllowSimplifyClip(useOp);
        pic->draw(&canvas);
        SkString outFile = make_filepath(useOp ? outSkpClipDir : outOldClipDir, pngName);
        if (!SkImageEncoder::EncodeFile(outFile.c_str(), bitmap, SkImageEncoder::kPNG_Type,
                100)) {
            SkDebugf("unable to encode %s (width=%d height=%d)\n", pngName.c_str(),
                     bitmap.width(), bitmap.height());
        }
    }
    SkDELETE(pic);
}

const char* tryFixed[] = {
    0
};

size_t tryFixedCount = sizeof(tryFixed) / sizeof(tryFixed[0]);

const char* skipOver[] = {
    "http___carpetplanet_ru.skp",  // cubic/cubic intersect
    "http___carrot_is.skp",  // bridgeOp()  SkASSERT(unsortable || !current->done());

/*!*/"http___dotsrc_org.skp",  // asserts in png decode
    "http___frauen_magazin_com.skp",  // bridgeOp()  SkASSERT(unsortable || !current->done());
    "http___i_gino_com.skp",  // unexpected cubic/quad coincidence
                            // {61, 857, 61, 789.06897, 116.068977, 734, 184, 734}
                            // {184, 734, 133.051727, 734, 97.0258636, 770.025879}
    "http___ilkoora_com.skp",  // assert wind sum != min32 from markDoneBinary / findNextOp #28k
/*!*/"http___migracioncolombia_gov_co.skp",  // crashes on picture decode
    "http___mm4everfriends_com.skp",  // bumpSpan/addTCoincident (from calcPartialCoincidentWinding)
    "http___mtrk_uz.skp",  // checkEnds() assert #36.3k
    "http___pchappy_com_au.skp",  // bridgeOp() assert unsortable || ! empty #37.2k
    "http___sciality_com.skp",  // bridgeOp()  SkASSERT(unsortable || !current->done()); #32.4k
/*!*/"http___sozialticker_com.skp",  // asserts in png decode
    "http___sudoestenegocios_com.skp",  // assert fT < 1 in addTCoincident
    "http___thesuburbanite_com.skp",  // bridgeOp()  SkASSERT(unsortable || !current->done());

    "http___fluentin3months_com.skp", // calcCommonCoincidentWinding from calcPartialCoincidentWinding #38.3k
    "http___teachersbadi_blogspot_in.skp",  // calcCommonCoincidentWinding from calcPartialCoincidentWinding #53.4k
    "http___wsms_ru.skp",  // assert wind sum != min32 from markDoneBinary / findNextOp #49.5k
    "http___voycer_de.skp",  // calcCommonCoincidentWinding from calcPartialCoincidentWinding #47k
    "http___77hz_jp.skp",  // addTCancel from calcCoincidentWinding #47.1k

    "http___hostloco_com.skp",  // t < 0  AddIntersectsT
/*!*/"http___oggicronaca_it.skp",  // asserts in png decode
    "http___sergeychunkevich_com.skp",  // t < 0  AddIntersectsT
    "http___tracksflow_com.skp",  // assert otherEnd >= 0 from nextChase
    "http___autobutler_dk.skp",  // t < 0  AddIntersectsT
    "http___onlinecollege_org.skp",  // bridgeOp() assert unsortable || ! empty #100.1k
    "http___national_com_au.skp",  // bridgeOp() assert unsortable || ! empty #110.2k
/*!*/"http___anitadongre_com.skp",  // exceptionally large width and height
    "http___rentacheat_com.skp",  // bridgeOp() assert unsortable || ! empty #110.8k
/*!*/"http___gruesse_de.skp",  // asserts in png decode
/*!*/"http___crn_in.png",  // width=1250047
    "http___breakmystyle_com.skp",  // assert qPt == lPt in quad intersection
    "http___naoxrane_ru.skp",  // assert t4+...t0 == 0 in quartic roots #128.3k
    "http___tcmevents_org.skp",  // assert in addTCoincident (from calcPartialCoincidentWinding) #143.3k
/*!*/"http___listbuildingcashsecrets_com.skp",  // asserts in png decode #152.7k
/*!*/"http___skyscraperpage_com.skp",  // asserts in png decode #155.5k
    "http___mlk_com.skp",  // bridgeOp() assert unsortable || ! empty #158.7k
    "http___sd_graphic_net.skp",  // bridgeOp() assert unsortable || ! empty #163.3k
    "http___kopepasah_com.skp",  // checkEnds() assert #188.2k
/*!*/"http___darkreloaded_com.skp",  // asserts in png decode #188.4k
    "http___redbullskatearcade_es.skp",  // bridgeOp() assert unsortable || ! empty #192.5k
    "http___partainasdemo250_org.skp",  //  bridgeOp() assert unsortable || ! empty #200.2k

// these failures are from the new 10k set
    "http___www_freerepublic_com_.skp",  // assert in opangle <
    "http___www_lavoixdunord_fr_.skp",  // bridgeOp() assert unsortable || ! empty
    "http___www_booking_com_.skp",  // bridgeOp() assert unsortable || ! empty
    "http___www_fj_p_com_.skp",  // markWinding assert from findChaseOp
    "http___www_leadpages_net_.skp",  // assert in opangle <
    "http___www_despegar_com_mx_.skp",  // bridgeOp() assert unsortable || ! empty
};

size_t skipOverCount = sizeof(skipOver) / sizeof(skipOver[0]);

static void PathOpsSkpClipTest(skiatest::Reporter* reporter) {
    SkOSFile::Iter iter(pictDir, "skp");
    SkString filename;
    int testCount = 0;
    while (iter.next(&filename)) {
        SkString pngName = make_png_name(filename);
        SkString oldPng = make_filepath(outOldClipDir, pngName);
        SkString newPng = make_filepath(outSkpClipDir, pngName);
        if (sk_exists(oldPng.c_str()) && sk_exists(newPng.c_str())) {
            reporter->bumpTestCount();
            continue;
        }
        for (size_t index = 0; index < skipOverCount; ++index) {
            if (skipOver[index] && strcmp(filename.c_str(), skipOver[index]) == 0) {
                reporter->bumpTestCount();
                goto skipOver;
            }
        }
        testOne(filename);
        if (reporter->verbose()) {
            SkDebugf(".");
            if (++testCount % 100 == 0) {
                SkDebugf("%d\n", testCount);
            }
        }
skipOver:
        reporter->bumpTestCount();
    }
}

static void bumpCount(skiatest::Reporter* reporter, bool skipping) {
    if (reporter->verbose()) {
        static int threadTestCount;
        if (!skipping) {
            SkDebugf(".");
        }
        sk_atomic_inc(&threadTestCount);
        if (!skipping && threadTestCount % 100 == 0) {
            SkDebugf("%d\n", threadTestCount);
        }
        if (skipping && threadTestCount % 10000 == 0) {
            SkDebugf("%d\n", threadTestCount);
        }
    }
}

static void testSkpClipMain(PathOpsThreadState* data) {
    SkString str(data->fSerialNo);
    testOne(str);
    bumpCount(data->fReporter, false);
    data->fReporter->bumpTestCount();
}

static void PathOpsSkpClipThreadedTest(skiatest::Reporter* reporter) {
    int threadCount = initializeTests(reporter, "skpClipThreadedTest");
    PathOpsThreadedTestRunner testRunner(reporter, threadCount);
    SkOSFile::Iter iter(pictDir, "skp");
    SkString filename;
    while (iter.next(&filename)) {
        SkString pngName = make_png_name(filename);
        SkString oldPng = make_filepath(outOldClipDir, pngName);
        SkString newPng = make_filepath(outSkpClipDir, pngName);
        if (sk_exists(oldPng.c_str()) && sk_exists(newPng.c_str())) {
            bumpCount(reporter, true);
            continue;
        }
        for (size_t index = 0; index < skipOverCount; ++index) {
            if (skipOver[index] && strcmp(filename.c_str(), skipOver[index]) == 0) {
                bumpCount(reporter, true);
                goto skipOver;
            }
        }
        *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                (&testSkpClipMain, filename.c_str(), &testRunner));
skipOver:
        ;
    }
    testRunner.render();
}

static void PathOpsSkpClipFixedTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < tryFixedCount; ) {
        SkString filename(tryFixed[index]);
        testOne(filename);
        ++index;
        if (reporter->verbose()) {
            SkDebugf(".");
            if (index % 100 == 0) {
                SkDebugf("\n");
            }
        }
        reporter->bumpTestCount();
    }
}

static void PathOpsSkpClipOneOffTest(skiatest::Reporter* reporter) {
    SkString filename("http___78_cn_.skp");
    testOne(filename);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsSkpClipTest)

DEFINE_TESTCLASS_SHORT(PathOpsSkpClipFixedTest)

DEFINE_TESTCLASS_SHORT(PathOpsSkpClipOneOffTest)

DEFINE_TESTCLASS_SHORT(PathOpsSkpClipThreadedTest)
