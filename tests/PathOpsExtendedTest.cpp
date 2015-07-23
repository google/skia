/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkForceLinking.h"
#include "SkMatrix.h"
#include "SkMutex.h"
#include "SkPaint.h"
#include "SkRTConf.h"
#include "SkStream.h"

#ifdef SK_BUILD_FOR_MAC
#include <sys/sysctl.h>
#endif

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_bool2(runFail, f, false, "run tests known to fail.");
DEFINE_bool2(runBinary, f, false, "run tests known to fail binary sect.");

static const char marker[] =
    "</div>\n"
    "\n"
    "<script type=\"text/javascript\">\n"
    "\n"
    "var testDivs = [\n";

static const char* opStrs[] = {
    "kDifference_SkPathOp",
    "kIntersect_SkPathOp",
    "kUnion_SkPathOp",
    "kXor_PathOp",
    "kReverseDifference_SkPathOp",
};

static const char* opSuffixes[] = {
    "d",
    "i",
    "u",
    "o",
};

#if DEBUG_SHOW_TEST_NAME
static void showPathData(const SkPath& path) {
    SkPath::RawIter iter(path);
    uint8_t verb;
    SkPoint pts[4];
    SkPoint firstPt = {0, 0}, lastPt = {0, 0};
    bool firstPtSet = false;
    bool lastPtSet = true;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (firstPtSet && lastPtSet && firstPt != lastPt) {
                    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", lastPt.fX, lastPt.fY,
                            firstPt.fX, firstPt.fY);
                    lastPtSet = false;
                }
                firstPt = pts[0];
                firstPtSet = true;
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", pts[0].fX, pts[0].fY,
                        pts[1].fX, pts[1].fY);
                lastPt = pts[1];
                lastPtSet = true;
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
                        pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                lastPt = pts[2];
                lastPtSet = true;
                break;
            case SkPath::kConic_Verb:
                SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},  //weight=%1.9g\n",
                        pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY,
                        iter.conicWeight());
                lastPt = pts[2];
                lastPtSet = true;
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n",
                        pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY,
                        pts[3].fX, pts[3].fY);
                lastPt = pts[3];
                lastPtSet = true;
                break;
            case SkPath::kClose_Verb:
                if (firstPtSet && lastPtSet && firstPt != lastPt) {
                    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", lastPt.fX, lastPt.fY,
                            firstPt.fX, firstPt.fY);
                }
                firstPtSet = lastPtSet = false;
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
    if (firstPtSet && lastPtSet && firstPt != lastPt) {
        SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", lastPt.fX, lastPt.fY,
                firstPt.fX, firstPt.fY);
    }
}
#endif

void showOp(const SkPathOp op) {
    switch (op) {
        case kDifference_SkPathOp:
            SkDebugf("op difference\n");
            break;
        case kIntersect_SkPathOp:
            SkDebugf("op intersect\n");
            break;
        case kUnion_SkPathOp:
            SkDebugf("op union\n");
            break;
        case kXOR_SkPathOp:
            SkDebugf("op xor\n");
            break;
        case kReverseDifference_SkPathOp:
            SkDebugf("op reverse difference\n");
            break;
        default:
            SkASSERT(0);
    }
}

#if DEBUG_SHOW_TEST_NAME
static char hexorator(int x) {
    if (x < 10) {
        return x + '0';
    }
    x -= 10;
    SkASSERT(x < 26);
    return x + 'A';
}
#endif

void ShowTestName(PathOpsThreadState* state, int a, int b, int c, int d) {
#if DEBUG_SHOW_TEST_NAME
    state->fSerialNo[0] = hexorator(state->fA);
    state->fSerialNo[1] = hexorator(state->fB);
    state->fSerialNo[2] = hexorator(state->fC);
    state->fSerialNo[3] = hexorator(state->fD);
    state->fSerialNo[4] = hexorator(a);
    state->fSerialNo[5] = hexorator(b);
    state->fSerialNo[6] = hexorator(c);
    state->fSerialNo[7] = hexorator(d);
    state->fSerialNo[8] = '\0';
    SkDebugf("%s\n", state->fSerialNo);
    if (strcmp(state->fSerialNo, state->fKey) == 0) {
        SkDebugf("%s\n", state->fPathStr);
    }
#endif
}

