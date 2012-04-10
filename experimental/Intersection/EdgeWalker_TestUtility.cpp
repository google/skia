#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include <algorithm>

static bool gShowPath = false;
static bool gComparePaths = true;
static bool gDrawLastAsciiPaths = true;
static bool gDrawAllAsciiPaths = false;
static bool gShowAsciiPaths = false;
static bool gComparePathsAssert = true;

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
        SkBitmap& bits, SkCanvas* c) {
    SkCanvas* canvasPtr = c;
    if (!c) {
        canvasPtr = new SkCanvas(bits);
    }
    const SkRect& bounds1 = one.getBounds();
    const SkRect& bounds2 = two.getBounds();
    SkRect larger = bounds1;
    larger.join(bounds2);
    int bitWidth = SkScalarCeil(larger.width()) + 2;
    int bitHeight = SkScalarCeil(larger.height()) + 2;
    if (bits.width() < bitWidth * 2 || bits.height() < bitHeight) {
        if (bits.width() >= 200) {
            SkDebugf("%s bitWidth=%d bitHeight=%d\n", __FUNCTION__, bitWidth, bitHeight);
        }
        bits.setConfig(SkBitmap::kARGB_8888_Config, bitWidth * 2, bitHeight);
        bits.allocPixels();
        canvasPtr->setBitmapDevice(bits);
    }
    SkCanvas& canvas = *canvasPtr;
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
    int errors = 0;
    for (int y = 0; y < bitHeight; ++y) {
        uint32_t* addr1 = bits.getAddr32(0, y);
        uint32_t* addr2 = bits.getAddr32(bitWidth, y);
        for (int x = 0; x < bitWidth; ++x) {
            errors += addr1[x] != addr2[x];
        }
    }
    if (!c) {
        delete canvasPtr;
    }
    return errors;
}

void bitmapInit(SkBitmap& bits) {
}

bool drawAsciiPaths(const SkPath& one, const SkPath& two,
        bool drawPaths) {
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

static int scaledDrawTheSame(const SkPath& one, const SkPath& two,
        int a, int b, bool drawPaths, SkBitmap& bitmap, SkCanvas* canvas) {
    SkMatrix scale;
    scale.reset();
    float aScale = 1.21f;
    float bScale = 1.11f;
    scale.preScale(a * aScale, b * bScale);
    SkPath scaledOne, scaledTwo;
    one.transform(scale, &scaledOne);
    two.transform(scale, &scaledTwo);
    int errors = pathsDrawTheSame(scaledOne, scaledTwo, bitmap, canvas);
    if (errors == 0) {
        return 0;
    }
    while (!drawAsciiPaths(scaledOne, scaledTwo, drawPaths)) {
        scale.reset();
        aScale *= 0.5f;
        bScale *= 0.5f;
        scale.preScale(a * aScale, b * bScale);
        one.transform(scale, &scaledOne);
        two.transform(scale, &scaledTwo);
    }
    return errors;
}

static int comparePaths(const SkPath& one, const SkPath& two, SkBitmap& bitmap,
        SkCanvas* canvas) {
    int errors = pathsDrawTheSame(one, two, bitmap, canvas);
    if (errors == 0) {
        return 0;
    }
    drawAsciiPaths(one, two, gDrawAllAsciiPaths);
    for (int x = 9; x <= 33; ++x) {
        errors = scaledDrawTheSame(one, two, x, x - (x >> 2), gDrawAllAsciiPaths,
            bitmap, canvas);
        if (errors == 0) {
            return 0;
        }
    }
    if (!gDrawAllAsciiPaths) {
        const SkRect& bounds1 = one.getBounds();
        const SkRect& bounds2 = two.getBounds();
        SkRect larger = bounds1;
        larger.join(bounds2);
        SkScalar xScale = std::max(80.0f / larger.width(), 1.0f);
        SkScalar yScale = std::max(60.0f / larger.height(), 1.0f);
        errors = scaledDrawTheSame(one, two, xScale, yScale, false, bitmap, canvas);
        if (errors > 8) {
            scaledDrawTheSame(one, two, xScale, yScale, true, bitmap, canvas);
        }
    }
    if (errors > 0) SkDebugf("\n%s errors=%d\n", __FUNCTION__, errors); 
    if (errors > 31 && gComparePathsAssert) {
        showPath(one);
        showPath(two, "simplified:");
        SkASSERT(0);
    }
    return errors;
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
