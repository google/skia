/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkPathPriv.h"
#include "tests/Test.h"

#include <initializer_list>
#include <string>

static SkPath build_squircle(SkPath::Verb verb, const SkRect& rect, SkPathDirection dir) {
    SkPathBuilder builder;
    bool reverse = SkPathDirection::kCCW == dir;
    switch (verb) {
        case SkPath::kLine_Verb:
            builder.addRect(rect, dir);
            reverse = false;
            break;
        case SkPath::kQuad_Verb:
            builder.moveTo(rect.centerX(), rect.fTop);
            builder.quadTo(rect.fRight, rect.fTop, rect.fRight, rect.centerY());
            builder.quadTo(rect.fRight, rect.fBottom, rect.centerX(), rect.fBottom);
            builder.quadTo(rect.fLeft, rect.fBottom, rect.fLeft, rect.centerY());
            builder.quadTo(rect.fLeft, rect.fTop, rect.centerX(), rect.fTop);
            break;
        case SkPath::kConic_Verb:
            builder.addCircle(rect.centerX(), rect.centerY(), rect.width() / 2, dir);
            reverse = false;
            break;
        case SkPath::kCubic_Verb: {
            SkScalar aX14 = rect.fLeft + rect.width() * 1 / 4;
            SkScalar aX34 = rect.fLeft + rect.width() * 3 / 4;
            SkScalar aY14 = rect.fTop + rect.height() * 1 / 4;
            SkScalar aY34 = rect.fTop + rect.height() * 3 / 4;
            builder.moveTo(rect.centerX(), rect.fTop);
            builder.cubicTo(aX34, rect.fTop, rect.fRight, aY14, rect.fRight, rect.centerY());
            builder.cubicTo(rect.fRight, aY34, aX34, rect.fBottom, rect.centerX(), rect.fBottom);
            builder.cubicTo(aX14, rect.fBottom, rect.fLeft, aY34, rect.fLeft, rect.centerY());
            builder.cubicTo(rect.fLeft, aY14, aX14, rect.fTop, rect.centerX(), rect.fTop);
            } break;
        default:
            SkASSERT(0);
    }
    SkPath path = builder.detach();
    if (reverse) {
        SkPathPriv::ReverseAddPath(&builder, path);
        path = builder.detach();
    }
    return path;
}

void bug12040_1(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
                  .moveTo(375, -30)
                  .cubicTo(578, -30, 749, 176, 749, 422)
                  .cubicTo(749, 583, 666, 706, 518, 765)
                  .lineTo(163, 611)
                  .lineTo(163, 579)
                  .lineTo(405, 684)
                  .cubicTo(551, 609, 645, 468, 645, 322)
                  .cubicTo(645, 183, 563, 82, 450, 82)
                  .cubicTo(303, 82, 179, 249, 179, 446)
                  .cubicTo(179, 579, 235, 689, 341, 768)
                  .lineTo(327, 786)
                  .cubicTo(165, 717, 56, 536, 56, 335)
                  .cubicTo(56, 125, 192, -30, 375, -30)
                  .close()
                  .moveTo(214, 225)
                  .cubicTo(333, 248, 396, 311, 396, 405)
                  .lineTo(396, 695)
                  .lineTo(267, 641)
                  .lineTo(267, 395)
                  .cubicTo(267, 324, 249, 285, 201, 254)
                  .cubicTo(201, 254, 214, 225, 214, 225)
                  .close()
                  .moveTo(682, -106)
                  .lineTo(832, 12)
                  .lineTo(813, 33)
                  .lineTo(772, 0)
                  .cubicTo(716, 29, 668, 76, 628, 140)
                  .lineTo(527, 44)
                  .cubicTo(575, -26, 628, -77, 682, -106)
                  .close()
                  .moveTo(450, 59)
                  .lineTo(480, 59)
                  .lineTo(480, 678)
                  .lineTo(450, 678)
                  .cubicTo(450, 678, 450, 59, 450, 59)
                  .close()
                  .moveTo(463, 374)
                  .lineTo(633, 459)
                  .lineTo(633, 490)
                  .lineTo(463, 406)
                  .cubicTo(463, 406, 463, 374, 463, 374)
                  .close()
                  .moveTo(463, 269)
                  .lineTo(667, 372)
                  .lineTo(667, 403)
                  .lineTo(463, 301)
                  .cubicTo(463, 301, 463, 269, 463, 269)
                  .close()
                  .detach();

    SkPath path2 = SkPathBuilder(SkPathFillType::kWinding)
                   .moveTo(-83.5464f, 188)
                   .cubicTo(-83.5464f, 184.285f, -84.8599f, 181.114f, -87.4868f, 178.487f)
                   .cubicTo(-90.1138f, 175.86f, -93.2849f, 174.546f, -97, 174.546f)
                   .cubicTo(-100.715f, 174.546f, -103.886f, 175.86f, -106.513f, 178.487f)
                   .cubicTo(-109.14f, 181.114f, -110.454f, 184.285f, -110.454f, 188)
                   .cubicTo(-110.454f, 191.715f, -109.14f, 194.886f, -106.513f, 197.513f)
                   .cubicTo(-103.886f, 200.14f, -100.715f, 201.454f, -97, 201.454f)
                   .cubicTo(-93.2849f, 201.454f, -90.1138f, 200.14f, -87.4868f, 197.513f)
                   .cubicTo(-84.8599f, 194.886f, -83.5464f, 191.715f, -83.5464f, 188)
                   .close()
                   .detach();

    SkPath opResult = Op(path, path2, kDifference_SkPathOp).value_or(SkPath());

    auto winding = AsWinding(opResult);
    REPORTER_ASSERT(reporter, winding.has_value());
    REPORTER_ASSERT(reporter, winding->getFillType() == SkPathFillType::kWinding);

    SkPath difference = Op(*winding, opResult, kXOR_SkPathOp).value_or(SkPath());
    REPORTER_ASSERT(reporter, difference.isEmpty());
}

