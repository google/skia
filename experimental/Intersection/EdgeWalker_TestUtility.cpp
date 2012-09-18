/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkStream.h"

#include <algorithm>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#undef SkASSERT
#define SkASSERT(cond) while (!(cond)) { sk_throw(); }

static const char marker[] =
    "</div>\n"
    "\n"
    "<script type=\"text/javascript\">\n"
    "\n"
    "var testDivs = [\n";

static const char preferredFilename[] = "/flash/debug/XX.txt";
static const char backupFilename[] = "../../experimental/Intersection/debugXX.txt";

static bool gShowPath = false;
static bool gComparePaths = true;
//static bool gDrawLastAsciiPaths = true;
//static bool gDrawAllAsciiPaths = false;
static bool gShowOutputProgress = false;
static bool gShowAsciiPaths = true;
static bool gComparePathsAssert = false;
static bool gPathStrAssert = true;

void showPath(const SkPath& path, const char* str) {
    SkDebugf("%s\n", !str ? "original:" : str);
    SkPath::Iter iter(path, true);
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("path.moveTo(%1.9g, %1.9g);\n", pts[0].fX, pts[0].fY);
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("path.lineTo(%1.9g, %1.9g);\n", pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("path.quadTo(%1.9g, %1.9g, %1.9g, %1.9g);\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("path.cubicTo(%1.9g, %1.9g, %1.9g, %1.9g);\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY,
                    pts[3].fX, pts[3].fY);
                break;
            case SkPath::kClose_Verb:
                SkDebugf("path.close();\n");
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
}

static int pathsDrawTheSame(const SkPath& one, const SkPath& two,
        SkBitmap& bits, SkCanvas* c, int& error2x2) {
    SkCanvas* canvasPtr = c;
    if (!c) {
        canvasPtr = new SkCanvas(bits);
    }
    const int bitWidth = 64;
    const int bitHeight = 64;
    if (bits.width() == 0) {
        bits.setConfig(SkBitmap::kARGB_8888_Config, bitWidth * 2, bitHeight);
        bits.allocPixels();
        canvasPtr->setBitmapDevice(bits);
    }
    SkRect larger = one.getBounds();
    larger.join(two.getBounds());
    SkScalar largerWidth = larger.width();
    if (largerWidth < 4) {
        largerWidth = 4;
    }
    SkScalar largerHeight = larger.height();
    if (largerHeight < 4) {
        largerHeight = 4;
    }
    SkScalar hScale = (bitWidth - 2) / largerWidth;
    SkScalar vScale = (bitHeight - 2) / largerHeight;
    SkMatrix scale;
    scale.reset();
    scale.preScale(hScale, vScale);
    SkPath scaledOne, scaledTwo;
    one.transform(scale, &scaledOne);
    two.transform(scale, &scaledTwo);
    const SkRect& bounds1 = scaledOne.getBounds();
    SkCanvas& canvas = *canvasPtr;
    canvas.drawColor(SK_ColorWHITE);
    SkPaint paint;
    canvas.save();
    canvas.translate(-bounds1.fLeft + 1, -bounds1.fTop + 1);
    canvas.drawPath(scaledOne, paint);
    canvas.restore();
    canvas.save();
    canvas.translate(-bounds1.fLeft + 1 + bitWidth, -bounds1.fTop + 1);
    canvas.drawPath(scaledTwo, paint);
    canvas.restore();
    int errors2 = 0;
    int errors = 0;
    for (int y = 0; y < bitHeight - 1; ++y) {
        uint32_t* addr1 = bits.getAddr32(0, y);
        uint32_t* addr2 = bits.getAddr32(0, y + 1);
        uint32_t* addr3 = bits.getAddr32(bitWidth, y);
        uint32_t* addr4 = bits.getAddr32(bitWidth, y + 1);
        for (int x = 0; x < bitWidth - 1; ++x) {
            // count 2x2 blocks
            bool err = addr1[x] != addr3[x];
            if (err) {
                errors2 += addr1[x + 1] != addr3[x + 1]
                        && addr2[x] != addr4[x] && addr2[x + 1] != addr4[x + 1];
                errors++;
            }
        }
    }
    if (!c) {
        delete canvasPtr;
    }
    if (errors2 >= 6 || errors > 160) {
        SkDebugf("%s errors2=%d errors=%d\n", __FUNCTION__, errors2, errors);
    }
    if (errors2 >= 7) {
        drawAsciiPaths(scaledOne, scaledTwo, true);
    }
    error2x2 = errors2;
    return errors;
}

bool drawAsciiPaths(const SkPath& one, const SkPath& two, bool drawPaths) {
    if (!drawPaths) {
        return true;
    }
    if (gShowAsciiPaths) {
        showPath(one, "one:");
        showPath(two, "two:");
    }
    const SkRect& bounds1 = one.getBounds();
    const SkRect& bounds2 = two.getBounds();
    SkRect larger = bounds1;
    larger.join(bounds2);
    SkBitmap bits;
    char out[256];
    int bitWidth = SkScalarCeil(larger.width()) + 2;
    if (bitWidth * 2 + 1 >= (int) sizeof(out)) {
        return false;
    }
    int bitHeight = SkScalarCeil(larger.height()) + 2;
    if (bitHeight >= (int) sizeof(out)) {
        return false;
    }
    bits.setConfig(SkBitmap::kARGB_8888_Config, bitWidth * 2, bitHeight);
    bits.allocPixels();
    SkCanvas canvas(bits);
    canvas.drawColor(SK_ColorWHITE);
    SkPaint paint;
    canvas.save();
    canvas.translate(-bounds1.fLeft + 1, -bounds1.fTop + 1);
    canvas.drawPath(one, paint);
    canvas.restore();
    canvas.save();
    canvas.translate(-bounds1.fLeft + 1 + bitWidth, -bounds1.fTop + 1);
    canvas.drawPath(two, paint);
    canvas.restore();
    for (int y = 0; y < bitHeight; ++y) {
        uint32_t* addr1 = bits.getAddr32(0, y);
        int x;
        char* outPtr = out;
        for (x = 0; x < bitWidth; ++x) {
            *outPtr++ = addr1[x] == (uint32_t) -1 ? '_' : 'x';
        }
        *outPtr++ = '|';
        for (x = bitWidth; x < bitWidth * 2; ++x) {
            *outPtr++ = addr1[x] == (uint32_t) -1 ? '_' : 'x';
        }
        *outPtr++ = '\0';
        SkDebugf("%s\n", out);
    }
    return true;
}

int comparePaths(const SkPath& one, const SkPath& two, SkBitmap& bitmap,
        SkCanvas* canvas) {
    int errors2x2;
    int errors = pathsDrawTheSame(one, two, bitmap, canvas, errors2x2);
    if (errors2x2 == 0) {
        return 0;
    }
    const int MAX_ERRORS = 8;
    if (errors2x2 > MAX_ERRORS && gComparePathsAssert) {
        SkDebugf("%s errors=%d\n", __FUNCTION__, errors);
        showPath(one);
        showPath(two, "simplified:");
        SkASSERT(0);
    }
    return errors2x2 > MAX_ERRORS ? errors2x2 : 0;
}

// doesn't work yet
void comparePathsTiny(const SkPath& one, const SkPath& two) {
    const SkRect& bounds1 = one.getBounds();
    const SkRect& bounds2 = two.getBounds();
    SkRect larger = bounds1;
    larger.join(bounds2);
    SkBitmap bits;
    int bitWidth = SkScalarCeil(larger.width()) + 2;
    int bitHeight = SkScalarCeil(larger.height()) + 2;
    bits.setConfig(SkBitmap::kA1_Config, bitWidth * 2, bitHeight);
    bits.allocPixels();
    SkCanvas canvas(bits);
    canvas.drawColor(SK_ColorWHITE);
    SkPaint paint;
    canvas.save();
    canvas.translate(-bounds1.fLeft + 1, -bounds1.fTop + 1);
    canvas.drawPath(one, paint);
    canvas.restore();
    canvas.save();
    canvas.translate(-bounds2.fLeft + 1, -bounds2.fTop + 1);
    canvas.drawPath(two, paint);
    canvas.restore();
    for (int y = 0; y < bitHeight; ++y) {
        uint8_t* addr1 = bits.getAddr1(0, y);
        uint8_t* addr2 = bits.getAddr1(bitWidth, y);
        for (int x = 0; x < bits.rowBytes(); ++x) {
            SkASSERT(addr1[x] == addr2[x]);
        }
    }
}

bool testSimplify(const SkPath& path, bool fill, SkPath& out, SkBitmap& bitmap,
        SkCanvas* canvas) {
    if (gShowPath) {
        showPath(path);
    }
    simplify(path, fill, out);
    if (!gComparePaths) {
        return true;
    }
    return comparePaths(path, out, bitmap, canvas) == 0;
}

bool testSimplifyx(SkPath& path, bool useXor, SkPath& out, State4& state,
        const char* pathStr) {
    SkPath::FillType fillType = useXor ? SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType;
    path.setFillType(fillType);
    if (gShowPath) {
        showPath(path);
    }
    simplifyx(path, out);
    if (!gComparePaths) {
        return true;
    }
    int result = comparePaths(path, out, state.bitmap, state.canvas);
    if (result && gPathStrAssert) {
        SkDebugf("addTest %s\n", state.filename);
        char temp[8192];
        bzero(temp, sizeof(temp));
        SkMemoryWStream stream(temp, sizeof(temp));
        outputToStream(state, pathStr, fillType, stream);
        SkDebugf(temp);
        SkASSERT(0);
    }
    return result == 0;
}

bool testSimplifyx(const SkPath& path) {
    SkPath out;
    simplifyx(path, out);
    SkBitmap bitmap;
    int result = comparePaths(path, out, bitmap, 0);
    if (result && gPathStrAssert) {
        SkASSERT(0);
    }
    return result == 0;
}

const int maxThreadsAllocated = 64;
static int maxThreads = 1;
static int threadIndex;
State4 threadState[maxThreadsAllocated];
static int testNumber;
static const char* testName;
static bool debugThreads = true;

State4* State4::queue = NULL;
pthread_mutex_t State4::addQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t State4::checkQueue = PTHREAD_COND_INITIALIZER;

State4::State4() {
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 150 * 2, 100);
    bitmap.allocPixels();
    canvas = new SkCanvas(bitmap);
}

