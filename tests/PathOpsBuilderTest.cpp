/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/pathops/SkPathOps.h"
#include "src/base/SkFloatBits.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/Test.h"

DEF_TEST(PathOpsBuilder, reporter) {
    SkOpBuilder builder;
    auto result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isEmpty());

    builder.add(*result, kDifference_SkPathOp);
    result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isEmpty());

    builder.add(*result, kUnion_SkPathOp);
    result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isEmpty());

    SkPath rectPath = SkPath::Rect({0, 1, 2, 3}, SkPathDirection::kCW)
                      .makeFillType(SkPathFillType::kEvenOdd);
    builder.add(rectPath, kUnion_SkPathOp);
    result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    bool closed;
    SkPathDirection dir;
    REPORTER_ASSERT(reporter, result->isRect(nullptr, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    REPORTER_ASSERT(reporter, dir == SkPathDirection::kCCW);
    int pixelDiff = comparePaths(reporter, __FUNCTION__, rectPath, *result);
    REPORTER_ASSERT(reporter, pixelDiff == 0);

    rectPath = SkPath::Rect({0, 1, 2, 3}, SkPathDirection::kCCW)
               .makeFillType(SkPathFillType::kEvenOdd);
    builder.add(rectPath, kUnion_SkPathOp);
    result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isRect(nullptr, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    REPORTER_ASSERT(reporter, dir == SkPathDirection::kCCW);
    REPORTER_ASSERT(reporter, rectPath == result);

    builder.add(rectPath, kDifference_SkPathOp);
    result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isEmpty());

    SkPath rect2 = SkPath::Rect({2, 1, 4, 3}, SkPathDirection::kCW),
           rect3 = SkPath::Rect({4, 1, 5, 3}, SkPathDirection::kCCW);
    builder.add(rectPath, kUnion_SkPathOp);
    builder.add(rect2, kUnion_SkPathOp);
    builder.add(rect3, kUnion_SkPathOp);
    result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    REPORTER_ASSERT(reporter, result->isRect(nullptr, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    SkRect expected;
    expected.setLTRB(0, 1, 5, 3);
    REPORTER_ASSERT(reporter, result->getBounds() == expected);

    SkPath circle1 = SkPath::Circle(5, 6, 4, SkPathDirection::kCW),
           circle2 = SkPath::Circle(7, 4, 8, SkPathDirection::kCCW),
           circle3 = SkPath::Circle(6, 5, 6, SkPathDirection::kCW);
    SkPath opCompare = Op(circle1, circle2, kUnion_SkPathOp).value_or(SkPath());
    if (auto res = Op(opCompare, circle3, kDifference_SkPathOp)) {
        opCompare = *res;
    }
    builder.add(circle1, kUnion_SkPathOp);
    builder.add(circle2, kUnion_SkPathOp);
    builder.add(circle3, kDifference_SkPathOp);
    result = builder.resolve();
    REPORTER_ASSERT(reporter, result.has_value());
    pixelDiff = comparePaths(reporter, __FUNCTION__, opCompare, *result);
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue3838, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(200, 170)
                  .lineTo(220, 170)
                  .lineTo(220, 230)
                  .lineTo(240, 230)
                  .lineTo(240, 210)
                  .lineTo(180, 210)
                  .lineTo(180, 190)
                  .lineTo(260, 190)
                  .lineTo(260, 250)
                  .lineTo(200, 250)
                  .lineTo(200, 170)
                  .close()
                  .detach();
    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    auto path2 = builder.resolve();
    int pixelDiff = comparePaths(reporter, __FUNCTION__, path, path2.value());
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue3838_2, reporter) {
    SkPath path = SkPath::Circle(100, 100, 50);

    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    builder.add(path, kUnion_SkPathOp);

    auto result = builder.resolve();
    int pixelDiff = comparePaths(reporter, __FUNCTION__, path, result.value());
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue3838_3, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(40, 10)
                  .lineTo(60, 10)
                  .lineTo(60, 30)
                  .lineTo(40, 30)
                  .lineTo(40, 10)
                  .moveTo(41, 11)
                  .lineTo(41, 29)
                  .lineTo(59, 29)
                  .lineTo(59, 11)
                  .lineTo(41, 11)
                  .detach();

    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    auto result = builder.resolve();
    int pixelDiff = comparePaths(reporter, __FUNCTION__, path, result.value());
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue502792_2, reporter) {
    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
                  .addRect({0, 0, 1, 1}, SkPathDirection::kCW)
                  .addRect({2, 2, 3, 3}, SkPathDirection::kCW)
                  .detach();
    SkPath pathB = SkPathBuilder(SkPathFillType::kEvenOdd)
                   .addRect({3, 3, 4, 4}, SkPathDirection::kCW)
                   .addRect({3, 3, 4, 4}, SkPathDirection::kCW)
                   .detach();
    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    builder.add(pathB, kDifference_SkPathOp);
    (void)builder.resolve();
}

DEF_TEST(Fuzz846, reporter) {
/*
<clipPath id="clip-circle">
    <circle id="circle" cx="60" cy="60" r="50" />
</clipPath>
<clipPath id="clip-rect">
    <clipPath id="clip-rect">
        <clipPath id="clip-rect">
            <clipPath id="clip-rect">
                <rect x="10" y="30" width="0" height="60" />
                <rect x="10" y="30" width="0" height="60" />
                <rect x="10" y="30" width="100" height="60" />
                <rect x="10" y="30" width="32668" />
                <rect x="10" y="30" width="100" height="18446744073709551615" />
                <rect x="10" y="255" width="100" height="60" />
                <rect width="100" height="60" />
                <rect x="10" y="30" width="100" height="60" />
                <rect x="10" y="30" width="100" height="4294967236" />
                <rect x="10" y="30" width="100" height="60" />
            </clipPath>
            <rect x="10" y="30" width="0" height="60" />
            <rect x="10" y="30" width="0" height="0.18093252719929986369568203" />
            <rect x="10" y="30" width="100" height="60" />
            <rect x="10" y="30" width="32668" height="60" />
            <rect x="10" y="30" width="100" height="18446744073709551615" />
            <rect x="10" y="255" width="100" height="60" />
            <rect x="2147483649" y="30" width="100" height="60" />
            <rect x="10" y="30" width="100" height="60" />
            <rect x="10" y="30" width="100" height="60" />
            <rect x="10" y="30" width="100" height="60" />
        </clipPath>
        <rect x="10" y="30" width="0" height="60" />
        <rect x="10" y="30" width="0" height="60" />
        <rect x="10" y="30" width="100" height="60" />
        <rect x="10" y="30" width="32668" height="60" />
        <rect x="10" y="30" width="100" height="18446744073709551615" />
        <rect x="10" y="255" width="100" height="60" />
        <rect x="2147483649" y="30" width="100" height="60" />
        <rect x="10" y="30" width="100" height="60" />
        <rect x="10" y="2879753595" width="100" height="60" />
        <rect x="10" y="30" width="100" height="60" />
    </clipPath>
    <rect x="10" y="30" width="100" height="60" />
    <rect x="10" y="30" width="0" height="60" />
    <rect x="10" y="30" width="100" height="60" />
    <rect x="10" y="30" width="32668" height="60" />
    <rect x="10" y="30" width="100" height="18446744073709551615" />
    <rect x="10" y="255" width="100" height="60" />
    <rect x="2147483649" y="30" width="100" height="60" />
    <rect x="10" y="30" width="100" height="60" />
    <rect x="10" y="30" width="100" height="4294967236" />
    <rect x="10" y="30" width="100" height="4294967236" />
    <rect x="10" y="30" width="100" height="4294967236" />
    <rect x="10" y="30" width="100" height="4294967236" />
    <rect x="10" y="30" width="100" height="60" />
    <rect x="757798030" y="30" width="100" height="60" />
*/
    SkPathBuilder clipRect;
    SkPath clipCircle = SkPath::Circle(60, 60, 50);             // <circle id="circle" cx="60" cy="60" r="50" />

    SkPathBuilder inner;
    inner.addRect({10, 30, 10+0, 30+60});           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect({10, 30, 10+0, 30+60});           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+32668, 30+0});        // <rect x="10" y="30" width="32668" />
    inner.addRect({10, 30, 10+100, 30+18446744073709551615.f}); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect({10, 255, 10+100, 255+60});       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect({0, 0, 0+100, 0+60});             //  <rect width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+4294967236.f}); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    clipRect.addPath(inner.detach());

    inner.addRect({10, 30, 10+0, 30+60});           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect({10, 30, 10+0, 30+0.18093252719929986369568203f}); // <rect x="10" y="30" width="0" height="0.18093252719929986369568203" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+32668, 30+60});       // <rect x="10" y="30" width="32668" height="60" />
    inner.addRect({10, 30, 10+100, 30+18446744073709551615.f}); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect({10, 255, 10+100, 255+60});       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect({2147483649.f, 30, 2147483649.f+100, 30+60}); // <rect x="2147483649" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    clipRect.addPath(inner.detach());

    inner.addRect({10, 30, 10+0, 30+60});           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect({10, 30, 10+0, 30+60});           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+32668, 30+60});       // <rect x="10" y="30" width="32668" height="60" />
    inner.addRect({10, 30, 10+100, 30+18446744073709551615.f}); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect({10, 255, 10+100, 255+60});       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect({2147483649.f, 30, 2147483649.f+100, 30+60}); // <rect x="2147483649" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 2879753595.f, 10+100, 30+2879753595.f}); // <rect x="10" y="2879753595" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    clipRect.addPath(inner.detach());

    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+0, 30+60});           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+32668, 30+60});       // <rect x="10" y="30" width="32668" height="60" />
    inner.addRect({10, 30, 10+100, 30+18446744073709551615.f}); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect({10, 255, 10+100, 255+60});       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect({2147483649.f, 30, 2147483649.f+100, 30+60}); // <rect x="2147483649" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({10, 30, 10+100, 30+4294967236.f}); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect({10, 30, 10+100, 30+4294967236.f}); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect({10, 30, 10+100, 30+4294967236.f}); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect({10, 30, 10+100, 30+4294967236.f}); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect({10, 30, 10+100, 30+60});         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect({757798030.f, 30, 757798030.f+100, 30+60}); // <rect x="757798030" y="30" width="100" height="60" />
    clipRect.addPath(inner.detach());

    SkOpBuilder builder;
    builder.add(clipCircle, kUnion_SkPathOp);
    builder.add(clipRect.detach(), kDifference_SkPathOp);
    (void)builder.resolve();
}