void bug12040_2(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
                  .moveTo(375, -30)
                  .cubicTo(578, -30, 749, 176, 749, 422)
                  .cubicTo(749, 583, 666, 706, 518, 765)
                  .lineTo(163, 611)
                  .lineTo(163, 579)
                  .lineTo(405, 684)
                  .cubicTo(551, 609, 645, 468, 645, 322)
                  .cubicTo(645, 183, 563, 82, 450, 82)
                  .cubicTo(303, 82, 179, 249, 179, 446)
                  .cubicTo(179, 579, 235, 689, 341, 768)
                  .lineTo(327, 786)
                  .cubicTo(165, 717, 56, 536, 56, 335)
                  .cubicTo(56, 125, 192, -30, 375, -30)
                  .close()
                  .moveTo(214, 225)
                  .cubicTo(333, 248, 396, 311, 396, 405)
                  .lineTo(396, 695)
                  .lineTo(267, 641)
                  .lineTo(267, 395)
                  .cubicTo(267, 324, 249, 285, 201, 254)
                  .cubicTo(201, 254, 214, 225, 214, 225)
                  .close()
                  .moveTo(682, -106)
                  .lineTo(832, 12)
                  .lineTo(813, 33)
                  .lineTo(772, 0)
                  .cubicTo(716, 29, 668, 76, 628, 140)
                  .lineTo(527, 44)
                  .cubicTo(575, -26, 628, -77, 682, -106)
                  .close()
                  .moveTo(450, 59)
                  .lineTo(480, 59)
                  .lineTo(480, 678)
                  .lineTo(450, 678)
                  .cubicTo(450, 678, 450, 59, 450, 59)
                  .close()
                  .moveTo(463, 374)
                  .lineTo(633, 459)
                  .lineTo(633, 490)
                  .lineTo(463, 406)
                  .cubicTo(463, 406, 463, 374, 463, 374)
                  .close()
                  .moveTo(463, 269)
                  .lineTo(667, 372)
                  .lineTo(667, 403)
                  .lineTo(463, 301)
                  .cubicTo(463, 301, 463, 269, 463, 269)
                  .close()
                  .detach();

    SkPath path2 = SkPathBuilder(SkPathFillType::kWinding)
                   .moveTo(269.134f, 71.3392f)
                   .cubicTo(269.134f, 67.6241f, 267.82f, 64.453f, 265.193f, 61.826f)
                   .cubicTo(262.566f, 59.1991f, 259.395f, 57.8856f, 255.68f, 57.8856f)
                   .cubicTo(251.965f, 57.8856f, 248.794f, 59.1991f, 246.167f, 61.826f)
                   .cubicTo(243.54f, 64.453f, 242.226f, 67.6241f, 242.226f, 71.3392f)
                   .cubicTo(242.226f, 75.0543f, 243.54f, 78.2255f, 246.167f, 80.8524f)
                   .cubicTo(248.794f, 83.4794f, 251.965f, 84.7928f, 255.68f, 84.7928f)
                   .cubicTo(259.395f, 84.7928f, 262.566f, 83.4794f, 265.193f, 80.8524f)
                   .cubicTo(267.82f, 78.2255f, 269.134f, 75.0543f, 269.134f, 71.3392f)
                   .close()
                   .detach();

    SkPath opResult = Op(path, path2, kDifference_SkPathOp).value_or(SkPath());

    // This produces the correct result in all the cases I've tried.
    // So it appears to be a winding issue.
    // canvas->drawPath(opResult, p);

    auto winding = AsWinding(opResult);
    path2.setFillType(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, winding.has_value());
    REPORTER_ASSERT(reporter, winding->getFillType() == SkPathFillType::kWinding);

    SkPath difference = Op(*winding, opResult, kXOR_SkPathOp).value_or(SkPath());
    REPORTER_ASSERT(reporter, difference.isEmpty());
}

