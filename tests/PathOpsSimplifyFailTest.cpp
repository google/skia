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
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, result.getFillType() != SkPath::kWinding_FillType);
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
    testSimplifyFuzz(reporter, path, filename);
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

static void fuzz763_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0xbcb63000), SkBits2Float(0xb6b6b6b7), SkBits2Float(0x38b6b6b6), SkBits2Float(0xafb63a5a), SkBits2Float(0xca000087), SkBits2Float(0xe93ae9e9));  // -0.0222397f, -5.44529e-06f, 8.71247e-05f, -3.31471e-10f, -2.09719e+06f, -1.41228e+25f
path.quadTo(SkBits2Float(0xb6007fb6), SkBits2Float(0xb69fb6b6), SkBits2Float(0xe9e964b6), SkBits2Float(0xe9e9e9e9));  // -1.91478e-06f, -4.75984e-06f, -3.52694e+25f, -3.5348e+25f
path.quadTo(SkBits2Float(0xb6b6b8b7), SkBits2Float(0xb60000b6), SkBits2Float(0xb6b6b6b6), SkBits2Float(0xe9e92064));  // -5.44553e-06f, -1.90739e-06f, -5.44529e-06f, -3.52291e+25f
path.quadTo(SkBits2Float(0x000200e9), SkBits2Float(0xe9e9d100), SkBits2Float(0xe93ae9e9), SkBits2Float(0xe964b6e9));  // 1.83997e-40f, -3.53333e+25f, -1.41228e+25f, -1.72812e+25f
path.quadTo(SkBits2Float(0x40b6e9e9), SkBits2Float(0xe9b60000), SkBits2Float(0x00b6b8e9), SkBits2Float(0xe9000001));  // 5.71605f, -2.75031e+25f, 1.67804e-38f, -9.67141e+24f
path.quadTo(SkBits2Float(0xe9d3b6b2), SkBits2Float(0x40404540), SkBits2Float(0x803d4043), SkBits2Float(0xe9e9e9ff));  // -3.19933e+25f, 3.00423f, -5.62502e-39f, -3.53481e+25f
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0xe8b3b6b6), SkBits2Float(0xe90a0003), SkBits2Float(0x4040403c), SkBits2Float(0x803d4040), SkBits2Float(0xe9e80900));  // 0, -6.78939e+24f, -1.0427e+25f, 3.00392f, -5.62501e-39f, -3.50642e+25f
path.quadTo(SkBits2Float(0xe9e910e9), SkBits2Float(0xe9e93ae9), SkBits2Float(0x0000b6b6), SkBits2Float(0xb6b6aab6));  // -3.52199e+25f, -3.52447e+25f, 6.55443e-41f, -5.4439e-06f
path.moveTo(SkBits2Float(0xe9e92064), SkBits2Float(0xe9e9d106));  // -3.52291e+25f, -3.53334e+25f
path.quadTo(SkBits2Float(0xe9e93ae9), SkBits2Float(0x0000abb6), SkBits2Float(0xb6b6bdb6), SkBits2Float(0xe92064b6));  // -3.52447e+25f, 6.15983e-41f, -5.44611e-06f, -1.2119e+25f
path.quadTo(SkBits2Float(0x0000e9e9), SkBits2Float(0xb6b6b6e9), SkBits2Float(0x05ffff05), SkBits2Float(0xe9ea06e9));  // 8.39112e-41f, -5.44532e-06f, 2.40738e-35f, -3.53652e+25f
path.quadTo(SkBits2Float(0xe93ae9e9), SkBits2Float(0x02007fe9), SkBits2Float(0xb8b7b600), SkBits2Float(0xe9e9b6b6));  // -1.41228e+25f, 9.44066e-38f, -8.76002e-05f, -3.53178e+25f
path.quadTo(SkBits2Float(0xe9e9e9b6), SkBits2Float(0xedb6b6b6), SkBits2Float(0x5a38a1b6), SkBits2Float(0xe93ae9e9));  // -3.53479e+25f, -7.06839e+27f, 1.29923e+16f, -1.41228e+25f
path.quadTo(SkBits2Float(0x0000b6b6), SkBits2Float(0xb6b6b6b6), SkBits2Float(0xe9e9e9b6), SkBits2Float(0xe9e9e954));  // 6.55443e-41f, -5.44529e-06f, -3.53479e+25f, -3.53477e+25f
path.quadTo(SkBits2Float(0xb6e9e93a), SkBits2Float(0x375837ff), SkBits2Float(0xceb6b6b6), SkBits2Float(0x0039e94f));  // -6.97109e-06f, 1.28876e-05f, -1.53271e+09f, 5.31832e-39f
path.quadTo(SkBits2Float(0xe9e9e9e9), SkBits2Float(0xe9e6e9e9), SkBits2Float(0xb6b641b6), SkBits2Float(0xede9e9e9));  // -3.5348e+25f, -3.48947e+25f, -5.43167e-06f, -9.0491e+27f
path.moveTo(SkBits2Float(0xb6b6e9e9), SkBits2Float(0xb6b60000));  // -5.45125e-06f, -5.42402e-06f
path.moveTo(SkBits2Float(0xe9b6b6b6), SkBits2Float(0xe9b6b8e9));  // -2.76109e+25f, -2.76122e+25f
path.close();
path.moveTo(SkBits2Float(0xe9b6b6b6), SkBits2Float(0xe9b6b8e9));  // -2.76109e+25f, -2.76122e+25f
path.quadTo(SkBits2Float(0xe93ae9e9), SkBits2Float(0xe964b6e9), SkBits2Float(0x0000203a), SkBits2Float(0xb6000000));  // -1.41228e+25f, -1.72812e+25f, 1.15607e-41f, -1.90735e-06f
path.moveTo(SkBits2Float(0x64b6b6b6), SkBits2Float(0xe9e9e900));  // 2.69638e+22f, -3.53475e+25f
path.quadTo(SkBits2Float(0xb6b6b6e9), SkBits2Float(0xb6b6b6b6), SkBits2Float(0xe9e9b6ce), SkBits2Float(0xe9e93ae9));  // -5.44532e-06f, -5.44529e-06f, -3.53179e+25f, -3.52447e+25f

    testSimplifyFuzz(reporter, path, filename);
}

