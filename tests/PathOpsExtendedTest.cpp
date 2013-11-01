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
#include "SkPaint.h"
#include "SkRTConf.h"
#include "SkStream.h"
#include "SkThreadPool.h"

#ifdef SK_BUILD_FOR_MAC
#include <sys/sysctl.h>
#endif

__SK_FORCE_IMAGE_DECODER_LINKING;

static const char marker[] =
    "</div>\n"
    "\n"
    "<script type=\"text/javascript\">\n"
    "\n"
    "var testDivs = [\n";

static const char* opStrs[] = {
    "kDifference_PathOp",
    "kIntersect_PathOp",
    "kUnion_PathOp",
    "kXor_PathOp",
    "kReverseDifference_PathOp",
};

static const char* opSuffixes[] = {
    "d",
    "i",
    "u",
    "o",
};

static bool gShowPath = false;
static bool gComparePathsAssert = true;
static bool gPathStrAssert = true;

static const char* gFillTypeStr[] = {
    "kWinding_FillType",
    "kEvenOdd_FillType",
    "kInverseWinding_FillType",
    "kInverseEvenOdd_FillType"
};

static void output_scalar(SkScalar num) {
    if (num == (int) num) {
        SkDebugf("%d", (int) num);
    } else {
        SkString str;
        str.printf("%1.9g", num);
        int width = str.size();
        const char* cStr = str.c_str();
        while (cStr[width - 1] == '0') {
            --width;
        }
        str.resize(width);
        SkDebugf("%sf", str.c_str());
    }
}

static void output_points(const SkPoint* pts, int count) {
    for (int index = 0; index < count; ++index) {
        output_scalar(pts[index].fX);
        SkDebugf(", ");
        output_scalar(pts[index].fY);
        if (index + 1 < count) {
            SkDebugf(", ");
        }
    }
    SkDebugf(");\n");
}

static void showPathContours(SkPath::RawIter& iter, const char* pathName) {
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("    %s.moveTo(", pathName);
                output_points(&pts[0], 1);
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("    %s.lineTo(", pathName);
                output_points(&pts[1], 1);
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("    %s.quadTo(", pathName);
                output_points(&pts[1], 2);
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("    %s.cubicTo(", pathName);
                output_points(&pts[1], 3);
                break;
            case SkPath::kClose_Verb:
                SkDebugf("    %s.close();\n", pathName);
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
}

static void showPath(const SkPath& path, const char* pathName, bool includeDeclaration) {
    SkPath::RawIter iter(path);
#define SUPPORT_RECT_CONTOUR_DETECTION 0
#if SUPPORT_RECT_CONTOUR_DETECTION
    int rectCount = path.isRectContours() ? path.rectContours(NULL, NULL) : 0;
    if (rectCount > 0) {
        SkTDArray<SkRect> rects;
        SkTDArray<SkPath::Direction> directions;
        rects.setCount(rectCount);
        directions.setCount(rectCount);
        path.rectContours(rects.begin(), directions.begin());
        for (int contour = 0; contour < rectCount; ++contour) {
            const SkRect& rect = rects[contour];
            SkDebugf("path.addRect(%1.9g, %1.9g, %1.9g, %1.9g, %s);\n", rect.fLeft, rect.fTop,
                    rect.fRight, rect.fBottom, directions[contour] == SkPath::kCCW_Direction
                    ? "SkPath::kCCW_Direction" : "SkPath::kCW_Direction");
        }
        return;
    }
#endif
    SkPath::FillType fillType = path.getFillType();
    SkASSERT(fillType >= SkPath::kWinding_FillType && fillType <= SkPath::kInverseEvenOdd_FillType);
    if (includeDeclaration) {
        SkDebugf("    SkPath %s;\n", pathName);
    }
    SkDebugf("    %s.setFillType(SkPath::%s);\n", pathName, gFillTypeStr[fillType]);
    iter.setPath(path);
    showPathContours(iter, pathName);
}

