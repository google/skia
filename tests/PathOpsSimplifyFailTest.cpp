/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkPoint.h"
#include "Test.h"

static const SkPoint nonFinitePts[] = {
    { SK_ScalarInfinity, 0 },
    { 0, SK_ScalarInfinity },
    { SK_ScalarInfinity, SK_ScalarInfinity },
    { SK_ScalarNegativeInfinity, 0},
    { 0, SK_ScalarNegativeInfinity },
    { SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity },
    { SK_ScalarNegativeInfinity, SK_ScalarInfinity },
    { SK_ScalarInfinity, SK_ScalarNegativeInfinity },
    { SK_ScalarNaN, 0 },
    { 0, SK_ScalarNaN },
    { SK_ScalarNaN, SK_ScalarNaN },
};

const size_t nonFinitePtsCount = sizeof(nonFinitePts) / sizeof(nonFinitePts[0]);

static const SkPoint finitePts[] = {
    { 0, 0 },
    { SK_ScalarMax, 0 },
    { 0, SK_ScalarMax },
    { SK_ScalarMax, SK_ScalarMax },
    { SK_ScalarMin, 0 },
    { 0, SK_ScalarMin },
    { SK_ScalarMin, SK_ScalarMin },
};

const size_t finitePtsCount = sizeof(finitePts) / sizeof(finitePts[0]);

static void failOne(skiatest::Reporter* reporter, int index) {
    SkPath path;
    int i = (int) (index % nonFinitePtsCount);
    int f = (int) (index % finitePtsCount);
    int g = (int) ((f + 1) % finitePtsCount);
    switch (index % 13) {
        case 0: path.lineTo(nonFinitePts[i]); break;
        case 1: path.quadTo(nonFinitePts[i], nonFinitePts[i]); break;
        case 2: path.quadTo(nonFinitePts[i], finitePts[f]); break;
        case 3: path.quadTo(finitePts[f], nonFinitePts[i]); break;
        case 4: path.cubicTo(nonFinitePts[i], finitePts[f], finitePts[f]); break;
        case 5: path.cubicTo(finitePts[f], nonFinitePts[i], finitePts[f]); break;
        case 6: path.cubicTo(finitePts[f], finitePts[f], nonFinitePts[i]); break;
        case 7: path.cubicTo(nonFinitePts[i], nonFinitePts[i], finitePts[f]); break;
        case 8: path.cubicTo(nonFinitePts[i], finitePts[f], nonFinitePts[i]); break;
        case 9: path.cubicTo(finitePts[f], nonFinitePts[i], nonFinitePts[i]); break;
        case 10: path.cubicTo(nonFinitePts[i], nonFinitePts[i], nonFinitePts[i]); break;
        case 11: path.cubicTo(nonFinitePts[i], finitePts[f], finitePts[g]); break;
        case 12: path.moveTo(nonFinitePts[i]); break;
    }
    SkPath result;
    result.setFillType(SkPath::kWinding_FillType);
    bool success = Simplify(path, &result);
    REPORTER_ASSERT(reporter, !success);
    REPORTER_ASSERT(reporter, result.isEmpty());
    REPORTER_ASSERT(reporter, result.getFillType() == SkPath::kWinding_FillType);
    reporter->bumpTestCount();
}

static void dontFailOne(skiatest::Reporter* reporter, int index) {
    SkPath path;
    int f = (int) (index % finitePtsCount);
    int g = (int) ((f + 1) % finitePtsCount);
    switch (index % 11) {
        case 0: path.lineTo(finitePts[f]); break;
        case 1: path.quadTo(finitePts[f], finitePts[f]); break;
        case 2: path.quadTo(finitePts[f], finitePts[g]); break;
        case 3: path.quadTo(finitePts[g], finitePts[f]); break;
        case 4: path.cubicTo(finitePts[f], finitePts[f], finitePts[f]); break;
        case 5: path.cubicTo(finitePts[f], finitePts[f], finitePts[g]); break;
        case 6: path.cubicTo(finitePts[f], finitePts[g], finitePts[f]); break;
        case 7: path.cubicTo(finitePts[f], finitePts[g], finitePts[g]); break;
        case 8: path.cubicTo(finitePts[g], finitePts[f], finitePts[f]); break;
        case 9: path.cubicTo(finitePts[g], finitePts[f], finitePts[g]); break;
        case 10: path.moveTo(finitePts[f]); break;
    }
    SkPath result;
    result.setFillType(SkPath::kWinding_FillType);
    bool success = Simplify(path, &result);
    if (index != 17 && index != 31 && index != 38) {  // cubic fails to chop in two without creating NaNs
        REPORTER_ASSERT(reporter, success);
        REPORTER_ASSERT(reporter, result.getFillType() != SkPath::kWinding_FillType);
    }
    reporter->bumpTestCount();
}