const int bitWidth = 64;
const int bitHeight = 64;

static void scaleMatrix(const SkPath& one, const SkPath& two, SkMatrix& scale) {
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
    scale.reset();
    scale.preScale(hScale, vScale);
}

static int pathsDrawTheSame(SkBitmap& bits, const SkPath& scaledOne, const SkPath& scaledTwo,
        int& error2x2) {
    if (bits.width() == 0) {
        bits.allocN32Pixels(bitWidth * 2, bitHeight);
    }
    SkCanvas canvas(bits);
    canvas.drawColor(SK_ColorWHITE);
    SkPaint paint;
    canvas.save();
    const SkRect& bounds1 = scaledOne.getBounds();
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
    error2x2 = errors2;
    return errors;
}

static int pathsDrawTheSame(const SkPath& one, const SkPath& two, SkBitmap& bits, SkPath& scaledOne,
        SkPath& scaledTwo, int& error2x2) {
    SkMatrix scale;
    scaleMatrix(one, two, scale);
    one.transform(scale, &scaledOne);
    two.transform(scale, &scaledTwo);
    return pathsDrawTheSame(bits, scaledOne, scaledTwo, error2x2);
}

bool drawAsciiPaths(const SkPath& one, const SkPath& two, bool drawPaths) {
    if (!drawPaths) {
        return true;
    }
    const SkRect& bounds1 = one.getBounds();
    const SkRect& bounds2 = two.getBounds();
    SkRect larger = bounds1;
    larger.join(bounds2);
    SkBitmap bits;
    char out[256];
    int bitWidth = SkScalarCeilToInt(larger.width()) + 2;
    if (bitWidth * 2 + 1 >= (int) sizeof(out)) {
        return false;
    }
    int bitHeight = SkScalarCeilToInt(larger.height()) + 2;
    if (bitHeight >= (int) sizeof(out)) {
        return false;
    }
    bits.allocN32Pixels(bitWidth * 2, bitHeight);
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

int comparePaths(skiatest::Reporter* reporter, const char* filename, const SkPath& one,
        const SkPath& two, SkBitmap& bitmap) {
    int errors2x2;
    SkPath scaledOne, scaledTwo;
    (void) pathsDrawTheSame(one, two, bitmap, scaledOne, scaledTwo, errors2x2);
    if (errors2x2 == 0) {
        return 0;
    }
    const int MAX_ERRORS = 9;
    return errors2x2 > MAX_ERRORS ? errors2x2 : 0;
}

const int gTestFirst = 41;
static int gTestNo = gTestFirst;
static SkTDArray<SkPathOp> gTestOp;

static void showPathOpPath(const char* testName, const SkPath& one, const SkPath& two,
        const SkPath& a, const SkPath& b, const SkPath& scaledOne, const SkPath& scaledTwo,
        const SkPathOp shapeOp, const SkMatrix& scale) {
    SkASSERT((unsigned) shapeOp < SK_ARRAY_COUNT(opStrs));
    if (!testName) {
        testName = "xOp";
    }
    SkDebugf("static void %s%d%s(skiatest::Reporter* reporter, const char* filename) {\n",
        testName, gTestNo, opSuffixes[shapeOp]);
    *gTestOp.append() = shapeOp;
    ++gTestNo;
    SkDebugf("    SkPath path, pathB;\n");
    SkPathOpsDebug::ShowOnePath(a, "path", false);
    SkPathOpsDebug::ShowOnePath(b, "pathB", false);
    SkDebugf("    testPathOp(reporter, path, pathB, %s, filename);\n", opStrs[shapeOp]);
    SkDebugf("}\n");
    drawAsciiPaths(scaledOne, scaledTwo, false);
}

void ShowTestArray(const char* testName) {
    if (!testName) {
        testName = "xOp";
    }
    for (int x = gTestFirst; x < gTestNo; ++x) {
        SkDebugf("    TEST(%s%d%s),\n", testName, x, opSuffixes[gTestOp[x - gTestFirst]]);
    }
}

SK_DECLARE_STATIC_MUTEX(compareDebugOut3);

static int comparePaths(skiatest::Reporter* reporter, const char* testName, const SkPath& one,
        const SkPath& scaledOne, const SkPath& two, const SkPath& scaledTwo, SkBitmap& bitmap,
        const SkPath& a, const SkPath& b, const SkPathOp shapeOp, const SkMatrix& scale,
        bool expectSuccess) {
    int errors2x2;
    const int MAX_ERRORS = 8;
    (void) pathsDrawTheSame(bitmap, scaledOne, scaledTwo, errors2x2);
    if (!expectSuccess) {
        if (errors2x2 < MAX_ERRORS) {
            REPORTER_ASSERT(reporter, 0);
        }
        return 0;
    }
    if (errors2x2 == 0) {
        return 0;
    }
    if (errors2x2 >= MAX_ERRORS) {
        SkAutoMutexAcquire autoM(compareDebugOut3);
        showPathOpPath(testName, one, two, a, b, scaledOne, scaledTwo, shapeOp, scale);
        SkDebugf("\n/*");
        REPORTER_ASSERT(reporter, 0);
        SkDebugf(" */\n");
    }
    return errors2x2 >= MAX_ERRORS ? errors2x2 : 0;
}

// Default values for when reporter->verbose() is false.
static int testNumber = 55;
static const char* testName = "pathOpTest";

static void writeTestName(const char* nameSuffix, SkMemoryWStream& outFile) {
    outFile.writeText(testName);
    outFile.writeDecAsText(testNumber);
    ++testNumber;
    if (nameSuffix) {
        outFile.writeText(nameSuffix);
    }
}

static void outputToStream(const char* pathStr, const char* pathPrefix, const char* nameSuffix,
        const char* testFunction, bool twoPaths, SkMemoryWStream& outFile) {
#if 0
    outFile.writeText("\n<div id=\"");
    writeTestName(nameSuffix, outFile);
    outFile.writeText("\">\n");
    if (pathPrefix) {
        outFile.writeText(pathPrefix);
    }
    outFile.writeText(pathStr);
    outFile.writeText("</div>\n\n");

    outFile.writeText(marker);
    outFile.writeText("    ");
    writeTestName(nameSuffix, outFile);
    outFile.writeText(",\n\n\n");
#endif
    outFile.writeText("static void ");
    writeTestName(nameSuffix, outFile);
    outFile.writeText("(skiatest::Reporter* reporter) {\n    SkPath path");
    if (twoPaths) {
        outFile.writeText(", pathB");
    }
    outFile.writeText(";\n");
    if (pathPrefix) {
        outFile.writeText(pathPrefix);
    }
    outFile.writeText(pathStr);
    outFile.writeText("    ");
    outFile.writeText(testFunction);
    outFile.writeText("\n}\n\n");
#if 0
    outFile.writeText("static void (*firstTest)() = ");
    writeTestName(nameSuffix, outFile);
    outFile.writeText(";\n\n");

    outFile.writeText("static struct {\n");
    outFile.writeText("    void (*fun)();\n");
    outFile.writeText("    const char* str;\n");
    outFile.writeText("} tests[] = {\n");
    outFile.writeText("    TEST(");
    writeTestName(nameSuffix, outFile);
    outFile.writeText("),\n");
#endif
    outFile.flush();
}

SK_DECLARE_STATIC_MUTEX(simplifyDebugOut);

bool testSimplify(SkPath& path, bool useXor, SkPath& out, PathOpsThreadState& state,
                  const char* pathStr) {
    SkPath::FillType fillType = useXor ? SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType;
    path.setFillType(fillType);
    state.fReporter->bumpTestCount();
    if (!Simplify(path, &out)) {
        SkDebugf("%s did not expect failure\n", __FUNCTION__);
        REPORTER_ASSERT(state.fReporter, 0);
        return false;
    }
    if (!state.fReporter->verbose()) {
        return true;
    }
    int result = comparePaths(state.fReporter, NULL, path, out, *state.fBitmap);
    if (result) {
        SkAutoMutexAcquire autoM(simplifyDebugOut);
        char temp[8192];
        sk_bzero(temp, sizeof(temp));
        SkMemoryWStream stream(temp, sizeof(temp));
        const char* pathPrefix = NULL;
        const char* nameSuffix = NULL;
        if (fillType == SkPath::kEvenOdd_FillType) {
            pathPrefix = "    path.setFillType(SkPath::kEvenOdd_FillType);\n";
            nameSuffix = "x";
        }
        const char testFunction[] = "testSimplify(reporter, path);";
        outputToStream(pathStr, pathPrefix, nameSuffix, testFunction, false, stream);
        SkDebugf("%s", temp);
        REPORTER_ASSERT(state.fReporter, 0);
    }
    state.fReporter->bumpTestCount();
    return result == 0;
}

static bool inner_simplify(skiatest::Reporter* reporter, const SkPath& path, const char* filename,
        bool checkFail) {
#if 0 && DEBUG_SHOW_TEST_NAME
    showPathData(path);
#endif
    SkPath out;
    if (!Simplify(path, &out)) {
        SkDebugf("%s did not expect %s failure\n", __FUNCTION__, filename);
        REPORTER_ASSERT(reporter, 0);
        return false;
    }
    SkBitmap bitmap;
    int errors = comparePaths(reporter, filename, path, out, bitmap);
    if (!checkFail) {
        if (!errors) {
            SkDebugf("%s failing test %s now succeeds\n", __FUNCTION__, filename);
            REPORTER_ASSERT(reporter, 0);
            return false;
        }
    } else if (errors) {
        REPORTER_ASSERT(reporter, 0);
    }
    reporter->bumpTestCount();
    return errors == 0;
}

bool testSimplify(skiatest::Reporter* reporter, const SkPath& path, const char* filename) {
    return inner_simplify(reporter, path, filename, true);
}

bool testSimplifyCheck(skiatest::Reporter* reporter, const SkPath& path, const char* filename,
        bool checkFail) {
    return inner_simplify(reporter, path, filename, checkFail);
}

#if DEBUG_SHOW_TEST_NAME
static void showName(const SkPath& a, const SkPath& b, const SkPathOp shapeOp) {
    SkDebugf("\n");
    showPathData(a);
    showOp(shapeOp);
    showPathData(b);
}
#endif

bool OpDebug(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result,
             bool expectSuccess  SkDEBUGPARAMS(const char* testName));

static bool innerPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName, bool expectSuccess) {
#if 0 && DEBUG_SHOW_TEST_NAME
    showName(a, b, shapeOp);
#endif
    SkPath out;
    if (!OpDebug(a, b, shapeOp, &out, expectSuccess  SkDEBUGPARAMS(testName))) {
        SkDebugf("%s did not expect failure\n", __FUNCTION__);
        REPORTER_ASSERT(reporter, 0);
        return false;
    }
    if (!reporter->verbose()) {
        return true;
    }
    SkPath pathOut, scaledPathOut;
    SkRegion rgnA, rgnB, openClip, rgnOut;
    openClip.setRect(-16000, -16000, 16000, 16000);
    rgnA.setPath(a, openClip);
    rgnB.setPath(b, openClip);
    rgnOut.op(rgnA, rgnB, (SkRegion::Op) shapeOp);
    rgnOut.getBoundaryPath(&pathOut);

    SkMatrix scale;
    scaleMatrix(a, b, scale);
    SkRegion scaledRgnA, scaledRgnB, scaledRgnOut;
    SkPath scaledA, scaledB;
    scaledA.addPath(a, scale);
    scaledA.setFillType(a.getFillType());
    scaledB.addPath(b, scale);
    scaledB.setFillType(b.getFillType());
    scaledRgnA.setPath(scaledA, openClip);
    scaledRgnB.setPath(scaledB, openClip);
    scaledRgnOut.op(scaledRgnA, scaledRgnB, (SkRegion::Op) shapeOp);
    scaledRgnOut.getBoundaryPath(&scaledPathOut);
    SkBitmap bitmap;
    SkPath scaledOut;
    scaledOut.addPath(out, scale);
    scaledOut.setFillType(out.getFillType());
    int result = comparePaths(reporter, testName, pathOut, scaledPathOut, out, scaledOut, bitmap,
            a, b, shapeOp, scale, expectSuccess);
    reporter->bumpTestCount();
    return result == 0;
}