void bug12040_3(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
                  .moveTo(375, -30)
                  .cubicTo(578, -30, 749, 176, 749, 422)
                  .cubicTo(749, 583, 666, 706, 518, 765)
                  .lineTo(163, 611)
                  .lineTo(163, 579)
                  .lineTo(405, 684)
                  .cubicTo(551, 609, 645, 468, 645, 322)
                  .cubicTo(645, 183, 563, 82, 450, 82)
                  .cubicTo(303, 82, 179, 249, 179, 446)
                  .cubicTo(179, 579, 235, 689, 341, 768)
                  .lineTo(327, 786)
                  .cubicTo(165, 717, 56, 536, 56, 335)
                  .cubicTo(56, 125, 192, -30, 375, -30)
                  .close()
                  .moveTo(214, 225)
                  .cubicTo(333, 248, 396, 311, 396, 405)
                  .lineTo(396, 695)
                  .lineTo(267, 641)
                  .lineTo(267, 395)
                  .cubicTo(267, 324, 249, 285, 201, 254)
                  .cubicTo(201, 254, 214, 225, 214, 225)
                  .close()
                  .moveTo(682, -106)
                  .lineTo(832, 12)
                  .lineTo(813, 33)
                  .lineTo(772, 0)
                  .cubicTo(716, 29, 668, 76, 628, 140)
                  .lineTo(527, 44)
                  .cubicTo(575, -26, 628, -77, 682, -106)
                  .close()
                  .moveTo(450, 59)
                  .lineTo(480, 59)
                  .lineTo(480, 678)
                  .lineTo(450, 678)
                  .cubicTo(450, 678, 450, 59, 450, 59)
                  .close()
                  .moveTo(463, 374)
                  .lineTo(633, 459)
                  .lineTo(633, 490)
                  .lineTo(463, 406)
                  .cubicTo(463, 406, 463, 374, 463, 374)
                  .close()
                  .moveTo(463, 269)
                  .lineTo(667, 372)
                  .lineTo(667, 403)
                  .lineTo(463, 301)
                  .cubicTo(463, 301, 463, 269, 463, 269)
                  .close()
                  .detach();

    SkPath path2 = SkPathBuilder(SkPathFillType::kWinding)
                   .moveTo(492.041f, 525.339f)
                   .cubicTo(492.041f, 521.624f, 490.727f, 518.453f, 488.1f, 515.826f)
                   .cubicTo(485.473f, 513.199f, 482.302f, 511.886f, 478.587f, 511.886f)
                   .cubicTo(474.872f, 511.886f, 471.701f, 513.199f, 469.074f, 515.826f)
                   .cubicTo(466.447f, 518.453f, 465.134f, 521.624f, 465.134f, 525.339f)
                   .cubicTo(465.134f, 529.054f, 466.447f, 532.226f, 469.074f, 534.853f)
                   .cubicTo(471.701f, 537.479f, 474.872f, 538.793f, 478.587f, 538.793f)
                   .cubicTo(482.302f, 538.793f, 485.473f, 537.479f, 488.1f, 534.853f)
                   .cubicTo(490.727f, 532.226f, 492.041f, 529.054f, 492.041f, 525.339f)
                   .close()
                   .detach();

    SkPath opResult = Op(path, path2, kDifference_SkPathOp).value_or(SkPath());

    auto winding = AsWinding(opResult);
    path2.setFillType(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, winding.has_value());
    REPORTER_ASSERT(reporter, winding->getFillType() == SkPathFillType::kWinding);

    SkPath difference = Op(*winding, opResult, kXOR_SkPathOp).value_or(SkPath());
    REPORTER_ASSERT(reporter, difference.isEmpty());
}