static void fuzz763_2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x76773011), SkBits2Float(0x5d66fe78), SkBits2Float(0xbbeeff66), SkBits2Float(0x637677a2), SkBits2Float(0x205266fe), SkBits2Float(0xec296fdf));  // 1.25339e+33f, 1.0403e+18f, -0.00729363f, 4.54652e+21f, 1.78218e-19f, -8.19347e+26f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0xec4eecec), SkBits2Float(0x6e6f10ec), SkBits2Float(0xb6b6ecf7), SkBits2Float(0xb6b6b6b6));  // -1.00063e+27f, 1.84968e+28f, -5.45161e-06f, -5.44529e-06f
path.moveTo(SkBits2Float(0x002032b8), SkBits2Float(0xecfeb6b6));  // 2.95693e-39f, -2.46344e+27f
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.cubicTo(SkBits2Float(0x1616ece4), SkBits2Float(0xdf020018), SkBits2Float(0x77772965), SkBits2Float(0x1009db73), SkBits2Float(0x80ececec), SkBits2Float(0xf7ffffff));  // 1.21917e-25f, -9.36751e+18f, 5.01303e+33f, 2.71875e-29f, -2.17582e-38f, -1.03846e+34f
path.lineTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.close();
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.conicTo(SkBits2Float(0xec0700ec), SkBits2Float(0xecececec), SkBits2Float(0xececccec), SkBits2Float(0x772965ec), SkBits2Float(0x77777377));  // -6.52837e+26f, -2.2914e+27f, -2.29019e+27f, 3.4358e+33f, 5.0189e+33f
path.moveTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.close();
path.moveTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.quadTo(SkBits2Float(0x29ec02ec), SkBits2Float(0x1009ecec), SkBits2Float(0x80ececec), SkBits2Float(0xf7ffffff));  // 1.0481e-13f, 2.7201e-29f, -2.17582e-38f, -1.03846e+34f
path.lineTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.close();
path.moveTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.conicTo(SkBits2Float(0xff003aff), SkBits2Float(0xdbec2300), SkBits2Float(0xecececec), SkBits2Float(0x6fdf6052), SkBits2Float(0x41ecec29));  // -1.70448e+38f, -1.32933e+17f, -2.2914e+27f, 1.38263e+29f, 29.6153f
path.lineTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.close();
path.moveTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.quadTo(SkBits2Float(0xecf76e6f), SkBits2Float(0xeccfddec), SkBits2Float(0xecececcc), SkBits2Float(0x66000066));  // -2.39301e+27f, -2.01037e+27f, -2.2914e+27f, 1.51118e+23f
path.lineTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.close();
path.moveTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.cubicTo(SkBits2Float(0x772965df), SkBits2Float(0x77777377), SkBits2Float(0x77777876), SkBits2Float(0x665266fe), SkBits2Float(0xecececdf), SkBits2Float(0x0285806e));  // 3.4358e+33f, 5.0189e+33f, 5.0193e+33f, 2.48399e+23f, -2.2914e+27f, 1.96163e-37f
path.lineTo(SkBits2Float(0xecececeb), SkBits2Float(0xecec0700));  // -2.2914e+27f, -2.28272e+27f
path.lineTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.close();
path.moveTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.lineTo(SkBits2Float(0x65ecfaec), SkBits2Float(0xde777729));  // 1.39888e+23f, -4.45794e+18f
path.conicTo(SkBits2Float(0x74777777), SkBits2Float(0x66fe7876), SkBits2Float(0xecdf6660), SkBits2Float(0x726eecec), SkBits2Float(0x29d610ec));  // 7.84253e+31f, 6.00852e+23f, -2.16059e+27f, 4.73241e+30f, 9.50644e-14f
path.lineTo(SkBits2Float(0xfe817477), SkBits2Float(0xdf665266));  // -8.60376e+37f, -1.65964e+19f
path.close();
path.moveTo(SkBits2Float(0xd0ecec10), SkBits2Float(0x6e6eecdb));  // -3.17991e+10f, 1.84859e+28f
path.quadTo(SkBits2Float(0x003affec), SkBits2Float(0xec2300ef), SkBits2Float(0xecececdb), SkBits2Float(0xcfececec));  // 5.41827e-39f, -7.88237e+26f, -2.2914e+27f, -7.9499e+09f
path.lineTo(SkBits2Float(0xd0ecec10), SkBits2Float(0x6e6eecdb));  // -3.17991e+10f, 1.84859e+28f
path.close();
path.moveTo(SkBits2Float(0xd0ecec10), SkBits2Float(0x6e6eecdb));  // -3.17991e+10f, 1.84859e+28f
path.quadTo(SkBits2Float(0xecccec80), SkBits2Float(0xfa66ecec), SkBits2Float(0x66fa0000), SkBits2Float(0x772965df));  // -1.9819e+27f, -2.99758e+35f, 5.90296e+23f, 3.4358e+33f
path.moveTo(SkBits2Float(0x77777790), SkBits2Float(0x00807677));  // 5.01923e+33f, 1.17974e-38f
path.close();
path.moveTo(SkBits2Float(0x77777790), SkBits2Float(0x00807677));  // 5.01923e+33f, 1.17974e-38f
path.cubicTo(SkBits2Float(0xecececec), SkBits2Float(0xfe66eaec), SkBits2Float(0xecdf1452), SkBits2Float(0x806eecec), SkBits2Float(0x10ececec), SkBits2Float(0xec000000));  // -2.2914e+27f, -7.67356e+37f, -2.15749e+27f, -1.01869e-38f, 9.34506e-29f, -6.1897e+26f
path.lineTo(SkBits2Float(0x77777790), SkBits2Float(0x00807677));  // 5.01923e+33f, 1.17974e-38f
path.close();
path.moveTo(SkBits2Float(0x77777790), SkBits2Float(0x00807677));  // 5.01923e+33f, 1.17974e-38f
path.cubicTo(SkBits2Float(0x52668062), SkBits2Float(0x2965df66), SkBits2Float(0x77777377), SkBits2Float(0x76777773), SkBits2Float(0x1697fe78), SkBits2Float(0xeebfff00));  // 2.47499e+11f, 5.1042e-14f, 5.0189e+33f, 1.2548e+33f, 2.4556e-25f, -2.971e+28f
path.lineTo(SkBits2Float(0x77777790), SkBits2Float(0x00807677));  // 5.01923e+33f, 1.17974e-38f
path.close();

    testSimplifyFuzz(reporter, path, filename);
}