bool testPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName) {
    return innerPathOp(reporter, a, b, shapeOp, testName, true);
}

bool testPathOpCheck(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName, bool checkFail) {
    return innerPathOp(reporter, a, b, shapeOp, testName, checkFail);
}

bool testPathOpFailCheck(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName) {
    return innerPathOp(reporter, a, b, shapeOp, testName, false);
}

bool testPathFailOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
                 const SkPathOp shapeOp, const char* testName) {
#if DEBUG_SHOW_TEST_NAME
    showName(a, b, shapeOp);
#endif
    SkPath orig;
    orig.lineTo(54, 43);
    SkPath out = orig;
    if (Op(a, b, shapeOp, &out) ) {
        SkDebugf("%s test is expected to fail\n", __FUNCTION__);
        REPORTER_ASSERT(reporter, 0);
        return false;
    }
    SkASSERT(out == orig);
    return true;
}

SK_DECLARE_STATIC_MUTEX(gMutex);

void initializeTests(skiatest::Reporter* reporter, const char* test) {
#if 0  // doesn't work yet
    SK_CONF_SET("images.jpeg.suppressDecoderWarnings", true);
    SK_CONF_SET("images.png.suppressDecoderWarnings", true);
#endif
    if (reporter->verbose()) {
        SkAutoMutexAcquire lock(gMutex);
        testName = test;
        size_t testNameSize = strlen(test);
        SkFILEStream inFile("../../experimental/Intersection/op.htm");
        if (inFile.isValid()) {
            SkTDArray<char> inData;
            inData.setCount((int) inFile.getLength());
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
    }
}

void outputProgress(char* ramStr, const char* pathStr, SkPath::FillType pathFillType) {
    const char testFunction[] = "testSimplify(path);";
    const char* pathPrefix = NULL;
    const char* nameSuffix = NULL;
    if (pathFillType == SkPath::kEvenOdd_FillType) {
        pathPrefix = "    path.setFillType(SkPath::kEvenOdd_FillType);\n";
        nameSuffix = "x";
    }
    SkMemoryWStream rRamStream(ramStr, PATH_STR_SIZE);
    outputToStream(pathStr, pathPrefix, nameSuffix, testFunction, false, rRamStream);
}

void outputProgress(char* ramStr, const char* pathStr, SkPathOp op) {
    const char testFunction[] = "testOp(path);";
    SkASSERT((size_t) op < SK_ARRAY_COUNT(opSuffixes));
    const char* nameSuffix = opSuffixes[op];
    SkMemoryWStream rRamStream(ramStr, PATH_STR_SIZE);
    outputToStream(pathStr, NULL, nameSuffix, testFunction, true, rRamStream);
}

void RunTestSet(skiatest::Reporter* reporter, TestDesc tests[], size_t count,
                void (*firstTest)(skiatest::Reporter* , const char* filename),
                void (*skipTest)(skiatest::Reporter* , const char* filename),
                void (*stopTest)(skiatest::Reporter* , const char* filename), bool reverse) {
    size_t index;
    if (firstTest) {
        index = count - 1;
        while (index > 0 && tests[index].fun != firstTest) {
            --index;
        }
#if DEBUG_SHOW_TEST_NAME
        SkDebugf("\n<div id=\"%s\">\n", tests[index].str);
#endif
        (*tests[index].fun)(reporter, tests[index].str);
        if (tests[index].fun == stopTest) {
            return;
        }
    }
    index = reverse ? count - 1 : 0;
    size_t last = reverse ? 0 : count - 1;
    bool foundSkip = !skipTest;
    do {
        if (tests[index].fun == skipTest) {
            foundSkip = true;
        }
        if (foundSkip && tests[index].fun != firstTest) {
    #if DEBUG_SHOW_TEST_NAME
            SkDebugf("\n<div id=\"%s\">\n", tests[index].str);
    #endif
            (*tests[index].fun)(reporter, tests[index].str);
        }
        if (tests[index].fun == stopTest || index == last) {
            break;
        }
        index += reverse ? -1 : 1;
    } while (true);
#if DEBUG_SHOW_TEST_NAME
    SkDebugf(
            "\n"
            "</div>\n"
            "\n"
            "<script type=\"text/javascript\">\n"
            "\n"
            "var testDivs = [\n"
    );
    index = reverse ? count - 1 : 0;
    last = reverse ? 0 : count - 1;
    foundSkip = !skipTest;
    do {
        if (tests[index].fun == skipTest) {
            foundSkip = true;
        }
        if (foundSkip && tests[index].fun != firstTest) {
            SkDebugf("    %s,\n", tests[index].str);
        }
        if (tests[index].fun == stopTest || index == last) {
            break;
        }
        index += reverse ? -1 : 1;
    } while (true);
#endif
}
