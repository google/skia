#if !SK_SUPPORT_GPU
#error "GPU support required"
#endif

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrRenderTarget.h"
#include "SkGpuDevice.h"
#include "gl/GrGLDefines.h"

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkRTConf.h"
#include "SkRunnable.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkThreadPool.h"
#include "SkTime.h"
#include "Test.h"

#ifdef SK_BUILD_FOR_WIN
    #define PATH_SLASH "\\"
    #define IN_DIR "D:\\9-30-13\\"
    #define OUT_DIR "D:\\skpSkGr\\11\\"
    #define LINE_FEED "\r\n"
#else
    #define PATH_SLASH "/"
    #define IN_DIR "/usr/local/google/home/caryclark" PATH_SLASH "9-30-13-skp"
    #define OUT_DIR "/media/01CD75512A7F9EE0/4" PATH_SLASH
    #define LINE_FEED \n"
#endif

#define PATH_STR_SIZE 512

static const struct {
    int directory;
    const char* filename;
} skipOverSkGr[] = {
    {1, "http___accuweather_com_.skp"},  // Couldn't convert bitmap to texture.http___absoku072_com_
};

static const size_t skipOverSkGrCount = 0; // SK_ARRAY_COUNT(skipOverSkGr);

/////////////////////////////////////////

class SkpSkGrThreadedRunnable;

enum TestStep {
    kCompareBits,
    kEncodeFiles,
};

enum {
    kMaxLength = 128,
    kMaxFiles = 128,
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
        outStr.printf("%s %d %d%s", fFilename, fPixelError, fTime, LINE_FEED);
        return outStr;
    }

    static void Test(int dirNo, const char* filename, TestStep testStep, bool verbose) {
        TestResult test;
        test.init(dirNo);
        test.fTestStep = testStep;
        strcpy(test.fFilename, filename);
        test.testOne();
        if (verbose) {
            SkDebugf("%s", test.status().c_str());
        }
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

struct SkpSkGrThreadState {
    void init(int dirNo) {
        fResult.init(dirNo);
        fFoundCount = 0;
        fSmallestError = 0;
        sk_bzero(fFilesFound, sizeof(fFilesFound));
        sk_bzero(fDirsFound, sizeof(fDirsFound));
        sk_bzero(fError, sizeof(fError));
    }

    char fFilesFound[kMaxFiles][kMaxLength];
    int fDirsFound[kMaxFiles];
    int fError[kMaxFiles];
    int fFoundCount;
    int fSmallestError;
    skiatest::Reporter* fReporter;
    TestResult fResult;
};

struct SkpSkGrThreadedTestRunner {
    SkpSkGrThreadedTestRunner(skiatest::Reporter* reporter, int threadCount)
        : fNumThreads(threadCount)
        , fReporter(reporter) {
    }

    ~SkpSkGrThreadedTestRunner();
    void render();
    int fNumThreads;
    SkTDArray<SkpSkGrThreadedRunnable*> fRunnables;
    skiatest::Reporter* fReporter;
};

class SkpSkGrThreadedRunnable : public SkRunnable {
public:
    SkpSkGrThreadedRunnable(void (*testFun)(SkpSkGrThreadState*), int dirNo, const char* str,
            SkpSkGrThreadedTestRunner* runner) {
        SkASSERT(strlen(str) < sizeof(fState.fResult.fFilename) - 1);
        fState.init(dirNo);
        strcpy(fState.fResult.fFilename, str);
        fState.fReporter = runner->fReporter;
        fTestFun = testFun;
    }

    virtual void run() SK_OVERRIDE {
        SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);
        (*fTestFun)(&fState);
    }

    SkpSkGrThreadState fState;
    void (*fTestFun)(SkpSkGrThreadState*);
};

SkpSkGrThreadedTestRunner::~SkpSkGrThreadedTestRunner() {
    for (int index = 0; index < fRunnables.count(); index++) {
        SkDELETE(fRunnables[index]);
    }
}

void SkpSkGrThreadedTestRunner::render() {
    SkThreadPool pool(fNumThreads);
    for (int index = 0; index < fRunnables.count(); ++ index) {
        pool.add(fRunnables[index]);
    }
}

////////////////////////////////////////////////