static void fuzz_59(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(SkBits2Float(0x430c0000), SkBits2Float(0xce58f41c));  // 140, -9.09969e+08f
    path.lineTo(SkBits2Float(0x43480000), SkBits2Float(0xce58f419));  // 200, -9.09969e+08f
    path.lineTo(SkBits2Float(0x42200000), SkBits2Float(0xce58f41b));  // 40, -9.09969e+08f
    path.lineTo(SkBits2Float(0x43700000), SkBits2Float(0xce58f41b));  // 240, -9.09969e+08f
    path.lineTo(SkBits2Float(0x428c0000), SkBits2Float(0xce58f419));  // 70, -9.09969e+08f
    path.lineTo(SkBits2Float(0x430c0000), SkBits2Float(0xce58f41c));  // 140, -9.09969e+08f
    path.close();
    testSimplifyCheck(reporter, path, filename, true);
}

static void fuzz_x1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x1931204a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a4a34), SkBits2Float(0x4a4a4a4a));  // 9.15721e-24f, 1.14845e-12f, 3.31014e+06f, 3.31014e+06f, 3.31432e+06f, 3.31432e+06f
path.moveTo(SkBits2Float(0x000010a1), SkBits2Float(0x19312000));  // 5.96533e-42f, 9.15715e-24f
path.cubicTo(SkBits2Float(0x4a6a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa14a4a4a), SkBits2Float(0x08ff2ba1), SkBits2Float(0x08ff4a4a), SkBits2Float(0x4a344a4a));  // 3.83861e+06f, 3.31432e+06f, -6.85386e-19f, 1.53575e-33f, 1.53647e-33f, 2.95387e+06f
path.cubicTo(SkBits2Float(0x4a4a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4e4a08ff), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa1a181ff));  // 3.31432e+06f, 3.31432e+06f, 1.14845e-12f, 8.47397e+08f, 3.31432e+06f, -1.09442e-18f
    testSimplify(reporter, path, filename);
}

static void fuzz_x2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x1931204a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a4a34), SkBits2Float(0x4a4a4a4a));  // 9.15721e-24f, 1.14845e-12f, 3.31014e+06f, 3.31014e+06f, 3.31432e+06f, 3.31432e+06f
path.moveTo(SkBits2Float(0x000010a1), SkBits2Float(0x19312000));  // 5.96533e-42f, 9.15715e-24f
path.cubicTo(SkBits2Float(0x4a6a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa14a4a4a), SkBits2Float(0x08ff2ba1), SkBits2Float(0x08ff4a4a), SkBits2Float(0x4a344a4a));  // 3.83861e+06f, 3.31432e+06f, -6.85386e-19f, 1.53575e-33f, 1.53647e-33f, 2.95387e+06f
path.cubicTo(SkBits2Float(0x4a4a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4e4a08ff), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa1a181ff));  // 3.31432e+06f, 3.31432e+06f, 1.14845e-12f, 8.47397e+08f, 3.31432e+06f, -1.09442e-18f
    testSimplify(reporter, path, filename);
}

#define TEST(test) test(reporter, #test)

DEF_TEST(PathOpsSimplifyFail, reporter) {
    TEST(fuzz_x2);
    TEST(fuzz_x1);
    TEST(fuzz_59);
    for (int index = 0; index < (int) (13 * nonFinitePtsCount * finitePtsCount); ++index) {
        failOne(reporter, index);
    }
    for (int index = 0; index < (int) (11 * finitePtsCount); ++index) {
        dontFailOne(reporter, index);
    }
}

#undef TEST

DEF_TEST(PathOpsSimplifyFailOne, reporter) {
    int index = 0;
    failOne(reporter, index);
}

DEF_TEST(PathOpsSimplifyDontFailOne, reporter) {
    int index = 17;
    dontFailOne(reporter, index);
}