#if DEBUG_SHOW_TEST_NAME
static void showPathData(const SkPath& path) {
    SkPath::RawIter iter(path);
    uint8_t verb;
    SkPoint pts[4];
    SkPoint firstPt, lastPt;
    bool firstPtSet = false;
    bool lastPtSet = true;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
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
}
#endif

void showOp(const SkPathOp op) {
    switch (op) {
        case kDifference_PathOp:
            SkDebugf("op difference\n");
            break;
        case kIntersect_PathOp:
            SkDebugf("op intersect\n");
            break;
        case kUnion_PathOp:
            SkDebugf("op union\n");
            break;
        case kXOR_PathOp:
            SkDebugf("op xor\n");
            break;
        case kReverseDifference_PathOp:
            SkDebugf("op reverse difference\n");
            break;
        default:
            SkASSERT(0);
    }
}

#if DEBUG_SHOW_TEST_NAME

void ShowFunctionHeader(const char* functionName) {
    SkDebugf("\nstatic void %s(skiatest::Reporter* reporter) {\n", functionName);
    if (strcmp("skphealth_com76", functionName) == 0) {
        SkDebugf("found it\n");
    }
}

static const char* gOpStrs[] = {
    "kDifference_PathOp",
    "kIntersect_PathOp",
    "kUnion_PathOp",
    "kXor_PathOp",
    "kReverseDifference_PathOp",
};