static const char outGrDir[] = OUT_DIR "grTest";
static const char outSkDir[] = OUT_DIR "skTest";
static const char outSkpDir[] = OUT_DIR "skpTest";
static const char outDiffDir[] = OUT_DIR "outTest";
static const char outStatusDir[] = OUT_DIR "statusTest";

static SkString make_filepath(int dirIndex, const char* dir, const char* name) {
    SkString path(dir);
    if (dirIndex) {
        path.appendf("%d", dirIndex);
    }
    path.append(PATH_SLASH);
    path.append(name);
    return path;
}

static SkString make_in_dir_name(int dirIndex) {
    SkString dirName(IN_DIR);
    dirName.appendf("%d", dirIndex);
    if (!sk_exists(dirName.c_str())) {
        SkDebugf("could not read dir %s\n", dirName.c_str());
        return SkString();
    }
    return dirName;
}

static bool make_out_dirs() {
    SkString outDir = make_filepath(0, OUT_DIR, "");
    if (!sk_exists(outDir.c_str())) {
        if (!sk_mkdir(outDir.c_str())) {
            SkDebugf("could not create dir %s\n", outDir.c_str());
            return false;
        }
    }
    SkString grDir = make_filepath(0, outGrDir, "");
    if (!sk_exists(grDir.c_str())) {
        if (!sk_mkdir(grDir.c_str())) {
            SkDebugf("could not create dir %s\n", grDir.c_str());
            return false;
        }
    }
    SkString skDir = make_filepath(0, outSkDir, "");
    if (!sk_exists(skDir.c_str())) {
        if (!sk_mkdir(skDir.c_str())) {
            SkDebugf("could not create dir %s\n", skDir.c_str());
            return false;
        }
    }
    SkString skpDir = make_filepath(0, outSkpDir, "");
    if (!sk_exists(skpDir.c_str())) {
        if (!sk_mkdir(skpDir.c_str())) {
            SkDebugf("could not create dir %s\n", skpDir.c_str());
            return false;
        }
    }
    SkString diffDir = make_filepath(0, outDiffDir, "");
    if (!sk_exists(diffDir.c_str())) {
        if (!sk_mkdir(diffDir.c_str())) {
            SkDebugf("could not create dir %s\n", diffDir.c_str());
            return false;
        }
    }
    SkString statusDir = make_filepath(0, outStatusDir, "");
    if (!sk_exists(statusDir.c_str())) {
        if (!sk_mkdir(statusDir.c_str())) {
            SkDebugf("could not create dir %s\n", statusDir.c_str());
            return false;
        }
    }
    return true;
}

static SkString make_png_name(const char* filename) {
    SkString pngName = SkString(filename);
    pngName.remove(pngName.size() - 3, 3);
    pngName.append("png");
    return pngName;
}

typedef GrContextFactory::GLContextType GLContextType;
#ifdef SK_BUILD_FOR_WIN
static const GLContextType kAngle = GrContextFactory::kANGLE_GLContextType;
#else
static const GLContextType kNative = GrContextFactory::kNative_GLContextType;
#endif

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
    SkTArray<char, true> errorRows;
    errorRows.push_back_n(width * kRowCount);
    SkAutoLockPixels autoGr(gr);
    SkAutoLockPixels autoSk(sk);
    char* base = &errorRows[0];
    for (int y = 0; y < height; ++y) {
        SkPMColor* grRow = gr.getAddr32(0, y);
        SkPMColor* skRow = sk.getAddr32(0, y);
        char* cOut = &errorRows[(y % kRowCount) * width];
        for (int x = 0; x < width; ++x) {
            SkPMColor grColor = grRow[x];
            SkPMColor skColor = skRow[x];
            int dr = SkGetPackedR32(grColor) - SkGetPackedR32(skColor);
            int dg = SkGetPackedG32(grColor) - SkGetPackedG32(skColor);
            int db = SkGetPackedB32(grColor) - SkGetPackedB32(skColor);
            int error = SkTMax(SkAbs32(dr), SkTMax(SkAbs32(dg), SkAbs32(db)));
            if ((cOut[x] = error >= kThreshold) && x >= 2
                    && base[x - 2] && base[width + x - 2] && base[width * 2 + x - 2]
                    && base[x - 1] && base[width + x - 1] && base[width * 2 + x - 1]
                    && base[x - 0] && base[width + x - 0] && base[width * 2 + x - 0]) {
                errorTotal += error;
            }
        }
    }
    return errorTotal;
}

