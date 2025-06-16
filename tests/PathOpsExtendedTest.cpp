/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTDArray.h"
#include "include/utils/SkParsePath.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkPathPriv.h"
#include "src/pathops/SkPathOpsDebug.h"
#include "tests/PathOpsDebug.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

std::vector<std::string> gUniqueNames;

#ifdef SK_BUILD_FOR_MAC
#include <sys/sysctl.h>
#endif

// std::to_string isn't implemented on android
#include <sstream>

template <typename T>
std::string std_to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

bool OpDebug(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result
             SkDEBUGPARAMS(bool skipAssert)
             SkDEBUGPARAMS(const char* testName));

bool SimplifyDebug(const SkPath& one, SkPath* result
                   SkDEBUGPARAMS(bool skipAssert)
                   SkDEBUGPARAMS(const char* testName));

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
    "kXOR_PathOp",
    "kReverseDifference_SkPathOp",
};

static const char* opSuffixes[] = {
    "d",
    "i",
    "u",
    "o",
    "r",
};

enum class ExpectSuccess {
    kNo,
    kYes,
    kFlaky
};

enum class SkipAssert {
    kNo,
    kYes
};

enum class ExpectMatch {
    kNo,
    kYes,
    kFlaky
};

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

const int kBitWidth = 64;
const int kBitHeight = 64;

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
    SkScalar hScale = (kBitWidth - 2) / largerWidth;
    SkScalar vScale = (kBitHeight - 2) / largerHeight;
    scale.reset();
    scale.preScale(hScale, vScale);
    larger.fLeft *= hScale;
    larger.fRight *= hScale;
    larger.fTop *= vScale;
    larger.fBottom *= vScale;
    SkScalar dx = -16000 > larger.fLeft ? -16000 - larger.fLeft
            : 16000 < larger.fRight ? 16000 - larger.fRight : 0;
    SkScalar dy = -16000 > larger.fTop ? -16000 - larger.fTop
            : 16000 < larger.fBottom ? 16000 - larger.fBottom : 0;
    scale.postTranslate(dx, dy);
}

