
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
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkThreadPool.h"
#include "SkTime.h"
#include "Test.h"

#ifdef SK_BUILD_FOR_WIN
    #define PATH_SLASH "\\"
    #define IN_DIR "D:\\9-30-13\\"
    #define OUT_DIR "D:\\opSkpClip\\1\\"
#else
    #define PATH_SLASH "/"
    #ifdef SK_BUILD_FOR_MAC
        #define IN_DIR "/Volumes/tera/9-30-13/skp"
        #define OUT_DIR "/Volumes/tera/out/9-30-13/1/"
    #else
        #define IN_DIR "/usr/local/google/home/caryclark/skps/9-30-13/skp"
        #define OUT_DIR "/mnt/skia/opSkpClip/1/"
    #endif
#endif

const struct {
    int directory;
    const char* filename;
} skipOverSept[] = {
    {9, "http___www_symptome_ch_.skp"}, // triangle clip with corner at x.999
    {11, "http___www_menly_fr_.skp"},
    {12, "http___www_banrasdr_com_.skp"},
};

size_t skipOverSeptCount = sizeof(skipOverSept) / sizeof(skipOverSept[0]);

enum TestStep {
    kCompareBits,
    kEncodeFiles,
};

enum {
    kMaxLength = 128,
    kMaxFiles = 128,
    kSmallLimit = 1000,
};

struct TestResult {
    void init(int dirNo) {
        fDirNo = dirNo;
        sk_bzero(fFilename, sizeof(fFilename));
        fTestStep = kCompareBits;
        fScaleOversized = true;
    }

    SkString status() {
        SkString outStr;
        outStr.printf("%s %d %d\n", fFilename, fPixelError, fTime);
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
    bool fScaleOversized;
};

struct TestState {
    void init(int dirNo, skiatest::Reporter* reporter) {
        fReporter = reporter;
        fResult.init(dirNo);
        fFoundCount = 0;
        TestState::fSmallCount = 0;
        fSmallestError = 0;
        sk_bzero(fFilesFound, sizeof(fFilesFound));
        sk_bzero(fDirsFound, sizeof(fDirsFound));
        sk_bzero(fError, sizeof(fError));
    }

    static bool bumpSmallCount() {
        sk_atomic_inc(&fSmallCount);
        return fSmallCount > kSmallLimit;
    }

    static void clearSmallCount() {
        if (fSmallCount < kSmallLimit) {
            fSmallCount = 0;
        }
    }

    char fFilesFound[kMaxFiles][kMaxLength];
    int fDirsFound[kMaxFiles];
    int fError[kMaxFiles];
    int fFoundCount;
    static int fSmallCount;
    int fSmallestError;
    skiatest::Reporter* fReporter;
    TestResult fResult;
};

int TestState::fSmallCount;

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
    TestRunnable(void (*testFun)(TestState*), int dirNo, TestRunner* runner) {
        fState.init(dirNo, runner->fReporter);
        fTestFun = testFun;
    }