static bool addError(SkpSkGrThreadState* data) {
    bool foundSmaller = false;
    int dCount = data->fFoundCount;
    int pixelError = data->fResult.fPixelError;
    if (data->fFoundCount < kMaxFiles) {
        data->fError[dCount] = pixelError;
        strcpy(data->fFilesFound[dCount], data->fResult.fFilename);
        data->fDirsFound[dCount] = data->fResult.fDirNo;
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
        strcpy(data->fFilesFound[smallestIndex], data->fResult.fFilename);
        data->fDirsFound[smallestIndex] = data->fResult.fDirNo;
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
        SkDebugf("unable to encode gr %s (width=%d height=%d)br \n", pngName,
                    bitmap.width(), bitmap.height());
    }
}

void TestResult::testOne() {
    SkPicture* pic = NULL;
    {
        SkString d;
        d.printf("    {%d, \"%s\"},", fDirNo, fFilename);
        SkString path = make_filepath(fDirNo, IN_DIR, fFilename);
        SkFILEStream stream(path.c_str());
        if (!stream.isValid()) {
            SkDebugf("invalid stream %s\n", path.c_str());
            goto finish;
        }
        if (fTestStep == kEncodeFiles) {
            size_t length = stream.getLength();
            SkTArray<char, true> bytes;
            bytes.push_back_n(length);
            stream.read(&bytes[0], length);
            stream.rewind();
            SkString wPath = make_filepath(0, outSkpDir, fFilename);
            SkFILEWStream wStream(wPath.c_str());
            wStream.write(&bytes[0], length);
            wStream.flush();
        }
        pic = SkPicture::CreateFromStream(&stream, &SkImageDecoder::DecodeMemory);
        if (!pic) {
            SkDebugf("unable to decode %s\n", fFilename);
            goto finish;
        }
        int pWidth = pic->width();
        int pHeight = pic->height();
        int pLargerWH = SkTMax(pWidth, pHeight);
        GrContextFactory contextFactory;
#ifdef SK_BUILD_FOR_WIN
        GrContext* context = contextFactory.get(kAngle);
#else
        GrContext* context = contextFactory.get(kNative);
#endif
        if (NULL == context) {
            SkDebugf("unable to allocate context for %s\n", fFilename);
            goto finish;
        }
        int maxWH = context->getMaxRenderTargetSize();
        int scale = 1;
        while (pLargerWH / scale > maxWH) {
            scale *= 2;
        }
        SkBitmap bitmap;
        SkIPoint dim;
        do {
            dim.fX = (pWidth + scale - 1) / scale;
            dim.fY = (pHeight + scale - 1) / scale;
            bitmap.setConfig(SkBitmap::kARGB_8888_Config, dim.fX, dim.fY);
            bool success = bitmap.allocPixels();
            if (success) {
                break;
            }
            SkDebugf("-%d-", scale);
        } while ((scale *= 2) < 256);
        if (scale >= 256) {
            SkDebugf("unable to allocate bitmap for %s (w=%d h=%d) (sw=%d sh=%d)\n",
                    fFilename, pWidth, pHeight, dim.fX, dim.fY);
            goto finish;
        }
        SkCanvas skCanvas(bitmap);
        drawPict(pic, &skCanvas, fScaleOversized ? scale : 1);
        GrTextureDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags = kRenderTarget_GrTextureFlagBit;
        desc.fWidth = dim.fX;
        desc.fHeight = dim.fY;
        desc.fSampleCnt = 0;
        SkAutoTUnref<GrTexture> texture(context->createUncachedTexture(desc, NULL, 0));
        if (!texture) {
            SkDebugf("unable to allocate texture for %s (w=%d h=%d)\n", fFilename,
                dim.fX, dim.fY);
            goto finish;
        }
        SkGpuDevice grDevice(context, texture.get());
        SkCanvas grCanvas(&grDevice);
        drawPict(pic, &grCanvas, fScaleOversized ? scale : 1);
        const SkBitmap& grBitmap = grDevice.accessBitmap(false);
        if (fTestStep == kCompareBits) {
            fPixelError = similarBits(grBitmap, bitmap);
            int skTime = timePict(pic, &skCanvas);
            int grTime = timePict(pic, &grCanvas);
            fTime = skTime - grTime;
        } else if (fTestStep == kEncodeFiles) {
            SkString pngStr = make_png_name(fFilename);
            const char* pngName = pngStr.c_str();
            writePict(grBitmap, outGrDir, pngName);
            writePict(bitmap, outSkDir, pngName);
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
        , fIndex(0)
        , fStatusPath(makeStatusString(dirNo)) {
        if (!sk_exists(fStatusPath.c_str())) {
            return;
        }
        SkFILEStream reader;
        reader.setPath(fStatusPath.c_str());
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
            SkAssertResult(reader.read(&c, 1) != 0);
            if (c == ' ') {
                break;
            }
            SkASSERT(c >= '0' && c <= '9');
            result->fPixelError = result->fPixelError * 10 + (c - '0');
        } while (true);
        bool minus = false;
        do {
            if (reader.read(&c, 1) == 0) {
                break;
            }
            if (c == '\r' && reader.read(&c, 1) == 0) {
                break;
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
        if (fIndex < fResults.count()) {
            *result = fResults[fIndex++];
            SkASSERT(filename.equals(result->fFilename));
            SkString outStr(result->status());
            stream->write(outStr.c_str(), outStr.size());
            stream->flush();
            return true;
        }
        return false;
    }

private:
    int fDirNo;
    int fIndex;
    SkTArray<TestResult, true> fResults;
    SkString fStatusPath;
};

static bool initTest() {
#if !defined SK_BUILD_FOR_WIN && !defined SK_BUILD_FOR_MAC
    SK_CONF_SET("images.jpeg.suppressDecoderWarnings", true);
    SK_CONF_SET("images.png.suppressDecoderWarnings", true);
#endif
    return make_out_dirs();
}

static void SkpSkGrTest(skiatest::Reporter* reporter) {
    SkTArray<TestResult, true> errors;
    if (!initTest()) {
        return;
    }
    SkpSkGrThreadState state;
    state.init(0);
    int smallCount = 0;
    for (int dirNo = 1; dirNo <= 100; ++dirNo) {
        SkString pictDir = make_in_dir_name(dirNo);
        SkASSERT(pictDir.size());
        if (reporter->verbose()) {
            SkDebugf("dirNo=%d\n", dirNo);
        }
        SkOSFile::Iter iter(pictDir.c_str(), "skp");
        SkString filename;
        int testCount = 0;
        PreParser preParser(dirNo);
        SkFILEWStream statusStream(makeStatusString(dirNo).c_str());
        while (iter.next(&filename)) {
            for (size_t index = 0; index < skipOverSkGrCount; ++index) {
                if (skipOverSkGr[index].directory == dirNo
                        && strcmp(filename.c_str(), skipOverSkGr[index].filename) == 0) {
                    goto skipOver;
                }
            }
            if (preParser.match(filename, &statusStream, &state.fResult)) {
                addError(&state);
                ++testCount;
                goto checkEarlyExit;
            }
            if (state.fSmallestError > 5000000) {
                goto breakOut;
            }
            {
                TestResult& result = state.fResult;
                result.test(dirNo, filename);
                SkString outStr(result.status());
                statusStream.write(outStr.c_str(), outStr.size());
                statusStream.flush();
                if (1) {
                    SkDebugf("%s", outStr.c_str());
                }
                bool noMatch = addError(&state);
                if (noMatch) {
                    smallCount = 0;
                } else if (++smallCount > 10000) {
                    goto breakOut;
                }
            }
            ++testCount;
            if (reporter->verbose()) {
                if (testCount % 100 == 0) {
                    SkDebugf("#%d\n", testCount);
                }
            }
    skipOver:
            reporter->bumpTestCount();
    checkEarlyExit:
            if (1 && testCount == 20) {
                break;
            }
        }
    }
breakOut:
    if (reporter->verbose()) {
        for (int index = 0; index < state.fFoundCount; ++index) {
            SkDebugf("%d %s %d\n", state.fDirsFound[index], state.fFilesFound[index],
                     state.fError[index]);
        }
    }
    for (int index = 0; index < state.fFoundCount; ++index) {
        TestResult::Test(state.fDirsFound[index], state.fFilesFound[index], kEncodeFiles,
                reporter->verbose());
        if (reporter->verbose()) SkDebugf("+");
    }
}

static void bumpCount(skiatest::Reporter* reporter, bool skipping) {
    if (reporter->verbose()) {
        static int threadTestCount;
        sk_atomic_inc(&threadTestCount);
        if (!skipping && threadTestCount % 100 == 0) {
            SkDebugf("#%d\n", threadTestCount);
        }
        if (skipping && threadTestCount % 10000 == 0) {
            SkDebugf("#%d\n", threadTestCount);
        }
    }
}

static void testSkGrMain(SkpSkGrThreadState* data) {
    data->fResult.testOne();
    bumpCount(data->fReporter, false);
    data->fReporter->bumpTestCount();
}

static void SkpSkGrThreadedTest(skiatest::Reporter* reporter) {
    if (!initTest()) {
        return;
    }
    int threadCount = reporter->allowThreaded() ? 3 : 1;
    SkpSkGrThreadedTestRunner testRunner(reporter, threadCount);
    for (int dirIndex = 1; dirIndex <= 100; ++dirIndex) {
        SkString pictDir = make_in_dir_name(dirIndex);
        if (pictDir.size() == 0) {
            continue;
        }
        SkOSFile::Iter iter(pictDir.c_str(), "skp");
        SkString filename;
        while (iter.next(&filename)) {
            SkString pngName = make_png_name(filename.c_str());
            SkString oldPng = make_filepath(dirIndex, outSkDir, pngName.c_str());
            SkString newPng = make_filepath(dirIndex, outGrDir, pngName.c_str());
            if (sk_exists(oldPng.c_str()) && sk_exists(newPng.c_str())) {
                bumpCount(reporter, true);
                continue;
            }
            for (size_t index = 0; index < skipOverSkGrCount; ++index) {
                if (skipOverSkGr[index].directory == dirIndex
                        && strcmp(filename.c_str(), skipOverSkGr[index].filename) == 0) {
                    bumpCount(reporter, true);
                    goto skipOver;
                }
            }
            *testRunner.fRunnables.append() = SkNEW_ARGS(SkpSkGrThreadedRunnable,
                    (&testSkGrMain, dirIndex, filename.c_str(), &testRunner));
    skipOver:
            ;
        }
    }
    testRunner.render();
    SkpSkGrThreadState& max = testRunner.fRunnables[0]->fState;
    for (int dirIndex = 2; dirIndex <= 100; ++dirIndex) {
        SkpSkGrThreadState& state = testRunner.fRunnables[dirIndex - 1]->fState;
        for (int index = 0; index < state.fFoundCount; ++index) {
            int maxIdx = max.fFoundCount;
            if (maxIdx < kMaxFiles) {
                max.fError[maxIdx] = state.fError[index];
                strcpy(max.fFilesFound[maxIdx], state.fFilesFound[index]);
                max.fDirsFound[maxIdx] = state.fDirsFound[index];
                ++max.fFoundCount;
                continue;
            }
            for (maxIdx = 0; maxIdx < max.fFoundCount; ++maxIdx) {
                if (max.fError[maxIdx] < state.fError[index]) {
                    max.fError[maxIdx] = state.fError[index];
                    strcpy(max.fFilesFound[maxIdx], state.fFilesFound[index]);
                    max.fDirsFound[maxIdx] = state.fDirsFound[index];
                    break;
                }
            }
        }
    }
    TestResult encoder;
    encoder.fTestStep = kEncodeFiles;
    for (int index = 0; index < max.fFoundCount; ++index) {
        encoder.fDirNo = max.fDirsFound[index];
        strcpy(encoder.fFilename, max.fFilesFound[index]);
        encoder.testOne();
        SkDebugf("+");
    }
}

static void SkpSkGrOneOffTest(skiatest::Reporter* reporter) {
    if (!initTest()) {
        return;
    }
    int testIndex = 166;
    int dirIndex = skipOverSkGr[testIndex - 166].directory;
    SkString pictDir = make_in_dir_name(dirIndex);
    if (pictDir.size() == 0) {
        return;
    }
    SkString filename(skipOverSkGr[testIndex - 166].filename);
    TestResult::Test(dirIndex, filename.c_str(), kCompareBits, reporter->verbose());
    TestResult::Test(dirIndex, filename.c_str(), kEncodeFiles, reporter->verbose());
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(SkpSkGrTest)

DEFINE_TESTCLASS_SHORT(SkpSkGrOneOffTest)

DEFINE_TESTCLASS_SHORT(SkpSkGrThreadedTest)
