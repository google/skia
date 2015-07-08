/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsTestCommon.h"

#define TEST(name) { name, #name }

static void issue414409(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1, path2;

    // one fill 1
    path1.moveTo(9.53595e-07f, -60);
    path1.lineTo(5.08228e-15f, -83);
    path1.cubicTo(32.8673f, -83, 62.6386f, -63.6055f, 75.9208f, -33.5416f);
    path1.cubicTo(89.2029f, -3.47759f, 83.4937f, 31.5921f, 61.3615f, 55.8907f);
    path1.lineTo(46.9383f, 68.4529f);
    path1.lineTo(33.9313f, 49.484f);
    path1.cubicTo(37.7451f, 46.8689f, 41.2438f, 43.8216f, 44.3577f, 40.4029f);
    path1.lineTo(44.3577f, 40.4029f);
    path1.cubicTo(60.3569f, 22.8376f, 64.4841f, -2.51392f, 54.8825f, -24.2469f);
    path1.cubicTo(45.2809f, -45.9799f, 23.7595f, -60, 9.53595e-07f, -60);
    path1.close();

    //  two fill 0
    path2.moveTo(46.9383f, 68.4529f);
    path2.cubicTo(17.5117f, 88.6307f, -21.518f, 87.7442f, -49.9981f, 66.251f);
    path2.cubicTo(-78.4781f, 44.7578f, -90.035f, 7.46781f, -78.7014f, -26.3644f);
    path2.cubicTo(-67.3679f, -60.1967f, -35.6801f, -83, -1.48383e-06f, -83);
    path2.lineTo(4.22689e-14f, -60);
    path2.cubicTo(-25.7929f, -60, -48.6997f, -43.5157f, -56.8926f, -19.0586f);
    path2.cubicTo(-65.0855f, 5.39842f, -56.7312f, 32.355f, -36.1432f, 47.8923f);
    path2.cubicTo(-15.5552f, 63.4296f, 12.6591f, 64.0704f, 33.9313f, 49.484f);
    path2.lineTo(46.9383f, 68.4529f);
    path2.close();
    testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void issue414409b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1, path2;
    // one fill=0 op=2
path1.setFillType((SkPath::FillType) 0);
path1.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path1.cubicTo(SkBits2Float(0x41f12edc), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4267b362), SkBits2Float(0xc2854e1f), SkBits2Float(0x42911faa), SkBits2Float(0xc2212f3b));
path1.cubicTo(SkBits2Float(0x42ae65a2), SkBits2Float(0xc15f08de), SkBits2Float(0x42acc913), SkBits2Float(0x41923f59), SkBits2Float(0x428ce9f0), SkBits2Float(0x422f7dc4));
path1.lineTo(SkBits2Float(0x424bbb16), SkBits2Float(0x41fdb8ed));
path1.cubicTo(SkBits2Float(0x4279cf6e), SkBits2Float(0x41537137), SkBits2Float(0x427c23ea), SkBits2Float(0xc1213ad2), SkBits2Float(0x4251d142), SkBits2Float(0xc1e909ae));
path1.cubicTo(SkBits2Float(0x42277e9a), SkBits2Float(0xc240baf8), SkBits2Float(0x41ae5968), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path1.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path1.close();

path2.setFillType((SkPath::FillType) 1);
path2.moveTo(SkBits2Float(0x428ce9ef), SkBits2Float(0x422f7dc6));
path2.cubicTo(SkBits2Float(0x4286af43), SkBits2Float(0x42437fa7), SkBits2Float(0x427ed0d6), SkBits2Float(0x42561f5a), SkBits2Float(0x426e69d2), SkBits2Float(0x42670c39));
path2.lineTo(SkBits2Float(0x422c58d6), SkBits2Float(0x422705c1));
path2.cubicTo(SkBits2Float(0x42383446), SkBits2Float(0x421ac98f), SkBits2Float(0x4242b98a), SkBits2Float(0x420d5308), SkBits2Float(0x424bbb17), SkBits2Float(0x41fdb8ee));
path2.lineTo(SkBits2Float(0x428ce9ef), SkBits2Float(0x422f7dc6));
path2.close();
    testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void issue414409c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1, path2;
path1.setFillType((SkPath::FillType) 1);
path1.moveTo(SkBits2Float(0x36961ef0), SkBits2Float(0xc2700000));
path1.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path1.cubicTo(SkBits2Float(0x3df86648), SkBits2Float(0xc2a60000), SkBits2Float(0x3e786777), SkBits2Float(0xc2a5ffdc), SkBits2Float(0x3eba4dc2), SkBits2Float(0xc2a5ff96));
path1.lineTo(SkBits2Float(0x3eba4dc3), SkBits2Float(0xc2a5ff97));
path1.cubicTo(SkBits2Float(0x3ec08370), SkBits2Float(0xc2a5ff8f), SkBits2Float(0x3ec6b964), SkBits2Float(0xc2a5ff88), SkBits2Float(0x3eccef58), SkBits2Float(0xc2a5ff80));
path1.lineTo(SkBits2Float(0x3e942522), SkBits2Float(0xc26fff49));
path1.cubicTo(SkBits2Float(0x3e8fa7da), SkBits2Float(0xc26fff56), SkBits2Float(0x3e8b2acd), SkBits2Float(0xc26fff61), SkBits2Float(0x3e86adc0), SkBits2Float(0xc26fff6b));
path1.lineTo(SkBits2Float(0x3e86ad6a), SkBits2Float(0xc26fff69));
path1.cubicTo(SkBits2Float(0x3e3391e9), SkBits2Float(0xc26fffce), SkBits2Float(0x3db3931e), SkBits2Float(0xc2700000), SkBits2Float(0x36961ef0), SkBits2Float(0xc2700000));
path1.close();

path2.setFillType((SkPath::FillType) 0);
path2.moveTo(SkBits2Float(0x3eccef1a), SkBits2Float(0xc2a5ff81));
path2.cubicTo(SkBits2Float(0x3f18c8a9), SkBits2Float(0xc2a5ff04), SkBits2Float(0x3f4b19b0), SkBits2Float(0xc2a5fe2d), SkBits2Float(0x3f7d6a37), SkBits2Float(0xc2a5fcfa));
path2.lineTo(SkBits2Float(0x3f3730f2), SkBits2Float(0xc26ffba1));
path2.cubicTo(SkBits2Float(0x3f12d1c8), SkBits2Float(0xc26ffd5d), SkBits2Float(0x3edce4b4), SkBits2Float(0xc26ffe95), SkBits2Float(0x3e942577), SkBits2Float(0xc26fff49));
path2.lineTo(SkBits2Float(0x3eccef1a), SkBits2Float(0xc2a5ff81));
path2.close();

testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

// fails to draw correctly
static void battleOp1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ea4d9f5), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f24d9a9), SkBits2Float(0xc2a5ff0a), SkBits2Float(0x3f774519), SkBits2Float(0xc2a5fd1f));
path.lineTo(SkBits2Float(0x3f32bfc3), SkBits2Float(0xc26ffbd7));
path.cubicTo(SkBits2Float(0x3eee5669), SkBits2Float(0xc26ffe9e), SkBits2Float(0x3e6e56cc), SkBits2Float(0xc2700000), SkBits2Float(0x357ffb40), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f774503), SkBits2Float(0xc2a5fd1f));
path.cubicTo(SkBits2Float(0x3f7f82ff), SkBits2Float(0xc2a5fcee), SkBits2Float(0x3f83e06d), SkBits2Float(0xc2a5fcbb), SkBits2Float(0x3f87ff59), SkBits2Float(0xc2a5fc85));
path.lineTo(SkBits2Float(0x3f449f80), SkBits2Float(0xc26ffaf7));
path.cubicTo(SkBits2Float(0x3f3eaa52), SkBits2Float(0xc26ffb47), SkBits2Float(0x3f38b4f5), SkBits2Float(0xc26ffb92), SkBits2Float(0x3f32bf98), SkBits2Float(0xc26ffbd9));
path.lineTo(SkBits2Float(0x3f774503), SkBits2Float(0xc2a5fd1f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ea4d9e6), SkBits2Float(0xc2a60000), SkBits2Float(0x3f24d99a), SkBits2Float(0xc2a5ff0a), SkBits2Float(0x3f774503), SkBits2Float(0xc2a5fd1f));

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f87ff64), SkBits2Float(0xc2a5fc85));
path.cubicTo(SkBits2Float(0x3fcac720), SkBits2Float(0xc2a5f91a), SkBits2Float(0x4006c62a), SkBits2Float(0xc2a5f329), SkBits2Float(0x40282667), SkBits2Float(0xc2a5eab4));
path.lineTo(SkBits2Float(0x3ff31bb9), SkBits2Float(0xc26fe136));
path.cubicTo(SkBits2Float(0x3fc2da88), SkBits2Float(0xc26fed71), SkBits2Float(0x3f9295ff), SkBits2Float(0xc26ff607), SkBits2Float(0x3f449f66), SkBits2Float(0xc26ffaf9));
path.lineTo(SkBits2Float(0x3f87ff64), SkBits2Float(0xc2a5fc85));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f19f03c), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f99ef95), SkBits2Float(0xc2a5fca7), SkBits2Float(0x3fe6e2fa), SkBits2Float(0xc2a5f5f7));
path.lineTo(SkBits2Float(0x3fa6e80c), SkBits2Float(0xc26ff17d));
path.cubicTo(SkBits2Float(0x3f5e8ed4), SkBits2Float(0xc26ffb2a), SkBits2Float(0x3ede8fc6), SkBits2Float(0xc2700000), SkBits2Float(0x35d9fd64), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fe6e322), SkBits2Float(0xc2a5f5f7));
path.cubicTo(SkBits2Float(0x3fee94fb), SkBits2Float(0xc2a5f54c), SkBits2Float(0x3ff646db), SkBits2Float(0xc2a5f497), SkBits2Float(0x3ffdf8ad), SkBits2Float(0xc2a5f3db));
path.lineTo(SkBits2Float(0x3fb79813), SkBits2Float(0xc26fee71));
path.cubicTo(SkBits2Float(0x3fb20800), SkBits2Float(0xc26fef82), SkBits2Float(0x3fac77ff), SkBits2Float(0xc26ff085), SkBits2Float(0x3fa6e7f4), SkBits2Float(0xc26ff17d));
path.lineTo(SkBits2Float(0x3fe6e322), SkBits2Float(0xc2a5f5f7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f19f03c), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f99ef95), SkBits2Float(0xc2a5fca7), SkBits2Float(0x3fe6e322), SkBits2Float(0xc2a5f5f7));
path.cubicTo(SkBits2Float(0x3fee94fb), SkBits2Float(0xc2a5f54c), SkBits2Float(0x3ff646db), SkBits2Float(0xc2a5f497), SkBits2Float(0x3ffdf8ad), SkBits2Float(0xc2a5f3db));
path.lineTo(SkBits2Float(0x3fb79813), SkBits2Float(0xc26fee71));
path.cubicTo(SkBits2Float(0x3fb20808), SkBits2Float(0xc26fef82), SkBits2Float(0x3fac780f), SkBits2Float(0xc26ff085), SkBits2Float(0x3fa6e80c), SkBits2Float(0xc26ff17d));
path.lineTo(SkBits2Float(0x3fa6e7f4), SkBits2Float(0xc26ff17d));
path.cubicTo(SkBits2Float(0x3f5e8eb4), SkBits2Float(0xc26ffb2a), SkBits2Float(0x3ede8fa6), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ffdf8c6), SkBits2Float(0xc2a5f3db));
path.cubicTo(SkBits2Float(0x403d5556), SkBits2Float(0xc2a5e7ed), SkBits2Float(0x407ba65a), SkBits2Float(0xc2a5d338), SkBits2Float(0x409cf3fe), SkBits2Float(0xc2a5b5bc));
path.lineTo(SkBits2Float(0x4062eb8a), SkBits2Float(0xc26f94a1));
path.cubicTo(SkBits2Float(0x4035ea63), SkBits2Float(0xc26fbf44), SkBits2Float(0x4008de16), SkBits2Float(0xc26fdd35), SkBits2Float(0x3fb79810), SkBits2Float(0xc26fee74));
path.lineTo(SkBits2Float(0x3ffdf8c6), SkBits2Float(0xc2a5f3db));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3fe06a9b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40606368), SkBits2Float(0xc2a5e38e), SkBits2Float(0x40a82f8a), SkBits2Float(0xc2a5aab6));
path.lineTo(SkBits2Float(0x40732902), SkBits2Float(0xc26f84b2));
path.cubicTo(SkBits2Float(0x4022355b), SkBits2Float(0xc26fd6e1), SkBits2Float(0x3fa23a8f), SkBits2Float(0xc2700000), SkBits2Float(0xb5600574), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40a82f91), SkBits2Float(0xc2a5aab7));
path.cubicTo(SkBits2Float(0x40adc8dc), SkBits2Float(0xc2a5a508), SkBits2Float(0x40b361d8), SkBits2Float(0xc2a59f10), SkBits2Float(0x40b8fa82), SkBits2Float(0xc2a598d0));
path.lineTo(SkBits2Float(0x4085b825), SkBits2Float(0xc26f6ad0));
path.cubicTo(SkBits2Float(0x4081ac7b), SkBits2Float(0xc26f73dc), SkBits2Float(0x407b412c), SkBits2Float(0xc26f7c7c), SkBits2Float(0x407328f8), SkBits2Float(0xc26f84b3));
path.lineTo(SkBits2Float(0x40a82f91), SkBits2Float(0xc2a5aab7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3fe06a9b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40606368), SkBits2Float(0xc2a5e38e), SkBits2Float(0x40a82f91), SkBits2Float(0xc2a5aab7));
path.cubicTo(SkBits2Float(0x40adc8dc), SkBits2Float(0xc2a5a508), SkBits2Float(0x40b361d8), SkBits2Float(0xc2a59f10), SkBits2Float(0x40b8fa82), SkBits2Float(0xc2a598d0));
path.lineTo(SkBits2Float(0x4085b825), SkBits2Float(0xc26f6ad0));
path.cubicTo(SkBits2Float(0x4081ac7d), SkBits2Float(0xc26f73dc), SkBits2Float(0x407b4133), SkBits2Float(0xc26f7c7c), SkBits2Float(0x40732902), SkBits2Float(0xc26f84b2));
path.cubicTo(SkBits2Float(0x4022355b), SkBits2Float(0xc26fd6e1), SkBits2Float(0x3fa23a8f), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();
path.moveTo(SkBits2Float(0x408fea52), SkBits2Float(0xc28dc28a));
path.lineTo(SkBits2Float(0x407328f8), SkBits2Float(0xc26f84b3));
path.lineTo(SkBits2Float(0x40732903), SkBits2Float(0xc26f84b3));
path.lineTo(SkBits2Float(0x408fea52), SkBits2Float(0xc28dc28a));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40b8fa77), SkBits2Float(0xc2a598d0));
path.cubicTo(SkBits2Float(0x4109d7e9), SkBits2Float(0xc2a5337c), SkBits2Float(0x4137014a), SkBits2Float(0xc2a483b2), SkBits2Float(0x4163cbb6), SkBits2Float(0xc2a38a24));
path.lineTo(SkBits2Float(0x4124abf0), SkBits2Float(0xc26c715c));
path.cubicTo(SkBits2Float(0x41044af8), SkBits2Float(0xc26dda2b), SkBits2Float(0x40c74ab0), SkBits2Float(0xc26ed852), SkBits2Float(0x4085b82e), SkBits2Float(0xc26f6ad1));
path.lineTo(SkBits2Float(0x40b8fa77), SkBits2Float(0xc2a598d0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3de5c884), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e65c882), SkBits2Float(0xc2a5ffe2), SkBits2Float(0x3eac5645), SkBits2Float(0xc2a5ffa7));
path.lineTo(SkBits2Float(0x3e79297e), SkBits2Float(0xc26fff7f));
path.cubicTo(SkBits2Float(0x3e261bbd), SkBits2Float(0xc26fffd7), SkBits2Float(0x3da61bbf), SkBits2Float(0xc2700000), SkBits2Float(0xb3244c00), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3eac564d), SkBits2Float(0xc2a5ffa7));
path.cubicTo(SkBits2Float(0x3eb21458), SkBits2Float(0xc2a5ffa1), SkBits2Float(0x3eb7d2fc), SkBits2Float(0xc2a5ff9b), SkBits2Float(0x3ebd91a0), SkBits2Float(0xc2a5ff94));
path.lineTo(SkBits2Float(0x3e8909ff), SkBits2Float(0xc26fff64));
path.cubicTo(SkBits2Float(0x3e84e2cf), SkBits2Float(0xc26fff6d), SkBits2Float(0x3e80bc02), SkBits2Float(0xc26fff76), SkBits2Float(0x3e792a69), SkBits2Float(0xc26fff7f));
path.lineTo(SkBits2Float(0x3eac564d), SkBits2Float(0xc2a5ffa7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3de5c884), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e65c882), SkBits2Float(0xc2a5ffe2), SkBits2Float(0x3eac564d), SkBits2Float(0xc2a5ffa7));
path.cubicTo(SkBits2Float(0x3eb21458), SkBits2Float(0xc2a5ffa1), SkBits2Float(0x3eb7d2fc), SkBits2Float(0xc2a5ff9b), SkBits2Float(0x3ebd91a0), SkBits2Float(0xc2a5ff94));
path.lineTo(SkBits2Float(0x3e8909ff), SkBits2Float(0xc26fff64));
path.lineTo(SkBits2Float(0x3e792a69), SkBits2Float(0xc26fff7f));
path.cubicTo(SkBits2Float(0x3e261bbd), SkBits2Float(0xc26fffd7), SkBits2Float(0x3da61bbf), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ebd921a), SkBits2Float(0xc2a5ff94));
path.cubicTo(SkBits2Float(0x3f0d545f), SkBits2Float(0xc2a5ff29), SkBits2Float(0x3f3bdfbd), SkBits2Float(0xc2a5fe71), SkBits2Float(0x3f6a6ab6), SkBits2Float(0xc2a5fd69));
path.lineTo(SkBits2Float(0x3f297558), SkBits2Float(0xc26ffc43));
path.cubicTo(SkBits2Float(0x3f07d00d), SkBits2Float(0xc26ffdc0), SkBits2Float(0x3ecc550f), SkBits2Float(0xc26ffecc), SkBits2Float(0x3e8909b7), SkBits2Float(0xc26fff65));
path.lineTo(SkBits2Float(0x3ebd921a), SkBits2Float(0xc2a5ff94));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp9(skiatest::Reporter* reporter, const char* filename) { // crashes
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ecc43bf), SkBits2Float(0xc2a60000), SkBits2Float(0x3f4c4385), SkBits2Float(0xc2a5fe87), SkBits2Float(0x3f993163), SkBits2Float(0xc2a5fb95));
path.lineTo(SkBits2Float(0x3f5d7bc4), SkBits2Float(0xc26ff99d));
path.cubicTo(SkBits2Float(0x3f13a919), SkBits2Float(0xc26ffdde), SkBits2Float(0x3e93a998), SkBits2Float(0xc26fffff), SkBits2Float(0x367b7ed0), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f993156), SkBits2Float(0xc2a5fb95));
path.cubicTo(SkBits2Float(0x3f9e4c7a), SkBits2Float(0xc2a5fb49), SkBits2Float(0x3fa36794), SkBits2Float(0xc2a5fafa), SkBits2Float(0x3fa882aa), SkBits2Float(0xc2a5faa7));
path.lineTo(SkBits2Float(0x3f73a149), SkBits2Float(0xc26ff845));
path.cubicTo(SkBits2Float(0x3f6c3f64), SkBits2Float(0xc26ff8bf), SkBits2Float(0x3f64dd9d), SkBits2Float(0xc26ff931), SkBits2Float(0x3f5d7bcf), SkBits2Float(0xc26ff99f));
path.lineTo(SkBits2Float(0x3f993156), SkBits2Float(0xc2a5fb95));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ddcd524), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e5cd462), SkBits2Float(0xc2a5ffe3), SkBits2Float(0x3ea59eff), SkBits2Float(0xc2a5ffac));
path.lineTo(SkBits2Float(0x3e6f74a3), SkBits2Float(0xc26fff89));
path.cubicTo(SkBits2Float(0x3e1fa33e), SkBits2Float(0xc26fffd9), SkBits2Float(0x3d9fa303), SkBits2Float(0xc2700000), SkBits2Float(0xb580e440), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ea59f9c), SkBits2Float(0xc2a5ffad));
path.cubicTo(SkBits2Float(0x3eab24c0), SkBits2Float(0xc2a5ffa7), SkBits2Float(0x3eb0aa54), SkBits2Float(0xc2a5ffa1), SkBits2Float(0x3eb62fe9), SkBits2Float(0xc2a5ff9b));
path.lineTo(SkBits2Float(0x3e83b355), SkBits2Float(0xc26fff6f));
path.cubicTo(SkBits2Float(0x3e7f6bdb), SkBits2Float(0xc26fff79), SkBits2Float(0x3e777021), SkBits2Float(0xc26fff81), SkBits2Float(0x3e6f7465), SkBits2Float(0xc26fff8a));
path.lineTo(SkBits2Float(0x3ea59f9c), SkBits2Float(0xc2a5ffad));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ddcd524), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e5cd462), SkBits2Float(0xc2a5ffe3), SkBits2Float(0x3ea59f9c), SkBits2Float(0xc2a5ffad));
path.lineTo(SkBits2Float(0x3eb62fe9), SkBits2Float(0xc2a5ff9b));
path.lineTo(SkBits2Float(0x3e83b355), SkBits2Float(0xc26fff6f));
path.cubicTo(SkBits2Float(0x3e7f6bf0), SkBits2Float(0xc26fff79), SkBits2Float(0x3e77704b), SkBits2Float(0xc26fff81), SkBits2Float(0x3e6f74a3), SkBits2Float(0xc26fff89));
path.cubicTo(SkBits2Float(0x3e1fa33e), SkBits2Float(0xc26fffd9), SkBits2Float(0x3d9fa303), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();
path.moveTo(SkBits2Float(0x3e7ee007), SkBits2Float(0xc27f7413));
path.lineTo(SkBits2Float(0x3e6f7465), SkBits2Float(0xc26fff8a));
path.lineTo(SkBits2Float(0x3e6f74a4), SkBits2Float(0xc26fff8a));
path.lineTo(SkBits2Float(0x3e7ee007), SkBits2Float(0xc27f7413));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3eb62f8c), SkBits2Float(0xc2a5ff9c));
path.cubicTo(SkBits2Float(0x3f07d31d), SkBits2Float(0xc2a5ff3a), SkBits2Float(0x3f348e3e), SkBits2Float(0xc2a5fe8f), SkBits2Float(0x3f614904), SkBits2Float(0xc2a5fd9c));
path.lineTo(SkBits2Float(0x3f22db6c), SkBits2Float(0xc26ffc8c));
path.cubicTo(SkBits2Float(0x3f0285bf), SkBits2Float(0xc26ffdeb), SkBits2Float(0x3ec45fa5), SkBits2Float(0xc26ffee1), SkBits2Float(0x3e83b387), SkBits2Float(0xc26fff6f));
path.lineTo(SkBits2Float(0x3eb62f8c), SkBits2Float(0xc2a5ff9c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp12(skiatest::Reporter* reporter, const char* filename) {  // crashed
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ecc43bf), SkBits2Float(0xc2a60000), SkBits2Float(0x3f4c4385), SkBits2Float(0xc2a5fe87), SkBits2Float(0x3f993163), SkBits2Float(0xc2a5fb95));
path.lineTo(SkBits2Float(0x3f5d7bc4), SkBits2Float(0xc26ff99d));
path.cubicTo(SkBits2Float(0x3f13a919), SkBits2Float(0xc26ffdde), SkBits2Float(0x3e93a998), SkBits2Float(0xc26fffff), SkBits2Float(0x367b7ed0), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f993156), SkBits2Float(0xc2a5fb95));
path.cubicTo(SkBits2Float(0x3f9e4c7a), SkBits2Float(0xc2a5fb49), SkBits2Float(0x3fa36794), SkBits2Float(0xc2a5fafa), SkBits2Float(0x3fa882aa), SkBits2Float(0xc2a5faa7));
path.lineTo(SkBits2Float(0x3f73a149), SkBits2Float(0xc26ff845));
path.cubicTo(SkBits2Float(0x3f6c3f64), SkBits2Float(0xc26ff8bf), SkBits2Float(0x3f64dd9d), SkBits2Float(0xc26ff931), SkBits2Float(0x3f5d7bcf), SkBits2Float(0xc26ff99f));
path.lineTo(SkBits2Float(0x3f993156), SkBits2Float(0xc2a5fb95));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// ../../third_party/tcmalloc/chromium/src/free_list.h:118] Memory corruption detected. 

static void battleOp13(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ddcd524), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e5cd462), SkBits2Float(0xc2a5ffe3), SkBits2Float(0x3ea59eff), SkBits2Float(0xc2a5ffac));
path.lineTo(SkBits2Float(0x3e6f74a3), SkBits2Float(0xc26fff89));
path.cubicTo(SkBits2Float(0x3e1fa33e), SkBits2Float(0xc26fffd9), SkBits2Float(0x3d9fa303), SkBits2Float(0xc2700000), SkBits2Float(0xb580e440), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ea59f9c), SkBits2Float(0xc2a5ffad));
path.cubicTo(SkBits2Float(0x3eab24c0), SkBits2Float(0xc2a5ffa7), SkBits2Float(0x3eb0aa54), SkBits2Float(0xc2a5ffa1), SkBits2Float(0x3eb62fe9), SkBits2Float(0xc2a5ff9b));
path.lineTo(SkBits2Float(0x3e83b355), SkBits2Float(0xc26fff6f));
path.cubicTo(SkBits2Float(0x3e7f6bdb), SkBits2Float(0xc26fff79), SkBits2Float(0x3e777021), SkBits2Float(0xc26fff81), SkBits2Float(0x3e6f7465), SkBits2Float(0xc26fff8a));
path.lineTo(SkBits2Float(0x3ea59f9c), SkBits2Float(0xc2a5ffad));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp14(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ddcd524), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e5cd462), SkBits2Float(0xc2a5ffe3), SkBits2Float(0x3ea59f9c), SkBits2Float(0xc2a5ffad));
path.lineTo(SkBits2Float(0x3eb62fe9), SkBits2Float(0xc2a5ff9b));
path.lineTo(SkBits2Float(0x3e83b355), SkBits2Float(0xc26fff6f));
path.cubicTo(SkBits2Float(0x3e7f6bf0), SkBits2Float(0xc26fff79), SkBits2Float(0x3e77704b), SkBits2Float(0xc26fff81), SkBits2Float(0x3e6f74a3), SkBits2Float(0xc26fff89));
path.cubicTo(SkBits2Float(0x3e1fa33e), SkBits2Float(0xc26fffd9), SkBits2Float(0x3d9fa303), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();
path.moveTo(SkBits2Float(0x3e7ee007), SkBits2Float(0xc27f7413));
path.lineTo(SkBits2Float(0x3e6f7465), SkBits2Float(0xc26fff8a));
path.lineTo(SkBits2Float(0x3e6f74a4), SkBits2Float(0xc26fff8a));
path.lineTo(SkBits2Float(0x3e7ee007), SkBits2Float(0xc27f7413));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3eb62f8c), SkBits2Float(0xc2a5ff9c));
path.cubicTo(SkBits2Float(0x3f07d31d), SkBits2Float(0xc2a5ff3a), SkBits2Float(0x3f348e3e), SkBits2Float(0xc2a5fe8f), SkBits2Float(0x3f614904), SkBits2Float(0xc2a5fd9c));
path.lineTo(SkBits2Float(0x3f22db6c), SkBits2Float(0xc26ffc8c));
path.cubicTo(SkBits2Float(0x3f0285bf), SkBits2Float(0xc26ffdeb), SkBits2Float(0x3ec45fa5), SkBits2Float(0xc26ffee1), SkBits2Float(0x3e83b387), SkBits2Float(0xc26fff6f));
path.lineTo(SkBits2Float(0x3eb62f8c), SkBits2Float(0xc2a5ff9c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f19f03c), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f99ef95), SkBits2Float(0xc2a5fca7), SkBits2Float(0x3fe6e2fa), SkBits2Float(0xc2a5f5f7));
path.lineTo(SkBits2Float(0x3fa6e80c), SkBits2Float(0xc26ff17d));
path.cubicTo(SkBits2Float(0x3f5e8ed4), SkBits2Float(0xc26ffb2a), SkBits2Float(0x3ede8fc6), SkBits2Float(0xc2700000), SkBits2Float(0x35d9fd64), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fe6e322), SkBits2Float(0xc2a5f5f7));
path.cubicTo(SkBits2Float(0x3fee94fb), SkBits2Float(0xc2a5f54c), SkBits2Float(0x3ff646db), SkBits2Float(0xc2a5f497), SkBits2Float(0x3ffdf8ad), SkBits2Float(0xc2a5f3db));
path.lineTo(SkBits2Float(0x3fb79813), SkBits2Float(0xc26fee71));
path.cubicTo(SkBits2Float(0x3fb20800), SkBits2Float(0xc26fef82), SkBits2Float(0x3fac77ff), SkBits2Float(0xc26ff085), SkBits2Float(0x3fa6e7f4), SkBits2Float(0xc26ff17d));
path.lineTo(SkBits2Float(0x3fe6e322), SkBits2Float(0xc2a5f5f7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp16(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f19f03c), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f99ef95), SkBits2Float(0xc2a5fca7), SkBits2Float(0x3fe6e322), SkBits2Float(0xc2a5f5f7));
path.cubicTo(SkBits2Float(0x3fee94fb), SkBits2Float(0xc2a5f54c), SkBits2Float(0x3ff646db), SkBits2Float(0xc2a5f497), SkBits2Float(0x3ffdf8ad), SkBits2Float(0xc2a5f3db));
path.lineTo(SkBits2Float(0x3fb79813), SkBits2Float(0xc26fee71));
path.cubicTo(SkBits2Float(0x3fb20808), SkBits2Float(0xc26fef82), SkBits2Float(0x3fac780f), SkBits2Float(0xc26ff085), SkBits2Float(0x3fa6e80c), SkBits2Float(0xc26ff17d));
path.lineTo(SkBits2Float(0x3fa6e7f4), SkBits2Float(0xc26ff17d));
path.cubicTo(SkBits2Float(0x3f5e8eb4), SkBits2Float(0xc26ffb2a), SkBits2Float(0x3ede8fa6), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ffdf8c6), SkBits2Float(0xc2a5f3db));
path.cubicTo(SkBits2Float(0x403d5556), SkBits2Float(0xc2a5e7ed), SkBits2Float(0x407ba65a), SkBits2Float(0xc2a5d338), SkBits2Float(0x409cf3fe), SkBits2Float(0xc2a5b5bc));
path.lineTo(SkBits2Float(0x4062eb8a), SkBits2Float(0xc26f94a1));
path.cubicTo(SkBits2Float(0x4035ea63), SkBits2Float(0xc26fbf44), SkBits2Float(0x4008de16), SkBits2Float(0xc26fdd35), SkBits2Float(0x3fb79810), SkBits2Float(0xc26fee74));
path.lineTo(SkBits2Float(0x3ffdf8c6), SkBits2Float(0xc2a5f3db));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp17(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f9860dc), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40185ea2), SkBits2Float(0xc2a5f2e2), SkBits2Float(0x40647d09), SkBits2Float(0xc2a5d8aa));
path.lineTo(SkBits2Float(0x40252c2a), SkBits2Float(0xc26fc723));
path.cubicTo(SkBits2Float(0x3fdc4b47), SkBits2Float(0xc26fed09), SkBits2Float(0x3f5c4ea6), SkBits2Float(0xc26ffffe), SkBits2Float(0x3664fea3), SkBits2Float(0xc26ffffe));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40647d17), SkBits2Float(0xc2a5d8ab));
path.cubicTo(SkBits2Float(0x406c19ae), SkBits2Float(0xc2a5d60b), SkBits2Float(0x4073b608), SkBits2Float(0xc2a5d34a), SkBits2Float(0x407b5230), SkBits2Float(0xc2a5d069));
path.lineTo(SkBits2Float(0x4035ad90), SkBits2Float(0xc26fbb32));
path.cubicTo(SkBits2Float(0x40302d3b), SkBits2Float(0xc26fbf5d), SkBits2Float(0x402aacbf), SkBits2Float(0xc26fc358), SkBits2Float(0x40252c21), SkBits2Float(0xc26fc722));
path.lineTo(SkBits2Float(0x40647d17), SkBits2Float(0xc2a5d8ab));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp18(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3664fea3), SkBits2Float(0xc26ffffe));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f9860dc), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40185ea2), SkBits2Float(0xc2a5f2e2), SkBits2Float(0x40647d17), SkBits2Float(0xc2a5d8ab));
path.cubicTo(SkBits2Float(0x406c19ae), SkBits2Float(0xc2a5d60b), SkBits2Float(0x4073b608), SkBits2Float(0xc2a5d34a), SkBits2Float(0x407b5230), SkBits2Float(0xc2a5d069));
path.lineTo(SkBits2Float(0x4035ad90), SkBits2Float(0xc26fbb32));
path.cubicTo(SkBits2Float(0x40302d3b), SkBits2Float(0xc26fbf5d), SkBits2Float(0x402aacbf), SkBits2Float(0xc26fc358), SkBits2Float(0x40252c2a), SkBits2Float(0xc26fc723));
path.cubicTo(SkBits2Float(0x3fdc4b47), SkBits2Float(0xc26fed09), SkBits2Float(0x3f5c4ea6), SkBits2Float(0xc26ffffe), SkBits2Float(0x3664fea3), SkBits2Float(0xc26ffffe));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x407b523a), SkBits2Float(0xc2a5d069));
path.cubicTo(SkBits2Float(0x40bb53e8), SkBits2Float(0xc2a5a1ad), SkBits2Float(0x40f8dfd1), SkBits2Float(0xc2a5508e), SkBits2Float(0x411b1813), SkBits2Float(0xc2a4dd32));
path.lineTo(SkBits2Float(0x40e03b7c), SkBits2Float(0xc26e5b8f));
path.cubicTo(SkBits2Float(0x40b3e8bb), SkBits2Float(0xc26f0259), SkBits2Float(0x40876aeb), SkBits2Float(0xc26f77a1), SkBits2Float(0x4035ad92), SkBits2Float(0xc26fbb33));
path.lineTo(SkBits2Float(0x407b523a), SkBits2Float(0xc2a5d069));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp19(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40272e66), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40a7227d), SkBits2Float(0xc2a5c0db), SkBits2Float(0x40fa5a70), SkBits2Float(0xc2a542ca));
path.lineTo(SkBits2Float(0x40b4fa6e), SkBits2Float(0xc26eee73));
path.cubicTo(SkBits2Float(0x4071a3f5), SkBits2Float(0xc26fa4b8), SkBits2Float(0x3ff1b53c), SkBits2Float(0xc2700000), SkBits2Float(0x359dfd46), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40fa5a6d), SkBits2Float(0xc2a542cb));
path.cubicTo(SkBits2Float(0x4101563b), SkBits2Float(0xc2a5362f), SkBits2Float(0x41057ec0), SkBits2Float(0xc2a528f4), SkBits2Float(0x4109a6c0), SkBits2Float(0xc2a51b18));
path.lineTo(SkBits2Float(0x40c70391), SkBits2Float(0xc26eb50e));
path.cubicTo(SkBits2Float(0x40c10142), SkBits2Float(0xc26ec918), SkBits2Float(0x40bafe32), SkBits2Float(0xc26edc3a), SkBits2Float(0x40b4fa70), SkBits2Float(0xc26eee73));
path.lineTo(SkBits2Float(0x40fa5a6d), SkBits2Float(0xc2a542cb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp20(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40272e63), SkBits2Float(0xc2a60000), SkBits2Float(0x40a7227a), SkBits2Float(0xc2a5c0db), SkBits2Float(0x40fa5a6c), SkBits2Float(0xc2a542ca));
path.lineTo(SkBits2Float(0x40fa5a6d), SkBits2Float(0xc2a542cb));
path.cubicTo(SkBits2Float(0x4101563b), SkBits2Float(0xc2a5362f), SkBits2Float(0x41057ec0), SkBits2Float(0xc2a528f4), SkBits2Float(0x4109a6c0), SkBits2Float(0xc2a51b18));
path.lineTo(SkBits2Float(0x40c70391), SkBits2Float(0xc26eb50e));
path.cubicTo(SkBits2Float(0x40c10142), SkBits2Float(0xc26ec918), SkBits2Float(0x40bafe32), SkBits2Float(0xc26edc3a), SkBits2Float(0x40b4fa6e), SkBits2Float(0xc26eee73));
path.cubicTo(SkBits2Float(0x4071a3f5), SkBits2Float(0xc26fa4b8), SkBits2Float(0x3ff1b53c), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4109a6bc), SkBits2Float(0xc2a51b19));
path.cubicTo(SkBits2Float(0x414d093d), SkBits2Float(0xc2a43a61), SkBits2Float(0x4187e474), SkBits2Float(0xc2a2b4fa), SkBits2Float(0x41a8a805), SkBits2Float(0xc2a08e4d));
path.lineTo(SkBits2Float(0x4173d72c), SkBits2Float(0xc2682105));
path.cubicTo(SkBits2Float(0x41447890), SkBits2Float(0xc26b3d2d), SkBits2Float(0x4114380c), SkBits2Float(0xc26d702b), SkBits2Float(0x40c70392), SkBits2Float(0xc26eb510));
path.lineTo(SkBits2Float(0x4109a6bc), SkBits2Float(0xc2a51b19));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x404ef9c5), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40cee321), SkBits2Float(0xc2a59f3a), SkBits2Float(0x411ad5ab), SkBits2Float(0xc2a4de2c));
path.lineTo(SkBits2Float(0x40dfdb77), SkBits2Float(0xc26e5cf8));
path.cubicTo(SkBits2Float(0x40958e99), SkBits2Float(0xc26f7414), SkBits2Float(0x40159f04), SkBits2Float(0xc26ffffe), SkBits2Float(0x36ae7f52), SkBits2Float(0xc26ffffe));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x411ad5aa), SkBits2Float(0xc2a4de2c));
path.cubicTo(SkBits2Float(0x411ff8ea), SkBits2Float(0xc2a4cadf), SkBits2Float(0x41251b3e), SkBits2Float(0xc2a4b69c), SkBits2Float(0x412a3c98), SkBits2Float(0xc2a4a163));
path.lineTo(SkBits2Float(0x40f6200f), SkBits2Float(0xc26e0518));
path.cubicTo(SkBits2Float(0x40eeb53e), SkBits2Float(0xc26e23c6), SkBits2Float(0x40e74902), SkBits2Float(0xc26e4112), SkBits2Float(0x40dfdb73), SkBits2Float(0xc26e5cf8));
path.lineTo(SkBits2Float(0x411ad5aa), SkBits2Float(0xc2a4de2c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end fail 1

static void battleOp22(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x407fb41a), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40ff895b), SkBits2Float(0xc2a56c4b), SkBits2Float(0x413f077c), SkBits2Float(0xc2a44609));
path.lineTo(SkBits2Float(0x410a17ee), SkBits2Float(0xc26d8104));
path.cubicTo(SkBits2Float(0x40b8b9ab), SkBits2Float(0xc26f2a74), SkBits2Float(0x4038d88b), SkBits2Float(0xc2700000), SkBits2Float(0x337fa8c0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x413f0780), SkBits2Float(0xc2a44609));
path.cubicTo(SkBits2Float(0x41455a4a), SkBits2Float(0xc2a4289f), SkBits2Float(0x414bab5a), SkBits2Float(0xc2a409bf), SkBits2Float(0x4151fa92), SkBits2Float(0xc2a3e96b));
path.lineTo(SkBits2Float(0x4117cabb), SkBits2Float(0xc26cfb1d));
path.cubicTo(SkBits2Float(0x41133b1d), SkBits2Float(0xc26d29dc), SkBits2Float(0x410eaa27), SkBits2Float(0xc26d567f), SkBits2Float(0x410a17f1), SkBits2Float(0xc26d8105));
path.lineTo(SkBits2Float(0x413f0780), SkBits2Float(0xc2a44609));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp23(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x407fb41a), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40ff895b), SkBits2Float(0xc2a56c4b), SkBits2Float(0x413f0780), SkBits2Float(0xc2a44609));
path.cubicTo(SkBits2Float(0x41455a4a), SkBits2Float(0xc2a4289f), SkBits2Float(0x414bab5a), SkBits2Float(0xc2a409bf), SkBits2Float(0x4151fa92), SkBits2Float(0xc2a3e96b));
path.lineTo(SkBits2Float(0x4117cabb), SkBits2Float(0xc26cfb1d));
path.cubicTo(SkBits2Float(0x41133b1d), SkBits2Float(0xc26d29dc), SkBits2Float(0x410eaa27), SkBits2Float(0xc26d567f), SkBits2Float(0x410a17ee), SkBits2Float(0xc26d8104));
path.cubicTo(SkBits2Float(0x40b8b9ab), SkBits2Float(0xc26f2a74), SkBits2Float(0x4038d88b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4151fa93), SkBits2Float(0xc2a3e96b));
path.cubicTo(SkBits2Float(0x419c2b7d), SkBits2Float(0xc2a1dce5), SkBits2Float(0x41ce36f8), SkBits2Float(0xc29e52a6), SkBits2Float(0x41fe1a0a), SkBits2Float(0xc2995d2e));
path.lineTo(SkBits2Float(0x41b7b024), SkBits2Float(0xc25dbb29));
path.cubicTo(SkBits2Float(0x41951228), SkBits2Float(0xc264e68b), SkBits2Float(0x4161c9b2), SkBits2Float(0xc26a04c8), SkBits2Float(0x4117cabf), SkBits2Float(0xc26cfb1e));
path.lineTo(SkBits2Float(0x4151fa93), SkBits2Float(0xc2a3e96b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp24(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x409bc7b0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x411ba103), SkBits2Float(0xc2a524b6), SkBits2Float(0x4168515c), SkBits2Float(0xc2a370af));
path.lineTo(SkBits2Float(0x4127f0cc), SkBits2Float(0xc26c4c8f));
path.cubicTo(SkBits2Float(0x40e1017a), SkBits2Float(0xc26ec2f6), SkBits2Float(0x40613965), SkBits2Float(0xc26fffff), SkBits2Float(0x3655fea5), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4168515e), SkBits2Float(0xc2a370b0));
path.cubicTo(SkBits2Float(0x416ffb5b), SkBits2Float(0xc2a3451c), SkBits2Float(0x4177a23d), SkBits2Float(0xc2a31761), SkBits2Float(0x417f45ca), SkBits2Float(0xc2a2e77f));
path.lineTo(SkBits2Float(0x413888ce), SkBits2Float(0xc26b8638));
path.cubicTo(SkBits2Float(0x41330328), SkBits2Float(0xc26bcb72), SkBits2Float(0x412d7b1a), SkBits2Float(0xc26c0d90), SkBits2Float(0x4127f0cb), SkBits2Float(0xc26c4c90));
path.lineTo(SkBits2Float(0x4168515e), SkBits2Float(0xc2a370b0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp25(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3655fea5), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x409bc7b0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x411ba103), SkBits2Float(0xc2a524b6), SkBits2Float(0x4168515e), SkBits2Float(0xc2a370b0));
path.cubicTo(SkBits2Float(0x416ffb5b), SkBits2Float(0xc2a3451c), SkBits2Float(0x4177a23d), SkBits2Float(0xc2a31761), SkBits2Float(0x417f45ca), SkBits2Float(0xc2a2e77f));
path.lineTo(SkBits2Float(0x413888ce), SkBits2Float(0xc26b8638));
path.cubicTo(SkBits2Float(0x41330328), SkBits2Float(0xc26bcb72), SkBits2Float(0x412d7b1a), SkBits2Float(0xc26c0d90), SkBits2Float(0x4127f0cc), SkBits2Float(0xc26c4c8f));
path.cubicTo(SkBits2Float(0x40e1017a), SkBits2Float(0xc26ec2f6), SkBits2Float(0x40613965), SkBits2Float(0xc26fffff), SkBits2Float(0x3655fea5), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x417f45c8), SkBits2Float(0xc2a2e780));
path.cubicTo(SkBits2Float(0x41bda27d), SkBits2Float(0xc29fde49), SkBits2Float(0x41f99531), SkBits2Float(0xc29aa2c4), SkBits2Float(0x4218d569), SkBits2Float(0xc2935d77));
path.lineTo(SkBits2Float(0x41dcf6db), SkBits2Float(0xc2550ed7));
path.cubicTo(SkBits2Float(0x41b46bda), SkBits2Float(0xc25f91e2), SkBits2Float(0x418915db), SkBits2Float(0xc2672288), SkBits2Float(0x413888d2), SkBits2Float(0xc26b8639));
path.lineTo(SkBits2Float(0x417f45c8), SkBits2Float(0xc2a2e780));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp26(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40b98c15), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41394aaf), SkBits2Float(0xc2a4c8e8), SkBits2Float(0x418a04fa), SkBits2Float(0xc2a25fd2));
path.lineTo(SkBits2Float(0x41478bd6), SkBits2Float(0xc26ac20e));
path.cubicTo(SkBits2Float(0x4105f224), SkBits2Float(0xc26e3e3c), SkBits2Float(0x40862167), SkBits2Float(0xc2700000), SkBits2Float(0xb4d00ae8), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x418a04fd), SkBits2Float(0xc2a25fd2));
path.cubicTo(SkBits2Float(0x418e8d81), SkBits2Float(0xc2a2222a), SkBits2Float(0x41931368), SkBits2Float(0xc2a1e17a), SkBits2Float(0x41979681), SkBits2Float(0xc2a19dc3));
path.lineTo(SkBits2Float(0x415b29c8), SkBits2Float(0xc269a97e));
path.cubicTo(SkBits2Float(0x4154a3c3), SkBits2Float(0xc26a0b66), SkBits2Float(0x414e19b0), SkBits2Float(0xc26a68ed), SkBits2Float(0x41478bd5), SkBits2Float(0xc26ac20f));
path.lineTo(SkBits2Float(0x418a04fd), SkBits2Float(0xc2a25fd2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp27(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40b98c15), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41394aaf), SkBits2Float(0xc2a4c8e8), SkBits2Float(0x418a04fd), SkBits2Float(0xc2a25fd2));
path.cubicTo(SkBits2Float(0x418e8d81), SkBits2Float(0xc2a2222a), SkBits2Float(0x41931368), SkBits2Float(0xc2a1e17a), SkBits2Float(0x41979681), SkBits2Float(0xc2a19dc3));
path.lineTo(SkBits2Float(0x415b29c8), SkBits2Float(0xc269a97e));
path.cubicTo(SkBits2Float(0x4154a3c3), SkBits2Float(0xc26a0b66), SkBits2Float(0x414e19b0), SkBits2Float(0xc26a68ed), SkBits2Float(0x41478bd6), SkBits2Float(0xc26ac20e));
path.cubicTo(SkBits2Float(0x4105f224), SkBits2Float(0xc26e3e3c), SkBits2Float(0x40862167), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41979680), SkBits2Float(0xc2a19dc4));
path.cubicTo(SkBits2Float(0x41e0e1b2), SkBits2Float(0xc29d51d4), SkBits2Float(0x42135c08), SkBits2Float(0xc295f036), SkBits2Float(0x42330e86), SkBits2Float(0xc28bc9b7));
path.lineTo(SkBits2Float(0x42017048), SkBits2Float(0xc24a1a63));
path.cubicTo(SkBits2Float(0x41d50cc4), SkBits2Float(0xc258c742), SkBits2Float(0x41a290a5), SkBits2Float(0xc263733c), SkBits2Float(0x415b29c7), SkBits2Float(0xc269a980));
path.lineTo(SkBits2Float(0x41979680), SkBits2Float(0xc2a19dc4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp28(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40dd1e63), SkBits2Float(0xc2a5ffff), SkBits2Float(0x415caf98), SkBits2Float(0xc2a44632), SkBits2Float(0x41a3e96c), SkBits2Float(0xc2a0dcda));
path.lineTo(SkBits2Float(0x416cfb1c), SkBits2Float(0xc2689294));
path.cubicTo(SkBits2Float(0x411f8831), SkBits2Float(0xc26d8140), SkBits2Float(0x409fd849), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41a3e96b), SkBits2Float(0xc2a0dcda));
path.cubicTo(SkBits2Float(0x41a94306), SkBits2Float(0xc2a085a1), SkBits2Float(0x41ae9839), SkBits2Float(0xc2a02a23), SkBits2Float(0x41b3e8b2), SkBits2Float(0xc29fca67));
path.lineTo(SkBits2Float(0x41820dff), SkBits2Float(0xc26705ca));
path.cubicTo(SkBits2Float(0x417c6d0a), SkBits2Float(0xc2679035), SkBits2Float(0x4174b742), SkBits2Float(0xc268147b), SkBits2Float(0x416cfb1d), SkBits2Float(0xc2689296));
path.lineTo(SkBits2Float(0x41a3e96b), SkBits2Float(0xc2a0dcda));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp29(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40dd1e62), SkBits2Float(0xc2a60000), SkBits2Float(0x415caf97), SkBits2Float(0xc2a44632), SkBits2Float(0x41a3e96b), SkBits2Float(0xc2a0dcda));
path.lineTo(SkBits2Float(0x416cfb1d), SkBits2Float(0xc2689296));
path.cubicTo(SkBits2Float(0x4174b742), SkBits2Float(0xc268147b), SkBits2Float(0x417c6d0a), SkBits2Float(0xc2679035), SkBits2Float(0x41820dff), SkBits2Float(0xc26705ca));
path.lineTo(SkBits2Float(0x41b3e8b2), SkBits2Float(0xc29fca67));
path.cubicTo(SkBits2Float(0x41ae9839), SkBits2Float(0xc2a02a23), SkBits2Float(0x41a94307), SkBits2Float(0xc2a085a1), SkBits2Float(0x41a3e96c), SkBits2Float(0xc2a0dcda));
path.lineTo(SkBits2Float(0x416cfb1c), SkBits2Float(0xc2689294));
path.cubicTo(SkBits2Float(0x411f8831), SkBits2Float(0xc26d8140), SkBits2Float(0x409fd849), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41b3e8b1), SkBits2Float(0xc29fca67));
path.cubicTo(SkBits2Float(0x4205291f), SkBits2Float(0xc299b5bb), SkBits2Float(0x422d73c0), SkBits2Float(0xc28f4fcf), SkBits2Float(0x425064bf), SkBits2Float(0xc2813989));
path.lineTo(SkBits2Float(0x4216a55b), SkBits2Float(0xc23ad4b9));
path.cubicTo(SkBits2Float(0x41fac62f), SkBits2Float(0xc24f329e), SkBits2Float(0x41c0857c), SkBits2Float(0xc25e3b2e), SkBits2Float(0x41820dfe), SkBits2Float(0xc26705cb));
path.lineTo(SkBits2Float(0x41b3e8b1), SkBits2Float(0xc29fca67));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp30(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41028186), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4182264a), SkBits2Float(0xc2a39869), SkBits2Float(0x41c098e8), SkBits2Float(0xc29edd15));
path.lineTo(SkBits2Float(0x418b3a1a), SkBits2Float(0xc265aeac));
path.cubicTo(SkBits2Float(0x413c2b06), SkBits2Float(0xc26c85fe), SkBits2Float(0x40bcaeed), SkBits2Float(0xc2700000), SkBits2Float(0x337fa8c0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41c098e9), SkBits2Float(0xc29edd15));
path.cubicTo(SkBits2Float(0x41c6d4b6), SkBits2Float(0xc29e642a), SkBits2Float(0x41cd0950), SkBits2Float(0xc29de562), SkBits2Float(0x41d33633), SkBits2Float(0xc29d60c8));
path.lineTo(SkBits2Float(0x4198aee4), SkBits2Float(0xc26388d7));
path.cubicTo(SkBits2Float(0x41943815), SkBits2Float(0xc264488f), SkBits2Float(0x418fbbb2), SkBits2Float(0xc264ffdc), SkBits2Float(0x418b3a19), SkBits2Float(0xc265aeae));
path.lineTo(SkBits2Float(0x41c098e9), SkBits2Float(0xc29edd15));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp31(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41028186), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4182264a), SkBits2Float(0xc2a39869), SkBits2Float(0x41c098e9), SkBits2Float(0xc29edd15));
path.cubicTo(SkBits2Float(0x41c6d4b6), SkBits2Float(0xc29e642a), SkBits2Float(0x41cd0950), SkBits2Float(0xc29de562), SkBits2Float(0x41d33633), SkBits2Float(0xc29d60c8));
path.lineTo(SkBits2Float(0x4198aee4), SkBits2Float(0xc26388d7));
path.cubicTo(SkBits2Float(0x41943816), SkBits2Float(0xc264488f), SkBits2Float(0x418fbbb2), SkBits2Float(0xc264ffda), SkBits2Float(0x418b3a1a), SkBits2Float(0xc265aeac));
path.cubicTo(SkBits2Float(0x413c2b06), SkBits2Float(0xc26c85fe), SkBits2Float(0x40bcaeed), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41d33633), SkBits2Float(0xc29d60c8));
path.cubicTo(SkBits2Float(0x421be102), SkBits2Float(0xc294f1be), SkBits2Float(0x4249615f), SkBits2Float(0xc2869cbc), SkBits2Float(0x426e4d45), SkBits2Float(0xc26729aa));
path.lineTo(SkBits2Float(0x422c4432), SkBits2Float(0xc2271b0a));
path.cubicTo(SkBits2Float(0x42119380), SkBits2Float(0xc2429ec2), SkBits2Float(0x41e15dfd), SkBits2Float(0xc257575a), SkBits2Float(0x4198aee4), SkBits2Float(0xc26388d8));
path.lineTo(SkBits2Float(0x41d33633), SkBits2Float(0xc29d60c8));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp32(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4118c001), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41982d6e), SkBits2Float(0xc2a2b4b2), SkBits2Float(0x41e01284), SkBits2Float(0xc29c4333));
path.lineTo(SkBits2Float(0x41a1fae3), SkBits2Float(0xc261ebf5));
path.cubicTo(SkBits2Float(0x415c0406), SkBits2Float(0xc26b3cc7), SkBits2Float(0x40dcd7ee), SkBits2Float(0xc2700000), SkBits2Float(0x35f7fd46), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41e01286), SkBits2Float(0xc29c4334));
path.cubicTo(SkBits2Float(0x41e73e86), SkBits2Float(0xc29b9ea8), SkBits2Float(0x41ee5f11), SkBits2Float(0xc29af239), SkBits2Float(0x41f57356), SkBits2Float(0xc29a3dfa));
path.lineTo(SkBits2Float(0x41b16f25), SkBits2Float(0xc25f0029));
path.cubicTo(SkBits2Float(0x41ac5112), SkBits2Float(0xc26004c3), SkBits2Float(0x41a72a20), SkBits2Float(0xc260fe11), SkBits2Float(0x41a1fae3), SkBits2Float(0xc261ebf7));
path.lineTo(SkBits2Float(0x41e01286), SkBits2Float(0xc29c4334));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp33(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4118c001), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41982d6e), SkBits2Float(0xc2a2b4b2), SkBits2Float(0x41e01286), SkBits2Float(0xc29c4334));
path.cubicTo(SkBits2Float(0x41e73e86), SkBits2Float(0xc29b9ea8), SkBits2Float(0x41ee5f11), SkBits2Float(0xc29af239), SkBits2Float(0x41f57356), SkBits2Float(0xc29a3dfa));
path.lineTo(SkBits2Float(0x41b16f25), SkBits2Float(0xc25f0029));
path.cubicTo(SkBits2Float(0x41ac5112), SkBits2Float(0xc26004c3), SkBits2Float(0x41a72a20), SkBits2Float(0xc260fe11), SkBits2Float(0x41a1fae3), SkBits2Float(0xc261ebf7));
path.lineTo(SkBits2Float(0x41a1fae3), SkBits2Float(0xc261ebf5));
path.cubicTo(SkBits2Float(0x415c0406), SkBits2Float(0xc26b3cc7), SkBits2Float(0x40dcd7ee), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41f57359), SkBits2Float(0xc29a3dfa));
path.cubicTo(SkBits2Float(0x42347528), SkBits2Float(0xc28ec218), SkBits2Float(0x42669614), SkBits2Float(0xc276cf04), SkBits2Float(0x4285b481), SkBits2Float(0xc244c364));
path.lineTo(SkBits2Float(0x42414f00), SkBits2Float(0xc20e3d0e));
path.cubicTo(SkBits2Float(0x4226b05a), SkBits2Float(0xc2326a79), SkBits2Float(0x4202738a), SkBits2Float(0xc24e65b9), SkBits2Float(0x41b16f25), SkBits2Float(0xc25f0028));
path.lineTo(SkBits2Float(0x41f57359), SkBits2Float(0xc29a3dfa));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp34(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41360dec), SkBits2Float(0xc2a60000), SkBits2Float(0x41b5150e), SkBits2Float(0xc2a1522b), SkBits2Float(0x42044925), SkBits2Float(0xc29840e5));
path.lineTo(SkBits2Float(0x41bf41a8), SkBits2Float(0xc25c2022));
path.cubicTo(SkBits2Float(0x4182e721), SkBits2Float(0xc2693c30), SkBits2Float(0x41039b08), SkBits2Float(0xc2700000), SkBits2Float(0x3673fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42044925), SkBits2Float(0xc29840e4));
path.cubicTo(SkBits2Float(0x4208721a), SkBits2Float(0xc2975992), SkBits2Float(0x420c9178), SkBits2Float(0xc296675c), SkBits2Float(0x4210a695), SkBits2Float(0xc2956a6a));
path.lineTo(SkBits2Float(0x41d1222e), SkBits2Float(0xc25805ce));
path.cubicTo(SkBits2Float(0x41cb3b2f), SkBits2Float(0xc2597382), SkBits2Float(0x41c5455b), SkBits2Float(0xc25ad1b2), SkBits2Float(0x41bf41a9), SkBits2Float(0xc25c2023));
path.lineTo(SkBits2Float(0x42044925), SkBits2Float(0xc29840e4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp35(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3673fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41360dec), SkBits2Float(0xc2a60000), SkBits2Float(0x41b5150e), SkBits2Float(0xc2a1522b), SkBits2Float(0x42044925), SkBits2Float(0xc29840e5));
path.lineTo(SkBits2Float(0x4210a695), SkBits2Float(0xc2956a6a));
path.lineTo(SkBits2Float(0x41d1222e), SkBits2Float(0xc25805ce));
path.cubicTo(SkBits2Float(0x41cb3b2f), SkBits2Float(0xc2597382), SkBits2Float(0x41c5455b), SkBits2Float(0xc25ad1b2), SkBits2Float(0x41bf41a9), SkBits2Float(0xc25c2023));
path.lineTo(SkBits2Float(0x41bf41a8), SkBits2Float(0xc25c2022));
path.cubicTo(SkBits2Float(0x4182e721), SkBits2Float(0xc2693c30), SkBits2Float(0x41039b08), SkBits2Float(0xc2700000), SkBits2Float(0x3673fea3), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4210a693), SkBits2Float(0xc2956a6a));
path.cubicTo(SkBits2Float(0x42536b4d), SkBits2Float(0xc2854182), SkBits2Float(0x4284b863), SkBits2Float(0xc254c33a), SkBits2Float(0x42950c68), SkBits2Float(0xc2122882));
path.lineTo(SkBits2Float(0x42577de3), SkBits2Float(0xc1d35027));
path.cubicTo(SkBits2Float(0x423fe27d), SkBits2Float(0xc219cde7), SkBits2Float(0x4218d548), SkBits2Float(0xc240a8bd), SkBits2Float(0x41d1222f), SkBits2Float(0xc25805ce));
path.lineTo(SkBits2Float(0x4210a693), SkBits2Float(0xc2956a6a));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp36(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x414e6589), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41ccf9e5), SkBits2Float(0xc29ffc89), SkBits2Float(0x4214a0bb), SkBits2Float(0xc2946fc8));
path.lineTo(SkBits2Float(0x41d6e236), SkBits2Float(0xc2569b72));
path.cubicTo(SkBits2Float(0x41942cf0), SkBits2Float(0xc2674e45), SkBits2Float(0x411533d1), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4214a0bb), SkBits2Float(0xc2946fc9));
path.cubicTo(SkBits2Float(0x421938a6), SkBits2Float(0xc293496b), SkBits2Float(0x421dc2c1), SkBits2Float(0xc2921574), SkBits2Float(0x42223e19), SkBits2Float(0xc290d421));
path.lineTo(SkBits2Float(0x41ea914d), SkBits2Float(0xc251640c));
path.cubicTo(SkBits2Float(0x41e4167f), SkBits2Float(0xc253349e), SkBits2Float(0x41dd8659), SkBits2Float(0xc254f1de), SkBits2Float(0x41d6e239), SkBits2Float(0xc2569b73));
path.lineTo(SkBits2Float(0x4214a0bb), SkBits2Float(0xc2946fc9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp37(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x414e6589), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41ccf9e5), SkBits2Float(0xc29ffc89), SkBits2Float(0x4214a0bb), SkBits2Float(0xc2946fc9));
path.cubicTo(SkBits2Float(0x421938a6), SkBits2Float(0xc293496b), SkBits2Float(0x421dc2c1), SkBits2Float(0xc2921574), SkBits2Float(0x42223e19), SkBits2Float(0xc290d421));
path.lineTo(SkBits2Float(0x41ea914d), SkBits2Float(0xc251640c));
path.cubicTo(SkBits2Float(0x41e4167f), SkBits2Float(0xc253349e), SkBits2Float(0x41dd8659), SkBits2Float(0xc254f1de), SkBits2Float(0x41d6e239), SkBits2Float(0xc2569b73));
path.lineTo(SkBits2Float(0x41d6e236), SkBits2Float(0xc2569b72));
path.cubicTo(SkBits2Float(0x41942cf0), SkBits2Float(0xc2674e45), SkBits2Float(0x411533d1), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42223e19), SkBits2Float(0xc290d422));
path.cubicTo(SkBits2Float(0x426bbc38), SkBits2Float(0xc2787e1d), SkBits2Float(0x42916a94), SkBits2Float(0xc234ee59), SkBits2Float(0x429e2fac), SkBits2Float(0xc1c951fc));
path.lineTo(SkBits2Float(0x4264b3f7), SkBits2Float(0xc191885f));
path.cubicTo(SkBits2Float(0x42523d91), SkBits2Float(0xc202cb25), SkBits2Float(0x422a6939), SkBits2Float(0xc233a21b), SkBits2Float(0x41ea914d), SkBits2Float(0xc251640d));
path.lineTo(SkBits2Float(0x42223e19), SkBits2Float(0xc290d422));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp38(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x416c96cf), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41ea70fe), SkBits2Float(0xc29e1973), SkBits2Float(0x422836c6), SkBits2Float(0xc28f1d8a));
path.lineTo(SkBits2Float(0x41f3336d), SkBits2Float(0xc24ee9f1));
path.cubicTo(SkBits2Float(0x41a979c6), SkBits2Float(0xc26493d6), SkBits2Float(0x412b073c), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x422836c5), SkBits2Float(0xc28f1d8b));
path.cubicTo(SkBits2Float(0x422d4896), SkBits2Float(0xc28da02f), SkBits2Float(0x423245ea), SkBits2Float(0xc28c11a8), SkBits2Float(0x42372d65), SkBits2Float(0xc28a7261));
path.lineTo(SkBits2Float(0x42046ad7), SkBits2Float(0xc24829ff));
path.cubicTo(SkBits2Float(0x4200df44), SkBits2Float(0xc24a8267), SkBits2Float(0x41fa87ca), SkBits2Float(0xc24cc296), SkBits2Float(0x41f3336d), SkBits2Float(0xc24ee9f1));
path.lineTo(SkBits2Float(0x422836c5), SkBits2Float(0xc28f1d8b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp39(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x416c96cf), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41ea70fe), SkBits2Float(0xc29e1973), SkBits2Float(0x422836c5), SkBits2Float(0xc28f1d8b));
path.cubicTo(SkBits2Float(0x422d4896), SkBits2Float(0xc28da02f), SkBits2Float(0x423245ea), SkBits2Float(0xc28c11a8), SkBits2Float(0x42372d65), SkBits2Float(0xc28a7261));
path.lineTo(SkBits2Float(0x42046ad7), SkBits2Float(0xc24829ff));
path.cubicTo(SkBits2Float(0x4200df44), SkBits2Float(0xc24a8267), SkBits2Float(0x41fa87ca), SkBits2Float(0xc24cc296), SkBits2Float(0x41f3336d), SkBits2Float(0xc24ee9f1));
path.cubicTo(SkBits2Float(0x41a979c6), SkBits2Float(0xc26493d6), SkBits2Float(0x412b073c), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42372d65), SkBits2Float(0xc28a7262));
path.cubicTo(SkBits2Float(0x4283f2b3), SkBits2Float(0xc25f7e9c), SkBits2Float(0x429ea5c2), SkBits2Float(0xc2098801), SkBits2Float(0x42a4b292), SkBits2Float(0xc12607b1));
path.lineTo(SkBits2Float(0x426e1def), SkBits2Float(0xc0f00b21));
path.cubicTo(SkBits2Float(0x42655eb1), SkBits2Float(0xc1c6d725), SkBits2Float(0x423ec4ad), SkBits2Float(0xc2218ff6), SkBits2Float(0x42046ad7), SkBits2Float(0xc2482a00));
path.lineTo(SkBits2Float(0x42372d65), SkBits2Float(0xc28a7262));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp40(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4184d4a8), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42034ddf), SkBits2Float(0xc29c0a4c), SkBits2Float(0x423a47b2), SkBits2Float(0xc289686d));
path.lineTo(SkBits2Float(0x4206a908), SkBits2Float(0xc246a97c));
path.cubicTo(SkBits2Float(0x41bdd65f), SkBits2Float(0xc26199af), SkBits2Float(0x41400b5c), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423a47b2), SkBits2Float(0xc289686d));
path.cubicTo(SkBits2Float(0x423fbcc3), SkBits2Float(0xc2878eef), SkBits2Float(0x4245154e), SkBits2Float(0xc285a0be), SkBits2Float(0x424a4f85), SkBits2Float(0xc2839e81));
path.lineTo(SkBits2Float(0x42123fa7), SkBits2Float(0xc23e4af2));
path.cubicTo(SkBits2Float(0x420e7846), SkBits2Float(0xc241326c), SkBits2Float(0x420a9af5), SkBits2Float(0xc243fcec), SkBits2Float(0x4206a907), SkBits2Float(0xc246a97c));
path.lineTo(SkBits2Float(0x423a47b2), SkBits2Float(0xc289686d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end fail 1

static void battleOp41(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4196c4f9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42148669), SkBits2Float(0xc2992c23), SkBits2Float(0x424f6452), SkBits2Float(0xc281a081));
path.lineTo(SkBits2Float(0x4215ebfd), SkBits2Float(0xc23b6999));
path.cubicTo(SkBits2Float(0x41d6bc2a), SkBits2Float(0xc25d7441), SkBits2Float(0x4159fada), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x424f6452), SkBits2Float(0xc281a081));
path.cubicTo(SkBits2Float(0x42553921), SkBits2Float(0xc27e96d1), SkBits2Float(0x425ae53b), SkBits2Float(0xc279ba9d), SkBits2Float(0x42606622), SkBits2Float(0xc274ae80));
path.lineTo(SkBits2Float(0x42223753), SkBits2Float(0xc230e0d8));
path.cubicTo(SkBits2Float(0x421e3cd8), SkBits2Float(0xc23486e8), SkBits2Float(0x421a2322), SkBits2Float(0xc2380a55), SkBits2Float(0x4215ebfe), SkBits2Float(0xc23b6999));
path.lineTo(SkBits2Float(0x424f6452), SkBits2Float(0xc281a081));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp42(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4196c4f9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42148669), SkBits2Float(0xc2992c23), SkBits2Float(0x424f6452), SkBits2Float(0xc281a081));
path.cubicTo(SkBits2Float(0x42553921), SkBits2Float(0xc27e96d1), SkBits2Float(0x425ae53b), SkBits2Float(0xc279ba9d), SkBits2Float(0x42606622), SkBits2Float(0xc274ae80));
path.lineTo(SkBits2Float(0x42223753), SkBits2Float(0xc230e0d8));
path.cubicTo(SkBits2Float(0x421e3cd8), SkBits2Float(0xc23486e8), SkBits2Float(0x421a2322), SkBits2Float(0xc2380a55), SkBits2Float(0x4215ebfd), SkBits2Float(0xc23b6999));
path.cubicTo(SkBits2Float(0x41d6bc2a), SkBits2Float(0xc25d7441), SkBits2Float(0x4159fada), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42606622), SkBits2Float(0xc274ae80));
path.cubicTo(SkBits2Float(0x429deeac), SkBits2Float(0xc220cc44), SkBits2Float(0x42b0742c), SkBits2Float(0xc1039d5c), SkBits2Float(0x42a03731), SkBits2Float(0x41adc1b3));
path.lineTo(SkBits2Float(0x4267a314), SkBits2Float(0x417b36e3));
path.cubicTo(SkBits2Float(0x427f1d2c), SkBits2Float(0xc0be4950), SkBits2Float(0x426455fc), SkBits2Float(0xc1e87a9a), SkBits2Float(0x42223754), SkBits2Float(0xc230e0d7));
path.lineTo(SkBits2Float(0x42606622), SkBits2Float(0xc274ae80));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp43(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41aa5d9e), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42271b56), SkBits2Float(0xc295a109), SkBits2Float(0x4264d340), SkBits2Float(0xc2708c1d));
path.lineTo(SkBits2Float(0x42256a74), SkBits2Float(0xc22de3bf));
path.cubicTo(SkBits2Float(0x41f199ac), SkBits2Float(0xc25854c9), SkBits2Float(0x41764fdb), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4264d342), SkBits2Float(0xc2708c1d));
path.cubicTo(SkBits2Float(0x426aec59), SkBits2Float(0xc26abf16), SkBits2Float(0x4270cc6c), SkBits2Float(0xc264b73d), SkBits2Float(0x42767031), SkBits2Float(0xc25e77e8));
path.lineTo(SkBits2Float(0x423225ec), SkBits2Float(0xc220d20e));
path.cubicTo(SkBits2Float(0x422e123c), SkBits2Float(0xc2255633), SkBits2Float(0x4229d2f5), SkBits2Float(0xc229b23c), SkBits2Float(0x42256a74), SkBits2Float(0xc22de3c0));
path.lineTo(SkBits2Float(0x4264d342), SkBits2Float(0xc2708c1d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp44(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41aa5d9e), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42271b56), SkBits2Float(0xc295a109), SkBits2Float(0x4264d340), SkBits2Float(0xc2708c1d));
path.lineTo(SkBits2Float(0x4264d342), SkBits2Float(0xc2708c1d));
path.cubicTo(SkBits2Float(0x426aec59), SkBits2Float(0xc26abf16), SkBits2Float(0x4270cc6c), SkBits2Float(0xc264b73d), SkBits2Float(0x42767031), SkBits2Float(0xc25e77e8));
path.lineTo(SkBits2Float(0x423225ec), SkBits2Float(0xc220d20e));
path.cubicTo(SkBits2Float(0x422e123c), SkBits2Float(0xc2255633), SkBits2Float(0x4229d2f5), SkBits2Float(0xc229b23c), SkBits2Float(0x42256a74), SkBits2Float(0xc22de3c0));
path.lineTo(SkBits2Float(0x42256a74), SkBits2Float(0xc22de3bf));
path.cubicTo(SkBits2Float(0x41f199ac), SkBits2Float(0xc25854c9), SkBits2Float(0x41764fdb), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42767032), SkBits2Float(0xc25e77e8));
path.cubicTo(SkBits2Float(0x42aa697a), SkBits2Float(0xc1ebd370), SkBits2Float(0x42b37ad4), SkBits2Float(0x410b48c2), SkBits2Float(0x4291d766), SkBits2Float(0x421e927b));
path.lineTo(SkBits2Float(0x4252dae4), SkBits2Float(0x41e542d2));
path.cubicTo(SkBits2Float(0x4281be95), SkBits2Float(0x40c95ff9), SkBits2Float(0x427660fe), SkBits2Float(0xc1aa7a03), SkBits2Float(0x423225ed), SkBits2Float(0xc220d20e));
path.lineTo(SkBits2Float(0x42767032), SkBits2Float(0xc25e77e8));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp45(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41bfbd07), SkBits2Float(0xc2a5ffff), SkBits2Float(0x423b0ef1), SkBits2Float(0xc2914772), SkBits2Float(0x427a1b1d), SkBits2Float(0xc25a5641));
path.lineTo(SkBits2Float(0x4234ccaa), SkBits2Float(0xc21dd57d));
path.cubicTo(SkBits2Float(0x42073912), SkBits2Float(0xc2520ac5), SkBits2Float(0x418a9b2a), SkBits2Float(0xc26fffff), SkBits2Float(0x3697ff52), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427a1b1e), SkBits2Float(0xc25a5642));
path.cubicTo(SkBits2Float(0x4280286a), SkBits2Float(0xc253393c), SkBits2Float(0x42831c11), SkBits2Float(0xc24bd939), SkBits2Float(0x4285e673), SkBits2Float(0xc2443b5f));
path.lineTo(SkBits2Float(0x42419733), SkBits2Float(0xc20ddaba));
path.cubicTo(SkBits2Float(0x423d8e5d), SkBits2Float(0xc2135c44), SkBits2Float(0x423949dc), SkBits2Float(0xc218b118), SkBits2Float(0x4234ccac), SkBits2Float(0xc21dd57e));
path.lineTo(SkBits2Float(0x427a1b1e), SkBits2Float(0xc25a5642));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp46(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41bfbd07), SkBits2Float(0xc2a5ffff), SkBits2Float(0x423b0ef1), SkBits2Float(0xc2914772), SkBits2Float(0x427a1b1e), SkBits2Float(0xc25a5642));
path.cubicTo(SkBits2Float(0x4280286a), SkBits2Float(0xc253393c), SkBits2Float(0x42831c11), SkBits2Float(0xc24bd939), SkBits2Float(0x4285e673), SkBits2Float(0xc2443b5f));
path.lineTo(SkBits2Float(0x42419733), SkBits2Float(0xc20ddaba));
path.cubicTo(SkBits2Float(0x423d8e5d), SkBits2Float(0xc2135c44), SkBits2Float(0x423949dc), SkBits2Float(0xc218b118), SkBits2Float(0x4234ccac), SkBits2Float(0xc21dd57e));
path.lineTo(SkBits2Float(0x4234ccaa), SkBits2Float(0xc21dd57d));
path.cubicTo(SkBits2Float(0x42073912), SkBits2Float(0xc2520ac5), SkBits2Float(0x418a9b2a), SkBits2Float(0xc26fffff), SkBits2Float(0x3697ff52), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4285e672), SkBits2Float(0xc2443b5f));
path.cubicTo(SkBits2Float(0x42b50145), SkBits2Float(0xc1875361), SkBits2Float(0x42afc74e), SkBits2Float(0x41db6d5e), SkBits2Float(0x4272e616), SkBits2Float(0x426253de));
path.lineTo(SkBits2Float(0x422f96e8), SkBits2Float(0x42239c3e));
path.cubicTo(SkBits2Float(0x427e233c), SkBits2Float(0x419e9f42), SkBits2Float(0x4282d8d3), SkBits2Float(0xc143a6d1), SkBits2Float(0x42419734), SkBits2Float(0xc20ddabb));
path.lineTo(SkBits2Float(0x4285e672), SkBits2Float(0xc2443b5f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp47(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d59904), SkBits2Float(0xc2a5ffff), SkBits2Float(0x424f13ae), SkBits2Float(0xc28c4fb7), SkBits2Float(0x4286bb70), SkBits2Float(0xc241f0ca));
path.lineTo(SkBits2Float(0x4242cb24), SkBits2Float(0xc20c32b1));
path.cubicTo(SkBits2Float(0x4215b1b4), SkBits2Float(0xc24adc20), SkBits2Float(0x419a6875), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4286bb71), SkBits2Float(0xc241f0ca));
path.cubicTo(SkBits2Float(0x4289cb2b), SkBits2Float(0xc2396eee), SkBits2Float(0x428ca6e5), SkBits2Float(0xc230a410), SkBits2Float(0x428f4c27), SkBits2Float(0xc22797c0));
path.lineTo(SkBits2Float(0x424f2d54), SkBits2Float(0xc1f24d85));
path.cubicTo(SkBits2Float(0x424b5a2a), SkBits2Float(0xc1ff6268), SkBits2Float(0x42473840), SkBits2Float(0xc2060c56), SkBits2Float(0x4242cb25), SkBits2Float(0xc20c32b2));
path.lineTo(SkBits2Float(0x4286bb71), SkBits2Float(0xc241f0ca));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp48(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d59904), SkBits2Float(0xc2a5ffff), SkBits2Float(0x424f13ae), SkBits2Float(0xc28c4fb7), SkBits2Float(0x4286bb71), SkBits2Float(0xc241f0ca));
path.cubicTo(SkBits2Float(0x4289cb2b), SkBits2Float(0xc2396eee), SkBits2Float(0x428ca6e5), SkBits2Float(0xc230a410), SkBits2Float(0x428f4c27), SkBits2Float(0xc22797c0));
path.lineTo(SkBits2Float(0x424f2d54), SkBits2Float(0xc1f24d85));
path.cubicTo(SkBits2Float(0x424b5a2a), SkBits2Float(0xc1ff6268), SkBits2Float(0x42473840), SkBits2Float(0xc2060c56), SkBits2Float(0x4242cb24), SkBits2Float(0xc20c32b1));
path.cubicTo(SkBits2Float(0x4215b1b4), SkBits2Float(0xc24adc20), SkBits2Float(0x419a6875), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428f4c27), SkBits2Float(0xc22797c0));
path.cubicTo(SkBits2Float(0x42bc6513), SkBits2Float(0xc055a915), SkBits2Float(0x42a45eb2), SkBits2Float(0x42389acf), SkBits2Float(0x4231df29), SkBits2Float(0x428c2a69));
path.lineTo(SkBits2Float(0x420094fc), SkBits2Float(0x424aa62f));
path.cubicTo(SkBits2Float(0x426da4ad), SkBits2Float(0x42057300), SkBits2Float(0x42883065), SkBits2Float(0xc01a7416), SkBits2Float(0x424f2d56), SkBits2Float(0xc1f24d87));
path.lineTo(SkBits2Float(0x428f4c27), SkBits2Float(0xc22797c0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp49(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41eed329), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4265a038), SkBits2Float(0xc285ef96), SkBits2Float(0x42905111), SkBits2Float(0xc2240eac));
path.lineTo(SkBits2Float(0x4250a68d), SkBits2Float(0xc1ed30fa));
path.cubicTo(SkBits2Float(0x4225fe9e), SkBits2Float(0xc241a46c), SkBits2Float(0x41aca4fc), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42905111), SkBits2Float(0xc2240ead));
path.cubicTo(SkBits2Float(0x429332f8), SkBits2Float(0xc219ea36), SkBits2Float(0x4295cfef), SkBits2Float(0xc20f79c4), SkBits2Float(0x4298252c), SkBits2Float(0xc204c875));
path.lineTo(SkBits2Float(0x425bf80f), SkBits2Float(0xc1bff9b9));
path.cubicTo(SkBits2Float(0x42589896), SkBits2Float(0xc1cf6f48), SkBits2Float(0x4254d168), SkBits2Float(0xc1de8710), SkBits2Float(0x4250a68e), SkBits2Float(0xc1ed30fc));
path.lineTo(SkBits2Float(0x42905111), SkBits2Float(0xc2240ead));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp50(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41eed328), SkBits2Float(0xc2a60000), SkBits2Float(0x4265a038), SkBits2Float(0xc285ef96), SkBits2Float(0x42905111), SkBits2Float(0xc2240ead));
path.lineTo(SkBits2Float(0x42905111), SkBits2Float(0xc2240eac));
path.cubicTo(SkBits2Float(0x429332f8), SkBits2Float(0xc219ea35), SkBits2Float(0x4295cfef), SkBits2Float(0xc20f79c4), SkBits2Float(0x4298252c), SkBits2Float(0xc204c875));
path.lineTo(SkBits2Float(0x425bf80f), SkBits2Float(0xc1bff9b9));
path.cubicTo(SkBits2Float(0x42589896), SkBits2Float(0xc1cf6f48), SkBits2Float(0x4254d168), SkBits2Float(0xc1de8710), SkBits2Float(0x4250a68d), SkBits2Float(0xc1ed30fa));
path.cubicTo(SkBits2Float(0x4225fe9e), SkBits2Float(0xc241a46c), SkBits2Float(0x41aca4fc), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4298252d), SkBits2Float(0xc204c875));
path.cubicTo(SkBits2Float(0x42ab560c), SkBits2Float(0xc1334da0), SkBits2Float(0x42aa8ee6), SkBits2Float(0x415dbf57), SkBits2Float(0x4296030d), SkBits2Float(0x420e292a));
path.cubicTo(SkBits2Float(0x42817734), SkBits2Float(0x4264e27f), SkBits2Float(0x42365290), SkBits2Float(0x4292cae0), SkBits2Float(0x41b3e39e), SkBits2Float(0x429fcac3));
path.lineTo(SkBits2Float(0x41820a52), SkBits2Float(0x4267064e));
path.cubicTo(SkBits2Float(0x4203cca7), SkBits2Float(0x42543ae9), SkBits2Float(0x423b2de4), SkBits2Float(0x42257578), SkBits2Float(0x4258e27d), SkBits2Float(0x41cd88a1));
path.cubicTo(SkBits2Float(0x42769717), SkBits2Float(0x41204ca2), SkBits2Float(0x4277b705), SkBits2Float(0xc1019de9), SkBits2Float(0x425bf810), SkBits2Float(0xc1bff9bb));
path.lineTo(SkBits2Float(0x4298252d), SkBits2Float(0xc204c875));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp51(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42044d64), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427bf9ef), SkBits2Float(0xc27d72ab), SkBits2Float(0x42984d42), SkBits2Float(0xc2041029));
path.lineTo(SkBits2Float(0x425c3202), SkBits2Float(0xc1beef44));
path.cubicTo(SkBits2Float(0x423626cb), SkBits2Float(0xc2373722), SkBits2Float(0x41bf47cb), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42984d42), SkBits2Float(0xc2041029));
path.cubicTo(SkBits2Float(0x429adc06), SkBits2Float(0xc1f08771), SkBits2Float(0x429d127e), SkBits2Float(0xc1d85b80), SkBits2Float(0x429eedcc), SkBits2Float(0xc1bfbbc5));
path.lineTo(SkBits2Float(0x4265c6d6), SkBits2Float(0xc18a9a3f));
path.cubicTo(SkBits2Float(0x426317a7), SkBits2Float(0xc19c6729), SkBits2Float(0x425fe4aa), SkBits2Float(0xc1ade05f), SkBits2Float(0x425c3203), SkBits2Float(0xc1beef45));
path.lineTo(SkBits2Float(0x42984d42), SkBits2Float(0xc2041029));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp52(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42044d64), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427bf9ef), SkBits2Float(0xc27d72ab), SkBits2Float(0x42984d42), SkBits2Float(0xc2041029));
path.cubicTo(SkBits2Float(0x429adc06), SkBits2Float(0xc1f08771), SkBits2Float(0x429d127e), SkBits2Float(0xc1d85b80), SkBits2Float(0x429eedcc), SkBits2Float(0xc1bfbbc5));
path.lineTo(SkBits2Float(0x4265c6d6), SkBits2Float(0xc18a9a3f));
path.cubicTo(SkBits2Float(0x426317a7), SkBits2Float(0xc19c6729), SkBits2Float(0x425fe4aa), SkBits2Float(0xc1ade05f), SkBits2Float(0x425c3202), SkBits2Float(0xc1beef44));
path.cubicTo(SkBits2Float(0x423626cb), SkBits2Float(0xc2373722), SkBits2Float(0x41bf47cb), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429eedcc), SkBits2Float(0xc1bfbbc6));
path.cubicTo(SkBits2Float(0x42ae408c), SkBits2Float(0x3fb7daeb), SkBits2Float(0x42a45c89), SkBits2Float(0x41e7c57e), SkBits2Float(0x42845101), SkBits2Float(0x42487bac));
path.cubicTo(SkBits2Float(0x42488af1), SkBits2Float(0x428e8a4c), SkBits2Float(0x41c7bd0e), SkBits2Float(0x42a6f806), SkBits2Float(0xbfc7d871), SkBits2Float(0x42a5f87b));
path.lineTo(SkBits2Float(0xbf90777c), SkBits2Float(0x426ff521));
path.cubicTo(SkBits2Float(0x419063a9), SkBits2Float(0x42716698), SkBits2Float(0x4210f87e), SkBits2Float(0x424e1511), SkBits2Float(0x423f4d05), SkBits2Float(0x4210ed75));
path.cubicTo(SkBits2Float(0x426da18c), SkBits2Float(0x41a78bb1), SkBits2Float(0x427bee4d), SkBits2Float(0x3f84e856), SkBits2Float(0x4265c6d8), SkBits2Float(0xc18a9a40));
path.lineTo(SkBits2Float(0x429eedcc), SkBits2Float(0xc1bfbbc6));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp53(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x421216db), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4289817d), SkBits2Float(0xc26c814f), SkBits2Float(0x429ecb3a), SkBits2Float(0xc1c183ed));
path.lineTo(SkBits2Float(0x426594dc), SkBits2Float(0xc18be3fc));
path.cubicTo(SkBits2Float(0x4246cdba), SkBits2Float(0xc22af7b1), SkBits2Float(0x41d336a3), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429ecb3a), SkBits2Float(0xc1c183e9));
path.cubicTo(SkBits2Float(0x42a0d9cb), SkBits2Float(0xc1a68281), SkBits2Float(0x42a27999), SkBits2Float(0xc18b01ce), SkBits2Float(0x42a3a81d), SkBits2Float(0xc15e595d));
path.lineTo(SkBits2Float(0x426c9cb2), SkBits2Float(0xc120bbfa));
path.cubicTo(SkBits2Float(0x426ae754), SkBits2Float(0xc148f95c), SkBits2Float(0x42688e2a), SkBits2Float(0xc170bcb0), SkBits2Float(0x426594dd), SkBits2Float(0xc18be3fd));
path.lineTo(SkBits2Float(0x429ecb3a), SkBits2Float(0xc1c183e9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp54(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x421216db), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4289817d), SkBits2Float(0xc26c814f), SkBits2Float(0x429ecb3a), SkBits2Float(0xc1c183ed));
path.lineTo(SkBits2Float(0x42a3a81d), SkBits2Float(0xc15e595d));
path.lineTo(SkBits2Float(0x426c9cb2), SkBits2Float(0xc120bbfa));
path.cubicTo(SkBits2Float(0x426ae754), SkBits2Float(0xc148f95c), SkBits2Float(0x42688e2a), SkBits2Float(0xc170bcb0), SkBits2Float(0x426594dd), SkBits2Float(0xc18be3fd));
path.lineTo(SkBits2Float(0x426594dc), SkBits2Float(0xc18be3fc));
path.cubicTo(SkBits2Float(0x4246cdba), SkBits2Float(0xc22af7b1), SkBits2Float(0x41d336a3), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a3a81d), SkBits2Float(0xc15e595e));
path.cubicTo(SkBits2Float(0x42ad725e), SkBits2Float(0x416ed313), SkBits2Float(0x42982fa2), SkBits2Float(0x4230cc44), SkBits2Float(0x42575fca), SkBits2Float(0x427ca963));
path.cubicTo(SkBits2Float(0x41fcc0a1), SkBits2Float(0x42a44341), SkBits2Float(0x3f80ed4e), SkBits2Float(0x42affc4e), SkBits2Float(0xc1d56b7f), SkBits2Float(0x429d3115));
path.lineTo(SkBits2Float(0xc19a478e), SkBits2Float(0x426343e2));
path.cubicTo(SkBits2Float(0x3f3a6666), SkBits2Float(0x427e6fe0), SkBits2Float(0x41b6b66f), SkBits2Float(0x426d7d04), SkBits2Float(0x421bb135), SkBits2Float(0x4236a5a5));
path.cubicTo(SkBits2Float(0x425c0733), SkBits2Float(0x41ff9c8c), SkBits2Float(0x427ac435), SkBits2Float(0x412ca4f2), SkBits2Float(0x426c9cb3), SkBits2Float(0xc120bbf8));
path.lineTo(SkBits2Float(0x42a3a81d), SkBits2Float(0xc15e595e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp55(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4220aa02), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42952310), SkBits2Float(0xc258f48d), SkBits2Float(0x42a35f68), SkBits2Float(0xc16b5614));
path.lineTo(SkBits2Float(0x426c3395), SkBits2Float(0xc12a1f61));
path.cubicTo(SkBits2Float(0x42579ea8), SkBits2Float(0xc21cd5ce), SkBits2Float(0x41e84916), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a35f69), SkBits2Float(0xc16b5613));
path.cubicTo(SkBits2Float(0x42a4bd24), SkBits2Float(0xc12ea3c2), SkBits2Float(0x42a59325), SkBits2Float(0xc0e282d6), SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.lineTo(SkBits2Float(0x426fd18d), SkBits2Float(0xc0154a48));
path.cubicTo(SkBits2Float(0x426f62a1), SkBits2Float(0xc0a3be33), SkBits2Float(0x426e2d39), SkBits2Float(0xc0fc7dbb), SkBits2Float(0x426c3397), SkBits2Float(0xc12a1f63));
path.lineTo(SkBits2Float(0x42a35f69), SkBits2Float(0xc16b5613));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp56(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4220aa02), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42952310), SkBits2Float(0xc258f48d), SkBits2Float(0x42a35f69), SkBits2Float(0xc16b5613));
path.cubicTo(SkBits2Float(0x42a4bd24), SkBits2Float(0xc12ea3c2), SkBits2Float(0x42a59325), SkBits2Float(0xc0e282d6), SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.lineTo(SkBits2Float(0x426fd18d), SkBits2Float(0xc0154a48));
path.cubicTo(SkBits2Float(0x426f62a1), SkBits2Float(0xc0a3be33), SkBits2Float(0x426e2d39), SkBits2Float(0xc0fc7dbb), SkBits2Float(0x426c3397), SkBits2Float(0xc12a1f63));
path.lineTo(SkBits2Float(0x426c3395), SkBits2Float(0xc12a1f61));
path.cubicTo(SkBits2Float(0x42579ea8), SkBits2Float(0xc21cd5ce), SkBits2Float(0x41e84916), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.cubicTo(SkBits2Float(0x42a85e4f), SkBits2Float(0x41e6959e), SkBits2Float(0x4285b4e3), SkBits2Float(0x426ae44f), SkBits2Float(0x4219b105), SkBits2Float(0x42932450));
path.cubicTo(SkBits2Float(0x411fe111), SkBits2Float(0x42b0d679), SkBits2Float(0xc1c3966b), SkBits2Float(0x42ab1d42), SkBits2Float(0xc2482755), SkBits2Float(0x428470e8));
path.lineTo(SkBits2Float(0xc210b07c), SkBits2Float(0x423f7b24));
path.cubicTo(SkBits2Float(0xc18d6382), SkBits2Float(0x427764e8), SkBits2Float(0x40e72680), SkBits2Float(0x427fab4e), SkBits2Float(0x41de345e), SkBits2Float(0x4254bc3b));
path.cubicTo(SkBits2Float(0x42414f8e), SkBits2Float(0x4229cd28), SkBits2Float(0x42736c9d), SkBits2Float(0x41a6b008), SkBits2Float(0x426fd18e), SkBits2Float(0xc0154a3f));
path.lineTo(SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp57(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x422b8e0b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x429d6dbc), SkBits2Float(0xc2494bad), SkBits2Float(0x42a54cb6), SkBits2Float(0xc0f3b760));
path.lineTo(SkBits2Float(0x426efcca), SkBits2Float(0xc0b02e2c));
path.cubicTo(SkBits2Float(0x42639b94), SkBits2Float(0xc21183d2), SkBits2Float(0x41f807f9), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a54cb7), SkBits2Float(0xc0f3b757));
path.cubicTo(SkBits2Float(0x42a60d08), SkBits2Float(0xc0628d9e), SkBits2Float(0x42a632b1), SkBits2Float(0x3f0efcd8), SkBits2Float(0x42a5bd61), SkBits2Float(0x4094a90a));
path.lineTo(SkBits2Float(0x426f9faf), SkBits2Float(0x4056ee3d));
path.cubicTo(SkBits2Float(0x42704949), SkBits2Float(0x3ecebaba), SkBits2Float(0x427012d8), SkBits2Float(0xc023c5fe), SkBits2Float(0x426efccb), SkBits2Float(0xc0b02e2d));
path.lineTo(SkBits2Float(0x42a54cb7), SkBits2Float(0xc0f3b757));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp58(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x422b8e0b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x429d6dbc), SkBits2Float(0xc2494bad), SkBits2Float(0x42a54cb7), SkBits2Float(0xc0f3b757));
path.cubicTo(SkBits2Float(0x42a60d08), SkBits2Float(0xc0628d9e), SkBits2Float(0x42a632b1), SkBits2Float(0x3f0efcd8), SkBits2Float(0x42a5bd61), SkBits2Float(0x4094a90a));
path.lineTo(SkBits2Float(0x426f9faf), SkBits2Float(0x4056ee3d));
path.cubicTo(SkBits2Float(0x42704949), SkBits2Float(0x3ecebaba), SkBits2Float(0x427012d8), SkBits2Float(0xc023c5fe), SkBits2Float(0x426efcca), SkBits2Float(0xc0b02e2c));
path.cubicTo(SkBits2Float(0x42639b94), SkBits2Float(0xc21183d2), SkBits2Float(0x41f807f9), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5bd62), SkBits2Float(0x4094a90c));
path.cubicTo(SkBits2Float(0x42a1e9d4), SkBits2Float(0x421b17cd), SkBits2Float(0x426944f3), SkBits2Float(0x428879ea), SkBits2Float(0x41ceac14), SkBits2Float(0x429dc116));
path.cubicTo(SkBits2Float(0xc0d4c6f5), SkBits2Float(0x42b30843), SkBits2Float(0xc2295516), SkBits2Float(0x429e4e8b), SkBits2Float(0xc2802142), SkBits2Float(0x4253148e));
path.lineTo(SkBits2Float(0xc2393f81), SkBits2Float(0x42189693));
path.cubicTo(SkBits2Float(0xc1f4d162), SkBits2Float(0x4264e09b), SkBits2Float(0xc099d099), SkBits2Float(0x42816bc3), SkBits2Float(0x419566d0), SkBits2Float(0x42641418));
path.cubicTo(SkBits2Float(0x4228a0e3), SkBits2Float(0x424550a9), SkBits2Float(0x426a177b), SkBits2Float(0x41e03b19), SkBits2Float(0x426f9fb0), SkBits2Float(0x4056ee3a));
path.lineTo(SkBits2Float(0x42a5bd62), SkBits2Float(0x4094a90c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp59(skiatest::Reporter* reporter, const char* filename) {  // hung
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x423693bc), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42a57249), SkBits2Float(0xc2389374), SkBits2Float(0x42a5ff3a), SkBits2Float(0xbf002494));
path.lineTo(SkBits2Float(0x426ffee2), SkBits2Float(0xbeb944c3));
path.cubicTo(SkBits2Float(0x426f331d), SkBits2Float(0xc2056daf), SkBits2Float(0x4203fbc4), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5ff3a), SkBits2Float(0xbf0024e6));
path.cubicTo(SkBits2Float(0x42a60c9b), SkBits2Float(0x40752b0d), SkBits2Float(0x42a56c5d), SkBits2Float(0x410284fd), SkBits2Float(0x42a41ffb), SkBits2Float(0x414709fb));
path.lineTo(SkBits2Float(0x426d49ff), SkBits2Float(0x410fe233));
path.cubicTo(SkBits2Float(0x426f2a8e), SkBits2Float(0x40bcb3f0), SkBits2Float(0x42701239), SkBits2Float(0x40313ae3), SkBits2Float(0x426ffee3), SkBits2Float(0xbeb944c6));
path.lineTo(SkBits2Float(0x42a5ff3a), SkBits2Float(0xbf0024e6));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp60(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e9334c2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f13342a), SkBits2Float(0xc2a5ff3c), SkBits2Float(0x3f5ccd0d), SkBits2Float(0xc2a5fdb4));
path.lineTo(SkBits2Float(0x3f1f9d85), SkBits2Float(0xc26ffcaf));
path.cubicTo(SkBits2Float(0x3ed4d324), SkBits2Float(0xc26ffee7), SkBits2Float(0x3e54d404), SkBits2Float(0xc2700000), SkBits2Float(0x36b23f68), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f5ccd1a), SkBits2Float(0xc2a5fdb5));
path.cubicTo(SkBits2Float(0x3f642956), SkBits2Float(0xc2a5fd8c), SkBits2Float(0x3f6b855d), SkBits2Float(0xc2a5fd63), SkBits2Float(0x3f72e163), SkBits2Float(0xc2a5fd38));
path.lineTo(SkBits2Float(0x3f2f9381), SkBits2Float(0xc26ffbfc));
path.cubicTo(SkBits2Float(0x3f2a4188), SkBits2Float(0xc26ffc3b), SkBits2Float(0x3f24ef95), SkBits2Float(0xc26ffc76), SkBits2Float(0x3f1f9da0), SkBits2Float(0xc26ffcb0));
path.lineTo(SkBits2Float(0x3f5ccd1a), SkBits2Float(0xc2a5fdb5));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp61(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b23f68), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e9334c2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f13342a), SkBits2Float(0xc2a5ff3c), SkBits2Float(0x3f5ccd1a), SkBits2Float(0xc2a5fdb5));
path.cubicTo(SkBits2Float(0x3f642956), SkBits2Float(0xc2a5fd8c), SkBits2Float(0x3f6b855d), SkBits2Float(0xc2a5fd63), SkBits2Float(0x3f72e163), SkBits2Float(0xc2a5fd38));
path.lineTo(SkBits2Float(0x3f2f9381), SkBits2Float(0xc26ffbfc));
path.cubicTo(SkBits2Float(0x3f2a4188), SkBits2Float(0xc26ffc3b), SkBits2Float(0x3f24ef95), SkBits2Float(0xc26ffc76), SkBits2Float(0x3f1f9d85), SkBits2Float(0xc26ffcaf));
path.cubicTo(SkBits2Float(0x3ed4d324), SkBits2Float(0xc26ffee7), SkBits2Float(0x3e54d404), SkBits2Float(0xc2700000), SkBits2Float(0x36b23f68), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f72e162), SkBits2Float(0xc2a5fd39));
path.cubicTo(SkBits2Float(0x3fb51288), SkBits2Float(0xc2a5fa80), SkBits2Float(0x3ff0b297), SkBits2Float(0xc2a5f5c4), SkBits2Float(0x401627a5), SkBits2Float(0xc2a5ef06));
path.lineTo(SkBits2Float(0x3fd9177b), SkBits2Float(0xc26fe773));
path.cubicTo(SkBits2Float(0x3fadff90), SkBits2Float(0xc26ff134), SkBits2Float(0x3f82e54e), SkBits2Float(0xc26ff80c), SkBits2Float(0x3f2f9393), SkBits2Float(0xc26ffbfc));
path.lineTo(SkBits2Float(0x3f72e162), SkBits2Float(0xc2a5fd39));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp62(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f614848), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3fe14683), SkBits2Float(0xc2a5f8d5), SkBits2Float(0x4028ee0f), SkBits2Float(0xc2a5ea81));
path.lineTo(SkBits2Float(0x3ff43c76), SkBits2Float(0xc26fe0ec));
path.cubicTo(SkBits2Float(0x3fa2d98a), SkBits2Float(0xc26ff5a4), SkBits2Float(0x3f22dad5), SkBits2Float(0xc2700000), SkBits2Float(0xb5420574), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4028ee15), SkBits2Float(0xc2a5ea81));
path.cubicTo(SkBits2Float(0x402e8f25), SkBits2Float(0xc2a5e912), SkBits2Float(0x40343026), SkBits2Float(0xc2a5e791), SkBits2Float(0x4039d111), SkBits2Float(0xc2a5e5fd));
path.lineTo(SkBits2Float(0x4006533c), SkBits2Float(0xc26fda66));
path.cubicTo(SkBits2Float(0x4002419e), SkBits2Float(0xc26fdcaf), SkBits2Float(0x3ffc5fdb), SkBits2Float(0xc26fdedc), SkBits2Float(0x3ff43c61), SkBits2Float(0xc26fe0ed));
path.lineTo(SkBits2Float(0x4028ee15), SkBits2Float(0xc2a5ea81));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp63(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f614848), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3fe14683), SkBits2Float(0xc2a5f8d5), SkBits2Float(0x4028ee15), SkBits2Float(0xc2a5ea81));
path.cubicTo(SkBits2Float(0x402e8f25), SkBits2Float(0xc2a5e912), SkBits2Float(0x40343026), SkBits2Float(0xc2a5e791), SkBits2Float(0x4039d111), SkBits2Float(0xc2a5e5fd));
path.lineTo(SkBits2Float(0x4006533c), SkBits2Float(0xc26fda66));
path.cubicTo(SkBits2Float(0x400241a2), SkBits2Float(0xc26fdcaf), SkBits2Float(0x3ffc5fea), SkBits2Float(0xc26fdedc), SkBits2Float(0x3ff43c76), SkBits2Float(0xc26fe0ec));
path.cubicTo(SkBits2Float(0x3fa2d98a), SkBits2Float(0xc26ff5a4), SkBits2Float(0x3f22dad5), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();
path.moveTo(SkBits2Float(0x40186abb), SkBits2Float(0xc295b297));
path.lineTo(SkBits2Float(0x3ff43c61), SkBits2Float(0xc26fe0ed));
path.lineTo(SkBits2Float(0x3ff43c77), SkBits2Float(0xc26fe0ed));
path.lineTo(SkBits2Float(0x40186abb), SkBits2Float(0xc295b297));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4039d102), SkBits2Float(0xc2a5e5fe));
path.cubicTo(SkBits2Float(0x408a83ff), SkBits2Float(0xc2a5cc72), SkBits2Float(0x40b8130f), SkBits2Float(0xc2a5a01a), SkBits2Float(0x40e58a06), SkBits2Float(0xc2a56100));
path.lineTo(SkBits2Float(0x40a5ee90), SkBits2Float(0xc26f1a20));
path.cubicTo(SkBits2Float(0x408510de), SkBits2Float(0xc26f755e), SkBits2Float(0x40484386), SkBits2Float(0xc26fb57a), SkBits2Float(0x40065347), SkBits2Float(0xc26fda68));
path.lineTo(SkBits2Float(0x4039d102), SkBits2Float(0xc2a5e5fe));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp64(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3faf587e), SkBits2Float(0xc2a5ffff), SkBits2Float(0x402f5505), SkBits2Float(0xc2a5eea1), SkBits2Float(0x408372de), SkBits2Float(0xc2a5cbeb));
path.lineTo(SkBits2Float(0x403e0bd0), SkBits2Float(0xc26fb4b6));
path.cubicTo(SkBits2Float(0x3ffd7de6), SkBits2Float(0xc26fe6e6), SkBits2Float(0x3f7d82fb), SkBits2Float(0xc2700000), SkBits2Float(0x363f7eb2), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x408372d6), SkBits2Float(0xc2a5cbec));
path.cubicTo(SkBits2Float(0x4087d39d), SkBits2Float(0xc2a5c874), SkBits2Float(0x408c3440), SkBits2Float(0xc2a5c4cf), SkBits2Float(0x409094bd), SkBits2Float(0xc2a5c0fe));
path.lineTo(SkBits2Float(0x40510866), SkBits2Float(0xc26fa4e7));
path.cubicTo(SkBits2Float(0x404ab468), SkBits2Float(0xc26faa6c), SkBits2Float(0x40446037), SkBits2Float(0xc26fafb2), SkBits2Float(0x403e0bd2), SkBits2Float(0xc26fb4b7));
path.lineTo(SkBits2Float(0x408372d6), SkBits2Float(0xc2a5cbec));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp65(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x363f7eb2), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3faf5872), SkBits2Float(0xc2a60000), SkBits2Float(0x402f54f9), SkBits2Float(0xc2a5eea1), SkBits2Float(0x408372d5), SkBits2Float(0xc2a5cbeb));
path.lineTo(SkBits2Float(0x408372d6), SkBits2Float(0xc2a5cbec));
path.cubicTo(SkBits2Float(0x4087d39d), SkBits2Float(0xc2a5c874), SkBits2Float(0x408c3440), SkBits2Float(0xc2a5c4cf), SkBits2Float(0x409094bd), SkBits2Float(0xc2a5c0fe));
path.lineTo(SkBits2Float(0x40510866), SkBits2Float(0xc26fa4e7));
path.cubicTo(SkBits2Float(0x404ab468), SkBits2Float(0xc26faa6c), SkBits2Float(0x40446037), SkBits2Float(0xc26fafb2), SkBits2Float(0x403e0bd0), SkBits2Float(0xc26fb4b6));
path.cubicTo(SkBits2Float(0x3ffd7de6), SkBits2Float(0xc26fe6e6), SkBits2Float(0x3f7d82fb), SkBits2Float(0xc2700000), SkBits2Float(0x363f7eb2), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x409094be), SkBits2Float(0xc2a5c0fe));
path.cubicTo(SkBits2Float(0x40d784bb), SkBits2Float(0xc2a5831d), SkBits2Float(0x410f22d3), SkBits2Float(0xc2a517ba), SkBits2Float(0x413255ec), SkBits2Float(0xc2a47f15));
path.lineTo(SkBits2Float(0x4100ead4), SkBits2Float(0xc26dd37e));
path.cubicTo(SkBits2Float(0x40cef193), SkBits2Float(0xc26eb02f), SkBits2Float(0x409bcbdf), SkBits2Float(0xc26f4b72), SkBits2Float(0x40510859), SkBits2Float(0xc26fa4e8));
path.lineTo(SkBits2Float(0x409094be), SkBits2Float(0xc2a5c0fe));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp66(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4037e518), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40b7d534), SkBits2Float(0xc2a5b39a), SkBits2Float(0x4109a47d), SkBits2Float(0xc2a51b1f));
path.lineTo(SkBits2Float(0x40c70051), SkBits2Float(0xc26eb519));
path.cubicTo(SkBits2Float(0x4084e427), SkBits2Float(0xc26f918c), SkBits2Float(0x4004efa4), SkBits2Float(0xc26fffff), SkBits2Float(0x3543fa8c), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4109a47c), SkBits2Float(0xc2a51b20));
path.cubicTo(SkBits2Float(0x410e36d1), SkBits2Float(0xc2a50be2), SkBits2Float(0x4112c883), SkBits2Float(0xc2a4fbe1), SkBits2Float(0x41175985), SkBits2Float(0xc2a4eb1d));
path.lineTo(SkBits2Float(0x40dad196), SkBits2Float(0xc26e6faf));
path.cubicTo(SkBits2Float(0x40d4377d), SkBits2Float(0xc26e87ed), SkBits2Float(0x40cd9c5c), SkBits2Float(0xc26e9f10), SkBits2Float(0x40c7004e), SkBits2Float(0xc26eb51a));
path.lineTo(SkBits2Float(0x4109a47c), SkBits2Float(0xc2a51b20));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp67(skiatest::Reporter* reporter, const char* filename) { // crashed
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4037e518), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40b7d534), SkBits2Float(0xc2a5b39a), SkBits2Float(0x4109a47c), SkBits2Float(0xc2a51b20));
path.cubicTo(SkBits2Float(0x410e36d1), SkBits2Float(0xc2a50be2), SkBits2Float(0x4112c883), SkBits2Float(0xc2a4fbe1), SkBits2Float(0x41175985), SkBits2Float(0xc2a4eb1d));
path.lineTo(SkBits2Float(0x40dad196), SkBits2Float(0xc26e6faf));
path.cubicTo(SkBits2Float(0x40d4377e), SkBits2Float(0xc26e87ed), SkBits2Float(0x40cd9c5f), SkBits2Float(0xc26e9f10), SkBits2Float(0x40c70052), SkBits2Float(0xc26eb51a));
path.lineTo(SkBits2Float(0x40c70051), SkBits2Float(0xc26eb519));
path.cubicTo(SkBits2Float(0x4084e427), SkBits2Float(0xc26f918c), SkBits2Float(0x4004efa4), SkBits2Float(0xc26fffff), SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4117597f), SkBits2Float(0xc2a4eb1d));
path.cubicTo(SkBits2Float(0x41616445), SkBits2Float(0xc2a3db51), SkBits2Float(0x41954b2d), SkBits2Float(0xc2a2048b), SkBits2Float(0x41b914a4), SkBits2Float(0xc29f6bcb));
path.lineTo(SkBits2Float(0x4185cb10), SkBits2Float(0xc2667d00));
path.cubicTo(SkBits2Float(0x4157d8a2), SkBits2Float(0xc26a3e17), SkBits2Float(0x4122ef07), SkBits2Float(0xc26ce6b9), SkBits2Float(0x40dad195), SkBits2Float(0xc26e6faf));
path.lineTo(SkBits2Float(0x4117597f), SkBits2Float(0xc2a4eb1d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp68(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e1b2207), SkBits2Float(0xc2a60000), SkBits2Float(0x3e9b2105), SkBits2Float(0xc2a5ffca), SkBits2Float(0x3ee8b0c0), SkBits2Float(0xc2a5ff5d));
path.lineTo(SkBits2Float(0x3ea83563), SkBits2Float(0xc26fff14));
path.cubicTo(SkBits2Float(0x3e60486a), SkBits2Float(0xc26fffb2), SkBits2Float(0x3de049e3), SkBits2Float(0xc2700000), SkBits2Float(0x36b67768), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ee8b040), SkBits2Float(0xc2a5ff5d));
path.cubicTo(SkBits2Float(0x3ef0720a), SkBits2Float(0xc2a5ff52), SkBits2Float(0x3ef83386), SkBits2Float(0xc2a5ff47), SkBits2Float(0x3efff501), SkBits2Float(0xc2a5ff3b));
path.lineTo(SkBits2Float(0x3eb90778), SkBits2Float(0xc26ffee3));
path.cubicTo(SkBits2Float(0x3eb36c27), SkBits2Float(0xc26ffef6), SkBits2Float(0x3eadd0dd), SkBits2Float(0xc26fff07), SkBits2Float(0x3ea83592), SkBits2Float(0xc26fff16));
path.lineTo(SkBits2Float(0x3ee8b040), SkBits2Float(0xc2a5ff5d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp69(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b67768), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e1b21b2), SkBits2Float(0xc2a60000), SkBits2Float(0x3e9b20b0), SkBits2Float(0xc2a5ffca), SkBits2Float(0x3ee8b040), SkBits2Float(0xc2a5ff5d));
path.cubicTo(SkBits2Float(0x3ef0720a), SkBits2Float(0xc2a5ff52), SkBits2Float(0x3ef83386), SkBits2Float(0xc2a5ff47), SkBits2Float(0x3efff501), SkBits2Float(0xc2a5ff3b));
path.lineTo(SkBits2Float(0x3eb90778), SkBits2Float(0xc26ffee3));
path.lineTo(SkBits2Float(0x3ea83592), SkBits2Float(0xc26fff16));
path.lineTo(SkBits2Float(0x3ea83563), SkBits2Float(0xc26fff14));
path.cubicTo(SkBits2Float(0x3e60486a), SkBits2Float(0xc26fffb2), SkBits2Float(0x3de049e3), SkBits2Float(0xc2700000), SkBits2Float(0x36b67768), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3efff501), SkBits2Float(0xc2a5ff3b));
path.cubicTo(SkBits2Float(0x3f3ed289), SkBits2Float(0xc2a5fe79), SkBits2Float(0x3f7daa5c), SkBits2Float(0xc2a5fd28), SkBits2Float(0x3f9e4099), SkBits2Float(0xc2a5fb49));
path.lineTo(SkBits2Float(0x3f64cc5f), SkBits2Float(0xc26ff92f));
path.cubicTo(SkBits2Float(0x3f375f8f), SkBits2Float(0xc26ffbe5), SkBits2Float(0x3f09f1cf), SkBits2Float(0xc26ffdcc), SkBits2Float(0x3eb9075f), SkBits2Float(0xc26ffee4));
path.lineTo(SkBits2Float(0x3efff501), SkBits2Float(0xc2a5ff3b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp70(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f0938d2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f893841), SkBits2Float(0xc2a5fd56), SkBits2Float(0x3fcdd137), SkBits2Float(0xc2a5f805));
path.lineTo(SkBits2Float(0x3f94c89b), SkBits2Float(0xc26ff478));
path.cubicTo(SkBits2Float(0x3f4663c1), SkBits2Float(0xc26ffc29), SkBits2Float(0x3ec6647d), SkBits2Float(0xc2700000), SkBits2Float(0x360ebeb2), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fcdd13c), SkBits2Float(0xc2a5f806));
path.cubicTo(SkBits2Float(0x3fd4ad55), SkBits2Float(0xc2a5f77d), SkBits2Float(0x3fdb895f), SkBits2Float(0xc2a5f6ef), SkBits2Float(0x3fe26560), SkBits2Float(0xc2a5f659));
path.lineTo(SkBits2Float(0x3fa3a8ea), SkBits2Float(0xc26ff20c));
path.cubicTo(SkBits2Float(0x3f9eb37e), SkBits2Float(0xc26ff2e6), SkBits2Float(0x3f99be11), SkBits2Float(0xc26ff3b4), SkBits2Float(0x3f94c89e), SkBits2Float(0xc26ff479));
path.lineTo(SkBits2Float(0x3fcdd13c), SkBits2Float(0xc2a5f806));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp71(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x360ebeb2), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f0938d2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f893841), SkBits2Float(0xc2a5fd56), SkBits2Float(0x3fcdd13c), SkBits2Float(0xc2a5f806));
path.cubicTo(SkBits2Float(0x3fd4ad55), SkBits2Float(0xc2a5f77d), SkBits2Float(0x3fdb895f), SkBits2Float(0xc2a5f6ef), SkBits2Float(0x3fe26560), SkBits2Float(0xc2a5f659));
path.lineTo(SkBits2Float(0x3fa3a8ea), SkBits2Float(0xc26ff20c));
path.cubicTo(SkBits2Float(0x3f9eb37e), SkBits2Float(0xc26ff2e6), SkBits2Float(0x3f99be11), SkBits2Float(0xc26ff3b4), SkBits2Float(0x3f94c89b), SkBits2Float(0xc26ff478));
path.cubicTo(SkBits2Float(0x3f4663c1), SkBits2Float(0xc26ffc29), SkBits2Float(0x3ec6647d), SkBits2Float(0xc2700000), SkBits2Float(0x360ebeb2), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fe26566), SkBits2Float(0xc2a5f65a));
path.cubicTo(SkBits2Float(0x4028c729), SkBits2Float(0xc2a5ecdf), SkBits2Float(0x406055f2), SkBits2Float(0xc2a5dc6a), SkBits2Float(0x408beceb), SkBits2Float(0xc2a5c4fb));
path.lineTo(SkBits2Float(0x404a4d47), SkBits2Float(0xc26faaae));
path.cubicTo(SkBits2Float(0x40222b9c), SkBits2Float(0xc26fcc90), SkBits2Float(0x3ff40427), SkBits2Float(0xc26fe45b), SkBits2Float(0x3fa3a8ee), SkBits2Float(0xc26ff20e));
path.lineTo(SkBits2Float(0x3fe26566), SkBits2Float(0xc2a5f65a));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp72(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f73aa4a), SkBits2Float(0xc2a60000), SkBits2Float(0x3ff3a7f0), SkBits2Float(0xc2a5f79e), SkBits2Float(0x4036b54b), SkBits2Float(0xc2a5e6db));
path.lineTo(SkBits2Float(0x40041412), SkBits2Float(0xc26fdba5));
path.cubicTo(SkBits2Float(0x3fb0230c), SkBits2Float(0xc26ff3e0), SkBits2Float(0x3f3024c1), SkBits2Float(0xc26fffff), SkBits2Float(0x359dfd4a), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4036b55d), SkBits2Float(0xc2a5e6db));
path.cubicTo(SkBits2Float(0x403ccbdf), SkBits2Float(0xc2a5e52d), SkBits2Float(0x4042e24c), SkBits2Float(0xc2a5e36a), SkBits2Float(0x4048f89e), SkBits2Float(0xc2a5e192));
path.lineTo(SkBits2Float(0x401147bc), SkBits2Float(0xc26fd403));
path.cubicTo(SkBits2Float(0x400ce144), SkBits2Float(0xc26fd6ae), SkBits2Float(0x40087ab2), SkBits2Float(0xc26fd939), SkBits2Float(0x4004140f), SkBits2Float(0xc26fdba5));
path.lineTo(SkBits2Float(0x4036b55d), SkBits2Float(0xc2a5e6db));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end fail 1

static void battleOp73(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40447e19), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40c46ab2), SkBits2Float(0xc2a5a8c7), SkBits2Float(0x4113078c), SkBits2Float(0xc2a4fabe));
path.lineTo(SkBits2Float(0x40d4929e), SkBits2Float(0xc26e8647));
path.cubicTo(SkBits2Float(0x408dfcf1), SkBits2Float(0xc26f81e6), SkBits2Float(0x400e0af8), SkBits2Float(0xc2700000), SkBits2Float(0x3655fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4113078b), SkBits2Float(0xc2a4fabe));
path.cubicTo(SkBits2Float(0x4117e908), SkBits2Float(0xc2a4e957), SkBits2Float(0x411cc9c0), SkBits2Float(0xc2a4d714), SkBits2Float(0x4121a9a1), SkBits2Float(0xc2a4c3f3));
path.lineTo(SkBits2Float(0x40e9baad), SkBits2Float(0xc26e370e));
path.cubicTo(SkBits2Float(0x40e2ae85), SkBits2Float(0xc26e52b6), SkBits2Float(0x40dba120), SkBits2Float(0xc26e6d20), SkBits2Float(0x40d4929a), SkBits2Float(0xc26e8647));
path.lineTo(SkBits2Float(0x4113078b), SkBits2Float(0xc2a4fabe));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end fail 1

static void battleOp74(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x406db78d), SkBits2Float(0xc2a60000), SkBits2Float(0x40ed953d), SkBits2Float(0xc2a58058), SkBits2Float(0x4131afb7), SkBits2Float(0xc2a481e4));
path.lineTo(SkBits2Float(0x410072b2), SkBits2Float(0xc26dd78e));
path.cubicTo(SkBits2Float(0x40abbf2e), SkBits2Float(0xc26f4770), SkBits2Float(0x402bd807), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4131afba), SkBits2Float(0xc2a481e4));
path.cubicTo(SkBits2Float(0x413792dd), SkBits2Float(0xc2a46874), SkBits2Float(0x413d74a2), SkBits2Float(0xc2a44dc1), SkBits2Float(0x414354e9), SkBits2Float(0xc2a431ca));
path.lineTo(SkBits2Float(0x410d3424), SkBits2Float(0xc26d63c0));
path.cubicTo(SkBits2Float(0x4108f4b6), SkBits2Float(0xc26d8c2e), SkBits2Float(0x4104b435), SkBits2Float(0xc26db2c8), SkBits2Float(0x410072b4), SkBits2Float(0xc26dd78e));
path.lineTo(SkBits2Float(0x4131afba), SkBits2Float(0xc2a481e4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp75(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x406db78d), SkBits2Float(0xc2a60000), SkBits2Float(0x40ed953d), SkBits2Float(0xc2a58058), SkBits2Float(0x4131afba), SkBits2Float(0xc2a481e4));
path.cubicTo(SkBits2Float(0x413792dd), SkBits2Float(0xc2a46874), SkBits2Float(0x413d74a2), SkBits2Float(0xc2a44dc1), SkBits2Float(0x414354e9), SkBits2Float(0xc2a431ca));
path.lineTo(SkBits2Float(0x410d3424), SkBits2Float(0xc26d63c0));
path.cubicTo(SkBits2Float(0x4108f4b6), SkBits2Float(0xc26d8c2e), SkBits2Float(0x4104b435), SkBits2Float(0xc26db2c8), SkBits2Float(0x410072b2), SkBits2Float(0xc26dd78e));
path.cubicTo(SkBits2Float(0x40abbf2e), SkBits2Float(0xc26f4770), SkBits2Float(0x402bd807), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x414354ed), SkBits2Float(0xc2a431cb));
path.cubicTo(SkBits2Float(0x419152e5), SkBits2Float(0xc2a26c3a), SkBits2Float(0x41c0119b), SkBits2Float(0xc29f5c06), SkBits2Float(0x41ed1335), SkBits2Float(0xc29b0f0a));
path.lineTo(SkBits2Float(0x41ab612b), SkBits2Float(0xc2602e6b));
path.cubicTo(SkBits2Float(0x418ad84d), SkBits2Float(0xc2666635), SkBits2Float(0x41521b54), SkBits2Float(0xc26ad3fe), SkBits2Float(0x410d3426), SkBits2Float(0xc26d63c0));
path.lineTo(SkBits2Float(0x414354ed), SkBits2Float(0xc2a431cb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp76(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40932e58), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41130dbc), SkBits2Float(0xc2a53c41), SkBits2Float(0x415ba178), SkBits2Float(0xc2a3b6ca));
path.lineTo(SkBits2Float(0x411ec4eb), SkBits2Float(0xc26cb1eb));
path.cubicTo(SkBits2Float(0x40d49b93), SkBits2Float(0xc26ee4ff), SkBits2Float(0x4054cab9), SkBits2Float(0xc26fffff), SkBits2Float(0x35f7fd46), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x415ba178), SkBits2Float(0xc2a3b6cb));
path.cubicTo(SkBits2Float(0x4162e261), SkBits2Float(0xc2a38fde), SkBits2Float(0x416a20aa), SkBits2Float(0xc2a36704), SkBits2Float(0x41715c23), SkBits2Float(0xc2a33c3e));
path.lineTo(SkBits2Float(0x412e7a25), SkBits2Float(0xc26c00bd));
path.cubicTo(SkBits2Float(0x41293fb6), SkBits2Float(0xc26c3e94), SkBits2Float(0x41240342), SkBits2Float(0xc26c79a4), SkBits2Float(0x411ec4e8), SkBits2Float(0xc26cb1eb));
path.lineTo(SkBits2Float(0x415ba178), SkBits2Float(0xc2a3b6cb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end fail 1

static void battleOp77(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40d0158a), SkBits2Float(0xc2a60000), SkBits2Float(0x414fb944), SkBits2Float(0xc2a478c0), SkBits2Float(0x419a74b5), SkBits2Float(0xc2a1724b));
path.lineTo(SkBits2Float(0x415f4f4c), SkBits2Float(0xc2696aa5));
path.cubicTo(SkBits2Float(0x41162967), SkBits2Float(0xc26dca57), SkBits2Float(0x40966c1f), SkBits2Float(0xc2700000), SkBits2Float(0x3655fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x419a74b6), SkBits2Float(0xc2a1724b));
path.cubicTo(SkBits2Float(0x419f8274), SkBits2Float(0xc2a124ef), SkBits2Float(0x41a48c82), SkBits2Float(0xc2a0d3c9), SkBits2Float(0x41a9929f), SkBits2Float(0xc2a07edb));
path.lineTo(SkBits2Float(0x41752a58), SkBits2Float(0xc2680ab0));
path.cubicTo(SkBits2Float(0x416de6e6), SkBits2Float(0xc268857b), SkBits2Float(0x41669dc0), SkBits2Float(0xc268facf), SkBits2Float(0x415f4f4b), SkBits2Float(0xc2696aa6));
path.lineTo(SkBits2Float(0x419a74b6), SkBits2Float(0xc2a1724b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp78(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3655fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40d0158a), SkBits2Float(0xc2a60000), SkBits2Float(0x414fb944), SkBits2Float(0xc2a478c0), SkBits2Float(0x419a74b6), SkBits2Float(0xc2a1724b));
path.cubicTo(SkBits2Float(0x419f8274), SkBits2Float(0xc2a124ef), SkBits2Float(0x41a48c82), SkBits2Float(0xc2a0d3c9), SkBits2Float(0x41a9929f), SkBits2Float(0xc2a07edb));
path.lineTo(SkBits2Float(0x41752a58), SkBits2Float(0xc2680ab0));
path.cubicTo(SkBits2Float(0x416de6e6), SkBits2Float(0xc268857b), SkBits2Float(0x41669dc0), SkBits2Float(0xc268facf), SkBits2Float(0x415f4f4c), SkBits2Float(0xc2696aa5));
path.cubicTo(SkBits2Float(0x41162967), SkBits2Float(0xc26dca57), SkBits2Float(0x40966c1f), SkBits2Float(0xc2700000), SkBits2Float(0x3655fea3), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41a9929f), SkBits2Float(0xc2a07edc));
path.cubicTo(SkBits2Float(0x41fb3aee), SkBits2Float(0xc29b1a71), SkBits2Float(0x422402f4), SkBits2Float(0xc291ddaf), SkBits2Float(0x4245eaa6), SkBits2Float(0xc2854763));
path.lineTo(SkBits2Float(0x420f1280), SkBits2Float(0xc240b13c));
path.cubicTo(SkBits2Float(0x41ed200b), SkBits2Float(0xc252e3f9), SkBits2Float(0x41b59cbb), SkBits2Float(0xc2603ee8), SkBits2Float(0x41752a58), SkBits2Float(0xc2680aaf));
path.lineTo(SkBits2Float(0x41a9929f), SkBits2Float(0xc2a07edc));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp79(skiatest::Reporter* reporter, const char* filename) {  //crashed
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4110a0cc), SkBits2Float(0xc2a60000), SkBits2Float(0x4190247a), SkBits2Float(0xc2a30bfe), SkBits2Float(0x41d4a5dc), SkBits2Float(0xc29d41d4));
path.lineTo(SkBits2Float(0x4199b8a9), SkBits2Float(0xc2635c16));
path.cubicTo(SkBits2Float(0x4150660f), SkBits2Float(0xc26bbaf8), SkBits2Float(0x40d119d0), SkBits2Float(0xc2700000), SkBits2Float(0x3673fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41d4a5d9), SkBits2Float(0xc29d41d4));
path.cubicTo(SkBits2Float(0x41db7bbd), SkBits2Float(0xc29cadef), SkBits2Float(0x41e247df), SkBits2Float(0xc29c12ec), SkBits2Float(0x41e9098d), SkBits2Float(0xc29b70d9));
path.lineTo(SkBits2Float(0x41a875f1), SkBits2Float(0xc260bbd5));
path.cubicTo(SkBits2Float(0x41a39393), SkBits2Float(0xc261a627), SkBits2Float(0x419ea9a6), SkBits2Float(0xc2628645), SkBits2Float(0x4199b8ab), SkBits2Float(0xc2635c17));
path.lineTo(SkBits2Float(0x41d4a5d9), SkBits2Float(0xc29d41d4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp80(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e15a675), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e95a67a), SkBits2Float(0xc2a5ffcd), SkBits2Float(0x3ee07980), SkBits2Float(0xc2a5ff68));
path.lineTo(SkBits2Float(0x3ea245bb), SkBits2Float(0xc26fff25));
path.cubicTo(SkBits2Float(0x3e585de0), SkBits2Float(0xc26fffb9), SkBits2Float(0x3dd85f11), SkBits2Float(0xc2700000), SkBits2Float(0x3691e768), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ee07a10), SkBits2Float(0xc2a5ff68));
path.cubicTo(SkBits2Float(0x3ee7f565), SkBits2Float(0xc2a5ff5d), SkBits2Float(0x3eef70d9), SkBits2Float(0xc2a5ff52), SkBits2Float(0x3ef6ec4d), SkBits2Float(0xc2a5ff47));
path.lineTo(SkBits2Float(0x3eb27fdb), SkBits2Float(0xc26ffef6));
path.cubicTo(SkBits2Float(0x3ead1768), SkBits2Float(0xc26fff07), SkBits2Float(0x3ea7aebe), SkBits2Float(0xc26fff17), SkBits2Float(0x3ea24612), SkBits2Float(0xc26fff26));
path.lineTo(SkBits2Float(0x3ee07a10), SkBits2Float(0xc2a5ff68));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp81(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3691e768), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e15a675), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e95a67a), SkBits2Float(0xc2a5ffcd), SkBits2Float(0x3ee07a10), SkBits2Float(0xc2a5ff68));
path.lineTo(SkBits2Float(0x3ef6ec4d), SkBits2Float(0xc2a5ff47));
path.lineTo(SkBits2Float(0x3eb27fdb), SkBits2Float(0xc26ffef6));
path.cubicTo(SkBits2Float(0x3ead1768), SkBits2Float(0xc26fff07), SkBits2Float(0x3ea7aebe), SkBits2Float(0xc26fff17), SkBits2Float(0x3ea245bb), SkBits2Float(0xc26fff25));
path.cubicTo(SkBits2Float(0x3e585de0), SkBits2Float(0xc26fffb9), SkBits2Float(0x3dd85f11), SkBits2Float(0xc2700000), SkBits2Float(0x3691e768), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ef6ec9b), SkBits2Float(0xc2a5ff48));
path.cubicTo(SkBits2Float(0x3f3816c9), SkBits2Float(0xc2a5fe94), SkBits2Float(0x3f74b6e1), SkBits2Float(0xc2a5fd5b), SkBits2Float(0x3f98ab0b), SkBits2Float(0xc2a5fb9d));
path.lineTo(SkBits2Float(0x3f5cb973), SkBits2Float(0xc26ff9a8));
path.cubicTo(SkBits2Float(0x3f30e6e7), SkBits2Float(0xc26ffc2e), SkBits2Float(0x3f05138e), SkBits2Float(0xc26ffdf2), SkBits2Float(0x3eb27fc6), SkBits2Float(0xc26ffef7));
path.lineTo(SkBits2Float(0x3ef6ec9b), SkBits2Float(0xc2a5ff48));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp82(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3eff98a5), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f7f97b3), SkBits2Float(0xc2a5fdb1), SkBits2Float(0x3fbfaf38), SkBits2Float(0xc2a5f914));
path.lineTo(SkBits2Float(0x3f8a9112), SkBits2Float(0xc26ff600));
path.cubicTo(SkBits2Float(0x3f38c3e7), SkBits2Float(0xc26ffcab), SkBits2Float(0x3eb8c475), SkBits2Float(0xc2700000), SkBits2Float(0x35877d28), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fbfaf15), SkBits2Float(0xc2a5f915));
path.cubicTo(SkBits2Float(0x3fc612b4), SkBits2Float(0xc2a5f8a0), SkBits2Float(0x3fcc7634), SkBits2Float(0xc2a5f824), SkBits2Float(0x3fd2d9ad), SkBits2Float(0xc2a5f7a2));
path.lineTo(SkBits2Float(0x3f986bef), SkBits2Float(0xc26ff3e6));
path.cubicTo(SkBits2Float(0x3f93cdb9), SkBits2Float(0xc26ff4a2), SkBits2Float(0x3f8f2f70), SkBits2Float(0xc26ff556), SkBits2Float(0x3f8a9121), SkBits2Float(0xc26ff601));
path.lineTo(SkBits2Float(0x3fbfaf15), SkBits2Float(0xc2a5f915));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp83(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3eff9875), SkBits2Float(0xc2a60000), SkBits2Float(0x3f7f9783), SkBits2Float(0xc2a5fdb1), SkBits2Float(0x3fbfaf14), SkBits2Float(0xc2a5f914));
path.lineTo(SkBits2Float(0x3fbfaf15), SkBits2Float(0xc2a5f915));
path.cubicTo(SkBits2Float(0x3fc612b4), SkBits2Float(0xc2a5f8a0), SkBits2Float(0x3fcc7634), SkBits2Float(0xc2a5f824), SkBits2Float(0x3fd2d9ad), SkBits2Float(0xc2a5f7a2));
path.lineTo(SkBits2Float(0x3f986bef), SkBits2Float(0xc26ff3e6));
path.cubicTo(SkBits2Float(0x3f93cdb9), SkBits2Float(0xc26ff4a2), SkBits2Float(0x3f8f2f70), SkBits2Float(0xc26ff556), SkBits2Float(0x3f8a9112), SkBits2Float(0xc26ff600));
path.cubicTo(SkBits2Float(0x3f38c3e7), SkBits2Float(0xc26ffcab), SkBits2Float(0x3eb8c475), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fd2d994), SkBits2Float(0xc2a5f7a1));
path.cubicTo(SkBits2Float(0x401d305c), SkBits2Float(0xc2a5ef69), SkBits2Float(0x4050ef71), SkBits2Float(0xc2a5e123), SkBits2Float(0x408252dc), SkBits2Float(0xc2a5ccd0));
path.lineTo(SkBits2Float(0x403c6b7d), SkBits2Float(0xc26fb5fe));
path.cubicTo(SkBits2Float(0x401709a2), SkBits2Float(0xc26fd362), SkBits2Float(0x3fe342dd), SkBits2Float(0xc26fe805), SkBits2Float(0x3f986be0), SkBits2Float(0xc26ff3e7));
path.lineTo(SkBits2Float(0x3fd2d994), SkBits2Float(0xc2a5f7a1));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp84(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f541e8b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3fd41d19), SkBits2Float(0xc2a5f9a6), SkBits2Float(0x401f1022), SkBits2Float(0xc2a5ecf2));
path.lineTo(SkBits2Float(0x3fe5f882), SkBits2Float(0xc26fe473));
path.cubicTo(SkBits2Float(0x3f9955cf), SkBits2Float(0xc26ff6d2), SkBits2Float(0x3f1956dc), SkBits2Float(0xc2700000), SkBits2Float(0xb5bb02d8), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x401f1027), SkBits2Float(0xc2a5ecf2));
path.cubicTo(SkBits2Float(0x40245d21), SkBits2Float(0xc2a5ebac), SkBits2Float(0x4029aa04), SkBits2Float(0xc2a5ea57), SkBits2Float(0x402ef6d6), SkBits2Float(0xc2a5e8f1));
path.lineTo(SkBits2Float(0x3ffcf5ba), SkBits2Float(0xc26fdeaa));
path.cubicTo(SkBits2Float(0x3ff54c2d), SkBits2Float(0xc26fe0b0), SkBits2Float(0x3feda268), SkBits2Float(0xc26fe29e), SkBits2Float(0x3fe5f88e), SkBits2Float(0xc26fe474));
path.lineTo(SkBits2Float(0x401f1027), SkBits2Float(0xc2a5ecf2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp85(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f541e8b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3fd41d19), SkBits2Float(0xc2a5f9a6), SkBits2Float(0x401f1027), SkBits2Float(0xc2a5ecf2));
path.cubicTo(SkBits2Float(0x40245d21), SkBits2Float(0xc2a5ebac), SkBits2Float(0x4029aa04), SkBits2Float(0xc2a5ea57), SkBits2Float(0x402ef6d6), SkBits2Float(0xc2a5e8f1));
path.lineTo(SkBits2Float(0x3ffcf5ba), SkBits2Float(0xc26fdeaa));
path.cubicTo(SkBits2Float(0x3ff54c2d), SkBits2Float(0xc26fe0b0), SkBits2Float(0x3feda268), SkBits2Float(0xc26fe29e), SkBits2Float(0x3fe5f882), SkBits2Float(0xc26fe473));
path.cubicTo(SkBits2Float(0x3f9955cf), SkBits2Float(0xc26ff6d2), SkBits2Float(0x3f1956dc), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x402ef6c3), SkBits2Float(0xc2a5e8f1));
path.cubicTo(SkBits2Float(0x40826d68), SkBits2Float(0xc2a5d24c), SkBits2Float(0x40ad550a), SkBits2Float(0xc2a5aafb), SkBits2Float(0x40d82890), SkBits2Float(0xc2a57308));
path.lineTo(SkBits2Float(0x409c425c), SkBits2Float(0xc26f3430));
path.cubicTo(SkBits2Float(0x407a99d8), SkBits2Float(0xc26f8515), SkBits2Float(0x403c91e6), SkBits2Float(0xc26fbded), SkBits2Float(0x3ffcf5ca), SkBits2Float(0xc26fdeaa));
path.lineTo(SkBits2Float(0x402ef6c3), SkBits2Float(0xc2a5e8f1));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp86(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40155bee), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40955364), SkBits2Float(0xc2a5cd99), SkBits2Float(0x40dfbd5f), SkBits2Float(0xc2a568f2));
path.lineTo(SkBits2Float(0x40a1bd53), SkBits2Float(0xc26f259d));
path.cubicTo(SkBits2Float(0x4057e483), SkBits2Float(0xc26fb724), SkBits2Float(0x3fd7f0d9), SkBits2Float(0xc2700000), SkBits2Float(0x3619fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40dfbd5e), SkBits2Float(0xc2a568f3));
path.cubicTo(SkBits2Float(0x40e72e1b), SkBits2Float(0xc2a55ee2), SkBits2Float(0x40ee9e1c), SkBits2Float(0xc2a55452), SkBits2Float(0x40f60d62), SkBits2Float(0xc2a54941));
path.lineTo(SkBits2Float(0x40b1de84), SkBits2Float(0xc26ef7c9));
path.cubicTo(SkBits2Float(0x40ac7ea0), SkBits2Float(0xc26f07cb), SkBits2Float(0x40a71e37), SkBits2Float(0xc26f1712), SkBits2Float(0x40a1bd4f), SkBits2Float(0xc26f259f));
path.lineTo(SkBits2Float(0x40dfbd5e), SkBits2Float(0xc2a568f3));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp87(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3619fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40155bee), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40955364), SkBits2Float(0xc2a5cd99), SkBits2Float(0x40dfbd5e), SkBits2Float(0xc2a568f3));
path.cubicTo(SkBits2Float(0x40e72e1b), SkBits2Float(0xc2a55ee2), SkBits2Float(0x40ee9e1c), SkBits2Float(0xc2a55452), SkBits2Float(0x40f60d62), SkBits2Float(0xc2a54941));
path.lineTo(SkBits2Float(0x40b1de84), SkBits2Float(0xc26ef7c9));
path.cubicTo(SkBits2Float(0x40ac7ea2), SkBits2Float(0xc26f07cb), SkBits2Float(0x40a71e3a), SkBits2Float(0xc26f1712), SkBits2Float(0x40a1bd54), SkBits2Float(0xc26f259f));
path.lineTo(SkBits2Float(0x40a1bd53), SkBits2Float(0xc26f259d));
path.cubicTo(SkBits2Float(0x4057e483), SkBits2Float(0xc26fb724), SkBits2Float(0x3fd7f0d9), SkBits2Float(0xc2700000), SkBits2Float(0x3619fea3), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40f60d69), SkBits2Float(0xc2a54941));
path.cubicTo(SkBits2Float(0x41374a21), SkBits2Float(0xc2a495d5), SkBits2Float(0x41731962), SkBits2Float(0xc2a35eca), SkBits2Float(0x419704b1), SkBits2Float(0xc2a1a64c));
path.lineTo(SkBits2Float(0x415a56f5), SkBits2Float(0xc269b5d4));
path.cubicTo(SkBits2Float(0x412fbbfb), SkBits2Float(0xc26c32af), SkBits2Float(0x41047f9a), SkBits2Float(0xc26df463), SkBits2Float(0x40b1de7e), SkBits2Float(0xc26ef7cb));
path.lineTo(SkBits2Float(0x40f60d69), SkBits2Float(0xc2a54941));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp88(skiatest::Reporter* reporter, const char* filename) {  // crashed
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4059d383), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40d9b918), SkBits2Float(0xc2a594d0), SkBits2Float(0x4122e820), SkBits2Float(0xc2a4bf0c));
path.lineTo(SkBits2Float(0x40eb871c), SkBits2Float(0xc26e2ff8));
path.cubicTo(SkBits2Float(0x409d63e0), SkBits2Float(0xc26f6508), SkBits2Float(0x401d76fa), SkBits2Float(0xc2700000), SkBits2Float(0x35f7fd4a), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4122e81e), SkBits2Float(0xc2a4bf0c));
path.cubicTo(SkBits2Float(0x41284f3c), SkBits2Float(0xc2a4a9ac), SkBits2Float(0x412db549), SkBits2Float(0xc2a4933e), SkBits2Float(0x41331a33), SkBits2Float(0xc2a47bbf));
path.lineTo(SkBits2Float(0x410178be), SkBits2Float(0xc26dceac));
path.cubicTo(SkBits2Float(0x40fb24f7), SkBits2Float(0xc26df0a4), SkBits2Float(0x40f356d1), SkBits2Float(0xc26e1114), SkBits2Float(0x40eb871f), SkBits2Float(0xc26e2ff8));
path.lineTo(SkBits2Float(0x4122e81e), SkBits2Float(0xc2a4bf0c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp89(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3dd41fb8), SkBits2Float(0xc2a5fffe), SkBits2Float(0x3e541e5b), SkBits2Float(0xc2a5ffe5), SkBits2Float(0x3e9f1657), SkBits2Float(0xc2a5ffb2));
path.lineTo(SkBits2Float(0x3e66012b), SkBits2Float(0xc26fff92));
path.cubicTo(SkBits2Float(0x3e1955e2), SkBits2Float(0xc26fffdc), SkBits2Float(0x3d99560b), SkBits2Float(0xc2700000), SkBits2Float(0x350f7780), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3e9f1626), SkBits2Float(0xc2a5ffb4));
path.cubicTo(SkBits2Float(0x3ea463a8), SkBits2Float(0xc2a5ffae), SkBits2Float(0x3ea9b10b), SkBits2Float(0xc2a5ffa8), SkBits2Float(0x3eaefe6d), SkBits2Float(0xc2a5ffa3));
path.lineTo(SkBits2Float(0x3e7d0144), SkBits2Float(0xc26fff7b));
path.cubicTo(SkBits2Float(0x3e75568f), SkBits2Float(0xc26fff84), SkBits2Float(0x3e6dac12), SkBits2Float(0xc26fff8c), SkBits2Float(0x3e660197), SkBits2Float(0xc26fff93));
path.lineTo(SkBits2Float(0x3e9f1626), SkBits2Float(0xc2a5ffb4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp90(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3dd41f74), SkBits2Float(0xc2a5fffe), SkBits2Float(0x3e541e17), SkBits2Float(0xc2a5ffe5), SkBits2Float(0x3e9f1624), SkBits2Float(0xc2a5ffb2));
path.lineTo(SkBits2Float(0x3e9f1626), SkBits2Float(0xc2a5ffb4));
path.cubicTo(SkBits2Float(0x3ea463a8), SkBits2Float(0xc2a5ffae), SkBits2Float(0x3ea9b10b), SkBits2Float(0xc2a5ffa8), SkBits2Float(0x3eaefe6d), SkBits2Float(0xc2a5ffa3));
path.lineTo(SkBits2Float(0x3e7d0144), SkBits2Float(0xc26fff7b));
path.cubicTo(SkBits2Float(0x3e75568f), SkBits2Float(0xc26fff84), SkBits2Float(0x3e6dac12), SkBits2Float(0xc26fff8c), SkBits2Float(0x3e66012b), SkBits2Float(0xc26fff92));
path.cubicTo(SkBits2Float(0x3e1955e2), SkBits2Float(0xc26fffdc), SkBits2Float(0x3d99560b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3eaefebc), SkBits2Float(0xc2a5ffa4));
path.cubicTo(SkBits2Float(0x3f0276b7), SkBits2Float(0xc2a5ff4a), SkBits2Float(0x3f2d6dea), SkBits2Float(0xc2a5feac), SkBits2Float(0x3f5864cc), SkBits2Float(0xc2a5fdcd));
path.lineTo(SkBits2Float(0x3f1c6df6), SkBits2Float(0xc26ffcd0));
path.cubicTo(SkBits2Float(0x3efabdec), SkBits2Float(0xc26ffe15), SkBits2Float(0x3ebc9f78), SkBits2Float(0xc26ffef9), SkBits2Float(0x3e7d0190), SkBits2Float(0xc26fff7c));
path.lineTo(SkBits2Float(0x3eaefebc), SkBits2Float(0xc2a5ffa4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp91(skiatest::Reporter* reporter, const char* filename) {  // crashed
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ec1e1ad), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f41e136), SkBits2Float(0xc2a5feac), SkBits2Float(0x3f9167c6), SkBits2Float(0xc2a5fc05));
path.lineTo(SkBits2Float(0x3f523979), SkBits2Float(0xc26ffa3f));
path.cubicTo(SkBits2Float(0x3f0c2737), SkBits2Float(0xc26ffe17), SkBits2Float(0x3e8c2756), SkBits2Float(0xc2700000), SkBits2Float(0xb5b74260), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f9167c1), SkBits2Float(0xc2a5fc05));
path.cubicTo(SkBits2Float(0x3f96406f), SkBits2Float(0xc2a5fbc1), SkBits2Float(0x3f9b1917), SkBits2Float(0xc2a5fb79), SkBits2Float(0x3f9ff1bc), SkBits2Float(0xc2a5fb2f));
path.lineTo(SkBits2Float(0x3f673ed7), SkBits2Float(0xc26ff909));
path.cubicTo(SkBits2Float(0x3f603cf4), SkBits2Float(0xc26ff977), SkBits2Float(0x3f593b3c), SkBits2Float(0xc26ff9dd), SkBits2Float(0x3f52397f), SkBits2Float(0xc26ffa3f));
path.lineTo(SkBits2Float(0x3f9167c1), SkBits2Float(0xc2a5fc05));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp92(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e2c5962), SkBits2Float(0xc2a60000), SkBits2Float(0x3eac58ef), SkBits2Float(0xc2a5ffbd), SkBits2Float(0x3f014269), SkBits2Float(0xc2a5ff37));
path.lineTo(SkBits2Float(0x3ebae1ca), SkBits2Float(0xc26ffedd));
path.cubicTo(SkBits2Float(0x3e792d51), SkBits2Float(0xc26fff9f), SkBits2Float(0x3df92dfa), SkBits2Float(0xc2700000), SkBits2Float(0x36163ed0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f014292), SkBits2Float(0xc2a5ff37));
path.cubicTo(SkBits2Float(0x3f0591a2), SkBits2Float(0xc2a5ff28), SkBits2Float(0x3f09e09b), SkBits2Float(0xc2a5ff1a), SkBits2Float(0x3f0e2f92), SkBits2Float(0xc2a5ff0b));
path.lineTo(SkBits2Float(0x3ecd91e5), SkBits2Float(0xc26ffea0));
path.cubicTo(SkBits2Float(0x3ec75718), SkBits2Float(0xc26ffeb6), SkBits2Float(0x3ec11c70), SkBits2Float(0xc26ffeca), SkBits2Float(0x3ebae1c7), SkBits2Float(0xc26ffedd));
path.lineTo(SkBits2Float(0x3f014292), SkBits2Float(0xc2a5ff37));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp93(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36163ed0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.quadTo(SkBits2Float(0x3e81430a), SkBits2Float(0xc2a60000), SkBits2Float(0x3f014292), SkBits2Float(0xc2a5ff37));
path.cubicTo(SkBits2Float(0x3f0591a2), SkBits2Float(0xc2a5ff28), SkBits2Float(0x3f09e09b), SkBits2Float(0xc2a5ff1a), SkBits2Float(0x3f0e2f92), SkBits2Float(0xc2a5ff0b));
path.lineTo(SkBits2Float(0x3ecd91e5), SkBits2Float(0xc26ffea0));
path.cubicTo(SkBits2Float(0x3ec75719), SkBits2Float(0xc26ffeb6), SkBits2Float(0x3ec11c72), SkBits2Float(0xc26ffeca), SkBits2Float(0x3ebae1ca), SkBits2Float(0xc26ffedd));
path.quadTo(SkBits2Float(0x3e3ae230), SkBits2Float(0xc2700000), SkBits2Float(0x36163ed0), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f0e2f94), SkBits2Float(0xc2a5ff0c));
path.cubicTo(SkBits2Float(0x3f5401b9), SkBits2Float(0xc2a5fe1c), SkBits2Float(0x3f8ce9a3), SkBits2Float(0xc2a5fc7d), SkBits2Float(0x3fafd1bd), SkBits2Float(0xc2a5fa2d));
path.lineTo(SkBits2Float(0x3f7e3238), SkBits2Float(0xc26ff796));
path.cubicTo(SkBits2Float(0x3f4bbaca), SkBits2Float(0xc26ffaee), SkBits2Float(0x3f194226), SkBits2Float(0xc26ffd46), SkBits2Float(0x3ecd9202), SkBits2Float(0xc26ffea0));
path.lineTo(SkBits2Float(0x3f0e2f94), SkBits2Float(0xc2a5ff0c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp94(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f167e4a), SkBits2Float(0xc2a60000), SkBits2Float(0x3f967d97), SkBits2Float(0xc2a5fcce), SkBits2Float(0x3fe1b83b), SkBits2Float(0xc2a5f668));
path.lineTo(SkBits2Float(0x3fa32ba2), SkBits2Float(0xc26ff222));
path.cubicTo(SkBits2Float(0x3f599370), SkBits2Float(0xc26ffb61), SkBits2Float(0x3ed9943c), SkBits2Float(0xc2700000), SkBits2Float(0x3437e940), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fe1b817), SkBits2Float(0xc2a5f668));
path.cubicTo(SkBits2Float(0x3fe93dd6), SkBits2Float(0xc2a5f5c4), SkBits2Float(0x3ff0c3a7), SkBits2Float(0xc2a5f518), SkBits2Float(0x3ff8496b), SkBits2Float(0xc2a5f464));
path.lineTo(SkBits2Float(0x3fb37c11), SkBits2Float(0xc26fef38));
path.cubicTo(SkBits2Float(0x3fae0bf9), SkBits2Float(0xc26ff03c), SkBits2Float(0x3fa89bd2), SkBits2Float(0xc26ff134), SkBits2Float(0x3fa32ba2), SkBits2Float(0xc26ff222));
path.lineTo(SkBits2Float(0x3fe1b817), SkBits2Float(0xc2a5f668));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp95(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f167e32), SkBits2Float(0xc2a60000), SkBits2Float(0x3f967d7f), SkBits2Float(0xc2a5fcce), SkBits2Float(0x3fe1b817), SkBits2Float(0xc2a5f668));

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ff8497f), SkBits2Float(0xc2a5f465));
path.cubicTo(SkBits2Float(0x40391895), SkBits2Float(0xc2a5e8fe), SkBits2Float(0x407604f1), SkBits2Float(0xc2a5d533), SkBits2Float(0x40997177), SkBits2Float(0xc2a5b905));
path.lineTo(SkBits2Float(0x405dd87f), SkBits2Float(0xc26f9962));
path.cubicTo(SkBits2Float(0x4031d867), SkBits2Float(0xc26fc221), SkBits2Float(0x4005cdec), SkBits2Float(0xc26fdebf), SkBits2Float(0x3fb37c22), SkBits2Float(0xc26fef39));
path.lineTo(SkBits2Float(0x3ff8497f), SkBits2Float(0xc2a5f465));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp96(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3fa966bb), SkBits2Float(0xc2a5ffff), SkBits2Float(0x402963a4), SkBits2Float(0xc2a5efcb), SkBits2Float(0x407dfe39), SkBits2Float(0xc2a5cf64));
path.lineTo(SkBits2Float(0x40379c05), SkBits2Float(0xc26fb9ba));
path.cubicTo(SkBits2Float(0x3ff4e689), SkBits2Float(0xc26fe893), SkBits2Float(0x3f74eb1f), SkBits2Float(0xc2700000), SkBits2Float(0x363f7e94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x407dfe3a), SkBits2Float(0xc2a5cf65));
path.cubicTo(SkBits2Float(0x40833a01), SkBits2Float(0xc2a5cc27), SkBits2Float(0x408774bf), SkBits2Float(0xc2a5c8c0), SkBits2Float(0x408baf5a), SkBits2Float(0xc2a5c52f));
path.lineTo(SkBits2Float(0x4049f448), SkBits2Float(0xc26faaf9));
path.cubicTo(SkBits2Float(0x4043d713), SkBits2Float(0xc26fb022), SkBits2Float(0x403db99f), SkBits2Float(0xc26fb50d), SkBits2Float(0x40379bfe), SkBits2Float(0xc26fb9bc));
path.lineTo(SkBits2Float(0x407dfe3a), SkBits2Float(0xc2a5cf65));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp97(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x363f7e94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3fa966bb), SkBits2Float(0xc2a5ffff), SkBits2Float(0x402963a4), SkBits2Float(0xc2a5efcb), SkBits2Float(0x407dfe3a), SkBits2Float(0xc2a5cf65));
path.cubicTo(SkBits2Float(0x40833a01), SkBits2Float(0xc2a5cc27), SkBits2Float(0x408774bf), SkBits2Float(0xc2a5c8c0), SkBits2Float(0x408baf5a), SkBits2Float(0xc2a5c52f));
path.lineTo(SkBits2Float(0x4049f448), SkBits2Float(0xc26faaf9));
path.cubicTo(SkBits2Float(0x4043d716), SkBits2Float(0xc26fb022), SkBits2Float(0x403db9a5), SkBits2Float(0xc26fb50d), SkBits2Float(0x40379c07), SkBits2Float(0xc26fb9bc));
path.lineTo(SkBits2Float(0x40379c05), SkBits2Float(0xc26fb9ba));
path.cubicTo(SkBits2Float(0x3ff4e689), SkBits2Float(0xc26fe893), SkBits2Float(0x3f74eb1f), SkBits2Float(0xc2700000), SkBits2Float(0x363f7e94), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x408baf5c), SkBits2Float(0xc2a5c530));
path.cubicTo(SkBits2Float(0x40d03963), SkBits2Float(0xc2a58b6e), SkBits2Float(0x410a4c7d), SkBits2Float(0xc2a52732), SkBits2Float(0x412c535f), SkBits2Float(0xc2a498b2));
path.lineTo(SkBits2Float(0x40f9253d), SkBits2Float(0xc26df886));
path.cubicTo(SkBits2Float(0x40c7f32d), SkBits2Float(0xc26ec68d), SkBits2Float(0x409685fb), SkBits2Float(0xc26f577a), SkBits2Float(0x4049f441), SkBits2Float(0xc26faafa));
path.lineTo(SkBits2Float(0x408baf5c), SkBits2Float(0xc2a5c530));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp98(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40155bee), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40955364), SkBits2Float(0xc2a5cd99), SkBits2Float(0x40dfbd5f), SkBits2Float(0xc2a568f2));
path.lineTo(SkBits2Float(0x40a1bd53), SkBits2Float(0xc26f259d));
path.cubicTo(SkBits2Float(0x4057e483), SkBits2Float(0xc26fb724), SkBits2Float(0x3fd7f0d9), SkBits2Float(0xc2700000), SkBits2Float(0x3619fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40dfbd5e), SkBits2Float(0xc2a568f3));
path.cubicTo(SkBits2Float(0x40e72e1b), SkBits2Float(0xc2a55ee2), SkBits2Float(0x40ee9e1c), SkBits2Float(0xc2a55452), SkBits2Float(0x40f60d62), SkBits2Float(0xc2a54941));
path.lineTo(SkBits2Float(0x40b1de84), SkBits2Float(0xc26ef7c9));
path.cubicTo(SkBits2Float(0x40ac7ea0), SkBits2Float(0xc26f07cb), SkBits2Float(0x40a71e37), SkBits2Float(0xc26f1712), SkBits2Float(0x40a1bd4f), SkBits2Float(0xc26f259f));
path.lineTo(SkBits2Float(0x40dfbd5e), SkBits2Float(0xc2a568f3));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp99(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3619fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40155bee), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40955364), SkBits2Float(0xc2a5cd99), SkBits2Float(0x40dfbd5e), SkBits2Float(0xc2a568f3));
path.cubicTo(SkBits2Float(0x40e72e1b), SkBits2Float(0xc2a55ee2), SkBits2Float(0x40ee9e1c), SkBits2Float(0xc2a55452), SkBits2Float(0x40f60d62), SkBits2Float(0xc2a54941));
path.lineTo(SkBits2Float(0x40b1de84), SkBits2Float(0xc26ef7c9));
path.cubicTo(SkBits2Float(0x40ac7ea2), SkBits2Float(0xc26f07cb), SkBits2Float(0x40a71e3a), SkBits2Float(0xc26f1712), SkBits2Float(0x40a1bd54), SkBits2Float(0xc26f259f));
path.lineTo(SkBits2Float(0x40a1bd53), SkBits2Float(0xc26f259d));
path.cubicTo(SkBits2Float(0x4057e483), SkBits2Float(0xc26fb724), SkBits2Float(0x3fd7f0d9), SkBits2Float(0xc2700000), SkBits2Float(0x3619fea3), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40f60d69), SkBits2Float(0xc2a54941));
path.cubicTo(SkBits2Float(0x41374a21), SkBits2Float(0xc2a495d5), SkBits2Float(0x41731962), SkBits2Float(0xc2a35eca), SkBits2Float(0x419704b1), SkBits2Float(0xc2a1a64c));
path.lineTo(SkBits2Float(0x415a56f5), SkBits2Float(0xc269b5d4));
path.cubicTo(SkBits2Float(0x412fbbfb), SkBits2Float(0xc26c32af), SkBits2Float(0x41047f9a), SkBits2Float(0xc26df463), SkBits2Float(0x40b1de7e), SkBits2Float(0xc26ef7cb));
path.lineTo(SkBits2Float(0x40f60d69), SkBits2Float(0xc2a54941));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp100(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x403cde0b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40bcccc9), SkBits2Float(0xc2a5af6a), SkBits2Float(0x410d5936), SkBits2Float(0xc2a50e98));
path.lineTo(SkBits2Float(0x40cc5bf6), SkBits2Float(0xc26ea2fc));
path.cubicTo(SkBits2Float(0x40887b5e), SkBits2Float(0xc26f8b7f), SkBits2Float(0x400887d8), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x410d5935), SkBits2Float(0xc2a50e99));
path.cubicTo(SkBits2Float(0x41120ace), SkBits2Float(0xc2a4fe85), SkBits2Float(0x4116bbb5), SkBits2Float(0xc2a4eda4), SkBits2Float(0x411b6bdd), SkBits2Float(0xc2a4dbf6));
path.lineTo(SkBits2Float(0x40e0b4a3), SkBits2Float(0xc26e59c7));
path.cubicTo(SkBits2Float(0x40d9ed7a), SkBits2Float(0xc26e7357), SkBits2Float(0x40d32536), SkBits2Float(0xc26e8bbe), SkBits2Float(0x40cc5bf1), SkBits2Float(0xc26ea2fc));
path.lineTo(SkBits2Float(0x410d5935), SkBits2Float(0xc2a50e99));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end fail 1

static void battleOp101(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x406db78d), SkBits2Float(0xc2a60000), SkBits2Float(0x40ed953d), SkBits2Float(0xc2a58058), SkBits2Float(0x4131afb7), SkBits2Float(0xc2a481e4));
path.lineTo(SkBits2Float(0x410072b2), SkBits2Float(0xc26dd78e));
path.cubicTo(SkBits2Float(0x40abbf2e), SkBits2Float(0xc26f4770), SkBits2Float(0x402bd807), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4131afba), SkBits2Float(0xc2a481e4));
path.cubicTo(SkBits2Float(0x413792dd), SkBits2Float(0xc2a46874), SkBits2Float(0x413d74a2), SkBits2Float(0xc2a44dc1), SkBits2Float(0x414354e9), SkBits2Float(0xc2a431ca));
path.lineTo(SkBits2Float(0x410d3424), SkBits2Float(0xc26d63c0));
path.cubicTo(SkBits2Float(0x4108f4b6), SkBits2Float(0xc26d8c2e), SkBits2Float(0x4104b435), SkBits2Float(0xc26db2c8), SkBits2Float(0x410072b4), SkBits2Float(0xc26dd78e));
path.lineTo(SkBits2Float(0x4131afba), SkBits2Float(0xc2a481e4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp102(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x406db78d), SkBits2Float(0xc2a60000), SkBits2Float(0x40ed953d), SkBits2Float(0xc2a58058), SkBits2Float(0x4131afba), SkBits2Float(0xc2a481e4));
path.cubicTo(SkBits2Float(0x413792dd), SkBits2Float(0xc2a46874), SkBits2Float(0x413d74a2), SkBits2Float(0xc2a44dc1), SkBits2Float(0x414354e9), SkBits2Float(0xc2a431ca));
path.lineTo(SkBits2Float(0x410d3424), SkBits2Float(0xc26d63c0));
path.cubicTo(SkBits2Float(0x4108f4b6), SkBits2Float(0xc26d8c2e), SkBits2Float(0x4104b435), SkBits2Float(0xc26db2c8), SkBits2Float(0x410072b2), SkBits2Float(0xc26dd78e));
path.cubicTo(SkBits2Float(0x40abbf2e), SkBits2Float(0xc26f4770), SkBits2Float(0x402bd807), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x414354ed), SkBits2Float(0xc2a431cb));
path.cubicTo(SkBits2Float(0x419152e5), SkBits2Float(0xc2a26c3a), SkBits2Float(0x41c0119b), SkBits2Float(0xc29f5c06), SkBits2Float(0x41ed1335), SkBits2Float(0xc29b0f0a));
path.lineTo(SkBits2Float(0x41ab612b), SkBits2Float(0xc2602e6b));
path.cubicTo(SkBits2Float(0x418ad84d), SkBits2Float(0xc2666635), SkBits2Float(0x41521b54), SkBits2Float(0xc26ad3fe), SkBits2Float(0x410d3426), SkBits2Float(0xc26d63c0));
path.lineTo(SkBits2Float(0x414354ed), SkBits2Float(0xc2a431cb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp103(skiatest::Reporter* reporter, const char* filename) {  //crash
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x408e2d73), SkBits2Float(0xc2a5ffff), SkBits2Float(0x410e100a), SkBits2Float(0xc2a54957), SkBits2Float(0x41543cd2), SkBits2Float(0xc2a3ddc8));
path.lineTo(SkBits2Float(0x41196cba), SkBits2Float(0xc26cea49));
path.cubicTo(SkBits2Float(0x40cd643f), SkBits2Float(0xc26ef7e9), SkBits2Float(0x404d8eb8), SkBits2Float(0xc26fffff), SkBits2Float(0xb5ac02ba), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41543cce), SkBits2Float(0xc2a3ddc8));
path.cubicTo(SkBits2Float(0x415b4057), SkBits2Float(0xc2a3b973), SkBits2Float(0x41624181), SkBits2Float(0xc2a39350), SkBits2Float(0x41694022), SkBits2Float(0xc2a36b60));
path.lineTo(SkBits2Float(0x41289d63), SkBits2Float(0xc26c44e1));
path.cubicTo(SkBits2Float(0x41238ef8), SkBits2Float(0xc26c7e9e), SkBits2Float(0x411e7eb5), SkBits2Float(0xc26cb5c1), SkBits2Float(0x41196cbd), SkBits2Float(0xc26cea4a));
path.lineTo(SkBits2Float(0x41543cce), SkBits2Float(0xc2a3ddc8));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp104(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3dd41fb8), SkBits2Float(0xc2a5fffe), SkBits2Float(0x3e541e5b), SkBits2Float(0xc2a5ffe5), SkBits2Float(0x3e9f1657), SkBits2Float(0xc2a5ffb2));
path.lineTo(SkBits2Float(0x3e66012b), SkBits2Float(0xc26fff92));
path.cubicTo(SkBits2Float(0x3e1955e2), SkBits2Float(0xc26fffdc), SkBits2Float(0x3d99560b), SkBits2Float(0xc2700000), SkBits2Float(0x350f7780), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3e9f1626), SkBits2Float(0xc2a5ffb4));
path.cubicTo(SkBits2Float(0x3ea463a8), SkBits2Float(0xc2a5ffae), SkBits2Float(0x3ea9b10b), SkBits2Float(0xc2a5ffa8), SkBits2Float(0x3eaefe6d), SkBits2Float(0xc2a5ffa3));
path.lineTo(SkBits2Float(0x3e7d0144), SkBits2Float(0xc26fff7b));
path.cubicTo(SkBits2Float(0x3e75568f), SkBits2Float(0xc26fff84), SkBits2Float(0x3e6dac12), SkBits2Float(0xc26fff8c), SkBits2Float(0x3e660197), SkBits2Float(0xc26fff93));
path.lineTo(SkBits2Float(0x3e9f1626), SkBits2Float(0xc2a5ffb4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp105(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3dd41f74), SkBits2Float(0xc2a5fffe), SkBits2Float(0x3e541e17), SkBits2Float(0xc2a5ffe5), SkBits2Float(0x3e9f1624), SkBits2Float(0xc2a5ffb2));
path.lineTo(SkBits2Float(0x3e9f1626), SkBits2Float(0xc2a5ffb4));
path.cubicTo(SkBits2Float(0x3ea463a8), SkBits2Float(0xc2a5ffae), SkBits2Float(0x3ea9b10b), SkBits2Float(0xc2a5ffa8), SkBits2Float(0x3eaefe6d), SkBits2Float(0xc2a5ffa3));
path.lineTo(SkBits2Float(0x3e7d0144), SkBits2Float(0xc26fff7b));
path.cubicTo(SkBits2Float(0x3e75568f), SkBits2Float(0xc26fff84), SkBits2Float(0x3e6dac12), SkBits2Float(0xc26fff8c), SkBits2Float(0x3e66012b), SkBits2Float(0xc26fff92));
path.cubicTo(SkBits2Float(0x3e1955e2), SkBits2Float(0xc26fffdc), SkBits2Float(0x3d99560b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3eaefebc), SkBits2Float(0xc2a5ffa4));
path.cubicTo(SkBits2Float(0x3f0276b7), SkBits2Float(0xc2a5ff4a), SkBits2Float(0x3f2d6dea), SkBits2Float(0xc2a5feac), SkBits2Float(0x3f5864cc), SkBits2Float(0xc2a5fdcd));
path.lineTo(SkBits2Float(0x3f1c6df6), SkBits2Float(0xc26ffcd0));
path.cubicTo(SkBits2Float(0x3efabdec), SkBits2Float(0xc26ffe15), SkBits2Float(0x3ebc9f78), SkBits2Float(0xc26ffef9), SkBits2Float(0x3e7d0190), SkBits2Float(0xc26fff7c));
path.lineTo(SkBits2Float(0x3eaefebc), SkBits2Float(0xc2a5ffa4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp106(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ee221f0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f622166), SkBits2Float(0xc2a5fe31), SkBits2Float(0x3fa9974d), SkBits2Float(0xc2a5fa95));
path.lineTo(SkBits2Float(0x3f753159), SkBits2Float(0xc26ff82c));
path.cubicTo(SkBits2Float(0x3f237814), SkBits2Float(0xc26ffd64), SkBits2Float(0x3ea3787a), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa50), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fa99777), SkBits2Float(0xc2a5fa96));
path.cubicTo(SkBits2Float(0x3faf3e7a), SkBits2Float(0xc2a5fa39), SkBits2Float(0x3fb4e596), SkBits2Float(0xc2a5f9d8), SkBits2Float(0x3fba8cad), SkBits2Float(0xc2a5f972));
path.lineTo(SkBits2Float(0x3f86dad5), SkBits2Float(0xc26ff687));
path.cubicTo(SkBits2Float(0x3f82c4d9), SkBits2Float(0xc26ff71a), SkBits2Float(0x3f7d5da4), SkBits2Float(0xc26ff7a6), SkBits2Float(0x3f753191), SkBits2Float(0xc26ff82c));
path.lineTo(SkBits2Float(0x3fa99777), SkBits2Float(0xc2a5fa96));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp107(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ee221f0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f622166), SkBits2Float(0xc2a5fe31), SkBits2Float(0x3fa99777), SkBits2Float(0xc2a5fa96));
path.cubicTo(SkBits2Float(0x3faf3e7a), SkBits2Float(0xc2a5fa39), SkBits2Float(0x3fb4e596), SkBits2Float(0xc2a5f9d8), SkBits2Float(0x3fba8cad), SkBits2Float(0xc2a5f972));
path.lineTo(SkBits2Float(0x3f86dad5), SkBits2Float(0xc26ff687));
path.cubicTo(SkBits2Float(0x3f82c4d9), SkBits2Float(0xc26ff71a), SkBits2Float(0x3f7d5da4), SkBits2Float(0xc26ff7a6), SkBits2Float(0x3f753159), SkBits2Float(0xc26ff82c));
path.cubicTo(SkBits2Float(0x3f237814), SkBits2Float(0xc26ffd64), SkBits2Float(0x3ea3787a), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fba8c96), SkBits2Float(0xc2a5f973));
path.cubicTo(SkBits2Float(0x400b1301), SkBits2Float(0xc2a5f303), SkBits2Float(0x4038dc7e), SkBits2Float(0xc2a5e7d6), SkBits2Float(0x40669fe4), SkBits2Float(0xc2a5d7ed));
path.lineTo(SkBits2Float(0x4026b765), SkBits2Float(0xc26fc611));
path.cubicTo(SkBits2Float(0x4005a27d), SkBits2Float(0xc26fdd13), SkBits2Float(0x3fc9123c), SkBits2Float(0xc26fed3b), SkBits2Float(0x3f86daf1), SkBits2Float(0xc26ff689));
path.lineTo(SkBits2Float(0x3fba8c96), SkBits2Float(0xc2a5f973));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp108(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f587304), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3fd8713e), SkBits2Float(0xc2a5f962), SkBits2Float(0x40224ed5), SkBits2Float(0xc2a5ec27));
path.lineTo(SkBits2Float(0x3feaa996), SkBits2Float(0xc26fe350));
path.cubicTo(SkBits2Float(0x3f9c76e4), SkBits2Float(0xc26ff671), SkBits2Float(0x3f1c780b), SkBits2Float(0xc2700000), SkBits2Float(0xb5510538), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40224ee4), SkBits2Float(0xc2a5ec28));
path.cubicTo(SkBits2Float(0x4027b77a), SkBits2Float(0xc2a5ead6), SkBits2Float(0x402d1ffd), SkBits2Float(0xc2a5e972), SkBits2Float(0x4032886f), SkBits2Float(0xc2a5e7fe));
path.lineTo(SkBits2Float(0x40010f64), SkBits2Float(0xc26fdd4a));
path.cubicTo(SkBits2Float(0x3ffa4d23), SkBits2Float(0xc26fdf64), SkBits2Float(0x3ff27b6d), SkBits2Float(0xc26fe166), SkBits2Float(0x3feaa9a1), SkBits2Float(0xc26fe350));
path.lineTo(SkBits2Float(0x40224ee4), SkBits2Float(0xc2a5ec28));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp109(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f587304), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3fd8713e), SkBits2Float(0xc2a5f962), SkBits2Float(0x40224ee4), SkBits2Float(0xc2a5ec28));
path.cubicTo(SkBits2Float(0x4027b77a), SkBits2Float(0xc2a5ead6), SkBits2Float(0x402d1ffd), SkBits2Float(0xc2a5e972), SkBits2Float(0x4032886f), SkBits2Float(0xc2a5e7fe));
path.lineTo(SkBits2Float(0x40010f64), SkBits2Float(0xc26fdd4a));
path.cubicTo(SkBits2Float(0x3ffa4d23), SkBits2Float(0xc26fdf64), SkBits2Float(0x3ff27b6d), SkBits2Float(0xc26fe166), SkBits2Float(0x3feaa996), SkBits2Float(0xc26fe350));
path.cubicTo(SkBits2Float(0x3f9c76e4), SkBits2Float(0xc26ff671), SkBits2Float(0x3f1c780b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4032887d), SkBits2Float(0xc2a5e7fe));
path.cubicTo(SkBits2Float(0x4085166b), SkBits2Float(0xc2a5d069), SkBits2Float(0x40b0dd8e), SkBits2Float(0xc2a5a77a), SkBits2Float(0x40dc8f53), SkBits2Float(0xc2a56d38));
path.lineTo(SkBits2Float(0x409f70d9), SkBits2Float(0xc26f2bca));
path.cubicTo(SkBits2Float(0x407fb58c), SkBits2Float(0xc26f8005), SkBits2Float(0x40406a74), SkBits2Float(0xc26fbb35), SkBits2Float(0x40010f5f), SkBits2Float(0xc26fdd4b));
path.lineTo(SkBits2Float(0x4032887d), SkBits2Float(0xc2a5e7fe));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp110(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x400cf1ae), SkBits2Float(0xc2a5ffff), SkBits2Float(0x408cea87), SkBits2Float(0xc2a5d31f), SkBits2Float(0x40d32a40), SkBits2Float(0xc2a57979));
path.lineTo(SkBits2Float(0x4098a645), SkBits2Float(0xc26f3d83));
path.cubicTo(SkBits2Float(0x404bbc01), SkBits2Float(0xc26fbf1e), SkBits2Float(0x3fcbc669), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff59), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40d32a46), SkBits2Float(0xc2a5797a));
path.cubicTo(SkBits2Float(0x40da306e), SkBits2Float(0xc2a57083), SkBits2Float(0x40e135fe), SkBits2Float(0xc2a5671a), SkBits2Float(0x40e83aef), SkBits2Float(0xc2a55d3f));
path.lineTo(SkBits2Float(0x40a7e090), SkBits2Float(0xc26f14b1));
path.cubicTo(SkBits2Float(0x40a2cd8d), SkBits2Float(0xc26f22f4), SkBits2Float(0x409dba1d), SkBits2Float(0xc26f308e), SkBits2Float(0x4098a641), SkBits2Float(0xc26f3d84));
path.lineTo(SkBits2Float(0x40d32a46), SkBits2Float(0xc2a5797a));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp111(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff59), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x400cf1ae), SkBits2Float(0xc2a5ffff), SkBits2Float(0x408cea87), SkBits2Float(0xc2a5d31f), SkBits2Float(0x40d32a46), SkBits2Float(0xc2a5797a));
path.cubicTo(SkBits2Float(0x40da306e), SkBits2Float(0xc2a57083), SkBits2Float(0x40e135fe), SkBits2Float(0xc2a5671a), SkBits2Float(0x40e83aef), SkBits2Float(0xc2a55d3f));
path.lineTo(SkBits2Float(0x40a7e090), SkBits2Float(0xc26f14b1));
path.cubicTo(SkBits2Float(0x40a2cd8f), SkBits2Float(0xc26f22f4), SkBits2Float(0x409dba20), SkBits2Float(0xc26f308e), SkBits2Float(0x4098a645), SkBits2Float(0xc26f3d83));
path.cubicTo(SkBits2Float(0x404bbc01), SkBits2Float(0xc26fbf1e), SkBits2Float(0x3fcbc669), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff59), SkBits2Float(0xc26fffff));
path.close();
path.moveTo(SkBits2Float(0x40b5a39a), SkBits2Float(0xc28e5650));
path.lineTo(SkBits2Float(0x4098a641), SkBits2Float(0xc26f3d84));
path.lineTo(SkBits2Float(0x4098a646), SkBits2Float(0xc26f3d84));
path.lineTo(SkBits2Float(0x40b5a39a), SkBits2Float(0xc28e5650));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40e83ae9), SkBits2Float(0xc2a55d3f));
path.cubicTo(SkBits2Float(0x412d0232), SkBits2Float(0xc2a4bd73), SkBits2Float(0x4165854a), SkBits2Float(0xc2a3a860), SkBits2Float(0x418ea651), SkBits2Float(0xc2a21fbf));
path.lineTo(SkBits2Float(0x414e3d91), SkBits2Float(0xc26a656a));
path.cubicTo(SkBits2Float(0x4125eb27), SkBits2Float(0xc26c9d13), SkBits2Float(0x40fa2207), SkBits2Float(0xc26e2daa), SkBits2Float(0x40a7e094), SkBits2Float(0xc26f14b2));
path.lineTo(SkBits2Float(0x40e83ae9), SkBits2Float(0xc2a55d3f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp112(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4035711d), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40b561d9), SkBits2Float(0xc2a5b5a1), SkBits2Float(0x4107d050), SkBits2Float(0xc2a5212f));
path.lineTo(SkBits2Float(0x40c45b76), SkBits2Float(0xc26ebddb));
path.cubicTo(SkBits2Float(0x40831ea4), SkBits2Float(0xc26f947a), SkBits2Float(0x400329ad), SkBits2Float(0xc26fffff), SkBits2Float(0x35bbfd46), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4107d054), SkBits2Float(0xc2a5212f));
path.cubicTo(SkBits2Float(0x410c5332), SkBits2Float(0xc2a51258), SkBits2Float(0x4110d578), SkBits2Float(0xc2a502c3), SkBits2Float(0x41155714), SkBits2Float(0xc2a4f271));
path.lineTo(SkBits2Float(0x40d7e9e2), SkBits2Float(0xc26e7a46));
path.cubicTo(SkBits2Float(0x40d16605), SkBits2Float(0xc26e91e0), SkBits2Float(0x40cae131), SkBits2Float(0xc26ea866), SkBits2Float(0x40c45b7a), SkBits2Float(0xc26ebddc));
path.lineTo(SkBits2Float(0x4107d054), SkBits2Float(0xc2a5212f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp113(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4035711d), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40b561d9), SkBits2Float(0xc2a5b5a1), SkBits2Float(0x4107d054), SkBits2Float(0xc2a5212f));
path.cubicTo(SkBits2Float(0x410c5332), SkBits2Float(0xc2a51258), SkBits2Float(0x4110d578), SkBits2Float(0xc2a502c3), SkBits2Float(0x41155714), SkBits2Float(0xc2a4f271));
path.lineTo(SkBits2Float(0x40d7e9e2), SkBits2Float(0xc26e7a46));
path.cubicTo(SkBits2Float(0x40d16605), SkBits2Float(0xc26e91e0), SkBits2Float(0x40cae131), SkBits2Float(0xc26ea866), SkBits2Float(0x40c45b76), SkBits2Float(0xc26ebddb));
path.cubicTo(SkBits2Float(0x40831ea4), SkBits2Float(0xc26f947a), SkBits2Float(0x400329ad), SkBits2Float(0xc26fffff), SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4115571a), SkBits2Float(0xc2a4f271));
path.cubicTo(SkBits2Float(0x415e6818), SkBits2Float(0xc2a3e9d4), SkBits2Float(0x41935478), SkBits2Float(0xc2a21f7a), SkBits2Float(0x41b6ad74), SkBits2Float(0xc29f981d));
path.lineTo(SkBits2Float(0x41840e5b), SkBits2Float(0xc266bd14));
path.cubicTo(SkBits2Float(0x415501d6), SkBits2Float(0xc26a6507), SkBits2Float(0x4120c6a0), SkBits2Float(0xc26cfbb4), SkBits2Float(0x40d7e9e6), SkBits2Float(0xc26e7a47));
path.lineTo(SkBits2Float(0x4115571a), SkBits2Float(0xc2a4f271));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp114(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x405f6414), SkBits2Float(0xc2a60000), SkBits2Float(0x40df4798), SkBits2Float(0xc2a58f44), SkBits2Float(0x41270b42), SkBits2Float(0xc2a4ae78));
path.lineTo(SkBits2Float(0x40f1826b), SkBits2Float(0xc26e1801));
path.cubicTo(SkBits2Float(0x40a16831), SkBits2Float(0xc26f5d03), SkBits2Float(0x40217cc8), SkBits2Float(0xc2700000), SkBits2Float(0x3507fa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41270b46), SkBits2Float(0xc2a4ae78));
path.cubicTo(SkBits2Float(0x412c952a), SkBits2Float(0xc2a497ff), SkBits2Float(0x41321de3), SkBits2Float(0xc2a48068), SkBits2Float(0x4137a563), SkBits2Float(0xc2a467b4));
path.lineTo(SkBits2Float(0x4104c195), SkBits2Float(0xc26db1b1));
path.cubicTo(SkBits2Float(0x4100c256), SkBits2Float(0xc26dd569), SkBits2Float(0x40f98465), SkBits2Float(0xc26df784), SkBits2Float(0x40f18273), SkBits2Float(0xc26e1801));
path.lineTo(SkBits2Float(0x41270b46), SkBits2Float(0xc2a4ae78));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp115(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x405f6414), SkBits2Float(0xc2a60000), SkBits2Float(0x40df4798), SkBits2Float(0xc2a58f44), SkBits2Float(0x41270b46), SkBits2Float(0xc2a4ae78));
path.cubicTo(SkBits2Float(0x412c952a), SkBits2Float(0xc2a497ff), SkBits2Float(0x41321de3), SkBits2Float(0xc2a48068), SkBits2Float(0x4137a563), SkBits2Float(0xc2a467b4));
path.lineTo(SkBits2Float(0x4104c195), SkBits2Float(0xc26db1b1));
path.cubicTo(SkBits2Float(0x4100c256), SkBits2Float(0xc26dd569), SkBits2Float(0x40f98465), SkBits2Float(0xc26df784), SkBits2Float(0x40f1826b), SkBits2Float(0xc26e1801));
path.cubicTo(SkBits2Float(0x40a16831), SkBits2Float(0xc26f5d03), SkBits2Float(0x40217cc8), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4137a563), SkBits2Float(0xc2a467b4));
path.cubicTo(SkBits2Float(0x4188a9bf), SkBits2Float(0xc2a2d700), SkBits2Float(0x41b4bec4), SkBits2Float(0xc2a021d5), SkBits2Float(0x41df619b), SkBits2Float(0xc29c5308));
path.lineTo(SkBits2Float(0x41a17afe), SkBits2Float(0xc26202d7));
path.cubicTo(SkBits2Float(0x4182a8c1), SkBits2Float(0xc2678433), SkBits2Float(0x414595cf), SkBits2Float(0xc26b6e5e), SkBits2Float(0x4104c197), SkBits2Float(0xc26db1b2));
path.lineTo(SkBits2Float(0x4137a563), SkBits2Float(0xc2a467b4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp116(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40894a00), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41092f84), SkBits2Float(0xc2a555af), SkBits2Float(0x414d01d5), SkBits2Float(0xc2a40295));
path.lineTo(SkBits2Float(0x411432a9), SkBits2Float(0xc26d1f80));
path.cubicTo(SkBits2Float(0x40c65728), SkBits2Float(0xc26f09c3), SkBits2Float(0x40467d64), SkBits2Float(0xc2700000), SkBits2Float(0xb5600574), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x414d01d1), SkBits2Float(0xc2a40296));
path.cubicTo(SkBits2Float(0x4153c92e), SkBits2Float(0xc2a3e0b1), SkBits2Float(0x415a8e6d), SkBits2Float(0xc2a3bd1e), SkBits2Float(0x41615162), SkBits2Float(0xc2a397de));
path.lineTo(SkBits2Float(0x4122e164), SkBits2Float(0xc26c8535));
path.cubicTo(SkBits2Float(0x411dfe19), SkBits2Float(0xc26cbb11), SkBits2Float(0x41191928), SkBits2Float(0xc26cee7f), SkBits2Float(0x411432ab), SkBits2Float(0xc26d1f80));
path.lineTo(SkBits2Float(0x414d01d1), SkBits2Float(0xc2a40296));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp117(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x408949fd), SkBits2Float(0xc2a60000), SkBits2Float(0x41092f81), SkBits2Float(0xc2a555af), SkBits2Float(0x414d01d0), SkBits2Float(0xc2a40295));
path.lineTo(SkBits2Float(0x414d01d1), SkBits2Float(0xc2a40296));
path.cubicTo(SkBits2Float(0x4153c92e), SkBits2Float(0xc2a3e0b1), SkBits2Float(0x415a8e6d), SkBits2Float(0xc2a3bd1e), SkBits2Float(0x41615162), SkBits2Float(0xc2a397de));
path.lineTo(SkBits2Float(0x4122e164), SkBits2Float(0xc26c8535));
path.cubicTo(SkBits2Float(0x411dfe19), SkBits2Float(0xc26cbb11), SkBits2Float(0x41191928), SkBits2Float(0xc26cee7f), SkBits2Float(0x411432a9), SkBits2Float(0xc26d1f80));
path.cubicTo(SkBits2Float(0x40c65728), SkBits2Float(0xc26f09c3), SkBits2Float(0x40467d64), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41615164), SkBits2Float(0xc2a397de));
path.cubicTo(SkBits2Float(0x41a78432), SkBits2Float(0xc2a13b6d), SkBits2Float(0x41dcf7f2), SkBits2Float(0xc29d27e8), SkBits2Float(0x4207e0f5), SkBits2Float(0xc29775db));
path.lineTo(SkBits2Float(0x41c47380), SkBits2Float(0xc25afa96));
path.cubicTo(SkBits2Float(0x419fbc7e), SkBits2Float(0xc263369d), SkBits2Float(0x41723143), SkBits2Float(0xc2691b52), SkBits2Float(0x4122e168), SkBits2Float(0xc26c8537));
path.lineTo(SkBits2Float(0x41615164), SkBits2Float(0xc2a397de));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp118(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40a2e582), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4122b94f), SkBits2Float(0xc2a51039), SkBits2Float(0x4172cca0), SkBits2Float(0xc2a333b4));
path.lineTo(SkBits2Float(0x412f847d), SkBits2Float(0xc26bf464));
path.cubicTo(SkBits2Float(0x40eb4376), SkBits2Float(0xc26ea556), SkBits2Float(0x406b836d), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4172cc9b), SkBits2Float(0xc2a333b4));
path.cubicTo(SkBits2Float(0x417acd1a), SkBits2Float(0xc2a30415), SkBits2Float(0x41816508), SkBits2Float(0xc2a2d21d), SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcb));
path.lineTo(SkBits2Float(0x4140d724), SkBits2Float(0xc26b1ba8));
path.cubicTo(SkBits2Float(0x413b139d), SkBits2Float(0xc26b674c), SkBits2Float(0x41354d54), SkBits2Float(0xc26baf8b), SkBits2Float(0x412f847c), SkBits2Float(0xc26bf463));
path.lineTo(SkBits2Float(0x4172cc9b), SkBits2Float(0xc2a333b4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp119(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40a2e57f), SkBits2Float(0xc2a60000), SkBits2Float(0x4122b94c), SkBits2Float(0xc2a51039), SkBits2Float(0x4172cc9b), SkBits2Float(0xc2a333b4));
path.lineTo(SkBits2Float(0x4172cca0), SkBits2Float(0xc2a333b4));
path.cubicTo(SkBits2Float(0x417acd1d), SkBits2Float(0xc2a30415), SkBits2Float(0x41816509), SkBits2Float(0xc2a2d21d), SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcb));
path.lineTo(SkBits2Float(0x4140d724), SkBits2Float(0xc26b1ba8));
path.cubicTo(SkBits2Float(0x413b139d), SkBits2Float(0xc26b674c), SkBits2Float(0x41354d54), SkBits2Float(0xc26baf8b), SkBits2Float(0x412f847c), SkBits2Float(0xc26bf463));
path.lineTo(SkBits2Float(0x412f847d), SkBits2Float(0xc26bf464));
path.cubicTo(SkBits2Float(0x40eb4376), SkBits2Float(0xc26ea556), SkBits2Float(0x406b836d), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcc));
path.cubicTo(SkBits2Float(0x41c61a92), SkBits2Float(0xc29f4c69), SkBits2Float(0x42023dd6), SkBits2Float(0xc299958f), SkBits2Float(0x421f3a98), SkBits2Float(0xc291a994));
path.lineTo(SkBits2Float(0x41e635e1), SkBits2Float(0xc25298a5));
path.cubicTo(SkBits2Float(0x41bc4d11), SkBits2Float(0xc25e0caa), SkBits2Float(0x418f3524), SkBits2Float(0xc2664fa2), SkBits2Float(0x4140d729), SkBits2Float(0xc26b1ba9));
path.lineTo(SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcc));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp120(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40c39389), SkBits2Float(0xc2a60000), SkBits2Float(0x414346f4), SkBits2Float(0xc2a4a65f), SkBits2Float(0x419158cf), SkBits2Float(0xc2a1f965));
path.lineTo(SkBits2Float(0x415223e0), SkBits2Float(0xc26a2df8));
path.cubicTo(SkBits2Float(0x410d2a0c), SkBits2Float(0xc26e0c4b), SkBits2Float(0x408d616c), SkBits2Float(0xc2700000), SkBits2Float(0x35bbfd46), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x419158d0), SkBits2Float(0xc2a1f965));
path.cubicTo(SkBits2Float(0x41961cea), SkBits2Float(0xc2a1b4f6), SkBits2Float(0x419addf6), SkBits2Float(0xc2a16d2c), SkBits2Float(0x419f9bbb), SkBits2Float(0xc2a12207));
path.lineTo(SkBits2Float(0x4166c251), SkBits2Float(0xc268f69a));
path.cubicTo(SkBits2Float(0x415fe778), SkBits2Float(0xc269633e), SkBits2Float(0x415907e2), SkBits2Float(0xc269cb09), SkBits2Float(0x415223e0), SkBits2Float(0xc26a2df8));
path.lineTo(SkBits2Float(0x419158d0), SkBits2Float(0xc2a1f965));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp121(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40c39389), SkBits2Float(0xc2a60000), SkBits2Float(0x414346f4), SkBits2Float(0xc2a4a65f), SkBits2Float(0x419158d0), SkBits2Float(0xc2a1f965));
path.cubicTo(SkBits2Float(0x41961cea), SkBits2Float(0xc2a1b4f6), SkBits2Float(0x419addf6), SkBits2Float(0xc2a16d2c), SkBits2Float(0x419f9bbb), SkBits2Float(0xc2a12207));
path.lineTo(SkBits2Float(0x4166c251), SkBits2Float(0xc268f69a));
path.cubicTo(SkBits2Float(0x415fe778), SkBits2Float(0xc269633e), SkBits2Float(0x415907e2), SkBits2Float(0xc269cb09), SkBits2Float(0x415223e0), SkBits2Float(0xc26a2df8));
path.cubicTo(SkBits2Float(0x410d2a0c), SkBits2Float(0xc26e0c4b), SkBits2Float(0x408d616c), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x419f9bbc), SkBits2Float(0xc2a12208));
path.cubicTo(SkBits2Float(0x41eca53e), SkBits2Float(0xc29c5d1a), SkBits2Float(0x421ad1be), SkBits2Float(0xc2942e2b), SkBits2Float(0x423b8fe1), SkBits2Float(0xc288f8a3));
path.lineTo(SkBits2Float(0x42079647), SkBits2Float(0xc24607dc));
path.cubicTo(SkBits2Float(0x41dfd5cc), SkBits2Float(0xc2563c94), SkBits2Float(0x41ab11aa), SkBits2Float(0xc2621167), SkBits2Float(0x4166c24e), SkBits2Float(0xc268f69b));
path.lineTo(SkBits2Float(0x419f9bbc), SkBits2Float(0xc2a12208));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp122(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x410a1653), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4189aa2f), SkBits2Float(0xc2a34ed0), SkBits2Float(0x41cb63be), SkBits2Float(0xc29e054b));
path.lineTo(SkBits2Float(0x41930758), SkBits2Float(0xc26476b2));
path.cubicTo(SkBits2Float(0x41470896), SkBits2Float(0xc26c1b98), SkBits2Float(0x40c7a4f2), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41cb63c3), SkBits2Float(0xc29e054c));
path.cubicTo(SkBits2Float(0x41d1f2f3), SkBits2Float(0xc29d7e37), SkBits2Float(0x41d879a0), SkBits2Float(0xc29cf09c), SkBits2Float(0x41def72d), SkBits2Float(0xc29c5c87));
path.lineTo(SkBits2Float(0x41a12e10), SkBits2Float(0xc2621091));
path.cubicTo(SkBits2Float(0x419c7cee), SkBits2Float(0xc262e6aa), SkBits2Float(0x4197c536), SkBits2Float(0xc263b366), SkBits2Float(0x41930757), SkBits2Float(0xc26476b3));
path.lineTo(SkBits2Float(0x41cb63c3), SkBits2Float(0xc29e054c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp123(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x410a1653), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4189aa2f), SkBits2Float(0xc2a34ed0), SkBits2Float(0x41cb63be), SkBits2Float(0xc29e054b));
path.lineTo(SkBits2Float(0x41cb63c3), SkBits2Float(0xc29e054c));
path.cubicTo(SkBits2Float(0x41d1f2f3), SkBits2Float(0xc29d7e37), SkBits2Float(0x41d879a0), SkBits2Float(0xc29cf09c), SkBits2Float(0x41def72d), SkBits2Float(0xc29c5c87));
path.lineTo(SkBits2Float(0x41a12e10), SkBits2Float(0xc2621091));
path.cubicTo(SkBits2Float(0x419c7cee), SkBits2Float(0xc262e6aa), SkBits2Float(0x4197c536), SkBits2Float(0xc263b366), SkBits2Float(0x41930757), SkBits2Float(0xc26476b3));
path.lineTo(SkBits2Float(0x41930758), SkBits2Float(0xc26476b2));
path.cubicTo(SkBits2Float(0x41470896), SkBits2Float(0xc26c1b98), SkBits2Float(0x40c7a4f2), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea3), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41def730), SkBits2Float(0xc29c5c87));
path.cubicTo(SkBits2Float(0x422459f2), SkBits2Float(0xc292f017), SkBits2Float(0x42539427), SkBits2Float(0xc282f764), SkBits2Float(0x4278c050), SkBits2Float(0xc25be110));
path.lineTo(SkBits2Float(0x4233d1f5), SkBits2Float(0xc21ef2e3));
path.cubicTo(SkBits2Float(0x4218f2cf), SkBits2Float(0xc23d5956), SkBits2Float(0x41ed9dce), SkBits2Float(0xc25470b6), SkBits2Float(0x41a12e11), SkBits2Float(0xc2621092));
path.lineTo(SkBits2Float(0x41def730), SkBits2Float(0xc29c5c87));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp124(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x411fc00b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x419f1845), SkBits2Float(0xc2a265a5), SkBits2Float(0x41e9da2b), SkBits2Float(0xc29b5d43));
path.lineTo(SkBits2Float(0x41a90cc1), SkBits2Float(0xc2609f84));
path.cubicTo(SkBits2Float(0x41660440), SkBits2Float(0xc26aca7c), SkBits2Float(0x40e6f6cd), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa8c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41e9da2e), SkBits2Float(0xc29b5d44));
path.cubicTo(SkBits2Float(0x41f14eda), SkBits2Float(0xc29aa9b5), SkBits2Float(0x41f8b671), SkBits2Float(0xc299ed94), SkBits2Float(0x42000805), SkBits2Float(0xc29928f7));
path.lineTo(SkBits2Float(0x41b91b05), SkBits2Float(0xc25d6faa));
path.cubicTo(SkBits2Float(0x41b3cad4), SkBits2Float(0xc25e8bec), SkBits2Float(0x41ae7086), SkBits2Float(0xc25f9beb), SkBits2Float(0x41a90cc3), SkBits2Float(0xc2609f85));
path.lineTo(SkBits2Float(0x41e9da2e), SkBits2Float(0xc29b5d44));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp125(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x411fc00b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x419f1845), SkBits2Float(0xc2a265a5), SkBits2Float(0x41e9da2e), SkBits2Float(0xc29b5d44));
path.cubicTo(SkBits2Float(0x41f14eda), SkBits2Float(0xc29aa9b5), SkBits2Float(0x41f8b671), SkBits2Float(0xc299ed94), SkBits2Float(0x42000805), SkBits2Float(0xc29928f7));
path.lineTo(SkBits2Float(0x41b91b05), SkBits2Float(0xc25d6faa));
path.cubicTo(SkBits2Float(0x41b3cad4), SkBits2Float(0xc25e8bec), SkBits2Float(0x41ae7086), SkBits2Float(0xc25f9beb), SkBits2Float(0x41a90cc1), SkBits2Float(0xc2609f84));
path.cubicTo(SkBits2Float(0x41660440), SkBits2Float(0xc26aca7c), SkBits2Float(0x40e6f6cd), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42000806), SkBits2Float(0xc29928f8));
path.cubicTo(SkBits2Float(0x423c0231), SkBits2Float(0xc28ca034), SkBits2Float(0x426f4e95), SkBits2Float(0xc26f2095), SkBits2Float(0x4289c821), SkBits2Float(0xc2392c12));
path.lineTo(SkBits2Float(0x424733db), SkBits2Float(0xc205dc02));
path.cubicTo(SkBits2Float(0x422cfe35), SkBits2Float(0xc22cdcf5), SkBits2Float(0x4207e8ea), SkBits2Float(0xc24b507f), SkBits2Float(0x41b91b06), SkBits2Float(0xc25d6faa));
path.lineTo(SkBits2Float(0x42000806), SkBits2Float(0xc29928f8));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp126(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41379cd4), SkBits2Float(0xc2a60000), SkBits2Float(0x41b69d77), SkBits2Float(0xc2a13d93), SkBits2Float(0x42055871), SkBits2Float(0xc29805ae));
path.lineTo(SkBits2Float(0x41c0c9e6), SkBits2Float(0xc25bca86));
path.cubicTo(SkBits2Float(0x418402cc), SkBits2Float(0xc2691e6b), SkBits2Float(0x4104bb66), SkBits2Float(0xc26fffff), SkBits2Float(0x3673fea5), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42055872), SkBits2Float(0xc29805ae));
path.cubicTo(SkBits2Float(0x420988d2), SkBits2Float(0xc2971a85), SkBits2Float(0x420daf5c), SkBits2Float(0xc296244f), SkBits2Float(0x4211cb64), SkBits2Float(0xc2952332));
path.lineTo(SkBits2Float(0x41d2c988), SkBits2Float(0xc2579ed7));
path.cubicTo(SkBits2Float(0x41ccd887), SkBits2Float(0xc2591291), SkBits2Float(0x41c6d852), SkBits2Float(0xc25a7689), SkBits2Float(0x41c0c9e6), SkBits2Float(0xc25bca86));
path.lineTo(SkBits2Float(0x42055872), SkBits2Float(0xc29805ae));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp127(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3673fea5), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41379cd4), SkBits2Float(0xc2a60000), SkBits2Float(0x41b69d77), SkBits2Float(0xc2a13d93), SkBits2Float(0x42055872), SkBits2Float(0xc29805ae));
path.cubicTo(SkBits2Float(0x420988d2), SkBits2Float(0xc2971a85), SkBits2Float(0x420daf5c), SkBits2Float(0xc296244f), SkBits2Float(0x4211cb64), SkBits2Float(0xc2952332));
path.lineTo(SkBits2Float(0x41d2c988), SkBits2Float(0xc2579ed7));
path.cubicTo(SkBits2Float(0x41ccd887), SkBits2Float(0xc2591291), SkBits2Float(0x41c6d852), SkBits2Float(0xc25a7689), SkBits2Float(0x41c0c9e6), SkBits2Float(0xc25bca86));
path.cubicTo(SkBits2Float(0x418402cc), SkBits2Float(0xc2691e6b), SkBits2Float(0x4104bb66), SkBits2Float(0xc26fffff), SkBits2Float(0x3673fea5), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4211cb65), SkBits2Float(0xc2952332));
path.cubicTo(SkBits2Float(0x42550406), SkBits2Float(0xc284b578), SkBits2Float(0x42859569), SkBits2Float(0xc252d13a), SkBits2Float(0x4295bbf4), SkBits2Float(0xc20f53bf));
path.lineTo(SkBits2Float(0x42587bb2), SkBits2Float(0xc1cf3850));
path.cubicTo(SkBits2Float(0x4241220a), SkBits2Float(0xc21865e8), SkBits2Float(0x4219fcbd), SkBits2Float(0xc23fde48), SkBits2Float(0x41d2c988), SkBits2Float(0xc2579ed8));
path.lineTo(SkBits2Float(0x4211cb65), SkBits2Float(0xc2952332));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp128(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4151cd59), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41d04f3f), SkBits2Float(0xc29fc954), SkBits2Float(0x4216e058), SkBits2Float(0xc293de54));
path.lineTo(SkBits2Float(0x41da226b), SkBits2Float(0xc255c926));
path.cubicTo(SkBits2Float(0x419695d1), SkBits2Float(0xc267043d), SkBits2Float(0x4117aa0a), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4216e057), SkBits2Float(0xc293de54));
path.cubicTo(SkBits2Float(0x421b86ea), SkBits2Float(0xc292aea0), SkBits2Float(0x42201eff), SkBits2Float(0xc29170ed), SkBits2Float(0x4224a79b), SkBits2Float(0xc290257e));
path.lineTo(SkBits2Float(0x41ee0e15), SkBits2Float(0xc2506790));
path.cubicTo(SkBits2Float(0x41e78019), SkBits2Float(0xc25246bf), SkBits2Float(0x41e0dbbc), SkBits2Float(0xc2541212), SkBits2Float(0x41da226b), SkBits2Float(0xc255c927));
path.lineTo(SkBits2Float(0x4216e057), SkBits2Float(0xc293de54));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp129(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4151cd58), SkBits2Float(0xc2a60000), SkBits2Float(0x41d04f3d), SkBits2Float(0xc29fc954), SkBits2Float(0x4216e057), SkBits2Float(0xc293de54));
path.lineTo(SkBits2Float(0x4216e058), SkBits2Float(0xc293de54));
path.cubicTo(SkBits2Float(0x421b86eb), SkBits2Float(0xc292aea0), SkBits2Float(0x42201eff), SkBits2Float(0xc29170ed), SkBits2Float(0x4224a79b), SkBits2Float(0xc290257e));
path.lineTo(SkBits2Float(0x41ee0e15), SkBits2Float(0xc2506790));
path.cubicTo(SkBits2Float(0x41e78019), SkBits2Float(0xc25246bf), SkBits2Float(0x41e0dbbc), SkBits2Float(0xc2541212), SkBits2Float(0x41da226b), SkBits2Float(0xc255c927));
path.lineTo(SkBits2Float(0x41da226b), SkBits2Float(0xc255c926));
path.cubicTo(SkBits2Float(0x419695d1), SkBits2Float(0xc267043d), SkBits2Float(0x4117aa0a), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4224a79b), SkBits2Float(0xc290257f));
path.cubicTo(SkBits2Float(0x426f06c3), SkBits2Float(0xc275d105), SkBits2Float(0x42930d85), SkBits2Float(0xc2303df6), SkBits2Float(0x429f3103), SkBits2Float(0xc1bc373f));
path.lineTo(SkBits2Float(0x42662806), SkBits2Float(0xc1880f44));
path.cubicTo(SkBits2Float(0x42549b44), SkBits2Float(0xc1fececc), SkBits2Float(0x422cca4c), SkBits2Float(0xc231b2de), SkBits2Float(0x41ee0e18), SkBits2Float(0xc2506792));
path.lineTo(SkBits2Float(0x4224a79b), SkBits2Float(0xc290257f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp130(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x417054a2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41ee1405), SkBits2Float(0xc29dd904), SkBits2Float(0x422a9595), SkBits2Float(0xc28e6989));
path.lineTo(SkBits2Float(0x41f6a0c0), SkBits2Float(0xc24de5b0));
path.cubicTo(SkBits2Float(0x41ac1ad0), SkBits2Float(0xc26436ad), SkBits2Float(0x412dbba0), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x422a9596), SkBits2Float(0xc28e6989));
path.cubicTo(SkBits2Float(0x422fb535), SkBits2Float(0xc28ce0c4), SkBits2Float(0x4234bf65), SkBits2Float(0xc28b465e), SkBits2Float(0x4239b2bc), SkBits2Float(0xc2899acc));
path.lineTo(SkBits2Float(0x42063d5a), SkBits2Float(0xc246f24e));
path.cubicTo(SkBits2Float(0x4202a934), SkBits2Float(0xc2495c7c), SkBits2Float(0x41fe0912), SkBits2Float(0xc24badd5), SkBits2Float(0x41f6a0c0), SkBits2Float(0xc24de5b1));
path.lineTo(SkBits2Float(0x422a9596), SkBits2Float(0xc28e6989));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp131(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x417054a2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41ee1405), SkBits2Float(0xc29dd904), SkBits2Float(0x422a9596), SkBits2Float(0xc28e6989));
path.cubicTo(SkBits2Float(0x422fb535), SkBits2Float(0xc28ce0c4), SkBits2Float(0x4234bf65), SkBits2Float(0xc28b465e), SkBits2Float(0x4239b2bc), SkBits2Float(0xc2899acc));
path.lineTo(SkBits2Float(0x42063d5a), SkBits2Float(0xc246f24e));
path.cubicTo(SkBits2Float(0x4202a934), SkBits2Float(0xc2495c7c), SkBits2Float(0x41fe0912), SkBits2Float(0xc24badd5), SkBits2Float(0x41f6a0c0), SkBits2Float(0xc24de5b0));
path.cubicTo(SkBits2Float(0x41ac1ad0), SkBits2Float(0xc26436ad), SkBits2Float(0x412dbba0), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4239b2bd), SkBits2Float(0xc2899acc));
path.cubicTo(SkBits2Float(0x42859c2b), SkBits2Float(0xc25c33ca), SkBits2Float(0x42a01474), SkBits2Float(0xc203e23a), SkBits2Float(0x42a51fce), SkBits2Float(0xc1083bae));
path.lineTo(SkBits2Float(0x426ebbdb), SkBits2Float(0xc0c4f6ab));
path.cubicTo(SkBits2Float(0x426770d9), SkBits2Float(0xc1beacda), SkBits2Float(0x42412bce), SkBits2Float(0xc21f2eb0), SkBits2Float(0x42063d5a), SkBits2Float(0xc246f24e));
path.lineTo(SkBits2Float(0x4239b2bd), SkBits2Float(0xc2899acc));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp132(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4187e175), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42063ec3), SkBits2Float(0xc29b93fb), SkBits2Float(0x423df6fd), SkBits2Float(0xc2882410));
path.lineTo(SkBits2Float(0x420952ef), SkBits2Float(0xc244d488));
path.cubicTo(SkBits2Float(0x41c216e4), SkBits2Float(0xc260eea0), SkBits2Float(0x4144743c), SkBits2Float(0xc26fffff), SkBits2Float(0x357ffa94), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423df6fe), SkBits2Float(0xc2882411));
path.cubicTo(SkBits2Float(0x42437e7a), SkBits2Float(0xc286364a), SkBits2Float(0x4248e78f), SkBits2Float(0xc2843312), SkBits2Float(0x424e304d), SkBits2Float(0xc2821b20));
path.lineTo(SkBits2Float(0x42150d53), SkBits2Float(0xc23c1ae0));
path.cubicTo(SkBits2Float(0x42113b72), SkBits2Float(0xc23f21be), SkBits2Float(0x420d522e), SkBits2Float(0xc2420aa4), SkBits2Float(0x420952ef), SkBits2Float(0xc244d48a));
path.lineTo(SkBits2Float(0x423df6fe), SkBits2Float(0xc2882411));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp133(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4187e175), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42063ec3), SkBits2Float(0xc29b93fb), SkBits2Float(0x423df6fe), SkBits2Float(0xc2882411));
path.cubicTo(SkBits2Float(0x42437e7a), SkBits2Float(0xc286364a), SkBits2Float(0x4248e78f), SkBits2Float(0xc2843312), SkBits2Float(0x424e304d), SkBits2Float(0xc2821b20));
path.lineTo(SkBits2Float(0x42150d53), SkBits2Float(0xc23c1ae0));
path.cubicTo(SkBits2Float(0x42113b72), SkBits2Float(0xc23f21be), SkBits2Float(0x420d522e), SkBits2Float(0xc2420aa4), SkBits2Float(0x420952ef), SkBits2Float(0xc244d48a));
path.lineTo(SkBits2Float(0x420952ef), SkBits2Float(0xc244d488));
path.cubicTo(SkBits2Float(0x41c216e4), SkBits2Float(0xc260eea0), SkBits2Float(0x4144743c), SkBits2Float(0xc26fffff), SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x424e304d), SkBits2Float(0xc2821b20));
path.cubicTo(SkBits2Float(0x4292cbf1), SkBits2Float(0xc23ef41d), SkBits2Float(0x42aa31a6), SkBits2Float(0xc1a4e14c), SkBits2Float(0x42a56158), SkBits2Float(0x40e54b3a));
path.lineTo(SkBits2Float(0x426f1a9e), SkBits2Float(0x40a5c12f));
path.cubicTo(SkBits2Float(0x42761044), SkBits2Float(0xc16e617c), SkBits2Float(0x42543c73), SkBits2Float(0xc20a09ea), SkBits2Float(0x42150d54), SkBits2Float(0xc23c1ae1));
path.lineTo(SkBits2Float(0x424e304d), SkBits2Float(0xc2821b20));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp134(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x419c5b1f), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4219d929), SkBits2Float(0xc29834b3), SkBits2Float(0x4255ae76), SkBits2Float(0xc27e184c));
path.lineTo(SkBits2Float(0x421a77f2), SkBits2Float(0xc237aede));
path.cubicTo(SkBits2Float(0x41de6e66), SkBits2Float(0xc25c0e82), SkBits2Float(0x41620e8a), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4255ae76), SkBits2Float(0xc27e184c));
path.cubicTo(SkBits2Float(0x425b9ab5), SkBits2Float(0xc2791d33), SkBits2Float(0x426159ea), SkBits2Float(0xc273ed7b), SkBits2Float(0x4266e960), SkBits2Float(0xc26e8b92));
path.lineTo(SkBits2Float(0x4226ec90), SkBits2Float(0xc22c713c));
path.cubicTo(SkBits2Float(0x4222e78d), SkBits2Float(0xc2305550), SkBits2Float(0x421ec008), SkBits2Float(0xc234151d), SkBits2Float(0x421a77f3), SkBits2Float(0xc237aedd));
path.lineTo(SkBits2Float(0x4255ae76), SkBits2Float(0xc27e184c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp135(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x419c5b1f), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4219d929), SkBits2Float(0xc29834b3), SkBits2Float(0x4255ae76), SkBits2Float(0xc27e184c));
path.cubicTo(SkBits2Float(0x425b9ab5), SkBits2Float(0xc2791d33), SkBits2Float(0x426159ea), SkBits2Float(0xc273ed7b), SkBits2Float(0x4266e960), SkBits2Float(0xc26e8b92));
path.lineTo(SkBits2Float(0x4226ec90), SkBits2Float(0xc22c713c));
path.cubicTo(SkBits2Float(0x4222e78d), SkBits2Float(0xc2305550), SkBits2Float(0x421ec008), SkBits2Float(0xc234151d), SkBits2Float(0x421a77f2), SkBits2Float(0xc237aede));
path.cubicTo(SkBits2Float(0x41de6e66), SkBits2Float(0xc25c0e82), SkBits2Float(0x41620e8a), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4266e961), SkBits2Float(0xc26e8b93));
path.cubicTo(SkBits2Float(0x42a1bfce), SkBits2Float(0xc214ebcf), SkBits2Float(0x42b1ee5a), SkBits2Float(0xc05d1412), SkBits2Float(0x429cf75a), SkBits2Float(0x41d80f2c));
path.lineTo(SkBits2Float(0x4262f06b), SkBits2Float(0x419c2ffb));
path.cubicTo(SkBits2Float(0x42809ff9), SkBits2Float(0xc01fd0e5), SkBits2Float(0x4269dab8), SkBits2Float(0xc1d74ec6), SkBits2Float(0x4226ec91), SkBits2Float(0xc22c713d));
path.lineTo(SkBits2Float(0x4266e961), SkBits2Float(0xc26e8b93));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp136(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ae0130), SkBits2Float(0xc2a5ffff), SkBits2Float(0x422a8737), SkBits2Float(0xc294ec91), SkBits2Float(0x42689b67), SkBits2Float(0xc26ce46c));
path.lineTo(SkBits2Float(0x42282651), SkBits2Float(0xc22b3f58));
path.cubicTo(SkBits2Float(0x41f68bfb), SkBits2Float(0xc2574fdc), SkBits2Float(0x417b92b3), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42689b68), SkBits2Float(0xc26ce46d));
path.cubicTo(SkBits2Float(0x426ebcd2), SkBits2Float(0xc266df67), SkBits2Float(0x4274a1d2), SkBits2Float(0xc2609e09), SkBits2Float(0x427a4701), SkBits2Float(0xc25a23f2));
path.lineTo(SkBits2Float(0x4234ec64), SkBits2Float(0xc21db11e));
path.cubicTo(SkBits2Float(0x4230d7ae), SkBits2Float(0xc2225fbc), SkBits2Float(0x422c94d6), SkBits2Float(0xc226e55a), SkBits2Float(0x42282652), SkBits2Float(0xc22b3f58));
path.lineTo(SkBits2Float(0x42689b68), SkBits2Float(0xc26ce46d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp137(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ae0130), SkBits2Float(0xc2a5ffff), SkBits2Float(0x422a8737), SkBits2Float(0xc294ec91), SkBits2Float(0x42689b68), SkBits2Float(0xc26ce46d));
path.cubicTo(SkBits2Float(0x426ebcd2), SkBits2Float(0xc266df67), SkBits2Float(0x4274a1d2), SkBits2Float(0xc2609e09), SkBits2Float(0x427a4701), SkBits2Float(0xc25a23f2));
path.lineTo(SkBits2Float(0x4234ec64), SkBits2Float(0xc21db11e));
path.cubicTo(SkBits2Float(0x4230d7ae), SkBits2Float(0xc2225fbc), SkBits2Float(0x422c94d6), SkBits2Float(0xc226e55a), SkBits2Float(0x42282651), SkBits2Float(0xc22b3f58));
path.cubicTo(SkBits2Float(0x41f68bfb), SkBits2Float(0xc2574fdc), SkBits2Float(0x417b92b3), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427a4702), SkBits2Float(0xc25a23f2));
path.cubicTo(SkBits2Float(0x42ac7185), SkBits2Float(0xc1db2f83), SkBits2Float(0x42b35ed0), SkBits2Float(0x413e447a), SkBits2Float(0x428e4a3d), SkBits2Float(0x422afde8));
path.lineTo(SkBits2Float(0x424db871), SkBits2Float(0x41f73799));
path.cubicTo(SkBits2Float(0x4281aa54), SkBits2Float(0x41098afa), SkBits2Float(0x427950da), SkBits2Float(0xc19e728d), SkBits2Float(0x4234ec66), SkBits2Float(0xc21db120));
path.lineTo(SkBits2Float(0x427a4702), SkBits2Float(0xc25a23f2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp138(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c2602d), SkBits2Float(0xc2a5ffff), SkBits2Float(0x423d7ece), SkBits2Float(0xc290b51a), SkBits2Float(0x427c92bc), SkBits2Float(0xc2577a5f));
path.lineTo(SkBits2Float(0x42369543), SkBits2Float(0xc21bc469));
path.cubicTo(SkBits2Float(0x4208fc10), SkBits2Float(0xc2513731), SkBits2Float(0x418c8338), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427c92be), SkBits2Float(0xc2577a5f));
path.cubicTo(SkBits2Float(0x42816448), SkBits2Float(0xc25032db), SkBits2Float(0x42845689), SkBits2Float(0xc248a77c), SkBits2Float(0x42871e08), SkBits2Float(0xc240ddaa));
path.lineTo(SkBits2Float(0x424359af), SkBits2Float(0xc20b6bce));
path.cubicTo(SkBits2Float(0x423f5505), SkBits2Float(0xc2110d1f), SkBits2Float(0x423b1287), SkBits2Float(0xc216814b), SkBits2Float(0x42369543), SkBits2Float(0xc21bc46a));
path.lineTo(SkBits2Float(0x427c92be), SkBits2Float(0xc2577a5f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp139(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c2602d), SkBits2Float(0xc2a5ffff), SkBits2Float(0x423d7ece), SkBits2Float(0xc290b51a), SkBits2Float(0x427c92bc), SkBits2Float(0xc2577a5f));
path.lineTo(SkBits2Float(0x427c92be), SkBits2Float(0xc2577a5f));
path.cubicTo(SkBits2Float(0x42816448), SkBits2Float(0xc25032db), SkBits2Float(0x42845689), SkBits2Float(0xc248a77c), SkBits2Float(0x42871e08), SkBits2Float(0xc240ddaa));
path.lineTo(SkBits2Float(0x424359af), SkBits2Float(0xc20b6bce));
path.cubicTo(SkBits2Float(0x423f5505), SkBits2Float(0xc2110d1f), SkBits2Float(0x423b1287), SkBits2Float(0xc216814a), SkBits2Float(0x42369543), SkBits2Float(0xc21bc469));
path.lineTo(SkBits2Float(0x42369543), SkBits2Float(0xc21bc46a));
path.cubicTo(SkBits2Float(0x4208fc10), SkBits2Float(0xc2513732), SkBits2Float(0x418c8337), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42871e08), SkBits2Float(0xc240ddaa));
path.cubicTo(SkBits2Float(0x42b615a2), SkBits2Float(0xc174ff4e), SkBits2Float(0x42aecf41), SkBits2Float(0x41edcc49), SkBits2Float(0x426bc7a7), SkBits2Float(0x4269bc09));
path.lineTo(SkBits2Float(0x422a717e), SkBits2Float(0x4228f6f7));
path.cubicTo(SkBits2Float(0x427cbca0), SkBits2Float(0x41abe6f4), SkBits2Float(0x4283a09b), SkBits2Float(0xc1311b44), SkBits2Float(0x424359af), SkBits2Float(0xc20b6bcd));
path.lineTo(SkBits2Float(0x42871e08), SkBits2Float(0xc240ddaa));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp140(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d9e52a), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4252f644), SkBits2Float(0xc28b460f), SkBits2Float(0x42887c98), SkBits2Float(0xc23cf83b));
path.lineTo(SkBits2Float(0x42455485), SkBits2Float(0xc2089ac5));
path.cubicTo(SkBits2Float(0x421880ae), SkBits2Float(0xc2495c0a), SkBits2Float(0x419d83bb), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42887c98), SkBits2Float(0xc23cf83b));
path.cubicTo(SkBits2Float(0x428b8706), SkBits2Float(0xc2342f4a), SkBits2Float(0x428e5ab7), SkBits2Float(0xc22b1c84), SkBits2Float(0x4290f525), SkBits2Float(0xc221c800));
path.lineTo(SkBits2Float(0x425193c7), SkBits2Float(0xc1e9e68d));
path.cubicTo(SkBits2Float(0x424dd044), SkBits2Float(0xc1f763d3), SkBits2Float(0x4249b9f6), SkBits2Float(0xc2024108), SkBits2Float(0x42455485), SkBits2Float(0xc2089ac6));
path.lineTo(SkBits2Float(0x42887c98), SkBits2Float(0xc23cf83b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp141(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d9e52a), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4252f644), SkBits2Float(0xc28b460f), SkBits2Float(0x42887c98), SkBits2Float(0xc23cf83b));
path.cubicTo(SkBits2Float(0x428b8706), SkBits2Float(0xc2342f4a), SkBits2Float(0x428e5ab7), SkBits2Float(0xc22b1c84), SkBits2Float(0x4290f525), SkBits2Float(0xc221c800));
path.lineTo(SkBits2Float(0x425193c7), SkBits2Float(0xc1e9e68d));
path.cubicTo(SkBits2Float(0x424dd044), SkBits2Float(0xc1f763d3), SkBits2Float(0x4249b9f6), SkBits2Float(0xc2024107), SkBits2Float(0x42455485), SkBits2Float(0xc2089ac5));
path.lineTo(SkBits2Float(0x42455485), SkBits2Float(0xc2089ac6));
path.cubicTo(SkBits2Float(0x421880ae), SkBits2Float(0xc2495c0b), SkBits2Float(0x419d83ba), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4290f526), SkBits2Float(0xc221c800));
path.cubicTo(SkBits2Float(0x42bd6cdd), SkBits2Float(0xbf1a1474), SkBits2Float(0x42a13baa), SkBits2Float(0x4246de93), SkBits2Float(0x4223add7), SkBits2Float(0x42906c8a));
path.lineTo(SkBits2Float(0x41eca4f8), SkBits2Float(0x4250ce48));
path.cubicTo(SkBits2Float(0x42691bac), SkBits2Float(0x420fc2d7), SkBits2Float(0x4288ef16), SkBits2Float(0xbedec420), SkBits2Float(0x425193c9), SkBits2Float(0xc1e9e690));
path.lineTo(SkBits2Float(0x4290f526), SkBits2Float(0xc221c800));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp142(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f6a97d), SkBits2Float(0xc2a60000), SkBits2Float(0x426c7f9e), SkBits2Float(0xc283d12f), SkBits2Float(0x4292f07c), SkBits2Float(0xc21a76e5));
path.lineTo(SkBits2Float(0x42547147), SkBits2Float(0xc1df5274));
path.cubicTo(SkBits2Float(0x422af677), SkBits2Float(0xc23e9438), SkBits2Float(0x41b24f58), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4292f07c), SkBits2Float(0xc21a76e5));
path.cubicTo(SkBits2Float(0x4295bcf6), SkBits2Float(0xc20fd099), SkBits2Float(0x42983ed1), SkBits2Float(0xc204de6d), SkBits2Float(0x429a7333), SkBits2Float(0xc1f3598c));
path.lineTo(SkBits2Float(0x425f4d1c), SkBits2Float(0xc1afea60));
path.cubicTo(SkBits2Float(0x425c1d22), SkBits2Float(0xc1c0197b), SkBits2Float(0x42587d28), SkBits2Float(0xc1cfecd2), SkBits2Float(0x42547148), SkBits2Float(0xc1df5275));
path.lineTo(SkBits2Float(0x4292f07c), SkBits2Float(0xc21a76e5));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp143(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f6a97d), SkBits2Float(0xc2a60000), SkBits2Float(0x426c7f9e), SkBits2Float(0xc283d12f), SkBits2Float(0x4292f07c), SkBits2Float(0xc21a76e5));
path.cubicTo(SkBits2Float(0x4295bcf6), SkBits2Float(0xc20fd099), SkBits2Float(0x42983ed1), SkBits2Float(0xc204de6d), SkBits2Float(0x429a7333), SkBits2Float(0xc1f3598c));
path.lineTo(SkBits2Float(0x425f4d1c), SkBits2Float(0xc1afea60));
path.cubicTo(SkBits2Float(0x425c1d22), SkBits2Float(0xc1c0197b), SkBits2Float(0x42587d28), SkBits2Float(0xc1cfecd2), SkBits2Float(0x42547147), SkBits2Float(0xc1df5274));
path.cubicTo(SkBits2Float(0x422af677), SkBits2Float(0xc23e9438), SkBits2Float(0x41b24f58), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429a7334), SkBits2Float(0xc1f3598d));
path.cubicTo(SkBits2Float(0x42ac9a56), SkBits2Float(0xc0ec08d5), SkBits2Float(0x42a93a4b), SkBits2Float(0x4194209c), SkBits2Float(0x42913f11), SkBits2Float(0x4220bdeb));
path.cubicTo(SkBits2Float(0x427287b0), SkBits2Float(0x42776b87), SkBits2Float(0x421e5dc6), SkBits2Float(0x429a1372), SkBits2Float(0x4173f4a4), SkBits2Float(0x42a32ccd));
path.lineTo(SkBits2Float(0x41305a7f), SkBits2Float(0x426bea6b));
path.cubicTo(SkBits2Float(0x41e4f69e), SkBits2Float(0x425ec2af), SkBits2Float(0x422f52ad), SkBits2Float(0x4232db9e), SkBits2Float(0x4251feaa), SkBits2Float(0x41e865df));
path.cubicTo(SkBits2Float(0x4274aaa7), SkBits2Float(0x41562902), SkBits2Float(0x42798bdd), SkBits2Float(0xc0aaa09a), SkBits2Float(0x425f4d1d), SkBits2Float(0xc1afea60));
path.lineTo(SkBits2Float(0x429a7334), SkBits2Float(0xc1f3598d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp144(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42079c39), SkBits2Float(0xc2a60000), SkBits2Float(0x4280cb64), SkBits2Float(0xc279860f), SkBits2Float(0x429a0d79), SkBits2Float(0xc1f758df));
path.lineTo(SkBits2Float(0x425eba08), SkBits2Float(0xc1b2ce1f));
path.cubicTo(SkBits2Float(0x423a357b), SkBits2Float(0xc23460ea), SkBits2Float(0x41c41023), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429a0d79), SkBits2Float(0xc1f758de));
path.cubicTo(SkBits2Float(0x429c811b), SkBits2Float(0xc1deea6e), SkBits2Float(0x429e9731), SkBits2Float(0xc1c5ec3a), SkBits2Float(0x42a04ce7), SkBits2Float(0xc1ac8024));
path.lineTo(SkBits2Float(0x4267c277), SkBits2Float(0xc17965fc));
path.cubicTo(SkBits2Float(0x426549a1), SkBits2Float(0xc18f13a3), SkBits2Float(0x42624575), SkBits2Float(0xc1a124d8), SkBits2Float(0x425eba09), SkBits2Float(0xc1b2ce1e));
path.lineTo(SkBits2Float(0x429a0d79), SkBits2Float(0xc1f758de));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp145(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42079c39), SkBits2Float(0xc2a60000), SkBits2Float(0x4280cb64), SkBits2Float(0xc279860f), SkBits2Float(0x429a0d79), SkBits2Float(0xc1f758df));
path.lineTo(SkBits2Float(0x42a04ce7), SkBits2Float(0xc1ac8024));
path.lineTo(SkBits2Float(0x4267c277), SkBits2Float(0xc17965fc));
path.cubicTo(SkBits2Float(0x426549a1), SkBits2Float(0xc18f13a3), SkBits2Float(0x42624575), SkBits2Float(0xc1a124d8), SkBits2Float(0x425eba09), SkBits2Float(0xc1b2ce1e));
path.lineTo(SkBits2Float(0x425eba08), SkBits2Float(0xc1b2ce1f));
path.cubicTo(SkBits2Float(0x423a357b), SkBits2Float(0xc23460ea), SkBits2Float(0x41c41023), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a04ce8), SkBits2Float(0xc1ac8024));
path.cubicTo(SkBits2Float(0x42ae6ca1), SkBits2Float(0x4095ff41), SkBits2Float(0x42a1f1fa), SkBits2Float(0x4202ed54), SkBits2Float(0x427dc9de), SkBits2Float(0x42560b98));
path.cubicTo(SkBits2Float(0x4237afc7), SkBits2Float(0x429494ee), SkBits2Float(0x419aa752), SkBits2Float(0x42aa57e8), SkBits2Float(0xc0f777b3), SkBits2Float(0x42a54724));
path.lineTo(SkBits2Float(0xc0b2e472), SkBits2Float(0x426ef4bb));
path.cubicTo(SkBits2Float(0x415f9870), SkBits2Float(0x42764794), SkBits2Float(0x4204c916), SkBits2Float(0x4256d126), SkBits2Float(0x4237762a), SkBits2Float(0x421abb46));
path.cubicTo(SkBits2Float(0x426a233f), SkBits2Float(0x41bd4acb), SkBits2Float(0x427c2e04), SkBits2Float(0x4058dcfe), SkBits2Float(0x4267c279), SkBits2Float(0xc17965fc));
path.lineTo(SkBits2Float(0x42a04ce8), SkBits2Float(0xc1ac8024));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp146(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x421472e7), SkBits2Float(0xc2a5ffff), SkBits2Float(0x428b6da4), SkBits2Float(0xc26973d7), SkBits2Float(0x429fb179), SkBits2Float(0xc1b54986));
path.lineTo(SkBits2Float(0x4266e1be), SkBits2Float(0xc1830d0f));
path.cubicTo(SkBits2Float(0x42499544), SkBits2Float(0xc228c2c8), SkBits2Float(0x41d69ff6), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429fb179), SkBits2Float(0xc1b54988));
path.cubicTo(SkBits2Float(0x42a1a632), SkBits2Float(0xc199b837), SkBits2Float(0x42a3282f), SkBits2Float(0xc17b594e), SkBits2Float(0x42a43501), SkBits2Float(0xc142a7ba));
path.lineTo(SkBits2Float(0x426d6865), SkBits2Float(0xc10cb6f0));
path.cubicTo(SkBits2Float(0x426be3bc), SkBits2Float(0xc135b2ae), SkBits2Float(0x4269b5af), SkBits2Float(0xc15e3ec8), SkBits2Float(0x4266e1be), SkBits2Float(0xc1830d0f));
path.lineTo(SkBits2Float(0x429fb179), SkBits2Float(0xc1b54988));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp147(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x421472e7), SkBits2Float(0xc2a60000), SkBits2Float(0x428b6da4), SkBits2Float(0xc26973d8), SkBits2Float(0x429fb179), SkBits2Float(0xc1b54988));
path.lineTo(SkBits2Float(0x429fb179), SkBits2Float(0xc1b54986));
path.cubicTo(SkBits2Float(0x42a1a632), SkBits2Float(0xc199b836), SkBits2Float(0x42a3282f), SkBits2Float(0xc17b594d), SkBits2Float(0x42a43501), SkBits2Float(0xc142a7ba));
path.lineTo(SkBits2Float(0x426d6865), SkBits2Float(0xc10cb6f0));
path.cubicTo(SkBits2Float(0x426be3bc), SkBits2Float(0xc135b2ae), SkBits2Float(0x4269b5af), SkBits2Float(0xc15e3ec8), SkBits2Float(0x4266e1be), SkBits2Float(0xc1830d0f));
path.cubicTo(SkBits2Float(0x42499544), SkBits2Float(0xc228c2c8), SkBits2Float(0x41d69ff6), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a43502), SkBits2Float(0xc142a7bb));
path.cubicTo(SkBits2Float(0x42ace9b0), SkBits2Float(0x4189ae79), SkBits2Float(0x429590d6), SkBits2Float(0x423ab1c1), SkBits2Float(0x424df762), SkBits2Float(0x428231a6));
path.cubicTo(SkBits2Float(0x41e19a31), SkBits2Float(0x42a70a69), SkBits2Float(0xc04a3289), SkBits2Float(0x42b03133), SkBits2Float(0xc1f5f36e), SkBits2Float(0x429a3139));
path.lineTo(SkBits2Float(0xc1b1cbb9), SkBits2Float(0x425eedb9));
path.cubicTo(SkBits2Float(0xc0122aac), SkBits2Float(0x427ebc5a), SkBits2Float(0x41a31606), SkBits2Float(0x42718130), SkBits2Float(0x4214e430), SkBits2Float(0x423c3b73));
path.cubicTo(SkBits2Float(0x42583d5c), SkBits2Float(0x4206f5b6), SkBits2Float(0x4279fe97), SkBits2Float(0x41470ec8), SkBits2Float(0x426d6866), SkBits2Float(0xc10cb6eb));
path.lineTo(SkBits2Float(0x42a43502), SkBits2Float(0xc142a7bb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp148(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42216831), SkBits2Float(0xc2a60000), SkBits2Float(0x4295b6bc), SkBits2Float(0xc257ea44), SkBits2Float(0x42a38b53), SkBits2Float(0xc1639572));
path.lineTo(SkBits2Float(0x426c7311), SkBits2Float(0xc12484b9));
path.cubicTo(SkBits2Float(0x42587424), SkBits2Float(0xc21c154e), SkBits2Float(0x41e95c08), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a38b52), SkBits2Float(0xc1639578));
path.cubicTo(SkBits2Float(0x42a4def8), SkBits2Float(0xc1269090), SkBits2Float(0x42a5a99a), SkBits2Float(0xc0d1c16f), SkBits2Float(0x42a5e9be), SkBits2Float(0xc02be63c));
path.lineTo(SkBits2Float(0x426fdfd2), SkBits2Float(0xbff8877d));
path.cubicTo(SkBits2Float(0x426f8319), SkBits2Float(0xc097a16e), SkBits2Float(0x426e5e22), SkBits2Float(0xc0f0d105), SkBits2Float(0x426c7311), SkBits2Float(0xc12484ba));
path.lineTo(SkBits2Float(0x42a38b52), SkBits2Float(0xc1639578));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp149(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42216831), SkBits2Float(0xc2a60000), SkBits2Float(0x4295b6bc), SkBits2Float(0xc257ea44), SkBits2Float(0x42a38b52), SkBits2Float(0xc1639578));
path.lineTo(SkBits2Float(0x426c7311), SkBits2Float(0xc12484ba));
path.cubicTo(SkBits2Float(0x42587424), SkBits2Float(0xc21c154e), SkBits2Float(0x41e95c08), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5e9be), SkBits2Float(0xc02be63f));
path.cubicTo(SkBits2Float(0x42a7ff8e), SkBits2Float(0x41ec1faa), SkBits2Float(0x42849fff), SkBits2Float(0x426da4e1), SkBits2Float(0x4216595b), SkBits2Float(0x429400af));
path.cubicTo(SkBits2Float(0x410dcade), SkBits2Float(0x42b12eec), SkBits2Float(0xc1cdb135), SkBits2Float(0x42aa7b1c), SkBits2Float(0xc24c6646), SkBits2Float(0x4282cf52));
path.lineTo(SkBits2Float(0xc213c238), SkBits2Float(0x423d1f66));
path.cubicTo(SkBits2Float(0xc194b176), SkBits2Float(0x42767a79), SkBits2Float(0x40cd0045), SkBits2Float(0x42801597), SkBits2Float(0x41d95f44), SkBits2Float(0x4255fad4));
path.cubicTo(SkBits2Float(0x423fbf3c), SkBits2Float(0x422bca7a), SkBits2Float(0x4272e39a), SkBits2Float(0x41aab11f), SkBits2Float(0x426fdfd3), SkBits2Float(0xbff88758));
path.lineTo(SkBits2Float(0x42a5e9be), SkBits2Float(0xc02be63f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp150(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x422dab0f), SkBits2Float(0xc2a5ffff), SkBits2Float(0x429efeec), SkBits2Float(0xc2462810), SkBits2Float(0x42a58789), SkBits2Float(0xc0c7d837));
path.lineTo(SkBits2Float(0x426f51d5), SkBits2Float(0xc0907750));
path.cubicTo(SkBits2Float(0x4265df9a), SkBits2Float(0xc20f3ee4), SkBits2Float(0x41fb162c), SkBits2Float(0xc26ffffe), SkBits2Float(0x3637fea5), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a58789), SkBits2Float(0xc0c7d840));
path.cubicTo(SkBits2Float(0x42a626ff), SkBits2Float(0xc0078454), SkBits2Float(0x42a62824), SkBits2Float(0x4001c6d5), SkBits2Float(0x42a58af5), SkBits2Float(0x40c4fc3c));
path.lineTo(SkBits2Float(0x426f56ca), SkBits2Float(0x408e6626));
path.cubicTo(SkBits2Float(0x42703a0b), SkBits2Float(0x3fbba106), SkBits2Float(0x42703864), SkBits2Float(0xbfc3ed93), SkBits2Float(0x426f51d4), SkBits2Float(0xc090774f));
path.lineTo(SkBits2Float(0x42a58789), SkBits2Float(0xc0c7d840));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp151(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x422dab0f), SkBits2Float(0xc2a60000), SkBits2Float(0x429efeec), SkBits2Float(0xc2462811), SkBits2Float(0x42a58789), SkBits2Float(0xc0c7d840));
path.lineTo(SkBits2Float(0x42a58789), SkBits2Float(0xc0c7d837));
path.cubicTo(SkBits2Float(0x42a626ff), SkBits2Float(0xc0078448), SkBits2Float(0x42a62824), SkBits2Float(0x4001c6db), SkBits2Float(0x42a58af5), SkBits2Float(0x40c4fc3c));
path.lineTo(SkBits2Float(0x426f56ca), SkBits2Float(0x408e6626));
path.cubicTo(SkBits2Float(0x42703a0b), SkBits2Float(0x3fbba106), SkBits2Float(0x42703864), SkBits2Float(0xbfc3ed93), SkBits2Float(0x426f51d4), SkBits2Float(0xc090774f));
path.lineTo(SkBits2Float(0x426f51d5), SkBits2Float(0xc0907750));
path.cubicTo(SkBits2Float(0x4265df9a), SkBits2Float(0xc20f3ee4), SkBits2Float(0x41fb162c), SkBits2Float(0xc26ffffe), SkBits2Float(0x3637fea5), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a58af6), SkBits2Float(0x40c4fc3d));
path.cubicTo(SkBits2Float(0x42a06986), SkBits2Float(0x422298c3), SkBits2Float(0x42621341), SkBits2Float(0x428bdf10), SkBits2Float(0x41ba9762), SkBits2Float(0x429f4f99));
path.cubicTo(SkBits2Float(0xc11def80), SkBits2Float(0x42b2c022), SkBits2Float(0xc236745f), SkBits2Float(0x429afb1c), SkBits2Float(0xc284c1e2), SkBits2Float(0x4247504a));
path.lineTo(SkBits2Float(0xc23ff038), SkBits2Float(0x42101509));
path.cubicTo(SkBits2Float(0xc203e517), SkBits2Float(0x4260119e), SkBits2Float(0xc0e45731), SkBits2Float(0x428137a0), SkBits2Float(0x4186e2a5), SkBits2Float(0x42665443));
path.cubicTo(SkBits2Float(0x42236d8c), SkBits2Float(0x424a3945), SkBits2Float(0x4267ebda), SkBits2Float(0x41eb1462), SkBits2Float(0x426f56cb), SkBits2Float(0x408e661a));
path.lineTo(SkBits2Float(0x42a58af6), SkBits2Float(0x40c4fc3d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp152(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41b12ed4), SkBits2Float(0xc2a60000), SkBits2Float(0x422d822c), SkBits2Float(0xc2944bde), SkBits2Float(0x426bdb91), SkBits2Float(0xc269a7f3));
path.cubicTo(SkBits2Float(0x42951a7b), SkBits2Float(0xc22ab829), SkBits2Float(0x42a66879), SkBits2Float(0xc1aaf2b1), SkBits2Float(0x42a5fe21), SkBits2Float(0x3f4744a4));
path.lineTo(SkBits2Float(0x426ffd4c), SkBits2Float(0x3f100c99));
path.cubicTo(SkBits2Float(0x4270970c), SkBits2Float(0xc177275d), SkBits2Float(0x4257923d), SkBits2Float(0xc1f6d2bd), SkBits2Float(0x422a7fe2), SkBits2Float(0xc228e872));
path.cubicTo(SkBits2Float(0x41fadb0b), SkBits2Float(0xc2566785), SkBits2Float(0x41801584), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5fe22), SkBits2Float(0x3f4744a1));
path.cubicTo(SkBits2Float(0x42a5e921), SkBits2Float(0x40a4df91), SkBits2Float(0x42a52322), SkBits2Float(0x411841f7), SkBits2Float(0x42a3adfe), SkBits2Float(0x415d43d0));
path.lineTo(SkBits2Float(0x426ca531), SkBits2Float(0x411ff355));
path.cubicTo(SkBits2Float(0x426ec0ad), SkBits2Float(0x40dc21ae), SkBits2Float(0x426fdeef), SkBits2Float(0x406e5efe), SkBits2Float(0x426ffd4d), SkBits2Float(0x3f100c9b));
path.lineTo(SkBits2Float(0x42a5fe22), SkBits2Float(0x3f4744a1));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp153(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41b12ed4), SkBits2Float(0xc2a60000), SkBits2Float(0x422d822c), SkBits2Float(0xc2944bde), SkBits2Float(0x426bdb91), SkBits2Float(0xc269a7f3));
path.cubicTo(SkBits2Float(0x42951a7b), SkBits2Float(0xc22ab829), SkBits2Float(0x42a66879), SkBits2Float(0xc1aaf2b1), SkBits2Float(0x42a5fe21), SkBits2Float(0x3f4744a0));
path.lineTo(SkBits2Float(0x426ffd4c), SkBits2Float(0x3f100c99));
path.cubicTo(SkBits2Float(0x4270970c), SkBits2Float(0xc177275d), SkBits2Float(0x4257923d), SkBits2Float(0xc1f6d2bd), SkBits2Float(0x422a7fe2), SkBits2Float(0xc228e872));
path.cubicTo(SkBits2Float(0x41fadb0b), SkBits2Float(0xc2566785), SkBits2Float(0x41801584), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a3adfe), SkBits2Float(0x415d43d0));
path.cubicTo(SkBits2Float(0x42977493), SkBits2Float(0x42480062), SkBits2Float(0x423a617c), SkBits2Float(0x429bbd03), SkBits2Float(0x4123044a), SkBits2Float(0x42a4be9a));
path.cubicTo(SkBits2Float(0xc1d1beaf), SkBits2Float(0x42adc030), SkBits2Float(0xc2750d30), SkBits2Float(0x4285e3a3), SkBits2Float(0xc2980208), SkBits2Float(0x42056911));
path.lineTo(SkBits2Float(0xc25bc541), SkBits2Float(0x41c0e1ed));
path.cubicTo(SkBits2Float(0xc231254e), SkBits2Float(0x42419328), SkBits2Float(0xc1979f72), SkBits2Float(0x427b34be), SkBits2Float(0x40ebafde), SkBits2Float(0x426e2f5c));
path.cubicTo(SkBits2Float(0x4206bbb1), SkBits2Float(0x426129fa), SkBits2Float(0x425af8c2), SkBits2Float(0x42109457), SkBits2Float(0x426ca533), SkBits2Float(0x411ff35b));
path.lineTo(SkBits2Float(0x42a3adfe), SkBits2Float(0x415d43d0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp154(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41bb5603), SkBits2Float(0xc2a60000), SkBits2Float(0x4236fa4e), SkBits2Float(0xc2923760), SkBits2Float(0x4275e892), SkBits2Float(0xc25f0dc8));
path.cubicTo(SkBits2Float(0x429a6b6b), SkBits2Float(0xc219acd0), SkBits2Float(0x42a9c473), SkBits2Float(0xc173c3a6), SkBits2Float(0x42a5369d), SkBits2Float(0x410121d8));
path.lineTo(SkBits2Float(0x426edcd8), SkBits2Float(0x40bab276));
path.cubicTo(SkBits2Float(0x42757264), SkBits2Float(0xc1303715), SkBits2Float(0x425f41dd), SkBits2Float(0xc1de2e4a), SkBits2Float(0x4231c3e2), SkBits2Float(0xc2213e66));
path.cubicTo(SkBits2Float(0x420445e8), SkBits2Float(0xc25365a8), SkBits2Float(0x41876c72), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5369e), SkBits2Float(0x410121d6));
path.cubicTo(SkBits2Float(0x42a450b5), SkBits2Float(0x414aab85), SkBits2Float(0x42a2a6cd), SkBits2Float(0x4189bd6e), SkBits2Float(0x42a03d57), SkBits2Float(0x41ad66e6));
path.lineTo(SkBits2Float(0x4267abf7), SkBits2Float(0x417ab39f));
path.cubicTo(SkBits2Float(0x426b28ae), SkBits2Float(0x41472463), SkBits2Float(0x426d9071), SkBits2Float(0x41128229), SkBits2Float(0x426edcd8), SkBits2Float(0x40bab277));
path.lineTo(SkBits2Float(0x42a5369e), SkBits2Float(0x410121d6));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp155(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41bb5603), SkBits2Float(0xc2a60000), SkBits2Float(0x4236fa4e), SkBits2Float(0xc2923760), SkBits2Float(0x4275e892), SkBits2Float(0xc25f0dc8));
path.cubicTo(SkBits2Float(0x429a6b6b), SkBits2Float(0xc219acd0), SkBits2Float(0x42a9c473), SkBits2Float(0xc173c3a8), SkBits2Float(0x42a5369d), SkBits2Float(0x410121d5));
path.lineTo(SkBits2Float(0x42a5369e), SkBits2Float(0x410121d6));
path.cubicTo(SkBits2Float(0x42a450b5), SkBits2Float(0x414aab85), SkBits2Float(0x42a2a6cd), SkBits2Float(0x4189bd6e), SkBits2Float(0x42a03d57), SkBits2Float(0x41ad66e6));
path.lineTo(SkBits2Float(0x4267abf7), SkBits2Float(0x417ab39f));
path.cubicTo(SkBits2Float(0x426b28ae), SkBits2Float(0x41472463), SkBits2Float(0x426d9071), SkBits2Float(0x41128229), SkBits2Float(0x426edcd8), SkBits2Float(0x40bab276));
path.cubicTo(SkBits2Float(0x42757264), SkBits2Float(0xc1303715), SkBits2Float(0x425f41dd), SkBits2Float(0xc1de2e4a), SkBits2Float(0x4231c3e2), SkBits2Float(0xc2213e66));
path.cubicTo(SkBits2Float(0x420445e8), SkBits2Float(0xc25365a8), SkBits2Float(0x41876c72), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a03d58), SkBits2Float(0x41ad66e7));
path.cubicTo(SkBits2Float(0x428bedd4), SkBits2Float(0x426cda0a), SkBits2Float(0x420c6f35), SkBits2Float(0x42a955c4), SkBits2Float(0xc06f4c79), SkBits2Float(0x42a5d4d6));
path.cubicTo(SkBits2Float(0xc22a58c2), SkBits2Float(0x42a253e8), SkBits2Float(0xc2960525), SkBits2Float(0x4252b394), SkBits2Float(0xc2a37db3), SkBits2Float(0x41660422));
path.lineTo(SkBits2Float(0xc26c5f63), SkBits2Float(0x412646cf));
path.cubicTo(SkBits2Float(0xc258e58a), SkBits2Float(0x4218507a), SkBits2Float(0xc1f648da), SkBits2Float(0x426ab0dc), SkBits2Float(0xc02cfcc3), SkBits2Float(0x426fc1a0));
path.cubicTo(SkBits2Float(0x41cb09aa), SkBits2Float(0x4274d265), SkBits2Float(0x424a4e9e), SkBits2Float(0x422b37da), SkBits2Float(0x4267abf8), SkBits2Float(0x417ab398));
path.lineTo(SkBits2Float(0x42a03d58), SkBits2Float(0x41ad66e7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp156(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c3ae1a), SkBits2Float(0xc2a60000), SkBits2Float(0x423eb2d3), SkBits2Float(0xc2906c00), SkBits2Float(0x427dc7c2), SkBits2Float(0xc2560e13));
path.cubicTo(SkBits2Float(0x429e6e58), SkBits2Float(0xc20b4426), SkBits2Float(0x42abdf2b), SkBits2Float(0xc121d7a7), SkBits2Float(0x42a39f93), SkBits2Float(0x415fea21));
path.lineTo(SkBits2Float(0x426c905a), SkBits2Float(0x4121ddae));
path.cubicTo(SkBits2Float(0x42787d42), SkBits2Float(0xc0e9fd34), SkBits2Float(0x42650e94), SkBits2Float(0xc1c95949), SkBits2Float(0x423774a6), SkBits2Float(0xc21abd13));
path.cubicTo(SkBits2Float(0x4209dab9), SkBits2Float(0xc250cd81), SkBits2Float(0x418d749b), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a39f93), SkBits2Float(0x415fea20));
path.cubicTo(SkBits2Float(0x42a1ffad), SkBits2Float(0x4195f252), SkBits2Float(0x429f8ce1), SkBits2Float(0x41bb4c45), SkBits2Float(0x429c4e4c), SkBits2Float(0x41df969a));
path.lineTo(SkBits2Float(0x4261fbff), SkBits2Float(0x41a1a14e));
path.cubicTo(SkBits2Float(0x4266acd9), SkBits2Float(0x41876566), SkBits2Float(0x426a370e), SkBits2Float(0x4158ca4c), SkBits2Float(0x426c905b), SkBits2Float(0x4121ddaf));
path.lineTo(SkBits2Float(0x42a39f93), SkBits2Float(0x415fea20));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp157(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c3ae1a), SkBits2Float(0xc2a60000), SkBits2Float(0x423eb2d3), SkBits2Float(0xc2906c00), SkBits2Float(0x427dc7c2), SkBits2Float(0xc2560e13));
path.cubicTo(SkBits2Float(0x429e6e58), SkBits2Float(0xc20b4426), SkBits2Float(0x42abdf2b), SkBits2Float(0xc121d7a8), SkBits2Float(0x42a39f93), SkBits2Float(0x415fea20));
path.lineTo(SkBits2Float(0x42a39f93), SkBits2Float(0x415fea21));
path.cubicTo(SkBits2Float(0x42a1ffad), SkBits2Float(0x4195f252), SkBits2Float(0x429f8ce1), SkBits2Float(0x41bb4c45), SkBits2Float(0x429c4e4c), SkBits2Float(0x41df969a));
path.lineTo(SkBits2Float(0x4261fbff), SkBits2Float(0x41a1a14e));
path.cubicTo(SkBits2Float(0x4266acd9), SkBits2Float(0x41876566), SkBits2Float(0x426a370e), SkBits2Float(0x4158ca4c), SkBits2Float(0x426c905b), SkBits2Float(0x4121ddaf));
path.lineTo(SkBits2Float(0x426c905a), SkBits2Float(0x4121ddae));
path.cubicTo(SkBits2Float(0x42787d42), SkBits2Float(0xc0e9fd34), SkBits2Float(0x42650e94), SkBits2Float(0xc1c95949), SkBits2Float(0x423774a6), SkBits2Float(0xc21abd13));
path.cubicTo(SkBits2Float(0x4209dab9), SkBits2Float(0xc250cd81), SkBits2Float(0x418d749b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429c4e4c), SkBits2Float(0x41df969b));
path.cubicTo(SkBits2Float(0x4280e391), SkBits2Float(0x4284903f), SkBits2Float(0x41c7a851), SkBits2Float(0x42b2072e), SkBits2Float(0xc1713833), SkBits2Float(0x42a33d14));
path.cubicTo(SkBits2Float(0xc25c7040), SkBits2Float(0x429472fb), SkBits2Float(0xc2a7bda2), SkBits2Float(0x421b8b2e), SkBits2Float(0xc2a5f5d6), SkBits2Float(0xbfe85110));
path.lineTo(SkBits2Float(0xc26ff14f), SkBits2Float(0xbfa7f00b));
path.cubicTo(SkBits2Float(0xc272844c), SkBits2Float(0x41e0e1f3), SkBits2Float(0xc21f5a65), SkBits2Float(0x4256a019), SkBits2Float(0xc12e6015), SkBits2Float(0x426c01f9));
path.cubicTo(SkBits2Float(0x419054b7), SkBits2Float(0x4280b1ec), SkBits2Float(0x423a5877), SkBits2Float(0x423fa872), SkBits2Float(0x4261fc02), SkBits2Float(0x41a1a142));
path.lineTo(SkBits2Float(0x429c4e4c), SkBits2Float(0x41df969b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp158(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41cb677f), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4245cb36), SkBits2Float(0xc28eb15b), SkBits2Float(0x42825fc2), SkBits2Float(0xc24d8299));
path.cubicTo(SkBits2Float(0x42a1d9e8), SkBits2Float(0xc1fb44f8), SkBits2Float(0x42ad4967), SkBits2Float(0xc0aa7cf8), SkBits2Float(0x42a1679f), SkBits2Float(0x419b26cf));
path.lineTo(SkBits2Float(0x42695b36), SkBits2Float(0x416050ca));
path.cubicTo(SkBits2Float(0x427a88f8), SkBits2Float(0xc0767d2a), SkBits2Float(0x426a0074), SkBits2Float(0xc1b5a3f9), SkBits2Float(0x423c7e1d), SkBits2Float(0xc2148fc2));
path.cubicTo(SkBits2Float(0x420efbc6), SkBits2Float(0xc24e4d87), SkBits2Float(0x41930a0e), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a1679f), SkBits2Float(0x419b26d0));
path.cubicTo(SkBits2Float(0x429f113c), SkBits2Float(0x41c20ede), SkBits2Float(0x429bdafe), SkBits2Float(0x41e80a2e), SkBits2Float(0x4297ceee), SkBits2Float(0x42065107));
path.lineTo(SkBits2Float(0x425b7b5f), SkBits2Float(0x41c2314a));
path.cubicTo(SkBits2Float(0x4261554b), SkBits2Float(0x41a7bd56), SkBits2Float(0x4265fa14), SkBits2Float(0x418c4870), SkBits2Float(0x42695b37), SkBits2Float(0x416050cb));
path.lineTo(SkBits2Float(0x42a1679f), SkBits2Float(0x419b26d0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp159(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41cb677f), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4245cb36), SkBits2Float(0xc28eb15b), SkBits2Float(0x42825fc2), SkBits2Float(0xc24d8299));
path.cubicTo(SkBits2Float(0x42a1d9e8), SkBits2Float(0xc1fb44f8), SkBits2Float(0x42ad4967), SkBits2Float(0xc0aa7cf8), SkBits2Float(0x42a1679f), SkBits2Float(0x419b26d0));
path.cubicTo(SkBits2Float(0x429f113c), SkBits2Float(0x41c20ede), SkBits2Float(0x429bdafe), SkBits2Float(0x41e80a2e), SkBits2Float(0x4297ceee), SkBits2Float(0x42065107));
path.lineTo(SkBits2Float(0x425b7b5f), SkBits2Float(0x41c2314a));
path.cubicTo(SkBits2Float(0x4261554b), SkBits2Float(0x41a7bd56), SkBits2Float(0x4265fa14), SkBits2Float(0x418c4870), SkBits2Float(0x42695b36), SkBits2Float(0x416050ca));
path.cubicTo(SkBits2Float(0x427a88f8), SkBits2Float(0xc0767d2a), SkBits2Float(0x426a0074), SkBits2Float(0xc1b5a3f9), SkBits2Float(0x423c7e1d), SkBits2Float(0xc2148fc2));
path.cubicTo(SkBits2Float(0x420efbc6), SkBits2Float(0xc24e4d87), SkBits2Float(0x41930a0e), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4297ceef), SkBits2Float(0x42065107));
path.cubicTo(SkBits2Float(0x426afc81), SkBits2Float(0x4290b9e3), SkBits2Float(0x4171c53f), SkBits2Float(0x42b7f2c1), SkBits2Float(0xc1ca446b), SkBits2Float(0x429e1c54));
path.cubicTo(SkBits2Float(0xc2835add), SkBits2Float(0x428445e8), SkBits2Float(0xc2b3ab9e), SkBits2Float(0x41c6c009), SkBits2Float(0xc2a29b10), SkBits2Float(0xc18596e4));
path.lineTo(SkBits2Float(0xc26b17b4), SkBits2Float(0xc141242b));
path.cubicTo(SkBits2Float(0xc281e1de), SkBits2Float(0x418faccb), SkBits2Float(0xc23de932), SkBits2Float(0x423f3d09), SkBits2Float(0xc19237aa), SkBits2Float(0x42649810));
path.cubicTo(SkBits2Float(0x412ec628), SkBits2Float(0x4284f98c), SkBits2Float(0x4229deab), SkBits2Float(0x42513e23), SkBits2Float(0x425b7b62), SkBits2Float(0x41c23147));
path.lineTo(SkBits2Float(0x4297ceef), SkBits2Float(0x42065107));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp160(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d3ccce), SkBits2Float(0xc2a5ffff), SkBits2Float(0x424d7252), SkBits2Float(0xc28cbd55), SkBits2Float(0x4285fbcc), SkBits2Float(0xc244010c));
path.cubicTo(SkBits2Float(0x42a53e6e), SkBits2Float(0xc1dd0edd), SkBits2Float(0x42ae3d82), SkBits2Float(0xbdb630d0), SkBits2Float(0x429e3366), SkBits2Float(0x41c92323));
path.lineTo(SkBits2Float(0x4264b95a), SkBits2Float(0x41916681));
path.cubicTo(SkBits2Float(0x427be9e4), SkBits2Float(0xbd83b620), SkBits2Float(0x426ee823), SkBits2Float(0xc19fcd11), SkBits2Float(0x4241b610), SkBits2Float(0xc20db091));
path.cubicTo(SkBits2Float(0x421483fd), SkBits2Float(0xc24b7a9a), SkBits2Float(0x41991bc1), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429e3367), SkBits2Float(0x41c92322));
path.cubicTo(SkBits2Float(0x429b0cbc), SkBits2Float(0x41f0ca9b), SkBits2Float(0x4296f94f), SkBits2Float(0x420b9629), SkBits2Float(0x429206e2), SkBits2Float(0x421de34f));
path.lineTo(SkBits2Float(0x42531f8a), SkBits2Float(0x41e4458f));
path.cubicTo(SkBits2Float(0x425a4685), SkBits2Float(0x41c9cfd9), SkBits2Float(0x42602b18), SkBits2Float(0x41ae10ed), SkBits2Float(0x4264b95a), SkBits2Float(0x41916682));
path.lineTo(SkBits2Float(0x429e3367), SkBits2Float(0x41c92322));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp161(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d3ccce), SkBits2Float(0xc2a5ffff), SkBits2Float(0x424d7252), SkBits2Float(0xc28cbd55), SkBits2Float(0x4285fbcc), SkBits2Float(0xc244010c));
path.cubicTo(SkBits2Float(0x42a53e6e), SkBits2Float(0xc1dd0edd), SkBits2Float(0x42ae3d82), SkBits2Float(0xbdb630d0), SkBits2Float(0x429e3367), SkBits2Float(0x41c92322));
path.cubicTo(SkBits2Float(0x429b0cbc), SkBits2Float(0x41f0ca9b), SkBits2Float(0x4296f94f), SkBits2Float(0x420b9629), SkBits2Float(0x429206e2), SkBits2Float(0x421de34f));
path.lineTo(SkBits2Float(0x42531f8a), SkBits2Float(0x41e4458f));
path.cubicTo(SkBits2Float(0x425a4685), SkBits2Float(0x41c9cfd9), SkBits2Float(0x42602b18), SkBits2Float(0x41ae10ed), SkBits2Float(0x4264b95a), SkBits2Float(0x41916681));
path.cubicTo(SkBits2Float(0x427be9e4), SkBits2Float(0xbd83b620), SkBits2Float(0x426ee823), SkBits2Float(0xc19fcd11), SkBits2Float(0x4241b610), SkBits2Float(0xc20db091));
path.cubicTo(SkBits2Float(0x421483fd), SkBits2Float(0xc24b7a9a), SkBits2Float(0x41991bc1), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429206e2), SkBits2Float(0x421de34f));
path.cubicTo(SkBits2Float(0x424fd7be), SkBits2Float(0x429cd433), SkBits2Float(0x40819da9), SkBits2Float(0x42bbf605), SkBits2Float(0xc20f7b98), SkBits2Float(0x4295b271));
path.cubicTo(SkBits2Float(0xc2979573), SkBits2Float(0x425eddba), SkBits2Float(0xc2bb57fe), SkBits2Float(0x4109ef62), SkBits2Float(0xc2990315), SkBits2Float(0xc200bcbb));
path.lineTo(SkBits2Float(0xc25d38e3), SkBits2Float(0xc1ba2048));
path.cubicTo(SkBits2Float(0xc2876de1), SkBits2Float(0x40c76c9c), SkBits2Float(0xc25b2842), SkBits2Float(0x42211baa), SkBits2Float(0xc1cf71e5), SkBits2Float(0x42586df1));
path.cubicTo(SkBits2Float(0x403b65b7), SkBits2Float(0x4287e01c), SkBits2Float(0x42163f6f), SkBits2Float(0x4262bd95), SkBits2Float(0x42531f8c), SkBits2Float(0x41e4458b));
path.lineTo(SkBits2Float(0x429206e2), SkBits2Float(0x421de34f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp162(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41da3d7f), SkBits2Float(0xc2a60000), SkBits2Float(0x425345ee), SkBits2Float(0xc28b3082), SkBits2Float(0x4288a01b), SkBits2Float(0xc23c9177));
path.cubicTo(SkBits2Float(0x42a79d3f), SkBits2Float(0xc1c583d9), SkBits2Float(0x42ae8eeb), SkBits2Float(0x407c6461), SkBits2Float(0x429b333a), SkBits2Float(0x41eb9731));
path.lineTo(SkBits2Float(0x426062bb), SkBits2Float(0x41aa4e75));
path.cubicTo(SkBits2Float(0x427c5f9a), SkBits2Float(0x403673d5), SkBits2Float(0x4272557b), SkBits2Float(0xc18ec82c), SkBits2Float(0x424587e0), SkBits2Float(0xc208507b));
path.cubicTo(SkBits2Float(0x4218ba46), SkBits2Float(0xc2493ce1), SkBits2Float(0x419dc399), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429b3339), SkBits2Float(0x41eb9733));
path.cubicTo(SkBits2Float(0x429766b3), SkBits2Float(0x4209d0f3), SkBits2Float(0x4292a485), SkBits2Float(0x421d0e17), SkBits2Float(0x428cfdb5), SkBits2Float(0x422f3e33));
path.lineTo(SkBits2Float(0x424bd7ac), SkBits2Float(0x41fd5d06));
path.cubicTo(SkBits2Float(0x42540374), SkBits2Float(0x41e3114e), SkBits2Float(0x425ae4ae), SkBits2Float(0x41c7409b), SkBits2Float(0x426062bc), SkBits2Float(0x41aa4e76));
path.lineTo(SkBits2Float(0x429b3339), SkBits2Float(0x41eb9733));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp163(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41da3d7f), SkBits2Float(0xc2a60000), SkBits2Float(0x425345ee), SkBits2Float(0xc28b3082), SkBits2Float(0x4288a01b), SkBits2Float(0xc23c9177));
path.cubicTo(SkBits2Float(0x42a79d3f), SkBits2Float(0xc1c583d9), SkBits2Float(0x42ae8eeb), SkBits2Float(0x407c6461), SkBits2Float(0x429b3339), SkBits2Float(0x41eb9733));
path.cubicTo(SkBits2Float(0x429766b3), SkBits2Float(0x4209d0f3), SkBits2Float(0x4292a485), SkBits2Float(0x421d0e17), SkBits2Float(0x428cfdb5), SkBits2Float(0x422f3e33));
path.lineTo(SkBits2Float(0x424bd7ac), SkBits2Float(0x41fd5d06));
path.cubicTo(SkBits2Float(0x42540374), SkBits2Float(0x41e3114e), SkBits2Float(0x425ae4ae), SkBits2Float(0x41c7409b), SkBits2Float(0x426062bb), SkBits2Float(0x41aa4e75));
path.cubicTo(SkBits2Float(0x427c5f9a), SkBits2Float(0x403673d5), SkBits2Float(0x4272557b), SkBits2Float(0xc18ec82c), SkBits2Float(0x424587e0), SkBits2Float(0xc208507b));
path.cubicTo(SkBits2Float(0x4218ba46), SkBits2Float(0xc2493ce1), SkBits2Float(0x419dc399), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428cfdb5), SkBits2Float(0x422f3e36));
path.cubicTo(SkBits2Float(0x42397b9c), SkBits2Float(0x42a54202), SkBits2Float(0xc0931849), SkBits2Float(0x42bd474f), SkBits2Float(0xc22e0fe8), SkBits2Float(0x428d5ab7));
path.cubicTo(SkBits2Float(0xc2a4de63), SkBits2Float(0x423adc3f), SkBits2Float(0xc2bd50df), SkBits2Float(0xc08673c0), SkBits2Float(0xc28db7cd), SkBits2Float(0xc22ce1b4));
path.lineTo(SkBits2Float(0xc24ce4bb), SkBits2Float(0xc1f9f306));
path.cubicTo(SkBits2Float(0xc288db72), SkBits2Float(0xc0426216), SkBits2Float(0xc26e5ec8), SkBits2Float(0x42071590), SkBits2Float(0xc1fba9c9), SkBits2Float(0x424c5fa5));
path.cubicTo(SkBits2Float(0xc054b001), SkBits2Float(0x4288d4dc), SkBits2Float(0x420615fc), SkBits2Float(0x426eee67), SkBits2Float(0x424bd7af), SkBits2Float(0x41fd5d01));
path.lineTo(SkBits2Float(0x428cfdb5), SkBits2Float(0x422f3e36));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp164(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41e183ec), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4259cec4), SkBits2Float(0xc2896274), SkBits2Float(0x428b79bc), SkBits2Float(0xc2340753));
path.cubicTo(SkBits2Float(0x42aa0c16), SkBits2Float(0xc1aa937d), SkBits2Float(0x42ae7c71), SkBits2Float(0x41080a55), SkBits2Float(0x42974339), SkBits2Float(0x4208c1d5));
path.lineTo(SkBits2Float(0x425ab161), SkBits2Float(0x41c5b8a2));
path.cubicTo(SkBits2Float(0x427c44e4), SkBits2Float(0x40c4af5a), SkBits2Float(0x4275d9f7), SkBits2Float(0xc1769dba), SkBits2Float(0x4249a6c2), SkBits2Float(0xc2022424));
path.cubicTo(SkBits2Float(0x421d738b), SkBits2Float(0xc246a0db), SkBits2Float(0x41a305f1), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42974339), SkBits2Float(0x4208c1d6));
path.cubicTo(SkBits2Float(0x4292b5f8), SkBits2Float(0x421ce537), SkBits2Float(0x428d2a3f), SkBits2Float(0x42301305), SkBits2Float(0x4286b52e), SkBits2Float(0x4242022c));
path.lineTo(SkBits2Float(0x4242c218), SkBits2Float(0x420c3f43));
path.cubicTo(SkBits2Float(0x424c1813), SkBits2Float(0x41fe90b7), SkBits2Float(0x42541cae), SkBits2Float(0x41e2d634), SkBits2Float(0x425ab162), SkBits2Float(0x41c5b8a3));
path.lineTo(SkBits2Float(0x42974339), SkBits2Float(0x4208c1d6));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp165(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41e183ec), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4259cec4), SkBits2Float(0xc2896274), SkBits2Float(0x428b79bc), SkBits2Float(0xc2340753));
path.cubicTo(SkBits2Float(0x42aa0c16), SkBits2Float(0xc1aa937d), SkBits2Float(0x42ae7c71), SkBits2Float(0x41080a55), SkBits2Float(0x42974339), SkBits2Float(0x4208c1d6));
path.cubicTo(SkBits2Float(0x4292b5f8), SkBits2Float(0x421ce537), SkBits2Float(0x428d2a3f), SkBits2Float(0x42301305), SkBits2Float(0x4286b52e), SkBits2Float(0x4242022c));
path.lineTo(SkBits2Float(0x4242c218), SkBits2Float(0x420c3f43));
path.cubicTo(SkBits2Float(0x424c1813), SkBits2Float(0x41fe90b7), SkBits2Float(0x42541cae), SkBits2Float(0x41e2d634), SkBits2Float(0x425ab161), SkBits2Float(0x41c5b8a2));
path.cubicTo(SkBits2Float(0x427c44e4), SkBits2Float(0x40c4af5a), SkBits2Float(0x4275d9f7), SkBits2Float(0xc1769dba), SkBits2Float(0x4249a6c2), SkBits2Float(0xc2022424));
path.cubicTo(SkBits2Float(0x421d738b), SkBits2Float(0xc246a0db), SkBits2Float(0x41a305f1), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4286b52e), SkBits2Float(0x4242022d));
path.cubicTo(SkBits2Float(0x4245f9c6), SkBits2Float(0x42929b97), SkBits2Float(0x419b96e9), SkBits2Float(0x42ac9135), SkBits2Float(0xc12da222), SkBits2Float(0x42a4933a));
path.cubicTo(SkBits2Float(0xc2249c85), SkBits2Float(0x429c9540), SkBits2Float(0xc2859c99), SkBits2Float(0x4267dd85), SkBits2Float(0xc29b4028), SkBits2Float(0x41eb0f05));
path.cubicTo(SkBits2Float(0xc2b0e3b8), SkBits2Float(0x3f4c608a), SkBits2Float(0xc2a55c16), SkBits2Float(0xc1fb5a07), SkBits2Float(0xc27a7a78), SkBits2Float(0xc259e8d8));
path.lineTo(SkBits2Float(0xc2351199), SkBits2Float(0xc21d8664));
path.cubicTo(SkBits2Float(0xc26f12eb), SkBits2Float(0xc1b5b32d), SkBits2Float(0xc27fbe43), SkBits2Float(0x3f13bb74), SkBits2Float(0xc2607541), SkBits2Float(0x41a9ebcd));
path.cubicTo(SkBits2Float(0xc2412c3e), SkBits2Float(0x42279ce1), SkBits2Float(0xc1edfdc7), SkBits2Float(0x4262625e), SkBits2Float(0xc0fb089d), SkBits2Float(0x426df06d));
path.cubicTo(SkBits2Float(0x4160f2f1), SkBits2Float(0x42797e7c), SkBits2Float(0x420f1d6a), SkBits2Float(0x4253f671), SkBits2Float(0x4242c21c), SkBits2Float(0x420c3f41));
path.lineTo(SkBits2Float(0x4286b52e), SkBits2Float(0x4242022d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp166(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41e5cd16), SkBits2Float(0xc2a60000), SkBits2Float(0x425da203), SkBits2Float(0xc2884b73), SkBits2Float(0x428d165b), SkBits2Float(0xc22eeec9));
path.cubicTo(SkBits2Float(0x42ab5bb4), SkBits2Float(0xc19a8d5b), SkBits2Float(0x42ae3add), SkBits2Float(0x4132f7c2), SkBits2Float(0x4294adf4), SkBits2Float(0x4213a75b));
path.lineTo(SkBits2Float(0x4256f554), SkBits2Float(0x41d579ab));
path.cubicTo(SkBits2Float(0x427be612), SkBits2Float(0x41015fcf), SkBits2Float(0x4277bf2e), SkBits2Float(0xc15f72f6), SkBits2Float(0x424bfb4d), SkBits2Float(0xc1fcea38));
path.cubicTo(SkBits2Float(0x4220376c), SkBits2Float(0xc2450d7a), SkBits2Float(0x41a61f08), SkBits2Float(0xc2700000), SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4294adf4), SkBits2Float(0x4213a75b));
path.cubicTo(SkBits2Float(0x428facea), SkBits2Float(0x4227cf1b), SkBits2Float(0x4289a8e5), SkBits2Float(0x423ae500), SkBits2Float(0x4282b9a7), SkBits2Float(0x424c9dab));
path.lineTo(SkBits2Float(0x423d0015), SkBits2Float(0x4213ea45));
path.cubicTo(SkBits2Float(0x424706b3), SkBits2Float(0x42071ac0), SkBits2Float(0x424fb93a), SkBits2Float(0x41f29d8f), SkBits2Float(0x4256f555), SkBits2Float(0x41d579ac));
path.lineTo(SkBits2Float(0x4294adf4), SkBits2Float(0x4213a75b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp167(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41e5cd16), SkBits2Float(0xc2a60000), SkBits2Float(0x425da203), SkBits2Float(0xc2884b73), SkBits2Float(0x428d165b), SkBits2Float(0xc22eeec9));
path.cubicTo(SkBits2Float(0x42ab5bb4), SkBits2Float(0xc19a8d5b), SkBits2Float(0x42ae3add), SkBits2Float(0x4132f7c2), SkBits2Float(0x4294adf4), SkBits2Float(0x4213a75b));
path.cubicTo(SkBits2Float(0x428facea), SkBits2Float(0x4227cf1b), SkBits2Float(0x4289a8e5), SkBits2Float(0x423ae500), SkBits2Float(0x4282b9a7), SkBits2Float(0x424c9dab));
path.lineTo(SkBits2Float(0x423d0015), SkBits2Float(0x4213ea45));
path.cubicTo(SkBits2Float(0x424706b3), SkBits2Float(0x42071ac0), SkBits2Float(0x424fb93a), SkBits2Float(0x41f29d8f), SkBits2Float(0x4256f554), SkBits2Float(0x41d579ab));
path.cubicTo(SkBits2Float(0x427be612), SkBits2Float(0x41015fcf), SkBits2Float(0x4277bf2e), SkBits2Float(0xc15f72f6), SkBits2Float(0x424bfb4d), SkBits2Float(0xc1fcea38));
path.cubicTo(SkBits2Float(0x4220376c), SkBits2Float(0xc2450d7a), SkBits2Float(0x41a61f08), SkBits2Float(0xc2700000), SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4282b9a8), SkBits2Float(0x424c9dac));
path.cubicTo(SkBits2Float(0x4238a98e), SkBits2Float(0x42975dcd), SkBits2Float(0x416d9db4), SkBits2Float(0x42aecc7f), SkBits2Float(0xc17bb856), SkBits2Float(0x42a2fd9a));
path.cubicTo(SkBits2Float(0xc2394396), SkBits2Float(0x42972eb6), SkBits2Float(0xc28e09e8), SkBits2Float(0x42543e5a), SkBits2Float(0xc29f69c3), SkBits2Float(0x41b9307a));
path.cubicTo(SkBits2Float(0xc2b0c99f), SkBits2Float(0xc0d86efe), SkBits2Float(0xc29f345f), SkBits2Float(0xc21c161b), SkBits2Float(0xc263c1d4), SkBits2Float(0xc2718f13));
path.lineTo(SkBits2Float(0xc224a4cd), SkBits2Float(0xc22e9eef));
path.cubicTo(SkBits2Float(0xc2662cd7), SkBits2Float(0xc1e1aab7), SkBits2Float(0xc27f98a3), SkBits2Float(0xc09c754c), SkBits2Float(0xc26679fe), SkBits2Float(0x4185df20));
path.cubicTo(SkBits2Float(0xc24d5b58), SkBits2Float(0x42196dcb), SkBits2Float(0xc205ecef), SkBits2Float(0x425a93a6), SkBits2Float(0xc135f72f), SkBits2Float(0x426ba619));
path.cubicTo(SkBits2Float(0x412bc560), SkBits2Float(0x427cb88a), SkBits2Float(0x42057da8), SkBits2Float(0x425ad7c5), SkBits2Float(0x423d0018), SkBits2Float(0x4213ea45));
path.lineTo(SkBits2Float(0x4282b9a8), SkBits2Float(0x424c9dac));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp168(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ea54b9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4261a7de), SkBits2Float(0xc2871f16), SkBits2Float(0x428ebc81), SkBits2Float(0xc2297f4d));
path.cubicTo(SkBits2Float(0x42aca513), SkBits2Float(0xc18980da), SkBits2Float(0x42adc9a4), SkBits2Float(0x41604127), SkBits2Float(0x4291be57), SkBits2Float(0x421eee87));
path.lineTo(SkBits2Float(0x4252b6a9), SkBits2Float(0x41e5c7e9));
path.cubicTo(SkBits2Float(0x427b4260), SkBits2Float(0x41221c9f), SkBits2Float(0x42799b62), SkBits2Float(0xc146ccc2), SkBits2Float(0x424e5da6), SkBits2Float(0xc1f50e65));
path.cubicTo(SkBits2Float(0x42231fea), SkBits2Float(0xc2435b34), SkBits2Float(0x41a9655c), SkBits2Float(0xc26ffffe), SkBits2Float(0x3725ffa9), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4291be57), SkBits2Float(0x421eee8a));
path.cubicTo(SkBits2Float(0x428c4169), SkBits2Float(0x42330feb), SkBits2Float(0x4285bd57), SkBits2Float(0x4246005c), SkBits2Float(0x427c99ac), SkBits2Float(0x4257723d));
path.lineTo(SkBits2Float(0x42369a46), SkBits2Float(0x421bbe89));
path.cubicTo(SkBits2Float(0x42415bc7), SkBits2Float(0x420f2230), SkBits2Float(0x424ac771), SkBits2Float(0x4201714b), SkBits2Float(0x4252b6a9), SkBits2Float(0x41e5c7e9));
path.lineTo(SkBits2Float(0x4291be57), SkBits2Float(0x421eee8a));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp169(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3725ffa9), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ea54b9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4261a7de), SkBits2Float(0xc2871f16), SkBits2Float(0x428ebc81), SkBits2Float(0xc2297f4d));
path.cubicTo(SkBits2Float(0x42aca513), SkBits2Float(0xc18980da), SkBits2Float(0x42adc9a4), SkBits2Float(0x41604127), SkBits2Float(0x4291be57), SkBits2Float(0x421eee8a));
path.cubicTo(SkBits2Float(0x428c4169), SkBits2Float(0x42330feb), SkBits2Float(0x4285bd57), SkBits2Float(0x4246005c), SkBits2Float(0x427c99ac), SkBits2Float(0x4257723d));
path.lineTo(SkBits2Float(0x42369a46), SkBits2Float(0x421bbe89));
path.cubicTo(SkBits2Float(0x42415bc7), SkBits2Float(0x420f2230), SkBits2Float(0x424ac771), SkBits2Float(0x4201714b), SkBits2Float(0x4252b6a9), SkBits2Float(0x41e5c7e9));
path.cubicTo(SkBits2Float(0x427b4260), SkBits2Float(0x41221c9f), SkBits2Float(0x42799b62), SkBits2Float(0xc146ccc2), SkBits2Float(0x424e5da6), SkBits2Float(0xc1f50e65));
path.cubicTo(SkBits2Float(0x42231fea), SkBits2Float(0xc2435b34), SkBits2Float(0x41a9655c), SkBits2Float(0xc26ffffe), SkBits2Float(0x3725ffa9), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427c99ad), SkBits2Float(0x4257723e));
path.cubicTo(SkBits2Float(0x422a2459), SkBits2Float(0x429c0ff6), SkBits2Float(0x411ef0c1), SkBits2Float(0x42b0a109), SkBits2Float(0xc1a68a7f), SkBits2Float(0x42a0b1a2));
path.cubicTo(SkBits2Float(0xc24e46af), SkBits2Float(0x4290c23b), SkBits2Float(0xc296269a), SkBits2Float(0x423e3c04), SkBits2Float(0xc2a2b82b), SkBits2Float(0x41835b51));
path.cubicTo(SkBits2Float(0xc2af49bc), SkBits2Float(0xc16b82d9), SkBits2Float(0xc2973524), SkBits2Float(0xc23adb29), SkBits2Float(0xc24965c6), SkBits2Float(0xc283f801));
path.lineTo(SkBits2Float(0xc21196ae), SkBits2Float(0xc23ecc58));
path.cubicTo(SkBits2Float(0xc25a9cfe), SkBits2Float(0xc20713a1), SkBits2Float(0xc27d6da1), SkBits2Float(0xc12a3fcc), SkBits2Float(0xc26b41bb), SkBits2Float(0x413de9a9));
path.cubicTo(SkBits2Float(0xc25915d3), SkBits2Float(0x420984c8), SkBits2Float(0xc2151d75), SkBits2Float(0x42514a1b), SkBits2Float(0xc170c819), SkBits2Float(0x4268540a));
path.cubicTo(SkBits2Float(0x40e5cb46), SkBits2Float(0x427f5dfa), SkBits2Float(0x41f5fd0c), SkBits2Float(0x4261a1d8), SkBits2Float(0x42369a4a), SkBits2Float(0x421bbe87));
path.lineTo(SkBits2Float(0x427c99ad), SkBits2Float(0x4257723e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp170(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ef3488), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4265f5fc), SkBits2Float(0xc285d5a4), SkBits2Float(0x429072a6), SkBits2Float(0xc2239841));
path.cubicTo(SkBits2Float(0x42adea4e), SkBits2Float(0xc16e14e5), SkBits2Float(0x42ad1da2), SkBits2Float(0x41886b20), SkBits2Float(0x428e5adb), SkBits2Float(0x422ac68e));
path.lineTo(SkBits2Float(0x424dd078), SkBits2Float(0x41f6e790));
path.cubicTo(SkBits2Float(0x427a49b4), SkBits2Float(0x41453b4b), SkBits2Float(0x427b719d), SkBits2Float(0xc12c1b6e), SkBits2Float(0x4250d71f), SkBits2Float(0xc1ec85c5));
path.cubicTo(SkBits2Float(0x42263ca0), SkBits2Float(0xc2417eea), SkBits2Float(0x41aceb63), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428e5adb), SkBits2Float(0x422ac690));
path.cubicTo(SkBits2Float(0x42885732), SkBits2Float(0x423ed443), SkBits2Float(0x428148a8), SkBits2Float(0x42518e43), SkBits2Float(0x42729aa0), SkBits2Float(0x4262a4bd));
path.lineTo(SkBits2Float(0x422f605c), SkBits2Float(0x4223d6b5));
path.cubicTo(SkBits2Float(0x423aea98), SkBits2Float(0x42177c70), SkBits2Float(0x42451e76), SkBits2Float(0x4209f2e4), SkBits2Float(0x424dd078), SkBits2Float(0x41f6e792));
path.lineTo(SkBits2Float(0x428e5adb), SkBits2Float(0x422ac690));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp171(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ef3488), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4265f5fc), SkBits2Float(0xc285d5a4), SkBits2Float(0x429072a6), SkBits2Float(0xc2239841));
path.cubicTo(SkBits2Float(0x42adea4e), SkBits2Float(0xc16e14e5), SkBits2Float(0x42ad1da2), SkBits2Float(0x41886b20), SkBits2Float(0x428e5adb), SkBits2Float(0x422ac690));
path.cubicTo(SkBits2Float(0x42885732), SkBits2Float(0x423ed443), SkBits2Float(0x428148a8), SkBits2Float(0x42518e43), SkBits2Float(0x42729aa0), SkBits2Float(0x4262a4bd));
path.lineTo(SkBits2Float(0x422f605c), SkBits2Float(0x4223d6b5));
path.cubicTo(SkBits2Float(0x423aea98), SkBits2Float(0x42177c70), SkBits2Float(0x42451e76), SkBits2Float(0x4209f2e4), SkBits2Float(0x424dd078), SkBits2Float(0x41f6e790));
path.cubicTo(SkBits2Float(0x427a49b4), SkBits2Float(0x41453b4b), SkBits2Float(0x427b719d), SkBits2Float(0xc12c1b6e), SkBits2Float(0x4250d71f), SkBits2Float(0xc1ec85c5));
path.cubicTo(SkBits2Float(0x42263ca0), SkBits2Float(0xc2417eea), SkBits2Float(0x41aceb63), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42729aa1), SkBits2Float(0x4262a4be));
path.cubicTo(SkBits2Float(0x421a0aa1), SkBits2Float(0x42a0b8ab), SkBits2Float(0x4092ff14), SkBits2Float(0x42b1fc82), SkBits2Float(0xc1d17709), SkBits2Float(0x429d861f));
path.cubicTo(SkBits2Float(0xc263d6eb), SkBits2Float(0x42890fbc), SkBits2Float(0xc29dea71), SkBits2Float(0x42253dbf), SkBits2Float(0xc2a5016a), SkBits2Float(0x4111261a));
path.cubicTo(SkBits2Float(0xc2ac1862), SkBits2Float(0xc1b95567), SkBits2Float(0xc28cface), SkBits2Float(0xc25a1117), SkBits2Float(0xc22aafa6), SkBits2Float(0xc28e61ba));
path.lineTo(SkBits2Float(0xc1f6c679), SkBits2Float(0xc24dda63));
path.cubicTo(SkBits2Float(0xc24bd376), SkBits2Float(0xc21da377), SkBits2Float(0xc278cff1), SkBits2Float(0xc185f9db), SkBits2Float(0xc26e8fe1), SkBits2Float(0x40d1da84));
path.cubicTo(SkBits2Float(0xc2644fd1), SkBits2Float(0x41eee71d), SkBits2Float(0xc224b3fc), SkBits2Float(0x4246293b), SkBits2Float(0xc1976b90), SkBits2Float(0x4263becd));
path.cubicTo(SkBits2Float(0x405486c0), SkBits2Float(0x4280aa2f), SkBits2Float(0x41deb5f2), SkBits2Float(0x42685e3e), SkBits2Float(0x422f605e), SkBits2Float(0x4223d6b6));
path.lineTo(SkBits2Float(0x42729aa1), SkBits2Float(0x4262a4be));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp172(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f30c96), SkBits2Float(0xc2a60000), SkBits2Float(0x426956a5), SkBits2Float(0xc284cd4a), SkBits2Float(0x4291c05e), SkBits2Float(0xc21ee718));
path.cubicTo(SkBits2Float(0x42aed56a), SkBits2Float(0xc150ce71), SkBits2Float(0x42ac7181), SkBits2Float(0x419b8107), SkBits2Float(0x428b8516), SkBits2Float(0x4233e422));
path.lineTo(SkBits2Float(0x4249b729), SkBits2Float(0x42020ab3));
path.cubicTo(SkBits2Float(0x427950d3), SkBits2Float(0x4160d339), SkBits2Float(0x427cc584), SkBits2Float(0xc116f1c4), SkBits2Float(0x4252b998), SkBits2Float(0xc1e5bd26));
path.cubicTo(SkBits2Float(0x4228adad), SkBits2Float(0xc24000b5), SkBits2Float(0x41afb2be), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428b8516), SkBits2Float(0x4233e422));
path.cubicTo(SkBits2Float(0x4285165c), SkBits2Float(0x4247d8d0), SkBits2Float(0x427b34bd), SkBits2Float(0x425a5d74), SkBits2Float(0x426a6401), SkBits2Float(0x426b20b1));
path.lineTo(SkBits2Float(0x42297063), SkBits2Float(0x4229f8c9));
path.cubicTo(SkBits2Float(0x42359840), SkBits2Float(0x421ddab1), SkBits2Float(0x42406a5a), SkBits2Float(0x421077b9), SkBits2Float(0x4249b72b), SkBits2Float(0x42020ab4));
path.lineTo(SkBits2Float(0x428b8516), SkBits2Float(0x4233e422));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp173(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f30c96), SkBits2Float(0xc2a60000), SkBits2Float(0x426956a5), SkBits2Float(0xc284cd4a), SkBits2Float(0x4291c05e), SkBits2Float(0xc21ee718));
path.cubicTo(SkBits2Float(0x42aed56a), SkBits2Float(0xc150ce71), SkBits2Float(0x42ac7181), SkBits2Float(0x419b8107), SkBits2Float(0x428b8516), SkBits2Float(0x4233e422));
path.cubicTo(SkBits2Float(0x4285165c), SkBits2Float(0x4247d8d0), SkBits2Float(0x427b34bd), SkBits2Float(0x425a5d74), SkBits2Float(0x426a6401), SkBits2Float(0x426b20b1));
path.lineTo(SkBits2Float(0x42297063), SkBits2Float(0x4229f8c9));
path.cubicTo(SkBits2Float(0x42359840), SkBits2Float(0x421ddab1), SkBits2Float(0x42406a5a), SkBits2Float(0x421077b9), SkBits2Float(0x4249b72b), SkBits2Float(0x42020ab4));
path.lineTo(SkBits2Float(0x4249b729), SkBits2Float(0x42020ab3));
path.cubicTo(SkBits2Float(0x427950d3), SkBits2Float(0x4160d339), SkBits2Float(0x427cc584), SkBits2Float(0xc116f1c4), SkBits2Float(0x4252b998), SkBits2Float(0xc1e5bd26));
path.cubicTo(SkBits2Float(0x4228adad), SkBits2Float(0xc24000b5), SkBits2Float(0x41afb2be), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x426a6401), SkBits2Float(0x426b20b0));
path.cubicTo(SkBits2Float(0x420d0644), SkBits2Float(0x42a419c2), SkBits2Float(0x3eb79d8f), SkBits2Float(0x42b29b69), SkBits2Float(0xc1f292a7), SkBits2Float(0x429a86c6));
path.cubicTo(SkBits2Float(0xc27401e4), SkBits2Float(0x42827223), SkBits2Float(0xc2a34d81), SkBits2Float(0x4210aea0), SkBits2Float(0xc2a5dfaf), SkBits2Float(0x404f3106));
path.cubicTo(SkBits2Float(0xc2a871dd), SkBits2Float(0xc1ed90fa), SkBits2Float(0xc283ccf3), SkBits2Float(0xc27113da), SkBits2Float(0xc21101fe), SkBits2Float(0xc2955440));
path.lineTo(SkBits2Float(0xc1d1a65c), SkBits2Float(0xc257e5c3));
path.cubicTo(SkBits2Float(0xc23e8e16), SkBits2Float(0xc22e45d9), SkBits2Float(0xc27388d2), SkBits2Float(0xc1abbc0d), SkBits2Float(0xc26fd138), SkBits2Float(0x4015c6fe));
path.cubicTo(SkBits2Float(0xc26c199f), SkBits2Float(0x41d12dcc), SkBits2Float(0xc2306400), SkBits2Float(0x423c98a5), SkBits2Float(0xc1af5a7e), SkBits2Float(0x425f695f));
path.cubicTo(SkBits2Float(0x3e84bf70), SkBits2Float(0x42811d0c), SkBits2Float(0x41cbe40c), SkBits2Float(0x426d40fa), SkBits2Float(0x42297064), SkBits2Float(0x4229f8cc));
path.lineTo(SkBits2Float(0x426a6401), SkBits2Float(0x426b20b0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp174(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f67553), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426c5214), SkBits2Float(0xc283df7d), SkBits2Float(0x4292df93), SkBits2Float(0xc21ab724));
path.cubicTo(SkBits2Float(0x42af961c), SkBits2Float(0xc136bd38), SkBits2Float(0x42abbe10), SkBits2Float(0x41ac5dd5), SkBits2Float(0x4288e395), SkBits2Float(0x423bcd53));
path.lineTo(SkBits2Float(0x4245e96c), SkBits2Float(0x4207c2b1));
path.cubicTo(SkBits2Float(0x42784d66), SkBits2Float(0x41793464), SkBits2Float(0x427ddc1f), SkBits2Float(0xc10419c2), SkBits2Float(0x425458d8), SkBits2Float(0xc1dfaf58));
path.cubicTo(SkBits2Float(0x422ad590), SkBits2Float(0xc23ea8e8), SkBits2Float(0x41b229a4), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4288e396), SkBits2Float(0x423bcd52));
path.cubicTo(SkBits2Float(0x42821571), SkBits2Float(0x424fa4b8), SkBits2Float(0x427470be), SkBits2Float(0x4261f24c), SkBits2Float(0x4262dfb6), SkBits2Float(0x4272637b));
path.lineTo(SkBits2Float(0x42240156), SkBits2Float(0x422f387f));
path.cubicTo(SkBits2Float(0x4230b436), SkBits2Float(0x422355b8), SkBits2Float(0x423c12ab), SkBits2Float(0x42161a8d), SkBits2Float(0x4245e96e), SkBits2Float(0x4207c2b2));
path.lineTo(SkBits2Float(0x4288e396), SkBits2Float(0x423bcd52));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp175(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f67553), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426c5214), SkBits2Float(0xc283df7d), SkBits2Float(0x4292df93), SkBits2Float(0xc21ab724));
path.cubicTo(SkBits2Float(0x42af961c), SkBits2Float(0xc136bd38), SkBits2Float(0x42abbe10), SkBits2Float(0x41ac5dd5), SkBits2Float(0x4288e396), SkBits2Float(0x423bcd52));
path.cubicTo(SkBits2Float(0x42821571), SkBits2Float(0x424fa4b8), SkBits2Float(0x427470be), SkBits2Float(0x4261f24c), SkBits2Float(0x4262dfb6), SkBits2Float(0x4272637b));
path.lineTo(SkBits2Float(0x42240156), SkBits2Float(0x422f387f));
path.cubicTo(SkBits2Float(0x4230b436), SkBits2Float(0x422355b8), SkBits2Float(0x423c12ab), SkBits2Float(0x42161a8d), SkBits2Float(0x4245e96e), SkBits2Float(0x4207c2b2));
path.lineTo(SkBits2Float(0x4245e96c), SkBits2Float(0x4207c2b1));
path.cubicTo(SkBits2Float(0x42784d66), SkBits2Float(0x41793464), SkBits2Float(0x427ddc1f), SkBits2Float(0xc10419c2), SkBits2Float(0x425458d8), SkBits2Float(0xc1dfaf58));
path.cubicTo(SkBits2Float(0x422ad590), SkBits2Float(0xc23ea8e8), SkBits2Float(0x41b229a4), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4262dfb7), SkBits2Float(0x4272637c));
path.cubicTo(SkBits2Float(0x4201435c), SkBits2Float(0x42a6e035), SkBits2Float(0xc05a052a), SkBits2Float(0x42b2d330), SkBits2Float(0xc207a774), SkBits2Float(0x429782c3));
path.cubicTo(SkBits2Float(0xc280d74a), SkBits2Float(0x427864aa), SkBits2Float(0xc2a78489), SkBits2Float(0x41fbcc10), SkBits2Float(0xc2a5f467), SkBits2Float(0xbff86670));
path.cubicTo(SkBits2Float(0xc2a46445), SkBits2Float(0xc20d6c6d), SkBits2Float(0xc275c9b5), SkBits2Float(0xc2821580), SkBits2Float(0xc1f2ade6), SkBits2Float(0xc29a8413));
path.lineTo(SkBits2Float(0xc1af6e4e), SkBits2Float(0xc25f6582));
path.cubicTo(SkBits2Float(0xc231ad90), SkBits2Float(0xc23c12bd), SkBits2Float(0xc26dacb3), SkBits2Float(0xc1cc77b7), SkBits2Float(0xc26fef30), SkBits2Float(0xbfb390a5));
path.cubicTo(SkBits2Float(0xc27231ae), SkBits2Float(0x41b605a0), SkBits2Float(0xc23a46a0), SkBits2Float(0x42338faf), SkBits2Float(0xc1c42047), SkBits2Float(0x425b0d36));
path.cubicTo(SkBits2Float(0xc01d9a6d), SkBits2Float(0x4281455e), SkBits2Float(0x41bae2f1), SkBits2Float(0x42714420), SkBits2Float(0x42240157), SkBits2Float(0x422f387f));
path.lineTo(SkBits2Float(0x4262dfb7), SkBits2Float(0x4272637c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp176(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f9cdf3), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426f3c43), SkBits2Float(0xc282f30b), SkBits2Float(0x4293f176), SkBits2Float(0xc2169536));
path.cubicTo(SkBits2Float(0x42b044ca), SkBits2Float(0xc11d115b), SkBits2Float(0x42aaf59e), SkBits2Float(0x41bcd986), SkBits2Float(0x428633ff), SkBits2Float(0x42436703));
path.lineTo(SkBits2Float(0x42420751), SkBits2Float(0x420d4138));
path.cubicTo(SkBits2Float(0x42772b98), SkBits2Float(0x41888496), SkBits2Float(0x427ed8af), SkBits2Float(0xc0e315f7), SkBits2Float(0x4255e4d4), SkBits2Float(0xc1d9b5cc));
path.cubicTo(SkBits2Float(0x422cf0fb), SkBits2Float(0xc23d530d), SkBits2Float(0x41b494e9), SkBits2Float(0xc2700000), SkBits2Float(0x3743ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428633ff), SkBits2Float(0x42436705));
path.cubicTo(SkBits2Float(0x427e0fd0), SkBits2Float(0x42571b29), SkBits2Float(0x426d975d), SkBits2Float(0x42692b9b), SkBits2Float(0x425b4ae0), SkBits2Float(0x427944c1));
path.lineTo(SkBits2Float(0x421e8652), SkBits2Float(0x423431b3));
path.cubicTo(SkBits2Float(0x422bc0b3), SkBits2Float(0x42288e8e), SkBits2Float(0x4237a8bb), SkBits2Float(0x421b7f95), SkBits2Float(0x42420752), SkBits2Float(0x420d4138));
path.lineTo(SkBits2Float(0x428633ff), SkBits2Float(0x42436705));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp177(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3743ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f9cdf3), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426f3c43), SkBits2Float(0xc282f30b), SkBits2Float(0x4293f176), SkBits2Float(0xc2169536));
path.cubicTo(SkBits2Float(0x42b044ca), SkBits2Float(0xc11d115b), SkBits2Float(0x42aaf59e), SkBits2Float(0x41bcd986), SkBits2Float(0x428633ff), SkBits2Float(0x42436705));
path.cubicTo(SkBits2Float(0x427e0fd0), SkBits2Float(0x42571b29), SkBits2Float(0x426d975d), SkBits2Float(0x42692b9b), SkBits2Float(0x425b4ae0), SkBits2Float(0x427944c1));
path.lineTo(SkBits2Float(0x421e8652), SkBits2Float(0x423431b3));
path.cubicTo(SkBits2Float(0x422bc0b3), SkBits2Float(0x42288e8e), SkBits2Float(0x4237a8bb), SkBits2Float(0x421b7f95), SkBits2Float(0x42420751), SkBits2Float(0x420d4138));
path.cubicTo(SkBits2Float(0x42772b98), SkBits2Float(0x41888496), SkBits2Float(0x427ed8af), SkBits2Float(0xc0e315f7), SkBits2Float(0x4255e4d4), SkBits2Float(0xc1d9b5cc));
path.cubicTo(SkBits2Float(0x422cf0fb), SkBits2Float(0xc23d530d), SkBits2Float(0x41b494e9), SkBits2Float(0xc2700000), SkBits2Float(0x3743ffa9), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x425b4ae0), SkBits2Float(0x427944c0));
path.cubicTo(SkBits2Float(0x41eb12b8), SkBits2Float(0x42a964d5), SkBits2Float(0xc0e3546a), SkBits2Float(0x42b2bc1c), SkBits2Float(0xc2157060), SkBits2Float(0x42943ba4));
path.cubicTo(SkBits2Float(0xc2873b19), SkBits2Float(0x426b7658), SkBits2Float(0xc2ab209f), SkBits2Float(0x41d60b1d), SkBits2Float(0xc2a5685b), SkBits2Float(0xc0e02f3c));
path.cubicTo(SkBits2Float(0xc29fb018), SkBits2Float(0xc223115c), SkBits2Float(0xc263001e), SkBits2Float(0xc28acd07), SkBits2Float(0xc1c2e1a0), SkBits2Float(0xc29eb07c));
path.lineTo(SkBits2Float(0xc18ce0d1), SkBits2Float(0xc2656e32));
path.cubicTo(SkBits2Float(0xc22418c2), SkBits2Float(0xc248ad0a), SkBits2Float(0xc266dfbc), SkBits2Float(0xc1ebc2b6), SkBits2Float(0xc26f24bb), SkBits2Float(0xc0a20f94));
path.cubicTo(SkBits2Float(0xc27769ba), SkBits2Float(0x419abaee), SkBits2Float(0xc24383ac), SkBits2Float(0x422a36b0), SkBits2Float(0xc1d80e5c), SkBits2Float(0x4256500a));
path.cubicTo(SkBits2Float(0xc0a45587), SkBits2Float(0x428134b2), SkBits2Float(0x41a9eeb8), SkBits2Float(0x4274e820), SkBits2Float(0x421e8655), SkBits2Float(0x423431b1));
path.lineTo(SkBits2Float(0x425b4ae0), SkBits2Float(0x427944c0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp178(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fc5f30), SkBits2Float(0xc2a5fffe), SkBits2Float(0x427176a0), SkBits2Float(0xc2823b95), SkBits2Float(0x4294be35), SkBits2Float(0xc21365c9));
path.cubicTo(SkBits2Float(0x42b0c118), SkBits2Float(0xc1095198), SkBits2Float(0x42aa4b8f), SkBits2Float(0x41c9721a), SkBits2Float(0x42841312), SkBits2Float(0x42491ec0));
path.lineTo(SkBits2Float(0x423ef37b), SkBits2Float(0x42116356));
path.cubicTo(SkBits2Float(0x427635bc), SkBits2Float(0x41919f96), SkBits2Float(0x427f8c66), SkBits2Float(0xc0c68887), SkBits2Float(0x42570cd6), SkBits2Float(0xc1d51ae4));
path.cubicTo(SkBits2Float(0x422e8d45), SkBits2Float(0xc23c49d3), SkBits2Float(0x41b66ffd), SkBits2Float(0xc2700000), SkBits2Float(0xb7060057), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42841313), SkBits2Float(0x42491ebf));
path.cubicTo(SkBits2Float(0x42793d8e), SkBits2Float(0x425cb36e), SkBits2Float(0x4268336d), SkBits2Float(0x426e9032), SkBits2Float(0x4255582b), SkBits2Float(0x427e60c5));
path.lineTo(SkBits2Float(0x421a3990), SkBits2Float(0x4237e342));
path.cubicTo(SkBits2Float(0x4227db27), SkBits2Float(0x422c7494), SkBits2Float(0x42342c7f), SkBits2Float(0x421f8af7), SkBits2Float(0x423ef37c), SkBits2Float(0x42116357));
path.lineTo(SkBits2Float(0x42841313), SkBits2Float(0x42491ebf));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp179(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb7060057), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fc5f30), SkBits2Float(0xc2a5fffe), SkBits2Float(0x427176a0), SkBits2Float(0xc2823b95), SkBits2Float(0x4294be35), SkBits2Float(0xc21365c9));
path.cubicTo(SkBits2Float(0x42b0c118), SkBits2Float(0xc1095198), SkBits2Float(0x42aa4b8f), SkBits2Float(0x41c9721a), SkBits2Float(0x42841313), SkBits2Float(0x42491ebf));
path.cubicTo(SkBits2Float(0x42793d8e), SkBits2Float(0x425cb36e), SkBits2Float(0x4268336d), SkBits2Float(0x426e9032), SkBits2Float(0x4255582b), SkBits2Float(0x427e60c5));
path.lineTo(SkBits2Float(0x421a3990), SkBits2Float(0x4237e342));
path.cubicTo(SkBits2Float(0x4227db27), SkBits2Float(0x422c7494), SkBits2Float(0x42342c7f), SkBits2Float(0x421f8af7), SkBits2Float(0x423ef37b), SkBits2Float(0x42116356));
path.cubicTo(SkBits2Float(0x427635bc), SkBits2Float(0x41919f96), SkBits2Float(0x427f8c66), SkBits2Float(0xc0c68887), SkBits2Float(0x42570cd6), SkBits2Float(0xc1d51ae4));
path.cubicTo(SkBits2Float(0x422e8d45), SkBits2Float(0xc23c49d3), SkBits2Float(0x41b66ffd), SkBits2Float(0xc2700000), SkBits2Float(0xb7060057), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4255582a), SkBits2Float(0x427e60c6));
path.cubicTo(SkBits2Float(0x41d8da26), SkBits2Float(0x42ab2f9f), SkBits2Float(0xc11f0392), SkBits2Float(0x42b2763a), SkBits2Float(0xc21fc8f1), SkBits2Float(0x4291829a));
path.cubicTo(SkBits2Float(0xc28be87e), SkBits2Float(0x42611df4), SkBits2Float(0xc2ad8941), SkBits2Float(0x41b88f93), SkBits2Float(0xc2a49219), SkBits2Float(0xc12de56c));
path.cubicTo(SkBits2Float(0xc29b9af2), SkBits2Float(0xc2333a80), SkBits2Float(0xc253c58e), SkBits2Float(0xc2910614), SkBits2Float(0xc19d7dc6), SkBits2Float(0xc2a14359));
path.lineTo(SkBits2Float(0xc163b2c9), SkBits2Float(0xc26926c4));
path.cubicTo(SkBits2Float(0xc2191685), SkBits2Float(0xc251ac40), SkBits2Float(0xc260f8ae), SkBits2Float(0xc201900e), SkBits2Float(0xc26deef7), SkBits2Float(0xc0fb6a70));
path.cubicTo(SkBits2Float(0xc27ae541), SkBits2Float(0x41856ae3), SkBits2Float(0xc24a46d8), SkBits2Float(0x4222bc35), SkBits2Float(0xc1e7039a), SkBits2Float(0x42526049));
path.cubicTo(SkBits2Float(0xc0e5e60c), SkBits2Float(0x4281022e), SkBits2Float(0x419cc2c4), SkBits2Float(0x42777f70), SkBits2Float(0x421a3996), SkBits2Float(0x4237e33e));
path.lineTo(SkBits2Float(0x4255582a), SkBits2Float(0x427e60c6));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp180(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fed5d1), SkBits2Float(0xc2a60000), SkBits2Float(0x4273981d), SkBits2Float(0xc28189e8), SkBits2Float(0x42957e40), SkBits2Float(0xc210547e));
path.cubicTo(SkBits2Float(0x42b13073), SkBits2Float(0xc0eca961), SkBits2Float(0x42a99b35), SkBits2Float(0x41d57c6c), SkBits2Float(0x4281fa62), SkBits2Float(0x424e82d3));
path.lineTo(SkBits2Float(0x423beb8b), SkBits2Float(0x421548fc));
path.cubicTo(SkBits2Float(0x427536c2), SkBits2Float(0x419a53c7), SkBits2Float(0x428016af), SkBits2Float(0xc0ab14a9), SkBits2Float(0x4258227d), SkBits2Float(0xc1d0ab83));
path.cubicTo(SkBits2Float(0x4230179a), SkBits2Float(0xc23b48ee), SkBits2Float(0x41b837da), SkBits2Float(0xc2700002), SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4281fa62), SkBits2Float(0x424e82d5));
path.cubicTo(SkBits2Float(0x4274817d), SkBits2Float(0x4261f5b7), SkBits2Float(0x4262ebfa), SkBits2Float(0x42739d02), SkBits2Float(0x424f88b8), SkBits2Float(0x428191ef));
path.lineTo(SkBits2Float(0x4216064f), SkBits2Float(0x423b5489));
path.cubicTo(SkBits2Float(0x42240a35), SkBits2Float(0x42301b25), SkBits2Float(0x4230c051), SkBits2Float(0x4223582f), SkBits2Float(0x423beb8c), SkBits2Float(0x421548fc));
path.lineTo(SkBits2Float(0x4281fa62), SkBits2Float(0x424e82d5));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp181(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fed5d1), SkBits2Float(0xc2a60000), SkBits2Float(0x4273981d), SkBits2Float(0xc28189e8), SkBits2Float(0x42957e40), SkBits2Float(0xc210547e));
path.cubicTo(SkBits2Float(0x42b13073), SkBits2Float(0xc0eca961), SkBits2Float(0x42a99b35), SkBits2Float(0x41d57c6c), SkBits2Float(0x4281fa62), SkBits2Float(0x424e82d5));
path.cubicTo(SkBits2Float(0x4274817d), SkBits2Float(0x4261f5b7), SkBits2Float(0x4262ebfa), SkBits2Float(0x42739d02), SkBits2Float(0x424f88b8), SkBits2Float(0x428191ef));
path.lineTo(SkBits2Float(0x4216064f), SkBits2Float(0x423b5489));
path.cubicTo(SkBits2Float(0x42240a35), SkBits2Float(0x42301b25), SkBits2Float(0x4230c051), SkBits2Float(0x4223582f), SkBits2Float(0x423beb8b), SkBits2Float(0x421548fc));
path.cubicTo(SkBits2Float(0x427536c2), SkBits2Float(0x419a53c7), SkBits2Float(0x428016af), SkBits2Float(0xc0ab14a9), SkBits2Float(0x4258227d), SkBits2Float(0xc1d0ab83));
path.cubicTo(SkBits2Float(0x4230179a), SkBits2Float(0xc23b48ee), SkBits2Float(0x41b837da), SkBits2Float(0xc2700002), SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x424f88ba), SkBits2Float(0x428191f0));
path.cubicTo(SkBits2Float(0x41c732b7), SkBits2Float(0x42acca52), SkBits2Float(0xc14a7268), SkBits2Float(0x42b208b4), SkBits2Float(0xc22982dc), SkBits2Float(0x428ebb75));
path.cubicTo(SkBits2Float(0xc2903490), SkBits2Float(0x4256dc6c), SkBits2Float(0xc2af8c6f), SkBits2Float(0x419be833), SkBits2Float(0xc2a36e37), SkBits2Float(0xc168c0a6));
path.cubicTo(SkBits2Float(0xc2974fff), SkBits2Float(0xc242546a), SkBits2Float(0xc2448acf), SkBits2Float(0xc29698ac), SkBits2Float(0xc17253d7), SkBits2Float(0xc2a33682));
path.lineTo(SkBits2Float(0xc12f2d38), SkBits2Float(0xc26bf872));
path.cubicTo(SkBits2Float(0xc20e1427), SkBits2Float(0xc259bacc), SkBits2Float(0xc25ac3d7), SkBits2Float(0xc20c7ab2), SkBits2Float(0xc26c48f7), SkBits2Float(0xc1284130));
path.cubicTo(SkBits2Float(0xc27dce17), SkBits2Float(0x41616864), SkBits2Float(0xc2507d50), SkBits2Float(0x421b5239), SkBits2Float(0xc1f51386), SkBits2Float(0x424e5c1e));
path.cubicTo(SkBits2Float(0xc11258cd), SkBits2Float(0x4280b301), SkBits2Float(0x418fffac), SkBits2Float(0x4279d13a), SkBits2Float(0x42160652), SkBits2Float(0x423b5488));
path.lineTo(SkBits2Float(0x424f88ba), SkBits2Float(0x428191f0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp182(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420048ef), SkBits2Float(0xc2a60000), SkBits2Float(0x4275172d), SkBits2Float(0xc2810bd2), SkBits2Float(0x429602e3), SkBits2Float(0xc20e29dc));
path.cubicTo(SkBits2Float(0x42b17a30), SkBits2Float(0xc0d1e0a1), SkBits2Float(0x42a9174e), SkBits2Float(0x41ddef9e), SkBits2Float(0x4280787d), SkBits2Float(0x4252400e));
path.lineTo(SkBits2Float(0x4239bd9f), SkBits2Float(0x4217fcf6));
path.cubicTo(SkBits2Float(0x4274780f), SkBits2Float(0x41a06f8c), SkBits2Float(0x42804bfe), SkBits2Float(0xc097b7f0), SkBits2Float(0x4258e240), SkBits2Float(0xc1cd899e));
path.cubicTo(SkBits2Float(0x42312c84), SkBits2Float(0xc23a929f), SkBits2Float(0x41b978e3), SkBits2Float(0xc2700000), SkBits2Float(0x36d3ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4280787d), SkBits2Float(0x42524010));
path.cubicTo(SkBits2Float(0x42711c0e), SkBits2Float(0x42659909), SkBits2Float(0x425f24ad), SkBits2Float(0x42771864), SkBits2Float(0x424b624a), SkBits2Float(0x4283347a));
path.lineTo(SkBits2Float(0x42130648), SkBits2Float(0x423db1a5));
path.cubicTo(SkBits2Float(0x42214ef3), SkBits2Float(0x42329f82), SkBits2Float(0x422e4bcd), SkBits2Float(0x4225f96c), SkBits2Float(0x4239bd9f), SkBits2Float(0x4217fcf7));
path.lineTo(SkBits2Float(0x4280787d), SkBits2Float(0x42524010));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp183(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36d3ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420048ef), SkBits2Float(0xc2a60000), SkBits2Float(0x4275172d), SkBits2Float(0xc2810bd2), SkBits2Float(0x429602e3), SkBits2Float(0xc20e29dc));
path.cubicTo(SkBits2Float(0x42b17a30), SkBits2Float(0xc0d1e0a1), SkBits2Float(0x42a9174e), SkBits2Float(0x41ddef9e), SkBits2Float(0x4280787d), SkBits2Float(0x42524010));
path.cubicTo(SkBits2Float(0x42711c0e), SkBits2Float(0x42659909), SkBits2Float(0x425f24ad), SkBits2Float(0x42771864), SkBits2Float(0x424b624a), SkBits2Float(0x4283347a));
path.lineTo(SkBits2Float(0x42130648), SkBits2Float(0x423db1a5));
path.cubicTo(SkBits2Float(0x42214ef3), SkBits2Float(0x42329f82), SkBits2Float(0x422e4bcd), SkBits2Float(0x4225f96c), SkBits2Float(0x4239bd9f), SkBits2Float(0x4217fcf6));
path.cubicTo(SkBits2Float(0x4274780f), SkBits2Float(0x41a06f8c), SkBits2Float(0x42804bfe), SkBits2Float(0xc097b7f0), SkBits2Float(0x4258e240), SkBits2Float(0xc1cd899e));
path.cubicTo(SkBits2Float(0x42312c84), SkBits2Float(0xc23a929f), SkBits2Float(0x41b978e3), SkBits2Float(0xc2700000), SkBits2Float(0x36d3ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x424b624a), SkBits2Float(0x42833479));
path.cubicTo(SkBits2Float(0x41baac2f), SkBits2Float(0x42adda12), SkBits2Float(0xc168f6a7), SkBits2Float(0x42b1a2b3), SkBits2Float(0xc2303c92), SkBits2Float(0x428cae5c));
path.cubicTo(SkBits2Float(0xc2931dbe), SkBits2Float(0x424f7409), SkBits2Float(0xc2b0c9d8), SkBits2Float(0x41878abe), SkBits2Float(0xc2a26e7f), SkBits2Float(0xc188ef9a));
path.cubicTo(SkBits2Float(0xc2941327), SkBits2Float(0xc24cb4f5), SkBits2Float(0xc2397a7c), SkBits2Float(0xc29a4742), SkBits2Float(0xc13ec328), SkBits2Float(0xc2a44746));
path.lineTo(SkBits2Float(0xc109e67a), SkBits2Float(0xc26d82d0));
path.cubicTo(SkBits2Float(0xc20614b0), SkBits2Float(0xc25f0d94), SkBits2Float(0xc2561585), SkBits2Float(0xc213fb18), SkBits2Float(0xc26ad744), SkBits2Float(0xc145fabb));
path.cubicTo(SkBits2Float(0xc27f9901), SkBits2Float(0x4143f6e8), SkBits2Float(0xc254b2af), SkBits2Float(0x4215f75b), SkBits2Float(0xc1feccbb), SkBits2Float(0x424b64f3));
path.cubicTo(SkBits2Float(0xc128682f), SkBits2Float(0x42806945), SkBits2Float(0x4186f1ba), SkBits2Float(0x427b5a1e), SkBits2Float(0x4213064f), SkBits2Float(0x423db1a2));
path.lineTo(SkBits2Float(0x424b624a), SkBits2Float(0x42833479));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp184(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42011b87), SkBits2Float(0xc2a5fffe), SkBits2Float(0x427681ab), SkBits2Float(0xc280937a), SkBits2Float(0x42967eb3), SkBits2Float(0xc20c1a94));
path.cubicTo(SkBits2Float(0x42b1bc91), SkBits2Float(0xc0b87191), SkBits2Float(0x42a89454), SkBits2Float(0x41e5ed6f), SkBits2Float(0x427e0902), SkBits2Float(0x4255c0a2));
path.lineTo(SkBits2Float(0x4237a3d0), SkBits2Float(0x421a8517));
path.cubicTo(SkBits2Float(0x4273bab4), SkBits2Float(0x41a63674), SkBits2Float(0x42807bfc), SkBits2Float(0xc0855530), SkBits2Float(0x42599545), SkBits2Float(0xc1ca8f4f));
path.cubicTo(SkBits2Float(0x42323293), SkBits2Float(0xc239e4a8), SkBits2Float(0x41baa959), SkBits2Float(0xc2700002), SkBits2Float(0xb5600574), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427e0901), SkBits2Float(0x4255c0a4));
path.cubicTo(SkBits2Float(0x426dd77c), SkBits2Float(0x4268ff65), SkBits2Float(0x425b838b), SkBits2Float(0x427a571f), SkBits2Float(0x42476779), SkBits2Float(0x4284b92f));
path.lineTo(SkBits2Float(0x421025c9), SkBits2Float(0x423fe3a3));
path.cubicTo(SkBits2Float(0x421eaf4b), SkBits2Float(0x4234f80b), SkBits2Float(0x422bef10), SkBits2Float(0x42286e9a), SkBits2Float(0x4237a3d2), SkBits2Float(0x421a8517));
path.lineTo(SkBits2Float(0x427e0901), SkBits2Float(0x4255c0a4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp185(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42011b87), SkBits2Float(0xc2a5fffe), SkBits2Float(0x427681ab), SkBits2Float(0xc280937a), SkBits2Float(0x42967eb3), SkBits2Float(0xc20c1a94));
path.cubicTo(SkBits2Float(0x42b1bc91), SkBits2Float(0xc0b87191), SkBits2Float(0x42a89454), SkBits2Float(0x41e5ed6f), SkBits2Float(0x427e0902), SkBits2Float(0x4255c0a2));
path.lineTo(SkBits2Float(0x427e0901), SkBits2Float(0x4255c0a4));
path.cubicTo(SkBits2Float(0x426dd77c), SkBits2Float(0x4268ff65), SkBits2Float(0x425b838b), SkBits2Float(0x427a571f), SkBits2Float(0x42476779), SkBits2Float(0x4284b92f));
path.lineTo(SkBits2Float(0x421025c9), SkBits2Float(0x423fe3a3));
path.cubicTo(SkBits2Float(0x421eaf4b), SkBits2Float(0x4234f80b), SkBits2Float(0x422bef10), SkBits2Float(0x42286e9a), SkBits2Float(0x4237a3d2), SkBits2Float(0x421a8517));
path.lineTo(SkBits2Float(0x4237a3d0), SkBits2Float(0x421a8517));
path.cubicTo(SkBits2Float(0x4273bab4), SkBits2Float(0x41a63674), SkBits2Float(0x42807bfc), SkBits2Float(0xc0855530), SkBits2Float(0x42599545), SkBits2Float(0xc1ca8f4f));
path.cubicTo(SkBits2Float(0x42323293), SkBits2Float(0xc239e4a8), SkBits2Float(0x41baa959), SkBits2Float(0xc2700002), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42476779), SkBits2Float(0x4284b92f));
path.cubicTo(SkBits2Float(0x41aeb99d), SkBits2Float(0x42aece6d), SkBits2Float(0xc182ebc7), SkBits2Float(0x42b12f04), SkBits2Float(0xc236847b), SkBits2Float(0x428aaa1d));
path.cubicTo(SkBits2Float(0xc295c989), SkBits2Float(0x42484a6d), SkBits2Float(0xc2b1d401), SkBits2Float(0x41683386), SkBits2Float(0xc2a15607), SkBits2Float(0xc19c4a77));
path.cubicTo(SkBits2Float(0xc290d80f), SkBits2Float(0xc2565754), SkBits2Float(0xc22ebdc1), SkBits2Float(0xc29d94aa), SkBits2Float(0xc10da15c), SkBits2Float(0xc2a50da2));
path.lineTo(SkBits2Float(0xc0ccc448), SkBits2Float(0xc26ea197));
path.cubicTo(SkBits2Float(0xc1fca350), SkBits2Float(0xc263d3da), SkBits2Float(0xc25169ba), SkBits2Float(0xc21af203), SkBits2Float(0xc26941c7), SkBits2Float(0xc161f664));
path.cubicTo(SkBits2Float(0xc2808cea), SkBits2Float(0x4127db45), SkBits2Float(0xc2588f4e), SkBits2Float(0x4210c9da), SkBits2Float(0xc203f0b6), SkBits2Float(0x42487a91));
path.cubicTo(SkBits2Float(0xc13d487f), SkBits2Float(0x428015a4), SkBits2Float(0x417c9d5c), SkBits2Float(0x427cbb65), SkBits2Float(0x421025ca), SkBits2Float(0x423fe3a2));
path.lineTo(SkBits2Float(0x42476779), SkBits2Float(0x4284b92f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp186(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4201bd60), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427797bb), SkBits2Float(0xc2803682), SkBits2Float(0x4296dc8c), SkBits2Float(0xc20a848f));
path.cubicTo(SkBits2Float(0x42b1ed3b), SkBits2Float(0xc0a4e0c3), SkBits2Float(0x42a82bcd), SkBits2Float(0x41ec0db8), SkBits2Float(0x427bc56e), SkBits2Float(0x42586a20));
path.lineTo(SkBits2Float(0x423600d6), SkBits2Float(0x421c71bc));
path.cubicTo(SkBits2Float(0x42732394), SkBits2Float(0x41aaa425), SkBits2Float(0x42809f29), SkBits2Float(0xc06e60a8), SkBits2Float(0x425a1cf3), SkBits2Float(0xc1c84447));
path.cubicTo(SkBits2Float(0x4232fb94), SkBits2Float(0xc2395e3c), SkBits2Float(0x41bb9357), SkBits2Float(0xc2700002), SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427bc56c), SkBits2Float(0x42586a22));
path.cubicTo(SkBits2Float(0x426b4cc6), SkBits2Float(0x426b93ad), SkBits2Float(0x4258b1e1), SkBits2Float(0x427ccbca), SkBits2Float(0x42445140), SkBits2Float(0x4285de6e));
path.lineTo(SkBits2Float(0x420dea8b), SkBits2Float(0x42418b9b));
path.cubicTo(SkBits2Float(0x421ca599), SkBits2Float(0x4236be7f), SkBits2Float(0x422a18a8), SkBits2Float(0x422a4be8), SkBits2Float(0x423600d6), SkBits2Float(0x421c71bc));
path.lineTo(SkBits2Float(0x427bc56c), SkBits2Float(0x42586a22));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp187(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4201bd60), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427797bb), SkBits2Float(0xc2803682), SkBits2Float(0x4296dc8c), SkBits2Float(0xc20a848f));
path.cubicTo(SkBits2Float(0x42b1ed3b), SkBits2Float(0xc0a4e0c3), SkBits2Float(0x42a82bcd), SkBits2Float(0x41ec0db8), SkBits2Float(0x427bc56e), SkBits2Float(0x42586a20));
path.lineTo(SkBits2Float(0x423600d6), SkBits2Float(0x421c71bc));
path.cubicTo(SkBits2Float(0x42732394), SkBits2Float(0x41aaa425), SkBits2Float(0x42809f29), SkBits2Float(0xc06e60a8), SkBits2Float(0x425a1cf3), SkBits2Float(0xc1c84447));
path.cubicTo(SkBits2Float(0x4232fb94), SkBits2Float(0xc2395e3c), SkBits2Float(0x41bb9357), SkBits2Float(0xc2700002), SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.close();
path.moveTo(SkBits2Float(0x423600d6), SkBits2Float(0x421c71bc));
path.lineTo(SkBits2Float(0x427bc56c), SkBits2Float(0x42586a22));
path.cubicTo(SkBits2Float(0x426b4cc6), SkBits2Float(0x426b93ad), SkBits2Float(0x4258b1e1), SkBits2Float(0x427ccbca), SkBits2Float(0x42445140), SkBits2Float(0x4285de6e));
path.lineTo(SkBits2Float(0x420dea8b), SkBits2Float(0x42418b9b));
path.cubicTo(SkBits2Float(0x421ca599), SkBits2Float(0x4236be7f), SkBits2Float(0x422a18a8), SkBits2Float(0x422a4be8), SkBits2Float(0x423600d6), SkBits2Float(0x421c71bc));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42445140), SkBits2Float(0x4285de6e));
path.cubicTo(SkBits2Float(0x41a5801a), SkBits2Float(0x42af8153), SkBits2Float(0xc18dfe3b), SkBits2Float(0x42b0c99d), SkBits2Float(0xc23b472e), SkBits2Float(0x42891183));
path.cubicTo(SkBits2Float(0xc297c79f), SkBits2Float(0x4242b2d1), SkBits2Float(0xc2b28961), SkBits2Float(0x414a2ba6), SkBits2Float(0xc2a0659f), SkBits2Float(0xc1ab0f22));
path.cubicTo(SkBits2Float(0xc28e41db), SkBits2Float(0xc25d9a0f), SkBits2Float(0xc2265613), SkBits2Float(0xc29ffd9f), SkBits2Float(0xc0cf8787), SkBits2Float(0xc2a57e12));
path.lineTo(SkBits2Float(0xc09605ca), SkBits2Float(0xc26f4428));
path.cubicTo(SkBits2Float(0xc1f07c7d), SkBits2Float(0xc2674fd1), SkBits2Float(0xc24dac50), SkBits2Float(0xc22031a9), SkBits2Float(0xc267e62b), SkBits2Float(0xc1775074));
path.cubicTo(SkBits2Float(0xc2811003), SkBits2Float(0x411225be), SkBits2Float(0xc25b70c1), SkBits2Float(0x420cbef2), SkBits2Float(0xc20761ad), SkBits2Float(0x42462bd0));
path.cubicTo(SkBits2Float(0xc14d4a68), SkBits2Float(0x427f98ac), SkBits2Float(0x416f472e), SkBits2Float(0x427dbe0b), SkBits2Float(0x420dea8f), SkBits2Float(0x42418b9b));
path.lineTo(SkBits2Float(0x42445140), SkBits2Float(0x4285de6e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp188(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42025498), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42789b1b), SkBits2Float(0xc27fbe84), SkBits2Float(0x42973334), SkBits2Float(0xc2090897));
path.cubicTo(SkBits2Float(0x42b218da), SkBits2Float(0xc092954a), SkBits2Float(0x42a7c71a), SkBits2Float(0x41f1c3b5), SkBits2Float(0x4279a1de), SkBits2Float(0x425ae0d9));
path.lineTo(SkBits2Float(0x42347503), SkBits2Float(0x421e39ac));
path.cubicTo(SkBits2Float(0x427291fe), SkBits2Float(0x41aec4fe), SkBits2Float(0x4280beb1), SkBits2Float(0xc053ed89), SkBits2Float(0x425a9a3a), SkBits2Float(0xc1c61ef1));
path.cubicTo(SkBits2Float(0x4233b713), SkBits2Float(0xc238e018), SkBits2Float(0x41bc6df5), SkBits2Float(0xc2700002), SkBits2Float(0xb7240057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4279a1de), SkBits2Float(0x425ae0d9));
path.cubicTo(SkBits2Float(0x4268e6ce), SkBits2Float(0x426df5b7), SkBits2Float(0x425609c8), SkBits2Float(0x427f0f64), SkBits2Float(0x42416967), SkBits2Float(0x4286ec0f));
path.lineTo(SkBits2Float(0x420bd0d2), SkBits2Float(0x42431170));
path.cubicTo(SkBits2Float(0x421ab9f8), SkBits2Float(0x4238617e), SkBits2Float(0x42285cd4), SkBits2Float(0x422c04e7), SkBits2Float(0x42347505), SkBits2Float(0x421e39ac));
path.lineTo(SkBits2Float(0x4279a1de), SkBits2Float(0x425ae0d9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp189(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb7240057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42025498), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42789b1b), SkBits2Float(0xc27fbe84), SkBits2Float(0x42973334), SkBits2Float(0xc2090897));
path.cubicTo(SkBits2Float(0x42b218da), SkBits2Float(0xc092954a), SkBits2Float(0x42a7c71a), SkBits2Float(0x41f1c3b5), SkBits2Float(0x4279a1de), SkBits2Float(0x425ae0d9));
path.cubicTo(SkBits2Float(0x4268e6ce), SkBits2Float(0x426df5b7), SkBits2Float(0x425609c8), SkBits2Float(0x427f0f64), SkBits2Float(0x42416967), SkBits2Float(0x4286ec0f));
path.lineTo(SkBits2Float(0x420bd0d2), SkBits2Float(0x42431170));
path.cubicTo(SkBits2Float(0x421ab9f8), SkBits2Float(0x4238617e), SkBits2Float(0x42285cd4), SkBits2Float(0x422c04e7), SkBits2Float(0x42347505), SkBits2Float(0x421e39ac));
path.lineTo(SkBits2Float(0x42347503), SkBits2Float(0x421e39ac));
path.cubicTo(SkBits2Float(0x427291fe), SkBits2Float(0x41aec4fe), SkBits2Float(0x4280beb1), SkBits2Float(0xc053ed89), SkBits2Float(0x425a9a3a), SkBits2Float(0xc1c61ef1));
path.cubicTo(SkBits2Float(0x4233b713), SkBits2Float(0xc238e018), SkBits2Float(0x41bc6df5), SkBits2Float(0xc2700002), SkBits2Float(0xb7240057), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42416967), SkBits2Float(0x4286ec0f));
path.cubicTo(SkBits2Float(0x419cd99a), SkBits2Float(0x42b02173), SkBits2Float(0xc19850b8), SkBits2Float(0x42b06117), SkBits2Float(0xc23fac11), SkBits2Float(0x42878a96));
path.cubicTo(SkBits2Float(0xc29997e3), SkBits2Float(0x423d682a), SkBits2Float(0xc2b3208c), SkBits2Float(0x412e025f), SkBits2Float(0xc29f71a3), SkBits2Float(0xc1b8c415));
path.cubicTo(SkBits2Float(0xc28bc2ba), SkBits2Float(0xc26444ae), SkBits2Float(0xc21e5e96), SkBits2Float(0xc2a223df), SkBits2Float(0xc088ac52), SkBits2Float(0xc2a5c7b3));
path.lineTo(SkBits2Float(0xc0459a01), SkBits2Float(0xc26fae99));
path.cubicTo(SkBits2Float(0xc1e4f7d0), SkBits2Float(0xc26a6b5c), SkBits2Float(0xc24a1045), SkBits2Float(0xc225035c), SkBits2Float(0xc266856e), SkBits2Float(0xc18590cd));
path.cubicTo(SkBits2Float(0xc2817d4a), SkBits2Float(0x40fb9475), SkBits2Float(0xc25e0ffd), SkBits2Float(0x4208ebae), SkBits2Float(0xc20a8edd), SkBits2Float(0x4243f69e));
path.cubicTo(SkBits2Float(0xc15c36ee), SkBits2Float(0x427f018f), SkBits2Float(0x4162c57c), SkBits2Float(0x427ea58e), SkBits2Float(0x420bd0d7), SkBits2Float(0x4243116e));
path.lineTo(SkBits2Float(0x42416967), SkBits2Float(0x4286ec0f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp190(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4202b56e), SkBits2Float(0xc2a60000), SkBits2Float(0x427940ff), SkBits2Float(0xc27f4e67), SkBits2Float(0x42976a2d), SkBits2Float(0xc20814ff));
path.cubicTo(SkBits2Float(0x42b233da), SkBits2Float(0xc086dcb5), SkBits2Float(0x42a78518), SkBits2Float(0x41f56a27), SkBits2Float(0x42784037), SkBits2Float(0x425c71a4));
path.lineTo(SkBits2Float(0x4233755d), SkBits2Float(0x421f5b67));
path.cubicTo(SkBits2Float(0x4272328d), SkBits2Float(0x41b16880), SkBits2Float(0x4280d235), SkBits2Float(0xc042fb32), SkBits2Float(0x425ae9b3), SkBits2Float(0xc1c4bebc));
path.cubicTo(SkBits2Float(0x42342efc), SkBits2Float(0xc2388f09), SkBits2Float(0x41bcf9fa), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42784038), SkBits2Float(0x425c71a4));
path.cubicTo(SkBits2Float(0x42675aa4), SkBits2Float(0x426f78d5), SkBits2Float(0x4254535c), SkBits2Float(0x42803f48), SkBits2Float(0x423f8a54), SkBits2Float(0x4287967e));
path.lineTo(SkBits2Float(0x420a7682), SkBits2Float(0x424407da));
path.cubicTo(SkBits2Float(0x42197d0c), SkBits2Float(0x42396aed), SkBits2Float(0x42273e74), SkBits2Float(0x422d1cc3), SkBits2Float(0x4233755f), SkBits2Float(0x421f5b68));
path.lineTo(SkBits2Float(0x42784038), SkBits2Float(0x425c71a4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp191(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4202b56e), SkBits2Float(0xc2a60000), SkBits2Float(0x427940ff), SkBits2Float(0xc27f4e67), SkBits2Float(0x42976a2d), SkBits2Float(0xc20814ff));
path.cubicTo(SkBits2Float(0x42b233da), SkBits2Float(0xc086dcb5), SkBits2Float(0x42a78518), SkBits2Float(0x41f56a27), SkBits2Float(0x42784038), SkBits2Float(0x425c71a4));
path.cubicTo(SkBits2Float(0x42675aa4), SkBits2Float(0x426f78d5), SkBits2Float(0x4254535c), SkBits2Float(0x42803f48), SkBits2Float(0x423f8a54), SkBits2Float(0x4287967e));
path.lineTo(SkBits2Float(0x420a7682), SkBits2Float(0x424407da));
path.cubicTo(SkBits2Float(0x42197d0c), SkBits2Float(0x42396aed), SkBits2Float(0x42273e74), SkBits2Float(0x422d1cc3), SkBits2Float(0x4233755f), SkBits2Float(0x421f5b68));
path.lineTo(SkBits2Float(0x4233755d), SkBits2Float(0x421f5b67));
path.cubicTo(SkBits2Float(0x4272328d), SkBits2Float(0x41b16880), SkBits2Float(0x4280d235), SkBits2Float(0xc042fb32), SkBits2Float(0x425ae9b3), SkBits2Float(0xc1c4bebc));
path.cubicTo(SkBits2Float(0x42342efc), SkBits2Float(0xc2388f09), SkBits2Float(0x41bcf9fa), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423f8a55), SkBits2Float(0x4287967f));
path.cubicTo(SkBits2Float(0x41974ba2), SkBits2Float(0x42b0846d), SkBits2Float(0xc19ee9a3), SkBits2Float(0x42b01937), SkBits2Float(0xc2427547), SkBits2Float(0x42868bae));
path.cubicTo(SkBits2Float(0xc29abade), SkBits2Float(0x4239fc4c), SkBits2Float(0xc2b3780d), SkBits2Float(0x411bee16), SkBits2Float(0xc29ecbab), SkBits2Float(0xc1c17e4f));
path.cubicTo(SkBits2Float(0xc28a1f48), SkBits2Float(0xc26879d6), SkBits2Float(0xc2193674), SkBits2Float(0xc2a376c5), SkBits2Float(0xc0368c8c), SkBits2Float(0xc2a5e6e5));
path.lineTo(SkBits2Float(0xc003f6b5), SkBits2Float(0xc26fdbb6));
path.cubicTo(SkBits2Float(0xc1dd8323), SkBits2Float(0xc26c555a), SkBits2Float(0xc247b1d3), SkBits2Float(0xc2280e0b), SkBits2Float(0xc2659575), SkBits2Float(0xc18bdff2));
path.cubicTo(SkBits2Float(0xc281bc8c), SkBits2Float(0x40e170d0), SkBits2Float(0xc25fb4ae), SkBits2Float(0x42067283), SkBits2Float(0xc20c926e), SkBits2Float(0x42428613));
path.cubicTo(SkBits2Float(0xc165c0b5), SkBits2Float(0x427e99a3), SkBits2Float(0x415abda1), SkBits2Float(0x427f34a6), SkBits2Float(0x420a7686), SkBits2Float(0x424407d8));
path.lineTo(SkBits2Float(0x423f8a55), SkBits2Float(0x4287967f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp192(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4202fa25), SkBits2Float(0xc2a60000), SkBits2Float(0x4279b699), SkBits2Float(0xc27efea4), SkBits2Float(0x429790ee), SkBits2Float(0xc20767f9));
path.cubicTo(SkBits2Float(0x42b24690), SkBits2Float(0xc07d14fa), SkBits2Float(0x42a75587), SkBits2Float(0x41f80076), SkBits2Float(0x427743d2), SkBits2Float(0x425d8c9b));
path.lineTo(SkBits2Float(0x4232bee9), SkBits2Float(0x422027f2));
path.cubicTo(SkBits2Float(0x4271edc7), SkBits2Float(0x41b34741), SkBits2Float(0x4280dfbb), SkBits2Float(0xc036f37a), SkBits2Float(0x425b21bb), SkBits2Float(0xc1c3c49a));
path.cubicTo(SkBits2Float(0x423483ff), SkBits2Float(0xc2385562), SkBits2Float(0x41bd5d54), SkBits2Float(0xc2700000), SkBits2Float(0x36d3ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427743d4), SkBits2Float(0x425d8c98));
path.cubicTo(SkBits2Float(0x4266401a), SkBits2Float(0x427089e5), SkBits2Float(0x42531ae2), SkBits2Float(0x4280c0a0), SkBits2Float(0x423e3514), SkBits2Float(0x42880e64));
path.lineTo(SkBits2Float(0x42097fd1), SkBits2Float(0x4244b531));
path.cubicTo(SkBits2Float(0x42189b26), SkBits2Float(0x423a25ea), SkBits2Float(0x42267233), SkBits2Float(0x422de224), SkBits2Float(0x4232beea), SkBits2Float(0x422027f3));
path.lineTo(SkBits2Float(0x427743d4), SkBits2Float(0x425d8c98));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp193(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e15a675), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e95a67a), SkBits2Float(0xc2a5ffcd), SkBits2Float(0x3ee07980), SkBits2Float(0xc2a5ff68));
path.lineTo(SkBits2Float(0x3ea245bb), SkBits2Float(0xc26fff25));
path.cubicTo(SkBits2Float(0x3e585de0), SkBits2Float(0xc26fffb9), SkBits2Float(0x3dd85f11), SkBits2Float(0xc2700000), SkBits2Float(0x3691e768), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ee07a10), SkBits2Float(0xc2a5ff68));
path.cubicTo(SkBits2Float(0x3ee7f565), SkBits2Float(0xc2a5ff5d), SkBits2Float(0x3eef70d9), SkBits2Float(0xc2a5ff52), SkBits2Float(0x3ef6ec4d), SkBits2Float(0xc2a5ff47));
path.lineTo(SkBits2Float(0x3eb27fdb), SkBits2Float(0xc26ffef6));
path.cubicTo(SkBits2Float(0x3ead1768), SkBits2Float(0xc26fff07), SkBits2Float(0x3ea7aebe), SkBits2Float(0xc26fff17), SkBits2Float(0x3ea24612), SkBits2Float(0xc26fff26));
path.lineTo(SkBits2Float(0x3ee07a10), SkBits2Float(0xc2a5ff68));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp194(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3691e768), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e15a675), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e95a67a), SkBits2Float(0xc2a5ffcd), SkBits2Float(0x3ee07a10), SkBits2Float(0xc2a5ff68));
path.lineTo(SkBits2Float(0x3ef6ec4d), SkBits2Float(0xc2a5ff47));
path.lineTo(SkBits2Float(0x3eb27fdb), SkBits2Float(0xc26ffef6));
path.cubicTo(SkBits2Float(0x3ead1768), SkBits2Float(0xc26fff07), SkBits2Float(0x3ea7aebe), SkBits2Float(0xc26fff17), SkBits2Float(0x3ea245bb), SkBits2Float(0xc26fff25));
path.cubicTo(SkBits2Float(0x3e585de0), SkBits2Float(0xc26fffb9), SkBits2Float(0x3dd85f11), SkBits2Float(0xc2700000), SkBits2Float(0x3691e768), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ef6ec9b), SkBits2Float(0xc2a5ff48));
path.cubicTo(SkBits2Float(0x3f3816c9), SkBits2Float(0xc2a5fe94), SkBits2Float(0x3f74b6e1), SkBits2Float(0xc2a5fd5b), SkBits2Float(0x3f98ab0b), SkBits2Float(0xc2a5fb9d));
path.lineTo(SkBits2Float(0x3f5cb973), SkBits2Float(0xc26ff9a8));
path.cubicTo(SkBits2Float(0x3f30e6e7), SkBits2Float(0xc26ffc2e), SkBits2Float(0x3f05138e), SkBits2Float(0xc26ffdf2), SkBits2Float(0x3eb27fc6), SkBits2Float(0xc26ffef7));
path.lineTo(SkBits2Float(0x3ef6ec9b), SkBits2Float(0xc2a5ff48));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp195(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f0607d9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3f860760), SkBits2Float(0xc2a5fd76), SkBits2Float(0x3fc90825), SkBits2Float(0xc2a5f863));
path.lineTo(SkBits2Float(0x3f9152f7), SkBits2Float(0xc26ff500));
path.cubicTo(SkBits2Float(0x3f41c6b2), SkBits2Float(0xc26ffc55), SkBits2Float(0x3ec1c794), SkBits2Float(0xc26fffff), SkBits2Float(0x36a51f4a), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fc9081a), SkBits2Float(0xc2a5f864));
path.cubicTo(SkBits2Float(0x3fcfbb75), SkBits2Float(0xc2a5f7e2), SkBits2Float(0x3fd66eab), SkBits2Float(0xc2a5f75a), SkBits2Float(0x3fdd21d8), SkBits2Float(0xc2a5f6cb));
path.lineTo(SkBits2Float(0x3f9fdac0), SkBits2Float(0xc26ff2b1));
path.cubicTo(SkBits2Float(0x3f9b02da), SkBits2Float(0xc26ff37f), SkBits2Float(0x3f962add), SkBits2Float(0xc26ff444), SkBits2Float(0x3f9152da), SkBits2Float(0xc26ff500));
path.lineTo(SkBits2Float(0x3fc9081a), SkBits2Float(0xc2a5f864));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp196(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36a51f4a), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3f0607d1), SkBits2Float(0xc2a60000), SkBits2Float(0x3f860758), SkBits2Float(0xc2a5fd76), SkBits2Float(0x3fc9081a), SkBits2Float(0xc2a5f864));

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3fdd21ce), SkBits2Float(0xc2a5f6cb));
path.cubicTo(SkBits2Float(0x4024daa1), SkBits2Float(0xc2a5edc0), SkBits2Float(0x405b1f05), SkBits2Float(0xc2a5de0d), SkBits2Float(0x4088aca3), SkBits2Float(0xc2a5c7b3));
path.lineTo(SkBits2Float(0x40459a01), SkBits2Float(0xc26fae99));
path.cubicTo(SkBits2Float(0x401e66a3), SkBits2Float(0xc26fceed), SkBits2Float(0x3fee57cd), SkBits2Float(0xc26fe5a0), SkBits2Float(0x3f9fdaba), SkBits2Float(0xc26ff2b3));
path.lineTo(SkBits2Float(0x3fdd21ce), SkBits2Float(0xc2a5f6cb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp197(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3fa0bd52), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4020babd), SkBits2Float(0xc2a5f168), SkBits2Float(0x40710446), SkBits2Float(0xc2a5d43c));
path.lineTo(SkBits2Float(0x402e3a94), SkBits2Float(0xc26fc0ba));
path.cubicTo(SkBits2Float(0x3fe86158), SkBits2Float(0xc26feae9), SkBits2Float(0x3f686554), SkBits2Float(0xc2700000), SkBits2Float(0x369bbf59), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4071043c), SkBits2Float(0xc2a5d43c));
path.cubicTo(SkBits2Float(0x40790b78), SkBits2Float(0xc2a5d151), SkBits2Float(0x40808943), SkBits2Float(0xc2a5ce41), SkBits2Float(0x40848cac), SkBits2Float(0xc2a5cb0c));
path.lineTo(SkBits2Float(0x403fa34c), SkBits2Float(0xc26fb371));
path.cubicTo(SkBits2Float(0x4039d5dd), SkBits2Float(0xc26fb815), SkBits2Float(0x40340849), SkBits2Float(0xc26fbc83), SkBits2Float(0x402e3a8d), SkBits2Float(0xc26fc0bb));
path.lineTo(SkBits2Float(0x4071043c), SkBits2Float(0xc2a5d43c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp198(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x369bbf59), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3fa0bd4b), SkBits2Float(0xc2a60000), SkBits2Float(0x4020bab6), SkBits2Float(0xc2a5f168), SkBits2Float(0x4071043c), SkBits2Float(0xc2a5d43c));
path.lineTo(SkBits2Float(0x40710446), SkBits2Float(0xc2a5d43c));
path.cubicTo(SkBits2Float(0x40790b7f), SkBits2Float(0xc2a5d151), SkBits2Float(0x40808945), SkBits2Float(0xc2a5ce41), SkBits2Float(0x40848cac), SkBits2Float(0xc2a5cb0c));
path.lineTo(SkBits2Float(0x403fa34c), SkBits2Float(0xc26fb371));
path.quadTo(SkBits2Float(0x4036ef2a), SkBits2Float(0xc26fba67), SkBits2Float(0x402e3a95), SkBits2Float(0xc26fc0bb));
path.lineTo(SkBits2Float(0x402e3a94), SkBits2Float(0xc26fc0ba));
path.cubicTo(SkBits2Float(0x3fe86158), SkBits2Float(0xc26feae9), SkBits2Float(0x3f686554), SkBits2Float(0xc2700000), SkBits2Float(0x369bbf59), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40848cae), SkBits2Float(0xc2a5cb0c));
path.cubicTo(SkBits2Float(0x40c597bc), SkBits2Float(0xc2a5970c), SkBits2Float(0x41033f43), SkBits2Float(0xc2a53cca), SkBits2Float(0x41238fb3), SkBits2Float(0xc2a4bc74));
path.lineTo(SkBits2Float(0x40ec7963), SkBits2Float(0xc26e2c38));
path.cubicTo(SkBits2Float(0x40bdc13f), SkBits2Float(0xc26ee5c4), SkBits2Float(0x408ed689), SkBits2Float(0xc26f6843), SkBits2Float(0x403fa341), SkBits2Float(0xc26fb372));
path.lineTo(SkBits2Float(0x40848cae), SkBits2Float(0xc2a5cb0c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp199(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ffdfad4), SkBits2Float(0xc2a60000), SkBits2Float(0x407df074), SkBits2Float(0xc2a5db92), SkBits2Float(0x40be4d32), SkBits2Float(0xc2a592c7));
path.lineTo(SkBits2Float(0x40899143), SkBits2Float(0xc26f6217));
path.cubicTo(SkBits2Float(0x40379219), SkBits2Float(0xc26fcb54), SkBits2Float(0x3fb799b8), SkBits2Float(0xc26fffff), SkBits2Float(0x3673fea3), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40be4d37), SkBits2Float(0xc2a592c7));
path.cubicTo(SkBits2Float(0x40c4a257), SkBits2Float(0xc2a58b80), SkBits2Float(0x40caf70c), SkBits2Float(0xc2a583db), SkBits2Float(0x40d14b4e), SkBits2Float(0xc2a57bda));
path.lineTo(SkBits2Float(0x40974c04), SkBits2Float(0xc26f40f2));
path.cubicTo(SkBits2Float(0x4092b8c1), SkBits2Float(0xc26f4c86), SkBits2Float(0x408e252c), SkBits2Float(0xc26f5792), SkBits2Float(0x4089914a), SkBits2Float(0xc26f6219));
path.lineTo(SkBits2Float(0x40be4d37), SkBits2Float(0xc2a592c7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp200(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3673fea3), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3ffdfad4), SkBits2Float(0xc2a60000), SkBits2Float(0x407df074), SkBits2Float(0xc2a5db92), SkBits2Float(0x40be4d37), SkBits2Float(0xc2a592c7));
path.cubicTo(SkBits2Float(0x40c4a257), SkBits2Float(0xc2a58b80), SkBits2Float(0x40caf70c), SkBits2Float(0xc2a583db), SkBits2Float(0x40d14b4e), SkBits2Float(0xc2a57bda));
path.lineTo(SkBits2Float(0x40974c04), SkBits2Float(0xc26f40f2));
path.cubicTo(SkBits2Float(0x4092b8c1), SkBits2Float(0xc26f4c86), SkBits2Float(0x408e252c), SkBits2Float(0xc26f5792), SkBits2Float(0x4089914a), SkBits2Float(0xc26f6219));
path.lineTo(SkBits2Float(0x40899143), SkBits2Float(0xc26f6217));
path.cubicTo(SkBits2Float(0x40379219), SkBits2Float(0xc26fcb54), SkBits2Float(0x3fb799b8), SkBits2Float(0xc26fffff), SkBits2Float(0x3673fea3), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x40d14b4a), SkBits2Float(0xc2a57bdb));
path.cubicTo(SkBits2Float(0x411bf161), SkBits2Float(0xc2a4fa1a), SkBits2Float(0x414ef5ad), SkBits2Float(0xc2a4190e), SkBits2Float(0x4180b83e), SkBits2Float(0xc2a2d9dc));
path.lineTo(SkBits2Float(0x413a19cf), SkBits2Float(0xc26b727f));
path.cubicTo(SkBits2Float(0x41159c04), SkBits2Float(0xc26d3fff), SkBits2Float(0x40e175a8), SkBits2Float(0xc26e855c), SkBits2Float(0x40974c02), SkBits2Float(0xc26f40f4));
path.lineTo(SkBits2Float(0x40d14b4a), SkBits2Float(0xc2a57bdb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp201(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4059d383), SkBits2Float(0xc2a5ffff), SkBits2Float(0x40d9b918), SkBits2Float(0xc2a594d0), SkBits2Float(0x4122e820), SkBits2Float(0xc2a4bf0c));
path.lineTo(SkBits2Float(0x40eb871c), SkBits2Float(0xc26e2ff8));
path.cubicTo(SkBits2Float(0x409d63e0), SkBits2Float(0xc26f6508), SkBits2Float(0x401d76fa), SkBits2Float(0xc2700000), SkBits2Float(0x35f7fd4a), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4122e81e), SkBits2Float(0xc2a4bf0c));
path.cubicTo(SkBits2Float(0x41284f3c), SkBits2Float(0xc2a4a9ac), SkBits2Float(0x412db549), SkBits2Float(0xc2a4933e), SkBits2Float(0x41331a33), SkBits2Float(0xc2a47bbf));
path.lineTo(SkBits2Float(0x410178be), SkBits2Float(0xc26dceac));
path.cubicTo(SkBits2Float(0x40fb24f7), SkBits2Float(0xc26df0a4), SkBits2Float(0x40f356d1), SkBits2Float(0xc26e1114), SkBits2Float(0x40eb871f), SkBits2Float(0xc26e2ff8));
path.lineTo(SkBits2Float(0x4122e81e), SkBits2Float(0xc2a4bf0c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp202(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4059d380), SkBits2Float(0xc2a60000), SkBits2Float(0x40d9b915), SkBits2Float(0xc2a594d0), SkBits2Float(0x4122e81e), SkBits2Float(0xc2a4bf0c));
path.lineTo(SkBits2Float(0x4122e820), SkBits2Float(0xc2a4bf0c));
path.cubicTo(SkBits2Float(0x41284f3d), SkBits2Float(0xc2a4a9ac), SkBits2Float(0x412db54a), SkBits2Float(0xc2a4933e), SkBits2Float(0x41331a33), SkBits2Float(0xc2a47bbf));
path.lineTo(SkBits2Float(0x410178be), SkBits2Float(0xc26dceac));
path.cubicTo(SkBits2Float(0x40fb24f7), SkBits2Float(0xc26df0a4), SkBits2Float(0x40f356d1), SkBits2Float(0xc26e1114), SkBits2Float(0x40eb871f), SkBits2Float(0xc26e2ff8));
path.lineTo(SkBits2Float(0x40eb871c), SkBits2Float(0xc26e2ff8));
path.cubicTo(SkBits2Float(0x409d63e0), SkBits2Float(0xc26f6508), SkBits2Float(0x401d76fa), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41331a39), SkBits2Float(0xc2a47bc0));
path.cubicTo(SkBits2Float(0x41854b40), SkBits2Float(0xc2a2feb5), SkBits2Float(0x41b05576), SkBits2Float(0xc2a06b6c), SkBits2Float(0x41da0834), SkBits2Float(0xc29ccbb1));
path.lineTo(SkBits2Float(0x419d9d10), SkBits2Float(0xc262b148));
path.cubicTo(SkBits2Float(0x417ef0c0), SkBits2Float(0xc267ee96), SkBits2Float(0x4140b6cf), SkBits2Float(0xc26ba7c4), SkBits2Float(0x410178c0), SkBits2Float(0xc26dcead));
path.lineTo(SkBits2Float(0x41331a39), SkBits2Float(0xc2a47bc0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp203(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4087af55), SkBits2Float(0xc2a5ffff), SkBits2Float(0x410795c5), SkBits2Float(0xc2a559a4), SkBits2Float(0x414aa20a), SkBits2Float(0xc2a40e63));
path.lineTo(SkBits2Float(0x41127b4b), SkBits2Float(0xc26d308f));
path.cubicTo(SkBits2Float(0x40c406cd), SkBits2Float(0xc26f0f7b), SkBits2Float(0x40442bc2), SkBits2Float(0xc26fffff), SkBits2Float(0x36b5ff52), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x414aa206), SkBits2Float(0xc2a40e63));
path.cubicTo(SkBits2Float(0x4151559c), SkBits2Float(0xc2a3ed46), SkBits2Float(0x41580726), SkBits2Float(0xc2a3ca86), SkBits2Float(0x415eb67b), SkBits2Float(0xc2a3a622));
path.lineTo(SkBits2Float(0x4120ff4d), SkBits2Float(0xc26c99d6));
path.cubicTo(SkBits2Float(0x411c2a2f), SkBits2Float(0xc26cce74), SkBits2Float(0x41175378), SkBits2Float(0xc26d00b1), SkBits2Float(0x41127b46), SkBits2Float(0xc26d308f));
path.lineTo(SkBits2Float(0x414aa206), SkBits2Float(0xc2a40e63));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp204(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b5ff52), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4087af52), SkBits2Float(0xc2a60000), SkBits2Float(0x410795c2), SkBits2Float(0xc2a559a4), SkBits2Float(0x414aa206), SkBits2Float(0xc2a40e63));
path.lineTo(SkBits2Float(0x414aa20a), SkBits2Float(0xc2a40e63));
path.cubicTo(SkBits2Float(0x4151559f), SkBits2Float(0xc2a3ed46), SkBits2Float(0x41580727), SkBits2Float(0xc2a3ca86), SkBits2Float(0x415eb67b), SkBits2Float(0xc2a3a622));
path.lineTo(SkBits2Float(0x4120ff4d), SkBits2Float(0xc26c99d6));
path.cubicTo(SkBits2Float(0x411c2a31), SkBits2Float(0xc26cce74), SkBits2Float(0x4117537b), SkBits2Float(0xc26d00b1), SkBits2Float(0x41127b4b), SkBits2Float(0xc26d308f));
path.lineTo(SkBits2Float(0x41127b46), SkBits2Float(0xc26d308f));
path.cubicTo(SkBits2Float(0x40c406c6), SkBits2Float(0xc26f0f7b), SkBits2Float(0x40442bbb), SkBits2Float(0xc26fffff), SkBits2Float(0x36b5ff52), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x415eb680), SkBits2Float(0xc2a3a623));
path.cubicTo(SkBits2Float(0x41a59721), SkBits2Float(0xc2a157ad), SkBits2Float(0x41da77ab), SkBits2Float(0xc29d5c25), SkBits2Float(0x420662d7), SkBits2Float(0xc297cafd));
path.lineTo(SkBits2Float(0x41c24b0d), SkBits2Float(0xc25b75ac));
path.cubicTo(SkBits2Float(0x419deda5), SkBits2Float(0xc2638226), SkBits2Float(0x416f6860), SkBits2Float(0xc269442a), SkBits2Float(0x4120ff4a), SkBits2Float(0xc26c99d9));
path.lineTo(SkBits2Float(0x415eb680), SkBits2Float(0xc2a3a623));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp205(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40a2e582), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4122b94f), SkBits2Float(0xc2a51039), SkBits2Float(0x4172cca0), SkBits2Float(0xc2a333b4));
path.lineTo(SkBits2Float(0x412f847d), SkBits2Float(0xc26bf464));
path.cubicTo(SkBits2Float(0x40eb4376), SkBits2Float(0xc26ea556), SkBits2Float(0x406b836d), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4172cc9b), SkBits2Float(0xc2a333b4));
path.cubicTo(SkBits2Float(0x417acd1a), SkBits2Float(0xc2a30415), SkBits2Float(0x41816508), SkBits2Float(0xc2a2d21d), SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcb));
path.lineTo(SkBits2Float(0x4140d724), SkBits2Float(0xc26b1ba8));
path.cubicTo(SkBits2Float(0x413b139d), SkBits2Float(0xc26b674c), SkBits2Float(0x41354d54), SkBits2Float(0xc26baf8b), SkBits2Float(0x412f847c), SkBits2Float(0xc26bf463));
path.lineTo(SkBits2Float(0x4172cc9b), SkBits2Float(0xc2a333b4));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp206(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40a2e57f), SkBits2Float(0xc2a60000), SkBits2Float(0x4122b94c), SkBits2Float(0xc2a51039), SkBits2Float(0x4172cc9b), SkBits2Float(0xc2a333b4));
path.lineTo(SkBits2Float(0x4172cca0), SkBits2Float(0xc2a333b4));
path.cubicTo(SkBits2Float(0x417acd1d), SkBits2Float(0xc2a30415), SkBits2Float(0x41816509), SkBits2Float(0xc2a2d21d), SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcb));
path.lineTo(SkBits2Float(0x4140d724), SkBits2Float(0xc26b1ba8));
path.cubicTo(SkBits2Float(0x413b139d), SkBits2Float(0xc26b674c), SkBits2Float(0x41354d54), SkBits2Float(0xc26baf8b), SkBits2Float(0x412f847c), SkBits2Float(0xc26bf463));
path.lineTo(SkBits2Float(0x412f847d), SkBits2Float(0xc26bf464));
path.cubicTo(SkBits2Float(0x40eb4376), SkBits2Float(0xc26ea556), SkBits2Float(0x406b836d), SkBits2Float(0xc2700000), SkBits2Float(0x36b5ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcc));
path.cubicTo(SkBits2Float(0x41c61a92), SkBits2Float(0xc29f4c69), SkBits2Float(0x42023dd6), SkBits2Float(0xc299958f), SkBits2Float(0x421f3a98), SkBits2Float(0xc291a994));
path.lineTo(SkBits2Float(0x41e635e1), SkBits2Float(0xc25298a5));
path.cubicTo(SkBits2Float(0x41bc4d11), SkBits2Float(0xc25e0caa), SkBits2Float(0x418f3524), SkBits2Float(0xc2664fa2), SkBits2Float(0x4140d729), SkBits2Float(0xc26b1ba9));
path.lineTo(SkBits2Float(0x4185619b), SkBits2Float(0xc2a29dcc));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp207(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40c39389), SkBits2Float(0xc2a60000), SkBits2Float(0x414346f4), SkBits2Float(0xc2a4a65f), SkBits2Float(0x419158cf), SkBits2Float(0xc2a1f965));
path.lineTo(SkBits2Float(0x415223e0), SkBits2Float(0xc26a2df8));
path.cubicTo(SkBits2Float(0x410d2a0c), SkBits2Float(0xc26e0c4b), SkBits2Float(0x408d616c), SkBits2Float(0xc2700000), SkBits2Float(0x35bbfd46), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x419158d0), SkBits2Float(0xc2a1f965));
path.cubicTo(SkBits2Float(0x41961cea), SkBits2Float(0xc2a1b4f6), SkBits2Float(0x419addf6), SkBits2Float(0xc2a16d2c), SkBits2Float(0x419f9bbb), SkBits2Float(0xc2a12207));
path.lineTo(SkBits2Float(0x4166c251), SkBits2Float(0xc268f69a));
path.cubicTo(SkBits2Float(0x415fe778), SkBits2Float(0xc269633e), SkBits2Float(0x415907e2), SkBits2Float(0xc269cb09), SkBits2Float(0x415223e0), SkBits2Float(0xc26a2df8));
path.lineTo(SkBits2Float(0x419158d0), SkBits2Float(0xc2a1f965));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp208(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40c39389), SkBits2Float(0xc2a60000), SkBits2Float(0x414346f4), SkBits2Float(0xc2a4a65f), SkBits2Float(0x419158d0), SkBits2Float(0xc2a1f965));
path.cubicTo(SkBits2Float(0x41961cea), SkBits2Float(0xc2a1b4f6), SkBits2Float(0x419addf6), SkBits2Float(0xc2a16d2c), SkBits2Float(0x419f9bbb), SkBits2Float(0xc2a12207));
path.lineTo(SkBits2Float(0x4166c251), SkBits2Float(0xc268f69a));
path.cubicTo(SkBits2Float(0x415fe778), SkBits2Float(0xc269633e), SkBits2Float(0x415907e2), SkBits2Float(0xc269cb09), SkBits2Float(0x415223e0), SkBits2Float(0xc26a2df8));
path.cubicTo(SkBits2Float(0x410d2a0c), SkBits2Float(0xc26e0c4b), SkBits2Float(0x408d616c), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x419f9bbc), SkBits2Float(0xc2a12208));
path.cubicTo(SkBits2Float(0x41eca53e), SkBits2Float(0xc29c5d1a), SkBits2Float(0x421ad1be), SkBits2Float(0xc2942e2b), SkBits2Float(0x423b8fe1), SkBits2Float(0xc288f8a3));
path.lineTo(SkBits2Float(0x42079647), SkBits2Float(0xc24607dc));
path.cubicTo(SkBits2Float(0x41dfd5cc), SkBits2Float(0xc2563c94), SkBits2Float(0x41ab11aa), SkBits2Float(0xc2621167), SkBits2Float(0x4166c24e), SkBits2Float(0xc268f69b));
path.lineTo(SkBits2Float(0x419f9bbc), SkBits2Float(0xc2a12208));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp209(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40e86425), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4167e385), SkBits2Float(0xc2a41801), SkBits2Float(0x41ac0ecd), SkBits2Float(0xc2a05484));
path.lineTo(SkBits2Float(0x4178c21d), SkBits2Float(0xc267cd79));
path.cubicTo(SkBits2Float(0x4127a168), SkBits2Float(0xc26d3e79), SkBits2Float(0x40a7fe68), SkBits2Float(0xc2700000), SkBits2Float(0x3673fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41ac0ecb), SkBits2Float(0xc2a05485));
path.cubicTo(SkBits2Float(0x41b1a941), SkBits2Float(0xc29ff44e), SkBits2Float(0x41b73ea0), SkBits2Float(0xc29f8f65), SkBits2Float(0x41bcce84), SkBits2Float(0xc29f25d1));
path.lineTo(SkBits2Float(0x41887c9d), SkBits2Float(0xc26617d6));
path.cubicTo(SkBits2Float(0x4184774a), SkBits2Float(0xc266b07c), SkBits2Float(0x41806e06), SkBits2Float(0xc2674260), SkBits2Float(0x4178c21e), SkBits2Float(0xc267cd7a));
path.lineTo(SkBits2Float(0x41ac0ecb), SkBits2Float(0xc2a05485));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp210(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3673fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x40e86421), SkBits2Float(0xc2a60000), SkBits2Float(0x4167e381), SkBits2Float(0xc2a41801), SkBits2Float(0x41ac0eca), SkBits2Float(0xc2a05484));
path.lineTo(SkBits2Float(0x41ac0ecd), SkBits2Float(0xc2a05484));
path.lineTo(SkBits2Float(0x4178c21e), SkBits2Float(0xc267cd7a));
path.lineTo(SkBits2Float(0x41ac0ecb), SkBits2Float(0xc2a05485));
path.cubicTo(SkBits2Float(0x41b1a941), SkBits2Float(0xc29ff44e), SkBits2Float(0x41b73ea0), SkBits2Float(0xc29f8f65), SkBits2Float(0x41bcce84), SkBits2Float(0xc29f25d1));
path.lineTo(SkBits2Float(0x41887c9d), SkBits2Float(0xc26617d6));
path.cubicTo(SkBits2Float(0x4184774a), SkBits2Float(0xc266b07c), SkBits2Float(0x41806e06), SkBits2Float(0xc2674260), SkBits2Float(0x4178c21d), SkBits2Float(0xc267cd79));
path.cubicTo(SkBits2Float(0x4127a168), SkBits2Float(0xc26d3e79), SkBits2Float(0x40a7fe68), SkBits2Float(0xc2700000), SkBits2Float(0x3673fea3), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41bcce83), SkBits2Float(0xc29f25d2));
path.cubicTo(SkBits2Float(0x420ba3b4), SkBits2Float(0xc2987080), SkBits2Float(0x42357f09), SkBits2Float(0xc28cfcb1), SkBits2Float(0x42592f07), SkBits2Float(0xc27b1ba7));
path.lineTo(SkBits2Float(0x421d0012), SkBits2Float(0xc235861c));
path.cubicTo(SkBits2Float(0x420333bc), SkBits2Float(0xc24bd636), SkBits2Float(0x41c9e36e), SkBits2Float(0xc25c64f6), SkBits2Float(0x41887c9c), SkBits2Float(0xc26617d7));
path.lineTo(SkBits2Float(0x41bcce83), SkBits2Float(0xc29f25d2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp211(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x411e5541), SkBits2Float(0xc2a5ffff), SkBits2Float(0x419db1ee), SkBits2Float(0xc2a275ef), SkBits2Float(0x41e7e0a3), SkBits2Float(0xc29b8c98));
path.lineTo(SkBits2Float(0x41a79f51), SkBits2Float(0xc260e3f1));
path.cubicTo(SkBits2Float(0x4163fe32), SkBits2Float(0xc26ae208), SkBits2Float(0x40e4ea54), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41e7e0a8), SkBits2Float(0xc29b8c98));
path.cubicTo(SkBits2Float(0x41ef46bb), SkBits2Float(0xc29adc20), SkBits2Float(0x41f6a013), SkBits2Float(0xc29a2338), SkBits2Float(0x41fdebc8), SkBits2Float(0xc29961f8));
path.lineTo(SkBits2Float(0x41b78eb0), SkBits2Float(0xc25dc215));
path.cubicTo(SkBits2Float(0x41b2488a), SkBits2Float(0xc25ed97a), SkBits2Float(0x41acf889), SkBits2Float(0xc25fe4cd), SkBits2Float(0x41a79f51), SkBits2Float(0xc260e3f1));
path.lineTo(SkBits2Float(0x41e7e0a8), SkBits2Float(0xc29b8c98));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp212(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea3), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x411e5541), SkBits2Float(0xc2a5ffff), SkBits2Float(0x419db1ee), SkBits2Float(0xc2a275ef), SkBits2Float(0x41e7e0a8), SkBits2Float(0xc29b8c98));
path.cubicTo(SkBits2Float(0x41ef46bb), SkBits2Float(0xc29adc20), SkBits2Float(0x41f6a013), SkBits2Float(0xc29a2338), SkBits2Float(0x41fdebc8), SkBits2Float(0xc29961f8));
path.lineTo(SkBits2Float(0x41b78eb0), SkBits2Float(0xc25dc215));
path.cubicTo(SkBits2Float(0x41b2488a), SkBits2Float(0xc25ed97a), SkBits2Float(0x41acf889), SkBits2Float(0xc25fe4cd), SkBits2Float(0x41a79f51), SkBits2Float(0xc260e3f1));
path.cubicTo(SkBits2Float(0x4163fe32), SkBits2Float(0xc26ae208), SkBits2Float(0x40e4ea54), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea3), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41fdebc9), SkBits2Float(0xc29961f9));
path.cubicTo(SkBits2Float(0x423a7ccd), SkBits2Float(0xc28d1085), SkBits2Float(0x426d8f8d), SkBits2Float(0xc270b4b0), SkBits2Float(0x4288fa0c), SkBits2Float(0xc23b8bbf));
path.lineTo(SkBits2Float(0x424609e8), SkBits2Float(0xc207934a));
path.cubicTo(SkBits2Float(0x422bbb0d), SkBits2Float(0xc22e0114), SkBits2Float(0x4206cf6b), SkBits2Float(0xc24bf2e1), SkBits2Float(0x41b78eaf), SkBits2Float(0xc25dc216));
path.lineTo(SkBits2Float(0x41fdebc9), SkBits2Float(0xc29961f9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp213(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4151cd59), SkBits2Float(0xc2a5ffff), SkBits2Float(0x41d04f3f), SkBits2Float(0xc29fc954), SkBits2Float(0x4216e058), SkBits2Float(0xc293de54));
path.lineTo(SkBits2Float(0x41da226b), SkBits2Float(0xc255c926));
path.cubicTo(SkBits2Float(0x419695d1), SkBits2Float(0xc267043d), SkBits2Float(0x4117aa0a), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4216e057), SkBits2Float(0xc293de54));
path.cubicTo(SkBits2Float(0x421b86ea), SkBits2Float(0xc292aea0), SkBits2Float(0x42201eff), SkBits2Float(0xc29170ed), SkBits2Float(0x4224a79b), SkBits2Float(0xc290257e));
path.lineTo(SkBits2Float(0x41ee0e15), SkBits2Float(0xc2506790));
path.cubicTo(SkBits2Float(0x41e78019), SkBits2Float(0xc25246bf), SkBits2Float(0x41e0dbbc), SkBits2Float(0xc2541212), SkBits2Float(0x41da226b), SkBits2Float(0xc255c927));
path.lineTo(SkBits2Float(0x4216e057), SkBits2Float(0xc293de54));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp214(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4151cd58), SkBits2Float(0xc2a60000), SkBits2Float(0x41d04f3d), SkBits2Float(0xc29fc954), SkBits2Float(0x4216e057), SkBits2Float(0xc293de54));
path.lineTo(SkBits2Float(0x4216e058), SkBits2Float(0xc293de54));
path.cubicTo(SkBits2Float(0x421b86eb), SkBits2Float(0xc292aea0), SkBits2Float(0x42201eff), SkBits2Float(0xc29170ed), SkBits2Float(0x4224a79b), SkBits2Float(0xc290257e));
path.lineTo(SkBits2Float(0x41ee0e15), SkBits2Float(0xc2506790));
path.cubicTo(SkBits2Float(0x41e78019), SkBits2Float(0xc25246bf), SkBits2Float(0x41e0dbbc), SkBits2Float(0xc2541212), SkBits2Float(0x41da226b), SkBits2Float(0xc255c927));
path.lineTo(SkBits2Float(0x41da226b), SkBits2Float(0xc255c926));
path.cubicTo(SkBits2Float(0x419695d1), SkBits2Float(0xc267043d), SkBits2Float(0x4117aa0a), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4224a79b), SkBits2Float(0xc290257f));
path.cubicTo(SkBits2Float(0x426f06c3), SkBits2Float(0xc275d105), SkBits2Float(0x42930d85), SkBits2Float(0xc2303df6), SkBits2Float(0x429f3103), SkBits2Float(0xc1bc373f));
path.lineTo(SkBits2Float(0x42662806), SkBits2Float(0xc1880f44));
path.cubicTo(SkBits2Float(0x42549b44), SkBits2Float(0xc1fececc), SkBits2Float(0x422cca4c), SkBits2Float(0xc231b2de), SkBits2Float(0x41ee0e18), SkBits2Float(0xc2506792));
path.lineTo(SkBits2Float(0x4224a79b), SkBits2Float(0xc290257f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp215(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41741cf0), SkBits2Float(0xc2a60000), SkBits2Float(0x41f1c060), SkBits2Float(0xc29d96da), SkBits2Float(0x422cf7a2), SkBits2Float(0xc28db11c));
path.lineTo(SkBits2Float(0x41fa12be), SkBits2Float(0xc24cdb0d));
path.cubicTo(SkBits2Float(0x41aec295), SkBits2Float(0xc263d704), SkBits2Float(0x413077a0), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x422cf7a1), SkBits2Float(0xc28db11c));
path.cubicTo(SkBits2Float(0x423224e7), SkBits2Float(0xc28c1ca8), SkBits2Float(0x42373bc3), SkBits2Float(0xc28a7620), SkBits2Float(0x423c3abd), SkBits2Float(0xc288bdfd));
path.lineTo(SkBits2Float(0x420811ca), SkBits2Float(0xc245b313));
path.cubicTo(SkBits2Float(0x4204753a), SkBits2Float(0xc2482f6b), SkBits2Float(0x4200c767), SkBits2Float(0xc24a924f), SkBits2Float(0x41fa12c1), SkBits2Float(0xc24cdb0e));
path.lineTo(SkBits2Float(0x422cf7a1), SkBits2Float(0xc28db11c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp216(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41741cef), SkBits2Float(0xc2a60000), SkBits2Float(0x41f1c05e), SkBits2Float(0xc29d96da), SkBits2Float(0x422cf7a1), SkBits2Float(0xc28db11c));
path.lineTo(SkBits2Float(0x422cf7a2), SkBits2Float(0xc28db11c));
path.cubicTo(SkBits2Float(0x423224e8), SkBits2Float(0xc28c1ca8), SkBits2Float(0x42373bc3), SkBits2Float(0xc28a7620), SkBits2Float(0x423c3abd), SkBits2Float(0xc288bdfd));
path.lineTo(SkBits2Float(0x420811ca), SkBits2Float(0xc245b313));
path.cubicTo(SkBits2Float(0x4204753a), SkBits2Float(0xc2482f6b), SkBits2Float(0x4200c767), SkBits2Float(0xc24a924f), SkBits2Float(0x41fa12c1), SkBits2Float(0xc24cdb0e));
path.lineTo(SkBits2Float(0x41fa12be), SkBits2Float(0xc24cdb0d));
path.cubicTo(SkBits2Float(0x41aec295), SkBits2Float(0xc263d704), SkBits2Float(0x413077a0), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423c3abe), SkBits2Float(0xc288bdfe));
path.cubicTo(SkBits2Float(0x42874551), SkBits2Float(0xc258d4f5), SkBits2Float(0x42a17ace), SkBits2Float(0xc1fc3ce7), SkBits2Float(0x42a57844), SkBits2Float(0xc0d41d22));
path.lineTo(SkBits2Float(0x426f3bc1), SkBits2Float(0xc09955d3));
path.cubicTo(SkBits2Float(0x426976f3), SkBits2Float(0xc1b65735), SkBits2Float(0x4243927c), SkBits2Float(0xc21cbef5), SkBits2Float(0x420811ca), SkBits2Float(0xc245b314));
path.lineTo(SkBits2Float(0x423c3abe), SkBits2Float(0xc288bdfe));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp217(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4188e880), SkBits2Float(0xc2a60000), SkBits2Float(0x42073c1a), SkBits2Float(0xc29b6b86), SkBits2Float(0x423f3295), SkBits2Float(0xc287b573));
path.lineTo(SkBits2Float(0x420a3712), SkBits2Float(0xc2443499));
path.cubicTo(SkBits2Float(0x41c3852b), SkBits2Float(0xc260b421), SkBits2Float(0x4145f08c), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423f3294), SkBits2Float(0xc287b572));
path.cubicTo(SkBits2Float(0x4244c015), SkBits2Float(0xc285c0c3), SkBits2Float(0x424a2e84), SkBits2Float(0xc283b664), SkBits2Float(0x424f7bec), SkBits2Float(0xc281970f));
path.lineTo(SkBits2Float(0x4215fd0e), SkBits2Float(0xc23b5bf1));
path.cubicTo(SkBits2Float(0x421227cb), SkBits2Float(0xc23e6d7a), SkBits2Float(0x420e3aa9), SkBits2Float(0xc24160b8), SkBits2Float(0x420a3713), SkBits2Float(0xc2443498));
path.lineTo(SkBits2Float(0x423f3294), SkBits2Float(0xc287b572));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp218(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4188e880), SkBits2Float(0xc2a60000), SkBits2Float(0x42073c1a), SkBits2Float(0xc29b6b86), SkBits2Float(0x423f3295), SkBits2Float(0xc287b573));
path.lineTo(SkBits2Float(0x424f7bec), SkBits2Float(0xc281970f));
path.lineTo(SkBits2Float(0x4215fd0e), SkBits2Float(0xc23b5bf1));
path.cubicTo(SkBits2Float(0x421227cb), SkBits2Float(0xc23e6d7a), SkBits2Float(0x420e3aa9), SkBits2Float(0xc24160b8), SkBits2Float(0x420a3713), SkBits2Float(0xc2443498));
path.lineTo(SkBits2Float(0x420a3712), SkBits2Float(0xc2443499));
path.cubicTo(SkBits2Float(0x41c3852b), SkBits2Float(0xc260b421), SkBits2Float(0x4145f08c), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x424f7bed), SkBits2Float(0xc281970f));
path.cubicTo(SkBits2Float(0x42939bdb), SkBits2Float(0xc23cf22a), SkBits2Float(0x42aabb70), SkBits2Float(0xc19e30f8), SkBits2Float(0x42a530dd), SkBits2Float(0x4102f5b1));
path.lineTo(SkBits2Float(0x426ed486), SkBits2Float(0x40bd56e4));
path.cubicTo(SkBits2Float(0x4276d778), SkBits2Float(0xc164b5d6), SkBits2Float(0x4255690c), SkBits2Float(0xc2089663), SkBits2Float(0x4215fd0d), SkBits2Float(0xc23b5bf2));
path.lineTo(SkBits2Float(0x424f7bed), SkBits2Float(0xc281970f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp219(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4198fc97), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4216a3e3), SkBits2Float(0xc298caff), SkBits2Float(0x4251e7a7), SkBits2Float(0xc2809c9b));
path.lineTo(SkBits2Float(0x4217bd0d), SkBits2Float(0xc239f1d8));
path.cubicTo(SkBits2Float(0x41d9cb04), SkBits2Float(0xc25ce7ce), SkBits2Float(0x415d2f7f), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4251e7a7), SkBits2Float(0xc2809c9c));
path.cubicTo(SkBits2Float(0x4257c623), SkBits2Float(0xc27c6f1e), SkBits2Float(0x425d7a38), SkBits2Float(0xc27771f7), SkBits2Float(0x42630157), SkBits2Float(0xc27243fd));
path.lineTo(SkBits2Float(0x422419a4), SkBits2Float(0xc22f21bb));
path.cubicTo(SkBits2Float(0x42201aab), SkBits2Float(0xc232e046), SkBits2Float(0x421bfb30), SkBits2Float(0xc2367b84), SkBits2Float(0x4217bd0d), SkBits2Float(0xc239f1d8));
path.lineTo(SkBits2Float(0x4251e7a7), SkBits2Float(0xc2809c9c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp220(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4198fc97), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4216a3e3), SkBits2Float(0xc298caff), SkBits2Float(0x4251e7a7), SkBits2Float(0xc2809c9c));
path.cubicTo(SkBits2Float(0x4257c623), SkBits2Float(0xc27c6f1e), SkBits2Float(0x425d7a38), SkBits2Float(0xc27771f7), SkBits2Float(0x42630157), SkBits2Float(0xc27243fd));
path.lineTo(SkBits2Float(0x422419a4), SkBits2Float(0xc22f21bb));
path.cubicTo(SkBits2Float(0x42201aab), SkBits2Float(0xc232e046), SkBits2Float(0x421bfb30), SkBits2Float(0xc2367b84), SkBits2Float(0x4217bd0d), SkBits2Float(0xc239f1d8));
path.cubicTo(SkBits2Float(0x41d9cb04), SkBits2Float(0xc25ce7ce), SkBits2Float(0x415d2f7f), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42630157), SkBits2Float(0xc27243ff));
path.cubicTo(SkBits2Float(0x429f78af), SkBits2Float(0xc21c1e80), SkBits2Float(0x42b11918), SkBits2Float(0xc0cad7ee), SkBits2Float(0x429f0274), SkBits2Float(0x41bea8f4));
path.lineTo(SkBits2Float(0x4265e4b4), SkBits2Float(0x4189d394));
path.cubicTo(SkBits2Float(0x428005cc), SkBits2Float(0xc092a249), SkBits2Float(0x42668fa3), SkBits2Float(0xc1e1b6e5), SkBits2Float(0x422419a4), SkBits2Float(0xc22f21bb));
path.lineTo(SkBits2Float(0x42630157), SkBits2Float(0xc27243ff));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp221(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ae0130), SkBits2Float(0xc2a5ffff), SkBits2Float(0x422a8737), SkBits2Float(0xc294ec91), SkBits2Float(0x42689b67), SkBits2Float(0xc26ce46c));
path.lineTo(SkBits2Float(0x42282651), SkBits2Float(0xc22b3f58));
path.cubicTo(SkBits2Float(0x41f68bfb), SkBits2Float(0xc2574fdc), SkBits2Float(0x417b92b3), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42689b68), SkBits2Float(0xc26ce46d));
path.cubicTo(SkBits2Float(0x426ebcd2), SkBits2Float(0xc266df67), SkBits2Float(0x4274a1d2), SkBits2Float(0xc2609e09), SkBits2Float(0x427a4701), SkBits2Float(0xc25a23f2));
path.lineTo(SkBits2Float(0x4234ec64), SkBits2Float(0xc21db11e));
path.cubicTo(SkBits2Float(0x4230d7ae), SkBits2Float(0xc2225fbc), SkBits2Float(0x422c94d6), SkBits2Float(0xc226e55a), SkBits2Float(0x42282652), SkBits2Float(0xc22b3f58));
path.lineTo(SkBits2Float(0x42689b68), SkBits2Float(0xc26ce46d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp222(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ae0130), SkBits2Float(0xc2a5ffff), SkBits2Float(0x422a8737), SkBits2Float(0xc294ec91), SkBits2Float(0x42689b68), SkBits2Float(0xc26ce46d));
path.cubicTo(SkBits2Float(0x426ebcd2), SkBits2Float(0xc266df67), SkBits2Float(0x4274a1d2), SkBits2Float(0xc2609e09), SkBits2Float(0x427a4701), SkBits2Float(0xc25a23f2));
path.lineTo(SkBits2Float(0x4234ec64), SkBits2Float(0xc21db11e));
path.cubicTo(SkBits2Float(0x4230d7ae), SkBits2Float(0xc2225fbc), SkBits2Float(0x422c94d6), SkBits2Float(0xc226e55a), SkBits2Float(0x42282651), SkBits2Float(0xc22b3f58));
path.cubicTo(SkBits2Float(0x41f68bfb), SkBits2Float(0xc2574fdc), SkBits2Float(0x417b92b3), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427a4702), SkBits2Float(0xc25a23f2));
path.cubicTo(SkBits2Float(0x42ac7185), SkBits2Float(0xc1db2f83), SkBits2Float(0x42b35ed0), SkBits2Float(0x413e447a), SkBits2Float(0x428e4a3d), SkBits2Float(0x422afde8));
path.lineTo(SkBits2Float(0x424db871), SkBits2Float(0x41f73799));
path.cubicTo(SkBits2Float(0x4281aa54), SkBits2Float(0x41098afa), SkBits2Float(0x427950da), SkBits2Float(0xc19e728d), SkBits2Float(0x4234ec66), SkBits2Float(0xc21db120));
path.lineTo(SkBits2Float(0x427a4702), SkBits2Float(0xc25a23f2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp223(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c50a2c), SkBits2Float(0xc2a60000), SkBits2Float(0x423ff37f), SkBits2Float(0xc2901f4e), SkBits2Float(0x427f077c), SkBits2Float(0xc25490c6));
path.lineTo(SkBits2Float(0x42385bc5), SkBits2Float(0xc219a96d));
path.cubicTo(SkBits2Float(0x420ac287), SkBits2Float(0xc2505e9c), SkBits2Float(0x418e7039), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427f077b), SkBits2Float(0xc25490c6));
path.cubicTo(SkBits2Float(0x42829e52), SkBits2Float(0xc24d1e28), SkBits2Float(0x42858ec1), SkBits2Float(0xc24566d6), SkBits2Float(0x428852e3), SkBits2Float(0xc23d7081));
path.lineTo(SkBits2Float(0x42451839), SkBits2Float(0xc208f1b7));
path.cubicTo(SkBits2Float(0x4241186a), SkBits2Float(0xc20eb335), SkBits2Float(0x423cd88e), SkBits2Float(0xc2144725), SkBits2Float(0x42385bc4), SkBits2Float(0xc219a96c));
path.lineTo(SkBits2Float(0x427f077b), SkBits2Float(0xc25490c6));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp224(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c50a2c), SkBits2Float(0xc2a60000), SkBits2Float(0x423ff37f), SkBits2Float(0xc2901f4e), SkBits2Float(0x427f077c), SkBits2Float(0xc25490c6));
path.lineTo(SkBits2Float(0x428852e3), SkBits2Float(0xc23d7081));
path.lineTo(SkBits2Float(0x42451839), SkBits2Float(0xc208f1b7));
path.cubicTo(SkBits2Float(0x4241186a), SkBits2Float(0xc20eb335), SkBits2Float(0x423cd88e), SkBits2Float(0xc2144725), SkBits2Float(0x42385bc4), SkBits2Float(0xc219a96c));
path.lineTo(SkBits2Float(0x42385bc5), SkBits2Float(0xc219a96d));
path.cubicTo(SkBits2Float(0x420ac287), SkBits2Float(0xc2505e9c), SkBits2Float(0x418e7039), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428852e3), SkBits2Float(0xc23d7081));
path.cubicTo(SkBits2Float(0x42b71f8a), SkBits2Float(0xc15aea65), SkBits2Float(0x42adb77f), SkBits2Float(0x42002593), SkBits2Float(0x42645e8b), SkBits2Float(0x4270faee));
path.lineTo(SkBits2Float(0x42251616), SkBits2Float(0x422e33d9));
path.cubicTo(SkBits2Float(0x427b2825), SkBits2Float(0x41b945be), SkBits2Float(0x428460d4), SkBits2Float(0xc11e4099), SkBits2Float(0x4245183a), SkBits2Float(0xc208f1b8));
path.lineTo(SkBits2Float(0x428852e3), SkBits2Float(0xc23d7081));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp225(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d8749b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4251a993), SkBits2Float(0xc28b9f9f), SkBits2Float(0x4287e789), SkBits2Float(0xc23ea40d));
path.lineTo(SkBits2Float(0x42447d05), SkBits2Float(0xc209d00a));
path.cubicTo(SkBits2Float(0x4217902d), SkBits2Float(0xc249dd89), SkBits2Float(0x419c7951), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4287e78a), SkBits2Float(0xc23ea40e));
path.cubicTo(SkBits2Float(0x428af3dc), SkBits2Float(0xc235f2f3), SkBits2Float(0x428dca5e), SkBits2Float(0xc22cf844), SkBits2Float(0x4290688d), SkBits2Float(0xc223bbef));
path.lineTo(SkBits2Float(0x4250c881), SkBits2Float(0xc1ecb95a));
path.cubicTo(SkBits2Float(0x424cff91), SkBits2Float(0xc1fa13ac), SkBits2Float(0x4248e532), SkBits2Float(0xc2038788), SkBits2Float(0x42447d06), SkBits2Float(0xc209d00a));
path.lineTo(SkBits2Float(0x4287e78a), SkBits2Float(0xc23ea40e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp226(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d8749b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4251a993), SkBits2Float(0xc28b9f9f), SkBits2Float(0x4287e78a), SkBits2Float(0xc23ea40e));
path.cubicTo(SkBits2Float(0x428af3dc), SkBits2Float(0xc235f2f3), SkBits2Float(0x428dca5e), SkBits2Float(0xc22cf844), SkBits2Float(0x4290688d), SkBits2Float(0xc223bbef));
path.lineTo(SkBits2Float(0x4250c881), SkBits2Float(0xc1ecb95a));
path.cubicTo(SkBits2Float(0x424cff91), SkBits2Float(0xc1fa13ac), SkBits2Float(0x4248e532), SkBits2Float(0xc2038788), SkBits2Float(0x42447d05), SkBits2Float(0xc209d00a));
path.cubicTo(SkBits2Float(0x4217902d), SkBits2Float(0xc249dd89), SkBits2Float(0x419c7951), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4290688d), SkBits2Float(0xc223bbef));
path.cubicTo(SkBits2Float(0x42bd187d), SkBits2Float(0xbfc2a74a), SkBits2Float(0x42a250ed), SkBits2Float(0x42421cbf), SkBits2Float(0x42287a28), SkBits2Float(0x428f09b7));
path.lineTo(SkBits2Float(0x41f394da), SkBits2Float(0x424ecd48));
path.cubicTo(SkBits2Float(0x426aac8a), SkBits2Float(0x420c527b), SkBits2Float(0x4288b219), SkBits2Float(0xbf8cb68f), SkBits2Float(0x4250c882), SkBits2Float(0xc1ecb95c));
path.lineTo(SkBits2Float(0x4290688d), SkBits2Float(0xc223bbef));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp227(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f1efaa), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42685cb5), SkBits2Float(0xc2851a3e), SkBits2Float(0x429160d2), SkBits2Float(0xc22043b6));
path.lineTo(SkBits2Float(0x42522f73), SkBits2Float(0xc1e7b52d));
path.cubicTo(SkBits2Float(0x4227f8ff), SkBits2Float(0xc2406ff8), SkBits2Float(0x41aee4c7), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429160d2), SkBits2Float(0xc22043b7));
path.cubicTo(SkBits2Float(0x42943aa0), SkBits2Float(0xc215eba6), SkBits2Float(0x4296cd42), SkBits2Float(0xc20b4794), SkBits2Float(0x429915e6), SkBits2Float(0xc200631e));
path.lineTo(SkBits2Float(0x425d5418), SkBits2Float(0xc1b99eb9));
path.cubicTo(SkBits2Float(0x425a06d4), SkBits2Float(0xc1c95e3a), SkBits2Float(0x42564e98), SkBits2Float(0xc1d8c0a6), SkBits2Float(0x42522f74), SkBits2Float(0xc1e7b52e));
path.lineTo(SkBits2Float(0x429160d2), SkBits2Float(0xc22043b7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp228(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f1efa9), SkBits2Float(0xc2a60000), SkBits2Float(0x42685cb5), SkBits2Float(0xc2851a3e), SkBits2Float(0x429160d2), SkBits2Float(0xc22043b7));
path.lineTo(SkBits2Float(0x429160d2), SkBits2Float(0xc22043b6));
path.cubicTo(SkBits2Float(0x42943aa0), SkBits2Float(0xc215eba5), SkBits2Float(0x4296cd42), SkBits2Float(0xc20b4794), SkBits2Float(0x429915e6), SkBits2Float(0xc200631e));
path.lineTo(SkBits2Float(0x425d5418), SkBits2Float(0xc1b99eb9));
path.cubicTo(SkBits2Float(0x425a06d4), SkBits2Float(0xc1c95e3a), SkBits2Float(0x42564e98), SkBits2Float(0xc1d8c0a6), SkBits2Float(0x42522f74), SkBits2Float(0xc1e7b52e));
path.lineTo(SkBits2Float(0x42522f73), SkBits2Float(0xc1e7b52d));
path.cubicTo(SkBits2Float(0x4227f8ff), SkBits2Float(0xc2406ff8), SkBits2Float(0x41aee4c7), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429915e6), SkBits2Float(0xc200631e));
path.cubicTo(SkBits2Float(0x42abe101), SkBits2Float(0xc11b0235), SkBits2Float(0x42aa16bb), SkBits2Float(0x417b685c), SkBits2Float(0x42942fff), SkBits2Float(0x42159e77));
path.cubicTo(SkBits2Float(0x427c9284), SkBits2Float(0x426c62d8), SkBits2Float(0x422cf27d), SkBits2Float(0x4295ccdb), SkBits2Float(0x419d039e), SkBits2Float(0x42a14aca));
path.lineTo(SkBits2Float(0x4163022c), SkBits2Float(0x42693188));
path.cubicTo(SkBits2Float(0x41fa0b56), SkBits2Float(0x42589424), SkBits2Float(0x4236951c), SkBits2Float(0x422ae1ad), SkBits2Float(0x42563f3c), SkBits2Float(0x41d85112));
path.cubicTo(SkBits2Float(0x4275e95c), SkBits2Float(0x4135bd94), SkBits2Float(0x42787fea), SkBits2Float(0xc0e01be1), SkBits2Float(0x425d5419), SkBits2Float(0xc1b99eba));
path.lineTo(SkBits2Float(0x429915e6), SkBits2Float(0xc200631e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp229(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4206c976), SkBits2Float(0xc2a60000), SkBits2Float(0x42801937), SkBits2Float(0xc27a823c), SkBits2Float(0x4299a0d7), SkBits2Float(0xc1fb88d1));
path.lineTo(SkBits2Float(0x425e1cfa), SkBits2Float(0xc1b5d505));
path.cubicTo(SkBits2Float(0x423933e1), SkBits2Float(0xc2351735), SkBits2Float(0x41c2df6b), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4299a0d8), SkBits2Float(0xc1fb88d0));
path.cubicTo(SkBits2Float(0x429c1b73), SkBits2Float(0xc1e34f53), SkBits2Float(0x429e39d2), SkBits2Float(0xc1ca8528), SkBits2Float(0x429ff920), SkBits2Float(0xc1b14b8c));
path.lineTo(SkBits2Float(0x42674955), SkBits2Float(0xc1802a45));
path.cubicTo(SkBits2Float(0x4264c2a3), SkBits2Float(0xc192666d), SkBits2Float(0x4261b27b), SkBits2Float(0xc1a45204), SkBits2Float(0x425e1cfb), SkBits2Float(0xc1b5d506));
path.lineTo(SkBits2Float(0x4299a0d8), SkBits2Float(0xc1fb88d0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp230(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4206c976), SkBits2Float(0xc2a60000), SkBits2Float(0x42801937), SkBits2Float(0xc27a823c), SkBits2Float(0x4299a0d8), SkBits2Float(0xc1fb88d0));
path.cubicTo(SkBits2Float(0x429c1b73), SkBits2Float(0xc1e34f53), SkBits2Float(0x429e39d2), SkBits2Float(0xc1ca8528), SkBits2Float(0x429ff920), SkBits2Float(0xc1b14b8c));
path.lineTo(SkBits2Float(0x42674955), SkBits2Float(0xc1802a45));
path.cubicTo(SkBits2Float(0x4264c2a3), SkBits2Float(0xc192666d), SkBits2Float(0x4261b27b), SkBits2Float(0xc1a45204), SkBits2Float(0x425e1cfa), SkBits2Float(0xc1b5d505));
path.cubicTo(SkBits2Float(0x423933e1), SkBits2Float(0xc2351735), SkBits2Float(0x41c2df6b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429ff91f), SkBits2Float(0xc1b14b8a));
path.cubicTo(SkBits2Float(0x42ae673b), SkBits2Float(0x40783c41), SkBits2Float(0x42a293c2), SkBits2Float(0x41fe6960), SkBits2Float(0x4280464e), SkBits2Float(0x4252ba7b));
path.cubicTo(SkBits2Float(0x423bf1b3), SkBits2Float(0x42932023), SkBits2Float(0x41a5f32c), SkBits2Float(0x42a99309), SkBits2Float(0xc0c67989), SkBits2Float(0x42a5892f));
path.lineTo(SkBits2Float(0xc08f79c7), SkBits2Float(0x426f5437));
path.cubicTo(SkBits2Float(0x416fed74), SkBits2Float(0x42752af2), SkBits2Float(0x4207dcfc), SkBits2Float(0x4254b62d), SkBits2Float(0x42397512), SkBits2Float(0x42185575));
path.cubicTo(SkBits2Float(0x426b0d26), SkBits2Float(0x41b7e97d), SkBits2Float(0x427c2639), SkBits2Float(0x40337286), SkBits2Float(0x42674956), SkBits2Float(0xc1802a46));
path.lineTo(SkBits2Float(0x429ff91f), SkBits2Float(0xc1b14b8a));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp231(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x421472e7), SkBits2Float(0xc2a5ffff), SkBits2Float(0x428b6da4), SkBits2Float(0xc26973d7), SkBits2Float(0x429fb179), SkBits2Float(0xc1b54986));
path.lineTo(SkBits2Float(0x4266e1be), SkBits2Float(0xc1830d0f));
path.cubicTo(SkBits2Float(0x42499544), SkBits2Float(0xc228c2c8), SkBits2Float(0x41d69ff6), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429fb179), SkBits2Float(0xc1b54988));
path.cubicTo(SkBits2Float(0x42a1a632), SkBits2Float(0xc199b837), SkBits2Float(0x42a3282f), SkBits2Float(0xc17b594e), SkBits2Float(0x42a43501), SkBits2Float(0xc142a7ba));
path.lineTo(SkBits2Float(0x426d6865), SkBits2Float(0xc10cb6f0));
path.cubicTo(SkBits2Float(0x426be3bc), SkBits2Float(0xc135b2ae), SkBits2Float(0x4269b5af), SkBits2Float(0xc15e3ec8), SkBits2Float(0x4266e1be), SkBits2Float(0xc1830d0f));
path.lineTo(SkBits2Float(0x429fb179), SkBits2Float(0xc1b54988));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp232(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x421472e7), SkBits2Float(0xc2a60000), SkBits2Float(0x428b6da4), SkBits2Float(0xc26973d8), SkBits2Float(0x429fb179), SkBits2Float(0xc1b54988));
path.lineTo(SkBits2Float(0x429fb179), SkBits2Float(0xc1b54986));
path.cubicTo(SkBits2Float(0x42a1a632), SkBits2Float(0xc199b836), SkBits2Float(0x42a3282f), SkBits2Float(0xc17b594d), SkBits2Float(0x42a43501), SkBits2Float(0xc142a7ba));
path.lineTo(SkBits2Float(0x426d6865), SkBits2Float(0xc10cb6f0));
path.cubicTo(SkBits2Float(0x426be3bc), SkBits2Float(0xc135b2ae), SkBits2Float(0x4269b5af), SkBits2Float(0xc15e3ec8), SkBits2Float(0x4266e1be), SkBits2Float(0xc1830d0f));
path.cubicTo(SkBits2Float(0x42499544), SkBits2Float(0xc228c2c8), SkBits2Float(0x41d69ff6), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a43502), SkBits2Float(0xc142a7bb));
path.cubicTo(SkBits2Float(0x42ace9b0), SkBits2Float(0x4189ae79), SkBits2Float(0x429590d6), SkBits2Float(0x423ab1c1), SkBits2Float(0x424df762), SkBits2Float(0x428231a6));
path.cubicTo(SkBits2Float(0x41e19a31), SkBits2Float(0x42a70a69), SkBits2Float(0xc04a3289), SkBits2Float(0x42b03133), SkBits2Float(0xc1f5f36e), SkBits2Float(0x429a3139));
path.lineTo(SkBits2Float(0xc1b1cbb9), SkBits2Float(0x425eedb9));
path.cubicTo(SkBits2Float(0xc0122aac), SkBits2Float(0x427ebc5a), SkBits2Float(0x41a31606), SkBits2Float(0x42718130), SkBits2Float(0x4214e430), SkBits2Float(0x423c3b73));
path.cubicTo(SkBits2Float(0x42583d5c), SkBits2Float(0x4206f5b6), SkBits2Float(0x4279fe97), SkBits2Float(0x41470ec8), SkBits2Float(0x426d6866), SkBits2Float(0xc10cb6eb));
path.lineTo(SkBits2Float(0x42a43502), SkBits2Float(0xc142a7bb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp233(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4220aa02), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42952310), SkBits2Float(0xc258f48d), SkBits2Float(0x42a35f68), SkBits2Float(0xc16b5614));
path.lineTo(SkBits2Float(0x426c3395), SkBits2Float(0xc12a1f61));
path.cubicTo(SkBits2Float(0x42579ea8), SkBits2Float(0xc21cd5ce), SkBits2Float(0x41e84916), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a35f69), SkBits2Float(0xc16b5613));
path.cubicTo(SkBits2Float(0x42a4bd24), SkBits2Float(0xc12ea3c2), SkBits2Float(0x42a59325), SkBits2Float(0xc0e282d6), SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.lineTo(SkBits2Float(0x426fd18d), SkBits2Float(0xc0154a48));
path.cubicTo(SkBits2Float(0x426f62a1), SkBits2Float(0xc0a3be33), SkBits2Float(0x426e2d39), SkBits2Float(0xc0fc7dbb), SkBits2Float(0x426c3397), SkBits2Float(0xc12a1f63));
path.lineTo(SkBits2Float(0x42a35f69), SkBits2Float(0xc16b5613));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp234(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4220aa02), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42952310), SkBits2Float(0xc258f48d), SkBits2Float(0x42a35f69), SkBits2Float(0xc16b5613));
path.cubicTo(SkBits2Float(0x42a4bd24), SkBits2Float(0xc12ea3c2), SkBits2Float(0x42a59325), SkBits2Float(0xc0e282d6), SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.lineTo(SkBits2Float(0x426fd18d), SkBits2Float(0xc0154a48));
path.cubicTo(SkBits2Float(0x426f62a1), SkBits2Float(0xc0a3be33), SkBits2Float(0x426e2d39), SkBits2Float(0xc0fc7dbb), SkBits2Float(0x426c3397), SkBits2Float(0xc12a1f63));
path.lineTo(SkBits2Float(0x426c3395), SkBits2Float(0xc12a1f61));
path.cubicTo(SkBits2Float(0x42579ea8), SkBits2Float(0xc21cd5ce), SkBits2Float(0x41e84916), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.cubicTo(SkBits2Float(0x42a85e4f), SkBits2Float(0x41e6959e), SkBits2Float(0x4285b4e3), SkBits2Float(0x426ae44f), SkBits2Float(0x4219b105), SkBits2Float(0x42932450));
path.cubicTo(SkBits2Float(0x411fe111), SkBits2Float(0x42b0d679), SkBits2Float(0xc1c3966b), SkBits2Float(0x42ab1d42), SkBits2Float(0xc2482755), SkBits2Float(0x428470e8));
path.lineTo(SkBits2Float(0xc210b07c), SkBits2Float(0x423f7b24));
path.cubicTo(SkBits2Float(0xc18d6382), SkBits2Float(0x427764e8), SkBits2Float(0x40e72680), SkBits2Float(0x427fab4e), SkBits2Float(0x41de345e), SkBits2Float(0x4254bc3b));
path.cubicTo(SkBits2Float(0x42414f8e), SkBits2Float(0x4229cd28), SkBits2Float(0x42736c9d), SkBits2Float(0x41a6b008), SkBits2Float(0x426fd18e), SkBits2Float(0xc0154a3f));
path.lineTo(SkBits2Float(0x42a5dfdf), SkBits2Float(0xc04e84a0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp235(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x422e5e2d), SkBits2Float(0xc2a5ffff), SkBits2Float(0x429f82f2), SkBits2Float(0xc2451c35), SkBits2Float(0x42a59867), SkBits2Float(0xc0b956c5));
path.lineTo(SkBits2Float(0x426f6a3b), SkBits2Float(0xc085fae3));
path.cubicTo(SkBits2Float(0x42669e7e), SkBits2Float(0xc20e7d42), SkBits2Float(0x41fc1920), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a59868), SkBits2Float(0xc0b956ca));
path.cubicTo(SkBits2Float(0x42a62cd8), SkBits2Float(0xbfd2dd07), SkBits2Float(0x42a621be), SkBits2Float(0x4020d557), SkBits2Float(0x42a57734), SkBits2Float(0x40d4ef9c));
path.lineTo(SkBits2Float(0x426f3a3b), SkBits2Float(0x4099edfc));
path.cubicTo(SkBits2Float(0x427030cb), SkBits2Float(0x3fe887ba), SkBits2Float(0x427040d6), SkBits2Float(0xbf986e77), SkBits2Float(0x426f6a3b), SkBits2Float(0xc085fae4));
path.lineTo(SkBits2Float(0x42a59868), SkBits2Float(0xc0b956ca));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp236(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x422e5e2d), SkBits2Float(0xc2a5ffff), SkBits2Float(0x429f82f2), SkBits2Float(0xc2451c35), SkBits2Float(0x42a59868), SkBits2Float(0xc0b956ca));
path.cubicTo(SkBits2Float(0x42a62cd8), SkBits2Float(0xbfd2dd07), SkBits2Float(0x42a621be), SkBits2Float(0x4020d557), SkBits2Float(0x42a57734), SkBits2Float(0x40d4ef9c));
path.lineTo(SkBits2Float(0x426f3a3b), SkBits2Float(0x4099edfc));
path.cubicTo(SkBits2Float(0x427030cb), SkBits2Float(0x3fe887bb), SkBits2Float(0x427040d6), SkBits2Float(0xbf986e74), SkBits2Float(0x426f6a3b), SkBits2Float(0xc085fae3));
path.lineTo(SkBits2Float(0x426f6a3b), SkBits2Float(0xc085fae4));
path.cubicTo(SkBits2Float(0x42669e7e), SkBits2Float(0xc20e7d42), SkBits2Float(0x41fc1920), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a57735), SkBits2Float(0x40d4ef9d));
path.cubicTo(SkBits2Float(0x429fe5e1), SkBits2Float(0x4225104d), SkBits2Float(0x425fa7d9), SkBits2Float(0x428cf91a), SkBits2Float(0x41b3ea58), SkBits2Float(0x429fca49));
path.cubicTo(SkBits2Float(0xc12ef606), SkBits2Float(0x42b29b77), SkBits2Float(0xc23abc07), SkBits2Float(0x4299d29d), SkBits2Float(0xc2863a28), SkBits2Float(0x42435615));
path.lineTo(SkBits2Float(0xc242103b), SkBits2Float(0x420d34fa));
path.cubicTo(SkBits2Float(0xc206fd22), SkBits2Float(0x425e64f1), SkBits2Float(0xc0fcf4a4), SkBits2Float(0x42811d1e), SkBits2Float(0x41820f34), SkBits2Float(0x426705a2));
path.cubicTo(SkBits2Float(0x4221adc8), SkBits2Float(0x424bd107), SkBits2Float(0x42672d88), SkBits2Float(0x41eea576), SkBits2Float(0x426f3a3c), SkBits2Float(0x4099edfe));
path.lineTo(SkBits2Float(0x42a57735), SkBits2Float(0x40d4ef9d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp237(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41b25a1b), SkBits2Float(0xc2a60000), SkBits2Float(0x422e9a51), SkBits2Float(0xc294100b), SkBits2Float(0x426d0a79), SkBits2Float(0xc26874a1));
path.cubicTo(SkBits2Float(0x4295bd51), SkBits2Float(0xc228c92e), SkBits2Float(0x42a6d6d5), SkBits2Float(0xc1a5596e), SkBits2Float(0x42a5f7e5), SkBits2Float(0x3fcf7f4c));
path.lineTo(SkBits2Float(0x426ff448), SkBits2Float(0x3f95ff69));
path.cubicTo(SkBits2Float(0x4271369b), SkBits2Float(0xc16f0f30), SkBits2Float(0x42587daa), SkBits2Float(0xc1f4071e), SkBits2Float(0x422b5ada), SkBits2Float(0xc2280a4b));
path.cubicTo(SkBits2Float(0x41fc7014), SkBits2Float(0xc2561107), SkBits2Float(0x4180eddd), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a5f7e5), SkBits2Float(0x3fcf7f2e));
path.cubicTo(SkBits2Float(0x42a5cbdf), SkBits2Float(0x40c0b7f8), SkBits2Float(0x42a4eca2), SkBits2Float(0x41268f7d), SkBits2Float(0x42a35c4c), SkBits2Float(0x416be04e));
path.lineTo(SkBits2Float(0x426c2f14), SkBits2Float(0x412a834e));
path.cubicTo(SkBits2Float(0x426e71e2), SkBits2Float(0x40f0cf74), SkBits2Float(0x426fb4a3), SkBits2Float(0x408b5090), SkBits2Float(0x426ff449), SkBits2Float(0x3f95ff6b));
path.lineTo(SkBits2Float(0x42a5f7e5), SkBits2Float(0x3fcf7f2e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp238(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41b25a1b), SkBits2Float(0xc2a60000), SkBits2Float(0x422e9a51), SkBits2Float(0xc294100b), SkBits2Float(0x426d0a79), SkBits2Float(0xc26874a1));
path.cubicTo(SkBits2Float(0x4295bd51), SkBits2Float(0xc228c92e), SkBits2Float(0x42a6d6d5), SkBits2Float(0xc1a5596f), SkBits2Float(0x42a5f7e5), SkBits2Float(0x3fcf7f2e));
path.lineTo(SkBits2Float(0x426c2f14), SkBits2Float(0x412a834e));
path.cubicTo(SkBits2Float(0x426e71e2), SkBits2Float(0x40f0cf74), SkBits2Float(0x426fb4a3), SkBits2Float(0x408b5090), SkBits2Float(0x426ff449), SkBits2Float(0x3f95ff6b));
path.lineTo(SkBits2Float(0x426ff448), SkBits2Float(0x3f95ff69));
path.cubicTo(SkBits2Float(0x4271369b), SkBits2Float(0xc16f0f30), SkBits2Float(0x42587daa), SkBits2Float(0xc1f4071e), SkBits2Float(0x422b5ada), SkBits2Float(0xc2280a4b));
path.cubicTo(SkBits2Float(0x41fc7014), SkBits2Float(0xc2561107), SkBits2Float(0x4180eddd), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a35c4c), SkBits2Float(0x416be04e));
path.cubicTo(SkBits2Float(0x42963d3f), SkBits2Float(0x424c5e0d), SkBits2Float(0x42354f77), SkBits2Float(0x429d76d6), SkBits2Float(0x41096c90), SkBits2Float(0x42a51bdb));
path.cubicTo(SkBits2Float(0xc1e1325f), SkBits2Float(0x42acc0e0), SkBits2Float(0xc27bf938), SkBits2Float(0x4282ec23), SkBits2Float(0xc299cad8), SkBits2Float(0x41f9ecd8));
path.lineTo(SkBits2Float(0xc25e59b3), SkBits2Float(0x41b4ab36));
path.cubicTo(SkBits2Float(0xc2362649), SkBits2Float(0x423d4911), SkBits2Float(0xc1a2caf7), SkBits2Float(0x4279c398), SkBits2Float(0x40c6af7d), SkBits2Float(0x426eb62b));
path.cubicTo(SkBits2Float(0x4203115b), SkBits2Float(0x4263a8be), SkBits2Float(0x425936a2), SkBits2Float(0x4213bc4a), SkBits2Float(0x426c2f16), SkBits2Float(0x412a8350));
path.lineTo(SkBits2Float(0x42a35c4c), SkBits2Float(0x416be04e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp239(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ba3f99), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4235f79d), SkBits2Float(0xc29271cf), SkBits2Float(0x4274db3f), SkBits2Float(0xc260354d));
path.cubicTo(SkBits2Float(0x4299df70), SkBits2Float(0xc21b86fd), SkBits2Float(0x42a97305), SkBits2Float(0xc17e5d7a), SkBits2Float(0x42a55ba0), SkBits2Float(0x40e961b4));
path.lineTo(SkBits2Float(0x426f1259), SkBits2Float(0x40a8b5ae));
path.cubicTo(SkBits2Float(0x4274fca8), SkBits2Float(0xc137e0e1), SkBits2Float(0x425e777b), SkBits2Float(0xc1e0dbdb), SkBits2Float(0x42310131), SkBits2Float(0xc2221408));
path.cubicTo(SkBits2Float(0x42038ae6), SkBits2Float(0xc253ba22), SkBits2Float(0x4186a32c), SkBits2Float(0xc2700000), SkBits2Float(0xb560056c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a55ba0), SkBits2Float(0x40e961b9));
path.cubicTo(SkBits2Float(0x42a48d09), SkBits2Float(0x413de0a1), SkBits2Float(0x42a2fc74), SkBits2Float(0x41833376), SkBits2Float(0x42a0adff), SkBits2Float(0x41a6c250));
path.lineTo(SkBits2Float(0x42684ed9), SkBits2Float(0x417118ef));
path.cubicTo(SkBits2Float(0x426ba483), SkBits2Float(0x413db02f), SkBits2Float(0x426de7aa), SkBits2Float(0x410942c3), SkBits2Float(0x426f1258), SkBits2Float(0x40a8b5ad));
path.lineTo(SkBits2Float(0x42a55ba0), SkBits2Float(0x40e961b9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp240(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ba3f99), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4235f79d), SkBits2Float(0xc29271cf), SkBits2Float(0x4274db3f), SkBits2Float(0xc260354d));
path.cubicTo(SkBits2Float(0x4299df70), SkBits2Float(0xc21b86fd), SkBits2Float(0x42a97305), SkBits2Float(0xc17e5d7a), SkBits2Float(0x42a55ba0), SkBits2Float(0x40e961b9));
path.cubicTo(SkBits2Float(0x42a48d09), SkBits2Float(0x413de0a1), SkBits2Float(0x42a2fc74), SkBits2Float(0x41833376), SkBits2Float(0x42a0adff), SkBits2Float(0x41a6c250));
path.lineTo(SkBits2Float(0x42684ed9), SkBits2Float(0x417118ef));
path.cubicTo(SkBits2Float(0x426ba483), SkBits2Float(0x413db02f), SkBits2Float(0x426de7aa), SkBits2Float(0x410942c3), SkBits2Float(0x426f1259), SkBits2Float(0x40a8b5ae));
path.cubicTo(SkBits2Float(0x4274fca8), SkBits2Float(0xc137e0e1), SkBits2Float(0x425e777b), SkBits2Float(0xc1e0dbdb), SkBits2Float(0x42310131), SkBits2Float(0xc2221408));
path.cubicTo(SkBits2Float(0x42038ae6), SkBits2Float(0xc253ba22), SkBits2Float(0x4186a32c), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a0ae00), SkBits2Float(0x41a6c250));
path.cubicTo(SkBits2Float(0x428d4422), SkBits2Float(0x4269069e), SkBits2Float(0x42118d33), SkBits2Float(0x42a8086f), SkBits2Float(0xc00fe376), SkBits2Float(0x42a5f066));
path.cubicTo(SkBits2Float(0xc22389a2), SkBits2Float(0x42a3d85e), SkBits2Float(0xc2935e5d), SkBits2Float(0x42596224), SkBits2Float(0xc2a2b39d), SkBits2Float(0x4183b53a));
path.lineTo(SkBits2Float(0xc26b3b33), SkBits2Float(0x413e6bca));
path.cubicTo(SkBits2Float(0xc2551027), SkBits2Float(0x421d2508), SkBits2Float(0xc1ec70a3), SkBits2Float(0x426ce27d), SkBits2Float(0xbfd007ff), SkBits2Float(0x426fe979));
path.cubicTo(SkBits2Float(0x41d26fa4), SkBits2Float(0x4272f076), SkBits2Float(0x424c3d84), SkBits2Float(0x422873d5), SkBits2Float(0x42684eda), SkBits2Float(0x417118ee));
path.lineTo(SkBits2Float(0x42a0ae00), SkBits2Float(0x41a6c250));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp241(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c2abe0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x423dc4ab), SkBits2Float(0xc290a493), SkBits2Float(0x427cd8fd), SkBits2Float(0xc25727eb));
path.cubicTo(SkBits2Float(0x429df6a6), SkBits2Float(0xc20d06b1), SkBits2Float(0x42aba628), SkBits2Float(0xc12bcbe5), SkBits2Float(0x42a3dc46), SkBits2Float(0x4154872f));
path.lineTo(SkBits2Float(0x426ce81c), SkBits2Float(0x4119a283));
path.cubicTo(SkBits2Float(0x42782ad8), SkBits2Float(0xc0f86165), SkBits2Float(0x42646188), SkBits2Float(0xc1cbe4ab), SkBits2Float(0x4236c80c), SkBits2Float(0xc21b88d1));
path.cubicTo(SkBits2Float(0x42092e8f), SkBits2Float(0xc2511f4c), SkBits2Float(0x418cb9f2), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a3dc46), SkBits2Float(0x41548735));
path.cubicTo(SkBits2Float(0x42a2537f), SkBits2Float(0x41901e3f), SkBits2Float(0x429ff996), SkBits2Float(0x41b55e92), SkBits2Float(0x429cd549), SkBits2Float(0x41d999a0));
path.lineTo(SkBits2Float(0x4262bf29), SkBits2Float(0x419d4d21));
path.cubicTo(SkBits2Float(0x42674a02), SkBits2Float(0x41831c46), SkBits2Float(0x426ab03e), SkBits2Float(0x41505d16), SkBits2Float(0x426ce81d), SkBits2Float(0x4119a283));
path.lineTo(SkBits2Float(0x42a3dc46), SkBits2Float(0x41548735));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp242(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41c2abe0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x423dc4ab), SkBits2Float(0xc290a493), SkBits2Float(0x427cd8fd), SkBits2Float(0xc25727eb));
path.cubicTo(SkBits2Float(0x429df6a6), SkBits2Float(0xc20d06b1), SkBits2Float(0x42aba628), SkBits2Float(0xc12bcbe5), SkBits2Float(0x42a3dc46), SkBits2Float(0x41548735));
path.cubicTo(SkBits2Float(0x42a2537f), SkBits2Float(0x41901e3f), SkBits2Float(0x429ff996), SkBits2Float(0x41b55e92), SkBits2Float(0x429cd549), SkBits2Float(0x41d999a0));
path.lineTo(SkBits2Float(0x4262bf29), SkBits2Float(0x419d4d21));
path.cubicTo(SkBits2Float(0x42674a02), SkBits2Float(0x41831c46), SkBits2Float(0x426ab03e), SkBits2Float(0x41505d16), SkBits2Float(0x426ce81c), SkBits2Float(0x4119a283));
path.cubicTo(SkBits2Float(0x42782ad8), SkBits2Float(0xc0f86165), SkBits2Float(0x42646188), SkBits2Float(0xc1cbe4ab), SkBits2Float(0x4236c80c), SkBits2Float(0xc21b88d1));
path.cubicTo(SkBits2Float(0x42092e8f), SkBits2Float(0xc2511f4c), SkBits2Float(0x418cb9f2), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429cd549), SkBits2Float(0x41d999a0));
path.cubicTo(SkBits2Float(0x42824b9e), SkBits2Float(0x4282e841), SkBits2Float(0x41d1b597), SkBits2Float(0x42b119ff), SkBits2Float(0xc15b80c3), SkBits2Float(0x42a3b776));
path.cubicTo(SkBits2Float(0xc2569b2d), SkBits2Float(0x429654ee), SkBits2Float(0xc2a5db0b), SkBits2Float(0x42228c64), SkBits2Float(0xc2a5ffee), SkBits2Float(0x3e172efd));
path.lineTo(SkBits2Float(0xc26fffe7), SkBits2Float(0x3dda91a4));
path.cubicTo(SkBits2Float(0xc26fca99), SkBits2Float(0x41eb0285), SkBits2Float(0xc21b2317), SkBits2Float(0x425958e5), SkBits2Float(0xc11ead4d), SkBits2Float(0x426cb2ed));
path.cubicTo(SkBits2Float(0x419798e1), SkBits2Float(0x4280067a), SkBits2Float(0x423c6102), SkBits2Float(0x423d4379), SkBits2Float(0x4262bf29), SkBits2Float(0x419d4d1f));
path.lineTo(SkBits2Float(0x429cd549), SkBits2Float(0x41d999a0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp243(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41caf078), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42455e40), SkBits2Float(0xc28ecc78), SkBits2Float(0x42822b31), SkBits2Float(0xc24e07b4));
path.cubicTo(SkBits2Float(0x42a1a743), SkBits2Float(0xc1fcecee), SkBits2Float(0x42ad3753), SkBits2Float(0xc0b3be45), SkBits2Float(0x42a18eed), SkBits2Float(0x419892cb));
path.lineTo(SkBits2Float(0x42699409), SkBits2Float(0x415c9689));
path.cubicTo(SkBits2Float(0x427a6ed6), SkBits2Float(0xc081ef5b), SkBits2Float(0x4269b739), SkBits2Float(0xc1b6d67a), SkBits2Float(0x423c321c), SkBits2Float(0xc214effc));
path.cubicTo(SkBits2Float(0x420eacff), SkBits2Float(0xc24e74bc), SkBits2Float(0x4192b3ff), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42a18eed), SkBits2Float(0x419892ca));
path.cubicTo(SkBits2Float(0x429f43c9), SkBits2Float(0x41bf6e44), SkBits2Float(0x429c198b), SkBits2Float(0x41e561a5), SkBits2Float(0x42981a0b), SkBits2Float(0x4204fb6e));
path.lineTo(SkBits2Float(0x425be7f8), SkBits2Float(0x41c0436a));
path.cubicTo(SkBits2Float(0x4261afba), SkBits2Float(0x41a5d162), SkBits2Float(0x42664329), SkBits2Float(0x418a6237), SkBits2Float(0x4269940a), SkBits2Float(0x415c968a));
path.lineTo(SkBits2Float(0x42a18eed), SkBits2Float(0x419892ca));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp244(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41caf078), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42455e40), SkBits2Float(0xc28ecc78), SkBits2Float(0x42822b31), SkBits2Float(0xc24e07b4));
path.cubicTo(SkBits2Float(0x42a1a743), SkBits2Float(0xc1fcecee), SkBits2Float(0x42ad3753), SkBits2Float(0xc0b3be48), SkBits2Float(0x42a18eed), SkBits2Float(0x419892ca));
path.lineTo(SkBits2Float(0x42a18eed), SkBits2Float(0x419892cb));
path.cubicTo(SkBits2Float(0x429f43c9), SkBits2Float(0x41bf6e45), SkBits2Float(0x429c198b), SkBits2Float(0x41e561a5), SkBits2Float(0x42981a0b), SkBits2Float(0x4204fb6e));
path.lineTo(SkBits2Float(0x425be7f8), SkBits2Float(0x41c0436a));
path.cubicTo(SkBits2Float(0x4261afba), SkBits2Float(0x41a5d162), SkBits2Float(0x42664329), SkBits2Float(0x418a6237), SkBits2Float(0x4269940a), SkBits2Float(0x415c968a));
path.lineTo(SkBits2Float(0x42699409), SkBits2Float(0x415c9689));
path.cubicTo(SkBits2Float(0x427a6ed6), SkBits2Float(0xc081ef5b), SkBits2Float(0x4269b739), SkBits2Float(0xc1b6d67a), SkBits2Float(0x423c321c), SkBits2Float(0xc214effc));
path.cubicTo(SkBits2Float(0x420eacff), SkBits2Float(0xc24e74bc), SkBits2Float(0x4192b3ff), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42981a0b), SkBits2Float(0x4204fb6e));
path.cubicTo(SkBits2Float(0x426c6b55), SkBits2Float(0x42900555), SkBits2Float(0x417b6a9f), SkBits2Float(0x42b7a6c3), SkBits2Float(0xc1c57072), SkBits2Float(0x429e7dd7));
path.cubicTo(SkBits2Float(0xc282258c), SkBits2Float(0x428554eb), SkBits2Float(0xc2b314c4), SkBits2Float(0x41cdbc89), SkBits2Float(0xc2a2f571), SkBits2Float(0xc17d09b6));
path.lineTo(SkBits2Float(0xc26b9a61), SkBits2Float(0xc136eb32));
path.cubicTo(SkBits2Float(0xc28174d0), SkBits2Float(0x4194b9b3), SkBits2Float(0xc23c29fc), SkBits2Float(0x4240c4dc), SkBits2Float(0xc18eba2f), SkBits2Float(0x4265250a));
path.cubicTo(SkBits2Float(0x4135bf41), SkBits2Float(0x4284c29d), SkBits2Float(0x422ae7d8), SkBits2Float(0x42503918), SkBits2Float(0x425be7f9), SkBits2Float(0x41c04367));
path.lineTo(SkBits2Float(0x42981a0b), SkBits2Float(0x4204fb6e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp245(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d28773), SkBits2Float(0xc2a5ffff), SkBits2Float(0x424c4acf), SkBits2Float(0xc28d0a47), SkBits2Float(0x428572fc), SkBits2Float(0xc24574fc));
path.cubicTo(SkBits2Float(0x42a4c090), SkBits2Float(0xc1e1aad9), SkBits2Float(0x42ae2294), SkBits2Float(0xbf62367e), SkBits2Float(0x429ebce0), SkBits2Float(0x41c23fec));
path.lineTo(SkBits2Float(0x4265801d), SkBits2Float(0x418c6be6));
path.cubicTo(SkBits2Float(0x427bc2fb), SkBits2Float(0xbf238720), SkBits2Float(0x426e322e), SkBits2Float(0xc1a32211), SkBits2Float(0x4240f046), SkBits2Float(0xc20ebd71));
path.cubicTo(SkBits2Float(0x4213ae61), SkBits2Float(0xc24be9da), SkBits2Float(0x41983095), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429ebce1), SkBits2Float(0x41c23fee));
path.cubicTo(SkBits2Float(0x429bb658), SkBits2Float(0x41e9cedc), SkBits2Float(0x4297c4ea), SkBits2Float(0x4208130e), SkBits2Float(0x4292f5c0), SkBits2Float(0x421a62d5));
path.lineTo(SkBits2Float(0x425478e6), SkBits2Float(0x41df3573));
path.cubicTo(SkBits2Float(0x425b6ce6), SkBits2Float(0x41c4bbf1), SkBits2Float(0x42612050), SkBits2Float(0x41a90494), SkBits2Float(0x4265801e), SkBits2Float(0x418c6be6));
path.lineTo(SkBits2Float(0x429ebce1), SkBits2Float(0x41c23fee));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp246(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d28773), SkBits2Float(0xc2a5ffff), SkBits2Float(0x424c4acf), SkBits2Float(0xc28d0a47), SkBits2Float(0x428572fc), SkBits2Float(0xc24574fc));
path.cubicTo(SkBits2Float(0x42a4c090), SkBits2Float(0xc1e1aad9), SkBits2Float(0x42ae2294), SkBits2Float(0xbf62367e), SkBits2Float(0x429ebce1), SkBits2Float(0x41c23fee));
path.cubicTo(SkBits2Float(0x429bb658), SkBits2Float(0x41e9cedc), SkBits2Float(0x4297c4ea), SkBits2Float(0x4208130e), SkBits2Float(0x4292f5c0), SkBits2Float(0x421a62d5));
path.lineTo(SkBits2Float(0x425478e6), SkBits2Float(0x41df3573));
path.cubicTo(SkBits2Float(0x425b6ce6), SkBits2Float(0x41c4bbf1), SkBits2Float(0x42612050), SkBits2Float(0x41a90494), SkBits2Float(0x4265801d), SkBits2Float(0x418c6be6));
path.cubicTo(SkBits2Float(0x427bc2fb), SkBits2Float(0xbf238720), SkBits2Float(0x426e322e), SkBits2Float(0xc1a32211), SkBits2Float(0x4240f046), SkBits2Float(0xc20ebd71));
path.cubicTo(SkBits2Float(0x4213ae61), SkBits2Float(0xc24be9da), SkBits2Float(0x41983095), SkBits2Float(0xc2700000), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4292f5c1), SkBits2Float(0x421a62d6));
path.cubicTo(SkBits2Float(0x42541a09), SkBits2Float(0x429b1363), SkBits2Float(0x40b7c75d), SkBits2Float(0x42bb84d6), SkBits2Float(0xc2093cef), SkBits2Float(0x42972755));
path.cubicTo(SkBits2Float(0xc294b966), SkBits2Float(0x426593a9), SkBits2Float(0xc2ba8c7c), SkBits2Float(0x4131f51c), SkBits2Float(0xc29ad8fe), SkBits2Float(0xc1ef45cd));
path.lineTo(SkBits2Float(0xc25fe048), SkBits2Float(0xc1acf7d7));
path.cubicTo(SkBits2Float(0xc286dac7), SkBits2Float(0x4100a4f0), SkBits2Float(0xc25705ec), SkBits2Float(0x4225f597), SkBits2Float(0xc1c66aa8), SkBits2Float(0x425a891e));
path.cubicTo(SkBits2Float(0x4084da24), SkBits2Float(0x42878e54), SkBits2Float(0x4219539e), SkBits2Float(0x426034bf), SkBits2Float(0x425478e7), SkBits2Float(0x41df3571));
path.lineTo(SkBits2Float(0x4292f5c1), SkBits2Float(0x421a62d6));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp247(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d91350), SkBits2Float(0xc2a5ffff), SkBits2Float(0x425238e3), SkBits2Float(0xc28b791f), SkBits2Float(0x428827e4), SkBits2Float(0xc23dec02));
path.cubicTo(SkBits2Float(0x42a73357), SkBits2Float(0xc1c9cb8b), SkBits2Float(0x42ae86ff), SkBits2Float(0x404daf5b), SkBits2Float(0x429bc6e8), SkBits2Float(0x41e56ae9));
path.lineTo(SkBits2Float(0x42613841), SkBits2Float(0x41a5d816));
path.cubicTo(SkBits2Float(0x427c5425), SkBits2Float(0x4014b024), SkBits2Float(0x4271bc5c), SkBits2Float(0xc191e03e), SkBits2Float(0x4244da12), SkBits2Float(0xc2094aff));
path.cubicTo(SkBits2Float(0x4217f7c8), SkBits2Float(0xc249a5df), SkBits2Float(0x419cec09), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x429bc6e9), SkBits2Float(0x41e56aeb));
path.cubicTo(SkBits2Float(0x429818bd), SkBits2Float(0x4206b36a), SkBits2Float(0x42937671), SkBits2Float(0x4219f01e), SkBits2Float(0x428df070), SkBits2Float(0x422c2771));
path.lineTo(SkBits2Float(0x424d369d), SkBits2Float(0x41f8e5bf));
path.cubicTo(SkBits2Float(0x425532f6), SkBits2Float(0x41de8f99), SkBits2Float(0x425be616), SkBits2Float(0x41c2bf8b), SkBits2Float(0x42613843), SkBits2Float(0x41a5d816));
path.lineTo(SkBits2Float(0x429bc6e9), SkBits2Float(0x41e56aeb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp248(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41d91350), SkBits2Float(0xc2a5ffff), SkBits2Float(0x425238e3), SkBits2Float(0xc28b791f), SkBits2Float(0x428827e4), SkBits2Float(0xc23dec02));
path.cubicTo(SkBits2Float(0x42a73357), SkBits2Float(0xc1c9cb8b), SkBits2Float(0x42ae86ff), SkBits2Float(0x404daf5b), SkBits2Float(0x429bc6e9), SkBits2Float(0x41e56aeb));
path.cubicTo(SkBits2Float(0x429818bd), SkBits2Float(0x4206b36a), SkBits2Float(0x42937671), SkBits2Float(0x4219f01e), SkBits2Float(0x428df070), SkBits2Float(0x422c2771));
path.lineTo(SkBits2Float(0x424d369d), SkBits2Float(0x41f8e5bf));
path.cubicTo(SkBits2Float(0x425532f6), SkBits2Float(0x41de8f99), SkBits2Float(0x425be616), SkBits2Float(0x41c2bf8b), SkBits2Float(0x42613843), SkBits2Float(0x41a5d816));
path.lineTo(SkBits2Float(0x42613841), SkBits2Float(0x41a5d816));
path.cubicTo(SkBits2Float(0x427c5425), SkBits2Float(0x4014b024), SkBits2Float(0x4271bc5c), SkBits2Float(0xc191e03e), SkBits2Float(0x4244da12), SkBits2Float(0xc2094aff));
path.cubicTo(SkBits2Float(0x4217f7c8), SkBits2Float(0xc249a5df), SkBits2Float(0x419cec09), SkBits2Float(0xc2700000), SkBits2Float(0xb630015b), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428df071), SkBits2Float(0x422c2771));
path.cubicTo(SkBits2Float(0x423d9ebb), SkBits2Float(0x42a3ca6a), SkBits2Float(0xc041a78f), SkBits2Float(0x42bd279e), SkBits2Float(0xc228abe7), SkBits2Float(0x428efaad));
path.cubicTo(SkBits2Float(0xc2a29eac), SkBits2Float(0x42419b78), SkBits2Float(0xc2bd3710), SkBits2Float(0xbfef63d4), SkBits2Float(0xc2900003), SkBits2Float(0xc2252a98));
path.lineTo(SkBits2Float(0xc250315d), SkBits2Float(0xc1eecb7c));
path.cubicTo(SkBits2Float(0xc288c864), SkBits2Float(0xbfad0c79), SkBits2Float(0xc26b1d6b), SkBits2Float(0x420bf56b), SkBits2Float(0xc1f3dd5d), SkBits2Float(0x424eb80d));
path.cubicTo(SkBits2Float(0xc00bff34), SkBits2Float(0x4288bd57), SkBits2Float(0x4209134e), SkBits2Float(0x426ccea7), SkBits2Float(0x424d369e), SkBits2Float(0x41f8e5bd));
path.lineTo(SkBits2Float(0x428df071), SkBits2Float(0x422c2771));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp249(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41df6bc7), SkBits2Float(0xc2a60000), SkBits2Float(0x4257ee8b), SkBits2Float(0xc289e8f6), SkBits2Float(0x428aab73), SkBits2Float(0xc2368066));
path.cubicTo(SkBits2Float(0x42a95fa1), SkBits2Float(0xc1b25dc1), SkBits2Float(0x42ae8dc1), SkBits2Float(0x40e61789), SkBits2Float(0x42987459), SkBits2Float(0x42035b41));
path.lineTo(SkBits2Float(0x425c6a87), SkBits2Float(0x41bde9b7));
path.cubicTo(SkBits2Float(0x427c5dea), SkBits2Float(0x40a654db), SkBits2Float(0x4274e0a0), SkBits2Float(0xc180f082), SkBits2Float(0x42487c82), SkBits2Float(0xc203edca));
path.cubicTo(SkBits2Float(0x421c1865), SkBits2Float(0xc2476353), SkBits2Float(0x41a18256), SkBits2Float(0xc2700000), SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42987459), SkBits2Float(0x42035b41));
path.cubicTo(SkBits2Float(0x42941f1a), SkBits2Float(0x421778e1), SkBits2Float(0x428ecdc9), SkBits2Float(0x422aae55), SkBits2Float(0x42889449), SkBits2Float(0x423cb3b9));
path.lineTo(SkBits2Float(0x424576c5), SkBits2Float(0x4208693e));
path.cubicTo(SkBits2Float(0x424e76a2), SkBits2Float(0x41f6c488), SkBits2Float(0x425626ce), SkBits2Float(0x41dafef6), SkBits2Float(0x425c6a88), SkBits2Float(0x41bde9b8));
path.lineTo(SkBits2Float(0x42987459), SkBits2Float(0x42035b41));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp250(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41df6bc7), SkBits2Float(0xc2a60000), SkBits2Float(0x4257ee8b), SkBits2Float(0xc289e8f6), SkBits2Float(0x428aab73), SkBits2Float(0xc2368066));
path.cubicTo(SkBits2Float(0x42a95fa1), SkBits2Float(0xc1b25dc1), SkBits2Float(0x42ae8dc1), SkBits2Float(0x40e61789), SkBits2Float(0x42987459), SkBits2Float(0x42035b41));
path.cubicTo(SkBits2Float(0x42941f1a), SkBits2Float(0x421778e1), SkBits2Float(0x428ecdc9), SkBits2Float(0x422aae55), SkBits2Float(0x42889449), SkBits2Float(0x423cb3b9));
path.lineTo(SkBits2Float(0x424576c5), SkBits2Float(0x4208693e));
path.cubicTo(SkBits2Float(0x424e76a2), SkBits2Float(0x41f6c488), SkBits2Float(0x425626ce), SkBits2Float(0x41dafef6), SkBits2Float(0x425c6a87), SkBits2Float(0x41bde9b7));
path.cubicTo(SkBits2Float(0x427c5dea), SkBits2Float(0x40a654db), SkBits2Float(0x4274e0a0), SkBits2Float(0xc180f082), SkBits2Float(0x42487c82), SkBits2Float(0xc203edca));
path.cubicTo(SkBits2Float(0x421c1865), SkBits2Float(0xc2476353), SkBits2Float(0x41a18256), SkBits2Float(0xc2700000), SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42889449), SkBits2Float(0x423cb3b8));
path.cubicTo(SkBits2Float(0x424c5291), SkBits2Float(0x42902c61), SkBits2Float(0x41ad609d), SkBits2Float(0x42ab4d26), SkBits2Float(0xc1072a9c), SkBits2Float(0x42a52356));
path.cubicTo(SkBits2Float(0xc21a459c), SkBits2Float(0x429ef985), SkBits2Float(0xc2813d9b), SkBits2Float(0x4270fef6), SkBits2Float(0xc298db30), SkBits2Float(0x420179e4));
path.cubicTo(SkBits2Float(0xc2b078c6), SkBits2Float(0x408fa686), SkBits2Float(0xc2a7d9d7), SkBits2Float(0xc1dcde62), SkBits2Float(0xc2825c7e), SkBits2Float(0xc24d8ae0));
path.lineTo(SkBits2Float(0xc23c7965), SkBits2Float(0xc21495bd));
path.cubicTo(SkBits2Float(0xc272ad07), SkBits2Float(0xc19fa9fe), SkBits2Float(0xc27f23bc), SkBits2Float(0x404faf9e), SkBits2Float(0xc25cff22), SkBits2Float(0x41bb31a8));
path.cubicTo(SkBits2Float(0xc23ada86), SkBits2Float(0x422e36b1), SkBits2Float(0xc1df0b0c), SkBits2Float(0x4265d7b2), SkBits2Float(0xc0c36b6f), SkBits2Float(0x426ec0e0));
path.cubicTo(SkBits2Float(0x417aaa9e), SkBits2Float(0x4277aa0e), SkBits2Float(0x4213b3f9), SkBits2Float(0x42507175), SkBits2Float(0x424576c8), SkBits2Float(0x4208693c));
path.lineTo(SkBits2Float(0x42889449), SkBits2Float(0x423cb3b8));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp251(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41e529f0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x425d10b2), SkBits2Float(0xc2887541), SkBits2Float(0x428cd9cf), SkBits2Float(0xc22fb184));
path.cubicTo(SkBits2Float(0x42ab2b45), SkBits2Float(0xc19cf10c), SkBits2Float(0x42ae472d), SkBits2Float(0x412c96c0), SkBits2Float(0x42951360), SkBits2Float(0x42120c0d));
path.lineTo(SkBits2Float(0x425787f7), SkBits2Float(0x41d32707));
path.cubicTo(SkBits2Float(0x427bf7e0), SkBits2Float(0x40f986c2), SkBits2Float(0x4277792b), SkBits2Float(0xc162e746), SkBits2Float(0x424ba3c8), SkBits2Float(0xc1fe03ba));
path.cubicTo(SkBits2Float(0x421fce66), SkBits2Float(0xc24549e8), SkBits2Float(0x41a5a922), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42951360), SkBits2Float(0x42120c0f));
path.cubicTo(SkBits2Float(0x429023a5), SkBits2Float(0x422633cd), SkBits2Float(0x428a3193), SkBits2Float(0x42394df4), SkBits2Float(0x42835484), SkBits2Float(0x424b0f7e));
path.lineTo(SkBits2Float(0x423ddffa), SkBits2Float(0x4212ca6e));
path.cubicTo(SkBits2Float(0x4247cc4f), SkBits2Float(0x4205f480), SkBits2Float(0x425064e4), SkBits2Float(0x41f04ae6), SkBits2Float(0x425787f8), SkBits2Float(0x41d32708));
path.lineTo(SkBits2Float(0x42951360), SkBits2Float(0x42120c0f));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp252(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41e529f0), SkBits2Float(0xc2a5ffff), SkBits2Float(0x425d10b2), SkBits2Float(0xc2887541), SkBits2Float(0x428cd9cf), SkBits2Float(0xc22fb184));
path.cubicTo(SkBits2Float(0x42ab2b45), SkBits2Float(0xc19cf10c), SkBits2Float(0x42ae472d), SkBits2Float(0x412c96c0), SkBits2Float(0x42951360), SkBits2Float(0x42120c0f));
path.cubicTo(SkBits2Float(0x429023a5), SkBits2Float(0x422633cd), SkBits2Float(0x428a3193), SkBits2Float(0x42394df4), SkBits2Float(0x42835484), SkBits2Float(0x424b0f7e));
path.lineTo(SkBits2Float(0x423ddffa), SkBits2Float(0x4212ca6e));
path.cubicTo(SkBits2Float(0x4247cc4f), SkBits2Float(0x4205f480), SkBits2Float(0x425064e4), SkBits2Float(0x41f04ae6), SkBits2Float(0x425787f7), SkBits2Float(0x41d32707));
path.cubicTo(SkBits2Float(0x427bf7e0), SkBits2Float(0x40f986c2), SkBits2Float(0x4277792b), SkBits2Float(0xc162e746), SkBits2Float(0x424ba3c8), SkBits2Float(0xc1fe03ba));
path.cubicTo(SkBits2Float(0x421fce66), SkBits2Float(0xc24549e8), SkBits2Float(0x41a5a922), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42835484), SkBits2Float(0x424b0f7e));
path.cubicTo(SkBits2Float(0x423aab34), SkBits2Float(0x4296ad9b), SkBits2Float(0x41789cf4), SkBits2Float(0x42ae7f70), SkBits2Float(0xc1702bd2), SkBits2Float(0x42a3434e));
path.cubicTo(SkBits2Float(0xc2363d27), SkBits2Float(0x4298072c), SkBits2Float(0xc28cd4c4), SkBits2Float(0x42573cf7), SkBits2Float(0xc29edb8e), SkBits2Float(0x41c0adb0));
path.cubicTo(SkBits2Float(0xc2b0e257), SkBits2Float(0xc0b47a14), SkBits2Float(0xc2a03550), SkBits2Float(0xc217a35b), SkBits2Float(0xc2674746), SkBits2Float(0xc26e3089));
path.lineTo(SkBits2Float(0xc2273070), SkBits2Float(0xc22c2f6e));
path.cubicTo(SkBits2Float(0xc267a050), SkBits2Float(0xc1db3c5e), SkBits2Float(0xc27fbc5f), SkBits2Float(0xc0827737), SkBits2Float(0xc265ac62), SkBits2Float(0x418b490c));
path.cubicTo(SkBits2Float(0xc24b9c64), SkBits2Float(0x421b97f2), SkBits2Float(0xc203bd1c), SkBits2Float(0x425bcc95), SkBits2Float(0xc12d9e08), SkBits2Float(0x426c0adc));
path.cubicTo(SkBits2Float(0x4133b85e), SkBits2Float(0x427c4921), SkBits2Float(0x4206f0f2), SkBits2Float(0x4259d90a), SkBits2Float(0x423ddff7), SkBits2Float(0x4212ca73));
path.lineTo(SkBits2Float(0x42835484), SkBits2Float(0x424b0f7e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp253(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ea9e19), SkBits2Float(0xc2a60000), SkBits2Float(0x4261e8db), SkBits2Float(0xc2870be6), SkBits2Float(0x428ed6bc), SkBits2Float(0xc22926d7));
path.cubicTo(SkBits2Float(0x42acb90a), SkBits2Float(0xc1886bc1), SkBits2Float(0x42adc0f7), SkBits2Float(0x41631db6), SkBits2Float(0x42918cff), SkBits2Float(0x421fa302));
path.lineTo(SkBits2Float(0x42526f53), SkBits2Float(0x41e6ccd4));
path.cubicTo(SkBits2Float(0x427b35d6), SkBits2Float(0x41242e26), SkBits2Float(0x4279b842), SkBits2Float(0xc1453c2f), SkBits2Float(0x424e8393), SkBits2Float(0xc1f48e84));
path.cubicTo(SkBits2Float(0x42234ee4), SkBits2Float(0xc2433f78), SkBits2Float(0x41a99a66), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42918d00), SkBits2Float(0x421fa301));
path.cubicTo(SkBits2Float(0x428c0830), SkBits2Float(0x4233c399), SkBits2Float(0x42857bfe), SkBits2Float(0x4246b13f), SkBits2Float(0x427c06a0), SkBits2Float(0x42581e30));
path.lineTo(SkBits2Float(0x42362ff8), SkBits2Float(0x421c3ad6));
path.cubicTo(SkBits2Float(0x4240fd4a), SkBits2Float(0x420fa210), SkBits2Float(0x424a74b5), SkBits2Float(0x4201f32f), SkBits2Float(0x42526f54), SkBits2Float(0x41e6ccd5));
path.lineTo(SkBits2Float(0x42918d00), SkBits2Float(0x421fa301));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp254(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41ea9e19), SkBits2Float(0xc2a60000), SkBits2Float(0x4261e8db), SkBits2Float(0xc2870be6), SkBits2Float(0x428ed6bc), SkBits2Float(0xc22926d7));
path.cubicTo(SkBits2Float(0x42acb90a), SkBits2Float(0xc1886bc1), SkBits2Float(0x42adc0f7), SkBits2Float(0x41631db6), SkBits2Float(0x42918d00), SkBits2Float(0x421fa301));
path.cubicTo(SkBits2Float(0x428c0830), SkBits2Float(0x4233c399), SkBits2Float(0x42857bfe), SkBits2Float(0x4246b13f), SkBits2Float(0x427c06a0), SkBits2Float(0x42581e30));
path.lineTo(SkBits2Float(0x42362ff8), SkBits2Float(0x421c3ad6));
path.cubicTo(SkBits2Float(0x4240fd4a), SkBits2Float(0x420fa210), SkBits2Float(0x424a74b5), SkBits2Float(0x4201f32f), SkBits2Float(0x42526f53), SkBits2Float(0x41e6ccd4));
path.cubicTo(SkBits2Float(0x427b35d6), SkBits2Float(0x41242e26), SkBits2Float(0x4279b842), SkBits2Float(0xc1453c2f), SkBits2Float(0x424e8393), SkBits2Float(0xc1f48e84));
path.cubicTo(SkBits2Float(0x42234ee4), SkBits2Float(0xc2433f78), SkBits2Float(0x41a99a66), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427c069f), SkBits2Float(0x42581e31));
path.cubicTo(SkBits2Float(0x4229355f), SkBits2Float(0x429c5901), SkBits2Float(0x4119ef9b), SkBits2Float(0x42b0b9f6), SkBits2Float(0xc1a91754), SkBits2Float(0x42a086fc));
path.cubicTo(SkBits2Float(0xc24f933a), SkBits2Float(0x42905402), SkBits2Float(0xc296a2af), SkBits2Float(0x423cccf9), SkBits2Float(0xc2a2e3f0), SkBits2Float(0x417fd713));
path.cubicTo(SkBits2Float(0xc2af2532), SkBits2Float(0xc17385be), SkBits2Float(0xc296a6d5), SkBits2Float(0xc23cbfbd), SkBits2Float(0xc247a7c9), SkBits2Float(0xc284a101));
path.lineTo(SkBits2Float(0xc210544b), SkBits2Float(0xc23fc0ab));
path.cubicTo(SkBits2Float(0xc259cf4c), SkBits2Float(0xc20871e9), SkBits2Float(0xc27d38da), SkBits2Float(0xc1300a36), SkBits2Float(0xc26b810f), SkBits2Float(0x4138f1f1));
path.cubicTo(SkBits2Float(0xc259c944), SkBits2Float(0x42087b85), SkBits2Float(0xc2160de3), SkBits2Float(0x4250aad1), SkBits2Float(0xc174780b), SkBits2Float(0x42681670));
path.cubicTo(SkBits2Float(0x40de8efd), SkBits2Float(0x427f820e), SkBits2Float(0x41f4a392), SkBits2Float(0x42620b79), SkBits2Float(0x42362ffc), SkBits2Float(0x421c3ad2));
path.lineTo(SkBits2Float(0x427c069f), SkBits2Float(0x42581e31));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp255(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41eeb164), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42658277), SkBits2Float(0xc285f892), SkBits2Float(0x42904565), SkBits2Float(0xc22437b5));
path.cubicTo(SkBits2Float(0x42adc98d), SkBits2Float(0xc171f916), SkBits2Float(0x42ad3226), SkBits2Float(0x4185deb6), SkBits2Float(0x428eb8d5), SkBits2Float(0x42298bae));
path.lineTo(SkBits2Float(0x424e5857), SkBits2Float(0x41f5204e));
path.cubicTo(SkBits2Float(0x427a675d), SkBits2Float(0x41418c03), SkBits2Float(0x427b4242), SkBits2Float(0xc12eeb9a), SkBits2Float(0x425095b0), SkBits2Float(0xc1ed6c50));
path.cubicTo(SkBits2Float(0x4225e91e), SkBits2Float(0xc241b169), SkBits2Float(0x41ac8c92), SkBits2Float(0xc2700000), SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428eb8d5), SkBits2Float(0x42298bad));
path.cubicTo(SkBits2Float(0x4288c365), SkBits2Float(0x423d9c15), SkBits2Float(0x4281c36f), SkBits2Float(0x42505c7e), SkBits2Float(0x4273ad50), SkBits2Float(0x42617d52));
path.lineTo(SkBits2Float(0x423026ec), SkBits2Float(0x42230126));
path.cubicTo(SkBits2Float(0x423b9c18), SkBits2Float(0x42169f65), SkBits2Float(0x4245bae4), SkBits2Float(0x42091136), SkBits2Float(0x424e5858), SkBits2Float(0x41f5204d));
path.lineTo(SkBits2Float(0x428eb8d5), SkBits2Float(0x42298bad));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp256(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41eeb164), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42658277), SkBits2Float(0xc285f892), SkBits2Float(0x42904565), SkBits2Float(0xc22437b5));
path.cubicTo(SkBits2Float(0x42adc98d), SkBits2Float(0xc171f917), SkBits2Float(0x42ad3226), SkBits2Float(0x4185deb4), SkBits2Float(0x428eb8d5), SkBits2Float(0x42298bad));
path.lineTo(SkBits2Float(0x428eb8d5), SkBits2Float(0x42298bae));
path.cubicTo(SkBits2Float(0x4288c365), SkBits2Float(0x423d9c16), SkBits2Float(0x4281c36f), SkBits2Float(0x42505c7e), SkBits2Float(0x4273ad50), SkBits2Float(0x42617d52));
path.lineTo(SkBits2Float(0x423026ec), SkBits2Float(0x42230126));
path.cubicTo(SkBits2Float(0x423b9c18), SkBits2Float(0x42169f65), SkBits2Float(0x4245bae4), SkBits2Float(0x42091136), SkBits2Float(0x424e5858), SkBits2Float(0x41f5204d));
path.cubicTo(SkBits2Float(0x427a675e), SkBits2Float(0x41418c02), SkBits2Float(0x427b4242), SkBits2Float(0xc12eeb9b), SkBits2Float(0x425095b0), SkBits2Float(0xc1ed6c50));
path.cubicTo(SkBits2Float(0x4225e91e), SkBits2Float(0xc241b169), SkBits2Float(0x41ac8c92), SkBits2Float(0xc2700000), SkBits2Float(0xb69400ae), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4273ad4f), SkBits2Float(0x42617d52));
path.cubicTo(SkBits2Float(0x421bc173), SkBits2Float(0x42a0404f), SkBits2Float(0x40a50405), SkBits2Float(0x42b1dfaa), SkBits2Float(0xc1cd0022), SkBits2Float(0x429de3fd));
path.cubicTo(SkBits2Float(0xc261a0a2), SkBits2Float(0x4289e850), SkBits2Float(0xc29d25ee), SkBits2Float(0x4227ed4e), SkBits2Float(0xc2a4d3d8), SkBits2Float(0x411d8f80));
path.cubicTo(SkBits2Float(0xc2ac81c3), SkBits2Float(0xc1b24b1c), SkBits2Float(0xc28e216c), SkBits2Float(0xc256e38c), SkBits2Float(0xc22e0453), SkBits2Float(0xc28d5ec3));
path.lineTo(SkBits2Float(0xc1fb9743), SkBits2Float(0xc24c63fd));
path.cubicTo(SkBits2Float(0xc24d7d6b), SkBits2Float(0xc21b575f), SkBits2Float(0xc279684a), SkBits2Float(0xc180e302), SkBits2Float(0xc26e4dff), SkBits2Float(0x40e3cc4e));
path.cubicTo(SkBits2Float(0xc26333b4), SkBits2Float(0x41f2c929), SkBits2Float(0xc2231aa4), SkBits2Float(0x42476256), SkBits2Float(0xc1943166), SkBits2Float(0x4264467e));
path.cubicTo(SkBits2Float(0x406e93d1), SkBits2Float(0x42809553), SkBits2Float(0x41e1305a), SkBits2Float(0x4267b03c), SkBits2Float(0x423026ed), SkBits2Float(0x42230127));
path.lineTo(SkBits2Float(0x4273ad4f), SkBits2Float(0x42617d52));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp257(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f2d268), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426923a2), SkBits2Float(0xc284dd06), SkBits2Float(0x4291aced), SkBits2Float(0xc21f2e53));
path.cubicTo(SkBits2Float(0x42aec809), SkBits2Float(0xc1528a66), SkBits2Float(0x42ac7c90), SkBits2Float(0x419a60b1), SkBits2Float(0x428bb0fe), SkBits2Float(0x42335ba0));
path.lineTo(SkBits2Float(0x4249f6a4), SkBits2Float(0x4201a806));
path.cubicTo(SkBits2Float(0x427960d2), SkBits2Float(0x415f325f), SkBits2Float(0x427cb22e), SkBits2Float(0xc11832b1), SkBits2Float(0x42529d7e), SkBits2Float(0xc1e62422));
path.cubicTo(SkBits2Float(0x422888ce), SkBits2Float(0xc2401775), SkBits2Float(0x41af88b3), SkBits2Float(0xc2700000), SkBits2Float(0x36d3ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428bb0ff), SkBits2Float(0x42335ba2));
path.cubicTo(SkBits2Float(0x4285489d), SkBits2Float(0x42475206), SkBits2Float(0x427ba631), SkBits2Float(0x4259da14), SkBits2Float(0x426ae250), SkBits2Float(0x426aa282));
path.lineTo(SkBits2Float(0x4229cbb3), SkBits2Float(0x42299d92));
path.cubicTo(SkBits2Float(0x4235ea43), SkBits2Float(0x421d7bb7), SkBits2Float(0x4240b302), SkBits2Float(0x42101649), SkBits2Float(0x4249f6a5), SkBits2Float(0x4201a807));
path.lineTo(SkBits2Float(0x428bb0ff), SkBits2Float(0x42335ba2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp258(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36d3ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f2d268), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426923a2), SkBits2Float(0xc284dd06), SkBits2Float(0x4291aced), SkBits2Float(0xc21f2e53));
path.cubicTo(SkBits2Float(0x42aec809), SkBits2Float(0xc1528a66), SkBits2Float(0x42ac7c90), SkBits2Float(0x419a60b1), SkBits2Float(0x428bb0ff), SkBits2Float(0x42335ba2));
path.cubicTo(SkBits2Float(0x4285489d), SkBits2Float(0x42475206), SkBits2Float(0x427ba631), SkBits2Float(0x4259da14), SkBits2Float(0x426ae250), SkBits2Float(0x426aa282));
path.lineTo(SkBits2Float(0x4229cbb3), SkBits2Float(0x42299d92));
path.cubicTo(SkBits2Float(0x4235ea43), SkBits2Float(0x421d7bb7), SkBits2Float(0x4240b302), SkBits2Float(0x42101649), SkBits2Float(0x4249f6a4), SkBits2Float(0x4201a806));
path.cubicTo(SkBits2Float(0x427960d2), SkBits2Float(0x415f325f), SkBits2Float(0x427cb22e), SkBits2Float(0xc11832b1), SkBits2Float(0x42529d7e), SkBits2Float(0xc1e62422));
path.cubicTo(SkBits2Float(0x422888ce), SkBits2Float(0xc2401775), SkBits2Float(0x41af88b3), SkBits2Float(0xc2700000), SkBits2Float(0x36d3ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x426ae251), SkBits2Float(0x426aa281));
path.cubicTo(SkBits2Float(0x420dcd2c), SkBits2Float(0x42a3e87c), SkBits2Float(0x3f1c0197), SkBits2Float(0x42b294d6), SkBits2Float(0xc1f0a2ab), SkBits2Float(0x429ab731));
path.cubicTo(SkBits2Float(0xc27312b1), SkBits2Float(0x4282d98e), SkBits2Float(0xc2a300b1), SkBits2Float(0x4211eaa7), SkBits2Float(0xc2a5d865), SkBits2Float(0x40654aaf));
path.cubicTo(SkBits2Float(0xc2a8b018), SkBits2Float(0xc1ea82a2), SkBits2Float(0xc2845e8a), SkBits2Float(0xc26fc272), SkBits2Float(0xc2128ebb), SkBits2Float(0xc294f34d));
path.lineTo(SkBits2Float(0xc1d3e3ef), SkBits2Float(0xc2575999));
path.cubicTo(SkBits2Float(0xc23f6093), SkBits2Float(0xc22d51f6), SkBits2Float(0xc273e2d0), SkBits2Float(0xc1a9868a), SkBits2Float(0xc26fc6b5), SkBits2Float(0x4025c090));
path.cubicTo(SkBits2Float(0xc26baa9a), SkBits2Float(0x41d2f6ae), SkBits2Float(0xc22fb71e), SkBits2Float(0x423d2e2a), SkBits2Float(0xc1adf403), SkBits2Float(0x425faf61));
path.cubicTo(SkBits2Float(0x3ee18e9e), SkBits2Float(0x4281184d), SkBits2Float(0x41cd03a3), SkBits2Float(0x426cf9bf), SkBits2Float(0x4229cbb7), SkBits2Float(0x42299d90));
path.lineTo(SkBits2Float(0x426ae251), SkBits2Float(0x426aa281));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp259(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f70d18), SkBits2Float(0xc2a60000), SkBits2Float(0x426cd682), SkBits2Float(0xc283b5d2), SkBits2Float(0x429310ae), SkBits2Float(0xc219fc22));
path.cubicTo(SkBits2Float(0x42afb61c), SkBits2Float(0xc132327f), SkBits2Float(0x42ab9c4e), SkBits2Float(0x41af4ab2), SkBits2Float(0x42886baa), SkBits2Float(0x423d2918));
path.lineTo(SkBits2Float(0x42453c0d), SkBits2Float(0x4208be17));
path.cubicTo(SkBits2Float(0x42781c98), SkBits2Float(0x417d6f0f), SkBits2Float(0x427e0a5e), SkBits2Float(0xc100d142), SkBits2Float(0x42549fd3), SkBits2Float(0xc1dea0fa));
path.cubicTo(SkBits2Float(0x422b3547), SkBits2Float(0xc23e6ca9), SkBits2Float(0x41b29756), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42886bab), SkBits2Float(0x423d2917));
path.cubicTo(SkBits2Float(0x42818ce6), SkBits2Float(0x4250fab6), SkBits2Float(0x42733ded), SkBits2Float(0x42633df9), SkBits2Float(0x42618b96), SkBits2Float(0x4273a01b));
path.lineTo(SkBits2Float(0x42230b75), SkBits2Float(0x42301d61));
path.cubicTo(SkBits2Float(0x422fd668), SkBits2Float(0x4224457a), SkBits2Float(0x423b4d41), SkBits2Float(0x421711c6), SkBits2Float(0x42453c0e), SkBits2Float(0x4208be17));
path.lineTo(SkBits2Float(0x42886bab), SkBits2Float(0x423d2917));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp260(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f70d18), SkBits2Float(0xc2a60000), SkBits2Float(0x426cd682), SkBits2Float(0xc283b5d2), SkBits2Float(0x429310ae), SkBits2Float(0xc219fc22));
path.cubicTo(SkBits2Float(0x42afb61c), SkBits2Float(0xc132327f), SkBits2Float(0x42ab9c4e), SkBits2Float(0x41af4ab2), SkBits2Float(0x42886bab), SkBits2Float(0x423d2917));
path.cubicTo(SkBits2Float(0x42818ce6), SkBits2Float(0x4250fab6), SkBits2Float(0x42733ded), SkBits2Float(0x42633df9), SkBits2Float(0x42618b96), SkBits2Float(0x4273a01b));
path.lineTo(SkBits2Float(0x42230b75), SkBits2Float(0x42301d61));
path.cubicTo(SkBits2Float(0x422fd668), SkBits2Float(0x4224457a), SkBits2Float(0x423b4d41), SkBits2Float(0x421711c6), SkBits2Float(0x42453c0d), SkBits2Float(0x4208be17));
path.cubicTo(SkBits2Float(0x42781c98), SkBits2Float(0x417d6f0f), SkBits2Float(0x427e0a5e), SkBits2Float(0xc100d142), SkBits2Float(0x42549fd3), SkBits2Float(0xc1dea0fa));
path.cubicTo(SkBits2Float(0x422b3547), SkBits2Float(0xc23e6ca9), SkBits2Float(0x41b29756), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42618b95), SkBits2Float(0x4273a01c));
path.cubicTo(SkBits2Float(0x41fe659e), SkBits2Float(0x42a75638), SkBits2Float(0xc081f8cf), SkBits2Float(0x42b2d4b3), SkBits2Float(0xc20a1eaa), SkBits2Float(0x4296f3e7));
path.cubicTo(SkBits2Float(0xc281ff1e), SkBits2Float(0x42762634), SkBits2Float(0xc2a8320c), SkBits2Float(0x41f52b39), SkBits2Float(0xc2a5e71e), SkBits2Float(0xc035be80));
path.cubicTo(SkBits2Float(0xc2a39c30), SkBits2Float(0xc2114d6a), SkBits2Float(0xc2728d06), SkBits2Float(0xc283ad37), SkBits2Float(0xc1ea4cbe), SkBits2Float(0xc29b5279));
path.lineTo(SkBits2Float(0xc1a95f99), SkBits2Float(0xc2608fe9));
path.cubicTo(SkBits2Float(0xc22f5688), SkBits2Float(0xc23e6034), SkBits2Float(0xc26c8b72), SkBits2Float(0xc1d2135a), SkBits2Float(0xc26fdc03), SkBits2Float(0xc003615b));
path.cubicTo(SkBits2Float(0xc2732c96), SkBits2Float(0x41b13b02), SkBits2Float(0xc23bf25c), SkBits2Float(0x4231f06e), SkBits2Float(0xc1c7b0f0), SkBits2Float(0x425a3eb1));
path.cubicTo(SkBits2Float(0xc03be91a), SkBits2Float(0x4281467b), SkBits2Float(0x41b7e6c5), SkBits2Float(0x4271eec4), SkBits2Float(0x42230b77), SkBits2Float(0x42301d61));
path.lineTo(SkBits2Float(0x42618b95), SkBits2Float(0x4273a01c));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp261(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f9750b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426eeefa), SkBits2Float(0xc2830bb8), SkBits2Float(0x4293d569), SkBits2Float(0xc2170343));
path.cubicTo(SkBits2Float(0x42b03354), SkBits2Float(0xc11fbc55), SkBits2Float(0x42ab0b89), SkBits2Float(0x41bb247a), SkBits2Float(0x42867c8e), SkBits2Float(0x42429f12));
path.lineTo(SkBits2Float(0x42427039), SkBits2Float(0x420cb0ae));
path.cubicTo(SkBits2Float(0x42774b4a), SkBits2Float(0x418748a6), SkBits2Float(0x427ebf70), SkBits2Float(0xc0e6f16a), SkBits2Float(0x4255bc46), SkBits2Float(0xc1da54e8));
path.cubicTo(SkBits2Float(0x422cb91b), SkBits2Float(0xc23d76ba), SkBits2Float(0x41b454a4), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42867c8e), SkBits2Float(0x42429f13));
path.cubicTo(SkBits2Float(0x427eb473), SkBits2Float(0x4256572c), SkBits2Float(0x426e4fbb), SkBits2Float(0x42686e49), SkBits2Float(0x425c16a2), SkBits2Float(0x427890ea));
path.lineTo(SkBits2Float(0x421f199c), SkBits2Float(0x4233afb3));
path.cubicTo(SkBits2Float(0x422c45f9), SkBits2Float(0x422805b5), SkBits2Float(0x42381fbf), SkBits2Float(0x421af1ea), SkBits2Float(0x4242703a), SkBits2Float(0x420cb0af));
path.lineTo(SkBits2Float(0x42867c8e), SkBits2Float(0x42429f13));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp262(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41f9750b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x426eeefa), SkBits2Float(0xc2830bb8), SkBits2Float(0x4293d569), SkBits2Float(0xc2170343));
path.cubicTo(SkBits2Float(0x42b03354), SkBits2Float(0xc11fbc55), SkBits2Float(0x42ab0b89), SkBits2Float(0x41bb247a), SkBits2Float(0x42867c8e), SkBits2Float(0x42429f13));
path.cubicTo(SkBits2Float(0x427eb473), SkBits2Float(0x4256572c), SkBits2Float(0x426e4fbb), SkBits2Float(0x42686e49), SkBits2Float(0x425c16a2), SkBits2Float(0x427890ea));
path.lineTo(SkBits2Float(0x421f199c), SkBits2Float(0x4233afb3));
path.cubicTo(SkBits2Float(0x422c45f9), SkBits2Float(0x422805b5), SkBits2Float(0x42381fbf), SkBits2Float(0x421af1ea), SkBits2Float(0x42427039), SkBits2Float(0x420cb0ae));
path.cubicTo(SkBits2Float(0x42774b4a), SkBits2Float(0x418748a6), SkBits2Float(0x427ebf70), SkBits2Float(0xc0e6f16a), SkBits2Float(0x4255bc46), SkBits2Float(0xc1da54e8));
path.cubicTo(SkBits2Float(0x422cb91b), SkBits2Float(0xc23d76ba), SkBits2Float(0x41b454a4), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x425c16a1), SkBits2Float(0x427890eb));
path.cubicTo(SkBits2Float(0x41ed85e5), SkBits2Float(0x42a9245e), SkBits2Float(0xc0d70d9a), SkBits2Float(0x42b2c211), SkBits2Float(0xc2140612), SkBits2Float(0x42949665));
path.cubicTo(SkBits2Float(0xc2869539), SkBits2Float(0x426cd56f), SkBits2Float(0xc2aac701), SkBits2Float(0x41d9ff9c), SkBits2Float(0xc2a57e3b), SkBits2Float(0xc0cf6824));
path.cubicTo(SkBits2Float(0xc2a03574), SkBits2Float(0xc220d9d7), SkBits2Float(0xc26501e3), SkBits2Float(0xc289ed78), SkBits2Float(0xc1c7e516), SkBits2Float(0xc29e4c97));
path.lineTo(SkBits2Float(0xc190809e), SkBits2Float(0xc264ddc3));
path.cubicTo(SkBits2Float(0xc2258c2b), SkBits2Float(0xc24769d4), SkBits2Float(0xc267a08f), SkBits2Float(0xc1e88e39), SkBits2Float(0xc26f4461), SkBits2Float(0xc095eec9));
path.cubicTo(SkBits2Float(0xc276e835), SkBits2Float(0x419d96da), SkBits2Float(0xc24293e3), SkBits2Float(0x422b3483), SkBits2Float(0xc1d60298), SkBits2Float(0x4256d347));
path.cubicTo(SkBits2Float(0xc09b75b0), SkBits2Float(0x42813905), SkBits2Float(0x41abb417), SkBits2Float(0x42748af0), SkBits2Float(0x421f199e), SkBits2Float(0x4233afb2));
path.lineTo(SkBits2Float(0x425c16a1), SkBits2Float(0x427890eb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp263(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fc38da), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4271556b), SkBits2Float(0xc2824656), SkBits2Float(0x4294b266), SkBits2Float(0xc213956f));
path.cubicTo(SkBits2Float(0x42b0ba15), SkBits2Float(0xc10a78c9), SkBits2Float(0x42aa55de), SkBits2Float(0x41c8b65d), SkBits2Float(0x42843343), SkBits2Float(0x4248ca15));
path.lineTo(SkBits2Float(0x423f2206), SkBits2Float(0x42112621));
path.cubicTo(SkBits2Float(0x427644a6), SkBits2Float(0x419117e2), SkBits2Float(0x427f8241), SkBits2Float(0xc0c83353), SkBits2Float(0x4256fbc4), SkBits2Float(0xc1d55fc8));
path.cubicTo(SkBits2Float(0x422e7546), SkBits2Float(0xc23c595d), SkBits2Float(0x41b6544b), SkBits2Float(0xc2700002), SkBits2Float(0x357ffa8c), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42843344), SkBits2Float(0x4248ca14));
path.cubicTo(SkBits2Float(0x4279865a), SkBits2Float(0x425c60b2), SkBits2Float(0x426884b7), SkBits2Float(0x426e4097), SkBits2Float(0x4255b1c1), SkBits2Float(0x427e1584));
path.lineTo(SkBits2Float(0x421a7a55), SkBits2Float(0x4237acdc));
path.cubicTo(SkBits2Float(0x422815ec), SkBits2Float(0x422c3b08), SkBits2Float(0x42346121), SkBits2Float(0x421f4f28), SkBits2Float(0x423f2207), SkBits2Float(0x42112621));
path.lineTo(SkBits2Float(0x42843344), SkBits2Float(0x4248ca14));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp264(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fc38da), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4271556b), SkBits2Float(0xc2824656), SkBits2Float(0x4294b266), SkBits2Float(0xc213956f));
path.cubicTo(SkBits2Float(0x42b0ba15), SkBits2Float(0xc10a78c9), SkBits2Float(0x42aa55de), SkBits2Float(0x41c8b65d), SkBits2Float(0x42843344), SkBits2Float(0x4248ca14));
path.cubicTo(SkBits2Float(0x4279865a), SkBits2Float(0x425c60b2), SkBits2Float(0x426884b7), SkBits2Float(0x426e4097), SkBits2Float(0x4255b1c1), SkBits2Float(0x427e1584));
path.lineTo(SkBits2Float(0x421a7a55), SkBits2Float(0x4237acdc));
path.cubicTo(SkBits2Float(0x422815ec), SkBits2Float(0x422c3b08), SkBits2Float(0x42346121), SkBits2Float(0x421f4f28), SkBits2Float(0x423f2206), SkBits2Float(0x42112621));
path.cubicTo(SkBits2Float(0x427644a6), SkBits2Float(0x419117e2), SkBits2Float(0x427f8241), SkBits2Float(0xc0c83353), SkBits2Float(0x4256fbc4), SkBits2Float(0xc1d55fc8));
path.cubicTo(SkBits2Float(0x422e7546), SkBits2Float(0xc23c595d), SkBits2Float(0x41b6544b), SkBits2Float(0xc2700002), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4255b1c2), SkBits2Float(0x427e1586));
path.cubicTo(SkBits2Float(0x41d9eb88), SkBits2Float(0x42ab15b8), SkBits2Float(0xc11c5ee2), SkBits2Float(0x42b27b8c), SkBits2Float(0xc21f2fec), SkBits2Float(0x4291ac82));
path.cubicTo(SkBits2Float(0xc28ba40f), SkBits2Float(0x4261baf0), SkBits2Float(0xc2ad6782), SkBits2Float(0x41ba4aab), SkBits2Float(0xc2a4a120), SkBits2Float(0xc12a4d95));
path.cubicTo(SkBits2Float(0xc29bdabd), SkBits2Float(0xc2324c20), SkBits2Float(0xc254adab), SkBits2Float(0xc290ac19), SkBits2Float(0xc19fafc0), SkBits2Float(0xc2a120ca));
path.lineTo(SkBits2Float(0xc166df50), SkBits2Float(0xc268f4ce));
path.cubicTo(SkBits2Float(0xc219be54), SkBits2Float(0xc2512a28), SkBits2Float(0xc26154eb), SkBits2Float(0xc200e3bb), SkBits2Float(0xc26e04b2), SkBits2Float(0xc0f6387e));
path.cubicTo(SkBits2Float(0xc27ab479), SkBits2Float(0x4186ab35), SkBits2Float(0xc249e3ea), SkBits2Float(0x42232db1), SkBits2Float(0xc1e62664), SkBits2Float(0x42529ce0));
path.cubicTo(SkBits2Float(0xc0e213c9), SkBits2Float(0x42810608), SkBits2Float(0x419d8860), SkBits2Float(0x427759fd), SkBits2Float(0x421a7a58), SkBits2Float(0x4237acda));
path.lineTo(SkBits2Float(0x4255b1c2), SkBits2Float(0x427e1586));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp265(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fe7454), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427343e8), SkBits2Float(0xc281a57b), SkBits2Float(0x429560d9), SkBits2Float(0xc210ce12));
path.cubicTo(SkBits2Float(0x42b11fbd), SkBits2Float(0xc0f2896e), SkBits2Float(0x42a9b750), SkBits2Float(0x41d3a0ba), SkBits2Float(0x42824e39), SkBits2Float(0x424daf12));
path.lineTo(SkBits2Float(0x423c64bf), SkBits2Float(0x4214afea));
path.cubicTo(SkBits2Float(0x42755f66), SkBits2Float(0x4198fbec), SkBits2Float(0x42800a9d), SkBits2Float(0xc0af53e2), SkBits2Float(0x4257f7fc), SkBits2Float(0xc1d15b49));
path.cubicTo(SkBits2Float(0x422fdabc), SkBits2Float(0xc23b70cc), SkBits2Float(0x41b7f168), SkBits2Float(0xc2700002), SkBits2Float(0xb5600574), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42824e38), SkBits2Float(0x424daf15));
path.cubicTo(SkBits2Float(0x42753e9a), SkBits2Float(0x4261276c), SkBits2Float(0x4263be9a), SkBits2Float(0x4272d73c), SkBits2Float(0x4250704b), SkBits2Float(0x428134df));
path.lineTo(SkBits2Float(0x4216adb6), SkBits2Float(0x423acdfc));
path.cubicTo(SkBits2Float(0x4224a276), SkBits2Float(0x422f8c2c), SkBits2Float(0x42314905), SkBits2Float(0x4222c30f), SkBits2Float(0x423c64c0), SkBits2Float(0x4214afec));
path.lineTo(SkBits2Float(0x42824e38), SkBits2Float(0x424daf15));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp266(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x41fe7454), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427343e8), SkBits2Float(0xc281a57b), SkBits2Float(0x429560d9), SkBits2Float(0xc210ce12));
path.cubicTo(SkBits2Float(0x42b11fbd), SkBits2Float(0xc0f2896e), SkBits2Float(0x42a9b750), SkBits2Float(0x41d3a0ba), SkBits2Float(0x42824e39), SkBits2Float(0x424daf12));
path.lineTo(SkBits2Float(0x42824e38), SkBits2Float(0x424daf15));
path.cubicTo(SkBits2Float(0x42753e9a), SkBits2Float(0x4261276c), SkBits2Float(0x4263be9a), SkBits2Float(0x4272d73c), SkBits2Float(0x4250704b), SkBits2Float(0x428134df));
path.lineTo(SkBits2Float(0x4216adb6), SkBits2Float(0x423acdfc));
path.cubicTo(SkBits2Float(0x4224a276), SkBits2Float(0x422f8c2c), SkBits2Float(0x42314905), SkBits2Float(0x4222c30f), SkBits2Float(0x423c64c0), SkBits2Float(0x4214afec));
path.lineTo(SkBits2Float(0x423c64bf), SkBits2Float(0x4214afea));
path.cubicTo(SkBits2Float(0x42755f66), SkBits2Float(0x4198fbec), SkBits2Float(0x42800a9d), SkBits2Float(0xc0af53e2), SkBits2Float(0x4257f7fc), SkBits2Float(0xc1d15b49));
path.cubicTo(SkBits2Float(0x422fdabc), SkBits2Float(0xc23b70cc), SkBits2Float(0x41b7f168), SkBits2Float(0xc2700002), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4250704d), SkBits2Float(0x428134e0));
path.cubicTo(SkBits2Float(0x41c9effb), SkBits2Float(0x42ac8cba), SkBits2Float(0xc143bd6b), SkBits2Float(0x42b21c58), SkBits2Float(0xc2280561), SkBits2Float(0x428f2c0c));
path.cubicTo(SkBits2Float(0xc28f8db2), SkBits2Float(0x42587782), SkBits2Float(0xc2af41ba), SkBits2Float(0x41a05b8a), SkBits2Float(0xc2a3a0d2), SkBits2Float(0xc15fb01a));
path.cubicTo(SkBits2Float(0xc297ffea), SkBits2Float(0xc24005d3), SkBits2Float(0xc246ef26), SkBits2Float(0xc295c2d5), SkBits2Float(0xc17d9b57), SkBits2Float(0xc2a2f1e8));
path.lineTo(SkBits2Float(0xc1375488), SkBits2Float(0xc26b9543));
path.cubicTo(SkBits2Float(0xc20fcecd), SkBits2Float(0xc25885a3), SkBits2Float(0xc25bc22e), SkBits2Float(0xc20acfc5), SkBits2Float(0xc26c9222), SkBits2Float(0xc121b3b7));
path.cubicTo(SkBits2Float(0xc27d6216), SkBits2Float(0x4167d7a5), SkBits2Float(0xc24f8c13), SkBits2Float(0x421c7b68), SkBits2Float(0xc1f2ebf9), SkBits2Float(0x424efee8));
path.cubicTo(SkBits2Float(0xc10d7f99), SkBits2Float(0x4280c134), SkBits2Float(0x4191fa9e), SkBits2Float(0x4279782f), SkBits2Float(0x4216adb8), SkBits2Float(0x423acdfc));
path.lineTo(SkBits2Float(0x4250704d), SkBits2Float(0x428134e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp267(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42003b3a), SkBits2Float(0xc2a60000), SkBits2Float(0x4274ff8d), SkBits2Float(0xc28113a0), SkBits2Float(0x4295fac2), SkBits2Float(0xc20e4c24));
path.cubicTo(SkBits2Float(0x42b175be), SkBits2Float(0xc0d38840), SkBits2Float(0x42a91fa3), SkBits2Float(0x41dd6a3d), SkBits2Float(0x42809081), SkBits2Float(0x4252054f));
path.lineTo(SkBits2Float(0x4239e059), SkBits2Float(0x4217d27c));
path.cubicTo(SkBits2Float(0x4274841b), SkBits2Float(0x41a00f1c), SkBits2Float(0x428048c8), SkBits2Float(0xc098ea38), SkBits2Float(0x4258d681), SkBits2Float(0xc1cdbb32));
path.cubicTo(SkBits2Float(0x42311b71), SkBits2Float(0xc23a9deb), SkBits2Float(0x41b96511), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42809082), SkBits2Float(0x4252054e));
path.cubicTo(SkBits2Float(0x4271521d), SkBits2Float(0x42655feb), SkBits2Float(0x425f60c7), SkBits2Float(0x4276e1ca), SkBits2Float(0x424ba43f), SkBits2Float(0x42831ae1));
path.lineTo(SkBits2Float(0x421335f7), SkBits2Float(0x423d8ca7));
path.cubicTo(SkBits2Float(0x42217a65), SkBits2Float(0x4232780c), SkBits2Float(0x422e72e3), SkBits2Float(0x4225d023), SkBits2Float(0x4239e05a), SkBits2Float(0x4217d27c));
path.lineTo(SkBits2Float(0x42809082), SkBits2Float(0x4252054e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp268(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42003b3a), SkBits2Float(0xc2a60000), SkBits2Float(0x4274ff8d), SkBits2Float(0xc28113a0), SkBits2Float(0x4295fac2), SkBits2Float(0xc20e4c24));
path.cubicTo(SkBits2Float(0x42b175be), SkBits2Float(0xc0d38840), SkBits2Float(0x42a91fa3), SkBits2Float(0x41dd6a3d), SkBits2Float(0x42809082), SkBits2Float(0x4252054e));
path.cubicTo(SkBits2Float(0x4271521d), SkBits2Float(0x42655feb), SkBits2Float(0x425f60c7), SkBits2Float(0x4276e1ca), SkBits2Float(0x424ba43f), SkBits2Float(0x42831ae1));
path.lineTo(SkBits2Float(0x421335f7), SkBits2Float(0x423d8ca7));
path.cubicTo(SkBits2Float(0x42217a65), SkBits2Float(0x4232780c), SkBits2Float(0x422e72e3), SkBits2Float(0x4225d023), SkBits2Float(0x4239e059), SkBits2Float(0x4217d27c));
path.cubicTo(SkBits2Float(0x4274841b), SkBits2Float(0x41a00f1c), SkBits2Float(0x428048c8), SkBits2Float(0xc098ea38), SkBits2Float(0x4258d681), SkBits2Float(0xc1cdbb32));
path.cubicTo(SkBits2Float(0x42311b71), SkBits2Float(0xc23a9deb), SkBits2Float(0x41b96511), SkBits2Float(0xc2700000), SkBits2Float(0x3697ff52), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x424ba440), SkBits2Float(0x42831ae2));
path.cubicTo(SkBits2Float(0x41bb72ba), SkBits2Float(0x42adc9b8), SkBits2Float(0xc16714ca), SkBits2Float(0x42b1a998), SkBits2Float(0xc22fd30d), SkBits2Float(0x428ccf5c));
path.cubicTo(SkBits2Float(0xc292f074), SkBits2Float(0x424fea41), SkBits2Float(0xc2b0b757), SkBits2Float(0x4188cdbd), SkBits2Float(0xc2a27f7d), SkBits2Float(0xc187abb1));
path.cubicTo(SkBits2Float(0xc29447a3), SkBits2Float(0xc24c1290), SkBits2Float(0xc23a2b5e), SkBits2Float(0xc29a0e93), SkBits2Float(0xc141f42b), SkBits2Float(0xc2a43853));
path.lineTo(SkBits2Float(0xc10c3538), SkBits2Float(0xc26d6d31));
path.cubicTo(SkBits2Float(0xc2069491), SkBits2Float(0xc25ebb9d), SkBits2Float(0xc2566164), SkBits2Float(0xc21385b2), SkBits2Float(0xc26aefd1), SkBits2Float(0xc1442672));
path.cubicTo(SkBits2Float(0xc27f7e3e), SkBits2Float(0x4145c9dc), SkBits2Float(0xc2547130), SkBits2Float(0x42164ccc), SkBits2Float(0xc1fe3427), SkBits2Float(0x424b94a6));
path.cubicTo(SkBits2Float(0xc1270bd9), SkBits2Float(0x42806e40), SkBits2Float(0x41878138), SkBits2Float(0x427b4278), SkBits2Float(0x421335f8), SkBits2Float(0x423d8ca8));
path.lineTo(SkBits2Float(0x424ba440), SkBits2Float(0x42831ae2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp269(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42011047), SkBits2Float(0xc2a60000), SkBits2Float(0x42766e56), SkBits2Float(0xc28099ef), SkBits2Float(0x42967824), SkBits2Float(0xc20c36c8));
path.cubicTo(SkBits2Float(0x42b1b91c), SkBits2Float(0xc0b9cd9b), SkBits2Float(0x42a89b7a), SkBits2Float(0x41e5804f), SkBits2Float(0x427e310b), SkBits2Float(0x42559106));
path.lineTo(SkBits2Float(0x4237c0bf), SkBits2Float(0x421a62ac));
path.cubicTo(SkBits2Float(0x4273c506), SkBits2Float(0x41a5e791), SkBits2Float(0x4280797a), SkBits2Float(0xc08650bf), SkBits2Float(0x42598bc5), SkBits2Float(0xc1cab811));
path.cubicTo(SkBits2Float(0x42322494), SkBits2Float(0xc239edfa), SkBits2Float(0x41ba9913), SkBits2Float(0xc2700002), SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427e3109), SkBits2Float(0x42559108));
path.cubicTo(SkBits2Float(0x426e0477), SkBits2Float(0x4268d13b), SkBits2Float(0x425bb575), SkBits2Float(0x427a2b1d), SkBits2Float(0x42479e2a), SkBits2Float(0x4284a4a0));
path.lineTo(SkBits2Float(0x42104d52), SkBits2Float(0x423fc5ea));
path.cubicTo(SkBits2Float(0x421ed35e), SkBits2Float(0x4234d83a), SkBits2Float(0x422c0f91), SkBits2Float(0x42284d3a), SkBits2Float(0x4237c0bf), SkBits2Float(0x421a62ad));
path.lineTo(SkBits2Float(0x427e3109), SkBits2Float(0x42559108));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp270(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb7060057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42011047), SkBits2Float(0xc2a60000), SkBits2Float(0x42766e56), SkBits2Float(0xc28099ef), SkBits2Float(0x42967824), SkBits2Float(0xc20c36c8));
path.cubicTo(SkBits2Float(0x42b1b91c), SkBits2Float(0xc0b9cd9b), SkBits2Float(0x42a89b7a), SkBits2Float(0x41e5804f), SkBits2Float(0x427e310b), SkBits2Float(0x42559106));
path.lineTo(SkBits2Float(0x4237c0bf), SkBits2Float(0x421a62ad));

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42479e29), SkBits2Float(0x4284a4a0));
path.cubicTo(SkBits2Float(0x41af5d68), SkBits2Float(0x42aec1b4), SkBits2Float(0xc1822698), SkBits2Float(0x42b135a9), SkBits2Float(0xc2362f3e), SkBits2Float(0x428ac623));
path.cubicTo(SkBits2Float(0xc295a599), SkBits2Float(0x4248ad36), SkBits2Float(0xc2b1c6ab), SkBits2Float(0x416a48a9), SkBits2Float(0xc2a165f3), SkBits2Float(0xc19b42cf));
path.cubicTo(SkBits2Float(0xc291053c), SkBits2Float(0xc255d4f6), SkBits2Float(0xc22f520a), SkBits2Float(0xc29d68ba), SkBits2Float(0xc110422a), SkBits2Float(0xc2a50486));
path.lineTo(SkBits2Float(0xc0d09136), SkBits2Float(0xc26e946c));
path.cubicTo(SkBits2Float(0xc1fd79b9), SkBits2Float(0xc2639452), SkBits2Float(0xc251ab0b), SkBits2Float(0xc21a93c1), SkBits2Float(0xc26958c8), SkBits2Float(0xc1607927));
path.cubicTo(SkBits2Float(0xc2808342), SkBits2Float(0x41295cae), SkBits2Float(0xc2585b55), SkBits2Float(0x42111142), SkBits2Float(0xc203b318), SkBits2Float(0x4248a313));
path.cubicTo(SkBits2Float(0xc13c2b63), SkBits2Float(0x42801a73), SkBits2Float(0x417d8a30), SkBits2Float(0x427ca903), SkBits2Float(0x42104d56), SkBits2Float(0x423fc5e8));
path.lineTo(SkBits2Float(0x42479e29), SkBits2Float(0x4284a4a0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp271(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4201b43a), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4277880a), SkBits2Float(0xc2803bc7), SkBits2Float(0x4296d747), SkBits2Float(0xc20a9b85));
path.cubicTo(SkBits2Float(0x42b1ea89), SkBits2Float(0xc0a5fbe3), SkBits2Float(0x42a831cc), SkBits2Float(0x41ebb52f), SkBits2Float(0x427be65b), SkBits2Float(0x425843c9));
path.lineTo(SkBits2Float(0x423618a6), SkBits2Float(0x421c5604));
path.cubicTo(SkBits2Float(0x42732c40), SkBits2Float(0x41aa6424), SkBits2Float(0x42809d37), SkBits2Float(0xc06ffa1c), SkBits2Float(0x425a1555), SkBits2Float(0xc1c8657d));
path.cubicTo(SkBits2Float(0x4232f03c), SkBits2Float(0xc23965db), SkBits2Float(0x41bb8620), SkBits2Float(0xc2700002), SkBits2Float(0xb5600574), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x427be65e), SkBits2Float(0x425843c9));
path.cubicTo(SkBits2Float(0x426b71bd), SkBits2Float(0x426b6e8c), SkBits2Float(0x4258dad9), SkBits2Float(0x427ca87a), SkBits2Float(0x42447e14), SkBits2Float(0x4285cdfb));
path.lineTo(SkBits2Float(0x420e0af4), SkBits2Float(0x424173d3));
path.cubicTo(SkBits2Float(0x421cc338), SkBits2Float(0x4236a4f9), SkBits2Float(0x422a3361), SkBits2Float(0x422a3113), SkBits2Float(0x423618a6), SkBits2Float(0x421c5605));
path.lineTo(SkBits2Float(0x427be65e), SkBits2Float(0x425843c9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp272(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4201b43a), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4277880a), SkBits2Float(0xc2803bc7), SkBits2Float(0x4296d747), SkBits2Float(0xc20a9b85));
path.cubicTo(SkBits2Float(0x42b1ea89), SkBits2Float(0xc0a5fbe3), SkBits2Float(0x42a831cc), SkBits2Float(0x41ebb52f), SkBits2Float(0x427be65b), SkBits2Float(0x425843c9));
path.lineTo(SkBits2Float(0x427be65e), SkBits2Float(0x425843c9));
path.cubicTo(SkBits2Float(0x426b71bd), SkBits2Float(0x426b6e8c), SkBits2Float(0x4258dad9), SkBits2Float(0x427ca87a), SkBits2Float(0x42447e14), SkBits2Float(0x4285cdfb));
path.lineTo(SkBits2Float(0x420e0af4), SkBits2Float(0x424173d3));
path.cubicTo(SkBits2Float(0x421cc338), SkBits2Float(0x4236a4f9), SkBits2Float(0x422a3361), SkBits2Float(0x422a3113), SkBits2Float(0x423618a6), SkBits2Float(0x421c5605));
path.lineTo(SkBits2Float(0x423618a6), SkBits2Float(0x421c5604));
path.cubicTo(SkBits2Float(0x42732c40), SkBits2Float(0x41aa6424), SkBits2Float(0x42809d37), SkBits2Float(0xc06ffa1c), SkBits2Float(0x425a1555), SkBits2Float(0xc1c8657d));
path.cubicTo(SkBits2Float(0x4232f03c), SkBits2Float(0xc23965db), SkBits2Float(0x41bb8620), SkBits2Float(0xc2700002), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42447e16), SkBits2Float(0x4285cdfb));
path.cubicTo(SkBits2Float(0x41a605d7), SkBits2Float(0x42af776a), SkBits2Float(0xc18d5e26), SkBits2Float(0x42b0cfa2), SkBits2Float(0xc23b02ad), SkBits2Float(0x428928e1));
path.cubicTo(SkBits2Float(0xc297ab24), SkBits2Float(0x42430442), SkBits2Float(0xc2b27fa9), SkBits2Float(0x414bdf0d), SkBits2Float(0xc2a073c8), SkBits2Float(0xc1aa3a13));
path.cubicTo(SkBits2Float(0xc28e67e7), SkBits2Float(0xc25d31d4), SkBits2Float(0xc226d0a4), SkBits2Float(0xc29fdb7e), SkBits2Float(0xc0d3d11a), SkBits2Float(0xc2a578a5));
path.lineTo(SkBits2Float(0xc0991eb2), SkBits2Float(0xc26f3c4f));
path.cubicTo(SkBits2Float(0xc1f12d9c), SkBits2Float(0xc2671e82), SkBits2Float(0xc24de350), SkBits2Float(0xc21fe656), SkBits2Float(0xc267faa7), SkBits2Float(0xc1761c74));
path.cubicTo(SkBits2Float(0xc28108ff), SkBits2Float(0x4113607a), SkBits2Float(0xc25b4798), SkBits2Float(0x420cf9d1), SkBits2Float(0xc207302c), SkBits2Float(0x42464d9a));
path.cubicTo(SkBits2Float(0xc14c6303), SkBits2Float(0x427fa162), SkBits2Float(0x4170087f), SkBits2Float(0x427dafb7), SkBits2Float(0x420e0af6), SkBits2Float(0x424173d2));
path.lineTo(SkBits2Float(0x42447e16), SkBits2Float(0x4285cdfb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp273(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42023f77), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427876e4), SkBits2Float(0xc27fd6f4), SkBits2Float(0x42972728), SkBits2Float(0xc2093dbb));
path.cubicTo(SkBits2Float(0x42b212de), SkBits2Float(0xc0952410), SkBits2Float(0x42a7d55b), SkBits2Float(0x41f0f791), SkBits2Float(0x4279eebf), SkBits2Float(0x425a890b));
path.lineTo(SkBits2Float(0x4234ac95), SkBits2Float(0x421dfa35));
path.cubicTo(SkBits2Float(0x4272a697), SkBits2Float(0x41ae3171), SkBits2Float(0x4280ba5e), SkBits2Float(0xc057a00f), SkBits2Float(0x425a88d0), SkBits2Float(0xc1c66bc2));
path.cubicTo(SkBits2Float(0x42339ce5), SkBits2Float(0xc238f1c1), SkBits2Float(0x41bc4f6b), SkBits2Float(0xc2700002), SkBits2Float(0xb630015d), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4279eebd), SkBits2Float(0x425a890e));
path.cubicTo(SkBits2Float(0x42693cf3), SkBits2Float(0x426da0dc), SkBits2Float(0x42566929), SkBits2Float(0x427ebed8), SkBits2Float(0x4241d1ac), SkBits2Float(0x4286c6a2));
path.lineTo(SkBits2Float(0x420c1c33), SkBits2Float(0x4242db53));
path.cubicTo(SkBits2Float(0x421afee9), SkBits2Float(0x42382742), SkBits2Float(0x42289b18), SkBits2Float(0x422bc78f), SkBits2Float(0x4234ac94), SkBits2Float(0x421dfa34));
path.lineTo(SkBits2Float(0x4279eebd), SkBits2Float(0x425a890e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp274(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015d), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42023f77), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427876e4), SkBits2Float(0xc27fd6f4), SkBits2Float(0x42972728), SkBits2Float(0xc2093dbb));
path.cubicTo(SkBits2Float(0x42b212de), SkBits2Float(0xc0952410), SkBits2Float(0x42a7d55b), SkBits2Float(0x41f0f791), SkBits2Float(0x4279eebf), SkBits2Float(0x425a890b));
path.lineTo(SkBits2Float(0x4234ac95), SkBits2Float(0x421dfa35));
path.cubicTo(SkBits2Float(0x4272a697), SkBits2Float(0x41ae3171), SkBits2Float(0x4280ba5e), SkBits2Float(0xc057a00f), SkBits2Float(0x425a88d0), SkBits2Float(0xc1c66bc2));
path.cubicTo(SkBits2Float(0x42339ce5), SkBits2Float(0xc238f1c1), SkBits2Float(0x41bc4f6b), SkBits2Float(0xc2700002), SkBits2Float(0xb630015d), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4241d1ad), SkBits2Float(0x4286c6a2));
path.cubicTo(SkBits2Float(0x419e0f8e), SkBits2Float(0x42b00b7b), SkBits2Float(0xc196dfc4), SkBits2Float(0x42b07042), SkBits2Float(0xc23f0fa7), SkBits2Float(0x4287c1be));
path.cubicTo(SkBits2Float(0xc29957b6), SkBits2Float(0x423e2672), SkBits2Float(0xc2b30c7a), SkBits2Float(0x4131f351), SkBits2Float(0xc29f94d8), SkBits2Float(0xc1b6db1d));
path.cubicTo(SkBits2Float(0xc28c1d38), SkBits2Float(0xc26357ee), SkBits2Float(0xc21f7d48), SkBits2Float(0xc2a1d87d), SkBits2Float(0xc09294c7), SkBits2Float(0xc2a5bf3c));
path.lineTo(SkBits2Float(0xc053ec94), SkBits2Float(0xc26fa25d));
path.cubicTo(SkBits2Float(0xc1e69644), SkBits2Float(0xc269fe64), SkBits2Float(0xc24a931a), SkBits2Float(0xc224583b), SkBits2Float(0xc266b858), SkBits2Float(0xc1842f59));
path.cubicTo(SkBits2Float(0xc2816ecb), SkBits2Float(0x4100a388), SkBits2Float(0xc25db33b), SkBits2Float(0x42097539), SkBits2Float(0xc20a1dd2), SkBits2Float(0x4244465c));
path.cubicTo(SkBits2Float(0xc15a2194), SkBits2Float(0x427f177f), SkBits2Float(0x41648588), SkBits2Float(0x427e85cc), SkBits2Float(0x420c1c35), SkBits2Float(0x4242db52));
path.lineTo(SkBits2Float(0x4241d1ad), SkBits2Float(0x4286c6a2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp275(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4202aab9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42792ea4), SkBits2Float(0xc27f5acc), SkBits2Float(0x4297641b), SkBits2Float(0xc2082fee));
path.cubicTo(SkBits2Float(0x42b230e5), SkBits2Float(0xc0882884), SkBits2Float(0x42a78c73), SkBits2Float(0x41f502e3), SkBits2Float(0x4278676f), SkBits2Float(0x425c4571));
path.lineTo(SkBits2Float(0x423391b8), SkBits2Float(0x421f3b73));
path.cubicTo(SkBits2Float(0x42723d33), SkBits2Float(0x41b11ddb), SkBits2Float(0x4280d014), SkBits2Float(0xc044db05), SkBits2Float(0x425ae0f2), SkBits2Float(0xc1c4e5b3));
path.cubicTo(SkBits2Float(0x423421be), SkBits2Float(0xc2389802), SkBits2Float(0x41bcea83), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42786771), SkBits2Float(0x425c4570));
path.cubicTo(SkBits2Float(0x42678692), SkBits2Float(0x426f4e2b), SkBits2Float(0x425483f6), SkBits2Float(0x42802b0f), SkBits2Float(0x423fbf6b), SkBits2Float(0x428783bc));
path.lineTo(SkBits2Float(0x420a9ce1), SkBits2Float(0x4243ecb9));
path.cubicTo(SkBits2Float(0x4219a02a), SkBits2Float(0x42394dac), SkBits2Float(0x42275e32), SkBits2Float(0x422cfde6), SkBits2Float(0x423391b8), SkBits2Float(0x421f3b72));
path.lineTo(SkBits2Float(0x42786771), SkBits2Float(0x425c4570));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp276(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4202aab9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x42792ea4), SkBits2Float(0xc27f5acc), SkBits2Float(0x4297641b), SkBits2Float(0xc2082fee));
path.cubicTo(SkBits2Float(0x42b230e5), SkBits2Float(0xc0882884), SkBits2Float(0x42a78c73), SkBits2Float(0x41f502e3), SkBits2Float(0x4278676f), SkBits2Float(0x425c4571));
path.cubicTo(SkBits2Float(0x42678690), SkBits2Float(0x426f4e2b), SkBits2Float(0x425483f5), SkBits2Float(0x42802b0f), SkBits2Float(0x423fbf6b), SkBits2Float(0x428783bc));
path.lineTo(SkBits2Float(0x420a9ce1), SkBits2Float(0x4243ecb9));
path.cubicTo(SkBits2Float(0x4219a02a), SkBits2Float(0x42394dac), SkBits2Float(0x42275e32), SkBits2Float(0x422cfde7), SkBits2Float(0x423391b8), SkBits2Float(0x421f3b73));
path.lineTo(SkBits2Float(0x423391b8), SkBits2Float(0x421f3b72));
path.cubicTo(SkBits2Float(0x42723d33), SkBits2Float(0x41b11dd9), SkBits2Float(0x4280d014), SkBits2Float(0xc044db09), SkBits2Float(0x425ae0f2), SkBits2Float(0xc1c4e5b3));
path.cubicTo(SkBits2Float(0x423421be), SkBits2Float(0xc2389802), SkBits2Float(0x41bcea83), SkBits2Float(0xc2700000), SkBits2Float(0x3725ffa9), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423fbf6b), SkBits2Float(0x428783bc));
path.cubicTo(SkBits2Float(0x4197e908), SkBits2Float(0x42b0799e), SkBits2Float(0xc19e2f01), SkBits2Float(0x42b0215b), SkBits2Float(0xc24226b0), SkBits2Float(0x4286a80b));
path.cubicTo(SkBits2Float(0xc29a9aef), SkBits2Float(0x423a5d79), SkBits2Float(0xc2b36ebb), SkBits2Float(0x411dee4a), SkBits2Float(0xc29ede64), SkBits2Float(0xc1c087c1));
path.cubicTo(SkBits2Float(0xc28a4e0d), SkBits2Float(0xc2680353), SkBits2Float(0xc219c8f7), SkBits2Float(0xc2a351d0), SkBits2Float(0xc0409740), SkBits2Float(0xc2a5e40e));
path.lineTo(SkBits2Float(0xc00b391c), SkBits2Float(0xc26fd79b));
path.cubicTo(SkBits2Float(0xc1de5701), SkBits2Float(0xc26c1feb), SkBits2Float(0xc247f576), SkBits2Float(0xc227b85e), SkBits2Float(0xc265b08d), SkBits2Float(0xc18b2dac));
path.cubicTo(SkBits2Float(0xc281b5d1), SkBits2Float(0x40e45588), SkBits2Float(0xc25f8687), SkBits2Float(0x4206b8c8), SkBits2Float(0xc20c59a1), SkBits2Float(0x4242af19));
path.cubicTo(SkBits2Float(0xc164b2eb), SkBits2Float(0x427ea56a), SkBits2Float(0x415ba119), SkBits2Float(0x427f2508), SkBits2Float(0x420a9ce0), SkBits2Float(0x4243ecba));
path.lineTo(SkBits2Float(0x423fbf6b), SkBits2Float(0x428783bc));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp277(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4202f62b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4279afc7), SkBits2Float(0xc27f0340), SkBits2Float(0x42978eaf), SkBits2Float(0xc20771fd));
path.cubicTo(SkBits2Float(0x42b2457b), SkBits2Float(0xc07e0b91), SkBits2Float(0x42a7584a), SkBits2Float(0x41f7da1e), SkBits2Float(0x42775276), SkBits2Float(0x425d7c3f));
path.lineTo(SkBits2Float(0x4232c97e), SkBits2Float(0x42201c22));
path.cubicTo(SkBits2Float(0x4271f1c7), SkBits2Float(0x41b32b8d), SkBits2Float(0x4280def3), SkBits2Float(0xc037a5cf), SkBits2Float(0x425b1e7c), SkBits2Float(0xc1c3d316));
path.cubicTo(SkBits2Float(0x42347f10), SkBits2Float(0xc23858b9), SkBits2Float(0x41bd578b), SkBits2Float(0xc26fffff), SkBits2Float(0xb7240057), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42775277), SkBits2Float(0x425d7c41));
path.cubicTo(SkBits2Float(0x4266507b), SkBits2Float(0x42707a20), SkBits2Float(0x42532cff), SkBits2Float(0x4280b928), SkBits2Float(0x423e48db), SkBits2Float(0x42880779));
path.lineTo(SkBits2Float(0x42098e1c), SkBits2Float(0x4244ab32));
path.cubicTo(SkBits2Float(0x4218a83e), SkBits2Float(0x423a1b21), SkBits2Float(0x42267e0b), SkBits2Float(0x422dd6be), SkBits2Float(0x4232c97e), SkBits2Float(0x42201c22));
path.lineTo(SkBits2Float(0x42775277), SkBits2Float(0x425d7c41));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp278(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb7240057), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x4202f62b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x4279afc7), SkBits2Float(0xc27f0340), SkBits2Float(0x42978eaf), SkBits2Float(0xc20771fd));
path.cubicTo(SkBits2Float(0x42b2457b), SkBits2Float(0xc07e0b91), SkBits2Float(0x42a7584a), SkBits2Float(0x41f7da1e), SkBits2Float(0x42775276), SkBits2Float(0x425d7c3f));
path.lineTo(SkBits2Float(0x42775277), SkBits2Float(0x425d7c41));
path.cubicTo(SkBits2Float(0x4266507b), SkBits2Float(0x42707a20), SkBits2Float(0x42532cff), SkBits2Float(0x4280b928), SkBits2Float(0x423e48db), SkBits2Float(0x42880779));
path.lineTo(SkBits2Float(0x42098e1c), SkBits2Float(0x4244ab32));
path.cubicTo(SkBits2Float(0x4218a83e), SkBits2Float(0x423a1b21), SkBits2Float(0x42267e0b), SkBits2Float(0x422dd6be), SkBits2Float(0x4232c97e), SkBits2Float(0x42201c22));
path.cubicTo(SkBits2Float(0x4271f1c7), SkBits2Float(0x41b32b8d), SkBits2Float(0x4280def3), SkBits2Float(0xc037a5cf), SkBits2Float(0x425b1e7c), SkBits2Float(0xc1c3d316));
path.cubicTo(SkBits2Float(0x42347f10), SkBits2Float(0xc23858b9), SkBits2Float(0x41bd578b), SkBits2Float(0xc26fffff), SkBits2Float(0xb7240057), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423e48db), SkBits2Float(0x4288077a));
path.cubicTo(SkBits2Float(0x41939344), SkBits2Float(0x42b0c509), SkBits2Float(0xc1a3515b), SkBits2Float(0x42afe6ff), SkBits2Float(0xc2444efb), SkBits2Float(0x4285df44));
path.cubicTo(SkBits2Float(0xc29b7aa2), SkBits2Float(0x4237af14), SkBits2Float(0xc2b3ae7d), SkBits2Float(0x410fd2d1), SkBits2Float(0xc29e5879), SkBits2Float(0xc1c74e5b));
path.cubicTo(SkBits2Float(0xc2890275), SkBits2Float(0xc26b4310), SkBits2Float(0xc215bdd9), SkBits2Float(0xc2a45375), SkBits2Float(0xbff3abc7), SkBits2Float(0xc2a5f4d2));
path.lineTo(SkBits2Float(0xbfb025f0), SkBits2Float(0xc26fefd6));
path.cubicTo(SkBits2Float(0xc1d87e6f), SkBits2Float(0xc26d946b), SkBits2Float(0xc246160c), SkBits2Float(0xc22a11a0), SkBits2Float(0xc264eef0), SkBits2Float(0xc190139e));
path.cubicTo(SkBits2Float(0xc281e3ea), SkBits2Float(0x40cff015), SkBits2Float(0xc260c9f8), SkBits2Float(0x4204c898), SkBits2Float(0xc20de8e2), SkBits2Float(0x42418cd3));
path.cubicTo(SkBits2Float(0xc16c1f36), SkBits2Float(0x427e510e), SkBits2Float(0x41555c9e), SkBits2Float(0x427f9213), SkBits2Float(0x42098e1b), SkBits2Float(0x4244ab33));
path.lineTo(SkBits2Float(0x423e48db), SkBits2Float(0x4288077a));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp279(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420331e6), SkBits2Float(0xc2a60000), SkBits2Float(0x427a15f4), SkBits2Float(0xc27ebdd3), SkBits2Float(0x4297b03a), SkBits2Float(0xc206db86));
path.cubicTo(SkBits2Float(0x42b2557a), SkBits2Float(0xc06f9378), SkBits2Float(0x42a72e7e), SkBits2Float(0x41fa194a), SkBits2Float(0x4276762d), SkBits2Float(0x425e7148));
path.lineTo(SkBits2Float(0x42322a40), SkBits2Float(0x4220cd43));
path.cubicTo(SkBits2Float(0x4271b558), SkBits2Float(0x41b4cb56), SkBits2Float(0x4280ea83), SkBits2Float(0xc02d3004), SkBits2Float(0x425b4efa), SkBits2Float(0xc1c2f986));
path.cubicTo(SkBits2Float(0x4234c8ee), SkBits2Float(0xc2382686), SkBits2Float(0x41bdadf1), SkBits2Float(0xc26fffff), SkBits2Float(0x3707ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4276762e), SkBits2Float(0x425e7147));
path.cubicTo(SkBits2Float(0x42655a01), SkBits2Float(0x42716669), SkBits2Float(0x42521c84), SkBits2Float(0x428128fd), SkBits2Float(0x423d1f69), SkBits2Float(0x42886f05));
path.lineTo(SkBits2Float(0x4208b718), SkBits2Float(0x424540e7));
path.cubicTo(SkBits2Float(0x4217e344), SkBits2Float(0x423abccf), SkBits2Float(0x4225cbdd), SkBits2Float(0x422e818f), SkBits2Float(0x42322a41), SkBits2Float(0x4220cd43));
path.lineTo(SkBits2Float(0x4276762e), SkBits2Float(0x425e7147));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp280(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3707ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420331e6), SkBits2Float(0xc2a60000), SkBits2Float(0x427a15f4), SkBits2Float(0xc27ebdd3), SkBits2Float(0x4297b03a), SkBits2Float(0xc206db86));
path.cubicTo(SkBits2Float(0x42b2557a), SkBits2Float(0xc06f937f), SkBits2Float(0x42a72e7e), SkBits2Float(0x41fa1948), SkBits2Float(0x4276762e), SkBits2Float(0x425e7147));
path.lineTo(SkBits2Float(0x4276762d), SkBits2Float(0x425e7148));
path.cubicTo(SkBits2Float(0x42655a00), SkBits2Float(0x4271666a), SkBits2Float(0x42521c84), SkBits2Float(0x428128fd), SkBits2Float(0x423d1f69), SkBits2Float(0x42886f05));
path.lineTo(SkBits2Float(0x4208b718), SkBits2Float(0x424540e7));
path.cubicTo(SkBits2Float(0x4217e344), SkBits2Float(0x423abccf), SkBits2Float(0x4225cbdd), SkBits2Float(0x422e818f), SkBits2Float(0x42322a41), SkBits2Float(0x4220cd43));
path.lineTo(SkBits2Float(0x42322a40), SkBits2Float(0x4220cd43));
path.cubicTo(SkBits2Float(0x4271b558), SkBits2Float(0x41b4cb56), SkBits2Float(0x4280ea83), SkBits2Float(0xc02d3004), SkBits2Float(0x425b4efa), SkBits2Float(0xc1c2f986));
path.cubicTo(SkBits2Float(0x4234c8ee), SkBits2Float(0xc2382686), SkBits2Float(0x41bdadf1), SkBits2Float(0xc26fffff), SkBits2Float(0x3707ffa9), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423d1f69), SkBits2Float(0x42886f06));
path.cubicTo(SkBits2Float(0x4190236c), SkBits2Float(0x42b0ff8c), SkBits2Float(0xc1a760b7), SkBits2Float(0x42afb726), SkBits2Float(0xc24601c7), SkBits2Float(0x42853ece));
path.cubicTo(SkBits2Float(0xc29c2998), SkBits2Float(0x42358ced), SkBits2Float(0xc2b3ddd5), SkBits2Float(0x4104a433), SkBits2Float(0xc29deb35), SkBits2Float(0xc1cca70e));
path.cubicTo(SkBits2Float(0xc287f895), SkBits2Float(0xc26dd020), SkBits2Float(0xc21285d2), SkBits2Float(0xc2a51ade), SkBits2Float(0xbf83a2cf), SkBits2Float(0xc2a5fcbd));
path.lineTo(SkBits2Float(0xbf3e53cf), SkBits2Float(0xc26ffb48));
path.cubicTo(SkBits2Float(0xc1d3d71b), SkBits2Float(0xc26eb4b2), SkBits2Float(0xc24495a7), SkBits2Float(0xc22be9b4), SkBits2Float(0xc26450f5), SkBits2Float(0xc193f109));
path.cubicTo(SkBits2Float(0xc2820621), SkBits2Float(0x40bfc558), SkBits2Float(0xc261c6ea), SkBits2Float(0x42033dc6), SkBits2Float(0xc20f2333), SkBits2Float(0x4240a4d2));
path.cubicTo(SkBits2Float(0xc171fde8), SkBits2Float(0x427e0bde), SkBits2Float(0x4150649d), SkBits2Float(0x427fe6ab), SkBits2Float(0x4208b71a), SkBits2Float(0x424540e8));
path.lineTo(SkBits2Float(0x423d1f69), SkBits2Float(0x42886f06));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp281(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42035955), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427a595d), SkBits2Float(0xc27e8fe6), SkBits2Float(0x4297c647), SkBits2Float(0xc206781b));
path.cubicTo(SkBits2Float(0x42b25fdf), SkBits2Float(0xc0660504), SkBits2Float(0x42a712a2), SkBits2Float(0x41fb94c7), SkBits2Float(0x4275e43b), SkBits2Float(0x425f1290));
path.lineTo(SkBits2Float(0x4231c0be), SkBits2Float(0x422141dc));
path.cubicTo(SkBits2Float(0x42718d10), SkBits2Float(0x41b5ddaf), SkBits2Float(0x4280f208), SkBits2Float(0xc026476c), SkBits2Float(0x425b6edc), SkBits2Float(0xc1c269cb));
path.cubicTo(SkBits2Float(0x4234f9ab), SkBits2Float(0xc2380553), SkBits2Float(0x41bde6f3), SkBits2Float(0xc26fffff), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x4275e43b), SkBits2Float(0x425f1292));
path.cubicTo(SkBits2Float(0x4264b6c3), SkBits2Float(0x427201df), SkBits2Float(0x4251681e), SkBits2Float(0x42817283), SkBits2Float(0x423c5a8f), SkBits2Float(0x4288b309));
path.lineTo(SkBits2Float(0x420828ca), SkBits2Float(0x4245a33c));
path.cubicTo(SkBits2Float(0x421760db), SkBits2Float(0x423b2719), SkBits2Float(0x422555d9), SkBits2Float(0x422ef1ee), SkBits2Float(0x4231c0be), SkBits2Float(0x422141da));
path.lineTo(SkBits2Float(0x4275e43b), SkBits2Float(0x425f1292));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp282(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42035955), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427a595d), SkBits2Float(0xc27e8fe6), SkBits2Float(0x4297c647), SkBits2Float(0xc206781b));
path.cubicTo(SkBits2Float(0x42b25fdf), SkBits2Float(0xc0660504), SkBits2Float(0x42a712a2), SkBits2Float(0x41fb94c7), SkBits2Float(0x4275e43b), SkBits2Float(0x425f1290));
path.lineTo(SkBits2Float(0x4275e43b), SkBits2Float(0x425f1292));
path.cubicTo(SkBits2Float(0x4264b6c3), SkBits2Float(0x427201df), SkBits2Float(0x4251681e), SkBits2Float(0x42817283), SkBits2Float(0x423c5a8f), SkBits2Float(0x4288b309));
path.lineTo(SkBits2Float(0x420828ca), SkBits2Float(0x4245a33c));
path.cubicTo(SkBits2Float(0x421760db), SkBits2Float(0x423b2719), SkBits2Float(0x422555d9), SkBits2Float(0x422ef1f0), SkBits2Float(0x4231c0be), SkBits2Float(0x422141dc));
path.cubicTo(SkBits2Float(0x42718d10), SkBits2Float(0x41b5ddaf), SkBits2Float(0x4280f208), SkBits2Float(0xc026476c), SkBits2Float(0x425b6edc), SkBits2Float(0xc1c269cb));
path.cubicTo(SkBits2Float(0x4234f9ab), SkBits2Float(0xc2380553), SkBits2Float(0x41bde6f3), SkBits2Float(0xc26fffff), SkBits2Float(0x3637fea5), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423c5a8f), SkBits2Float(0x4288b30a));
path.cubicTo(SkBits2Float(0x418dddd4), SkBits2Float(0x42b12599), SkBits2Float(0xc1aa0e7c), SkBits2Float(0x42af96c0), SkBits2Float(0xc2471fb7), SkBits2Float(0x4284d41e));
path.cubicTo(SkBits2Float(0xc29c9c18), SkBits2Float(0x423422f8), SkBits2Float(0xc2b3fb95), SkBits2Float(0x40fa8096), SkBits2Float(0xc29da17e), SkBits2Float(0xc1d02ca0));
path.cubicTo(SkBits2Float(0xc2874768), SkBits2Float(0xc26f7cb1), SkBits2Float(0xc2106396), SkBits2Float(0xc2a59c4c), SkBits2Float(0xbee6b152), SkBits2Float(0xc2a5ff5f));
path.lineTo(SkBits2Float(0xbea6c49b), SkBits2Float(0xc26fff18));
path.cubicTo(SkBits2Float(0xc1d0c156), SkBits2Float(0xc26f6fd8), SkBits2Float(0xc2439580), SkBits2Float(0xc22d1f86), SkBits2Float(0xc263e663), SkBits2Float(0xc1967cc0));
path.cubicTo(SkBits2Float(0xc2821ba4), SkBits2Float(0x40b51622), SkBits2Float(0xc2626c73), SkBits2Float(0x4202381f), SkBits2Float(0xc20ff1e5), SkBits2Float(0x42400a93));
path.cubicTo(SkBits2Float(0xc175dd55), SkBits2Float(0x427ddd08), SkBits2Float(0x414d1bd1), SkBits2Float(0x42800ed7), SkBits2Float(0x420828d0), SkBits2Float(0x4245a338));
path.lineTo(SkBits2Float(0x423c5a8f), SkBits2Float(0x4288b30a));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp283(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42036bf7), SkBits2Float(0xc2a60000), SkBits2Float(0x427a7934), SkBits2Float(0xc27e7a35), SkBits2Float(0x4297d0ad), SkBits2Float(0xc2064926));
path.cubicTo(SkBits2Float(0x42b264c0), SkBits2Float(0xc061818a), SkBits2Float(0x42a70569), SkBits2Float(0x41fc47ee), SkBits2Float(0x42759f2d), SkBits2Float(0x425f5e99));
path.lineTo(SkBits2Float(0x42318ed2), SkBits2Float(0x422178d2));
path.cubicTo(SkBits2Float(0x427179f2), SkBits2Float(0x41b65f2f), SkBits2Float(0x4280f58f), SkBits2Float(0xc0230424), SkBits2Float(0x425b7de6), SkBits2Float(0xc1c225e6));
path.cubicTo(SkBits2Float(0x423510af), SkBits2Float(0xc237f5a4), SkBits2Float(0x41be01e5), SkBits2Float(0xc26fffff), SkBits2Float(0x3707ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42759f2b), SkBits2Float(0x425f5e9b));
path.cubicTo(SkBits2Float(0x42646988), SkBits2Float(0x42724b20), SkBits2Float(0x425112cb), SkBits2Float(0x42819524), SkBits2Float(0x423bfd7a), SkBits2Float(0x4288d30e));
path.lineTo(SkBits2Float(0x4207e580), SkBits2Float(0x4245d187));
path.cubicTo(SkBits2Float(0x4217232e), SkBits2Float(0x423b592c), SkBits2Float(0x42251e07), SkBits2Float(0x422f26e4), SkBits2Float(0x42318ed3), SkBits2Float(0x422178d2));
path.lineTo(SkBits2Float(0x42759f2b), SkBits2Float(0x425f5e9b));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp284(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3707ffa9), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42036bf7), SkBits2Float(0xc2a60000), SkBits2Float(0x427a7934), SkBits2Float(0xc27e7a35), SkBits2Float(0x4297d0ad), SkBits2Float(0xc2064926));
path.cubicTo(SkBits2Float(0x42b264c0), SkBits2Float(0xc061818a), SkBits2Float(0x42a70569), SkBits2Float(0x41fc47ee), SkBits2Float(0x42759f2d), SkBits2Float(0x425f5e99));
path.lineTo(SkBits2Float(0x42318ed3), SkBits2Float(0x422178d2));

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bfd7a), SkBits2Float(0x4288d30e));
path.cubicTo(SkBits2Float(0x418ccafd), SkBits2Float(0x42b13768), SkBits2Float(0xc1ab522b), SkBits2Float(0x42af873b), SkBits2Float(0xc247a66c), SkBits2Float(0x4284a188));
path.cubicTo(SkBits2Float(0xc29cd1e0), SkBits2Float(0x423377ac), SkBits2Float(0xc2b40936), SkBits2Float(0x40f384e7), SkBits2Float(0xc29d7e41), SkBits2Float(0xc1d1d5b9));
path.cubicTo(SkBits2Float(0xc286f34a), SkBits2Float(0xc2704657), SkBits2Float(0xc20f6108), SkBits2Float(0xc2a5d8cf), SkBits2Float(0xbe35f437), SkBits2Float(0xc2a5ffe6));
path.lineTo(SkBits2Float(0xbe038989), SkBits2Float(0xc26fffdc));
path.cubicTo(SkBits2Float(0xc1cf4b80), SkBits2Float(0xc26fc755), SkBits2Float(0xc2431bdf), SkBits2Float(0xc22db14d), SkBits2Float(0xc263b36c), SkBits2Float(0xc197b016));
path.cubicTo(SkBits2Float(0xc282257d), SkBits2Float(0x40b009af), SkBits2Float(0xc262ba31), SkBits2Float(0x4201bc49), SkBits2Float(0xc2105343), SkBits2Float(0x423fc16f));
path.cubicTo(SkBits2Float(0xc177b158), SkBits2Float(0x427dc695), SkBits2Float(0x414b8e67), SkBits2Float(0x42801bb6), SkBits2Float(0x4207e581), SkBits2Float(0x4245d188));
path.lineTo(SkBits2Float(0x423bfd7a), SkBits2Float(0x4288d30e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp285(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420374f9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427a8897), SkBits2Float(0xc27e6fb3), SkBits2Float(0x4297d5b1), SkBits2Float(0xc2063270));
path.cubicTo(SkBits2Float(0x42b26718), SkBits2Float(0xc05f52ba), SkBits2Float(0x42a6ff00), SkBits2Float(0x41fc9e87), SkBits2Float(0x42757dbf), SkBits2Float(0x425f8353));
path.lineTo(SkBits2Float(0x423176ab), SkBits2Float(0x4221935e));
path.cubicTo(SkBits2Float(0x427170b0), SkBits2Float(0x41b69dc5), SkBits2Float(0x4280f73f), SkBits2Float(0xc0217057), SkBits2Float(0x425b8525), SkBits2Float(0xc1c20512));
path.cubicTo(SkBits2Float(0x42351bcc), SkBits2Float(0xc237ee0d), SkBits2Float(0x41be0ee4), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757dc1), SkBits2Float(0x425f8353));
path.cubicTo(SkBits2Float(0x4264442b), SkBits2Float(0x42726e80), SkBits2Float(0x4250e985), SkBits2Float(0x4281a5dc), SkBits2Float(0x423bd072), SkBits2Float(0x4288e283));
path.lineTo(SkBits2Float(0x4207c4f4), SkBits2Float(0x4245e7df));
path.cubicTo(SkBits2Float(0x42170559), SkBits2Float(0x423b7158), SkBits2Float(0x42250305), SkBits2Float(0x422f4076), SkBits2Float(0x423176ac), SkBits2Float(0x4221935e));
path.lineTo(SkBits2Float(0x42757dc1), SkBits2Float(0x425f8353));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp286(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420374f9), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427a8897), SkBits2Float(0xc27e6fb3), SkBits2Float(0x4297d5b1), SkBits2Float(0xc2063270));
path.cubicTo(SkBits2Float(0x42b26718), SkBits2Float(0xc05f52c1), SkBits2Float(0x42a6ff01), SkBits2Float(0x41fc9e87), SkBits2Float(0x42757dc1), SkBits2Float(0x425f8353));
path.cubicTo(SkBits2Float(0x4264442b), SkBits2Float(0x42726e80), SkBits2Float(0x4250e985), SkBits2Float(0x4281a5dc), SkBits2Float(0x423bd072), SkBits2Float(0x4288e283));
path.lineTo(SkBits2Float(0x4207c4f4), SkBits2Float(0x4245e7df));
path.cubicTo(SkBits2Float(0x42170559), SkBits2Float(0x423b7158), SkBits2Float(0x42250305), SkBits2Float(0x422f4076), SkBits2Float(0x423176ab), SkBits2Float(0x4221935e));
path.cubicTo(SkBits2Float(0x427170b0), SkBits2Float(0x41b69dc5), SkBits2Float(0x4280f73f), SkBits2Float(0xc0217057), SkBits2Float(0x425b8525), SkBits2Float(0xc1c20512));
path.cubicTo(SkBits2Float(0x42351bcc), SkBits2Float(0xc237ee0d), SkBits2Float(0x41be0ee4), SkBits2Float(0xc26fffff), SkBits2Float(0xb630015b), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bd073), SkBits2Float(0x4288e283));
path.cubicTo(SkBits2Float(0x418c461b), SkBits2Float(0x42b13ffc), SkBits2Float(0xc1abee9c), SkBits2Float(0x42af7fac), SkBits2Float(0xc247e775), SkBits2Float(0x42848907));
path.cubicTo(SkBits2Float(0xc29cebcd), SkBits2Float(0x423324c4), SkBits2Float(0xc2b40fb2), SkBits2Float(0x40f02474), SkBits2Float(0xc29d6d1c), SkBits2Float(0xc1d2a316));
path.cubicTo(SkBits2Float(0xc286ca87), SkBits2Float(0xc270a7a6), SkBits2Float(0xc20ee3ea), SkBits2Float(0xc2a5f5e9), SkBits2Float(0xbd3ba09e), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0xbd0796d7), SkBits2Float(0xc26ffffe));
path.cubicTo(SkBits2Float(0xc1ce9695), SkBits2Float(0xc26ff16b), SkBits2Float(0xc242e0ee), SkBits2Float(0xc22df7a5), SkBits2Float(0xc2639aa3), SkBits2Float(0xc198448c));
path.cubicTo(SkBits2Float(0xc2822a2c), SkBits2Float(0x40ad98d0), SkBits2Float(0xc262dfac), SkBits2Float(0x4201805e), SkBits2Float(0xc2108243), SkBits2Float(0x423f9e03));
path.cubicTo(SkBits2Float(0xc178936c), SkBits2Float(0x427dbba8), SkBits2Float(0x414ace5d), SkBits2Float(0x428021e8), SkBits2Float(0x4207c4fa), SkBits2Float(0x4245e7dc));
path.lineTo(SkBits2Float(0x423bd073), SkBits2Float(0x4288e283));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp287(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420377c9), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8d67), SkBits2Float(0xc27e6c6d), SkBits2Float(0x4297d744), SkBits2Float(0xc2062b59));
path.cubicTo(SkBits2Float(0x42b267d3), SkBits2Float(0xc05ea43d), SkBits2Float(0x42a6fd01), SkBits2Float(0x41fcb991), SkBits2Float(0x42757351), SkBits2Float(0x425f8ecb));
path.lineTo(SkBits2Float(0x42316f1e), SkBits2Float(0x42219ba8));
path.cubicTo(SkBits2Float(0x42716dc9), SkBits2Float(0x41b6b154), SkBits2Float(0x4280f7c8), SkBits2Float(0xc020f212), SkBits2Float(0x425b876b), SkBits2Float(0xc1c1fad0));
path.cubicTo(SkBits2Float(0x42351f48), SkBits2Float(0xc237ebae), SkBits2Float(0x41be12f9), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757350), SkBits2Float(0x425f8ecb));
path.cubicTo(SkBits2Float(0x42643881), SkBits2Float(0x4272798e), SkBits2Float(0x4250dca0), SkBits2Float(0x4281ab15), SkBits2Float(0x423bc262), SkBits2Float(0x4288e756));
path.lineTo(SkBits2Float(0x4207bac8), SkBits2Float(0x4245eed9));
path.cubicTo(SkBits2Float(0x4216fc05), SkBits2Float(0x423b78e5), SkBits2Float(0x4224fa94), SkBits2Float(0x422f4874), SkBits2Float(0x42316f1f), SkBits2Float(0x42219baa));
path.lineTo(SkBits2Float(0x42757350), SkBits2Float(0x425f8ecb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp288(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420377c9), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8d67), SkBits2Float(0xc27e6c6d), SkBits2Float(0x4297d744), SkBits2Float(0xc2062b59));
path.cubicTo(SkBits2Float(0x42b267d3), SkBits2Float(0xc05ea43d), SkBits2Float(0x42a6fd01), SkBits2Float(0x41fcb991), SkBits2Float(0x42757351), SkBits2Float(0x425f8ecb));
path.lineTo(SkBits2Float(0x423bc262), SkBits2Float(0x4288e756));
path.lineTo(SkBits2Float(0x4207bac8), SkBits2Float(0x4245eed9));
path.cubicTo(SkBits2Float(0x4216fc05), SkBits2Float(0x423b78e5), SkBits2Float(0x4224fa94), SkBits2Float(0x422f4874), SkBits2Float(0x42316f1f), SkBits2Float(0x42219baa));
path.lineTo(SkBits2Float(0x42316f1e), SkBits2Float(0x42219ba8));
path.cubicTo(SkBits2Float(0x42716dc9), SkBits2Float(0x41b6b154), SkBits2Float(0x4280f7c8), SkBits2Float(0xc020f212), SkBits2Float(0x425b876b), SkBits2Float(0xc1c1fad0));
path.cubicTo(SkBits2Float(0x42351f48), SkBits2Float(0xc237ebae), SkBits2Float(0x41be12f9), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc261), SkBits2Float(0x4288e756));
path.cubicTo(SkBits2Float(0x418c1c95), SkBits2Float(0x42b142a6), SkBits2Float(0xc1ac1f7e), SkBits2Float(0x42af7d4d), SkBits2Float(0xc247fbc6), SkBits2Float(0x4284815d));
path.cubicTo(SkBits2Float(0xc29cf3e6), SkBits2Float(0x42330ad8), SkBits2Float(0xc2b411b5), SkBits2Float(0x40ef163d), SkBits2Float(0xc29d67bc), SkBits2Float(0xc1d2e345));
path.cubicTo(SkBits2Float(0xc286bdc4), SkBits2Float(0xc270c60d), SkBits2Float(0xc20ebcc7), SkBits2Float(0xc2a5feff), SkBits2Float(0xbb958372), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0xbb591ee2), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce5e0c), SkBits2Float(0xc26ffe8b), SkBits2Float(0xc242ce80), SkBits2Float(0xc22e0d9d), SkBits2Float(0xc26392e3), SkBits2Float(0xc19872ed));
path.cubicTo(SkBits2Float(0xc2822ba3), SkBits2Float(0x40acd588), SkBits2Float(0xc262eb66), SkBits2Float(0x42016da1), SkBits2Float(0xc21090f8), SkBits2Float(0x423f92f0));
path.cubicTo(SkBits2Float(0xc178da2a), SkBits2Float(0x427db83e), SkBits2Float(0x414a923f), SkBits2Float(0x428023d8), SkBits2Float(0x4207baca), SkBits2Float(0x4245eed8));
path.lineTo(SkBits2Float(0x423bc261), SkBits2Float(0x4288e756));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp289(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp290(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp291(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp292(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp293(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp294(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp295(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp296(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp297(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp298(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp299(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp300(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp301(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp302(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp303(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp304(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp305(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp306(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp307(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp308(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp309(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp310(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp311(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp312(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp313(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp314(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp315(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp316(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp317(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp318(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp319(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp320(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp321(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp322(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp323(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp324(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp325(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp326(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp327(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp328(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp329(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp330(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp331(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp332(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp333(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp334(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp335(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp336(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp337(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp338(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp339(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp340(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp341(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp342(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp343(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp344(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp345(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp346(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp347(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3d570205), SkBits2Float(0xc2a60000), SkBits2Float(0x3dd7026d), SkBits2Float(0xc2a5fffa), SkBits2Float(0x3e2141e6), SkBits2Float(0xc2a5ffed));
path.lineTo(SkBits2Float(0x3de92565), SkBits2Float(0xc26fffe4));
path.cubicTo(SkBits2Float(0x3d9b6fac), SkBits2Float(0xc26ffff9), SkBits2Float(0x3d1b715b), SkBits2Float(0xc2700002), SkBits2Float(0x365677c0), SkBits2Float(0xc2700002));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3e214267), SkBits2Float(0xc2a5ffec));
path.cubicTo(SkBits2Float(0x3e26a1f2), SkBits2Float(0xc2a5ffeb), SkBits2Float(0x3e2c025b), SkBits2Float(0xc2a5ffe9), SkBits2Float(0x3e3162c6), SkBits2Float(0xc2a5ffe7));
path.lineTo(SkBits2Float(0x3e003af5), SkBits2Float(0xc26fffde));
path.cubicTo(SkBits2Float(0x3df8b0d2), SkBits2Float(0xc26fffe0), SkBits2Float(0x3df0ead2), SkBits2Float(0xc26fffe2), SkBits2Float(0x3de924d4), SkBits2Float(0xc26fffe4));
path.lineTo(SkBits2Float(0x3e214267), SkBits2Float(0xc2a5ffec));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp348(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x365677c0), SkBits2Float(0xc2700002));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3d570205), SkBits2Float(0xc2a60000), SkBits2Float(0x3dd7026d), SkBits2Float(0xc2a5fffa), SkBits2Float(0x3e2141e6), SkBits2Float(0xc2a5ffed));
path.lineTo(SkBits2Float(0x3e0492ca), SkBits2Float(0xc28878a2));
path.lineTo(SkBits2Float(0x3e214267), SkBits2Float(0xc2a5ffec));
path.cubicTo(SkBits2Float(0x3e26a1f2), SkBits2Float(0xc2a5ffeb), SkBits2Float(0x3e2c025b), SkBits2Float(0xc2a5ffe9), SkBits2Float(0x3e3162c6), SkBits2Float(0xc2a5ffe7));
path.lineTo(SkBits2Float(0x3e003af5), SkBits2Float(0xc26fffde));
path.lineTo(SkBits2Float(0x3de92565), SkBits2Float(0xc26fffe4));
path.lineTo(SkBits2Float(0x3de924d4), SkBits2Float(0xc26fffe4));
path.cubicTo(SkBits2Float(0x3d9b6f4b), SkBits2Float(0xc26ffff9), SkBits2Float(0x3d1b70fa), SkBits2Float(0xc2700002), SkBits2Float(0x365677c0), SkBits2Float(0xc2700002));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3e3162a4), SkBits2Float(0xc2a5ffe8));
path.cubicTo(SkBits2Float(0x3e843f51), SkBits2Float(0xc2a5ffd1), SkBits2Float(0x3eafcce9), SkBits2Float(0xc2a5ffa8), SkBits2Float(0x3edb5a6f), SkBits2Float(0xc2a5ff6f));
path.lineTo(SkBits2Float(0x3e9e9160), SkBits2Float(0xc26fff2e));
path.cubicTo(SkBits2Float(0x3e7e2aec), SkBits2Float(0xc26fff82), SkBits2Float(0x3e3f3306), SkBits2Float(0xc26fffbd), SkBits2Float(0x3e003b0e), SkBits2Float(0xc26fffdf));
path.lineTo(SkBits2Float(0x3e3162a4), SkBits2Float(0xc2a5ffe8));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp349(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e678fda), SkBits2Float(0xc2a60000), SkBits2Float(0x3ee78f7d), SkBits2Float(0xc2a5ff87), SkBits2Float(0x3f2dab18), SkBits2Float(0xc2a5fe96));
path.lineTo(SkBits2Float(0x3efb15d4), SkBits2Float(0xc26ffdf3));
path.cubicTo(SkBits2Float(0x3ea764ab), SkBits2Float(0xc26fff52), SkBits2Float(0x3e2764f3), SkBits2Float(0xc2700000), SkBits2Float(0x35c73da0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f2daad3), SkBits2Float(0xc2a5fe95));
path.cubicTo(SkBits2Float(0x3f3374d8), SkBits2Float(0xc2a5fe7b), SkBits2Float(0x3f393eae), SkBits2Float(0xc2a5fe62), SkBits2Float(0x3f3f0885), SkBits2Float(0xc2a5fe46));
path.lineTo(SkBits2Float(0x3f0a18b8), SkBits2Float(0xc26ffd84));
path.cubicTo(SkBits2Float(0x3f05e964), SkBits2Float(0xc26ffdad), SkBits2Float(0x3f01ba2f), SkBits2Float(0xc26ffdd1), SkBits2Float(0x3efb15f0), SkBits2Float(0xc26ffdf5));
path.lineTo(SkBits2Float(0x3f2daad3), SkBits2Float(0xc2a5fe95));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp350(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e678fda), SkBits2Float(0xc2a60000), SkBits2Float(0x3ee78f7d), SkBits2Float(0xc2a5ff87), SkBits2Float(0x3f2dab18), SkBits2Float(0xc2a5fe96));
path.cubicTo(SkBits2Float(0x3f3374d8), SkBits2Float(0xc2a5fe7b), SkBits2Float(0x3f393eae), SkBits2Float(0xc2a5fe62), SkBits2Float(0x3f3f0885), SkBits2Float(0xc2a5fe46));
path.lineTo(SkBits2Float(0x3f0a18b8), SkBits2Float(0xc26ffd84));
path.cubicTo(SkBits2Float(0x3f05e964), SkBits2Float(0xc26ffdad), SkBits2Float(0x3f01ba2f), SkBits2Float(0xc26ffdd1), SkBits2Float(0x3efb15f0), SkBits2Float(0xc26ffdf5));
path.lineTo(SkBits2Float(0x3efb15d4), SkBits2Float(0xc26ffdf3));
path.cubicTo(SkBits2Float(0x3ea764ab), SkBits2Float(0xc26fff52), SkBits2Float(0x3e2764f3), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f3f0899), SkBits2Float(0xc2a5fe48));
path.cubicTo(SkBits2Float(0x3f8e6b81), SkBits2Float(0xc2a5fc98), SkBits2Float(0x3fbd51fb), SkBits2Float(0xc2a5f9aa), SkBits2Float(0x3fec36d3), SkBits2Float(0xc2a5f57e));
path.lineTo(SkBits2Float(0x3faac1d7), SkBits2Float(0xc26ff0d0));
path.cubicTo(SkBits2Float(0x3f88dbac), SkBits2Float(0xc26ff6d7), SkBits2Float(0x3f4de8bb), SkBits2Float(0xc26ffb13), SkBits2Float(0x3f0a18e7), SkBits2Float(0xc26ffd83));
path.lineTo(SkBits2Float(0x3f3f0899), SkBits2Float(0xc2a5fe48));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp351(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x403f62fc), SkBits2Float(0xc2a60000), SkBits2Float(0x40bf510b), SkBits2Float(0xc2a5ad41), SkBits2Float(0x410f39cc), SkBits2Float(0xc2a50821));
path.lineTo(SkBits2Float(0x40cf12cc), SkBits2Float(0xc26e99a0));
path.cubicTo(SkBits2Float(0x408a4d18), SkBits2Float(0xc26f885f), SkBits2Float(0x400a5a13), SkBits2Float(0xc2700000), SkBits2Float(0x36a6ff52), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x410f39cd), SkBits2Float(0xc2a50820));
path.cubicTo(SkBits2Float(0x4113fb3b), SkBits2Float(0xc2a4f79d), SkBits2Float(0x4118bbf1), SkBits2Float(0xc2a4e648), SkBits2Float(0x411d7be1), SkBits2Float(0xc2a4d421));
path.lineTo(SkBits2Float(0x40e3b008), SkBits2Float(0xc26e4e75));
path.cubicTo(SkBits2Float(0x40dcd206), SkBits2Float(0xc26e68b4), SkBits2Float(0x40d5f2eb), SkBits2Float(0xc26e81c3), SkBits2Float(0x40cf12c6), SkBits2Float(0xc26e99a1));
path.lineTo(SkBits2Float(0x410f39cd), SkBits2Float(0xc2a50820));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp352(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e0b17a8), SkBits2Float(0xc2a60000), SkBits2Float(0x3e8b179e), SkBits2Float(0xc2a5ffd4), SkBits2Float(0x3ed0a337), SkBits2Float(0xc2a5ff7c));
path.lineTo(SkBits2Float(0x3ed0a338), SkBits2Float(0xc2a5ff7d));
path.cubicTo(SkBits2Float(0x3ed797a0), SkBits2Float(0xc2a5ff73), SkBits2Float(0x3ede8c36), SkBits2Float(0xc2a5ff6a), SkBits2Float(0x3ee580cb), SkBits2Float(0xc2a5ff60));
path.lineTo(SkBits2Float(0x3ea5e78a), SkBits2Float(0xc26fff1b));
path.cubicTo(SkBits2Float(0x3ea0e0bb), SkBits2Float(0xc26fff29), SkBits2Float(0x3e9bd9a1), SkBits2Float(0xc26fff36), SkBits2Float(0x3e96d286), SkBits2Float(0xc26fff43));
path.lineTo(SkBits2Float(0x3e96d285), SkBits2Float(0xc26fff42));
path.cubicTo(SkBits2Float(0x3e491945), SkBits2Float(0xc26fffc2), SkBits2Float(0x3dc91958), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ee58048), SkBits2Float(0xc2a5ff61));
path.cubicTo(SkBits2Float(0x3f2b1987), SkBits2Float(0xc2a5fec4), SkBits2Float(0x3f637253), SkBits2Float(0xc2a5fdb6), SkBits2Float(0x3f8de535), SkBits2Float(0xc2a5fc35));
path.lineTo(SkBits2Float(0x3f4d269a), SkBits2Float(0xc26ffa85));
path.cubicTo(SkBits2Float(0x3f246b51), SkBits2Float(0xc26ffcb3), SkBits2Float(0x3ef75f30), SkBits2Float(0xc26ffe3a), SkBits2Float(0x3ea5e737), SkBits2Float(0xc26fff1c));
path.lineTo(SkBits2Float(0x3ee58048), SkBits2Float(0xc2a5ff61));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp1390(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xb7240057), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x420377ff), SkBits2Float(0xc2a5ffff), SkBits2Float(0x427a8dc0), SkBits2Float(0xc27e6c2f), SkBits2Float(0x4297d760), SkBits2Float(0xc2062ad2));
path.cubicTo(SkBits2Float(0x42b267e1), SkBits2Float(0xc05e974f), SkBits2Float(0x42a6fcda), SkBits2Float(0x41fcbb92), SkBits2Float(0x42757289), SkBits2Float(0x425f8fa5));
path.cubicTo(SkBits2Float(0x426437a0), SkBits2Float(0x42727a5f), SkBits2Float(0x4250dbaa), SkBits2Float(0x4281ab79), SkBits2Float(0x423bc155), SkBits2Float(0x4288e7b2));
path.lineTo(SkBits2Float(0x4207ba06), SkBits2Float(0x4245ef5e));
path.cubicTo(SkBits2Float(0x4216fb52), SkBits2Float(0x423b7973), SkBits2Float(0x4224f9f2), SkBits2Float(0x422f490a), SkBits2Float(0x42316e8e), SkBits2Float(0x42219c46));
path.cubicTo(SkBits2Float(0x42716d91), SkBits2Float(0x41b6b2c9), SkBits2Float(0x4280f7d1), SkBits2Float(0xc020e8c8), SkBits2Float(0x425b8794), SkBits2Float(0xc1c1fa0e));
path.cubicTo(SkBits2Float(0x42351f87), SkBits2Float(0xc237eb83), SkBits2Float(0x41be1342), SkBits2Float(0xc2700002), SkBits2Float(0xb7240057), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc156), SkBits2Float(0x4288e7b2));
path.cubicTo(SkBits2Float(0x418c1984), SkBits2Float(0x42b142da), SkBits2Float(0xc1ac2314), SkBits2Float(0x42af7d21), SkBits2Float(0xc247fd43), SkBits2Float(0x428480ce));
path.cubicTo(SkBits2Float(0xc29cf47f), SkBits2Float(0x423308f3), SkBits2Float(0xc2b411dd), SkBits2Float(0x40ef0242), SkBits2Float(0xc29d6757), SkBits2Float(0xc1d2e807));
path.cubicTo(SkBits2Float(0xc286bcd2), SkBits2Float(0xc270c84c), SkBits2Float(0xc20eb9e2), SkBits2Float(0xc2a5ffaa), SkBits2Float(0xbac6f0ca), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0xba901698), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce59d7), SkBits2Float(0xc26fff83), SkBits2Float(0xc242cd21), SkBits2Float(0xc22e0f3f), SkBits2Float(0xc263924f), SkBits2Float(0xc1987661));
path.cubicTo(SkBits2Float(0xc2822bbf), SkBits2Float(0x40acc6fd), SkBits2Float(0xc262ec43), SkBits2Float(0x42016c3b), SkBits2Float(0xc2109210), SkBits2Float(0x423f921c));
path.cubicTo(SkBits2Float(0xc178df72), SkBits2Float(0x427db7fc), SkBits2Float(0x414a8dba), SkBits2Float(0x428023fd), SkBits2Float(0x4207ba05), SkBits2Float(0x4245ef60));
path.lineTo(SkBits2Float(0x423bc156), SkBits2Float(0x4288e7b2));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp1391(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp1392(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp1393(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3c436965), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3cc36072), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d128619), SkBits2Float(0xc2a5fffe));
path.lineTo(SkBits2Float(0x3cd3db06), SkBits2Float(0xc26fffff));
path.cubicTo(SkBits2Float(0x3c8d3d03), SkBits2Float(0xc2700000), SkBits2Float(0x3c0d4407), SkBits2Float(0xc2700000), SkBits2Float(0x36606a00), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3d12888d), SkBits2Float(0xc2a5ffff));
path.cubicTo(SkBits2Float(0x3d176d55), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d1c4dcb), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d212e40), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x3ce90a84), SkBits2Float(0xc26ffffe));
path.cubicTo(SkBits2Float(0x3ce1ffb6), SkBits2Float(0xc26ffffe), SkBits2Float(0x3cdaedb6), SkBits2Float(0xc26fffff), SkBits2Float(0x3cd3dbb7), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x3d12888d), SkBits2Float(0xc2a5ffff));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp1394(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x36606a00), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3c436965), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3cc36072), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d128619), SkBits2Float(0xc2a5fffe));
path.lineTo(SkBits2Float(0x3d12888d), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x3d212e40), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x3ce90a84), SkBits2Float(0xc26ffffe));
path.cubicTo(SkBits2Float(0x3ce1ffb6), SkBits2Float(0xc26ffffe), SkBits2Float(0x3cdaedb6), SkBits2Float(0xc26fffff), SkBits2Float(0x3cd3db06), SkBits2Float(0xc26fffff));
path.cubicTo(SkBits2Float(0x3c8d3d03), SkBits2Float(0xc2700000), SkBits2Float(0x3c0d4407), SkBits2Float(0xc2700000), SkBits2Float(0x36606a00), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3d212fd0), SkBits2Float(0xc2a5ffff));
path.cubicTo(SkBits2Float(0x3d705530), SkBits2Float(0xc2a5fffe), SkBits2Float(0x3d9fbf82), SkBits2Float(0xc2a5fffc), SkBits2Float(0x3dc7546b), SkBits2Float(0xc2a5fffa));
path.lineTo(SkBits2Float(0x3d901696), SkBits2Float(0xc26ffff5));
path.cubicTo(SkBits2Float(0x3d66f230), SkBits2Float(0xc26ffff9), SkBits2Float(0x3d2dbab1), SkBits2Float(0xc26ffffc), SkBits2Float(0x3ce90664), SkBits2Float(0xc26ffffe));
path.lineTo(SkBits2Float(0x3d212fd0), SkBits2Float(0xc2a5ffff));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp1395(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e06023f), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e860192), SkBits2Float(0xc2a5ffd6), SkBits2Float(0x3ec901db), SkBits2Float(0xc2a5ff85));
path.lineTo(SkBits2Float(0x3e914e16), SkBits2Float(0xc26fff50));
path.cubicTo(SkBits2Float(0x3e41bddf), SkBits2Float(0xc26fffc5), SkBits2Float(0x3dc1be4c), SkBits2Float(0xc26fffff), SkBits2Float(0x35c55da0), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ec9015b), SkBits2Float(0xc2a5ff86));
path.cubicTo(SkBits2Float(0x3ecfb4f0), SkBits2Float(0xc2a5ff7d), SkBits2Float(0x3ed66842), SkBits2Float(0xc2a5ff75), SkBits2Float(0x3edd1b92), SkBits2Float(0xc2a5ff6c));
path.lineTo(SkBits2Float(0x3e9fd5de), SkBits2Float(0xc26fff2b));
path.cubicTo(SkBits2Float(0x3e9afe3a), SkBits2Float(0xc26fff39), SkBits2Float(0x3e96263d), SkBits2Float(0xc26fff45), SkBits2Float(0x3e914e41), SkBits2Float(0xc26fff51));
path.lineTo(SkBits2Float(0x3ec9015b), SkBits2Float(0xc2a5ff86));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp1396(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e0601e9), SkBits2Float(0xc2a60000), SkBits2Float(0x3e86013c), SkBits2Float(0xc2a5ffd6), SkBits2Float(0x3ec9015a), SkBits2Float(0xc2a5ff85));
path.lineTo(SkBits2Float(0x3ec9015b), SkBits2Float(0xc2a5ff86));
path.cubicTo(SkBits2Float(0x3ecfb4f0), SkBits2Float(0xc2a5ff7d), SkBits2Float(0x3ed66842), SkBits2Float(0xc2a5ff75), SkBits2Float(0x3edd1b92), SkBits2Float(0xc2a5ff6c));
path.lineTo(SkBits2Float(0x3e9fd5de), SkBits2Float(0xc26fff2b));
path.cubicTo(SkBits2Float(0x3e9afe3a), SkBits2Float(0xc26fff39), SkBits2Float(0x3e96263d), SkBits2Float(0xc26fff45), SkBits2Float(0x3e914e16), SkBits2Float(0xc26fff50));
path.cubicTo(SkBits2Float(0x3e41bddf), SkBits2Float(0xc26fffc5), SkBits2Float(0x3dc1be4c), SkBits2Float(0xc26fffff), SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3edd1b0d), SkBits2Float(0xc2a5ff6d));
path.cubicTo(SkBits2Float(0x3f24d70e), SkBits2Float(0xc2a5fedc), SkBits2Float(0x3f5b204e), SkBits2Float(0xc2a5fde1), SkBits2Float(0x3f88b475), SkBits2Float(0xc2a5fc7b));
path.lineTo(SkBits2Float(0x3f45a57e), SkBits2Float(0xc26ffaea));
path.cubicTo(SkBits2Float(0x3f1e67a6), SkBits2Float(0xc26ffcf1), SkBits2Float(0x3eee52e7), SkBits2Float(0xc26ffe5c), SkBits2Float(0x3e9fd606), SkBits2Float(0xc26fff2d));
path.lineTo(SkBits2Float(0x3edd1b0d), SkBits2Float(0xc2a5ff6d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp2193(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e3881bc), SkBits2Float(0xc2a60000), SkBits2Float(0x3eb88238), SkBits2Float(0xc2a5ffb3), SkBits2Float(0x3f0a6190), SkBits2Float(0xc2a5ff19));
path.lineTo(SkBits2Float(0x3ec8119b), SkBits2Float(0xc26ffeb2));
path.cubicTo(SkBits2Float(0x3e856151), SkBits2Float(0xc26fff91), SkBits2Float(0x3e0561b2), SkBits2Float(0xc2700000), SkBits2Float(0x3629eed0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f0a6183), SkBits2Float(0xc2a5ff19));
path.cubicTo(SkBits2Float(0x3f0efe46), SkBits2Float(0xc2a5ff0a), SkBits2Float(0x3f139b44), SkBits2Float(0xc2a5fef9), SkBits2Float(0x3f183842), SkBits2Float(0xc2a5fee9));
path.lineTo(SkBits2Float(0x3edc1349), SkBits2Float(0xc26ffe6c));
path.cubicTo(SkBits2Float(0x3ed567f5), SkBits2Float(0xc26ffe84), SkBits2Float(0x3ecebccf), SkBits2Float(0xc26ffe9c), SkBits2Float(0x3ec811a8), SkBits2Float(0xc26ffeb2));
path.lineTo(SkBits2Float(0x3f0a6183), SkBits2Float(0xc2a5ff19));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp2194(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3629eed0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e3881ab), SkBits2Float(0xc2a60000), SkBits2Float(0x3eb88227), SkBits2Float(0xc2a5ffb3), SkBits2Float(0x3f0a6183), SkBits2Float(0xc2a5ff19));
path.lineTo(SkBits2Float(0x3f0a6190), SkBits2Float(0xc2a5ff19));
path.cubicTo(SkBits2Float(0x3f0efe4f), SkBits2Float(0xc2a5ff0a), SkBits2Float(0x3f139b48), SkBits2Float(0xc2a5fef9), SkBits2Float(0x3f183842), SkBits2Float(0xc2a5fee9));
path.lineTo(SkBits2Float(0x3edc1349), SkBits2Float(0xc26ffe6c));
path.cubicTo(SkBits2Float(0x3ed567f5), SkBits2Float(0xc26ffe84), SkBits2Float(0x3ecebccf), SkBits2Float(0xc26ffe9c), SkBits2Float(0x3ec811a8), SkBits2Float(0xc26ffeb2));
path.lineTo(SkBits2Float(0x3ec8119b), SkBits2Float(0xc26ffeb2));
path.cubicTo(SkBits2Float(0x3e856151), SkBits2Float(0xc26fff91), SkBits2Float(0x3e0561b2), SkBits2Float(0xc2700000), SkBits2Float(0x3629eed0), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f183800), SkBits2Float(0xc2a5fee9));
path.cubicTo(SkBits2Float(0x3f62f7a2), SkBits2Float(0xc2a5fdd7), SkBits2Float(0x3f96db12), SkBits2Float(0xc2a5fbfa), SkBits2Float(0x3fbc3981), SkBits2Float(0xc2a5f954));
path.lineTo(SkBits2Float(0x3f8810cc), SkBits2Float(0xc26ff65b));
path.cubicTo(SkBits2Float(0x3f5a1a86), SkBits2Float(0xc26ffa2f), SkBits2Float(0x3f241256), SkBits2Float(0xc26ffcdf), SkBits2Float(0x3edc1312), SkBits2Float(0xc26ffe6c));
path.lineTo(SkBits2Float(0x3f183800), SkBits2Float(0xc2a5fee9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp3368(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp3369(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp3370(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp3371(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3c85f8a2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d05fda5), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d48fefa), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d114e3a), SkBits2Float(0xc26ffffd));
path.cubicTo(SkBits2Float(0x3cc1c2c0), SkBits2Float(0xc26fffff), SkBits2Float(0x3c41c57e), SkBits2Float(0xc26fffff), SkBits2Float(0x35afaa00), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3d49018c), SkBits2Float(0xc2a5fffe));
path.cubicTo(SkBits2Float(0x3d4fb7df), SkBits2Float(0xc2a5fffd), SkBits2Float(0x3d5667bf), SkBits2Float(0xc2a5fffd), SkBits2Float(0x3d5d179f), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d1fd60d), SkBits2Float(0xc26ffffd));
path.cubicTo(SkBits2Float(0x3d1afde4), SkBits2Float(0xc26fffff), SkBits2Float(0x3d162864), SkBits2Float(0xc26fffff), SkBits2Float(0x3d1152e4), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x3d49018c), SkBits2Float(0xc2a5fffe));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp3372(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3c85f8a2), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d05fda5), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d48fefa), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d49018c), SkBits2Float(0xc2a5fffe));
path.cubicTo(SkBits2Float(0x3d4fb7df), SkBits2Float(0xc2a5fffd), SkBits2Float(0x3d5667bf), SkBits2Float(0xc2a5fffd), SkBits2Float(0x3d5d179f), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d1fd60d), SkBits2Float(0xc26ffffd));
path.cubicTo(SkBits2Float(0x3d1afde4), SkBits2Float(0xc26fffff), SkBits2Float(0x3d162864), SkBits2Float(0xc26fffff), SkBits2Float(0x3d1152e4), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x3d114e3a), SkBits2Float(0xc26ffffd));
path.cubicTo(SkBits2Float(0x3cc1c2c0), SkBits2Float(0xc26fffff), SkBits2Float(0x3c41c57e), SkBits2Float(0xc26fffff), SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3d5d1b4e), SkBits2Float(0xc2a5fffe));
path.cubicTo(SkBits2Float(0x3da4d661), SkBits2Float(0xc2a5fffc), SkBits2Float(0x3ddb1fb1), SkBits2Float(0xc2a5fff8), SkBits2Float(0x3e08b47e), SkBits2Float(0xc2a5fff2));
path.lineTo(SkBits2Float(0x3dc5a6e0), SkBits2Float(0xc26fffec));
path.cubicTo(SkBits2Float(0x3d9e671d), SkBits2Float(0xc26ffff6), SkBits2Float(0x3d6e51bc), SkBits2Float(0xc26ffffb), SkBits2Float(0x3d1fd53d), SkBits2Float(0xc26ffffe));
path.lineTo(SkBits2Float(0x3d5d1b4e), SkBits2Float(0xc2a5fffe));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp4290(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp4291(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp4292(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp4293(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x357ffa94), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.cubicTo(SkBits2Float(0x42643732), SkBits2Float(0x42727ac8), SkBits2Float(0x4250db30), SkBits2Float(0x4281abaa), SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42757226), SkBits2Float(0x425f9012));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp4294(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x42037818), SkBits2Float(0xc2a60000), SkBits2Float(0x427a8dee), SkBits2Float(0xc27e6c10), SkBits2Float(0x4297d76f), SkBits2Float(0xc2062a8f));
path.cubicTo(SkBits2Float(0x42b267e8), SkBits2Float(0xc05e90e8), SkBits2Float(0x42a6fcc7), SkBits2Float(0x41fcbc94), SkBits2Float(0x42757227), SkBits2Float(0x425f9011));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.lineTo(SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.cubicTo(SkBits2Float(0x4216fafb), SkBits2Float(0x423b79ba), SkBits2Float(0x4224f9a4), SkBits2Float(0x422f4956), SkBits2Float(0x42316e48), SkBits2Float(0x42219c94));
path.lineTo(SkBits2Float(0x42316e47), SkBits2Float(0x42219c94));
path.cubicTo(SkBits2Float(0x42716d77), SkBits2Float(0x41b6b381), SkBits2Float(0x4280f7d6), SkBits2Float(0xc020e418), SkBits2Float(0x425b87ab), SkBits2Float(0xc1c1f9ac));
path.cubicTo(SkBits2Float(0x42351faa), SkBits2Float(0xc237eb6b), SkBits2Float(0x41be136b), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.cubicTo(SkBits2Float(0x418c17fd), SkBits2Float(0x42b142f1), SkBits2Float(0xc1ac24e4), SkBits2Float(0x42af7d09), SkBits2Float(0xc247fe03), SkBits2Float(0x42848083));
path.cubicTo(SkBits2Float(0xc29cf4c9), SkBits2Float(0x423307fa), SkBits2Float(0xc2b411ee), SkBits2Float(0x40eef84a), SkBits2Float(0xc29d6723), SkBits2Float(0xc1d2ea61));
path.cubicTo(SkBits2Float(0xc286bc59), SkBits2Float(0xc270c968), SkBits2Float(0xc20eb871), SkBits2Float(0xc2a5ffff), SkBits2Float(0xb5c727ee), SkBits2Float(0xc2a5ffff));
path.lineTo(SkBits2Float(0x293e5cb4), SkBits2Float(0xc2700000));
path.cubicTo(SkBits2Float(0xc1ce57c4), SkBits2Float(0xc2700000), SkBits2Float(0xc242cc76), SkBits2Float(0xc22e100c), SkBits2Float(0xc2639208), SkBits2Float(0xc1987810));
path.cubicTo(SkBits2Float(0xc2822bcd), SkBits2Float(0x40acbfe2), SkBits2Float(0xc262ecb3), SkBits2Float(0x42016b8c), SkBits2Float(0xc210929c), SkBits2Float(0x423f91b4));
path.cubicTo(SkBits2Float(0xc178e211), SkBits2Float(0x427db7dc), SkBits2Float(0x414a8b85), SkBits2Float(0x4280240f), SkBits2Float(0x4207b9a6), SkBits2Float(0x4245efa0));
path.lineTo(SkBits2Float(0x423bc0d1), SkBits2Float(0x4288e7e0));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp4295(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e3881bc), SkBits2Float(0xc2a60000), SkBits2Float(0x3eb88238), SkBits2Float(0xc2a5ffb3), SkBits2Float(0x3f0a6190), SkBits2Float(0xc2a5ff19));
path.lineTo(SkBits2Float(0x3ec8119b), SkBits2Float(0xc26ffeb2));
path.cubicTo(SkBits2Float(0x3e856151), SkBits2Float(0xc26fff91), SkBits2Float(0x3e0561b2), SkBits2Float(0xc2700000), SkBits2Float(0x3629eed0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f0a6183), SkBits2Float(0xc2a5ff19));
path.cubicTo(SkBits2Float(0x3f0efe46), SkBits2Float(0xc2a5ff0a), SkBits2Float(0x3f139b44), SkBits2Float(0xc2a5fef9), SkBits2Float(0x3f183842), SkBits2Float(0xc2a5fee9));
path.lineTo(SkBits2Float(0x3edc1349), SkBits2Float(0xc26ffe6c));
path.cubicTo(SkBits2Float(0x3ed567f5), SkBits2Float(0xc26ffe84), SkBits2Float(0x3ecebccf), SkBits2Float(0xc26ffe9c), SkBits2Float(0x3ec811a8), SkBits2Float(0xc26ffeb2));
path.lineTo(SkBits2Float(0x3f0a6183), SkBits2Float(0xc2a5ff19));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp4296(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x3629eed0), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e3881ab), SkBits2Float(0xc2a60000), SkBits2Float(0x3eb88227), SkBits2Float(0xc2a5ffb3), SkBits2Float(0x3f0a6183), SkBits2Float(0xc2a5ff19));
path.lineTo(SkBits2Float(0x3f0a6190), SkBits2Float(0xc2a5ff19));
path.cubicTo(SkBits2Float(0x3f0efe4f), SkBits2Float(0xc2a5ff0a), SkBits2Float(0x3f139b48), SkBits2Float(0xc2a5fef9), SkBits2Float(0x3f183842), SkBits2Float(0xc2a5fee9));
path.lineTo(SkBits2Float(0x3edc1349), SkBits2Float(0xc26ffe6c));
path.cubicTo(SkBits2Float(0x3ed567f5), SkBits2Float(0xc26ffe84), SkBits2Float(0x3ecebccf), SkBits2Float(0xc26ffe9c), SkBits2Float(0x3ec811a8), SkBits2Float(0xc26ffeb2));
path.lineTo(SkBits2Float(0x3ec8119b), SkBits2Float(0xc26ffeb2));
path.cubicTo(SkBits2Float(0x3e856151), SkBits2Float(0xc26fff91), SkBits2Float(0x3e0561b2), SkBits2Float(0xc2700000), SkBits2Float(0x3629eed0), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f183800), SkBits2Float(0xc2a5fee9));
path.cubicTo(SkBits2Float(0x3f62f7a2), SkBits2Float(0xc2a5fdd7), SkBits2Float(0x3f96db12), SkBits2Float(0xc2a5fbfa), SkBits2Float(0x3fbc3981), SkBits2Float(0xc2a5f954));
path.lineTo(SkBits2Float(0x3f8810cc), SkBits2Float(0xc26ff65b));
path.cubicTo(SkBits2Float(0x3f5a1a86), SkBits2Float(0xc26ffa2f), SkBits2Float(0x3f241256), SkBits2Float(0xc26ffcdf), SkBits2Float(0x3edc1312), SkBits2Float(0xc26ffe6c));
path.lineTo(SkBits2Float(0x3f183800), SkBits2Float(0xc2a5fee9));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp5193(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e0b17ea), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3e8b17df), SkBits2Float(0xc2a5ffd4), SkBits2Float(0x3ed0a399), SkBits2Float(0xc2a5ff7c));
path.lineTo(SkBits2Float(0x3e96d285), SkBits2Float(0xc26fff42));
path.cubicTo(SkBits2Float(0x3e491945), SkBits2Float(0xc26fffc2), SkBits2Float(0x3dc91958), SkBits2Float(0xc2700000), SkBits2Float(0x340ae940), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ed0a338), SkBits2Float(0xc2a5ff7d));
path.cubicTo(SkBits2Float(0x3ed797a0), SkBits2Float(0xc2a5ff73), SkBits2Float(0x3ede8c36), SkBits2Float(0xc2a5ff6a), SkBits2Float(0x3ee580cb), SkBits2Float(0xc2a5ff60));
path.lineTo(SkBits2Float(0x3ea5e78a), SkBits2Float(0xc26fff1b));
path.cubicTo(SkBits2Float(0x3ea0e0aa), SkBits2Float(0xc26fff29), SkBits2Float(0x3e9bd97e), SkBits2Float(0xc26fff36), SkBits2Float(0x3e96d252), SkBits2Float(0xc26fff43));
path.lineTo(SkBits2Float(0x3ed0a338), SkBits2Float(0xc2a5ff7d));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}
// op end success 1

static void battleOp5194(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e0b17a8), SkBits2Float(0xc2a60000), SkBits2Float(0x3e8b179e), SkBits2Float(0xc2a5ffd4), SkBits2Float(0x3ed0a337), SkBits2Float(0xc2a5ff7c));
path.lineTo(SkBits2Float(0x3ed0a338), SkBits2Float(0xc2a5ff7d));
path.cubicTo(SkBits2Float(0x3ed797a0), SkBits2Float(0xc2a5ff73), SkBits2Float(0x3ede8c36), SkBits2Float(0xc2a5ff6a), SkBits2Float(0x3ee580cb), SkBits2Float(0xc2a5ff60));
path.lineTo(SkBits2Float(0x3ea5e78a), SkBits2Float(0xc26fff1b));
path.cubicTo(SkBits2Float(0x3ea0e0bb), SkBits2Float(0xc26fff29), SkBits2Float(0x3e9bd9a1), SkBits2Float(0xc26fff36), SkBits2Float(0x3e96d286), SkBits2Float(0xc26fff43));
path.lineTo(SkBits2Float(0x3e96d285), SkBits2Float(0xc26fff42));
path.cubicTo(SkBits2Float(0x3e491945), SkBits2Float(0xc26fffc2), SkBits2Float(0x3dc91958), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ee58048), SkBits2Float(0xc2a5ff61));
path.cubicTo(SkBits2Float(0x3f2b1987), SkBits2Float(0xc2a5fec4), SkBits2Float(0x3f637253), SkBits2Float(0xc2a5fdb6), SkBits2Float(0x3f8de535), SkBits2Float(0xc2a5fc35));
path.lineTo(SkBits2Float(0x3f4d269a), SkBits2Float(0xc26ffa85));
path.cubicTo(SkBits2Float(0x3f246b51), SkBits2Float(0xc26ffcb3), SkBits2Float(0x3ef75f30), SkBits2Float(0xc26ffe3a), SkBits2Float(0x3ea5e737), SkBits2Float(0xc26fff1c));
path.lineTo(SkBits2Float(0x3ee58048), SkBits2Float(0xc2a5ff61));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp402(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3e0b17a8), SkBits2Float(0xc2a60000), SkBits2Float(0x3e8b179e), SkBits2Float(0xc2a5ffd4), SkBits2Float(0x3ed0a337), SkBits2Float(0xc2a5ff7c));
path.lineTo(SkBits2Float(0x3ed0a338), SkBits2Float(0xc2a5ff7d));
path.cubicTo(SkBits2Float(0x3ed797a0), SkBits2Float(0xc2a5ff73), SkBits2Float(0x3ede8c36), SkBits2Float(0xc2a5ff6a), SkBits2Float(0x3ee580cb), SkBits2Float(0xc2a5ff60));
path.lineTo(SkBits2Float(0x3ea5e78a), SkBits2Float(0xc26fff1b));
path.cubicTo(SkBits2Float(0x3ea0e0bb), SkBits2Float(0xc26fff29), SkBits2Float(0x3e9bd9a1), SkBits2Float(0xc26fff36), SkBits2Float(0x3e96d286), SkBits2Float(0xc26fff43));
path.lineTo(SkBits2Float(0x3e96d285), SkBits2Float(0xc26fff42));
path.cubicTo(SkBits2Float(0x3e491945), SkBits2Float(0xc26fffc2), SkBits2Float(0x3dc91958), SkBits2Float(0xc2700000), SkBits2Float(0x00000000), SkBits2Float(0xc2700000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ee58048), SkBits2Float(0xc2a5ff61));
path.cubicTo(SkBits2Float(0x3f2b1987), SkBits2Float(0xc2a5fec4), SkBits2Float(0x3f637253), SkBits2Float(0xc2a5fdb6), SkBits2Float(0x3f8de535), SkBits2Float(0xc2a5fc35));
path.lineTo(SkBits2Float(0x3f4d269a), SkBits2Float(0xc26ffa85));
path.cubicTo(SkBits2Float(0x3f246b51), SkBits2Float(0xc26ffcb3), SkBits2Float(0x3ef75f30), SkBits2Float(0xc26ffe3a), SkBits2Float(0x3ea5e737), SkBits2Float(0xc26fff1c));
path.lineTo(SkBits2Float(0x3ee58048), SkBits2Float(0xc2a5ff61));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp6000(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3c9b2383), SkBits2Float(0xc2a60000), SkBits2Float(0x3d1b200b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d68ae54), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d283599), SkBits2Float(0xc26ffffc));
path.cubicTo(SkBits2Float(0x3ce049ca), SkBits2Float(0xc26ffffe), SkBits2Float(0x3c604794), SkBits2Float(0xc26fffff), SkBits2Float(0xb58d9000), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x27b71bcd), SkBits2Float(0xc2a60000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3d68b08b), SkBits2Float(0xc2a5fffd));
path.cubicTo(SkBits2Float(0x3d707589), SkBits2Float(0xc2a5fffd), SkBits2Float(0x3d783329), SkBits2Float(0xc2a5fffd), SkBits2Float(0x3d7ff0c9), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d3907c2), SkBits2Float(0xc26ffffc));
path.cubicTo(SkBits2Float(0x3d336bee), SkBits2Float(0xc26ffffd), SkBits2Float(0x3d2dd36e), SkBits2Float(0xc26ffffd), SkBits2Float(0x3d283aee), SkBits2Float(0xc26ffffd));
path.lineTo(SkBits2Float(0x3d68b08b), SkBits2Float(0xc2a5fffd));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void battleOp6001(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xc2a60000));
path.cubicTo(SkBits2Float(0x3c9b2383), SkBits2Float(0xc2a60000), SkBits2Float(0x3d1b200b), SkBits2Float(0xc2a5ffff), SkBits2Float(0x3d68ae54), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d7ff0c9), SkBits2Float(0xc2a5fffd));
path.lineTo(SkBits2Float(0x3d3907c2), SkBits2Float(0xc26ffffc));
path.cubicTo(SkBits2Float(0x3d336bee), SkBits2Float(0xc26ffffd), SkBits2Float(0x3d2dd36e), SkBits2Float(0xc26ffffd), SkBits2Float(0x3d283aee), SkBits2Float(0xc26ffffd));
path.lineTo(SkBits2Float(0x3d283599), SkBits2Float(0xc26ffffc));
path.cubicTo(SkBits2Float(0x3ce049ca), SkBits2Float(0xc26ffffe), SkBits2Float(0x3c604794), SkBits2Float(0xc26fffff), SkBits2Float(0x00000000), SkBits2Float(0xc26fffff));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3d7ff566), SkBits2Float(0xc2a5fffd));
path.cubicTo(SkBits2Float(0x3dbed1a5), SkBits2Float(0xc2a5fffa), SkBits2Float(0x3dfda9cc), SkBits2Float(0xc2a5fff4), SkBits2Float(0x3e1e40f8), SkBits2Float(0xc2a5ffed));
path.lineTo(SkBits2Float(0x3de4ce81), SkBits2Float(0xc26fffe5));
path.cubicTo(SkBits2Float(0x3db75eff), SkBits2Float(0xc26ffff0), SkBits2Float(0x3d89f101), SkBits2Float(0xc26ffff8), SkBits2Float(0x3d390604), SkBits2Float(0xc26ffffc));
path.lineTo(SkBits2Float(0x3d7ff566), SkBits2Float(0xc2a5fffd));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void (*firstTest)(skiatest::Reporter* , const char* filename) = battleOp183;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = 0;

static struct TestDesc tests[] = {
    TEST(battleOp1),
    TEST(battleOp2),
    TEST(battleOp3),
    TEST(battleOp4),
    TEST(battleOp5),
    TEST(battleOp6),
    TEST(battleOp7),
    TEST(battleOp8),
    TEST(battleOp9),
    TEST(battleOp10),

    TEST(battleOp11),
    TEST(battleOp12),
    TEST(battleOp13),
    TEST(battleOp14),
    TEST(battleOp15),
    TEST(battleOp16),
    TEST(battleOp17),
    TEST(battleOp18),
    TEST(battleOp19),
    TEST(battleOp20),

    TEST(battleOp21),
    TEST(battleOp22),
    TEST(battleOp23),
    TEST(battleOp24),
    TEST(battleOp25),
    TEST(battleOp26),
    TEST(battleOp27),
    TEST(battleOp28),
    TEST(battleOp29),
    TEST(battleOp30),

    TEST(battleOp31),
    TEST(battleOp32),
    TEST(battleOp33),
    TEST(battleOp34),
    TEST(battleOp35),
    TEST(battleOp36),
    TEST(battleOp37),
    TEST(battleOp38),
    TEST(battleOp39),
    TEST(battleOp40),

    TEST(battleOp41),
    TEST(battleOp42),
    TEST(battleOp43),
    TEST(battleOp44),
    TEST(battleOp45),
    TEST(battleOp47),
    TEST(battleOp48),
    TEST(battleOp49),
    TEST(battleOp50),

    TEST(battleOp51),
    TEST(battleOp52),
    TEST(battleOp53),
    TEST(battleOp55),
    TEST(battleOp56),
    TEST(battleOp57),
    TEST(battleOp58),
    TEST(battleOp59),
    TEST(battleOp60),

    TEST(battleOp61),
    TEST(battleOp62),
    TEST(battleOp64),
    TEST(battleOp65),
    TEST(battleOp66),
    TEST(battleOp67),
    TEST(battleOp68),
    TEST(battleOp69),
    TEST(battleOp70),

    TEST(battleOp71),
    TEST(battleOp72),
    TEST(battleOp73),
    TEST(battleOp74),
    TEST(battleOp75),
    TEST(battleOp76),
    TEST(battleOp77),
    TEST(battleOp78),
    TEST(battleOp79),
    TEST(battleOp80),

    TEST(battleOp81),
    TEST(battleOp82),
    TEST(battleOp83),
    TEST(battleOp84),
    TEST(battleOp85),
    TEST(battleOp86),
    TEST(battleOp87),
    TEST(battleOp88),
    TEST(battleOp89),
    TEST(battleOp90),

    TEST(battleOp91),
    TEST(battleOp92),
    TEST(battleOp93),
    TEST(battleOp94),
    TEST(battleOp95),
    TEST(battleOp96),
    TEST(battleOp97),
    TEST(battleOp98),
    TEST(battleOp99),
    TEST(battleOp100),

    TEST(battleOp101),
    TEST(battleOp102),
    TEST(battleOp103),
    TEST(battleOp104),
    TEST(battleOp105),
    TEST(battleOp106),
    TEST(battleOp107),
    TEST(battleOp108),
    TEST(battleOp109),
    TEST(battleOp110),

    TEST(battleOp111),
    TEST(battleOp112),
    TEST(battleOp113),
    TEST(battleOp114),
    TEST(battleOp115),
    TEST(battleOp116),
    TEST(battleOp117),
    TEST(battleOp118),
    TEST(battleOp119),
    TEST(battleOp120),

    TEST(battleOp121),
    TEST(battleOp122),
    TEST(battleOp123),
    TEST(battleOp124),
    TEST(battleOp125),
    TEST(battleOp126),
    TEST(battleOp127),
    TEST(battleOp128),
    TEST(battleOp129),
    TEST(battleOp130),

    TEST(battleOp131),
    TEST(battleOp132),
    TEST(battleOp133),
    TEST(battleOp134),
    TEST(battleOp135),
    TEST(battleOp136),
    TEST(battleOp137),
    TEST(battleOp138),
    TEST(battleOp139),
    TEST(battleOp140),

    TEST(battleOp141),
    TEST(battleOp142),
    TEST(battleOp143),
    TEST(battleOp144),
    TEST(battleOp145),
    TEST(battleOp146),
    TEST(battleOp147),
    TEST(battleOp149),
    TEST(battleOp150),

    TEST(battleOp151),
    TEST(battleOp153),
    TEST(battleOp154),
    TEST(battleOp155),
    TEST(battleOp156),
    TEST(battleOp158),
    TEST(battleOp159),
    TEST(battleOp160),

    TEST(battleOp161),
    TEST(battleOp162),
    TEST(battleOp164),
    TEST(battleOp165),
    TEST(battleOp166),
    TEST(battleOp167),
    TEST(battleOp168),
    TEST(battleOp169),
    TEST(battleOp170),

    TEST(battleOp171),
    TEST(battleOp172),
    TEST(battleOp173),
    TEST(battleOp174),
    TEST(battleOp175),
    TEST(battleOp176),
    TEST(battleOp177),
    TEST(battleOp178),
    TEST(battleOp179),
    TEST(battleOp180),

    TEST(battleOp182),
    TEST(battleOp184),
    TEST(battleOp185),
    TEST(battleOp186),
    TEST(battleOp187),
    TEST(battleOp188),
    TEST(battleOp189),
    TEST(battleOp190),

    TEST(battleOp191),
    TEST(battleOp192),
    TEST(battleOp193),
    TEST(battleOp194),
    TEST(battleOp196),
    TEST(battleOp197),
    TEST(battleOp199),
    TEST(battleOp200),

    TEST(battleOp201),
    TEST(battleOp202),
    TEST(battleOp203),
    TEST(battleOp204),
    TEST(battleOp205),
    TEST(battleOp206),
    TEST(battleOp207),
    TEST(battleOp208),
    TEST(battleOp209),
    TEST(battleOp210),

    TEST(battleOp211),
    TEST(battleOp212),
    TEST(battleOp213),
    TEST(battleOp214),
    TEST(battleOp215),
    TEST(battleOp216),
    TEST(battleOp217),
    TEST(battleOp218),
    TEST(battleOp219),
    TEST(battleOp220),

    TEST(battleOp221),
    TEST(battleOp222),
    TEST(battleOp223),
    TEST(battleOp224),
    TEST(battleOp225),
    TEST(battleOp226),
    TEST(battleOp227),
    TEST(battleOp228),
    TEST(battleOp229),

    TEST(battleOp231),
    TEST(battleOp232),
    TEST(battleOp233),
    TEST(battleOp234),
    TEST(battleOp235),
    TEST(battleOp236),
    TEST(battleOp237),
    TEST(battleOp238),
    TEST(battleOp239),
    TEST(battleOp240),

    TEST(battleOp241),
    TEST(battleOp242),
    TEST(battleOp243),
    TEST(battleOp244),
    TEST(battleOp245),
    TEST(battleOp246),
    TEST(battleOp247),
    TEST(battleOp248),
    TEST(battleOp249),
    TEST(battleOp250),

    TEST(battleOp251),
    TEST(battleOp252),
    TEST(battleOp253),
    TEST(battleOp254),
    TEST(battleOp255),
    TEST(battleOp257),
    TEST(battleOp258),
    TEST(battleOp259),
    TEST(battleOp260),

    TEST(battleOp261),
    TEST(battleOp262),
    TEST(battleOp263),
    TEST(battleOp264),
    TEST(battleOp265),
    TEST(battleOp266),
    TEST(battleOp267),
    TEST(battleOp268),
    TEST(battleOp270),

    TEST(battleOp271),
    TEST(battleOp272),
    TEST(battleOp274),
    TEST(battleOp275),
    TEST(battleOp276),
    TEST(battleOp277),
    TEST(battleOp278),
    TEST(battleOp279),
    TEST(battleOp280),

    TEST(battleOp281),
    TEST(battleOp282),
    TEST(battleOp284),
    TEST(battleOp285),
    TEST(battleOp286),
    TEST(battleOp287),
    TEST(battleOp288),
    TEST(battleOp289),
    TEST(battleOp290),

    TEST(battleOp291),
    TEST(battleOp292),
    TEST(battleOp293),
    TEST(battleOp294),
    TEST(battleOp295),
    TEST(battleOp296),
    TEST(battleOp297),
    TEST(battleOp298),
    TEST(battleOp299),
    TEST(battleOp300),

    TEST(battleOp301),
    TEST(battleOp302),
    TEST(battleOp303),
    TEST(battleOp304),
    TEST(battleOp305),
    TEST(battleOp306),
    TEST(battleOp307),
    TEST(battleOp308),
    TEST(battleOp309),
    TEST(battleOp310),

    TEST(battleOp311),
    TEST(battleOp312),
    TEST(battleOp313),
    TEST(battleOp314),
    TEST(battleOp315),
    TEST(battleOp316),
    TEST(battleOp317),
    TEST(battleOp318),
    TEST(battleOp319),
    TEST(battleOp320),

    TEST(battleOp321),
    TEST(battleOp322),
    TEST(battleOp323),
    TEST(battleOp324),
    TEST(battleOp325),
    TEST(battleOp326),
    TEST(battleOp327),
    TEST(battleOp328),
    TEST(battleOp329),
    TEST(battleOp330),

    TEST(battleOp331),
    TEST(battleOp332),
    TEST(battleOp333),
    TEST(battleOp334),
    TEST(battleOp335),
    TEST(battleOp336),
    TEST(battleOp337),
    TEST(battleOp338),
    TEST(battleOp339),
    TEST(battleOp340),

    TEST(battleOp341),
    TEST(battleOp342),
    TEST(battleOp343),
    TEST(battleOp344),
    TEST(battleOp345),
    TEST(battleOp346),
    TEST(battleOp347),
    TEST(battleOp348),
    TEST(battleOp349),
    TEST(battleOp350),

    TEST(battleOp351),
    TEST(battleOp352),

    TEST(battleOp402),

    TEST(battleOp1390),
    TEST(battleOp1391),
    TEST(battleOp1392),
    TEST(battleOp1393),
    TEST(battleOp1394),
    TEST(battleOp1395),
    TEST(battleOp1396),

    TEST(battleOp2193),
    TEST(battleOp2194),

    TEST(battleOp3368),
    TEST(battleOp3369),
    TEST(battleOp3370),
    TEST(battleOp3371),
    TEST(battleOp3372),

    TEST(battleOp4290),
    TEST(battleOp4291),
    TEST(battleOp4292),
    TEST(battleOp4293),
    TEST(battleOp4294),
    TEST(battleOp4295),
    TEST(battleOp4296),

    TEST(battleOp5193),
    TEST(battleOp5194),

    TEST(battleOp6000),
    TEST(battleOp6001),

    TEST(issue414409c),
    TEST(issue414409b),
    TEST(issue414409),

    // these draw wrong
    TEST(battleOp46),  // dropped an outer cubic incorrectly
                       // if assembly rewrite was done, the error would be hidden
    TEST(battleOp54),
    TEST(battleOp63),
    TEST(battleOp152),
    TEST(battleOp157),
    TEST(battleOp163),
    TEST(battleOp181),
    TEST(battleOp183),
    TEST(battleOp195),
    TEST(battleOp198),
    TEST(battleOp230),
    TEST(battleOp256),
    TEST(battleOp269),
    TEST(battleOp273),
    TEST(battleOp148),
    TEST(battleOp283),
};


static const size_t testCount = SK_ARRAY_COUNT(tests);

static bool runReverse = false;

DEF_TEST(PathOpsBattle, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    RunTestSet(reporter, tests, testCount, firstTest, NULL, stopTest, runReverse);
}
