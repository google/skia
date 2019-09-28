/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkGeometry.h"
#include "src/pathops/SkIntersections.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

/*
manually compute the intersection of a pair of circles and see if the conic intersection matches
  given two circles
    construct a line connecting their centers

 */

static const ConicPts testSet[] = {
    {{{{306.588013,-227.983994}, {212.464996,-262.242004}, {95.5512009,58.9763985}}}, 0.707107008f},
    {{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},

    {{{{5.1114602088928223, 628.77813720703125},
        {10.834027290344238, 988.964111328125},
        {163.40835571289062, 988.964111328125}}}, 0.72944212f},
    {{{{163.40835571289062, 988.964111328125},
        {5, 988.964111328125},
        {5, 614.7423095703125}}}, 0.707106769f},

    {{{{11.17222976684570312, -8.103978157043457031},
        {22.91432571411132812, -10.37866020202636719},
        {23.7764129638671875, -7.725424289703369141}}}, 1.00862849f},
    {{{{-1.545085430145263672, -4.755282402038574219},
        {22.23132705688476562, -12.48070907592773438},
        {23.7764129638671875, -7.725427150726318359}}}, 0.707106769f},

    {{{{-4,1}, {-4,5}, {0,5}}}, 0.707106769f},
    {{{{-3,4}, {-3,1}, {0,1}}}, 0.707106769f},

    {{{{0, 0}, {0, 1}, {1, 1}}}, 0.5f},
    {{{{1, 0}, {0, 0}, {0, 1}}}, 0.5f},

};

const int testSetCount = (int) SK_ARRAY_COUNT(testSet);

static void chopCompare(const SkConic chopped[2], const SkDConic dChopped[2]) {
    SkASSERT(roughly_equal(chopped[0].fW, dChopped[0].fWeight));
    SkASSERT(roughly_equal(chopped[1].fW, dChopped[1].fWeight));
    for (int cIndex = 0; cIndex < 2; ++cIndex) {
        for (int pIndex = 0; pIndex < 3; ++pIndex) {
            SkDPoint up;
            up.set(chopped[cIndex].fPts[pIndex]);
            SkASSERT(dChopped[cIndex].fPts[pIndex].approximatelyEqual(up));
        }
    }
#if DEBUG_VISUALIZE_CONICS
    dChopped[0].dump();
    dChopped[1].dump();
#endif
}

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "src/pathops/SkPathOpsRect.h"

#define DEBUG_VISUALIZE_CONICS 0

#if DEBUG_VISUALIZE_CONICS
static void writePng(const SkConic& c, const SkConic ch[2], const char* name) {
    const int scale = 10;
    SkConic conic, chopped[2];
    for (int index = 0; index < 3; ++index) {
        conic.fPts[index].fX = c.fPts[index].fX * scale;
        conic.fPts[index].fY = c.fPts[index].fY * scale;
        for (int chIndex = 0; chIndex < 2; ++chIndex) {
            chopped[chIndex].fPts[index].fX = ch[chIndex].fPts[index].fX * scale;
            chopped[chIndex].fPts[index].fY = ch[chIndex].fPts[index].fY * scale;
        }
    }
    conic.fW = c.fW;
    chopped[0].fW = ch[0].fW;
    chopped[1].fW = ch[1].fW;
    SkBitmap bitmap;
    SkRect bounds;
    conic.computeTightBounds(&bounds);
    bounds.outset(10, 10);
    bitmap.tryAllocPixels(SkImageInfo::MakeN32Premul(
          SkScalarRoundToInt(bounds.width()), SkScalarRoundToInt(bounds.height())));
    SkCanvas canvas(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas.translate(-bounds.fLeft, -bounds.fTop);
    canvas.drawColor(SK_ColorWHITE);
    SkPath path;
    path.moveTo(conic.fPts[0]);
    path.conicTo(conic.fPts[1], conic.fPts[2], conic.fW);
    paint.setARGB(0x80, 0xFF, 0, 0);
    canvas.drawPath(path, paint);
    path.reset();
    path.moveTo(chopped[0].fPts[0]);
    path.conicTo(chopped[0].fPts[1], chopped[0].fPts[2], chopped[0].fW);
    path.moveTo(chopped[1].fPts[0]);
    path.conicTo(chopped[1].fPts[1], chopped[1].fPts[2], chopped[1].fW);
    paint.setARGB(0x80, 0, 0, 0xFF);
    canvas.drawPath(path, paint);
    SkString filename("c:\\Users\\caryclark\\Documents\\");
    filename.appendf("%s.png", name);
    ToolUtils::EncodeImageToFile(filename.c_str(), bitmap, SkEncodedImageFormat::kPNG, 100);
}

static void writeDPng(const SkDConic& dC, const char* name) {
    const int scale = 5;
    SkDConic dConic = {{{ {dC.fPts[0].fX * scale, dC.fPts[0].fY * scale },
        {dC.fPts[1].fX * scale, dC.fPts[1].fY * scale },
        {dC.fPts[2].fX * scale, dC.fPts[2].fY * scale }}}, dC.fWeight };
    SkBitmap bitmap;
    SkDRect bounds;
    bounds.setBounds(dConic);
    bounds.fLeft -= 10;
    bounds.fTop -= 10;
    bounds.fRight += 10;
    bounds.fBottom += 10;
    bitmap.tryAllocPixels(SkImageInfo::MakeN32Premul(
          SkScalarRoundToInt(SkDoubleToScalar(bounds.width())),
          SkScalarRoundToInt(SkDoubleToScalar(bounds.height()))));
    SkCanvas canvas(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas.translate(SkDoubleToScalar(-bounds.fLeft), SkDoubleToScalar(-bounds.fTop));
    canvas.drawColor(SK_ColorWHITE);
    SkPath path;
    path.moveTo(dConic.fPts[0].asSkPoint());
    path.conicTo(dConic.fPts[1].asSkPoint(), dConic.fPts[2].asSkPoint(), dConic.fWeight);
    paint.setARGB(0x80, 0xFF, 0, 0);
    canvas.drawPath(path, paint);
    path.reset();
    const int chops = 2;
    for (int tIndex = 0; tIndex < chops; ++tIndex) {
        SkDConic chopped = dConic.subDivide(tIndex / (double) chops,
                (tIndex + 1) / (double) chops);
        path.moveTo(chopped.fPts[0].asSkPoint());
        path.conicTo(chopped.fPts[1].asSkPoint(), chopped.fPts[2].asSkPoint(), chopped.fWeight);
    }
    paint.setARGB(0x80, 0, 0, 0xFF);
    canvas.drawPath(path, paint);
    SkString filename("c:\\Users\\caryclark\\Documents\\");
    filename.appendf("%s.png", name);
    ToolUtils::EncodeImageToFile(filename.c_str(), bitmap, SkEncodedImageFormat::kPNG, 100);
}
#endif

static void chopBothWays(const SkDConic& dConic, double t, const char* name) {
    SkConic conic;
    for (int index = 0; index < 3; ++index) {
        conic.fPts[index] = dConic.fPts[index].asSkPoint();
    }
    conic.fW = dConic.fWeight;
    SkConic chopped[2];
    SkDConic dChopped[2];
    if (!conic.chopAt(SkDoubleToScalar(t), chopped)) {
        return;
    }
    dChopped[0] = dConic.subDivide(0, t);
    dChopped[1] = dConic.subDivide(t, 1);
#if DEBUG_VISUALIZE_CONICS
    dConic.dump();
#endif
    chopCompare(chopped, dChopped);
#if DEBUG_VISUALIZE_CONICS
    writePng(conic, chopped, name);
#endif
}

#if DEBUG_VISUALIZE_CONICS
const SkDConic frame0[] = {
{{{{306.588013,-227.983994}, {212.464996,-262.242004}, {95.5512009,58.9763985}}}, 0.707107008f},
{{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},
};

const SkDConic frame1[] = {
{{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},
{{{{306.58801299999999, -227.983994}, {212.46499600000001, -262.24200400000001}, {95.551200899999998, 58.976398500000002}}}, 0.707107008f},
{{{{377.21899400000001, -141.98100299999999}, {237.77799285476553, -166.56830755921084}, {134.08399674208422, -155.06258330544892}}}, 0.788580656f},
{{{{134.08399674208422, -155.06258330544892}, {30.390000629402859, -143.55685905168704}, {23.185499199999999, -102.697998}}}, 0.923879623f},
};

const SkDConic frame2[] = {
{{{{306.588013,-227.983994}, {212.464996,-262.242004}, {95.5512009,58.9763985}}}, 0.707107008f},
{{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},
{{{{205.78973252799028, -158.12538713371103}, {143.97848953841861, -74.076645245042371}, {95.551200899999998, 58.976398500000002}}}, 0.923879623f},
{{{{377.21899400000001, -141.98100299999999}, {237.77799285476553, -166.56830755921084}, {134.08399674208422, -155.06258330544892}}}, 0.788580656f},
};

const SkDConic frame3[] = {
{{{{306.588013,-227.983994}, {212.464996,-262.242004}, {95.5512009,58.9763985}}}, 0.707107008f},
{{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},
{{{{205.78973252799028, -158.12538713371103}, {143.97848953841861, -74.076645245042371}, {95.551200899999998, 58.976398500000002}}}, 0.923879623f},
{{{{252.08225670812539, -156.90491625851064}, {185.93099479842493, -160.81544543232982}, {134.08399674208422, -155.06258330544892}}}, 0.835816324f},
};

const SkDConic frame4[] = {
{{{{306.588013,-227.983994}, {212.464996,-262.242004}, {95.5512009,58.9763985}}}, 0.707107008f},
{{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},
{{{{205.78973252799028, -158.12538713371103}, {174.88411103320448, -116.10101618937664}, {145.19509369736275, -56.857102571363754}}}, 0.871667147f},
{{{{252.08225670812539, -156.90491625851064}, {185.93099479842493, -160.81544543232982}, {134.08399674208422, -155.06258330544892}}}, 0.835816324f},
};

const SkDConic frame5[] = {
{{{{306.588013,-227.983994}, {212.464996,-262.242004}, {95.5512009,58.9763985}}}, 0.707107008f},
{{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},
{{{{205.78973252799028, -158.12538713371103}, {174.88411103320448, -116.10101618937664}, {145.19509369736275, -56.857102571363754}}}, 0.871667147f},
{{{{252.08225670812539, -156.90491625851064}, {219.70109133058406, -158.81912754088933}, {190.17095392508796, -158.38373974664466}}}, 0.858306944f},
};

const SkDConic frame6[] = {
{{{{306.588013,-227.983994}, {212.464996,-262.242004}, {95.5512009,58.9763985}}}, 0.707107008f},
{{{{377.218994,-141.981003}, {40.578701,-201.339996}, {23.1854992,-102.697998}}}, 0.707107008f},
{{{{205.78973252799028, -158.12538713371103}, {190.33692178059735, -137.11320166154385}, {174.87004877564593, -111.2132534799228}}}, 0.858117759f},
{{{{252.08225670812539, -156.90491625851064}, {219.70109133058406, -158.81912754088933}, {190.17095392508796, -158.38373974664466}}}, 0.858306944f},
};

const SkDConic* frames[] = {
    frame0, frame1, frame2, frame3, frame4, frame5, frame6
};

const int frameSizes[] = { (int) SK_ARRAY_COUNT(frame0), (int) SK_ARRAY_COUNT(frame1),
        (int) SK_ARRAY_COUNT(frame2), (int) SK_ARRAY_COUNT(frame3),
        (int) SK_ARRAY_COUNT(frame4), (int) SK_ARRAY_COUNT(frame5),
        (int) SK_ARRAY_COUNT(frame6),
};

static void writeFrames() {
    const int scale = 5;

    for (int index = 0; index < (int) SK_ARRAY_COUNT(frameSizes); ++index) {
        SkDRect bounds;
        bool boundsSet = false;
        int frameSize = frameSizes[index];
        for (int fIndex = 0; fIndex < frameSize; ++fIndex) {
            const SkDConic& dC = frames[index][fIndex];
            SkDConic dConic = {{{ {dC.fPts[0].fX * scale, dC.fPts[0].fY * scale },
                {dC.fPts[1].fX * scale, dC.fPts[1].fY * scale },
                {dC.fPts[2].fX * scale, dC.fPts[2].fY * scale }}}, dC.fWeight };
            SkDRect dBounds;
            dBounds.setBounds(dConic);
            if (!boundsSet) {
                bounds = dBounds;
                boundsSet = true;
            } else {
                bounds.add((SkDPoint&) dBounds.fLeft);
                bounds.add((SkDPoint&) dBounds.fRight);
            }
        }
        bounds.fLeft -= 10;
        bounds.fTop -= 10;
        bounds.fRight += 10;
        bounds.fBottom += 10;
        SkBitmap bitmap;
        bitmap.tryAllocPixels(SkImageInfo::MakeN32Premul(
              SkScalarRoundToInt(SkDoubleToScalar(bounds.width())),
              SkScalarRoundToInt(SkDoubleToScalar(bounds.height()))));
        SkCanvas canvas(bitmap);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas.translate(SkDoubleToScalar(-bounds.fLeft), SkDoubleToScalar(-bounds.fTop));
        canvas.drawColor(SK_ColorWHITE);
        for (int fIndex = 0; fIndex < frameSize; ++fIndex) {
            const SkDConic& dC = frames[index][fIndex];
            SkDConic dConic = {{{ {dC.fPts[0].fX * scale, dC.fPts[0].fY * scale },
                {dC.fPts[1].fX * scale, dC.fPts[1].fY * scale },
                {dC.fPts[2].fX * scale, dC.fPts[2].fY * scale }}}, dC.fWeight };
            SkPath path;
            path.moveTo(dConic.fPts[0].asSkPoint());
            path.conicTo(dConic.fPts[1].asSkPoint(), dConic.fPts[2].asSkPoint(), dConic.fWeight);
            if (fIndex < 2) {
                paint.setARGB(0x80, 0xFF, 0, 0);
            } else {
                paint.setARGB(0x80, 0, 0, 0xFF);
            }
            canvas.drawPath(path, paint);
        }
        SkString filename("c:\\Users\\caryclark\\Documents\\");
        filename.appendf("f%d.png", index);
        ToolUtils::EncodeImageToFile(filename.c_str(), bitmap, SkEncodedImageFormat::kPNG, 100);
    }
}
#endif

static void oneOff(skiatest::Reporter* reporter, const ConicPts& conic1, const ConicPts& conic2,
        bool coin) {
#if DEBUG_VISUALIZE_CONICS
    writeFrames();
#endif
    SkDConic c1, c2;
    c1.debugSet(conic1.fPts.fPts, conic1.fWeight);
    c2.debugSet(conic2.fPts.fPts, conic2.fWeight);
    chopBothWays(c1, 0.5, "c1");
    chopBothWays(c2, 0.5, "c2");
#if DEBUG_VISUALIZE_CONICS
    writeDPng(c1, "d1");
    writeDPng(c2, "d2");
#endif
    SkASSERT(ValidConic(c1));
    SkASSERT(ValidConic(c2));
    SkIntersections intersections;
    intersections.intersect(c1, c2);
    if (coin && intersections.used() != 2) {
        SkDebugf("");
    }
    REPORTER_ASSERT(reporter, !coin || intersections.used() == 2);
    double tt1, tt2;
    SkDPoint xy1, xy2;
    for (int pt3 = 0; pt3 < intersections.used(); ++pt3) {
        tt1 = intersections[0][pt3];
        xy1 = c1.ptAtT(tt1);
        tt2 = intersections[1][pt3];
        xy2 = c2.ptAtT(tt2);
        const SkDPoint& iPt = intersections.pt(pt3);
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(iPt));
        REPORTER_ASSERT(reporter, xy2.approximatelyEqual(iPt));
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
    }
    reporter->bumpTestCount();
}

static void oneOff(skiatest::Reporter* reporter, int outer, int inner) {
    const ConicPts& c1 = testSet[outer];
    const ConicPts& c2 = testSet[inner];
    oneOff(reporter, c1, c2, false);
}

static void oneOffTests(skiatest::Reporter* reporter) {
    for (int outer = 0; outer < testSetCount - 1; ++outer) {
        for (int inner = outer + 1; inner < testSetCount; ++inner) {
            oneOff(reporter, outer, inner);
        }
    }
}

DEF_TEST(PathOpsConicIntersectionOneOff, reporter) {
    oneOff(reporter, 0, 1);
}

DEF_TEST(PathOpsConicIntersection, reporter) {
    oneOffTests(reporter);
}
