#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPaint.h"

static bool gDrawLastAsciiPaths = true;
static bool gDrawAllAsciiPaths = false;
static bool gShowPath = true;

static void showPath(const char* str, const SkPath& path) {
    if (!gShowPath) {
        return;
    }
    SkDebugf("%s\n", str);
    SkPath::Iter iter(path, true);
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("path.moveTo(%g, %g);\n", pts[0].fX, pts[0].fY);
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("path.lineTo(%g, %g);\n", pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("path.quadTo(%g, %g, %g, %g);\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("path.cubicTo(%g, %g, %g, %g);\n",
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

static bool pathsDrawTheSame(const SkPath& one, const SkPath& two) {
    const SkRect& bounds1 = one.getBounds();
    const SkRect& bounds2 = two.getBounds();
    SkRect larger = bounds1;
    larger.join(bounds2);
    SkBitmap bits;
    int bitWidth = SkScalarCeil(larger.width()) + 2;
    int bitHeight = SkScalarCeil(larger.height()) + 2;
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
        uint32_t* addr2 = bits.getAddr32(bitWidth, y);
        for (int x = 0; x < bitWidth; ++x) {
            if (addr1[x] != addr2[x]) {
                return false;
                break;
            }
        }
    }
    return true;
}

static void drawAsciiPaths(const SkPath& one, const SkPath& two,
        bool drawPaths) {
    if (!drawPaths) {
        return;
    }
    if (0) {
        showPath("one:", one);
        showPath("two:", two);
    }
    const SkRect& bounds1 = one.getBounds();
    const SkRect& bounds2 = two.getBounds();
    SkRect larger = bounds1;
    larger.join(bounds2);
    SkBitmap bits;
    int bitWidth = SkScalarCeil(larger.width()) + 2;
    int bitHeight = SkScalarCeil(larger.height()) + 2;
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
    canvas.translate(-bounds2.fLeft + 1 + bitWidth, -bounds2.fTop + 1);
    canvas.drawPath(two, paint);
    canvas.restore();
    char out[1024];
    SkASSERT(bitWidth * 2 + 1 < (int) sizeof(out));
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
}

static bool scaledDrawTheSame(const SkPath& one, const SkPath& two,
        int a, int b, bool drawPaths) {
    SkMatrix scale;
    scale.reset();
    scale.preScale(a * 1.21f, b * 1.11f);
    SkPath scaledOne, scaledTwo;
    one.transform(scale, &scaledOne);
    two.transform(scale, &scaledTwo);
    if (pathsDrawTheSame(scaledOne, scaledTwo)) {
        return true;
    }
    drawAsciiPaths(scaledOne, scaledTwo, drawPaths);
    return false;
}

void comparePaths(const SkPath& one, const SkPath& two) {
    if (pathsDrawTheSame(one, two)) {
        return;
    }
    drawAsciiPaths(one, two, gDrawAllAsciiPaths);
    for (int x = 9; x <= 33; ++x) {
        if (scaledDrawTheSame(one, two, x, x - (x >> 2), gDrawAllAsciiPaths)) {
            return;
        }
    }
    if (!gDrawAllAsciiPaths) {
        scaledDrawTheSame(one, two, 9, 7, gDrawLastAsciiPaths);
    }
    showPath("original:", one);
    showPath("simplified:", two);
    SkASSERT(0);
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

