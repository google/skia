/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsExtendedTest.h"
#include "PathOpsTestCommon.h"
#include "SkBitmap.h"
#include "Test.h"

DEF_TEST(PathOpsBuilder, reporter) {
    SkOpBuilder builder;
    SkPath result;
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    builder.add(result, kDifference_SkPathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    builder.add(result, kUnion_SkPathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    SkPath rectPath;
    rectPath.setFillType(SkPath::kEvenOdd_FillType);
    rectPath.addRect(0, 1, 2, 3, SkPath::kCW_Direction);
    builder.add(rectPath, kUnion_SkPathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    bool closed;
    SkPath::Direction dir;
    REPORTER_ASSERT(reporter, result.isRect(NULL, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    REPORTER_ASSERT(reporter, dir == SkPath::kCCW_Direction);
    int pixelDiff = comparePaths(reporter, __FUNCTION__, rectPath, result);
    REPORTER_ASSERT(reporter, pixelDiff == 0);

    rectPath.reset();
    rectPath.setFillType(SkPath::kEvenOdd_FillType);
    rectPath.addRect(0, 1, 2, 3, SkPath::kCCW_Direction);
    builder.add(rectPath, kUnion_SkPathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isRect(NULL, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    REPORTER_ASSERT(reporter, dir == SkPath::kCCW_Direction);
    REPORTER_ASSERT(reporter, rectPath == result);

    builder.add(rectPath, kDifference_SkPathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    SkPath rect2, rect3;
    rect2.addRect(2, 1, 4, 3, SkPath::kCW_Direction);
    rect3.addRect(4, 1, 5, 3, SkPath::kCCW_Direction);
    builder.add(rectPath, kUnion_SkPathOp);
    builder.add(rect2, kUnion_SkPathOp);
    builder.add(rect3, kUnion_SkPathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isRect(NULL, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    SkRect expected;
    expected.set(0, 1, 5, 3);
    REPORTER_ASSERT(reporter, result.getBounds() == expected);

    SkPath circle1, circle2, circle3;
    circle1.addCircle(5, 6, 4, SkPath::kCW_Direction);
    circle2.addCircle(7, 4, 8, SkPath::kCCW_Direction);
    circle3.addCircle(6, 5, 6, SkPath::kCW_Direction);
    SkPath opCompare;
    Op(circle1, circle2, kUnion_SkPathOp, &opCompare);
    Op(opCompare, circle3, kDifference_SkPathOp, &opCompare);
    builder.add(circle1, kUnion_SkPathOp);
    builder.add(circle2, kUnion_SkPathOp);
    builder.add(circle3, kDifference_SkPathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    pixelDiff = comparePaths(reporter, __FUNCTION__, opCompare, result);
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue3838, reporter) {
    SkPath path;
    path.moveTo(200, 170);
    path.lineTo(220, 170);
    path.lineTo(220, 230);
    path.lineTo(240, 230);
    path.lineTo(240, 210);
    path.lineTo(180, 210);
    path.lineTo(180, 190);
    path.lineTo(260, 190);
    path.lineTo(260, 250);
    path.lineTo(200, 250);
    path.lineTo(200, 170);
    path.close();
    SkPath path2;
    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    builder.resolve(&path2);
    int pixelDiff = comparePaths(reporter, __FUNCTION__, path, path2);
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue3838_2, reporter) {
    SkPath path;
    path.addCircle(100, 100, 50);

    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    builder.add(path, kUnion_SkPathOp);

    SkPath result;
    builder.resolve(&result);
    int pixelDiff = comparePaths(reporter, __FUNCTION__, path, result);
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue3838_3, reporter) {
    SkPath path;
    path.moveTo(40, 10);
    path.lineTo(60, 10);
    path.lineTo(60, 30);
    path.lineTo(40, 30);
    path.lineTo(40, 10);
    path.moveTo(41, 11);
    path.lineTo(41, 29);
    path.lineTo(59, 29);
    path.lineTo(59, 11);
    path.lineTo(41, 11);

    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    SkPath result;
    builder.resolve(&result);
    int pixelDiff = comparePaths(reporter, __FUNCTION__, path, result);
    REPORTER_ASSERT(reporter, pixelDiff == 0);
}

DEF_TEST(BuilderIssue502792_2, reporter) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(2, 2, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(3, 3, 4, 4, SkPath::kCW_Direction);
    pathB.addRect(3, 3, 4, 4, SkPath::kCW_Direction);
    SkOpBuilder builder;
    builder.add(path, kUnion_SkPathOp);
    builder.add(pathB, kDifference_SkPathOp);
    SkPath result;
    builder.resolve(&result);
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
    SkPath clipCircle, clipRect;
    SkPath inner;
    clipCircle.addCircle(60, 60, 50);             // <circle id="circle" cx="60" cy="60" r="50" />

    inner.addRect(10, 30, 10+0, 30+60);           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect(10, 30, 10+0, 30+60);           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+32668, 30+0);        // <rect x="10" y="30" width="32668" />
    inner.addRect(10, 30, 10+100, 30+18446744073709551615.f); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect(10, 255, 10+100, 255+60);       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect(0, 0, 0+100, 0+60);             //  <rect width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+4294967236.f); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    clipRect.addPath(inner);
    inner.reset();
    inner.addRect(10, 30, 10+0, 30+60);           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect(10, 30, 10+0, 30+0.18093252719929986369568203f); // <rect x="10" y="30" width="0" height="0.18093252719929986369568203" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+32668, 30+60);       // <rect x="10" y="30" width="32668" height="60" />
    inner.addRect(10, 30, 10+100, 30+18446744073709551615.f); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect(10, 255, 10+100, 255+60);       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect(2147483649.f, 30, 2147483649.f+100, 30+60); // <rect x="2147483649" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    clipRect.addPath(inner);
    inner.reset();
    inner.addRect(10, 30, 10+0, 30+60);           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect(10, 30, 10+0, 30+60);           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+32668, 30+60);       // <rect x="10" y="30" width="32668" height="60" />
    inner.addRect(10, 30, 10+100, 30+18446744073709551615.f); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect(10, 255, 10+100, 255+60);       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect(2147483649.f, 30, 2147483649.f+100, 30+60); // <rect x="2147483649" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 2879753595.f, 10+100, 30+2879753595.f); // <rect x="10" y="2879753595" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    clipRect.addPath(inner);
    inner.reset();
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+0, 30+60);           // <rect x="10" y="30" width="0" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+32668, 30+60);       // <rect x="10" y="30" width="32668" height="60" />
    inner.addRect(10, 30, 10+100, 30+18446744073709551615.f); // <rect x="10" y="30" width="100" height="18446744073709551615" />
    inner.addRect(10, 255, 10+100, 255+60);       // <rect x="10" y="255" width="100" height="60" />
    inner.addRect(2147483649.f, 30, 2147483649.f+100, 30+60); // <rect x="2147483649" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(10, 30, 10+100, 30+4294967236.f); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect(10, 30, 10+100, 30+4294967236.f); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect(10, 30, 10+100, 30+4294967236.f); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect(10, 30, 10+100, 30+4294967236.f); // <rect x="10" y="30" width="100" height="4294967236" />
    inner.addRect(10, 30, 10+100, 30+60);         // <rect x="10" y="30" width="100" height="60" />
    inner.addRect(757798030.f, 30, 757798030.f+100, 30+60); // <rect x="757798030" y="30" width="100" height="60" />
    clipRect.addPath(inner);

    SkOpBuilder builder;
    builder.add(clipCircle, kUnion_SkPathOp);
    builder.add(clipRect, kDifference_SkPathOp);
    SkPath result;
    builder.resolve(&result);
}

