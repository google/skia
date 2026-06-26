/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkFloatBits.h"
#include "src/core/SkRandom.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/Test.h"
#include "tools/flags/CommandLineFlags.h"

struct OvalSet {
    SkRect fBounds;
    int fColumns;
    int fRows;
    int fRotations;
    SkScalar fXSpacing;
    SkScalar fYSpacing;
};

static void testOvalSet(const OvalSet& set, const SkPath& oval, SkOpBuilder* builder,
        SkRegion* region, SkPath* result) {
    for (int x = 0; x < set.fColumns; ++x) {
        for (int y = 0; y < set.fRows; ++y) {
            for (SkScalar r = 0; r < 360; r += 360.f / set.fRotations) {
                SkMatrix matrix;
                matrix.reset();
                matrix.postRotate(r, 0, 0);
                matrix.postTranslate(x * set.fXSpacing, y * set.fYSpacing);
                SkPath rotated = oval.makeTransform(matrix);
                if (builder) {
                    builder->add(rotated, kUnion_SkPathOp);
                } else if (!region) {
                    if (auto res = Op(*result, rotated, kUnion_SkPathOp)) {
                        *result = *res;
                    }
                } else {
                    SkRegion rgnB, openClip;
                    openClip.setRect({-16000, -16000, 16000, 16000});
                    rgnB.setPath(rotated, openClip);
                    region->op(rgnB, SkRegion::kUnion_Op);
                }
            }
        }
    }
    if (builder) {
        if (auto res = builder->resolve()) {
            *result = *res;
        }
    } else if (region) {
        *result = region->getBoundaryPath();
    }
}

static void testOne(skiatest::Reporter* reporter, const OvalSet& set) {
    SkPath regionResult, builderResult, opResult;
    SkPath oval = SkPath::Oval(set.fBounds);
    SkOpBuilder builder;
    SkRegion region;
    testOvalSet(set, oval, nullptr, &region, &regionResult);
    testOvalSet(set, oval, &builder, nullptr, &builderResult);
    testOvalSet(set, oval, nullptr, nullptr, &opResult);
    SkBitmap bitmap;
    comparePaths(reporter, __FUNCTION__  , regionResult, builderResult, bitmap);
    comparePaths(reporter, __FUNCTION__  , regionResult, opResult, bitmap);
}

struct OvalSetOneOff {
    int fCol;
    int fRow;
    int fRot;
    int fTrial;
} oneOffs[] = {
    { 2, 2, 9, 73 },
    { 1, 2, 7, 93 }
};

static void setupOne(skiatest::Reporter* reporter, int col, int row, int rot, int trial) {
    const int scale = 10;
    SkRandom r;
    r.setSeed(col * 100000000 + row * 10000000 + rot * 1000000 + trial);
    SkScalar xOffset = r.nextRangeScalar(1, 40) * scale;
    SkScalar yOffset = r.nextRangeScalar(1, 100) * scale;
    OvalSet set = {{0, 0, 0, 0}, col, row, rot, xOffset, yOffset};
    set.fBounds.setXYWH(5, 5,
        r.nextRangeScalar(5, 50) * scale, r.nextRangeScalar(50, 90) * scale);
    testOne(reporter, set);
}

static DEFINE_int(processOffset, 0,
                    "Offset the test by this value. This permits multiple processes "
                    "to exercise the same test in parallel with different test values.");
static DEFINE_int(processCount, 1,
                    "Test iteration count. This permits multiple processes "
                    "to exercise the same test in parallel with different test values.");
static DEFINE_int(trialRuns, 100, "Run this many tests (defaults to 100).");

DEF_TEST(SixtyOvals, reporter) {
    bool skipOneOffs = false;
    int trialRuns = FLAGS_trialRuns / FLAGS_processCount;
    for (int col = 1; col <= 2; ++col) {
        for (int row = 1; row <= 3; ++row) {
            for (int rot = 2; rot <= 9; ++rot) {
                for (int trial = FLAGS_processOffset * trialRuns; --trialRuns >= 0; ++trial) {
                    if (skipOneOffs) {
                        for (const OvalSetOneOff& oneOff : oneOffs) {
                            if (col == oneOff.fCol && row == oneOff.fRow && rot == oneOff.fRot
                                    && trial == oneOff.fTrial) {
                                goto skipTest;
                            }
                        }
                    }
                    setupOne(reporter, col, row, rot, trial);
            skipTest:
                    ;
                }
            }
        }
    }
}

DEF_TEST(SixtyOvalsOneOff, reporter) {
    for (const OvalSetOneOff& oneOff : oneOffs) {
        setupOne(reporter, oneOff.fCol, oneOff.fRow, oneOff.fRot, oneOff.fTrial);
    }
}