void createThread(State4* statePtr, void* (*testFun)(void* )) {
    int threadError = pthread_create(&statePtr->threadID, NULL, testFun,
            (void*) statePtr);
    SkASSERT(!threadError);
}

int dispatchTest4(void* (*testFun)(void* ), int a, int b, int c, int d) {
    int testsRun = 0;
    State4* statePtr;
    if (!gRunTestsInOneThread) {
        pthread_mutex_lock(&State4::addQueue);
        if (threadIndex < maxThreads) {
            statePtr = &threadState[threadIndex];
            statePtr->testsRun = 0;
            statePtr->a = a;
            statePtr->b = b;
            statePtr->c = c;
            statePtr->d = d;
            statePtr->done = false;
            statePtr->index = threadIndex;
            statePtr->last = false;
            if (debugThreads) SkDebugf("%s %d create done=%d last=%d\n", __FUNCTION__,
                    statePtr->index, statePtr->done, statePtr->last);
            pthread_cond_init(&statePtr->initialized, NULL);
            ++threadIndex;
            createThread(statePtr, testFun);
        } else {
            while (!State4::queue) {
                if (debugThreads) SkDebugf("%s checkQueue\n", __FUNCTION__);
                pthread_cond_wait(&State4::checkQueue, &State4::addQueue);
            }
            statePtr = State4::queue;
            testsRun += statePtr->testsRun;
            statePtr->testsRun = 0;
            statePtr->a = a;
            statePtr->b = b;
            statePtr->c = c;
            statePtr->d = d;
            statePtr->done = false;
            State4::queue = NULL;
            for (int index = 0; index < maxThreads; ++index) {
                if (threadState[index].done) {
                    State4::queue = &threadState[index];
                }
            }
            if (debugThreads) SkDebugf("%s %d init done=%d last=%d queued=%d\n", __FUNCTION__,
                    statePtr->index, statePtr->done, statePtr->last,
                    State4::queue ? State4::queue->index : -1);
            pthread_cond_signal(&statePtr->initialized);
        }
        pthread_mutex_unlock(&State4::addQueue);
    } else {
        statePtr = &threadState[0];
        testsRun += statePtr->testsRun;
        statePtr->testsRun = 0;
        statePtr->a = a;
        statePtr->b = b;
        statePtr->c = c;
        statePtr->d = d;
        statePtr->done = false;
        statePtr->index = threadIndex;
        statePtr->last = false;
        (*testFun)(statePtr);
    }
    return testsRun;
}

