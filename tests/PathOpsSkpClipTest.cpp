/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CrashHandler.h"
// #include "OverwriteLine.h"
#include "Resources.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkDevice.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPathOpsDebug.h"
#include "SkPicture.h"
#include "SkRTConf.h"
#include "SkRunnable.h"
#include "SkTSort.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTaskGroup.h"
#include "SkTemplates.h"
#include "SkTime.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

/* add local exceptions here */
/* TODO : add command flag interface */
const struct SkipOverTest {
    int directory;
    const char* filename;
    bool blamePathOps;
} skipOver[] = {
    { 2, "http___www_groupon_sg_.skp", false},  // SkAAClip::Builder::addRun SkASSERT(fBounds.contains(x, y));
    { 6, "http___www_googleventures_com_.skp", true},  // addTCoincident SkASSERT(test->fT < 1);
    { 7, "http___www_foxsports_nl_.skp", true},  // (no repro on mac) addT SkASSERT(this != other || fVerb == SkPath::kCubic_Verb)
    {13, "http___www_modernqigong_com_.skp", false},  // SkAAClip::Builder::addRun SkASSERT(fBounds.contains(x, y));
    {14, "http___www_devbridge_com_.skp", true},  // checkSmallCoincidence SkASSERT(!next->fSmall || checkMultiple);
    {16, "http___www_1023world_net_.skp", false},  // bitmap decode assert (corrupt skp?)
    {19, "http___www_alamdi_com_.skp", true},  // cubic/quad intersection
    {26, "http___www_liveencounters_net_.skp", true},  // (no repro on mac) checkSmall addT:549 (line, expects cubic)
    {28, "http___www_encros_fr_.skp", false},  // SkAAClip::Builder::addRun SkASSERT(fBounds.contains(x, y));
    {37, "http___www_familysurvivalprotocol_wordpress_com_.skp", true},  // bumpSpan SkASSERT(span->fOppValue >= 0);
    {39, "http___sufeinet_com_.skp", false}, // bitmap decode assert (corrupt skp?)
    {41, "http___www_rano360_com_.skp", true}, // checkSmallCoincidence SkASSERT(!next->fSmall || checkMultiple);
    {44, "http___www_firstunitedbank_com_.skp", true},  // addTCancel SkASSERT(oIndex > 0);
    {46, "http___www_shinydemos_com_.skp", true},  // addSimpleAngle SkASSERT(index == count() - 2);
    {48, "http___www_familysurvivalprotocol_com_.skp", true},  // bumpSpan SkASSERT "span->fOppValue >= 0"
    {57, "http___www_lptemp_com_.skp", true}, // addTCoincident oPeek = &other->fTs[++oPeekIndex];
    {71, "http___www_1milyonkahraman_org_.skp", true},  // addTCoincident SkASSERT(test->fT < 1);
    {88, "http___www_apuntesdelechuza_wordpress_com_.skp", true},  // bumpSpan SkASSERT "span->fOppValue >= 0"
    {89, "http___www_mobilizedconsulting_com_.skp", true}, // addTCancel SkASSERT(oIndex > 0);
    {93, "http___www_simple_living_in_suffolk_co_uk_.skp", true},  // bumpSpan SkASSERT "span->fOppValue >= 0"
};

size_t skipOverCount = sizeof(skipOver) / sizeof(skipOver[0]);


/* customize file in/out here */
/* TODO : add command flag interface */
#define CHROME_VERSION "1e5dfa4-4a995df"
#define SUMMARY_RUN 1

#ifdef SK_BUILD_FOR_WIN
    #define DRIVE_SPEC "D:"
    #define PATH_SLASH "\\"
#else
    #define DRIVE_SPEC ""
    #define PATH_SLASH "/"
#endif

#define IN_DIR_PRE  DRIVE_SPEC PATH_SLASH "skps"   PATH_SLASH "slave"
#define OUT_DIR_PRE DRIVE_SPEC PATH_SLASH "skpOut" PATH_SLASH "slave"
#define OUT_DIR_SUM DRIVE_SPEC PATH_SLASH "skpOut" PATH_SLASH "summary"
#define DIR_POST               PATH_SLASH "All"    PATH_SLASH CHROME_VERSION

static const char outOpDir[]     = "opClip";
static const char outOldDir[]    = "oldClip";
static const char outStatusDir[] = "statusTest";