static int pathsDrawTheSame(SkBitmap& bits, const SkPath& scaledOne, const SkPath& scaledTwo,
        int& error2x2) {
    if (bits.width() == 0) {
        bits.allocN32Pixels(kBitWidth * 2, kBitHeight);
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
    canvas.translate(-bounds1.fLeft + 1 + kBitWidth, -bounds1.fTop + 1);
    canvas.drawPath(scaledTwo, paint);
    canvas.restore();
    int errors2 = 0;
    int errors = 0;
    for (int y = 0; y < kBitHeight - 1; ++y) {
        uint32_t* addr1 = bits.getAddr32(0, y);
        uint32_t* addr2 = bits.getAddr32(0, y + 1);
        uint32_t* addr3 = bits.getAddr32(kBitWidth, y);
        uint32_t* addr4 = bits.getAddr32(kBitWidth, y + 1);
        for (int x = 0; x < kBitWidth - 1; ++x) {
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

static SkTDArray<SkPathOp> gTestOp;

static void showPathOpPath(const char* testName, const SkPath& one, const SkPath& two,
        const SkPath& a, const SkPath& b, const SkPath& scaledOne, const SkPath& scaledTwo,
        const SkPathOp shapeOp, const SkMatrix& scale) {
    SkASSERT((unsigned) shapeOp < std::size(opStrs));
    if (!testName) {
        testName = "xOp";
    }
    SkDebugf("static void %s_%s(skiatest::Reporter* reporter, const char* filename) {\n",
        testName, opSuffixes[shapeOp]);
    *gTestOp.append() = shapeOp;
    SkDebugf("    SkPath path, pathB;\n");
    SkPathOpsDebug::ShowOnePath(a, "path", false);
    SkPathOpsDebug::ShowOnePath(b, "pathB", false);
    SkDebugf("    testPathOp(reporter, path, pathB, %s, filename);\n", opStrs[shapeOp]);
    SkDebugf("}\n");
    drawAsciiPaths(scaledOne, scaledTwo, true);
}

static int comparePaths(skiatest::Reporter* reporter, const char* testName, const SkPath& one,
        const SkPath& scaledOne, const SkPath& two, const SkPath& scaledTwo, SkBitmap& bitmap,
        const SkPath& a, const SkPath& b, const SkPathOp shapeOp, const SkMatrix& scale,
        ExpectMatch expectMatch) {
    static SkMutex& compareDebugOut3 = *(new SkMutex);
    int errors2x2;
    const int MAX_ERRORS = 8;
    (void) pathsDrawTheSame(bitmap, scaledOne, scaledTwo, errors2x2);
    if (ExpectMatch::kNo == expectMatch) {
        if (errors2x2 < MAX_ERRORS) {
            REPORTER_ASSERT(reporter, 0);
        }
        return 0;
    }
    if (errors2x2 == 0) {
        return 0;
    }
    if (ExpectMatch::kYes == expectMatch && errors2x2 >= MAX_ERRORS) {
        SkAutoMutexExclusive autoM(compareDebugOut3);
        showPathOpPath(testName, one, two, a, b, scaledOne, scaledTwo, shapeOp, scale);
        SkDebugf("\n/*");
        REPORTER_ASSERT(reporter, 0);
        SkDebugf(" */\n");
    }
    return errors2x2 >= MAX_ERRORS ? errors2x2 : 0;
}

// Default values for when reporter->verbose() is false.
static int sTestNumber = 55;
static const char* sTestName = "pathOpTest";

static void appendTestName(const char* nameSuffix, std::string& out) {
    out += sTestName;
    out += std_to_string(sTestNumber);
    ++sTestNumber;
    if (nameSuffix) {
        out.append(nameSuffix);
    }
}

static void appendTest(const char* pathStr, const char* pathPrefix, const char* nameSuffix,
                       const char* testFunction, bool twoPaths, std::string& out) {
#if 0
    out.append("\n<div id=\"");
    appendTestName(nameSuffix, out);
    out.append("\">\n");
    if (pathPrefix) {
        out.append(pathPrefix);
    }
    out.append(pathStr);
    out.append("</div>\n\n");

    out.append(marker);
    out.append("    ");
    appendTestName(nameSuffix, out);
    out.append(",\n\n\n");
#endif
    out.append("static void ");
    appendTestName(nameSuffix, out);
    out.append("(skiatest::Reporter* reporter) {\n    SkPath path");
    if (twoPaths) {
        out.append(", pathB");
    }
    out.append(";\n");
    if (pathPrefix) {
        out.append(pathPrefix);
    }
    out += pathStr;
    out += "    ";
    out += testFunction;
#if 0
    out.append("static void (*firstTest)() = ");
    appendTestName(nameSuffix, out);
    out.append(";\n\n");

    out.append("static struct {\n");
    out.append("    void (*fun)();\n");
    out.append("    const char* str;\n");
    out.append("} tests[] = {\n");
    out.append("    TEST(");
    appendTestName(nameSuffix, out);
    out.append("),\n");
#endif
}

void markTestFlakyForPathKit() {
    if (PathOpsDebug::gJson) {
        SkASSERT(!PathOpsDebug::gMarkJsonFlaky);
        PathOpsDebug::gMarkJsonFlaky = true;
    }
}

bool testSimplify(SkPath& path, bool useXor, SkPath& out, PathOpsThreadState& state,
                  const char* pathStr) {
    static SkMutex& simplifyDebugOut = *(new SkMutex);
    SkPathFillType fillType = useXor ? SkPathFillType::kEvenOdd : SkPathFillType::kWinding;
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
    int result = comparePaths(state.fReporter, nullptr, path, out, *state.fBitmap);
    if (result) {
        SkAutoMutexExclusive autoM(simplifyDebugOut);
        std::string str;
        const char* pathPrefix = nullptr;
        const char* nameSuffix = nullptr;
        if (fillType == SkPathFillType::kEvenOdd) {
            pathPrefix = "    path.setFillType(SkPathFillType::kEvenOdd);\n";
            nameSuffix = "x";
        }
        const char testFunction[] = "testSimplify(reporter, path);";
        appendTest(pathStr, pathPrefix, nameSuffix, testFunction, false, str);
        SkDebugf("%s", str.c_str());
        REPORTER_ASSERT(state.fReporter, 0);
    }
    state.fReporter->bumpTestCount();
    return result == 0;
}

static void json_status(ExpectSuccess expectSuccess, ExpectMatch expectMatch, bool opSucceeded) {
    fprintf(PathOpsDebug::gOut, "  \"expectSuccess\": \"%s\",\n",
            ExpectSuccess::kNo == expectSuccess ? "no" :
            ExpectSuccess::kYes == expectSuccess ? "yes" : "flaky");
    if (PathOpsDebug::gMarkJsonFlaky) {
        expectMatch = ExpectMatch::kFlaky;
        PathOpsDebug::gMarkJsonFlaky = false;
    }
    fprintf(PathOpsDebug::gOut, "  \"expectMatch\": \"%s\",\n",
            ExpectMatch::kNo == expectMatch ? "no" :
            ExpectMatch::kYes == expectMatch ? "yes" : "flaky");
    fprintf(PathOpsDebug::gOut, "  \"succeeded\": %s,\n", opSucceeded ? "true" : "false");
}

static void json_path_out(const SkPath& path, const char* pathName, const char* fillTypeName,
        bool lastField) {
    char const * const gFillTypeStrs[] = {
        "Winding",
        "EvenOdd",
        "InverseWinding",
        "InverseEvenOdd",
    };
    if (PathOpsDebug::gOutputSVG) {
        SkString svg = SkParsePath::ToSVGString(path);
        fprintf(PathOpsDebug::gOut, "  \"%s\": \"%s\",\n", pathName, svg.c_str());
    } else {
                                 // MOVE, LINE, QUAD, CONIC, CUBIC, CLOSE
        const int verbConst[] =  {     0,    1,    2,     3,     4,     5 };
        const int pointIndex[] = {     0,    1,    1,     1,     1,     0 };
        const int pointCount[] = {     1,    2,    3,     3,     4,     0 };
        fprintf(PathOpsDebug::gOut, "  \"%s\": [", pathName);
        bool first = true;
        for (auto [verb, points, w] : SkPathPriv::Iterate(path)) {
            if (first) {
                first = false;
            } else {
                fprintf(PathOpsDebug::gOut, ",\n    ");
            }
            int verbIndex = (int) verb;
            fprintf(PathOpsDebug::gOut, "[%d", verbConst[verbIndex]);
            for (int i = pointIndex[verbIndex]; i < pointCount[verbIndex]; ++i) {
                fprintf(PathOpsDebug::gOut, ", \"0x%08x\", \"0x%08x\"",
                        SkFloat2Bits(points[i].fX), SkFloat2Bits(points[i].fY));
            }
            if (SkPathVerb::kConic == verb) {
                fprintf(PathOpsDebug::gOut, ", \"0x%08x\"", SkFloat2Bits(*w));
            }
            fprintf(PathOpsDebug::gOut, "]");
        }
        fprintf(PathOpsDebug::gOut, "],\n");
    }
    fprintf(PathOpsDebug::gOut, "  \"fillType%s\": \"k%s_FillType\"%s", fillTypeName,
            gFillTypeStrs[(int) path.getFillType()], lastField ? "\n}" : ",\n");
}

static bool check_for_duplicate_names(const char* testName) {
    if (PathOpsDebug::gCheckForDuplicateNames) {
        if (gUniqueNames.end() != std::find(gUniqueNames.begin(), gUniqueNames.end(),
                std::string(testName))) {
            SkDebugf("%s", "");  // convenience for setting breakpoints
        }
        gUniqueNames.push_back(std::string(testName));
        return true;
    }
    return false;
}

static bool inner_simplify(skiatest::Reporter* reporter, const SkPath& path, const char* filename,
        ExpectSuccess expectSuccess, SkipAssert skipAssert, ExpectMatch expectMatch) {
    if (PathOpsDebug::gJson) {
        if (check_for_duplicate_names(filename)) {
            return true;
        }
        if (!PathOpsDebug::gOutFirst) {
            fprintf(PathOpsDebug::gOut, ",\n");
        }
        PathOpsDebug::gOutFirst = false;
        fprintf(PathOpsDebug::gOut, "\"%s\": {\n", filename);
        json_path_out(path, "path", "", false);
    }
    SkPath out;
    if (!SimplifyDebug(path, &out  SkDEBUGPARAMS(SkipAssert::kYes == skipAssert)
            SkDEBUGPARAMS(sTestName))) {
        if (ExpectSuccess::kYes == expectSuccess) {
            SkDebugf("%s did not expect %s failure\n", __FUNCTION__, filename);
            REPORTER_ASSERT(reporter, 0);
        }
        if (PathOpsDebug::gJson) {
            json_status(expectSuccess, expectMatch, false);
            fprintf(PathOpsDebug::gOut, "  \"out\": \"\"\n}");
        }
        return false;
    } else {
        if (ExpectSuccess::kNo == expectSuccess) {
            SkDebugf("%s %s unexpected success\n", __FUNCTION__, filename);
            REPORTER_ASSERT(reporter, 0);
        }
        if (PathOpsDebug::gJson) {
            json_status(expectSuccess, expectMatch, true);
            json_path_out(out, "out", "Out", true);
        }
    }
    SkBitmap bitmap;
    int errors = comparePaths(reporter, filename, path, out, bitmap);
    if (ExpectMatch::kNo == expectMatch) {
        if (!errors) {
            SkDebugf("%s failing test %s now succeeds\n", __FUNCTION__, filename);
            REPORTER_ASSERT(reporter, 0);
            return false;
        }
    } else if (ExpectMatch::kYes == expectMatch && errors) {
        REPORTER_ASSERT(reporter, 0);
    }
    reporter->bumpTestCount();
    return errors == 0;
}

bool testSimplify(skiatest::Reporter* reporter, const SkPath& path, const char* filename) {
    return inner_simplify(reporter, path, filename, ExpectSuccess::kYes, SkipAssert::kNo,
            ExpectMatch::kYes);
}

bool testSimplifyFuzz(skiatest::Reporter* reporter, const SkPath& path, const char* filename) {
    return inner_simplify(reporter, path, filename, ExpectSuccess::kFlaky, SkipAssert::kYes,
            ExpectMatch::kFlaky);
}

bool testSimplifyCheck(skiatest::Reporter* reporter, const SkPath& path, const char* filename,
        bool checkFail) {
    return inner_simplify(reporter, path, filename, checkFail ?
            ExpectSuccess::kYes : ExpectSuccess::kNo, SkipAssert::kNo, ExpectMatch::kNo);
}

bool testSimplifyFail(skiatest::Reporter* reporter, const SkPath& path, const char* filename) {
    return inner_simplify(reporter, path, filename,
            ExpectSuccess::kNo, SkipAssert::kYes, ExpectMatch::kNo);
}

static bool innerPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName, ExpectSuccess expectSuccess,
        SkipAssert skipAssert, ExpectMatch expectMatch) {
    if (PathOpsDebug::gJson) {
        if (check_for_duplicate_names(testName)) {
            return true;
        }
        if (!PathOpsDebug::gOutFirst) {
            fprintf(PathOpsDebug::gOut, ",\n");
        }
        PathOpsDebug::gOutFirst = false;
        fprintf(PathOpsDebug::gOut, "\"%s\": {\n", testName);
        json_path_out(a, "p1", "1", false);
        json_path_out(b, "p2", "2", false);
        fprintf(PathOpsDebug::gOut, "  \"op\": \"%s\",\n", opStrs[shapeOp]);
    }
    SkPath out;
    if (!OpDebug(a, b, shapeOp, &out  SkDEBUGPARAMS(SkipAssert::kYes == skipAssert)
            SkDEBUGPARAMS(testName))) {
        if (ExpectSuccess::kYes == expectSuccess) {
            SkDebugf("%s %s did not expect failure\n", __FUNCTION__, testName);
            REPORTER_ASSERT(reporter, 0);
        }
        if (PathOpsDebug::gJson) {
            json_status(expectSuccess, expectMatch, false);
            fprintf(PathOpsDebug::gOut, "  \"out\": \"\"\n}");
        }
        return false;
    } else {
        if (ExpectSuccess::kNo == expectSuccess) {
                SkDebugf("%s %s unexpected success\n", __FUNCTION__, testName);
                REPORTER_ASSERT(reporter, 0);
        }
        if (PathOpsDebug::gJson) {
            json_status(expectSuccess, expectMatch, true);
            json_path_out(out, "out", "Out", true);
        }
    }
    if (!reporter->verbose()) {
        return true;
    }
    SkPath pathOut, scaledPathOut;
    SkRegion rgnA, rgnB, openClip, rgnOut;
    openClip.setRect({-16000, -16000, 16000, 16000});
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
            a, b, shapeOp, scale, expectMatch);
    reporter->bumpTestCount();
    return result == 0;
}

bool testPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName) {
    return innerPathOp(reporter, a, b, shapeOp, testName, ExpectSuccess::kYes, SkipAssert::kNo,
            ExpectMatch::kYes);
}

bool testPathOpCheck(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName, bool checkFail) {
    return innerPathOp(reporter, a, b, shapeOp, testName, checkFail ?
            ExpectSuccess::kYes : ExpectSuccess::kNo, SkipAssert::kNo, ExpectMatch::kNo);
}

bool testPathOpFuzz(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
        const SkPathOp shapeOp, const char* testName) {
    return innerPathOp(reporter, a, b, shapeOp, testName, ExpectSuccess::kFlaky, SkipAssert::kYes,
            ExpectMatch::kFlaky);
}

bool testPathOpFail(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
                 const SkPathOp shapeOp, const char* testName) {
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

void initializeTests(skiatest::Reporter* reporter, const char* test) {
    static SkMutex& mu = *(new SkMutex);
    if (reporter->verbose()) {
        SkAutoMutexExclusive lock(mu);
        sTestName = test;
        size_t testNameSize = strlen(test);
        SkFILEStream inFile("../../experimental/Intersection/op.htm");
        if (inFile.isValid()) {
            SkTDArray<char> inData;
            inData.resize((int) inFile.getLength());
            size_t inLen = inData.size();
            inFile.read(inData.begin(), inLen);
            inFile.close();
            char* insert = strstr(inData.begin(), marker);
            if (insert) {
                insert += sizeof(marker) - 1;
                const char* numLoc = insert + 4 /* indent spaces */ + testNameSize - 1;
                sTestNumber = atoi(numLoc) + 1;
            }
        }
    }
}

void PathOpsThreadState::outputProgress(const char* pathStr, SkPathFillType pathFillType) {
    const char testFunction[] = "testSimplify(path);";
    const char* pathPrefix = nullptr;
    const char* nameSuffix = nullptr;
    if (pathFillType == SkPathFillType::kEvenOdd) {
        pathPrefix = "    path.setFillType(SkPathFillType::kEvenOdd);\n";
        nameSuffix = "x";
    }
    appendTest(pathStr, pathPrefix, nameSuffix, testFunction, false, fPathStr);
}

void PathOpsThreadState::outputProgress(const char* pathStr, SkPathOp op) {
    const char testFunction[] = "testOp(path);";
    SkASSERT((size_t) op < std::size(opSuffixes));
    const char* nameSuffix = opSuffixes[op];
    appendTest(pathStr, nullptr, nameSuffix, testFunction, true, fPathStr);
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
             (*tests[index].fun)(reporter, tests[index].str);
        }
        if (tests[index].fun == stopTest || index == last) {
            break;
        }
        index += reverse ? -1 : 1;
    } while (true);
}