void initializeTests(const char* test, size_t testNameSize) {
    testName = test;
    if (!gRunTestsInOneThread) {
        int threads = -1;
        size_t size = sizeof(threads);
        sysctlbyname("hw.logicalcpu_max", &threads, &size, NULL, 0);
        if (threads > 0) {
            maxThreads = threads;
        } else {
            maxThreads = 8;
        }
    }
    SkFILEStream inFile("../../experimental/Intersection/op.htm");
    if (inFile.isValid()) {
        SkTDArray<char> inData;
        inData.setCount(inFile.getLength());
        size_t inLen = inData.count();
        inFile.read(inData.begin(), inLen);
        inFile.setPath(NULL);
        char* insert = strstr(inData.begin(), marker);
        if (insert) {
            insert += sizeof(marker) - 1;
            const char* numLoc = insert + 4 /* indent spaces */ + testNameSize - 1;
            testNumber = atoi(numLoc) + 1;
        }
    }
    const char* filename = preferredFilename;
    SkFILEWStream preferredTest(filename);
    if (!preferredTest.isValid()) {
        filename = backupFilename;
        SkFILEWStream backupTest(filename);
        SkASSERT(backupTest.isValid());
    }
    for (int index = 0; index < maxThreads; ++index) {
        State4* statePtr = &threadState[index];
        strcpy(statePtr->filename, filename);
        size_t len = strlen(filename);
        SkASSERT(statePtr->filename[len - 6] == 'X');
        SkASSERT(statePtr->filename[len - 5] == 'X');
        statePtr->filename[len - 6] = '0' + index / 10;
        statePtr->filename[len - 5] = '0' + index % 10;
    }
    threadIndex = 0;
}