    virtual void run() SK_OVERRIDE {
        SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);
        (*fTestFun)(&fState);
    }

    TestState fState;
    void (*fTestFun)(TestState*);
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
    bool foundSmaller = false;
    int dCount = data->fFoundCount;
    int pixelError = testResult.fPixelError;
    if (data->fFoundCount < kMaxFiles) {
        data->fError[dCount] = pixelError;
        strcpy(data->fFilesFound[dCount], testResult.fFilename);
        data->fDirsFound[dCount] = testResult.fDirNo;
        ++data->fFoundCount;
    } else if (pixelError > data->fSmallestError) {
        int smallest = SK_MaxS32;
        int smallestIndex = 0;
        for (int index = 0; index < kMaxFiles; ++index) {
            if (smallest > data->fError[index]) {
                smallest = data->fError[index];
                smallestIndex = index;
            }
        }
        data->fError[smallestIndex] = pixelError;
        strcpy(data->fFilesFound[smallestIndex], testResult.fFilename);
        data->fDirsFound[smallestIndex] = testResult.fDirNo;
        data->fSmallestError = SK_MaxS32;
        for (int index = 0; index < kMaxFiles; ++index) {
            if (data->fSmallestError > data->fError[index]) {
                data->fSmallestError = data->fError[index];
            }
        }
        SkDebugf("*%d*", data->fSmallestError);
        foundSmaller = true;
    }
    return foundSmaller;
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
        SkPicture* pic = SkPicture::CreateFromStream(&stream, &SkImageDecoder::DecodeMemory);
        if (!pic) {
            SkDebugf("unable to decode %s\n", fFilename);
            goto finish;
        }
        int width = pic->width();
        int height = pic->height();
        SkBitmap oldBitmap, opBitmap;
        int scale = 1;
        do {
            int dimX = (width + scale - 1) / scale;
            int dimY = (height + scale - 1) / scale;
            oldBitmap.setConfig(SkBitmap::kARGB_8888_Config, dimX, dimY);
            opBitmap.setConfig(SkBitmap::kARGB_8888_Config, dimX, dimY);
            bool success = oldBitmap.allocPixels() && opBitmap.allocPixels();
            if (success) {
                break;
            }
            SkDebugf("-%d-", scale);
        } while ((scale *= 2) < 256);
        if (scale >= 256) {
            SkDebugf("unable to allocate bitmap for %s (w=%d h=%d)\n", fFilename,
                    width, height);
            return;
        }
        oldBitmap.eraseColor(SK_ColorWHITE);
        SkCanvas oldCanvas(oldBitmap);
        oldCanvas.setAllowSimplifyClip(false);
        opBitmap.eraseColor(SK_ColorWHITE);
        SkCanvas opCanvas(opBitmap);
        opCanvas.setAllowSimplifyClip(true);
        drawPict(pic, &oldCanvas, fScaleOversized ? scale : 1);
        drawPict(pic, &opCanvas, fScaleOversized ? scale : 1);
        if (fTestStep == kCompareBits) {
            fPixelError = similarBits(oldBitmap, opBitmap);
            int oldTime = timePict(pic, &oldCanvas);
            int opTime = timePict(pic, &opCanvas);
            fTime = oldTime - opTime;
        } else if (fTestStep == kEncodeFiles) {
            SkString pngStr = make_png_name(fFilename);
            const char* pngName = pngStr.c_str();
            writePict(oldBitmap, outOldDir, pngName);
            writePict(opBitmap, outOpDir, pngName);
        }
    }
finish:
    SkDELETE(pic);
}

static SkString makeStatusString(int dirNo) {
    SkString statName;
    statName.printf("stats%d.txt", dirNo);
    SkString statusFile = make_filepath(0, outStatusDir, statName.c_str());
    return statusFile;
}