DEF_TEST(SixtyOvals_2_2_9_73, reporter) {
    SkPath path = SkPathBuilder()
    .moveTo(SkBits2Float(0x434d53ca), SkBits2Float(0x43ad6ab0))  // 205.327f, 346.833f
    .conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x40a00000), SkBits2Float(0x42d253ca), SkBits2Float(0x40a00000), SkBits2Float(0x3f3504f3))   // 205.327f, 5, 105.164f, 5, 0.707107f
    .conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x40a00000), SkBits2Float(0x40a00000), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3))   // 5, 5, 5, 346.833f, 0.707107f
    .conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x442c2ab0), SkBits2Float(0x42d253ca), SkBits2Float(0x442c2ab0), SkBits2Float(0x3f3504f3))   // 5, 688.667f, 105.164f, 688.667f, 0.707107f
    .conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x442c2ab0), SkBits2Float(0x434d53ca), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3))   // 205.327f, 688.667f, 205.327f, 346.833f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc2834d04), SkBits2Float(0x43c6d5fb))   // -65.6504f, 397.672f
    .conicTo(SkBits2Float(0x431a136e), SkBits2Float(0x4307cfe3), SkBits2Float(0x429ab133), SkBits2Float(0x428edb31), SkBits2Float(0x3f3504f3))   // 154.076f, 135.812f, 77.3461f, 71.4281f, 0.707107f
    .conicTo(SkBits2Float(0x3f1dc4d0), SkBits2Float(0x40e169c2), SkBits2Float(0xc35b1c2c), SkBits2Float(0x438673b0), SkBits2Float(0x3f3504f3))   // 0.616284f, 7.04416f, -219.11f, 268.904f, 0.707107f
    .conicTo(SkBits2Float(0xc3db6b0e), SkBits2Float(0x4404b0dc), SkBits2Float(0xc3b50da4), SkBits2Float(0x4414c96f), SkBits2Float(0x3f3504f3))   // -438.836f, 530.763f, -362.107f, 595.147f, 0.707107f
    .conicTo(SkBits2Float(0xc38eb03a), SkBits2Float(0x4424e202), SkBits2Float(0xc2834d04), SkBits2Float(0x43c6d5fb), SkBits2Float(0x3f3504f3))   // -285.377f, 659.531f, -65.6504f, 397.672f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc398f46d), SkBits2Float(0x438337ac))   // -305.91f, 262.435f
    .conicTo(SkBits2Float(0x41f5d870), SkBits2Float(0x434b137f), SkBits2Float(0x41556629), SkBits2Float(0x42d0de52), SkBits2Float(0x3f3504f3))   // 30.7307f, 203.076f, 13.3374f, 104.434f, 0.707107f
    .conicTo(SkBits2Float(0xc081c918), SkBits2Float(0x40b95a5c), SkBits2Float(0xc3aa5918), SkBits2Float(0x42824d58), SkBits2Float(0x3f3504f3))   // -4.0558f, 5.79228f, -340.696f, 65.1511f, 0.707107f
    .conicTo(SkBits2Float(0xc4295587), SkBits2Float(0x42f9050a), SkBits2Float(0xc424fc5c), SkBits2Float(0x435f26db), SkBits2Float(0x3f3504f3))   // -677.336f, 124.51f, -659.943f, 223.152f, 0.707107f
    .conicTo(SkBits2Float(0xc420a331), SkBits2Float(0x43a0e598), SkBits2Float(0xc398f46d), SkBits2Float(0x438337ac), SkBits2Float(0x3f3504f3))   // -642.55f, 321.794f, -305.91f, 262.435f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc3c983e0), SkBits2Float(0x408cdc40))   // -403.03f, 4.40189f
    .conicTo(SkBits2Float(0xc2d5fcd2), SkBits2Float(0x432f5193), SkBits2Float(0xc263a5d9), SkBits2Float(0x42b12617), SkBits2Float(0x3f3504f3))   // -106.994f, 175.319f, -56.912f, 88.5744f, 0.707107f
    .conicTo(SkBits2Float(0xc0da9066), SkBits2Float(0x3fea4196), SkBits2Float(0xc3976eed), SkBits2Float(0xc329162e), SkBits2Float(0x3f3504f3))   // -6.83013f, 1.83013f, -302.867f, -169.087f, 0.707107f
    .conicTo(SkBits2Float(0xc415b9cc), SkBits2Float(0xc3aa006f), SkBits2Float(0xc4223f09), SkBits2Float(0xc37d4256), SkBits2Float(0x3f3504f3))   // -598.903f, -340.003f, -648.985f, -253.259f, 0.707107f
    .conicTo(SkBits2Float(0xc42ec446), SkBits2Float(0xc32683cf), SkBits2Float(0xc3c983e0), SkBits2Float(0x408cdc40), SkBits2Float(0x3f3504f3))   // -699.067f, -166.515f, -403.03f, 4.40189f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc39bc8c8), SkBits2Float(0xc37fb0d7))   // -311.569f, -255.691f
    .conicTo(SkBits2Float(0xc342a797), SkBits2Float(0x42830e25), SkBits2Float(0xc2c9102e), SkBits2Float(0x41fa2834), SkBits2Float(0x3f3504f3))   // -194.655f, 65.5276f, -100.532f, 31.2696f, 0.707107f
    .conicTo(SkBits2Float(0xc0cd12f5), SkBits2Float(0xc03f4152), SkBits2Float(0xc2f6a523), SkBits2Float(0xc3a21a77), SkBits2Float(0x3f3504f3))   // -6.40856f, -2.98836f, -123.323f, -324.207f, 0.707107f
    .conicTo(SkBits2Float(0xc3703c8a), SkBits2Float(0xc4215b37), SkBits2Float(0xc3a72e05), SkBits2Float(0xc418cab4), SkBits2Float(0x3f3504f3))   // -240.236f, -645.425f, -334.36f, -611.167f, 0.707107f
    .conicTo(SkBits2Float(0xc3d63dc5), SkBits2Float(0xc4103a31), SkBits2Float(0xc39bc8c8), SkBits2Float(0xc37fb0d7), SkBits2Float(0x3f3504f3))   // -428.483f, -576.909f, -311.569f, -255.691f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc294a419), SkBits2Float(0xc3c6124c))   // -74.3205f, -396.143f
    .conicTo(SkBits2Float(0xc33f3c05), SkBits2Float(0xc295d95d), SkBits2Float(0xc2c2390a), SkBits2Float(0xc222aa8c), SkBits2Float(0x3f3504f3))   // -191.234f, -74.9245f, -97.1114f, -40.6665f, 0.707107f
    .conicTo(SkBits2Float(0xc03f4154), SkBits2Float(0xc0cd12f4), SkBits2Float(0x42e3d9e6), SkBits2Float(0xc3a3d041), SkBits2Float(0x3f3504f3))   // -2.98836f, -6.40856f, 113.926f, -327.627f, 0.707107f
    .conicTo(SkBits2Float(0x4366d6ec), SkBits2Float(0xc422361b), SkBits2Float(0x4308b76c), SkBits2Float(0xc42ac69e), SkBits2Float(0x3f3504f3))   // 230.84f, -648.845f, 136.716f, -683.103f, 0.707107f
    .conicTo(SkBits2Float(0x422a5fb0), SkBits2Float(0xc4335721), SkBits2Float(0xc294a419), SkBits2Float(0xc3c6124c), SkBits2Float(0x3f3504f3))   // 42.5934f, -717.361f, -74.3205f, -396.143f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x4345b3f8), SkBits2Float(0xc3af9e21))   // 197.703f, -351.235f
    .conicTo(SkBits2Float(0xc2c4aac2), SkBits2Float(0xc3345194), SkBits2Float(0xc24101bb), SkBits2Float(0xc2bb2617), SkBits2Float(0x3f3504f3))   // -98.3335f, -180.319f, -48.2517f, -93.5744f, 0.707107f
    .conicTo(SkBits2Float(0x3fea41a0), SkBits2Float(0xc0da9066), SkBits2Float(0x4394eeee), SkBits2Float(0xc331bf31), SkBits2Float(0x3f3504f3))   // 1.83013f, -6.83013f, 297.867f, -177.747f, 0.707107f
    .conicTo(SkBits2Float(0x441479cd), SkBits2Float(0xc3ae54f0), SkBits2Float(0x4407f490), SkBits2Float(0xc3d9b434), SkBits2Float(0x3f3504f3))   // 593.903f, -348.664f, 543.821f, -435.408f, 0.707107f
    .conicTo(SkBits2Float(0x43f6dea8), SkBits2Float(0xc40289bc), SkBits2Float(0x4345b3f8), SkBits2Float(0xc3af9e21), SkBits2Float(0x3f3504f3))   // 493.74f, -522.152f, 197.703f, -351.235f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43bc9c08), SkBits2Float(0xc30dfb1e))   // 377.219f, -141.981f
    .conicTo(SkBits2Float(0x422250a2), SkBits2Float(0xc34956f5), SkBits2Float(0x41b97bee), SkBits2Float(0xc2cd653e), SkBits2Float(0x3f3504f3))   // 40.5787f, -201.34f, 23.1855f, -102.698f, 0.707107f
    .conicTo(SkBits2Float(0x40b95a5b), SkBits2Float(0xc081c919), SkBits2Float(0x43ab375e), SkBits2Float(0x425d363a), SkBits2Float(0x3f3504f3))   // 5.79228f, -4.0558f, 342.433f, 55.303f, 0.707107f
    .conicTo(SkBits2Float(0x4429c4a9), SkBits2Float(0x42e552cb), SkBits2Float(0x442e1dd4), SkBits2Float(0x4180287c), SkBits2Float(0x3f3504f3))   // 679.073f, 114.662f, 696.466f, 16.0198f, 0.707107f
    .conicTo(SkBits2Float(0x443276ff), SkBits2Float(0xc2a53e8d), SkBits2Float(0x43bc9c08), SkBits2Float(0xc30dfb1e), SkBits2Float(0x3f3504f3))   // 713.859f, -82.6222f, 377.219f, -141.981f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43be1d75), SkBits2Float(0x4305b53c))   // 380.23f, 133.708f
    .conicTo(SkBits2Float(0x432080f6), SkBits2Float(0xc30026d3), SkBits2Float(0x42a78c44), SkBits2Float(0xc27f121c), SkBits2Float(0x3f3504f3))   // 160.504f, -128.152f, 83.774f, -63.7677f, 0.707107f
    .conicTo(SkBits2Float(0x40e169c3), SkBits2Float(0x3f1dc4b8), SkBits2Float(0x4362c542), SkBits2Float(0x43833cea), SkBits2Float(0x3f3504f3))   // 7.04416f, 0.616283f, 226.771f, 262.476f, 0.707107f
    .conicTo(SkBits2Float(0x43df3f9c), SkBits2Float(0x44031579), SkBits2Float(0x4402ce83), SkBits2Float(0x43e5f9cc), SkBits2Float(0x3f3504f3))   // 446.497f, 524.336f, 523.227f, 459.952f, 0.707107f
    .conicTo(SkBits2Float(0x4415fd38), SkBits2Float(0x43c5c8a6), SkBits2Float(0x43be1d75), SkBits2Float(0x4305b53c), SkBits2Float(0x3f3504f3))   // 599.957f, 395.568f, 380.23f, 133.708f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x434d53ca), SkBits2Float(0x44487cfb))   // 205.327f, 801.953f
    .conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x43e60f46), SkBits2Float(0x42d253ca), SkBits2Float(0x43e60f46), SkBits2Float(0x3f3504f3))   // 205.327f, 460.119f, 105.164f, 460.119f, 0.707107f
    .conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x43e60f46), SkBits2Float(0x40a00000), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3))   // 5, 460.119f, 5, 801.953f, 0.707107f
    .conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x448ef92a), SkBits2Float(0x42d253ca), SkBits2Float(0x448ef92a), SkBits2Float(0x3f3504f3))   // 5, 1143.79f, 105.164f, 1143.79f, 0.707107f
    .conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x448ef92a), SkBits2Float(0x434d53ca), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3))   // 205.327f, 1143.79f, 205.327f, 801.953f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc2834d04), SkBits2Float(0x445532a0))   // -65.6504f, 852.791f
    .conicTo(SkBits2Float(0x431a136e), SkBits2Float(0x4413bb9c), SkBits2Float(0x429ab133), SkBits2Float(0x4403a309), SkBits2Float(0x3f3504f3))   // 154.076f, 590.931f, 77.3461f, 526.547f, 0.707107f
    .conicTo(SkBits2Float(0x3f1dc4d0), SkBits2Float(0x43e714ed), SkBits2Float(0xc35b1c2c), SkBits2Float(0x4435017b), SkBits2Float(0x3f3504f3))   // 0.616284f, 462.163f, -219.11f, 724.023f, 0.707107f
    .conicTo(SkBits2Float(0xc3db6b0e), SkBits2Float(0x4476787f), SkBits2Float(0xc3b50da4), SkBits2Float(0x44834889), SkBits2Float(0x3f3504f3))   // -438.836f, 985.883f, -362.107f, 1050.27f, 0.707107f
    .conicTo(SkBits2Float(0xc38eb03a), SkBits2Float(0x448b54d2), SkBits2Float(0xc2834d04), SkBits2Float(0x445532a0), SkBits2Float(0x3f3504f3))   // -285.377f, 1114.65f, -65.6504f, 852.791f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc398f46d), SkBits2Float(0x44336379))   // -305.91f, 717.554f
    .conicTo(SkBits2Float(0x41f5d870), SkBits2Float(0x44248c83), SkBits2Float(0x41556629), SkBits2Float(0x440be36d), SkBits2Float(0x3f3504f3))   // 30.7307f, 658.195f, 13.3374f, 559.554f, 0.707107f
    .conicTo(SkBits2Float(0xc081c918), SkBits2Float(0x43e674af), SkBits2Float(0xc3aa5918), SkBits2Float(0x4402114e), SkBits2Float(0x3f3504f3))   // -4.0558f, 460.912f, -340.696f, 520.27f, 0.707107f
    .conicTo(SkBits2Float(0xc4295587), SkBits2Float(0x4410e844), SkBits2Float(0xc424fc5c), SkBits2Float(0x4429915a), SkBits2Float(0x3f3504f3))   // -677.336f, 579.629f, -659.943f, 678.271f, 0.707107f
    .conicTo(SkBits2Float(0xc420a331), SkBits2Float(0x44423a6f), SkBits2Float(0xc398f46d), SkBits2Float(0x44336379), SkBits2Float(0x3f3504f3))   // -642.55f, 776.913f, -305.91f, 717.554f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc3c983e0), SkBits2Float(0x43e5c2b7))   // -403.03f, 459.521f
    .conicTo(SkBits2Float(0xc2d5fcd2), SkBits2Float(0x441d9c08), SkBits2Float(0xc263a5d9), SkBits2Float(0x4407ec66), SkBits2Float(0x3f3504f3))   // -106.994f, 630.438f, -56.912f, 543.694f, 0.707107f
    .conicTo(SkBits2Float(0xc0da9066), SkBits2Float(0x43e47988), SkBits2Float(0xc3976eed), SkBits2Float(0x438f042f), SkBits2Float(0x3f3504f3))   // -6.83013f, 456.949f, -302.867f, 286.033f, 0.707107f
    .conicTo(SkBits2Float(0xc415b9cc), SkBits2Float(0x42e63b5c), SkBits2Float(0xc4223f09), SkBits2Float(0x4349dc36), SkBits2Float(0x3f3504f3))   // -598.903f, 115.116f, -648.985f, 201.86f, 0.707107f
    .conicTo(SkBits2Float(0xc42ec446), SkBits2Float(0x43904d5e), SkBits2Float(0xc3c983e0), SkBits2Float(0x43e5c2b7), SkBits2Float(0x3f3504f3))   // -699.067f, 288.604f, -403.03f, 459.521f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc39bc8c8), SkBits2Float(0x43476db5))   // -311.569f, 199.429f
    .conicTo(SkBits2Float(0xc342a797), SkBits2Float(0x44022968), SkBits2Float(0xc2c9102e), SkBits2Float(0x43f331c9), SkBits2Float(0x3f3504f3))   // -194.655f, 520.647f, -100.532f, 486.389f, 0.707107f
    .conicTo(SkBits2Float(0xc0cd12f5), SkBits2Float(0x43e210c3), SkBits2Float(0xc2f6a523), SkBits2Float(0x4302e99e), SkBits2Float(0x3f3504f3))   // -6.40856f, 452.131f, -123.323f, 130.913f, 0.707107f
    .conicTo(SkBits2Float(0xc3703c8a), SkBits2Float(0xc33e4e50), SkBits2Float(0xc3a72e05), SkBits2Float(0xc31c0c44), SkBits2Float(0x3f3504f3))   // -240.236f, -190.306f, -334.36f, -156.048f, 0.707107f
    .conicTo(SkBits2Float(0xc3d63dc5), SkBits2Float(0xc2f39470), SkBits2Float(0xc39bc8c8), SkBits2Float(0x43476db5), SkBits2Float(0x3f3504f3))   // -428.483f, -121.79f, -311.569f, 199.429f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc294a419), SkBits2Float(0x426be7d0))   // -74.3205f, 58.9764f
    .conicTo(SkBits2Float(0xc33f3c05), SkBits2Float(0x43be18ef), SkBits2Float(0xc2c2390a), SkBits2Float(0x43cf39f4), SkBits2Float(0x3f3504f3))   // -191.234f, 380.195f, -97.1114f, 414.453f, 0.707107f
    .conicTo(SkBits2Float(0xc03f4154), SkBits2Float(0x43e05afa), SkBits2Float(0x42e3d9e6), SkBits2Float(0x42fefc14), SkBits2Float(0x3f3504f3))   // -2.98836f, 448.711f, 113.926f, 127.492f, 0.707107f
    .conicTo(SkBits2Float(0x4366d6ec), SkBits2Float(0xc341b9e0), SkBits2Float(0x4308b76c), SkBits2Float(0xc363fbec), SkBits2Float(0x3f3504f3))   // 230.84f, -193.726f, 136.716f, -227.984f, 0.707107f
    .conicTo(SkBits2Float(0x422a5fb0), SkBits2Float(0xc3831efc), SkBits2Float(0xc294a419), SkBits2Float(0x426be7d0), SkBits2Float(0x3f3504f3))   // 42.5934f, -262.242f, -74.3205f, 58.9764f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x4345b3f8), SkBits2Float(0x42cfc494))   // 197.703f, 103.884f
    .conicTo(SkBits2Float(0xc2c4aac2), SkBits2Float(0x4389667c), SkBits2Float(0xc24101bb), SkBits2Float(0x43b4c5c0), SkBits2Float(0x3f3504f3))   // -98.3335f, 274.801f, -48.2517f, 361.545f, 0.707107f
    .conicTo(SkBits2Float(0x3fea41a0), SkBits2Float(0x43e02504), SkBits2Float(0x4394eeee), SkBits2Float(0x438aafae), SkBits2Float(0x3f3504f3))   // 1.83013f, 448.289f, 297.867f, 277.372f, 0.707107f
    .conicTo(SkBits2Float(0x441479cd), SkBits2Float(0x42d4e958), SkBits2Float(0x4407f490), SkBits2Float(0x419db120), SkBits2Float(0x3f3504f3))   // 593.903f, 106.456f, 543.821f, 19.7115f, 0.707107f
    .conicTo(SkBits2Float(0x43f6dea8), SkBits2Float(0xc28610c8), SkBits2Float(0x4345b3f8), SkBits2Float(0x42cfc494), SkBits2Float(0x3f3504f3))   // 493.74f, -67.0328f, 197.703f, 103.884f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43bc9c08), SkBits2Float(0x439c91b7))   // 377.219f, 313.138f
    .conicTo(SkBits2Float(0x422250a2), SkBits2Float(0x437dc797), SkBits2Float(0x41b97bee), SkBits2Float(0x43b035f6), SkBits2Float(0x3f3504f3))   // 40.5787f, 253.78f, 23.1855f, 352.422f, 0.707107f
    .conicTo(SkBits2Float(0x40b95a5b), SkBits2Float(0x43e18822), SkBits2Float(0x43ab375e), SkBits2Float(0x43ff360d), SkBits2Float(0x3f3504f3))   // 5.79228f, 451.064f, 342.433f, 510.422f, 0.707107f
    .conicTo(SkBits2Float(0x4429c4a9), SkBits2Float(0x440e71fc), SkBits2Float(0x442e1dd4), SkBits2Float(0x43eb91ce), SkBits2Float(0x3f3504f3))   // 679.073f, 569.781f, 696.466f, 471.139f, 0.707107f
    .conicTo(SkBits2Float(0x443276ff), SkBits2Float(0x43ba3fa3), SkBits2Float(0x43bc9c08), SkBits2Float(0x439c91b7), SkBits2Float(0x3f3504f3))   // 713.859f, 372.497f, 377.219f, 313.138f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43be1d75), SkBits2Float(0x441334f2))   // 380.23f, 588.827f
    .conicTo(SkBits2Float(0x432080f6), SkBits2Float(0x43a37bdc), SkBits2Float(0x42a78c44), SkBits2Float(0x43c3ad02), SkBits2Float(0x3f3504f3))   // 160.504f, 326.968f, 83.774f, 391.352f, 0.707107f
    .conicTo(SkBits2Float(0x40e169c3), SkBits2Float(0x43e3de28), SkBits2Float(0x4362c542), SkBits2Float(0x44336618), SkBits2Float(0x3f3504f3))   // 7.04416f, 455.736f, 226.771f, 717.595f, 0.707107f
    .conicTo(SkBits2Float(0x43df3f9c), SkBits2Float(0x4474dd1c), SkBits2Float(0x4402ce83), SkBits2Float(0x4464c489), SkBits2Float(0x3f3504f3))   // 446.497f, 979.455f, 523.227f, 915.071f, 0.707107f
    .conicTo(SkBits2Float(0x4415fd38), SkBits2Float(0x4454abf6), SkBits2Float(0x43be1d75), SkBits2Float(0x441334f2), SkBits2Float(0x3f3504f3))   // 599.957f, 850.687f, 380.23f, 588.827f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43bb9978), SkBits2Float(0x43ad6ab0))   // 375.199f, 346.833f
    .conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x40a00000), SkBits2Float(0x43898486), SkBits2Float(0x40a00000), SkBits2Float(0x3f3504f3))   // 375.199f, 5, 275.035f, 5, 0.707107f
    .conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x40a00000), SkBits2Float(0x432edf26), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3))   // 174.872f, 5, 174.872f, 346.833f, 0.707107f
    .conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x442c2ab0), SkBits2Float(0x43898486), SkBits2Float(0x442c2ab0), SkBits2Float(0x3f3504f3))   // 174.872f, 688.667f, 275.035f, 688.667f, 0.707107f
    .conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x442c2ab0), SkBits2Float(0x43bb9978), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3))   // 375.199f, 688.667f, 375.199f, 346.833f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x42d07148), SkBits2Float(0x43c6d5fb))   // 104.221f, 397.672f
    .conicTo(SkBits2Float(0x43a1f94a), SkBits2Float(0x4307cfe3), SkBits2Float(0x437737c0), SkBits2Float(0x428edb31), SkBits2Float(0x3f3504f3))   // 323.948f, 135.812f, 247.218f, 71.4281f, 0.707107f
    .conicTo(SkBits2Float(0x432a7ceb), SkBits2Float(0x40e169c2), SkBits2Float(0xc244f418), SkBits2Float(0x438673b0), SkBits2Float(0x3f3504f3))   // 170.488f, 7.04416f, -49.2384f, 268.904f, 0.707107f
    .conicTo(SkBits2Float(0xc3867b7b), SkBits2Float(0x4404b0dc), SkBits2Float(0xc3403c22), SkBits2Float(0x4414c96f), SkBits2Float(0x3f3504f3))   // -268.965f, 530.763f, -192.235f, 595.147f, 0.707107f
    .conicTo(SkBits2Float(0xc2e7029c), SkBits2Float(0x4424e202), SkBits2Float(0x42d07148), SkBits2Float(0x43c6d5fb), SkBits2Float(0x3f3504f3))   // -115.505f, 659.531f, 104.221f, 397.672f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc30809b4), SkBits2Float(0x438337ac))   // -136.038f, 262.435f
    .conicTo(SkBits2Float(0x43489a34), SkBits2Float(0x434b137f), SkBits2Float(0x43373589), SkBits2Float(0x42d0de52), SkBits2Float(0x3f3504f3))   // 200.602f, 203.076f, 183.209f, 104.434f, 0.707107f
    .conicTo(SkBits2Float(0x4325d0dd), SkBits2Float(0x40b95a5c), SkBits2Float(0xc32ad30a), SkBits2Float(0x42824d58), SkBits2Float(0x3f3504f3))   // 165.816f, 5.79228f, -170.824f, 65.1511f, 0.707107f
    .conicTo(SkBits2Float(0xc3fdbb7b), SkBits2Float(0x42f9050a), SkBits2Float(0xc3f50925), SkBits2Float(0x435f26db), SkBits2Float(0x3f3504f3))   // -507.465f, 124.51f, -490.071f, 223.152f, 0.707107f
    .conicTo(SkBits2Float(0xc3ec56cf), SkBits2Float(0x43a0e598), SkBits2Float(0xc30809b4), SkBits2Float(0x438337ac), SkBits2Float(0x3f3504f3))   // -472.678f, 321.794f, -136.038f, 262.435f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc369289a), SkBits2Float(0x408cdc40))   // -233.159f, 4.40189f
    .conicTo(SkBits2Float(0x427b82f4), SkBits2Float(0x432f5193), SkBits2Float(0x42e1eb60), SkBits2Float(0x42b12617), SkBits2Float(0x3f3504f3))   // 62.8779f, 175.319f, 112.96f, 88.5744f, 0.707107f
    .conicTo(SkBits2Float(0x43230aa3), SkBits2Float(0x3fea4196), SkBits2Float(0xc304feb4), SkBits2Float(0xc329162e), SkBits2Float(0x3f3504f3))   // 163.042f, 1.83013f, -132.995f, -169.087f, 0.707107f
    .conicTo(SkBits2Float(0xc3d68405), SkBits2Float(0xc3aa006f), SkBits2Float(0xc3ef8e7f), SkBits2Float(0xc37d4256), SkBits2Float(0x3f3504f3))   // -429.031f, -340.003f, -479.113f, -253.259f, 0.707107f
    .conicTo(SkBits2Float(0xc4044c7c), SkBits2Float(0xc32683cf), SkBits2Float(0xc369289a), SkBits2Float(0x408cdc40), SkBits2Float(0x3f3504f3))   // -529.195f, -166.515f, -233.159f, 4.40189f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc30db26a), SkBits2Float(0xc37fb0d7))   // -141.697f, -255.691f
    .conicTo(SkBits2Float(0xc1c64388), SkBits2Float(0x42830e25), SkBits2Float(0x428aae1e), SkBits2Float(0x41fa2834), SkBits2Float(0x3f3504f3))   // -24.783f, 65.5276f, 69.3401f, 31.2696f, 0.707107f
    .conicTo(SkBits2Float(0x4323768e), SkBits2Float(0xc03f4152), SkBits2Float(0x423a3252), SkBits2Float(0xc3a21a77), SkBits2Float(0x3f3504f3))   // 163.463f, -2.98836f, 46.5491f, -324.207f, 0.707107f
    .conicTo(SkBits2Float(0xc28cbac8), SkBits2Float(0xc4215b37), SkBits2Float(0xc3247ce4), SkBits2Float(0xc418cab4), SkBits2Float(0x3f3504f3))   // -70.3648f, -645.425f, -164.488f, -611.167f, 0.707107f
    .conicTo(SkBits2Float(0xc3814e32), SkBits2Float(0xc4103a31), SkBits2Float(0xc30db26a), SkBits2Float(0xc37fb0d7), SkBits2Float(0x3f3504f3))   // -258.611f, -576.909f, -141.697f, -255.691f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x42bf1a33), SkBits2Float(0xc3c6124c))   // 95.5512f, -396.143f
    .conicTo(SkBits2Float(0xc1aae6f8), SkBits2Float(0xc295d95d), SkBits2Float(0x42918542), SkBits2Float(0xc222aa8c), SkBits2Float(0x3f3504f3))   // -21.3628f, -74.9245f, 72.7603f, -40.6665f, 0.707107f
    .conicTo(SkBits2Float(0x4326e221), SkBits2Float(0xc0cd12f4), SkBits2Float(0x438de60c), SkBits2Float(0xc3a3d041), SkBits2Float(0x3f3504f3))   // 166.883f, -6.40856f, 283.797f, -327.627f, 0.707107f
    .conicTo(SkBits2Float(0x43c85b09), SkBits2Float(0xc422361b), SkBits2Float(0x43994b49), SkBits2Float(0xc42ac69e), SkBits2Float(0x3f3504f3))   // 400.711f, -648.845f, 306.588f, -683.103f, 0.707107f
    .conicTo(SkBits2Float(0x43547712), SkBits2Float(0xc4335721), SkBits2Float(0x42bf1a33), SkBits2Float(0xc3c6124c), SkBits2Float(0x3f3504f3))   // 212.465f, -717.361f, 95.5512f, -396.143f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43b7c98f), SkBits2Float(0xc3af9e21))   // 367.575f, -351.235f
    .conicTo(SkBits2Float(0x428f138a), SkBits2Float(0xc3345194), SkBits2Float(0x42f33d6e), SkBits2Float(0xc2bb2617), SkBits2Float(0x3f3504f3))   // 71.5382f, -180.319f, 121.62f, -93.5744f, 0.707107f
    .conicTo(SkBits2Float(0x432bb3a9), SkBits2Float(0xc0da9066), SkBits2Float(0x43e9de81), SkBits2Float(0xc331bf31), SkBits2Float(0x3f3504f3))   // 171.702f, -6.83013f, 467.738f, -177.747f, 0.707107f
    .conicTo(SkBits2Float(0x443ef196), SkBits2Float(0xc3ae54f0), SkBits2Float(0x44326c5a), SkBits2Float(0xc3d9b434), SkBits2Float(0x3f3504f3))   // 763.775f, -348.664f, 713.693f, -435.408f, 0.707107f
    .conicTo(SkBits2Float(0x4425e71e), SkBits2Float(0xc40289bc), SkBits2Float(0x43b7c98f), SkBits2Float(0xc3af9e21), SkBits2Float(0x3f3504f3))   // 663.611f, -522.152f, 367.575f, -351.235f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x4408c5ce), SkBits2Float(0xc30dfb1e))   // 547.091f, -141.981f
    .conicTo(SkBits2Float(0x4352734e), SkBits2Float(0xc34956f5), SkBits2Float(0x43410ea4), SkBits2Float(0xc2cd653e), SkBits2Float(0x3f3504f3))   // 210.45f, -201.34f, 193.057f, -102.698f, 0.707107f
    .conicTo(SkBits2Float(0x432fa9f9), SkBits2Float(0xc081c919), SkBits2Float(0x44001378), SkBits2Float(0x425d363a), SkBits2Float(0x3f3504f3))   // 175.664f, -4.0558f, 512.304f, 55.303f, 0.707107f
    .conicTo(SkBits2Float(0x44543c72), SkBits2Float(0x42e552cb), SkBits2Float(0x4458959e), SkBits2Float(0x4180287c), SkBits2Float(0x3f3504f3))   // 848.944f, 114.662f, 866.338f, 16.0198f, 0.707107f
    .conicTo(SkBits2Float(0x445ceec8), SkBits2Float(0xc2a53e8d), SkBits2Float(0x4408c5ce), SkBits2Float(0xc30dfb1e), SkBits2Float(0x3f3504f3))   // 883.731f, -82.6222f, 547.091f, -141.981f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x44098684), SkBits2Float(0x4305b53c))   // 550.102f, 133.708f
    .conicTo(SkBits2Float(0x43a5300e), SkBits2Float(0xc30026d3), SkBits2Float(0x437da548), SkBits2Float(0xc27f121c), SkBits2Float(0x3f3504f3))   // 330.375f, -128.152f, 253.646f, -63.7677f, 0.707107f
    .conicTo(SkBits2Float(0x4330ea74), SkBits2Float(0x3f1dc4b8), SkBits2Float(0x43c65234), SkBits2Float(0x43833cea), SkBits2Float(0x3f3504f3))   // 176.916f, 0.616283f, 396.642f, 262.476f, 0.707107f
    .conicTo(SkBits2Float(0x441a1798), SkBits2Float(0x44031579), SkBits2Float(0x442d464c), SkBits2Float(0x43e5f9cc), SkBits2Float(0x3f3504f3))   // 616.369f, 524.336f, 693.098f, 459.952f, 0.707107f
    .conicTo(SkBits2Float(0x44407502), SkBits2Float(0x43c5c8a6), SkBits2Float(0x44098684), SkBits2Float(0x4305b53c), SkBits2Float(0x3f3504f3))   // 769.828f, 395.568f, 550.102f, 133.708f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43bb9978), SkBits2Float(0x44487cfb))   // 375.199f, 801.953f
    .conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x43e60f46), SkBits2Float(0x43898486), SkBits2Float(0x43e60f46), SkBits2Float(0x3f3504f3))   // 375.199f, 460.119f, 275.035f, 460.119f, 0.707107f
    .conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x43e60f46), SkBits2Float(0x432edf26), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3))   // 174.872f, 460.119f, 174.872f, 801.953f, 0.707107f
    .conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x448ef92a), SkBits2Float(0x43898486), SkBits2Float(0x448ef92a), SkBits2Float(0x3f3504f3))   // 174.872f, 1143.79f, 275.035f, 1143.79f, 0.707107f
    .conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x448ef92a), SkBits2Float(0x43bb9978), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3))   // 375.199f, 1143.79f, 375.199f, 801.953f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x42d07148), SkBits2Float(0x445532a0))   // 104.221f, 852.791f
    .conicTo(SkBits2Float(0x43a1f94a), SkBits2Float(0x4413bb9c), SkBits2Float(0x437737c0), SkBits2Float(0x4403a309), SkBits2Float(0x3f3504f3))   // 323.948f, 590.931f, 247.218f, 526.547f, 0.707107f
    .conicTo(SkBits2Float(0x432a7ceb), SkBits2Float(0x43e714ed), SkBits2Float(0xc244f418), SkBits2Float(0x4435017b), SkBits2Float(0x3f3504f3))   // 170.488f, 462.163f, -49.2384f, 724.023f, 0.707107f
    .conicTo(SkBits2Float(0xc3867b7b), SkBits2Float(0x4476787f), SkBits2Float(0xc3403c22), SkBits2Float(0x44834889), SkBits2Float(0x3f3504f3))   // -268.965f, 985.883f, -192.235f, 1050.27f, 0.707107f
    .conicTo(SkBits2Float(0xc2e7029c), SkBits2Float(0x448b54d2), SkBits2Float(0x42d07148), SkBits2Float(0x445532a0), SkBits2Float(0x3f3504f3))   // -115.505f, 1114.65f, 104.221f, 852.791f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc30809b4), SkBits2Float(0x44336379))   // -136.038f, 717.554f
    .conicTo(SkBits2Float(0x43489a34), SkBits2Float(0x44248c83), SkBits2Float(0x43373589), SkBits2Float(0x440be36d), SkBits2Float(0x3f3504f3))   // 200.602f, 658.195f, 183.209f, 559.554f, 0.707107f
    .conicTo(SkBits2Float(0x4325d0dd), SkBits2Float(0x43e674af), SkBits2Float(0xc32ad30a), SkBits2Float(0x4402114e), SkBits2Float(0x3f3504f3))   // 165.816f, 460.912f, -170.824f, 520.27f, 0.707107f
    .conicTo(SkBits2Float(0xc3fdbb7b), SkBits2Float(0x4410e844), SkBits2Float(0xc3f50925), SkBits2Float(0x4429915a), SkBits2Float(0x3f3504f3))   // -507.465f, 579.629f, -490.071f, 678.271f, 0.707107f
    .conicTo(SkBits2Float(0xc3ec56cf), SkBits2Float(0x44423a6f), SkBits2Float(0xc30809b4), SkBits2Float(0x44336379), SkBits2Float(0x3f3504f3))   // -472.678f, 776.913f, -136.038f, 717.554f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc369289a), SkBits2Float(0x43e5c2b7))   // -233.159f, 459.521f
    .conicTo(SkBits2Float(0x427b82f4), SkBits2Float(0x441d9c08), SkBits2Float(0x42e1eb60), SkBits2Float(0x4407ec66), SkBits2Float(0x3f3504f3))   // 62.8779f, 630.438f, 112.96f, 543.694f, 0.707107f
    .conicTo(SkBits2Float(0x43230aa3), SkBits2Float(0x43e47988), SkBits2Float(0xc304feb4), SkBits2Float(0x438f042f), SkBits2Float(0x3f3504f3))   // 163.042f, 456.949f, -132.995f, 286.033f, 0.707107f
    .conicTo(SkBits2Float(0xc3d68405), SkBits2Float(0x42e63b5c), SkBits2Float(0xc3ef8e7f), SkBits2Float(0x4349dc36), SkBits2Float(0x3f3504f3))   // -429.031f, 115.116f, -479.113f, 201.86f, 0.707107f
    .conicTo(SkBits2Float(0xc4044c7c), SkBits2Float(0x43904d5e), SkBits2Float(0xc369289a), SkBits2Float(0x43e5c2b7), SkBits2Float(0x3f3504f3))   // -529.195f, 288.604f, -233.159f, 459.521f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0xc30db26a), SkBits2Float(0x43476db5))   // -141.697f, 199.429f
    .conicTo(SkBits2Float(0xc1c64388), SkBits2Float(0x44022968), SkBits2Float(0x428aae1e), SkBits2Float(0x43f331c9), SkBits2Float(0x3f3504f3))   // -24.783f, 520.647f, 69.3401f, 486.389f, 0.707107f
    .conicTo(SkBits2Float(0x4323768e), SkBits2Float(0x43e210c3), SkBits2Float(0x423a3252), SkBits2Float(0x4302e99e), SkBits2Float(0x3f3504f3))   // 163.463f, 452.131f, 46.5491f, 130.913f, 0.707107f
    .conicTo(SkBits2Float(0xc28cbac8), SkBits2Float(0xc33e4e50), SkBits2Float(0xc3247ce4), SkBits2Float(0xc31c0c44), SkBits2Float(0x3f3504f3))   // -70.3648f, -190.306f, -164.488f, -156.048f, 0.707107f
    .conicTo(SkBits2Float(0xc3814e32), SkBits2Float(0xc2f39470), SkBits2Float(0xc30db26a), SkBits2Float(0x43476db5), SkBits2Float(0x3f3504f3))   // -258.611f, -121.79f, -141.697f, 199.429f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x42bf1a33), SkBits2Float(0x426be7d0))   // 95.5512f, 58.9764f
    .conicTo(SkBits2Float(0xc1aae6f8), SkBits2Float(0x43be18ef), SkBits2Float(0x42918542), SkBits2Float(0x43cf39f4), SkBits2Float(0x3f3504f3))   // -21.3628f, 380.195f, 72.7603f, 414.453f, 0.707107f
    .conicTo(SkBits2Float(0x4326e221), SkBits2Float(0x43e05afa), SkBits2Float(0x438de60c), SkBits2Float(0x42fefc14), SkBits2Float(0x3f3504f3))   // 166.883f, 448.711f, 283.797f, 127.492f, 0.707107f
    .conicTo(SkBits2Float(0x43c85b09), SkBits2Float(0xc341b9e0), SkBits2Float(0x43994b49), SkBits2Float(0xc363fbec), SkBits2Float(0x3f3504f3))   // 400.711f, -193.726f, 306.588f, -227.984f, 0.707107f
    .conicTo(SkBits2Float(0x43547712), SkBits2Float(0xc3831efc), SkBits2Float(0x42bf1a33), SkBits2Float(0x426be7d0), SkBits2Float(0x3f3504f3))   // 212.465f, -262.242f, 95.5512f, 58.9764f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x43b7c98f), SkBits2Float(0x42cfc494))   // 367.575f, 103.884f
    .conicTo(SkBits2Float(0x428f138a), SkBits2Float(0x4389667c), SkBits2Float(0x42f33d6e), SkBits2Float(0x43b4c5c0), SkBits2Float(0x3f3504f3))   // 71.5382f, 274.801f, 121.62f, 361.545f, 0.707107f
    .conicTo(SkBits2Float(0x432bb3a9), SkBits2Float(0x43e02504), SkBits2Float(0x43e9de81), SkBits2Float(0x438aafae), SkBits2Float(0x3f3504f3))   // 171.702f, 448.289f, 467.738f, 277.372f, 0.707107f
    .conicTo(SkBits2Float(0x443ef196), SkBits2Float(0x42d4e958), SkBits2Float(0x44326c5a), SkBits2Float(0x419db120), SkBits2Float(0x3f3504f3))   // 763.775f, 106.456f, 713.693f, 19.7115f, 0.707107f
    .conicTo(SkBits2Float(0x4425e71e), SkBits2Float(0xc28610c8), SkBits2Float(0x43b7c98f), SkBits2Float(0x42cfc494), SkBits2Float(0x3f3504f3))   // 663.611f, -67.0328f, 367.575f, 103.884f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x4408c5ce), SkBits2Float(0x439c91b7))   // 547.091f, 313.138f
    .conicTo(SkBits2Float(0x4352734e), SkBits2Float(0x437dc797), SkBits2Float(0x43410ea4), SkBits2Float(0x43b035f6), SkBits2Float(0x3f3504f3))   // 210.45f, 253.78f, 193.057f, 352.422f, 0.707107f
    .conicTo(SkBits2Float(0x432fa9f9), SkBits2Float(0x43e18822), SkBits2Float(0x44001378), SkBits2Float(0x43ff360d), SkBits2Float(0x3f3504f3))   // 175.664f, 451.064f, 512.304f, 510.422f, 0.707107f
    .conicTo(SkBits2Float(0x44543c72), SkBits2Float(0x440e71fc), SkBits2Float(0x4458959e), SkBits2Float(0x43eb91ce), SkBits2Float(0x3f3504f3))   // 848.944f, 569.781f, 866.338f, 471.139f, 0.707107f
    .conicTo(SkBits2Float(0x445ceec8), SkBits2Float(0x43ba3fa3), SkBits2Float(0x4408c5ce), SkBits2Float(0x439c91b7), SkBits2Float(0x3f3504f3))   // 883.731f, 372.497f, 547.091f, 313.138f, 0.707107f
    .close()
    .moveTo(SkBits2Float(0x44098684), SkBits2Float(0x441334f2))   // 550.102f, 588.827f
    .conicTo(SkBits2Float(0x43a5300e), SkBits2Float(0x43a37bdc), SkBits2Float(0x437da548), SkBits2Float(0x43c3ad02), SkBits2Float(0x3f3504f3))   // 330.375f, 326.968f, 253.646f, 391.352f, 0.707107f
    .conicTo(SkBits2Float(0x4330ea74), SkBits2Float(0x43e3de28), SkBits2Float(0x43c65234), SkBits2Float(0x44336618), SkBits2Float(0x3f3504f3))   // 176.916f, 455.736f, 396.642f, 717.595f, 0.707107f
    .conicTo(SkBits2Float(0x441a1798), SkBits2Float(0x4474dd1c), SkBits2Float(0x442d464c), SkBits2Float(0x4464c489), SkBits2Float(0x3f3504f3))   // 616.369f, 979.455f, 693.098f, 915.071f, 0.707107f
    .conicTo(SkBits2Float(0x44407502), SkBits2Float(0x4454abf6), SkBits2Float(0x44098684), SkBits2Float(0x441334f2), SkBits2Float(0x3f3504f3))   // 769.828f, 850.687f, 550.102f, 588.827f, 0.707107f
    .close()
    .detach();

    testSimplify(reporter, path, __FUNCTION__);
}