static void fuzz_twister(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(0, 600);
path.lineTo(3.35544e+07f, 600);
path.lineTo(3.35544e+07f, 0);
path.lineTo(0, 0);
path.lineTo(0, 600);
path.close();
path.moveTo(63, 600);
path.lineTo(3.35545e+07f, 600);
path.lineTo(3.35545e+07f, 0);
path.lineTo(63, 0);
path.lineTo(63, 600);
path.close();
path.moveTo(93, 600);
path.lineTo(3.35545e+07f, 600);
path.lineTo(3.35545e+07f, 0);
path.lineTo(93, 0);
path.lineTo(93, 600);
path.close();
path.moveTo(123, 600);
path.lineTo(3.35546e+07f, 600);
path.lineTo(3.35546e+07f, 0);
path.lineTo(123, 0);
path.lineTo(123, 600);
path.close();
    testSimplifyFuzz(reporter, path, filename);
}

static void fuzz_twister2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;

path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x44160000));  // 0, 600
path.lineTo(SkBits2Float(0x4bfffffe), SkBits2Float(0x44160000));  // 3.35544e+07f, 600
path.lineTo(SkBits2Float(0x4bfffffe), SkBits2Float(0x00000000));  // 3.35544e+07f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x44160000));  // 0, 600
path.close();

