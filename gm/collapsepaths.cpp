/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

namespace {

void test_collapse1(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->translate(0, 0);
    path.moveTo(       652.830078125,       673.9365234375);
    path.lineTo(  479.50152587890625,  213.412628173828125);
    path.lineTo( 511.840545654296875, 209.1551055908203125);
    path.lineTo(  528.14959716796875,    208.6212158203125);
    path.moveTo(  370.50653076171875,   73.684051513671875);
    path.lineTo(  525.02093505859375, 208.6413726806640625);
    path.lineTo(    478.403564453125, 213.5998992919921875);
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->drawPath(path, paint);
}

void test_collapse2(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo(    492.781982421875,    508.7139892578125);
    path.lineTo( 361.946746826171875, 161.0923004150390625);
    path.lineTo( 386.357513427734375, 157.8785552978515625);
    path.lineTo(    398.668212890625,  157.475555419921875);
    path.moveTo( 279.673004150390625, 55.619640350341796875);
    path.lineTo(  396.30657958984375, 157.4907684326171875);
    path.lineTo( 361.117950439453125, 161.2336578369140625);
    canvas->drawPath(path, paint);
}

void test_collapse3(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo(31.9730987548828125,  69.4149169921875);
    path.lineTo(36.630767822265625,   67.66190338134765625);
    path.lineTo(51.1498870849609375,  64.2765045166015625);
    path.moveTo(52.94580078125,       64.05560302734375);
    path.lineTo(38.9994354248046875,  66.8980712890625);
    path.lineTo(32.229583740234375,   69.31696319580078125);
    path.lineTo(12.99810791015625,    22.4723663330078125);
    canvas->drawPath(path, paint);
}

void test_collapse4(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo(  122.66265869140625, 77.81488800048828125);
    path.lineTo(    161.983642578125,  128.557952880859375);
    path.lineTo(22.599969863891601562, 76.61859893798828125);
    path.lineTo(18.03154754638671875,   76.055633544921875);
    path.lineTo(15.40312957763671875,  75.7647247314453125);
    path.lineTo(18.572841644287109375,  75.2251129150390625);
    path.lineTo(20.895002365112304688,  73.7937774658203125);
    canvas->drawPath(path, paint);
}

void test_collapse5(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo(52.659847259521484375,          782.0546875);
    path.lineTo(136.6915130615234375,   690.18011474609375);
    path.lineTo( 392.147796630859375,    554.6090087890625);
    path.lineTo(  516.51470947265625,   534.44134521484375);
    path.moveTo(154.6182708740234375,  188.230926513671875);
    path.lineTo( 430.242095947265625,   546.76605224609375);
    path.lineTo(      373.1005859375,    559.0906982421875);
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->drawPath(path, paint);
}

void test_collapse6(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo(13.314494132995605469, 197.7343902587890625);
    path.lineTo(34.56102752685546875, 174.5048675537109375);
    path.lineTo(99.15048980712890625,   140.22711181640625);
    path.lineTo( 130.595367431640625,       135.1279296875);
    path.moveTo(39.09362030029296875, 47.59223175048828125);
    path.lineTo(108.7822418212890625,  138.244110107421875);
    path.lineTo(94.33460235595703125,  141.360260009765625);
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->drawPath(path, paint);
}

void test_collapse7(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo(13.737141609191894531, 204.0111541748046875);
    path.lineTo(  35.658111572265625,   180.04425048828125);
    path.lineTo(102.2978668212890625,   144.67840576171875);
    path.lineTo(  134.74090576171875,    139.4173583984375);
    path.moveTo(40.33458709716796875, 49.10297393798828125);
    path.lineTo(112.2353668212890625,    142.6324462890625);
    path.lineTo(97.32910919189453125, 145.8475189208984375);
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->drawPath(path, paint);
}

void test_collapse8(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo( 11.75, 174.50);
    path.lineTo( 30.50, 154.00);
    path.lineTo( 87.50, 123.75);
    path.lineTo(115.25, 119.25);
    path.moveTo( 34.50,  42.00);
    path.lineTo( 96.00, 122.00);
    path.lineTo( 83.25, 124.75);
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->drawPath(path, paint);
}

void test_collapse9(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo(SkDoubleToScalar( 13.25),   SkDoubleToScalar(197.75));
    path.lineTo(SkDoubleToScalar( 34.75),   SkDoubleToScalar(174.75));
    path.lineTo(SkDoubleToScalar( 99.0364), SkDoubleToScalar(140.364));
    path.lineTo(SkDoubleToScalar( 99.25),   SkDoubleToScalar(140.25));
    path.lineTo(SkDoubleToScalar(100.167),  SkDoubleToScalar(140.096));
    path.lineTo(SkDoubleToScalar(130.50),   SkDoubleToScalar(135.00));
    path.moveTo(SkDoubleToScalar( 39.25),   SkDoubleToScalar( 47.50));
    path.lineTo(SkDoubleToScalar(100.167),  SkDoubleToScalar(140.096));
    path.lineTo(SkDoubleToScalar( 99.0364), SkDoubleToScalar(140.364));
    path.lineTo(SkDoubleToScalar( 94.25),   SkDoubleToScalar(141.50));
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->drawPath(path, paint);
}

// This one is a thin inverted 'v', but the edges should not disappear at any point.
void test_collapse10(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    path.moveTo( 5.5,  36.0);
    path.lineTo(47.5,   5.0);
    path.lineTo(90.0,  36.0);
    path.lineTo(88.5,  36.0);
    path.lineTo(47.5,   6.0);
    path.lineTo( 7.0,  36.0);
    canvas->drawPath(path, paint);
}

};

DEF_SIMPLE_GM(collapsepaths, canvas, 500, 600) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    test_collapse1(canvas, paint);
    test_collapse2(canvas, paint);
    test_collapse3(canvas, paint);
    test_collapse4(canvas, paint);
    test_collapse5(canvas, paint);
    test_collapse6(canvas, paint);
    test_collapse7(canvas, paint);
    test_collapse8(canvas, paint);
    test_collapse9(canvas, paint);
    test_collapse10(canvas, paint);
}