DEF_TEST(Issue569540, reporter) {
    SkPath path1 = SkPathBuilder()
                   .moveTo(5, -225)
                   .lineTo(-225, 7425)
                   .lineTo(7425, 7425)
                   .lineTo(7425, -225)
                   .lineTo(-225, -225)
                   .lineTo(5, -225)
                   .close()
                   .detach();

    SkPath path2 = SkPathBuilder()
                   .moveTo(5940, 2790)
                   .lineTo(5940, 2160)
                   .lineTo(5970, 1980)
                   .lineTo(5688, 773669888)
                   .lineTo(5688, 2160)
                   .lineTo(5688, 2430)
                   .lineTo(5400, 4590)
                   .lineTo(5220, 4590)
                   .lineTo(5220, 4920)
                   .cubicTo(5182.22900390625f, 4948.328125f, 5160, 4992.78662109375f, 5160, 5040.00048828125f)
                   .lineTo(5940, 2790)
                   .close()
                   .detach();

    SkOpBuilder builder;
    builder.add(path1, kUnion_SkPathOp);
    builder.add(path2, kUnion_SkPathOp);
    (void)builder.resolve();
}

DEF_TEST(SkOpBuilderFuzz665, reporter) {
    SkPath path = SkPathBuilder(SkPathFillType::kEvenOdd)
    .moveTo(SkBits2Float(0xcc4264a7), SkBits2Float(0x4bb12e50))   // -5.0959e+07f, 2.32235e+07f
    .lineTo(SkBits2Float(0xcc4264b0), SkBits2Float(0x4bb12e48))   // -5.0959e+07f, 2.32234e+07f
    .lineTo(SkBits2Float(0xcc4264a7), SkBits2Float(0x4bb12e50))   // -5.0959e+07f, 2.32235e+07f
    .close()
    .detach();
    SkPath path1(path);
    path = SkPathBuilder(SkPathFillType::kWinding)
    .moveTo(SkBits2Float(0x43213333), SkBits2Float(0x43080000))   // 161.2f, 136
    .lineTo(SkBits2Float(0x43038000), SkBits2Float(0x43080000))   // 131.5f, 136
    .cubicTo(SkBits2Float(0x43038000), SkBits2Float(0x42f00000), SkBits2Float(0x42f16666), SkBits2Float(0x42d53333), SkBits2Float(0x42d3cccd), SkBits2Float(0x42cd6666))   // 131.5f, 120, 120.7f, 106.6f, 105.9f, 102.7f
    .lineTo(SkBits2Float(0x42e33333), SkBits2Float(0x42940000))   // 113.6f, 74
    .detach();
    SkPath path2(path);
    SkOpBuilder builder;
    builder.add(path1, kUnion_SkPathOp);
    builder.add(path2, kUnion_SkPathOp);
    (void)builder.resolve();
}