void ShowOp(SkPathOp op, const char* pathOne, const char* pathTwo) {
    SkDebugf("    testPathOp(reporter, %s, %s, %s);\n", pathOne, pathTwo, gOpStrs[op]);
    SkDebugf("}\n");
}
#endif

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
        bits.setConfig(SkBitmap::kARGB_8888_Config, bitWidth * 2, bitHeight);
        bits.allocPixels();
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
    if (errors2 >= 6 || errors > 160) {
        SkDebugf("%s errors2=%d errors=%d\n", __FUNCTION__, errors2, errors);
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

static void showSimplifiedPath(const SkPath& one, const SkPath& two,
        const SkPath& scaledOne, const SkPath& scaledTwo) {
    showPath(one, "path", false);
    drawAsciiPaths(scaledOne, scaledTwo, true);
}

static int comparePaths(skiatest::Reporter* reporter, const SkPath& one, const SkPath& two,
                 SkBitmap& bitmap) {
    int errors2x2;
    SkPath scaledOne, scaledTwo;
    int errors = pathsDrawTheSame(one, two, bitmap, scaledOne, scaledTwo, errors2x2);
    if (errors2x2 == 0) {
        return 0;
    }
    const int MAX_ERRORS = 9;
    if (errors2x2 == MAX_ERRORS || errors2x2 == MAX_ERRORS - 1) {
        showSimplifiedPath(one, two, scaledOne, scaledTwo);
    }
    if (errors2x2 > MAX_ERRORS && gComparePathsAssert) {
        SkDebugf("%s errors=%d\n", __FUNCTION__, errors);
        showSimplifiedPath(one, two, scaledOne, scaledTwo);
        REPORTER_ASSERT(reporter, 0);
    }
    return errors2x2 > MAX_ERRORS ? errors2x2 : 0;
}

static void showPathOpPath(const SkPath& one, const SkPath& two, const SkPath& a, const SkPath& b,
        const SkPath& scaledOne, const SkPath& scaledTwo, const SkPathOp shapeOp,
        const SkMatrix& scale) {
    SkASSERT((unsigned) shapeOp < SK_ARRAY_COUNT(opStrs));
    SkDebugf("static void xOp#%s(skiatest::Reporter* reporter) {\n", opSuffixes[shapeOp]);
    SkDebugf("    SkPath path, pathB;\n");
    showPath(a, "path", false);
    showPath(b, "pathB", false);
    SkDebugf("    testPathOp(reporter, path, pathB, %s);\n", opStrs[shapeOp]);
    SkDebugf("}\n");
    drawAsciiPaths(scaledOne, scaledTwo, true);
}

static int comparePaths(skiatest::Reporter* reporter, const SkPath& one, const SkPath& scaledOne,
                        const SkPath& two, const SkPath& scaledTwo, SkBitmap& bitmap,
                        const SkPath& a, const SkPath& b, const SkPathOp shapeOp,
                        const SkMatrix& scale) {
    int errors2x2;
    int errors = pathsDrawTheSame(bitmap, scaledOne, scaledTwo, errors2x2);
    if (errors2x2 == 0) {
        if (gShowPath) {
            showPathOpPath(one, two, a, b, scaledOne, scaledTwo, shapeOp, scale);
        }
        return 0;
    }
    const int MAX_ERRORS = 8;
    if (gShowPath || errors2x2 == MAX_ERRORS || errors2x2 == MAX_ERRORS - 1) {
        showPathOpPath(one, two, a, b, scaledOne, scaledTwo, shapeOp, scale);
    }
    if (errors2x2 > MAX_ERRORS && gComparePathsAssert) {
        SkDebugf("%s errors=%d\n", __FUNCTION__, errors);
        showPathOpPath(one, two, a, b, scaledOne, scaledTwo, shapeOp, scale);
        REPORTER_ASSERT(reporter, 0);
    }
    return errors2x2 > MAX_ERRORS ? errors2x2 : 0;
}

// Default values for when reporter->verbose() is false.
static int testNumber = 1;
static const char* testName = "pathOpTest";

static void writeTestName(const char* nameSuffix, SkMemoryWStream& outFile) {
    outFile.writeText(testName);
    outFile.writeDecAsText(testNumber);
    if (nameSuffix) {
        outFile.writeText(nameSuffix);
    }
}

static void outputToStream(const char* pathStr, const char* pathPrefix, const char* nameSuffix,
        const char* testFunction, bool twoPaths, SkMemoryWStream& outFile) {
    outFile.writeText("<div id=\"");
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

    outFile.writeText("static void ");
    writeTestName(nameSuffix, outFile);
    outFile.writeText("() {\n    SkPath path");
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
    outFile.flush();
}

bool testSimplify(SkPath& path, bool useXor, SkPath& out, PathOpsThreadState& state,
                  const char* pathStr) {
    SkPath::FillType fillType = useXor ? SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType;
    path.setFillType(fillType);
    if (gShowPath) {
        showPath(path, "path", false);
    }
    if (!Simplify(path, &out)) {
        SkDebugf("%s did not expect failure\n", __FUNCTION__);
        REPORTER_ASSERT(state.fReporter, 0);
        return false;
    }
    if (!state.fReporter->verbose()) {
        return true;
    }
    int result = comparePaths(state.fReporter, path, out, *state.fBitmap);
    if (result && gPathStrAssert) {
        char temp[8192];
        sk_bzero(temp, sizeof(temp));
        SkMemoryWStream stream(temp, sizeof(temp));
        const char* pathPrefix = NULL;
        const char* nameSuffix = NULL;
        if (fillType == SkPath::kEvenOdd_FillType) {
            pathPrefix = "    path.setFillType(SkPath::kEvenOdd_FillType);\n";
            nameSuffix = "x";
        }
        const char testFunction[] = "testSimplifyx(path);";
        outputToStream(pathStr, pathPrefix, nameSuffix, testFunction, false, stream);
        SkDebugf(temp);
        REPORTER_ASSERT(state.fReporter, 0);
    }
    state.fReporter->bumpTestCount();
    return result == 0;
}

bool testSimplify(skiatest::Reporter* reporter, const SkPath& path) {
#if DEBUG_SHOW_TEST_NAME
    showPathData(path);
#endif
    SkPath out;
    if (!Simplify(path, &out)) {
        SkDebugf("%s did not expect failure\n", __FUNCTION__);
        REPORTER_ASSERT(reporter, 0);
        return false;
    }
    SkBitmap bitmap;
    int result = comparePaths(reporter, path, out, bitmap);
    if (result && gPathStrAssert) {
        REPORTER_ASSERT(reporter, 0);
    }
    reporter->bumpTestCount();
    return result == 0;
}

#if DEBUG_SHOW_TEST_NAME
void SkPathOpsDebug::ShowPath(const SkPath& a, const SkPath& b, SkPathOp shapeOp,
        const char* testName) {
    ShowFunctionHeader(testName);
    showPath(a, "path", true);
    showPath(b, "pathB", true);
    ShowOp(shapeOp, "path", "pathB");
}
#endif

static bool innerPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
                 const SkPathOp shapeOp, const char* testName, bool threaded) {
#if DEBUG_SHOW_TEST_NAME
    if (testName == NULL) {
        SkDebugf("\n");
        showPathData(a);
        showOp(shapeOp);
        showPathData(b);
    } else {
        SkPathOpsDebug::ShowPath(a, b, shapeOp, testName);
    }
#endif
    SkPath out;
    if (!Op(a, b, shapeOp, &out) ) {
        SkDebugf("%s did not expect failure\n", __FUNCTION__);
        REPORTER_ASSERT(reporter, 0);
        return false;
    }
    if (threaded && !reporter->verbose()) {
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
    int result = comparePaths(reporter, pathOut, scaledPathOut, out, scaledOut, bitmap, a, b,
                              shapeOp, scale);
    if (result && gPathStrAssert) {
        REPORTER_ASSERT(reporter, 0);
    }
    reporter->bumpTestCount();
    return result == 0;
}

bool testPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
                 const SkPathOp shapeOp, const char* testName) {
    return innerPathOp(reporter, a, b, shapeOp, testName, false);
}