path.moveTo(SkBits2Float(0x427c0000), SkBits2Float(0x00000000));  // 63, 0
path.lineTo(SkBits2Float(0x4c00000f), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x4c00000f), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x427c0000), SkBits2Float(0x00000000));  // 63, 0
path.close();

path.moveTo(SkBits2Float(0x42ba0000), SkBits2Float(0x00000000));  // 93, 0
path.lineTo(SkBits2Float(0x4c000016), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x4c000016), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x42ba0000), SkBits2Float(0x00000000));  // 93, 0
path.close();

path.moveTo(SkBits2Float(0x42f60000), SkBits2Float(0x00000000));  // 123, 0
path.lineTo(SkBits2Float(0x4c00001e), SkBits2Float(0x00000000));  // 3.35546e+07f, 0
path.lineTo(SkBits2Float(0x4c00001e), SkBits2Float(0x00000000));  // 3.35546e+07f, 0
path.lineTo(SkBits2Float(0x42f60000), SkBits2Float(0x00000000));  // 123, 0
path.close();

    testSimplifyFuzz(reporter, path, filename);
}

static void fuzz994s_11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x42b40000));  // 110, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x46ff4c00), SkBits2Float(0x41f00000));  // 32678, 30
path.lineTo(SkBits2Float(0x46ff4c00), SkBits2Float(0x41f00000));  // 32678, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x4c000006));  // 110, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x439d8000));  // 10, 315
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x439d8000));  // 110, 315
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x437f0000));  // 110, 255
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x437f0000));  // 10, 255
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x439d8000));  // 10, 315
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x42700000));  // 0, 60
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x42700000));  // 100, 60
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x00000000));  // 100, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x42700000));  // 0, 60
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x42b40000));  // 110, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x4c000006));  // 110, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x42b40000));  // 110, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();

    testSimplifyFuzz(reporter, path, filename);
}