DEF_TEST(SkOpBuilder618991, reporter) {
    SkPath path0 = SkPathBuilder()
                   .moveTo(140, 40)
                   .lineTo(200, 210)
                   .lineTo(40, 100)
                   .lineTo(2.22223e+07f, 2.22222e+14f)
                   .lineTo(2.22223e+07f, 2.22222e+14f)
                   .detach();

    SkPath path1 = SkPathBuilder()
                   .moveTo(160, 60)
                   .lineTo(220, 230)
                   .lineTo(60, 120)
                   .lineTo(2.22223e+07f, 2.22222e+14f)
                   .lineTo(2.22223e+07f, 2.22222e+14f)
                   .detach();

    SkOpBuilder builder;
    builder.add(path0, SkPathOp::kUnion_SkPathOp);
    builder.add(path1, SkPathOp::kUnion_SkPathOp);
    (void)builder.resolve();
}

DEF_TEST(SkOpBuilderKFuzz1, reporter) {
    SkPath path = SkPathBuilder()
    .moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000))   // 0, 0
    .lineTo(SkBits2Float(0x39008001), SkBits2Float(0xd31fbc1d))   // 0.000122547f, -6.86056e+11f
    .conicTo(SkBits2Float(0x246a205a), SkBits2Float(0x0080d3fb), SkBits2Float(0xce000001), SkBits2Float(0x04d31fbc), SkBits2Float(0x57a82c00))   // 5.07681e-17f, 1.1831e-38f, -5.36871e+08f, 4.9635e-36f, 3.69814e+14f
    .detach();
    SkPath path0(path);
    path = SkPathBuilder()
    .moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000))   // 0, 0
    .cubicTo(SkBits2Float(0x80d3f924), SkBits2Float(0xcecece4f), SkBits2Float(0xcececece), SkBits2Float(0xcececece), SkBits2Float(0x9a9a9ace), SkBits2Float(0x9a9a9a9a))   // -1.94667e-38f, -1.73481e+09f, -1.73483e+09f, -1.73483e+09f, -6.3943e-23f, -6.39427e-23f
    .moveTo(SkBits2Float(0x9a9a019a), SkBits2Float(0xa59a9a9a))   // -6.36955e-23f, -2.68195e-16f
    .detach();
    SkPath path1(path);
    SkOpBuilder builder;
    builder.add(path0, SkPathOp::kUnion_SkPathOp);
    builder.add(path1, SkPathOp::kUnion_SkPathOp);
    (void)builder.resolve();
}