void bug12040_4(skiatest::Reporter* reporter) {
    for (int moveX = 199; moveX <= 201; ++moveX) {
        for (int moveY = 299; moveY <= 301; ++moveY) {
            for (int lineX = 199; lineX <= 201; ++lineX) {
                for (int lineY = 199; lineY <= 201; ++lineY) {
                    SkPath path = SkPath::Circle(250, 250, 150);

                    SkPath path2 = SkPathBuilder(SkPathFillType::kWinding)
                                   .moveTo(moveX, moveY)  // 200, 300 works... But not 200, 301!!
                                   .lineTo(lineX, lineY)  // 200, 200 works... But not 199, 200!!
                                   .lineTo(300, 300)
                                   .close()
                                   .detach();

                    SkPath opResult = Op(path, path2, kDifference_SkPathOp).value_or(SkPath());

                    auto winding = AsWinding(opResult);
                    REPORTER_ASSERT(reporter, winding.has_value());
                    REPORTER_ASSERT(reporter, winding->getFillType() == SkPathFillType::kWinding);

                    SkPath difference = Op(*winding, opResult, kXOR_SkPathOp).value_or(SkPath());
                    REPORTER_ASSERT(reporter, difference.isEmpty());
                }
            }
        }
    }
}

void bug12040_5(skiatest::Reporter* reporter) {
    for (int moveX = 199; moveX <= 201; ++moveX) {
        for (int moveY = 299; moveY <= 301; ++moveY) {
            for (int lineX = 199; lineX <= 201; ++lineX) {
                for (int lineY = 199; lineY <= 201; ++lineY) {
                    SkPath path = SkPath::Rect({100, 100, 400, 400});

                    SkPath path2 = SkPathBuilder(SkPathFillType::kWinding)
                                   .moveTo(moveX, moveY)
                                   .lineTo(lineX, lineY)  // 200, 200 works... But not 199, 200!!
                                   .lineTo(300, 300)
                                   .close()
                                   .detach();

                    SkPath opResult = Op(path, path2, kDifference_SkPathOp).value_or(SkPath());

                    auto winding = AsWinding(opResult);
                    REPORTER_ASSERT(reporter, winding.has_value());
                    REPORTER_ASSERT(reporter, winding->getFillType() == SkPathFillType::kWinding);

                    SkPath difference = Op(*winding, opResult, kXOR_SkPathOp).value_or(SkPath());
                    REPORTER_ASSERT(reporter, difference.isEmpty());
                }
            }
        }
    }
}