void outputProgress(const State4& state, const char* pathStr, SkPath::FillType pathFillType) {
    if (gRunTestsInOneThread && gShowOutputProgress) {
        if (pathFillType == SkPath::kEvenOdd_FillType) {
            SkDebugf("    path.setFillType(SkPath::kEvenOdd_FillType);\n", pathStr);
        }
        SkDebugf("%s\n", pathStr);
    }
    SkFILEWStream outFile(state.filename);
    if (!outFile.isValid()) {
        SkASSERT(0);
        return;
    }
    outputToStream(state, pathStr, pathFillType, outFile);
}

static void writeTestName(SkPath::FillType pathFillType, SkWStream& outFile) {
    outFile.writeText(testName);
    outFile.writeDecAsText(testNumber);
    if (pathFillType == SkPath::kEvenOdd_FillType) {
        outFile.writeText("x");
    }
}

void outputToStream(const State4& state, const char* pathStr, SkPath::FillType pathFillType, SkWStream& outFile) {
    outFile.writeText("<div id=\"");
    writeTestName(pathFillType, outFile);
    outFile.writeText("\">\n");
    if (pathFillType == SkPath::kEvenOdd_FillType) {
        outFile.writeText("    path.setFillType(SkPath::kEvenOdd_FillType);\n");
    }
    outFile.writeText(pathStr);
    outFile.writeText("</div>\n\n");

    outFile.writeText(marker);
    outFile.writeText("    ");
    writeTestName(pathFillType, outFile);
    outFile.writeText(",\n\n\n");

    outFile.writeText("static void ");
    writeTestName(pathFillType, outFile);
    outFile.writeText("() {\n    SkPath path;\n");
    if (pathFillType == SkPath::kEvenOdd_FillType) {
        outFile.writeText("    path.setFillType(SkPath::kEvenOdd_FillType);\n");
    }
    outFile.writeText(pathStr);
    outFile.writeText("    testSimplifyx(path);\n}\n\n");
    outFile.writeText("static void (*firstTest)() = ");
    writeTestName(pathFillType, outFile);
    outFile.writeText(";\n\n");

    outFile.writeText("static struct {\n");
    outFile.writeText("    void (*fun)();\n");
    outFile.writeText("    const char* str;\n");
    outFile.writeText("} tests[] = {\n");
    outFile.writeText("    TEST(");
    writeTestName(pathFillType, outFile);
    outFile.writeText("),\n");
    outFile.flush();
}