DEF_TEST(SixtyOvals_2_2_9_73_reduced, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(377.219f, -141.981f)
                  .conicTo(40.5787f, -201.34f, 23.1855f, -102.698f, 0.707107f)
                  .lineTo(377.219f, -141.981f)
                  .close()
                  .moveTo(306.588f, -227.984f)
                  .conicTo(212.465f, -262.242f, 95.5512f, 58.9764f, 0.707107f)
                  .lineTo(306.588f, -227.984f)
                  .close()
                  .detach();
testSimplify(reporter, path, __FUNCTION__);
}

DEF_TEST(SixtyOvalsA, reporter) {
SkPath path = SkPathBuilder(SkPathFillType::kEvenOdd)
              .moveTo(11.1722f, -8.10398f)
              .conicTo(22.9143f, -10.3787f, 23.7764f, -7.72542f, 1.00863f)
              .conicTo(24.6671f, -4.98406f, 13.8147f, 0.0166066f, 0.973016f)
              .conicTo(24.6378f, 5.07425f, 23.7764f, 7.72542f, 1.00888f)
              .conicTo(22.8777f, 10.4915f, 11.1648f, 8.13034f, 0.960143f)
              .conicTo(16.9503f, 18.5866f, 14.6946f, 20.2254f, 1.00881f)
              .conicTo(12.4417f, 21.8623f, 4.29722f, 13.1468f, 1.0092f)
              .conicTo(2.92708f, 25, 0, 25, 0.955692f)
              .conicTo(-2.79361f, 25, -4.258f, 13.1048f, 1.00818f)
              .conicTo(-4.27813f, 13.1264f, -4.29822f, 13.1479f, 1.03158f)
              .conicTo(-12.44f, 21.8635f, -14.6946f, 20.2254f, 1.00811f)
              .conicTo(-16.9933f, 18.5554f, -11.1722f, 8.10398f, 0.989875f)
              .conicTo(-22.9143f, 10.3787f, -23.7764f, 7.72542f, 1.00863f)
              .conicTo(-24.6671f, 4.98406f, -13.8147f, -0.0166066f, 0.973016f)
              .conicTo(-24.6378f, -5.07425f, -23.7764f, -7.72542f, 1.00888f)
              .conicTo(-22.8777f, -10.4915f, -11.1648f, -8.13034f, 0.960143f)
              .conicTo(-16.9503f, -18.5866f, -14.6946f, -20.2254f, 1.00881f)
              .conicTo(-12.4417f, -21.8623f, -4.29722f, -13.1468f, 1.0092f)
              .conicTo(-2.92708f, -25, 0, -25, 0.955692f)
              .conicTo(2.79361f, -25, 4.258f, -13.1048f, 1.00818f)
              .conicTo(4.27813f, -13.1264f, 4.29822f, -13.1479f, 1.03158f)
              .conicTo(12.44f, -21.8635f, 14.6946f, -20.2254f, 1.00811f)
              .conicTo(16.9933f, -18.5554f, 11.1722f, -8.10398f, 0.989875f)
              .close()
              .detach();
    SkPath one(path);

    path = SkPathBuilder(SkPathFillType::kWinding)
           .moveTo(-1.54509f, -4.75528f)
           .conicTo(22.2313f, -12.4807f, 23.7764f, -7.72543f, 0.707107f)
           .conicTo(25.3215f, -2.97014f, 1.54509f, 4.75528f, 0.707107f)
           .conicTo(-22.2313f, 12.4807f, -23.7764f, 7.72543f, 0.707107f)
           .conicTo(-25.3215f, 2.97014f, -1.54509f, -4.75528f, 0.707107f)
           .close()
           .detach();
    SkPath two(path);
    std::ignore = Op(one, two, kUnion_SkPathOp);
}