class PreParser {
public:
    PreParser(int dirNo)
        : fDirNo(dirNo)
        , fIndex(0) {
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
                SkASSERT(i == 0);
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
            SkAssertResult(reader.read(&c, 1));
            if (c == ' ') {
                break;
            }
            SkASSERT(c >= '0' && c <= '9');
            result->fPixelError = result->fPixelError * 10 + (c - '0');
        } while (true);
        bool minus = false;
        do {
            SkAssertResult(reader.read(&c, 1));
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
        if (fIndex < fResults.count()) {
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
};

static bool doOneDir(TestState* state) {
    int dirNo = state->fResult.fDirNo;
    skiatest::Reporter* reporter = state->fReporter;
    SkString dirName = make_in_dir_name(dirNo);
    SkASSERT(dirName.size());
    SkOSFile::Iter iter(dirName.c_str(), "skp");
    SkString filename;
    int testCount = 0;
    PreParser preParser(dirNo);
    SkFILEWStream statusStream(makeStatusString(dirNo).c_str());
    while (iter.next(&filename)) {
        for (size_t index = 0; index < skipOverSeptCount; ++index) {
            if (skipOverSept[index].directory == dirNo
                    && strcmp(filename.c_str(), skipOverSept[index].filename) == 0) {
                goto skipOver;
            }
        }
        if (preParser.match(filename, &statusStream, &state->fResult)) {
            addError(state, state->fResult);
            ++testCount;
            goto checkEarlyExit;
        }
        if (state->fSmallestError > 5000000) {
            return false;
        }
        {
            TestResult& result = state->fResult;
            result.test(dirNo, filename);
            SkString outStr(result.status());
            statusStream.write(outStr.c_str(), outStr.size());
            statusStream.flush();
            if (1) {
                SkDebugf("%s", outStr.c_str());
            }
            bool noMatch = addError(state, state->fResult);
            if (noMatch) {
                state->clearSmallCount();
            } else if (state->bumpSmallCount()) {
                return false;
            }
        }
        ++testCount;
        if (reporter->verbose()) {
            SkDebugf(".");
            if (++testCount % 100 == 0) {
                SkDebugf("%d\n", testCount);
            }
        }
skipOver:
        if (reporter->verbose()) {
            static int threadTestCount;
            SkDebugf(".");
            sk_atomic_inc(&threadTestCount);
            if (threadTestCount % 100 == 0) {
                SkDebugf("%d\n", threadTestCount);
            }
        }
checkEarlyExit:
        if (1 && testCount == 20) {
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

static void encodeFound(skiatest::Reporter* reporter, TestState& state) {
    if (reporter->verbose()) {
        for (int index = 0; index < state.fFoundCount; ++index) {
            SkDebugf("%d %s %d\n", state.fDirsFound[index], state.fFilesFound[index],
                     state.fError[index]);
        }
    }
    for (int index = 0; index < state.fFoundCount; ++index) {
        TestResult::Test(state.fDirsFound[index], state.fFilesFound[index], kEncodeFiles);
        if (state.fReporter->verbose()) SkDebugf("+");
    }
}

static void PathOpsSkpClipTest(skiatest::Reporter* reporter) {
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
        if (!doOneDir(&state)) {
            break;
        }
    }
    encodeFound(reporter, state);
}

static void testSkpClipMain(TestState* data) {
        (void) doOneDir(data);
}

static void PathOpsSkpClipThreadedTest(skiatest::Reporter* reporter) {
    if (!initTest()) {
        return;
    }
    int threadCount = reporter->allowThreaded() ? SkThreadPool::kThreadPerCore : 1;
    TestRunner testRunner(reporter, threadCount);
    for (int dirNo = 1; dirNo <= 100; ++dirNo) {
        *testRunner.fRunnables.append() = SkNEW_ARGS(TestRunnable,
                (&testSkpClipMain, dirNo, &testRunner));
    }
    testRunner.render();
    TestState state;
    state.init(0, reporter);
    for (int dirNo = 1; dirNo <= 100; ++dirNo) {
        TestState& testState = testRunner.fRunnables[dirNo - 1]->fState;
        for (int inner = 0; inner < testState.fFoundCount; ++inner) {
            TestResult& testResult = testState.fResult;
            SkASSERT(testResult.fDirNo == dirNo);
            testResult.fPixelError = testState.fError[inner];
            strcpy(testResult.fFilename, testState.fFilesFound[inner]);
            addError(&state, testResult);
        }
    }
    encodeFound(reporter, state);
}

static void PathOpsSkpClipOneOffTest(skiatest::Reporter* reporter) {
    if (!initTest()) {
        return;
    }
    const int testIndex = 43 - 41;
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

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsSkpClipTest)

DEFINE_TESTCLASS_SHORT(PathOpsSkpClipOneOffTest)

DEFINE_TESTCLASS_SHORT(PathOpsSkpClipThreadedTest)