static SkString get_in_path(int dirNo, const char* filename) {
    SkString path;
    SkASSERT(dirNo);
    path.appendf("%s%d%s", IN_DIR_PRE, dirNo, DIR_POST);
    if (!sk_exists(path.c_str())) {
        SkDebugf("could not read %s\n", path.c_str());
        return SkString();
    }
    if (filename) {
        path.appendf("%s%s", PATH_SLASH, filename);
        if (!sk_exists(path.c_str())) {
            SkDebugf("could not read %s\n", path.c_str());
            return SkString();
        }
    }
    return path;
}

static void make_recursive_dir(const SkString& path) {
    if (sk_exists(path.c_str())) {
        return;
    }
    const char* pathStr = path.c_str();
    int last = (int) path.size();
    do {
        while (last > 0 && pathStr[--last] != PATH_SLASH[0])
            ;
        SkASSERT(last > 0);
        SkString shorter(pathStr, last);
        if (sk_mkdir(shorter.c_str())) {
            break;
        }
    } while (true);
    do {
        while (last < (int) path.size() && pathStr[++last] != PATH_SLASH[0])
            ;
        SkString shorter(pathStr, last);
        SkAssertResult(sk_mkdir(shorter.c_str()));
    } while (last < (int) path.size());
}

static SkString get_out_path(int dirNo, const char* dirName) {
    SkString path;
    SkASSERT(dirNo);
    SkASSERT(dirName);
    path.appendf("%s%d%s%s%s", OUT_DIR_PRE, dirNo, DIR_POST, PATH_SLASH, dirName);
    make_recursive_dir(path);
    return path;
}

static SkString get_sum_path(const char* dirName) {
    SkString path;
    SkASSERT(dirName);
    path.appendf("%s%d%s%s", OUT_DIR_SUM, SUMMARY_RUN, PATH_SLASH, dirName);
    SkDebugf("%s\n", path.c_str());
    make_recursive_dir(path);
    return path;
}

static SkString make_png_name(const char* filename) {
    SkString pngName = SkString(filename);
    pngName.remove(pngName.size() - 3, 3);
    pngName.append("png");
    return pngName;
}

////////////////////////////////////////////////////////

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

    void init(int dirNo, const SkString& filename) {
        fDirNo = dirNo;
        strcpy(fFilename, filename.c_str());
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
    void init(int dirNo) {
        fResult.init(dirNo);
    }

    SkTDArray<SortByPixel> fPixelWorst;
    SkTDArray<SortByTime> fSlowest;
    TestResult fResult;
};

struct TestRunner {
    ~TestRunner();
    void render();
    SkTDArray<class TestRunnable*> fRunnables;
};

class TestRunnable : public SkRunnable {
public:
    void run() override {
        SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);
        (*fTestFun)(&fState);
    }

    TestState fState;
    void (*fTestFun)(TestState*);
};


class TestRunnableDir : public TestRunnable {
public:
    TestRunnableDir(void (*testFun)(TestState*), int dirNo, TestRunner* runner) {
        fState.init(dirNo);
        fTestFun = testFun;
    }

};