void bug13496_1(skiatest::Reporter* reporter) {
    std::string originalPathStr =
            "M5.93 -3.12C5.93 -5.03 4.73 -6.06 3.5 -6.06C2.67 -6.06 1.98 -5.59 1.76 -5.34L1.67 "
            "-5.93L0.75 -5.93L0.75 2.23L1.87 2.04L1.87 -0.12C2.12 -0.03 2.62 0.07 3.18 0.07C4.57 "
            "0.07 5.93 -1.06 5.93 -3.12ZM4.81 -3.09C4.81 -1.51 4.18 -0.85 3.17 -0.85C2.57 -0.85 "
            "2.15 -0.98 1.87 -1.12L1.87 -4.15C2.34 -4.73 2.75 -5.09 3.42 -5.09C4.31 -5.09 4.81 "
            "-4.46 4.81 -3.09Z";

    SkPath path = SkParsePath::FromSVGString(originalPathStr.c_str()).value_or(SkPath());

    SkPath simplifiedPath = Simplify(path).value_or(SkPath());

    auto windingPath = AsWinding(simplifiedPath);
    REPORTER_ASSERT(reporter, windingPath.has_value());
    REPORTER_ASSERT(reporter, windingPath->getFillType() == SkPathFillType::kWinding);

    SkPath difference = Op(*windingPath, simplifiedPath, kXOR_SkPathOp).value_or(SkPath());
    REPORTER_ASSERT(reporter, difference.isEmpty());
}

void bug13496_2(skiatest::Reporter* reporter) {
    std::string originalPathStr =
            "M4 0"
            "L0 0"
            "L0 5"
            "L4 4"
            "Z"
            "M3 3"
            "L1 3"
            "L1 1"
            "L3 1"
            "Z";

    SkPath path = SkParsePath::FromSVGString(originalPathStr.c_str()).value_or(SkPath());

    SkPath simplifiedPath = Simplify(path).value_or(SkPath());

    auto windingPath = AsWinding(simplifiedPath);
    REPORTER_ASSERT(reporter, windingPath.has_value());
    REPORTER_ASSERT(reporter, windingPath->getFillType() == SkPathFillType::kWinding);

    SkPath difference = Op(*windingPath, simplifiedPath, kXOR_SkPathOp).value_or(SkPath());
    REPORTER_ASSERT(reporter, difference.isEmpty());
}

void bug13496_3(skiatest::Reporter* reporter) {
    std::string originalPathStr =
            "M4 0"
            "L0 0"
            "L0 4"
            "L4 4"
            "Z"
            "M3 3"
            "L1 3"
            "L1 1"
            "L3 1"
            "Z";

    SkPath path = SkParsePath::FromSVGString(originalPathStr.c_str()).value_or(SkPath());

    SkPath simplifiedPath = Simplify(path).value_or(SkPath());

    auto windingPath = AsWinding(simplifiedPath);
    REPORTER_ASSERT(reporter, windingPath.has_value());
    REPORTER_ASSERT(reporter, windingPath->getFillType() == SkPathFillType::kWinding);

    SkPath difference = Op(*windingPath, simplifiedPath, kXOR_SkPathOp).value_or(SkPath());
    REPORTER_ASSERT(reporter, difference.isEmpty());
}