static void fuzz994s_3414(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42c80000), SkBits2Float(0x42480000));  // 100, 50
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100, 0, 50, 0, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0, 0, 0, 50, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0, 100, 50, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100, 100, 100, 50, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42c84964), SkBits2Float(0x42480000));  // 100.143f, 50
path.conicTo(SkBits2Float(0x42c84964), SkBits2Float(0x00000000), SkBits2Float(0x424892c8), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100.143f, 0, 50.1433f, 0, 0.707107f
path.conicTo(SkBits2Float(0x3e12c788), SkBits2Float(0x00000000), SkBits2Float(0x3e12c788), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0.143339f, 0, 0.143339f, 50, 0.707107f
path.conicTo(SkBits2Float(0x3e12c788), SkBits2Float(0x42c80000), SkBits2Float(0x424892c8), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0.143339f, 100, 50.1433f, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c84964), SkBits2Float(0x42c80000), SkBits2Float(0x42c84964), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100.143f, 100, 100.143f, 50, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42c80000), SkBits2Float(0x42480000));  // 100, 50
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100, 0, 50, 0, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0, 0, 0, 50, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0, 100, 50, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100, 100, 100, 50, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x4c00006b), SkBits2Float(0x424c0000));  // 3.35549e+07f, 51
path.conicTo(SkBits2Float(0x4c00006b), SkBits2Float(0xcbffffe5), SkBits2Float(0x43d6e720), SkBits2Float(0xcbffffe5), SkBits2Float(0x3f3504f3));  // 3.35549e+07f, -3.35544e+07f, 429.806f, -3.35544e+07f, 0.707107f
path.conicTo(SkBits2Float(0xcbffff28), SkBits2Float(0xcbffffe5), SkBits2Float(0xcbffff28), SkBits2Float(0x424c0000), SkBits2Float(0x3f3504f3));  // -3.3554e+07f, -3.35544e+07f, -3.3554e+07f, 51, 0.707107f
path.conicTo(SkBits2Float(0xcbffff28), SkBits2Float(0x4c00000c), SkBits2Float(0x43d6e720), SkBits2Float(0x4c00000c), SkBits2Float(0x3f3504f3));  // -3.3554e+07f, 3.35545e+07f, 429.806f, 3.35545e+07f, 0.707107f
path.conicTo(SkBits2Float(0x4c00006b), SkBits2Float(0x4c00000c), SkBits2Float(0x4c00006b), SkBits2Float(0x424c0000), SkBits2Float(0x3f3504f3));  // 3.35549e+07f, 3.35545e+07f, 3.35549e+07f, 51, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43ef6720), SkBits2Float(0x42480000));  // 478.806f, 50
path.conicTo(SkBits2Float(0x43ef6720), SkBits2Float(0x00000000), SkBits2Float(0x43d66720), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 478.806f, 0, 428.806f, 0, 0.707107f
path.conicTo(SkBits2Float(0x43bd6720), SkBits2Float(0x00000000), SkBits2Float(0x43bd6720), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 378.806f, 0, 378.806f, 50, 0.707107f
path.conicTo(SkBits2Float(0x43bd6720), SkBits2Float(0x42c80000), SkBits2Float(0x43d66720), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 378.806f, 100, 428.806f, 100, 0.707107f
path.conicTo(SkBits2Float(0x43ef6720), SkBits2Float(0x42c80000), SkBits2Float(0x43ef6720), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 478.806f, 100, 478.806f, 50, 0.707107f
path.close();

    testSimplify(reporter, path, filename);
}

static void fuzz763_4713_b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.close();

path.moveTo(SkBits2Float(0x4204f72e), SkBits2Float(0x41c56cd2));
path.quadTo(SkBits2Float(0x42123842), SkBits2Float(0x41c52adf), SkBits2Float(0x421baed7), SkBits2Float(0x41d7bac6));
path.quadTo(SkBits2Float(0x4225256d), SkBits2Float(0x41ea4aad), SkBits2Float(0x42254667), SkBits2Float(0x4202666b));
path.quadTo(SkBits2Float(0x42256760), SkBits2Float(0x420fa77f), SkBits2Float(0x421c1f6c), SkBits2Float(0x42191e14));
path.quadTo(SkBits2Float(0x421bff97), SkBits2Float(0x42193e89), SkBits2Float(0x421bdf6b), SkBits2Float(0x42195eb8));
path.quadTo(SkBits2Float(0x421bbff6), SkBits2Float(0x42197f32), SkBits2Float(0x421ba03b), SkBits2Float(0x42199f57));
path.quadTo(SkBits2Float(0x421b605e), SkBits2Float(0x4219e00a), SkBits2Float(0x421b1fa8), SkBits2Float(0x421a1f22));
path.quadTo(SkBits2Float(0x421ae0f1), SkBits2Float(0x421a604b), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41db19b1), SkBits2Float(0x41da63d5), SkBits2Float(0x41db755b), SkBits2Float(0x41da0a9b));
path.quadTo(SkBits2Float(0x41dbce01), SkBits2Float(0x41d9ae59), SkBits2Float(0x41dc285e), SkBits2Float(0x41d952ce));
path.quadTo(SkBits2Float(0x41dc55b6), SkBits2Float(0x41d924df), SkBits2Float(0x41dc82cd), SkBits2Float(0x41d8f7cd));
path.quadTo(SkBits2Float(0x41dcaf1e), SkBits2Float(0x41d8ca01), SkBits2Float(0x41dcdc4c), SkBits2Float(0x41d89bf0));
path.quadTo(SkBits2Float(0x41ef6c33), SkBits2Float(0x41c5aec5), SkBits2Float(0x4204f72e), SkBits2Float(0x41c56cd2));
path.close();
testSimplify(reporter, path, filename);
}

static void fuzz864a(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(10, 90);
    path.lineTo(10, 90);
    path.lineTo(10, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(10, 90);
    path.lineTo(10, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(110, 90);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 30);
    path.lineTo(32678, 30);
    path.lineTo(32678, 30);
    path.lineTo(10, 30);
    path.close();
    path.moveTo(10, 3.35545e+07f);
    path.lineTo(110, 3.35545e+07f);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 3.35545e+07f);
    path.close();
    path.moveTo(10, 315);
    path.lineTo(110, 315);
    path.lineTo(110, 255);
    path.lineTo(10, 255);
    path.lineTo(10, 315);
    path.close();
    path.moveTo(0, 60);
    path.lineTo(100, 60);
    path.lineTo(100, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 60);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(110, 90);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 3.35545e+07f);
    path.lineTo(110, 3.35545e+07f);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 3.35545e+07f);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(110, 90);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    testSimplifyFuzz(reporter, path, filename);
}

#define TEST(test) test(reporter, #test)

DEF_TEST(PathOpsSimplifyFail, reporter) {
    TEST(fuzz864a);
    TEST(fuzz763_4713_b);
    TEST(fuzz994s_3414);
    TEST(fuzz994s_11);
    TEST(fuzz_twister2);
    TEST(fuzz_twister);
    TEST(fuzz763_2);
    TEST(fuzz763_1);
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