bool runNextTestSet(State4& state) {
    if (gRunTestsInOneThread) {
        return false;
    }
    pthread_mutex_lock(&State4::addQueue);
    state.done = true;
    State4::queue = &state;
    if (debugThreads) SkDebugf("%s %d checkQueue done=%d last=%d\n", __FUNCTION__, state.index,
        state.done, state.last);
    pthread_cond_signal(&State4::checkQueue);
    while (state.done && !state.last) {
        if (debugThreads) SkDebugf("%s %d done=%d last=%d\n", __FUNCTION__, state.index, state.done, state.last);
        pthread_cond_wait(&state.initialized, &State4::addQueue);
    }
    pthread_mutex_unlock(&State4::addQueue);
    return !state.last;
}

int waitForCompletion() {
    int testsRun = 0;
    if (!gRunTestsInOneThread) {
        pthread_mutex_lock(&State4::addQueue);
        int runningThreads = maxThreads;
        int index;
        while (runningThreads > 0) {
            while (!State4::queue) {
                if (debugThreads) SkDebugf("%s checkQueue\n", __FUNCTION__);
                pthread_cond_wait(&State4::checkQueue, &State4::addQueue);
            }
            while (State4::queue) {
                --runningThreads;
                SkDebugf("•");
                State4::queue->last = true;
                State4* next = NULL;
                for (index = 0; index < maxThreads; ++index) {
                    State4& test = threadState[index];
                    if (test.done && !test.last) {
                        next = &test;
                    }
                }
                if (debugThreads) SkDebugf("%s %d next=%d deQueue\n", __FUNCTION__,
                    State4::queue->index, next ? next->index : -1);
                pthread_cond_signal(&State4::queue->initialized);
                State4::queue = next;
            }
        }
        pthread_mutex_unlock(&State4::addQueue);
        for (index = 0; index < maxThreads; ++index) {
            pthread_join(threadState[index].threadID, NULL);
            testsRun += threadState[index].testsRun;
        }
        SkDebugf("\n");
    }
#ifdef SK_DEBUG
    gDebugMaxWindSum = SK_MaxS32;
    gDebugMaxWindValue = SK_MaxS32;
#endif
    return testsRun;
}