class TestRunnableFile : public TestRunnable {
public:
    TestRunnableFile(void (*testFun)(TestState*), int dirNo, const char* name, TestRunner* runner) {
        fState.init(dirNo);
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
    // TODO: this doesn't really need to use SkRunnables any more.
    // We can just write the code to run in the for-loop directly.
    sk_parallel_for(fRunnables.count(), [&](int i) {
        fRunnables[i]->run();
    });
}

////////////////////////////////////////////////


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
    SkScalar pWidth = pic->cullRect().width();
    SkScalar pHeight = pic->cullRect().height();
    const SkScalar maxDimension = 1000.0f;
    const int slices = 3;
    SkScalar xInterval = SkTMax(pWidth - maxDimension, 0.0f) / (slices - 1);
    SkScalar yInterval = SkTMax(pHeight - maxDimension, 0.0f) / (slices - 1);
    SkRect rect = {0, 0, SkTMin(maxDimension, pWidth), SkTMin(maxDimension, pHeight) };
    canvas->clipRect(rect);
    SkMSec start = SkTime::GetMSecs();
    for (int x = 0; x < slices; ++x) {
        for (int y = 0; y < slices; ++y) {
            pic->playback(canvas);
            canvas->translate(0, yInterval);
        }
        canvas->translate(xInterval, -yInterval * slices);
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
    pic->playback(canvas);
    if (scale != 1) {
        canvas->restore();
    }
}

static void writePict(const SkBitmap& bitmap, const char* outDir, const char* pngName) {
    SkString outFile = get_sum_path(outDir);
    outFile.appendf("%s%s", PATH_SLASH, pngName);
    if (!SkImageEncoder::EncodeFile(outFile.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100)) {
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
        SkString path = get_in_path(fDirNo, fFilename);
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
        SkScalar width = pic->cullRect().width();
        SkScalar height = pic->cullRect().height();
        SkBitmap oldBitmap, opBitmap;
        fScale = 1;
        while (width / fScale > 32767 || height / fScale > 32767) {
            ++fScale;
        }
        do {
            int dimX = SkScalarCeilToInt(width / fScale);
            int dimY = SkScalarCeilToInt(height / fScale);
            if (oldBitmap.tryAllocN32Pixels(dimX, dimY) && opBitmap.tryAllocN32Pixels(dimX, dimY)) {
                break;
            }
            SkDebugf("-%d-", fScale);
        } while (++fScale < 256);
        if (fScale >= 256) {
            SkDebugf("unable to allocate bitmap for %s (w=%f h=%f)\n", fFilename,
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

DEFINE_string2(match, m, "PathOpsSkpClipThreaded",
        "[~][^]substring[$] [...] of test name to run.\n"
        "Multiple matches may be separated by spaces.\n"
        "~ causes a matching test to always be skipped\n"
        "^ requires the start of the test to match\n"
        "$ requires the end of the test to match\n"
        "^ and $ requires an exact match\n"
        "If a test does not match any list entry,\n"
        "it is skipped unless some list entry starts with ~");
DEFINE_string2(dir, d, NULL, "range of directories (e.g., 1-100)");
DEFINE_string2(skp, s, NULL, "skp to test");
DEFINE_bool2(single, z, false, "run tests on a single thread internally.");
DEFINE_int32(testIndex, 0, "override local test index (PathOpsSkpClipOneOff only).");
DEFINE_bool2(verbose, v, false, "enable verbose output.");

static bool verbose() {
    return FLAGS_verbose;
}

class Dirs {
public:
    Dirs() {
        reset();
        sk_bzero(fRun, sizeof(fRun));
        fSet = false;
    }

    int first() const {
        int index = 0;
        while (++index < kMaxDir) {
            if (fRun[index]) {
                return index;
            }
        }
        SkASSERT(0);
        return -1;
    }

    int last() const {
        int index = kMaxDir;
        while (--index > 0 && !fRun[index])
            ;
        return index;
    }

    int next() {
        while (++fIndex < kMaxDir) {
            if (fRun[fIndex]) {
                return fIndex;
            }
        }
        return -1;
    }

    void reset() {
        fIndex = -1;
    }

    void set(int start, int end) {
        while (start < end) {
            fRun[start++] = 1;
        }
        fSet = true;
    }

    void setDefault() {
        if (!fSet) {
            set(1, 100);
        }
    }

private:
    enum {
         kMaxDir = 101
    };
    char fRun[kMaxDir];
    int fIndex;
    bool fSet;
} gDirs;

class Filenames {
public:
    Filenames()
        : fIndex(-1) {
    }

    const char* next() {
        while (fNames && ++fIndex < fNames->count()) {
            return (*fNames)[fIndex];
        }
        return NULL;
    }

    void set(const SkCommandLineFlags::StringArray& names) {
        fNames = &names;
    }

private:
    int fIndex;
    const SkCommandLineFlags::StringArray* fNames;
} gNames;

static bool buildTestDir(int dirNo, int firstDirNo,
        SkTDArray<TestResult>* tests, SkTDArray<SortByName*>* sorted) {
    SkString dirName = get_out_path(dirNo, outStatusDir);
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
        return true;
    }
    SkTDArray<TestResult>& testSet = tests[dirNo - firstDirNo];
    int count = testSet.count();
    for (int index = 0; index < count; ++index) {
        *sorted[dirNo - firstDirNo].append() = (SortByName*) &testSet[index];
    }
    if (sorted[dirNo - firstDirNo].count()) {
        SkTQSort<SortByName>(sorted[dirNo - firstDirNo].begin(),
                sorted[dirNo - firstDirNo].end() - 1);
        if (verbose()) {
            SkDebugf("+");
        }
    }
    return true;
}

static void testSkpClip(TestState* data) {
    data->fResult.testOne();
    SkString statName(data->fResult.fFilename);
    SkASSERT(statName.endsWith(".skp"));
    statName.remove(statName.size() - 4, 4);
    statName.appendf(".%d.%d.skp", data->fResult.fPixelError, data->fResult.fTime);
    SkString statusFile = get_out_path(data->fResult.fDirNo, outStatusDir);
    if (!statusFile.size()) {
        SkDebugf("failed to create %s", statusFile.c_str());
        return;
    }
    statusFile.appendf("%s%s", PATH_SLASH, statName.c_str());
    SkFILE* file = sk_fopen(statusFile.c_str(), kWrite_SkFILE_Flag);
    if (!file) {
            SkDebugf("failed to create %s", statusFile.c_str());
            return;
    }
    sk_fclose(file);
    if (verbose()) {
        if (data->fResult.fPixelError || data->fResult.fTime) {
            SkDebugf("%s", data->fResult.progress().c_str());
        } else {
            SkDebugf(".");
        }
    }
}

bool Less(const SortByName& a, const SortByName& b);
bool Less(const SortByName& a, const SortByName& b) {
    return a < b;
}

static bool doOneDir(TestState* state, bool threaded) {
    int dirNo = state->fResult.fDirNo;
    SkString dirName = get_in_path(dirNo, NULL);
    if (!dirName.size()) {
        return false;
    }
    SkTDArray<TestResult> tests[1];
    SkTDArray<SortByName*> sorted[1];
    if (!buildTestDir(dirNo, dirNo, tests, sorted)) {
        return false;
    }
    SkOSFile::Iter iter(dirName.c_str(), "skp");
    SkString filename;
    while (iter.next(&filename)) {
        for (size_t index = 0; index < skipOverCount; ++index) {
            if (skipOver[index].directory == dirNo
                    && strcmp(filename.c_str(), skipOver[index].filename) == 0) {
                goto checkEarlyExit;
            }
        }
        {
            SortByName name;
            name.init(dirNo);
            strncpy(name.fFilename, filename.c_str(), filename.size() - 4);  // drop .skp
            int count = sorted[0].count();
            int idx = SkTSearch<SortByName, Less>(sorted[0].begin(), count, &name, sizeof(&name));
            if (idx >= 0) {
                SortByName* found = sorted[0][idx];
                (void) addError(state, *found);
                continue;
            }
            TestResult test;
            test.init(dirNo, filename);
            state->fResult = test;
            testSkpClip(state);
#if 0 // artificially limit to a few while debugging code
            static int debugLimit = 0;
            if (++debugLimit == 5) {
                return true;
            }
#endif
        }
checkEarlyExit:
        ;
    }
    return true;
}

static void initTest() {
#if !defined SK_BUILD_FOR_WIN && !defined SK_BUILD_FOR_MAC
    SK_CONF_SET("images.jpeg.suppressDecoderWarnings", true);
    SK_CONF_SET("images.png.suppressDecoderWarnings", true);
#endif
}

static void testSkpClipEncode(TestState* data) {
    data->fResult.testOne();
    if (verbose()) {
        SkDebugf("+");
    }
}

static void encodeFound(TestState& state) {
    if (verbose()) {
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
    TestRunner testRunner;
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
}

class Test {
public:
    Test() {}
    virtual ~Test() {}

    const char* getName() { onGetName(&fName); return fName.c_str(); }
    void run() { onRun(); }

protected:
    virtual void onGetName(SkString*) = 0;
    virtual void onRun() = 0;

private:
    SkString    fName;
};

typedef SkTRegistry<Test*(*)(void*)> TestRegistry;

#define DEF_TEST(name)                                        \
    static void test_##name();                       \
    class name##Class : public Test {                                   \
    public:                                                             \
        static Test* Factory(void*) { return SkNEW(name##Class); }      \
    protected:                                                          \
        void onGetName(SkString* name) override {            \
            name->set(#name);                                           \
        }                                                               \
        void onRun() override { test_##name(); } \
    };                                                                  \
    static TestRegistry gReg_##name##Class(name##Class::Factory);       \
    static void test_##name()

DEF_TEST(PathOpsSkpClip) {
    gDirs.setDefault();
    initTest();
    SkTArray<TestResult, true> errors;
    TestState state;
    state.init(0);
    int dirNo;
    gDirs.reset();
    while ((dirNo = gDirs.next()) > 0) {
        if (verbose()) {
            SkDebugf("dirNo=%d\n", dirNo);
        }
        state.fResult.fDirNo = dirNo;
        if (!doOneDir(&state, false)) {
            break;
        }
    }
    encodeFound(state);
}

static void testSkpClipMain(TestState* data) {
        (void) doOneDir(data, true);
}

DEF_TEST(PathOpsSkpClipThreaded) {
    gDirs.setDefault();
    initTest();
    TestRunner testRunner;
    int dirNo;
    gDirs.reset();
    while ((dirNo = gDirs.next()) > 0) {
        *testRunner.fRunnables.append() = SkNEW_ARGS(TestRunnableDir,
                (&testSkpClipMain, dirNo, &testRunner));
    }
    testRunner.render();
    TestState state;
    state.init(0);
    gDirs.reset();
    while ((dirNo = gDirs.next()) > 0) {
        TestState& testState = testRunner.fRunnables[dirNo - 1]->fState;
        SkASSERT(testState.fResult.fDirNo == dirNo);
        for (int inner = 0; inner < testState.fPixelWorst.count(); ++inner) {
            addError(&state, testState.fPixelWorst[inner]);
        }
        for (int inner = 0; inner < testState.fSlowest.count(); ++inner) {
            addError(&state, testState.fSlowest[inner]);
        }
    }
    encodeFound(state);
}

static bool buildTests(SkTDArray<TestResult>* tests, SkTDArray<SortByName*>* sorted) {
    int firstDirNo = gDirs.first();
    int dirNo;
    while ((dirNo = gDirs.next()) > 0) {
        if (!buildTestDir(dirNo, firstDirNo, tests, sorted)) {
            return false;
        }
    }
    return true;
}

DEF_TEST(PathOpsSkpClipUberThreaded) {
    gDirs.setDefault();
    const int firstDirNo = gDirs.next();
    const int lastDirNo = gDirs.last();
    initTest();
    int dirCount = lastDirNo - firstDirNo + 1;
    SkAutoTDeleteArray<SkTDArray<TestResult> > tests(new SkTDArray<TestResult>[dirCount]);
    SkAutoTDeleteArray<SkTDArray<SortByName*> > sorted(new SkTDArray<SortByName*>[dirCount]);
    if (!buildTests(tests.get(), sorted.get())) {
        return;
    }
    TestRunner testRunner;
    int dirNo;
    gDirs.reset();
    while ((dirNo = gDirs.next()) > 0) {
        SkString dirName = get_in_path(dirNo, NULL);
        if (!dirName.size()) {
            continue;
        }
        SkOSFile::Iter iter(dirName.c_str(), "skp");
        SkString filename;
        while (iter.next(&filename)) {
            for (size_t index = 0; index < skipOverCount; ++index) {
                if (skipOver[index].directory == dirNo
                        && strcmp(filename.c_str(), skipOver[index].filename) == 0) {
                    goto checkEarlyExit;
                }
            }
            {
                SortByName name;
                name.init(dirNo);
                strncpy(name.fFilename, filename.c_str(), filename.size() - 4);  // drop .skp
                int count = sorted.get()[dirNo - firstDirNo].count();
                if (SkTSearch<SortByName, Less>(sorted.get()[dirNo - firstDirNo].begin(),
                        count, &name, sizeof(&name)) < 0) {
                    *testRunner.fRunnables.append() = SkNEW_ARGS(TestRunnableFile,
                            (&testSkpClip, dirNo, filename.c_str(), &testRunner));
                }
            }
    checkEarlyExit:
            ;
        }

    }
    testRunner.render();
    SkAutoTDeleteArray<SkTDArray<TestResult> > results(new SkTDArray<TestResult>[dirCount]);
    if (!buildTests(results.get(), NULL)) {
        return;
    }
    SkTDArray<TestResult> allResults;
    for (int dirNo = firstDirNo; dirNo <= lastDirNo; ++dirNo) {
        SkTDArray<TestResult>& array = results.get()[dirNo - firstDirNo];
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
    encodeFound(state);
}

DEF_TEST(PathOpsSkpClipOneOff) {
    const int testIndex = FLAGS_testIndex;
    int dirNo = gDirs.next();
    if (dirNo < 0) {
        dirNo = skipOver[testIndex].directory;
    }
    const char* skp = gNames.next();
    if (!skp) {
        skp = skipOver[testIndex].filename;
    }
    initTest();
    SkAssertResult(get_in_path(dirNo, skp).size());
    SkString filename(skp);
    TestResult state;
    state.test(dirNo, filename);
    if (verbose()) {
        SkDebugf("%s", state.status().c_str());
    }
    state.fTestStep = kEncodeFiles;
    state.testOne();
}

DEF_TEST(PathOpsTestSkipped) {
    for (size_t index = 0; index < skipOverCount; ++index) {
        const SkipOverTest& skip = skipOver[index];
        if (!skip.blamePathOps) {
            continue;
        }
        int dirNo = skip.directory;
        const char* skp = skip.filename;
        initTest();
        SkAssertResult(get_in_path(dirNo, skp).size());
        SkString filename(skp);
        TestResult state;
        state.test(dirNo, filename);
        if (verbose()) {
            SkDebugf("%s", state.status().c_str());
        }
        state.fTestStep = kEncodeFiles;
        state.testOne();
    }
}

DEF_TEST(PathOpsCopyFails) {
    FLAGS_verbose = true;
    for (size_t index = 0; index < skipOverCount; ++index) {
        int dirNo = skipOver[index].directory;
        SkDebugf("mkdir -p " IN_DIR_PRE "%d" DIR_POST "\n", dirNo);
    }
    for (size_t index = 0; index < skipOverCount; ++index) {
        int dirNo = skipOver[index].directory;
        const char* filename = skipOver[index].filename;
        SkDebugf("rsync -av cary-linux.cnc:/tera" PATH_SLASH "skps" PATH_SLASH "slave"
            "%d" DIR_POST "/%s " IN_DIR_PRE "%d" DIR_POST "\n", dirNo, filename, dirNo);
    }
}

template TestRegistry* TestRegistry::gHead;

class Iter {
public:
    Iter() { this->reset(); }
    void reset() { fReg = TestRegistry::Head(); }

    Test* next() {
        if (fReg) {
            TestRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            Test* test = fact(NULL);
            return test;
        }
        return NULL;
    }

private:
    const TestRegistry* fReg;
};

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SetupCrashHandler();
    SkCommandLineFlags::SetUsage("");
    SkCommandLineFlags::Parse(argc, argv);
    SkGraphics::Init();
    SkString header("PathOps SkpClip:");
    if (!FLAGS_match.isEmpty()) {
        header.appendf(" --match");
        for (int index = 0; index < FLAGS_match.count(); ++index) {
            header.appendf(" %s", FLAGS_match[index]);
        }
    }
    if (!FLAGS_dir.isEmpty()) {
        int count = FLAGS_dir.count();
        for (int i = 0; i < count; ++i) {
            const char* range = FLAGS_dir[i];
            const char* dash = strchr(range, '-');
            if (!dash) {
                dash = strchr(range, ',');
            }
            int first = atoi(range);
            int last = dash ? atoi(dash + 1) : first;
            if (!first || !last) {
                SkDebugf("couldn't parse --dir %s\n", range);
                return 1;
            }
            gDirs.set(first, last);
        }
    }
    if (!FLAGS_skp.isEmpty()) {
        gNames.set(FLAGS_skp);
    }
#ifdef SK_DEBUG
    header.append(" SK_DEBUG");
#else
    header.append(" SK_RELEASE");
#endif
    if (FLAGS_verbose) {
        header.appendf("\n");
    }
    SkDebugf("%s", header.c_str());
    Iter iter;
    Test* test;
    while ((test = iter.next()) != NULL) {
        SkAutoTDelete<Test> owned(test);
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, test->getName())) {
            test->run();
        }
    }
    SkGraphics::Term();
    return 0;
}

#if !defined(SK_BUILD_FOR_IOS)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