DEF_TEST(PathOpsAsWinding, reporter) {
    SkPath test = SkPath::Rect({1, 2, 3, 4});
    // if test is winding
    auto result = AsWinding(test);
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, test == *result);
    // if test is empty
    test = SkPath().makeFillType(SkPathFillType::kEvenOdd);
    result = AsWinding(test);
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isEmpty());
    REPORTER_ASSERT(reporter, result->getFillType() == SkPathFillType::kWinding);
    // if test is convex
    test = SkPath::Circle(5, 5, 10).makeFillType(SkPathFillType::kEvenOdd);
    result = AsWinding(test);
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isConvex());
    test = test.makeFillType(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, test == *result);
    // if test has infinity
    test = SkPath::Rect({1, 2, 3, SK_ScalarInfinity})
           .makeFillType(SkPathFillType::kEvenOdd);
    result = AsWinding(test);
    REPORTER_ASSERT(reporter, !result.has_value());

    // if test has only one contour
    const SkPoint ell[] = {{0, 0}, {4, 0}, {4, 1}, {1, 1}, {1, 4}, {0, 4}};
    SkPathBuilder builder(SkPathFillType::kEvenOdd);
    builder.addPolygon(ell, true);
    test = builder.snapshot();
    result = AsWinding(test);
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, !result->isConvex());
    test = test.makeFillType(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, test == *result);
    // test two contours that do not overlap or share bounds
    builder.addRect({5, 2, 6, 3});
    test = builder.detach();
    result = AsWinding(test);
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, !result->isConvex());
    test = test.makeFillType(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, test == *result);

    // test two contours that do not overlap but share bounds
    test = SkPathBuilder(SkPathFillType::kEvenOdd)
           .addPolygon(ell, true)
           .addRect({2, 2, 3, 3})
           .detach();
    result = AsWinding(test);
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, !result->isConvex());
    test = test.makeFillType(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, test == *result);
    // test two contours that partially overlap
    test = SkPathBuilder(SkPathFillType::kEvenOdd)
           .addRect({0, 0, 3, 3})
           .addRect({1, 1, 4, 4})
           .detach();
    result = AsWinding(test);
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, !result->isConvex());
    test = test.makeFillType(SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, test == *result);

    // test a in b, b in a, cw/ccw
    constexpr SkRect rectA = {0, 0, 3, 3};
    constexpr SkRect rectB = {1, 1, 2, 2};
    const std::initializer_list<SkPoint> revBccw = {{1, 2}, {2, 2}, {2, 1}, {1, 1}};
    const std::initializer_list<SkPoint> revBcw  = {{2, 1}, {2, 2}, {1, 2}, {1, 1}};
    for (bool aFirst : {false, true}) {
        for (auto dirA : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            for (auto dirB : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
                builder.reset().setFillType(SkPathFillType::kEvenOdd);
                if (aFirst) {
                    builder.addRect(rectA, dirA);
                    builder.addRect(rectB, dirB);
                } else {
                    builder.addRect(rectB, dirB);
                    builder.addRect(rectA, dirA);
                }
                test = builder.detach();
                SkPath original = test;
                result = AsWinding(test);
                REPORTER_ASSERT(reporter, result.has_value());
                REPORTER_ASSERT(reporter, result->getFillType() == SkPathFillType::kWinding);

                if (aFirst) {
                    builder.addRect(rectA, dirA);
                }
                if (dirA != dirB) {
                    builder.addRect(rectB, dirB);
                } else {
                    builder.addPolygon(SkPathDirection::kCW == dirA ? revBccw : revBcw, true);
                }
                if (!aFirst) {
                    builder.addRect(rectA, dirA);
                }
                test = builder.detach();
                REPORTER_ASSERT(reporter, test == *result);
            }
        }
    }
    // Test curve types with donuts. Create a donut with outer and hole in all directions.
    // After converting to winding, all donuts should have a hole in the middle.
    for (bool aFirst : {false, true}) {
        for (auto dirA : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            for (auto dirB : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
                for (auto curveA : { SkPath::kLine_Verb, SkPath::kQuad_Verb,
                                     SkPath::kConic_Verb, SkPath::kCubic_Verb } ) {
                    SkPath pathA = build_squircle(curveA, rectA, dirA);
                    for (auto curveB : { SkPath::kLine_Verb, SkPath::kQuad_Verb,
                                     SkPath::kConic_Verb, SkPath::kCubic_Verb } ) {
                        builder.reset();
                        if (aFirst) {
                            builder = pathA;
                        }
                        builder.addPath(build_squircle(curveB, rectB, dirB));
                        if (!aFirst) {
                            builder.addPath(pathA);
                        }
                        builder.setFillType(SkPathFillType::kEvenOdd);
                        test = builder.detach();
                        result = AsWinding(test);
                        REPORTER_ASSERT(reporter, result.has_value());
                        REPORTER_ASSERT(reporter, result->getFillType() == SkPathFillType::kWinding);
                        for (SkScalar x = rectA.fLeft - 1; x <= rectA.fRight + 1; ++x) {
                            for (SkScalar y = rectA.fTop - 1; y <= rectA.fBottom + 1; ++y) {
                                bool evenOddContains = test.contains(x, y);
                                bool windingContains = result->contains(x, y);
                                REPORTER_ASSERT(reporter, evenOddContains == windingContains);
                            }
                        }
                    }
                }
            }
        }
    }
    // test https://bugs.chromium.org/p/skia/issues/detail?id=12040
    bug12040_1(reporter);
    bug12040_2(reporter);
    bug12040_3(reporter);
    bug12040_4(reporter);
    bug12040_5(reporter);
    // test https://bugs.chromium.org/p/skia/issues/detail?id=13496
    bug13496_1(reporter);
    bug13496_2(reporter);
    bug13496_3(reporter);
}
