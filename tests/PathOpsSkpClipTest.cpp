#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkBitmap.h"
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
#define IN_DIR "/Volumes/Untitled" PATH_SLASH
#define OUT_DIR PATH_SLASH
#endif

static const char pictDir[] = IN_DIR ;
static const char outSkpClipDir[] = OUT_DIR "skpClip";
static const char outOldClipDir[] = OUT_DIR "oldClip";

static void make_filepath(SkString* path, const char* dir, const SkString& name) {
    size_t len = strlen(dir);
    path->set(dir);
    if (len > 0 && dir[len - 1] != PATH_SLASH[0]) {
        path->append(PATH_SLASH);
    }
    path->append(name);
}

static void testOne(const SkString& filename) {
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
    SkString path;
    make_filepath(&path, pictDir, filename);
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
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bool success = bitmap.allocPixels();
    if (!success) {
        SkDebugf("unable to allocate bitmap for %s\n", filename.c_str());
        return;
    }
    SkCanvas canvas(bitmap);
    SkString pngName(filename);
    pngName.remove(pngName.size() - 3, 3);
    pngName.append("png");
    for (int i = 0; i < 2; ++i) {
        bool useOp = i ? true : false;
        canvas.setAllowSimplifyClip(useOp);
        pic->draw(&canvas);
        SkString outFile;
        make_filepath(&outFile, useOp ? outSkpClipDir : outOldClipDir, pngName);
        SkImageEncoder::EncodeFile(outFile.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
    }
    SkDELETE(pic);
}

const char skipBefore[] = "http___kkiste_to.skp";

static void PathOpsSkpClipTest(skiatest::Reporter* reporter) {
    SkOSFile::Iter iter(pictDir, "skp");
    SkString filename;
    int testCount = 0;
    while (iter.next(&filename)) {
        if (strcmp(filename.c_str(), skipBefore) < 0) {
            continue;
        }
        testOne(filename);
        if (reporter->verbose()) {
            SkDebugf(".");
            if (++testCount % 100 == 0) {
                SkDebugf("\n");
            }
        }
        reporter->bumpTestCount();
    }
}

static void testSkpClipMain(PathOpsThreadState* data) {
        SkString str(data->fSerialNo);
        testOne(str);
        if (data->fReporter->verbose()) {
            SkDebugf(".");
            static int threadTestCount;
            sk_atomic_inc(&threadTestCount);
            if (threadTestCount % 100 == 0) {
                SkDebugf("\n");
            }
        }
}

static void PathOpsSkpClipThreadedTest(skiatest::Reporter* reporter) {
    int threadCount = initializeTests(reporter, "skpClipThreadedTest");
    PathOpsThreadedTestRunner testRunner(reporter, threadCount);
    SkOSFile::Iter iter(pictDir, "skp");
    SkString filename;
    while (iter.next(&filename)) {
        if (strcmp(filename.c_str(), skipBefore) < 0) {
            continue;
        }
        *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                (&testSkpClipMain, filename.c_str(), &testRunner));
        reporter->bumpTestCount();
    }
    testRunner.render();
}

static void PathOpsSkpClipOneOffTest(skiatest::Reporter* reporter) {
    SkString filename(skipBefore);
    testOne(filename);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsSkpClipTest)

DEFINE_TESTCLASS_SHORT(PathOpsSkpClipOneOffTest)

DEFINE_TESTCLASS_SHORT(PathOpsSkpClipThreadedTest)