DEF_TEST(SixtyOvalsAX, reporter) {
    SkPath path = SkPathBuilder(SkPathFillType::kEvenOdd)
    .moveTo(SkBits2Float(0x4132c174), SkBits2Float(0xc101a9e5))   // 11.1722f, -8.10398f
    .conicTo(SkBits2Float(0x41b7508a), SkBits2Float(0xc1260efe), SkBits2Float(0x41be3618), SkBits2Float(0xc0f736ad), SkBits2Float(0x3f811abd))   // 22.9143f, -10.3787f, 23.7764f, -7.72542f, 1.00863f
    .conicTo(SkBits2Float(0x41c5564b), SkBits2Float(0xc09f7d6d), SkBits2Float(0x415d0934), SkBits2Float(0x3c880a93), SkBits2Float(0x3f79179a))   // 24.6671f, -4.98406f, 13.8147f, 0.0166066f, 0.973016f
    .conicTo(SkBits2Float(0x41c51a48), SkBits2Float(0x40a2603c), SkBits2Float(0x41be3618), SkBits2Float(0x40f736ac), SkBits2Float(0x3f8122f3))   // 24.6378f, 5.07425f, 23.7764f, 7.72542f, 1.00888f
    .conicTo(SkBits2Float(0x41b7056f), SkBits2Float(0x4127dd49), SkBits2Float(0x4132a328), SkBits2Float(0x410215e1), SkBits2Float(0x3f75cbec))   // 22.8777f, 10.4915f, 11.1648f, 8.13034f, 0.960143f
    .conicTo(SkBits2Float(0x41879a3b), SkBits2Float(0x4194b151), SkBits2Float(0x416b1d34), SkBits2Float(0x41a1cdac), SkBits2Float(0x3f8120d4))   // 16.9503f, 18.5866f, 14.6946f, 20.2254f, 1.00881f
    .conicTo(SkBits2Float(0x41471107), SkBits2Float(0x41aee601), SkBits2Float(0x408982d1), SkBits2Float(0x41525939), SkBits2Float(0x3f812d7f))   // 12.4417f, 21.8623f, 4.29722f, 13.1468f, 1.0092f
    .conicTo(SkBits2Float(0x403b5543), SkBits2Float(0x41c80000), SkBits2Float(0x00000000), SkBits2Float(0x41c80000), SkBits2Float(0x3f74a837))   // 2.92708f, 25, 0, 25, 0.955692f
    .conicTo(SkBits2Float(0xc032ca93), SkBits2Float(0x41c80000), SkBits2Float(0xc088418e), SkBits2Float(0x4151ad32), SkBits2Float(0x3f810c2d))   // -2.79361f, 25, -4.258f, 13.1048f, 1.00818f
    .conicTo(SkBits2Float(0xc088e66c), SkBits2Float(0x4152058a), SkBits2Float(0xc0898afc), SkBits2Float(0x41525d9e), SkBits2Float(0x3f840adb))   // -4.27813f, 13.1264f, -4.29822f, 13.1479f, 1.03158f
    .conicTo(SkBits2Float(0xc1470a56), SkBits2Float(0x41aee870), SkBits2Float(0xc16b1d36), SkBits2Float(0x41a1cdac), SkBits2Float(0x3f81099f))   // -12.44f, 21.8635f, -14.6946f, 20.2254f, 1.00811f
    .conicTo(SkBits2Float(0xc187f23a), SkBits2Float(0x41947162), SkBits2Float(0xc132c174), SkBits2Float(0x4101a9e5), SkBits2Float(0x3f7d6873))   // -16.9933f, 18.5554f, -11.1722f, 8.10398f, 0.989875f
    .conicTo(SkBits2Float(0xc1b7508a), SkBits2Float(0x41260efe), SkBits2Float(0xc1be3618), SkBits2Float(0x40f736ad), SkBits2Float(0x3f811abd))   // -22.9143f, 10.3787f, -23.7764f, 7.72542f, 1.00863f
    .conicTo(SkBits2Float(0xc1c5564b), SkBits2Float(0x409f7d6d), SkBits2Float(0xc15d0934), SkBits2Float(0xbc880a93), SkBits2Float(0x3f79179a))   // -24.6671f, 4.98406f, -13.8147f, -0.0166066f, 0.973016f
    .conicTo(SkBits2Float(0xc1c51a48), SkBits2Float(0xc0a2603c), SkBits2Float(0xc1be3618), SkBits2Float(0xc0f736ac), SkBits2Float(0x3f8122f3))   // -24.6378f, -5.07425f, -23.7764f, -7.72542f, 1.00888f
    .conicTo(SkBits2Float(0xc1b7056f), SkBits2Float(0xc127dd49), SkBits2Float(0xc132a328), SkBits2Float(0xc10215e1), SkBits2Float(0x3f75cbec))   // -22.8777f, -10.4915f, -11.1648f, -8.13034f, 0.960143f
    .conicTo(SkBits2Float(0xc1879a3b), SkBits2Float(0xc194b151), SkBits2Float(0xc16b1d34), SkBits2Float(0xc1a1cdac), SkBits2Float(0x3f8120d4))   // -16.9503f, -18.5866f, -14.6946f, -20.2254f, 1.00881f
    .conicTo(SkBits2Float(0xc1471107), SkBits2Float(0xc1aee601), SkBits2Float(0xc08982d1), SkBits2Float(0xc1525939), SkBits2Float(0x3f812d7f))   // -12.4417f, -21.8623f, -4.29722f, -13.1468f, 1.0092f
    .conicTo(SkBits2Float(0xc03b5543), SkBits2Float(0xc1c80000), SkBits2Float(0x00000000), SkBits2Float(0xc1c80000), SkBits2Float(0x3f74a837))   // -2.92708f, -25, 0, -25, 0.955692f
    .conicTo(SkBits2Float(0x4032ca93), SkBits2Float(0xc1c80000), SkBits2Float(0x4088418e), SkBits2Float(0xc151ad32), SkBits2Float(0x3f810c2d))   // 2.79361f, -25, 4.258f, -13.1048f, 1.00818f
    .conicTo(SkBits2Float(0x4088e66c), SkBits2Float(0xc152058a), SkBits2Float(0x40898afc), SkBits2Float(0xc1525d9e), SkBits2Float(0x3f840adb))   // 4.27813f, -13.1264f, 4.29822f, -13.1479f, 1.03158f
    .conicTo(SkBits2Float(0x41470a56), SkBits2Float(0xc1aee870), SkBits2Float(0x416b1d36), SkBits2Float(0xc1a1cdac), SkBits2Float(0x3f81099f))   // 12.44f, -21.8635f, 14.6946f, -20.2254f, 1.00811f
    .conicTo(SkBits2Float(0x4187f23a), SkBits2Float(0xc1947162), SkBits2Float(0x4132c174), SkBits2Float(0xc101a9e5), SkBits2Float(0x3f7d6873))   // 16.9933f, -18.5554f, 11.1722f, -8.10398f, 0.989875f
    .close()
    .close()
    .detach();
    SkPath one(path);

    path = SkPathBuilder(SkPathFillType::kWinding)
    .moveTo(SkBits2Float(0xbfc5c55c), SkBits2Float(0xc0982b46))  // -1.54509f, -4.75528f
    .conicTo(SkBits2Float(0x41b1d9c2), SkBits2Float(0xc147b0fc), SkBits2Float(0x41be3618), SkBits2Float(0xc0f736b3), SkBits2Float(0x3f3504f3))   // 22.2313f, -12.4807f, 23.7764f, -7.72543f, 0.707107f
    .conicTo(SkBits2Float(0x41ca926e), SkBits2Float(0xc03e16da), SkBits2Float(0x3fc5c55c), SkBits2Float(0x40982b46), SkBits2Float(0x3f3504f3))   // 25.3215f, -2.97014f, 1.54509f, 4.75528f, 0.707107f
    .conicTo(SkBits2Float(0xc1b1d9c2), SkBits2Float(0x4147b0fc), SkBits2Float(0xc1be3618), SkBits2Float(0x40f736b3), SkBits2Float(0x3f3504f3))   // -22.2313f, 12.4807f, -23.7764f, 7.72543f, 0.707107f
    .conicTo(SkBits2Float(0xc1ca926e), SkBits2Float(0x403e16da), SkBits2Float(0xbfc5c55c), SkBits2Float(0xc0982b46), SkBits2Float(0x3f3504f3))   // -25.3215f, 2.97014f, -1.54509f, -4.75528f, 0.707107f
    .close()
    .detach();
    SkPath two(path);

    std::ignore = Op(one, two, kUnion_SkPathOp);
}