bool testThreadedPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
                 const SkPathOp shapeOp, const char* testName) {
    return innerPathOp(reporter, a, b, shapeOp, testName, true);
}

SK_DECLARE_STATIC_MUTEX(gMutex);

int initializeTests(skiatest::Reporter* reporter, const char* test) {
#if 0  // doesn't work yet
    SK_CONF_SET("images.jpeg.suppressDecoderWarnings", true);
    SK_CONF_SET("images.png.suppressDecoderWarnings", true);
#endif
#ifdef SK_DEBUG
    SkPathOpsDebug::gMaxWindSum = 4;
    SkPathOpsDebug::gMaxWindValue = 4;
#endif
    if (reporter->verbose()) {
        SkAutoMutexAcquire lock(gMutex);
        testName = test;
        size_t testNameSize = strlen(test);
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
    }
    return reporter->allowThreaded() ? SkThreadPool::kThreadPerCore : 1;
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
                void (*firstTest)(skiatest::Reporter* ),
                void (*stopTest)(skiatest::Reporter* ), bool reverse) {
    size_t index;
    if (firstTest) {
        index = count - 1;
        while (index > 0 && tests[index].fun != firstTest) {
            --index;
        }
#if DEBUG_SHOW_TEST_NAME
            SkDebugf("<div id=\"%s\">\n", tests[index].str);
            SkDebugf("  %s [%s]\n", __FUNCTION__, tests[index].str);
#endif
        (*tests[index].fun)(reporter);
    }
    index = reverse ? count - 1 : 0;
    size_t last = reverse ? 0 : count - 1;
    do {
        if (tests[index].fun != firstTest) {
    #if DEBUG_SHOW_TEST_NAME
            SkDebugf("<div id=\"%s\">\n", tests[index].str);
            SkDebugf("  %s [%s]\n", __FUNCTION__, tests[index].str);
    #endif
            (*tests[index].fun)(reporter);
        }
        if (tests[index].fun == stopTest) {
            SkDebugf("lastTest\n");
        }
        if (index == last) {
            break;
        }
        index += reverse ? -1 : 1;
    } while (true);
}
