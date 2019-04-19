/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsExtendedTest.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SubsetPath.h"

#define DEBUG_SIMPLIFY_FAILS 0

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
                SkPath rotated;
                SkMatrix matrix;
                matrix.reset();
                matrix.postRotate(r, 0, 0);
                matrix.postTranslate(x * set.fXSpacing, y * set.fYSpacing);
                oval.transform(matrix, &rotated);
                if (builder) {
                    builder->add(rotated, kUnion_SkPathOp);
                } else if (!region) {
                    Op(*result, rotated, kUnion_SkPathOp, result);
                } else {
                    SkRegion rgnB, openClip;
                    openClip.setRect(-16000, -16000, 16000, 16000);
                    rgnB.setPath(rotated, openClip);
                    region->op(rgnB, SkRegion::kUnion_Op);
                }
            }
        }
    }
    if (builder) {
        builder->resolve(result);
    } else if (region) {
        region->getBoundaryPath(result);
    }
}

static void testOne(skiatest::Reporter* reporter, const OvalSet& set) {
    SkPath oval, regionResult, builderResult, opResult;
    oval.setFillType(SkPath::kWinding_FillType);
    oval.addOval(set.fBounds);
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

#include "CommandLineFlags.h"

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

#if DEBUG_SIMPLIFY_FAILS
static bool simplify_fails(skiatest::Reporter* reporter, const SkPath& path) {
    SkPath dummy;
    bool failed = !Simplify(path, &dummy);
    if (!failed) {
        SkBitmap bitmap;
        failed = !!comparePaths(reporter, __FUNCTION__, path, dummy, bitmap);
    }
    return failed;
}

static SkPath subset_simplify_fail(skiatest::Reporter* reporter, const SkPath& path) {
    SubsetContours subsetContours(path);
    bool failed = simplify_fails(reporter, path);
    SkASSERT(failed);
    SkPath lastFailed = path;
    SkPath minimal;
    while (subsetContours.subset(failed, &minimal)) {
        failed = simplify_fails(reporter, minimal);
        SkDebugf(" %s\n", failed ? "failed" : "");
        if (failed) {
            lastFailed = minimal;
        }
    }
    failed = simplify_fails(reporter, lastFailed);
    SkASSERT(failed);
    SubsetVerbs subsetVerbs(lastFailed);
    while (subsetVerbs.subset(failed, &minimal)) {
        failed = simplify_fails(reporter, minimal);
        SkDebugf(" %s\n", failed ? "failed" : "");
        if (failed) {
            lastFailed = minimal;
        }
    }
    return lastFailed;
}
#endif

DEF_TEST(SixtyOvals_2_2_9_73, reporter) {
    SkPath path;
    path.moveTo(SkBits2Float(0x434d53ca), SkBits2Float(0x43ad6ab0));  // 205.327f, 346.833f
path.conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x40a00000), SkBits2Float(0x42d253ca), SkBits2Float(0x40a00000), SkBits2Float(0x3f3504f3));  // 205.327f, 5, 105.164f, 5, 0.707107f
path.conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x40a00000), SkBits2Float(0x40a00000), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3));  // 5, 5, 5, 346.833f, 0.707107f
path.conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x442c2ab0), SkBits2Float(0x42d253ca), SkBits2Float(0x442c2ab0), SkBits2Float(0x3f3504f3));  // 5, 688.667f, 105.164f, 688.667f, 0.707107f
path.conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x442c2ab0), SkBits2Float(0x434d53ca), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3));  // 205.327f, 688.667f, 205.327f, 346.833f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc2834d04), SkBits2Float(0x43c6d5fb));  // -65.6504f, 397.672f
path.conicTo(SkBits2Float(0x431a136e), SkBits2Float(0x4307cfe3), SkBits2Float(0x429ab133), SkBits2Float(0x428edb31), SkBits2Float(0x3f3504f3));  // 154.076f, 135.812f, 77.3461f, 71.4281f, 0.707107f
path.conicTo(SkBits2Float(0x3f1dc4d0), SkBits2Float(0x40e169c2), SkBits2Float(0xc35b1c2c), SkBits2Float(0x438673b0), SkBits2Float(0x3f3504f3));  // 0.616284f, 7.04416f, -219.11f, 268.904f, 0.707107f
path.conicTo(SkBits2Float(0xc3db6b0e), SkBits2Float(0x4404b0dc), SkBits2Float(0xc3b50da4), SkBits2Float(0x4414c96f), SkBits2Float(0x3f3504f3));  // -438.836f, 530.763f, -362.107f, 595.147f, 0.707107f
path.conicTo(SkBits2Float(0xc38eb03a), SkBits2Float(0x4424e202), SkBits2Float(0xc2834d04), SkBits2Float(0x43c6d5fb), SkBits2Float(0x3f3504f3));  // -285.377f, 659.531f, -65.6504f, 397.672f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc398f46d), SkBits2Float(0x438337ac));  // -305.91f, 262.435f
path.conicTo(SkBits2Float(0x41f5d870), SkBits2Float(0x434b137f), SkBits2Float(0x41556629), SkBits2Float(0x42d0de52), SkBits2Float(0x3f3504f3));  // 30.7307f, 203.076f, 13.3374f, 104.434f, 0.707107f
path.conicTo(SkBits2Float(0xc081c918), SkBits2Float(0x40b95a5c), SkBits2Float(0xc3aa5918), SkBits2Float(0x42824d58), SkBits2Float(0x3f3504f3));  // -4.0558f, 5.79228f, -340.696f, 65.1511f, 0.707107f
path.conicTo(SkBits2Float(0xc4295587), SkBits2Float(0x42f9050a), SkBits2Float(0xc424fc5c), SkBits2Float(0x435f26db), SkBits2Float(0x3f3504f3));  // -677.336f, 124.51f, -659.943f, 223.152f, 0.707107f
path.conicTo(SkBits2Float(0xc420a331), SkBits2Float(0x43a0e598), SkBits2Float(0xc398f46d), SkBits2Float(0x438337ac), SkBits2Float(0x3f3504f3));  // -642.55f, 321.794f, -305.91f, 262.435f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc3c983e0), SkBits2Float(0x408cdc40));  // -403.03f, 4.40189f
path.conicTo(SkBits2Float(0xc2d5fcd2), SkBits2Float(0x432f5193), SkBits2Float(0xc263a5d9), SkBits2Float(0x42b12617), SkBits2Float(0x3f3504f3));  // -106.994f, 175.319f, -56.912f, 88.5744f, 0.707107f
path.conicTo(SkBits2Float(0xc0da9066), SkBits2Float(0x3fea4196), SkBits2Float(0xc3976eed), SkBits2Float(0xc329162e), SkBits2Float(0x3f3504f3));  // -6.83013f, 1.83013f, -302.867f, -169.087f, 0.707107f
path.conicTo(SkBits2Float(0xc415b9cc), SkBits2Float(0xc3aa006f), SkBits2Float(0xc4223f09), SkBits2Float(0xc37d4256), SkBits2Float(0x3f3504f3));  // -598.903f, -340.003f, -648.985f, -253.259f, 0.707107f
path.conicTo(SkBits2Float(0xc42ec446), SkBits2Float(0xc32683cf), SkBits2Float(0xc3c983e0), SkBits2Float(0x408cdc40), SkBits2Float(0x3f3504f3));  // -699.067f, -166.515f, -403.03f, 4.40189f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc39bc8c8), SkBits2Float(0xc37fb0d7));  // -311.569f, -255.691f
path.conicTo(SkBits2Float(0xc342a797), SkBits2Float(0x42830e25), SkBits2Float(0xc2c9102e), SkBits2Float(0x41fa2834), SkBits2Float(0x3f3504f3));  // -194.655f, 65.5276f, -100.532f, 31.2696f, 0.707107f
path.conicTo(SkBits2Float(0xc0cd12f5), SkBits2Float(0xc03f4152), SkBits2Float(0xc2f6a523), SkBits2Float(0xc3a21a77), SkBits2Float(0x3f3504f3));  // -6.40856f, -2.98836f, -123.323f, -324.207f, 0.707107f
path.conicTo(SkBits2Float(0xc3703c8a), SkBits2Float(0xc4215b37), SkBits2Float(0xc3a72e05), SkBits2Float(0xc418cab4), SkBits2Float(0x3f3504f3));  // -240.236f, -645.425f, -334.36f, -611.167f, 0.707107f
path.conicTo(SkBits2Float(0xc3d63dc5), SkBits2Float(0xc4103a31), SkBits2Float(0xc39bc8c8), SkBits2Float(0xc37fb0d7), SkBits2Float(0x3f3504f3));  // -428.483f, -576.909f, -311.569f, -255.691f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc294a419), SkBits2Float(0xc3c6124c));  // -74.3205f, -396.143f
path.conicTo(SkBits2Float(0xc33f3c05), SkBits2Float(0xc295d95d), SkBits2Float(0xc2c2390a), SkBits2Float(0xc222aa8c), SkBits2Float(0x3f3504f3));  // -191.234f, -74.9245f, -97.1114f, -40.6665f, 0.707107f
path.conicTo(SkBits2Float(0xc03f4154), SkBits2Float(0xc0cd12f4), SkBits2Float(0x42e3d9e6), SkBits2Float(0xc3a3d041), SkBits2Float(0x3f3504f3));  // -2.98836f, -6.40856f, 113.926f, -327.627f, 0.707107f
path.conicTo(SkBits2Float(0x4366d6ec), SkBits2Float(0xc422361b), SkBits2Float(0x4308b76c), SkBits2Float(0xc42ac69e), SkBits2Float(0x3f3504f3));  // 230.84f, -648.845f, 136.716f, -683.103f, 0.707107f
path.conicTo(SkBits2Float(0x422a5fb0), SkBits2Float(0xc4335721), SkBits2Float(0xc294a419), SkBits2Float(0xc3c6124c), SkBits2Float(0x3f3504f3));  // 42.5934f, -717.361f, -74.3205f, -396.143f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x4345b3f8), SkBits2Float(0xc3af9e21));  // 197.703f, -351.235f
path.conicTo(SkBits2Float(0xc2c4aac2), SkBits2Float(0xc3345194), SkBits2Float(0xc24101bb), SkBits2Float(0xc2bb2617), SkBits2Float(0x3f3504f3));  // -98.3335f, -180.319f, -48.2517f, -93.5744f, 0.707107f
path.conicTo(SkBits2Float(0x3fea41a0), SkBits2Float(0xc0da9066), SkBits2Float(0x4394eeee), SkBits2Float(0xc331bf31), SkBits2Float(0x3f3504f3));  // 1.83013f, -6.83013f, 297.867f, -177.747f, 0.707107f
path.conicTo(SkBits2Float(0x441479cd), SkBits2Float(0xc3ae54f0), SkBits2Float(0x4407f490), SkBits2Float(0xc3d9b434), SkBits2Float(0x3f3504f3));  // 593.903f, -348.664f, 543.821f, -435.408f, 0.707107f
path.conicTo(SkBits2Float(0x43f6dea8), SkBits2Float(0xc40289bc), SkBits2Float(0x4345b3f8), SkBits2Float(0xc3af9e21), SkBits2Float(0x3f3504f3));  // 493.74f, -522.152f, 197.703f, -351.235f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43bc9c08), SkBits2Float(0xc30dfb1e));  // 377.219f, -141.981f
path.conicTo(SkBits2Float(0x422250a2), SkBits2Float(0xc34956f5), SkBits2Float(0x41b97bee), SkBits2Float(0xc2cd653e), SkBits2Float(0x3f3504f3));  // 40.5787f, -201.34f, 23.1855f, -102.698f, 0.707107f
path.conicTo(SkBits2Float(0x40b95a5b), SkBits2Float(0xc081c919), SkBits2Float(0x43ab375e), SkBits2Float(0x425d363a), SkBits2Float(0x3f3504f3));  // 5.79228f, -4.0558f, 342.433f, 55.303f, 0.707107f
path.conicTo(SkBits2Float(0x4429c4a9), SkBits2Float(0x42e552cb), SkBits2Float(0x442e1dd4), SkBits2Float(0x4180287c), SkBits2Float(0x3f3504f3));  // 679.073f, 114.662f, 696.466f, 16.0198f, 0.707107f
path.conicTo(SkBits2Float(0x443276ff), SkBits2Float(0xc2a53e8d), SkBits2Float(0x43bc9c08), SkBits2Float(0xc30dfb1e), SkBits2Float(0x3f3504f3));  // 713.859f, -82.6222f, 377.219f, -141.981f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43be1d75), SkBits2Float(0x4305b53c));  // 380.23f, 133.708f
path.conicTo(SkBits2Float(0x432080f6), SkBits2Float(0xc30026d3), SkBits2Float(0x42a78c44), SkBits2Float(0xc27f121c), SkBits2Float(0x3f3504f3));  // 160.504f, -128.152f, 83.774f, -63.7677f, 0.707107f
path.conicTo(SkBits2Float(0x40e169c3), SkBits2Float(0x3f1dc4b8), SkBits2Float(0x4362c542), SkBits2Float(0x43833cea), SkBits2Float(0x3f3504f3));  // 7.04416f, 0.616283f, 226.771f, 262.476f, 0.707107f
path.conicTo(SkBits2Float(0x43df3f9c), SkBits2Float(0x44031579), SkBits2Float(0x4402ce83), SkBits2Float(0x43e5f9cc), SkBits2Float(0x3f3504f3));  // 446.497f, 524.336f, 523.227f, 459.952f, 0.707107f
path.conicTo(SkBits2Float(0x4415fd38), SkBits2Float(0x43c5c8a6), SkBits2Float(0x43be1d75), SkBits2Float(0x4305b53c), SkBits2Float(0x3f3504f3));  // 599.957f, 395.568f, 380.23f, 133.708f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x434d53ca), SkBits2Float(0x44487cfb));  // 205.327f, 801.953f
path.conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x43e60f46), SkBits2Float(0x42d253ca), SkBits2Float(0x43e60f46), SkBits2Float(0x3f3504f3));  // 205.327f, 460.119f, 105.164f, 460.119f, 0.707107f
path.conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x43e60f46), SkBits2Float(0x40a00000), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3));  // 5, 460.119f, 5, 801.953f, 0.707107f
path.conicTo(SkBits2Float(0x40a00000), SkBits2Float(0x448ef92a), SkBits2Float(0x42d253ca), SkBits2Float(0x448ef92a), SkBits2Float(0x3f3504f3));  // 5, 1143.79f, 105.164f, 1143.79f, 0.707107f
path.conicTo(SkBits2Float(0x434d53ca), SkBits2Float(0x448ef92a), SkBits2Float(0x434d53ca), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3));  // 205.327f, 1143.79f, 205.327f, 801.953f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc2834d04), SkBits2Float(0x445532a0));  // -65.6504f, 852.791f
path.conicTo(SkBits2Float(0x431a136e), SkBits2Float(0x4413bb9c), SkBits2Float(0x429ab133), SkBits2Float(0x4403a309), SkBits2Float(0x3f3504f3));  // 154.076f, 590.931f, 77.3461f, 526.547f, 0.707107f
path.conicTo(SkBits2Float(0x3f1dc4d0), SkBits2Float(0x43e714ed), SkBits2Float(0xc35b1c2c), SkBits2Float(0x4435017b), SkBits2Float(0x3f3504f3));  // 0.616284f, 462.163f, -219.11f, 724.023f, 0.707107f
path.conicTo(SkBits2Float(0xc3db6b0e), SkBits2Float(0x4476787f), SkBits2Float(0xc3b50da4), SkBits2Float(0x44834889), SkBits2Float(0x3f3504f3));  // -438.836f, 985.883f, -362.107f, 1050.27f, 0.707107f
path.conicTo(SkBits2Float(0xc38eb03a), SkBits2Float(0x448b54d2), SkBits2Float(0xc2834d04), SkBits2Float(0x445532a0), SkBits2Float(0x3f3504f3));  // -285.377f, 1114.65f, -65.6504f, 852.791f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc398f46d), SkBits2Float(0x44336379));  // -305.91f, 717.554f
path.conicTo(SkBits2Float(0x41f5d870), SkBits2Float(0x44248c83), SkBits2Float(0x41556629), SkBits2Float(0x440be36d), SkBits2Float(0x3f3504f3));  // 30.7307f, 658.195f, 13.3374f, 559.554f, 0.707107f
path.conicTo(SkBits2Float(0xc081c918), SkBits2Float(0x43e674af), SkBits2Float(0xc3aa5918), SkBits2Float(0x4402114e), SkBits2Float(0x3f3504f3));  // -4.0558f, 460.912f, -340.696f, 520.27f, 0.707107f
path.conicTo(SkBits2Float(0xc4295587), SkBits2Float(0x4410e844), SkBits2Float(0xc424fc5c), SkBits2Float(0x4429915a), SkBits2Float(0x3f3504f3));  // -677.336f, 579.629f, -659.943f, 678.271f, 0.707107f
path.conicTo(SkBits2Float(0xc420a331), SkBits2Float(0x44423a6f), SkBits2Float(0xc398f46d), SkBits2Float(0x44336379), SkBits2Float(0x3f3504f3));  // -642.55f, 776.913f, -305.91f, 717.554f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc3c983e0), SkBits2Float(0x43e5c2b7));  // -403.03f, 459.521f
path.conicTo(SkBits2Float(0xc2d5fcd2), SkBits2Float(0x441d9c08), SkBits2Float(0xc263a5d9), SkBits2Float(0x4407ec66), SkBits2Float(0x3f3504f3));  // -106.994f, 630.438f, -56.912f, 543.694f, 0.707107f
path.conicTo(SkBits2Float(0xc0da9066), SkBits2Float(0x43e47988), SkBits2Float(0xc3976eed), SkBits2Float(0x438f042f), SkBits2Float(0x3f3504f3));  // -6.83013f, 456.949f, -302.867f, 286.033f, 0.707107f
path.conicTo(SkBits2Float(0xc415b9cc), SkBits2Float(0x42e63b5c), SkBits2Float(0xc4223f09), SkBits2Float(0x4349dc36), SkBits2Float(0x3f3504f3));  // -598.903f, 115.116f, -648.985f, 201.86f, 0.707107f
path.conicTo(SkBits2Float(0xc42ec446), SkBits2Float(0x43904d5e), SkBits2Float(0xc3c983e0), SkBits2Float(0x43e5c2b7), SkBits2Float(0x3f3504f3));  // -699.067f, 288.604f, -403.03f, 459.521f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc39bc8c8), SkBits2Float(0x43476db5));  // -311.569f, 199.429f
path.conicTo(SkBits2Float(0xc342a797), SkBits2Float(0x44022968), SkBits2Float(0xc2c9102e), SkBits2Float(0x43f331c9), SkBits2Float(0x3f3504f3));  // -194.655f, 520.647f, -100.532f, 486.389f, 0.707107f
path.conicTo(SkBits2Float(0xc0cd12f5), SkBits2Float(0x43e210c3), SkBits2Float(0xc2f6a523), SkBits2Float(0x4302e99e), SkBits2Float(0x3f3504f3));  // -6.40856f, 452.131f, -123.323f, 130.913f, 0.707107f
path.conicTo(SkBits2Float(0xc3703c8a), SkBits2Float(0xc33e4e50), SkBits2Float(0xc3a72e05), SkBits2Float(0xc31c0c44), SkBits2Float(0x3f3504f3));  // -240.236f, -190.306f, -334.36f, -156.048f, 0.707107f
path.conicTo(SkBits2Float(0xc3d63dc5), SkBits2Float(0xc2f39470), SkBits2Float(0xc39bc8c8), SkBits2Float(0x43476db5), SkBits2Float(0x3f3504f3));  // -428.483f, -121.79f, -311.569f, 199.429f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc294a419), SkBits2Float(0x426be7d0));  // -74.3205f, 58.9764f
path.conicTo(SkBits2Float(0xc33f3c05), SkBits2Float(0x43be18ef), SkBits2Float(0xc2c2390a), SkBits2Float(0x43cf39f4), SkBits2Float(0x3f3504f3));  // -191.234f, 380.195f, -97.1114f, 414.453f, 0.707107f
path.conicTo(SkBits2Float(0xc03f4154), SkBits2Float(0x43e05afa), SkBits2Float(0x42e3d9e6), SkBits2Float(0x42fefc14), SkBits2Float(0x3f3504f3));  // -2.98836f, 448.711f, 113.926f, 127.492f, 0.707107f
path.conicTo(SkBits2Float(0x4366d6ec), SkBits2Float(0xc341b9e0), SkBits2Float(0x4308b76c), SkBits2Float(0xc363fbec), SkBits2Float(0x3f3504f3));  // 230.84f, -193.726f, 136.716f, -227.984f, 0.707107f
path.conicTo(SkBits2Float(0x422a5fb0), SkBits2Float(0xc3831efc), SkBits2Float(0xc294a419), SkBits2Float(0x426be7d0), SkBits2Float(0x3f3504f3));  // 42.5934f, -262.242f, -74.3205f, 58.9764f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x4345b3f8), SkBits2Float(0x42cfc494));  // 197.703f, 103.884f
path.conicTo(SkBits2Float(0xc2c4aac2), SkBits2Float(0x4389667c), SkBits2Float(0xc24101bb), SkBits2Float(0x43b4c5c0), SkBits2Float(0x3f3504f3));  // -98.3335f, 274.801f, -48.2517f, 361.545f, 0.707107f
path.conicTo(SkBits2Float(0x3fea41a0), SkBits2Float(0x43e02504), SkBits2Float(0x4394eeee), SkBits2Float(0x438aafae), SkBits2Float(0x3f3504f3));  // 1.83013f, 448.289f, 297.867f, 277.372f, 0.707107f
path.conicTo(SkBits2Float(0x441479cd), SkBits2Float(0x42d4e958), SkBits2Float(0x4407f490), SkBits2Float(0x419db120), SkBits2Float(0x3f3504f3));  // 593.903f, 106.456f, 543.821f, 19.7115f, 0.707107f
path.conicTo(SkBits2Float(0x43f6dea8), SkBits2Float(0xc28610c8), SkBits2Float(0x4345b3f8), SkBits2Float(0x42cfc494), SkBits2Float(0x3f3504f3));  // 493.74f, -67.0328f, 197.703f, 103.884f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43bc9c08), SkBits2Float(0x439c91b7));  // 377.219f, 313.138f
path.conicTo(SkBits2Float(0x422250a2), SkBits2Float(0x437dc797), SkBits2Float(0x41b97bee), SkBits2Float(0x43b035f6), SkBits2Float(0x3f3504f3));  // 40.5787f, 253.78f, 23.1855f, 352.422f, 0.707107f
path.conicTo(SkBits2Float(0x40b95a5b), SkBits2Float(0x43e18822), SkBits2Float(0x43ab375e), SkBits2Float(0x43ff360d), SkBits2Float(0x3f3504f3));  // 5.79228f, 451.064f, 342.433f, 510.422f, 0.707107f
path.conicTo(SkBits2Float(0x4429c4a9), SkBits2Float(0x440e71fc), SkBits2Float(0x442e1dd4), SkBits2Float(0x43eb91ce), SkBits2Float(0x3f3504f3));  // 679.073f, 569.781f, 696.466f, 471.139f, 0.707107f
path.conicTo(SkBits2Float(0x443276ff), SkBits2Float(0x43ba3fa3), SkBits2Float(0x43bc9c08), SkBits2Float(0x439c91b7), SkBits2Float(0x3f3504f3));  // 713.859f, 372.497f, 377.219f, 313.138f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43be1d75), SkBits2Float(0x441334f2));  // 380.23f, 588.827f
path.conicTo(SkBits2Float(0x432080f6), SkBits2Float(0x43a37bdc), SkBits2Float(0x42a78c44), SkBits2Float(0x43c3ad02), SkBits2Float(0x3f3504f3));  // 160.504f, 326.968f, 83.774f, 391.352f, 0.707107f
path.conicTo(SkBits2Float(0x40e169c3), SkBits2Float(0x43e3de28), SkBits2Float(0x4362c542), SkBits2Float(0x44336618), SkBits2Float(0x3f3504f3));  // 7.04416f, 455.736f, 226.771f, 717.595f, 0.707107f
path.conicTo(SkBits2Float(0x43df3f9c), SkBits2Float(0x4474dd1c), SkBits2Float(0x4402ce83), SkBits2Float(0x4464c489), SkBits2Float(0x3f3504f3));  // 446.497f, 979.455f, 523.227f, 915.071f, 0.707107f
path.conicTo(SkBits2Float(0x4415fd38), SkBits2Float(0x4454abf6), SkBits2Float(0x43be1d75), SkBits2Float(0x441334f2), SkBits2Float(0x3f3504f3));  // 599.957f, 850.687f, 380.23f, 588.827f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43bb9978), SkBits2Float(0x43ad6ab0));  // 375.199f, 346.833f
path.conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x40a00000), SkBits2Float(0x43898486), SkBits2Float(0x40a00000), SkBits2Float(0x3f3504f3));  // 375.199f, 5, 275.035f, 5, 0.707107f
path.conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x40a00000), SkBits2Float(0x432edf26), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3));  // 174.872f, 5, 174.872f, 346.833f, 0.707107f
path.conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x442c2ab0), SkBits2Float(0x43898486), SkBits2Float(0x442c2ab0), SkBits2Float(0x3f3504f3));  // 174.872f, 688.667f, 275.035f, 688.667f, 0.707107f
path.conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x442c2ab0), SkBits2Float(0x43bb9978), SkBits2Float(0x43ad6ab0), SkBits2Float(0x3f3504f3));  // 375.199f, 688.667f, 375.199f, 346.833f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42d07148), SkBits2Float(0x43c6d5fb));  // 104.221f, 397.672f
path.conicTo(SkBits2Float(0x43a1f94a), SkBits2Float(0x4307cfe3), SkBits2Float(0x437737c0), SkBits2Float(0x428edb31), SkBits2Float(0x3f3504f3));  // 323.948f, 135.812f, 247.218f, 71.4281f, 0.707107f
path.conicTo(SkBits2Float(0x432a7ceb), SkBits2Float(0x40e169c2), SkBits2Float(0xc244f418), SkBits2Float(0x438673b0), SkBits2Float(0x3f3504f3));  // 170.488f, 7.04416f, -49.2384f, 268.904f, 0.707107f
path.conicTo(SkBits2Float(0xc3867b7b), SkBits2Float(0x4404b0dc), SkBits2Float(0xc3403c22), SkBits2Float(0x4414c96f), SkBits2Float(0x3f3504f3));  // -268.965f, 530.763f, -192.235f, 595.147f, 0.707107f
path.conicTo(SkBits2Float(0xc2e7029c), SkBits2Float(0x4424e202), SkBits2Float(0x42d07148), SkBits2Float(0x43c6d5fb), SkBits2Float(0x3f3504f3));  // -115.505f, 659.531f, 104.221f, 397.672f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc30809b4), SkBits2Float(0x438337ac));  // -136.038f, 262.435f
path.conicTo(SkBits2Float(0x43489a34), SkBits2Float(0x434b137f), SkBits2Float(0x43373589), SkBits2Float(0x42d0de52), SkBits2Float(0x3f3504f3));  // 200.602f, 203.076f, 183.209f, 104.434f, 0.707107f
path.conicTo(SkBits2Float(0x4325d0dd), SkBits2Float(0x40b95a5c), SkBits2Float(0xc32ad30a), SkBits2Float(0x42824d58), SkBits2Float(0x3f3504f3));  // 165.816f, 5.79228f, -170.824f, 65.1511f, 0.707107f
path.conicTo(SkBits2Float(0xc3fdbb7b), SkBits2Float(0x42f9050a), SkBits2Float(0xc3f50925), SkBits2Float(0x435f26db), SkBits2Float(0x3f3504f3));  // -507.465f, 124.51f, -490.071f, 223.152f, 0.707107f
path.conicTo(SkBits2Float(0xc3ec56cf), SkBits2Float(0x43a0e598), SkBits2Float(0xc30809b4), SkBits2Float(0x438337ac), SkBits2Float(0x3f3504f3));  // -472.678f, 321.794f, -136.038f, 262.435f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc369289a), SkBits2Float(0x408cdc40));  // -233.159f, 4.40189f
path.conicTo(SkBits2Float(0x427b82f4), SkBits2Float(0x432f5193), SkBits2Float(0x42e1eb60), SkBits2Float(0x42b12617), SkBits2Float(0x3f3504f3));  // 62.8779f, 175.319f, 112.96f, 88.5744f, 0.707107f
path.conicTo(SkBits2Float(0x43230aa3), SkBits2Float(0x3fea4196), SkBits2Float(0xc304feb4), SkBits2Float(0xc329162e), SkBits2Float(0x3f3504f3));  // 163.042f, 1.83013f, -132.995f, -169.087f, 0.707107f
path.conicTo(SkBits2Float(0xc3d68405), SkBits2Float(0xc3aa006f), SkBits2Float(0xc3ef8e7f), SkBits2Float(0xc37d4256), SkBits2Float(0x3f3504f3));  // -429.031f, -340.003f, -479.113f, -253.259f, 0.707107f
path.conicTo(SkBits2Float(0xc4044c7c), SkBits2Float(0xc32683cf), SkBits2Float(0xc369289a), SkBits2Float(0x408cdc40), SkBits2Float(0x3f3504f3));  // -529.195f, -166.515f, -233.159f, 4.40189f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc30db26a), SkBits2Float(0xc37fb0d7));  // -141.697f, -255.691f
path.conicTo(SkBits2Float(0xc1c64388), SkBits2Float(0x42830e25), SkBits2Float(0x428aae1e), SkBits2Float(0x41fa2834), SkBits2Float(0x3f3504f3));  // -24.783f, 65.5276f, 69.3401f, 31.2696f, 0.707107f
path.conicTo(SkBits2Float(0x4323768e), SkBits2Float(0xc03f4152), SkBits2Float(0x423a3252), SkBits2Float(0xc3a21a77), SkBits2Float(0x3f3504f3));  // 163.463f, -2.98836f, 46.5491f, -324.207f, 0.707107f
path.conicTo(SkBits2Float(0xc28cbac8), SkBits2Float(0xc4215b37), SkBits2Float(0xc3247ce4), SkBits2Float(0xc418cab4), SkBits2Float(0x3f3504f3));  // -70.3648f, -645.425f, -164.488f, -611.167f, 0.707107f
path.conicTo(SkBits2Float(0xc3814e32), SkBits2Float(0xc4103a31), SkBits2Float(0xc30db26a), SkBits2Float(0xc37fb0d7), SkBits2Float(0x3f3504f3));  // -258.611f, -576.909f, -141.697f, -255.691f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42bf1a33), SkBits2Float(0xc3c6124c));  // 95.5512f, -396.143f
path.conicTo(SkBits2Float(0xc1aae6f8), SkBits2Float(0xc295d95d), SkBits2Float(0x42918542), SkBits2Float(0xc222aa8c), SkBits2Float(0x3f3504f3));  // -21.3628f, -74.9245f, 72.7603f, -40.6665f, 0.707107f
path.conicTo(SkBits2Float(0x4326e221), SkBits2Float(0xc0cd12f4), SkBits2Float(0x438de60c), SkBits2Float(0xc3a3d041), SkBits2Float(0x3f3504f3));  // 166.883f, -6.40856f, 283.797f, -327.627f, 0.707107f
path.conicTo(SkBits2Float(0x43c85b09), SkBits2Float(0xc422361b), SkBits2Float(0x43994b49), SkBits2Float(0xc42ac69e), SkBits2Float(0x3f3504f3));  // 400.711f, -648.845f, 306.588f, -683.103f, 0.707107f
path.conicTo(SkBits2Float(0x43547712), SkBits2Float(0xc4335721), SkBits2Float(0x42bf1a33), SkBits2Float(0xc3c6124c), SkBits2Float(0x3f3504f3));  // 212.465f, -717.361f, 95.5512f, -396.143f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43b7c98f), SkBits2Float(0xc3af9e21));  // 367.575f, -351.235f
path.conicTo(SkBits2Float(0x428f138a), SkBits2Float(0xc3345194), SkBits2Float(0x42f33d6e), SkBits2Float(0xc2bb2617), SkBits2Float(0x3f3504f3));  // 71.5382f, -180.319f, 121.62f, -93.5744f, 0.707107f
path.conicTo(SkBits2Float(0x432bb3a9), SkBits2Float(0xc0da9066), SkBits2Float(0x43e9de81), SkBits2Float(0xc331bf31), SkBits2Float(0x3f3504f3));  // 171.702f, -6.83013f, 467.738f, -177.747f, 0.707107f
path.conicTo(SkBits2Float(0x443ef196), SkBits2Float(0xc3ae54f0), SkBits2Float(0x44326c5a), SkBits2Float(0xc3d9b434), SkBits2Float(0x3f3504f3));  // 763.775f, -348.664f, 713.693f, -435.408f, 0.707107f
path.conicTo(SkBits2Float(0x4425e71e), SkBits2Float(0xc40289bc), SkBits2Float(0x43b7c98f), SkBits2Float(0xc3af9e21), SkBits2Float(0x3f3504f3));  // 663.611f, -522.152f, 367.575f, -351.235f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x4408c5ce), SkBits2Float(0xc30dfb1e));  // 547.091f, -141.981f
path.conicTo(SkBits2Float(0x4352734e), SkBits2Float(0xc34956f5), SkBits2Float(0x43410ea4), SkBits2Float(0xc2cd653e), SkBits2Float(0x3f3504f3));  // 210.45f, -201.34f, 193.057f, -102.698f, 0.707107f
path.conicTo(SkBits2Float(0x432fa9f9), SkBits2Float(0xc081c919), SkBits2Float(0x44001378), SkBits2Float(0x425d363a), SkBits2Float(0x3f3504f3));  // 175.664f, -4.0558f, 512.304f, 55.303f, 0.707107f
path.conicTo(SkBits2Float(0x44543c72), SkBits2Float(0x42e552cb), SkBits2Float(0x4458959e), SkBits2Float(0x4180287c), SkBits2Float(0x3f3504f3));  // 848.944f, 114.662f, 866.338f, 16.0198f, 0.707107f
path.conicTo(SkBits2Float(0x445ceec8), SkBits2Float(0xc2a53e8d), SkBits2Float(0x4408c5ce), SkBits2Float(0xc30dfb1e), SkBits2Float(0x3f3504f3));  // 883.731f, -82.6222f, 547.091f, -141.981f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x44098684), SkBits2Float(0x4305b53c));  // 550.102f, 133.708f
path.conicTo(SkBits2Float(0x43a5300e), SkBits2Float(0xc30026d3), SkBits2Float(0x437da548), SkBits2Float(0xc27f121c), SkBits2Float(0x3f3504f3));  // 330.375f, -128.152f, 253.646f, -63.7677f, 0.707107f
path.conicTo(SkBits2Float(0x4330ea74), SkBits2Float(0x3f1dc4b8), SkBits2Float(0x43c65234), SkBits2Float(0x43833cea), SkBits2Float(0x3f3504f3));  // 176.916f, 0.616283f, 396.642f, 262.476f, 0.707107f
path.conicTo(SkBits2Float(0x441a1798), SkBits2Float(0x44031579), SkBits2Float(0x442d464c), SkBits2Float(0x43e5f9cc), SkBits2Float(0x3f3504f3));  // 616.369f, 524.336f, 693.098f, 459.952f, 0.707107f
path.conicTo(SkBits2Float(0x44407502), SkBits2Float(0x43c5c8a6), SkBits2Float(0x44098684), SkBits2Float(0x4305b53c), SkBits2Float(0x3f3504f3));  // 769.828f, 395.568f, 550.102f, 133.708f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43bb9978), SkBits2Float(0x44487cfb));  // 375.199f, 801.953f
path.conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x43e60f46), SkBits2Float(0x43898486), SkBits2Float(0x43e60f46), SkBits2Float(0x3f3504f3));  // 375.199f, 460.119f, 275.035f, 460.119f, 0.707107f
path.conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x43e60f46), SkBits2Float(0x432edf26), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3));  // 174.872f, 460.119f, 174.872f, 801.953f, 0.707107f
path.conicTo(SkBits2Float(0x432edf26), SkBits2Float(0x448ef92a), SkBits2Float(0x43898486), SkBits2Float(0x448ef92a), SkBits2Float(0x3f3504f3));  // 174.872f, 1143.79f, 275.035f, 1143.79f, 0.707107f
path.conicTo(SkBits2Float(0x43bb9978), SkBits2Float(0x448ef92a), SkBits2Float(0x43bb9978), SkBits2Float(0x44487cfb), SkBits2Float(0x3f3504f3));  // 375.199f, 1143.79f, 375.199f, 801.953f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42d07148), SkBits2Float(0x445532a0));  // 104.221f, 852.791f
path.conicTo(SkBits2Float(0x43a1f94a), SkBits2Float(0x4413bb9c), SkBits2Float(0x437737c0), SkBits2Float(0x4403a309), SkBits2Float(0x3f3504f3));  // 323.948f, 590.931f, 247.218f, 526.547f, 0.707107f
path.conicTo(SkBits2Float(0x432a7ceb), SkBits2Float(0x43e714ed), SkBits2Float(0xc244f418), SkBits2Float(0x4435017b), SkBits2Float(0x3f3504f3));  // 170.488f, 462.163f, -49.2384f, 724.023f, 0.707107f
path.conicTo(SkBits2Float(0xc3867b7b), SkBits2Float(0x4476787f), SkBits2Float(0xc3403c22), SkBits2Float(0x44834889), SkBits2Float(0x3f3504f3));  // -268.965f, 985.883f, -192.235f, 1050.27f, 0.707107f
path.conicTo(SkBits2Float(0xc2e7029c), SkBits2Float(0x448b54d2), SkBits2Float(0x42d07148), SkBits2Float(0x445532a0), SkBits2Float(0x3f3504f3));  // -115.505f, 1114.65f, 104.221f, 852.791f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc30809b4), SkBits2Float(0x44336379));  // -136.038f, 717.554f
path.conicTo(SkBits2Float(0x43489a34), SkBits2Float(0x44248c83), SkBits2Float(0x43373589), SkBits2Float(0x440be36d), SkBits2Float(0x3f3504f3));  // 200.602f, 658.195f, 183.209f, 559.554f, 0.707107f
path.conicTo(SkBits2Float(0x4325d0dd), SkBits2Float(0x43e674af), SkBits2Float(0xc32ad30a), SkBits2Float(0x4402114e), SkBits2Float(0x3f3504f3));  // 165.816f, 460.912f, -170.824f, 520.27f, 0.707107f
path.conicTo(SkBits2Float(0xc3fdbb7b), SkBits2Float(0x4410e844), SkBits2Float(0xc3f50925), SkBits2Float(0x4429915a), SkBits2Float(0x3f3504f3));  // -507.465f, 579.629f, -490.071f, 678.271f, 0.707107f
path.conicTo(SkBits2Float(0xc3ec56cf), SkBits2Float(0x44423a6f), SkBits2Float(0xc30809b4), SkBits2Float(0x44336379), SkBits2Float(0x3f3504f3));  // -472.678f, 776.913f, -136.038f, 717.554f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc369289a), SkBits2Float(0x43e5c2b7));  // -233.159f, 459.521f
path.conicTo(SkBits2Float(0x427b82f4), SkBits2Float(0x441d9c08), SkBits2Float(0x42e1eb60), SkBits2Float(0x4407ec66), SkBits2Float(0x3f3504f3));  // 62.8779f, 630.438f, 112.96f, 543.694f, 0.707107f
path.conicTo(SkBits2Float(0x43230aa3), SkBits2Float(0x43e47988), SkBits2Float(0xc304feb4), SkBits2Float(0x438f042f), SkBits2Float(0x3f3504f3));  // 163.042f, 456.949f, -132.995f, 286.033f, 0.707107f
path.conicTo(SkBits2Float(0xc3d68405), SkBits2Float(0x42e63b5c), SkBits2Float(0xc3ef8e7f), SkBits2Float(0x4349dc36), SkBits2Float(0x3f3504f3));  // -429.031f, 115.116f, -479.113f, 201.86f, 0.707107f
path.conicTo(SkBits2Float(0xc4044c7c), SkBits2Float(0x43904d5e), SkBits2Float(0xc369289a), SkBits2Float(0x43e5c2b7), SkBits2Float(0x3f3504f3));  // -529.195f, 288.604f, -233.159f, 459.521f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0xc30db26a), SkBits2Float(0x43476db5));  // -141.697f, 199.429f
path.conicTo(SkBits2Float(0xc1c64388), SkBits2Float(0x44022968), SkBits2Float(0x428aae1e), SkBits2Float(0x43f331c9), SkBits2Float(0x3f3504f3));  // -24.783f, 520.647f, 69.3401f, 486.389f, 0.707107f
path.conicTo(SkBits2Float(0x4323768e), SkBits2Float(0x43e210c3), SkBits2Float(0x423a3252), SkBits2Float(0x4302e99e), SkBits2Float(0x3f3504f3));  // 163.463f, 452.131f, 46.5491f, 130.913f, 0.707107f
path.conicTo(SkBits2Float(0xc28cbac8), SkBits2Float(0xc33e4e50), SkBits2Float(0xc3247ce4), SkBits2Float(0xc31c0c44), SkBits2Float(0x3f3504f3));  // -70.3648f, -190.306f, -164.488f, -156.048f, 0.707107f
path.conicTo(SkBits2Float(0xc3814e32), SkBits2Float(0xc2f39470), SkBits2Float(0xc30db26a), SkBits2Float(0x43476db5), SkBits2Float(0x3f3504f3));  // -258.611f, -121.79f, -141.697f, 199.429f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42bf1a33), SkBits2Float(0x426be7d0));  // 95.5512f, 58.9764f
path.conicTo(SkBits2Float(0xc1aae6f8), SkBits2Float(0x43be18ef), SkBits2Float(0x42918542), SkBits2Float(0x43cf39f4), SkBits2Float(0x3f3504f3));  // -21.3628f, 380.195f, 72.7603f, 414.453f, 0.707107f
path.conicTo(SkBits2Float(0x4326e221), SkBits2Float(0x43e05afa), SkBits2Float(0x438de60c), SkBits2Float(0x42fefc14), SkBits2Float(0x3f3504f3));  // 166.883f, 448.711f, 283.797f, 127.492f, 0.707107f
path.conicTo(SkBits2Float(0x43c85b09), SkBits2Float(0xc341b9e0), SkBits2Float(0x43994b49), SkBits2Float(0xc363fbec), SkBits2Float(0x3f3504f3));  // 400.711f, -193.726f, 306.588f, -227.984f, 0.707107f
path.conicTo(SkBits2Float(0x43547712), SkBits2Float(0xc3831efc), SkBits2Float(0x42bf1a33), SkBits2Float(0x426be7d0), SkBits2Float(0x3f3504f3));  // 212.465f, -262.242f, 95.5512f, 58.9764f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43b7c98f), SkBits2Float(0x42cfc494));  // 367.575f, 103.884f
path.conicTo(SkBits2Float(0x428f138a), SkBits2Float(0x4389667c), SkBits2Float(0x42f33d6e), SkBits2Float(0x43b4c5c0), SkBits2Float(0x3f3504f3));  // 71.5382f, 274.801f, 121.62f, 361.545f, 0.707107f
path.conicTo(SkBits2Float(0x432bb3a9), SkBits2Float(0x43e02504), SkBits2Float(0x43e9de81), SkBits2Float(0x438aafae), SkBits2Float(0x3f3504f3));  // 171.702f, 448.289f, 467.738f, 277.372f, 0.707107f
path.conicTo(SkBits2Float(0x443ef196), SkBits2Float(0x42d4e958), SkBits2Float(0x44326c5a), SkBits2Float(0x419db120), SkBits2Float(0x3f3504f3));  // 763.775f, 106.456f, 713.693f, 19.7115f, 0.707107f
path.conicTo(SkBits2Float(0x4425e71e), SkBits2Float(0xc28610c8), SkBits2Float(0x43b7c98f), SkBits2Float(0x42cfc494), SkBits2Float(0x3f3504f3));  // 663.611f, -67.0328f, 367.575f, 103.884f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x4408c5ce), SkBits2Float(0x439c91b7));  // 547.091f, 313.138f
path.conicTo(SkBits2Float(0x4352734e), SkBits2Float(0x437dc797), SkBits2Float(0x43410ea4), SkBits2Float(0x43b035f6), SkBits2Float(0x3f3504f3));  // 210.45f, 253.78f, 193.057f, 352.422f, 0.707107f
path.conicTo(SkBits2Float(0x432fa9f9), SkBits2Float(0x43e18822), SkBits2Float(0x44001378), SkBits2Float(0x43ff360d), SkBits2Float(0x3f3504f3));  // 175.664f, 451.064f, 512.304f, 510.422f, 0.707107f
path.conicTo(SkBits2Float(0x44543c72), SkBits2Float(0x440e71fc), SkBits2Float(0x4458959e), SkBits2Float(0x43eb91ce), SkBits2Float(0x3f3504f3));  // 848.944f, 569.781f, 866.338f, 471.139f, 0.707107f
path.conicTo(SkBits2Float(0x445ceec8), SkBits2Float(0x43ba3fa3), SkBits2Float(0x4408c5ce), SkBits2Float(0x439c91b7), SkBits2Float(0x3f3504f3));  // 883.731f, 372.497f, 547.091f, 313.138f, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x44098684), SkBits2Float(0x441334f2));  // 550.102f, 588.827f
path.conicTo(SkBits2Float(0x43a5300e), SkBits2Float(0x43a37bdc), SkBits2Float(0x437da548), SkBits2Float(0x43c3ad02), SkBits2Float(0x3f3504f3));  // 330.375f, 326.968f, 253.646f, 391.352f, 0.707107f
path.conicTo(SkBits2Float(0x4330ea74), SkBits2Float(0x43e3de28), SkBits2Float(0x43c65234), SkBits2Float(0x44336618), SkBits2Float(0x3f3504f3));  // 176.916f, 455.736f, 396.642f, 717.595f, 0.707107f
path.conicTo(SkBits2Float(0x441a1798), SkBits2Float(0x4474dd1c), SkBits2Float(0x442d464c), SkBits2Float(0x4464c489), SkBits2Float(0x3f3504f3));  // 616.369f, 979.455f, 693.098f, 915.071f, 0.707107f
path.conicTo(SkBits2Float(0x44407502), SkBits2Float(0x4454abf6), SkBits2Float(0x44098684), SkBits2Float(0x441334f2), SkBits2Float(0x3f3504f3));  // 769.828f, 850.687f, 550.102f, 588.827f, 0.707107f
path.close();
SkPath lastFailed = path;
#if DEBUG_SIMPLIFY_FAILS
    for (;;) {
        SkPath failed = subset_simplify_fail(reporter, lastFailed);
        if (failed == lastFailed) {
            break;
        }
        lastFailed = failed;
    }
#endif
    testSimplify(reporter, lastFailed, __FUNCTION__);
}

DEF_TEST(SixtyOvals_2_2_9_73_reduced, reporter) {
    SkPath path;
path.moveTo(377.219f, -141.981f);
path.conicTo(40.5787f, -201.34f, 23.1855f, -102.698f, 0.707107f);
path.lineTo(377.219f, -141.981f);
path.close();
path.moveTo(306.588f, -227.984f);
path.conicTo(212.465f, -262.242f, 95.5512f, 58.9764f, 0.707107f);
path.lineTo(306.588f, -227.984f);
path.close();
testSimplify(reporter, path, __FUNCTION__);
}

DEF_TEST(SixtyOvalsA, reporter) {
SkPath path;
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(11.1722f, -8.10398f);
path.conicTo(22.9143f, -10.3787f, 23.7764f, -7.72542f, 1.00863f);
path.conicTo(24.6671f, -4.98406f, 13.8147f, 0.0166066f, 0.973016f);
path.conicTo(24.6378f, 5.07425f, 23.7764f, 7.72542f, 1.00888f);
path.conicTo(22.8777f, 10.4915f, 11.1648f, 8.13034f, 0.960143f);
path.conicTo(16.9503f, 18.5866f, 14.6946f, 20.2254f, 1.00881f);
path.conicTo(12.4417f, 21.8623f, 4.29722f, 13.1468f, 1.0092f);
path.conicTo(2.92708f, 25, 0, 25, 0.955692f);
path.conicTo(-2.79361f, 25, -4.258f, 13.1048f, 1.00818f);
path.conicTo(-4.27813f, 13.1264f, -4.29822f, 13.1479f, 1.03158f);
path.conicTo(-12.44f, 21.8635f, -14.6946f, 20.2254f, 1.00811f);
path.conicTo(-16.9933f, 18.5554f, -11.1722f, 8.10398f, 0.989875f);
path.conicTo(-22.9143f, 10.3787f, -23.7764f, 7.72542f, 1.00863f);
path.conicTo(-24.6671f, 4.98406f, -13.8147f, -0.0166066f, 0.973016f);
path.conicTo(-24.6378f, -5.07425f, -23.7764f, -7.72542f, 1.00888f);
path.conicTo(-22.8777f, -10.4915f, -11.1648f, -8.13034f, 0.960143f);
path.conicTo(-16.9503f, -18.5866f, -14.6946f, -20.2254f, 1.00881f);
path.conicTo(-12.4417f, -21.8623f, -4.29722f, -13.1468f, 1.0092f);
path.conicTo(-2.92708f, -25, 0, -25, 0.955692f);
path.conicTo(2.79361f, -25, 4.258f, -13.1048f, 1.00818f);
path.conicTo(4.27813f, -13.1264f, 4.29822f, -13.1479f, 1.03158f);
path.conicTo(12.44f, -21.8635f, 14.6946f, -20.2254f, 1.00811f);
path.conicTo(16.9933f, -18.5554f, 11.1722f, -8.10398f, 0.989875f);
path.close();
SkPath one(path);
path.reset();
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(-1.54509f, -4.75528f);
path.conicTo(22.2313f, -12.4807f, 23.7764f, -7.72543f, 0.707107f);
path.conicTo(25.3215f, -2.97014f, 1.54509f, 4.75528f, 0.707107f);
path.conicTo(-22.2313f, 12.4807f, -23.7764f, 7.72543f, 0.707107f);
path.conicTo(-25.3215f, 2.97014f, -1.54509f, -4.75528f, 0.707107f);
path.close();
SkPath two(path);
SkPath result;
Op(one, two, kUnion_SkPathOp, &result);
}

DEF_TEST(SixtyOvalsAX, reporter) {
SkPath path;
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0x4132c174), SkBits2Float(0xc101a9e5));  // 11.1722f, -8.10398f
path.conicTo(SkBits2Float(0x41b7508a), SkBits2Float(0xc1260efe), SkBits2Float(0x41be3618), SkBits2Float(0xc0f736ad), SkBits2Float(0x3f811abd));  // 22.9143f, -10.3787f, 23.7764f, -7.72542f, 1.00863f
path.conicTo(SkBits2Float(0x41c5564b), SkBits2Float(0xc09f7d6d), SkBits2Float(0x415d0934), SkBits2Float(0x3c880a93), SkBits2Float(0x3f79179a));  // 24.6671f, -4.98406f, 13.8147f, 0.0166066f, 0.973016f
path.conicTo(SkBits2Float(0x41c51a48), SkBits2Float(0x40a2603c), SkBits2Float(0x41be3618), SkBits2Float(0x40f736ac), SkBits2Float(0x3f8122f3));  // 24.6378f, 5.07425f, 23.7764f, 7.72542f, 1.00888f
path.conicTo(SkBits2Float(0x41b7056f), SkBits2Float(0x4127dd49), SkBits2Float(0x4132a328), SkBits2Float(0x410215e1), SkBits2Float(0x3f75cbec));  // 22.8777f, 10.4915f, 11.1648f, 8.13034f, 0.960143f
path.conicTo(SkBits2Float(0x41879a3b), SkBits2Float(0x4194b151), SkBits2Float(0x416b1d34), SkBits2Float(0x41a1cdac), SkBits2Float(0x3f8120d4));  // 16.9503f, 18.5866f, 14.6946f, 20.2254f, 1.00881f
path.conicTo(SkBits2Float(0x41471107), SkBits2Float(0x41aee601), SkBits2Float(0x408982d1), SkBits2Float(0x41525939), SkBits2Float(0x3f812d7f));  // 12.4417f, 21.8623f, 4.29722f, 13.1468f, 1.0092f
path.conicTo(SkBits2Float(0x403b5543), SkBits2Float(0x41c80000), SkBits2Float(0x00000000), SkBits2Float(0x41c80000), SkBits2Float(0x3f74a837));  // 2.92708f, 25, 0, 25, 0.955692f
path.conicTo(SkBits2Float(0xc032ca93), SkBits2Float(0x41c80000), SkBits2Float(0xc088418e), SkBits2Float(0x4151ad32), SkBits2Float(0x3f810c2d));  // -2.79361f, 25, -4.258f, 13.1048f, 1.00818f
path.conicTo(SkBits2Float(0xc088e66c), SkBits2Float(0x4152058a), SkBits2Float(0xc0898afc), SkBits2Float(0x41525d9e), SkBits2Float(0x3f840adb));  // -4.27813f, 13.1264f, -4.29822f, 13.1479f, 1.03158f
path.conicTo(SkBits2Float(0xc1470a56), SkBits2Float(0x41aee870), SkBits2Float(0xc16b1d36), SkBits2Float(0x41a1cdac), SkBits2Float(0x3f81099f));  // -12.44f, 21.8635f, -14.6946f, 20.2254f, 1.00811f
path.conicTo(SkBits2Float(0xc187f23a), SkBits2Float(0x41947162), SkBits2Float(0xc132c174), SkBits2Float(0x4101a9e5), SkBits2Float(0x3f7d6873));  // -16.9933f, 18.5554f, -11.1722f, 8.10398f, 0.989875f
path.conicTo(SkBits2Float(0xc1b7508a), SkBits2Float(0x41260efe), SkBits2Float(0xc1be3618), SkBits2Float(0x40f736ad), SkBits2Float(0x3f811abd));  // -22.9143f, 10.3787f, -23.7764f, 7.72542f, 1.00863f
path.conicTo(SkBits2Float(0xc1c5564b), SkBits2Float(0x409f7d6d), SkBits2Float(0xc15d0934), SkBits2Float(0xbc880a93), SkBits2Float(0x3f79179a));  // -24.6671f, 4.98406f, -13.8147f, -0.0166066f, 0.973016f
path.conicTo(SkBits2Float(0xc1c51a48), SkBits2Float(0xc0a2603c), SkBits2Float(0xc1be3618), SkBits2Float(0xc0f736ac), SkBits2Float(0x3f8122f3));  // -24.6378f, -5.07425f, -23.7764f, -7.72542f, 1.00888f
path.conicTo(SkBits2Float(0xc1b7056f), SkBits2Float(0xc127dd49), SkBits2Float(0xc132a328), SkBits2Float(0xc10215e1), SkBits2Float(0x3f75cbec));  // -22.8777f, -10.4915f, -11.1648f, -8.13034f, 0.960143f
path.conicTo(SkBits2Float(0xc1879a3b), SkBits2Float(0xc194b151), SkBits2Float(0xc16b1d34), SkBits2Float(0xc1a1cdac), SkBits2Float(0x3f8120d4));  // -16.9503f, -18.5866f, -14.6946f, -20.2254f, 1.00881f
path.conicTo(SkBits2Float(0xc1471107), SkBits2Float(0xc1aee601), SkBits2Float(0xc08982d1), SkBits2Float(0xc1525939), SkBits2Float(0x3f812d7f));  // -12.4417f, -21.8623f, -4.29722f, -13.1468f, 1.0092f
path.conicTo(SkBits2Float(0xc03b5543), SkBits2Float(0xc1c80000), SkBits2Float(0x00000000), SkBits2Float(0xc1c80000), SkBits2Float(0x3f74a837));  // -2.92708f, -25, 0, -25, 0.955692f
path.conicTo(SkBits2Float(0x4032ca93), SkBits2Float(0xc1c80000), SkBits2Float(0x4088418e), SkBits2Float(0xc151ad32), SkBits2Float(0x3f810c2d));  // 2.79361f, -25, 4.258f, -13.1048f, 1.00818f
path.conicTo(SkBits2Float(0x4088e66c), SkBits2Float(0xc152058a), SkBits2Float(0x40898afc), SkBits2Float(0xc1525d9e), SkBits2Float(0x3f840adb));  // 4.27813f, -13.1264f, 4.29822f, -13.1479f, 1.03158f
path.conicTo(SkBits2Float(0x41470a56), SkBits2Float(0xc1aee870), SkBits2Float(0x416b1d36), SkBits2Float(0xc1a1cdac), SkBits2Float(0x3f81099f));  // 12.44f, -21.8635f, 14.6946f, -20.2254f, 1.00811f
path.conicTo(SkBits2Float(0x4187f23a), SkBits2Float(0xc1947162), SkBits2Float(0x4132c174), SkBits2Float(0xc101a9e5), SkBits2Float(0x3f7d6873));  // 16.9933f, -18.5554f, 11.1722f, -8.10398f, 0.989875f
path.close();
path.close();
SkPath one(path);
path.reset();
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0xbfc5c55c), SkBits2Float(0xc0982b46));  // -1.54509f, -4.75528f
path.conicTo(SkBits2Float(0x41b1d9c2), SkBits2Float(0xc147b0fc), SkBits2Float(0x41be3618), SkBits2Float(0xc0f736b3), SkBits2Float(0x3f3504f3));  // 22.2313f, -12.4807f, 23.7764f, -7.72543f, 0.707107f
path.conicTo(SkBits2Float(0x41ca926e), SkBits2Float(0xc03e16da), SkBits2Float(0x3fc5c55c), SkBits2Float(0x40982b46), SkBits2Float(0x3f3504f3));  // 25.3215f, -2.97014f, 1.54509f, 4.75528f, 0.707107f
path.conicTo(SkBits2Float(0xc1b1d9c2), SkBits2Float(0x4147b0fc), SkBits2Float(0xc1be3618), SkBits2Float(0x40f736b3), SkBits2Float(0x3f3504f3));  // -22.2313f, 12.4807f, -23.7764f, 7.72543f, 0.707107f
path.conicTo(SkBits2Float(0xc1ca926e), SkBits2Float(0x403e16da), SkBits2Float(0xbfc5c55c), SkBits2Float(0xc0982b46), SkBits2Float(0x3f3504f3));  // -25.3215f, 2.97014f, -1.54509f, -4.75528f, 0.707107f
path.close();
SkPath two(path);
SkPath result;
Op(one, two, kUnion_SkPathOp, &result);
}

const char ovalsAsQuads[] = "M 146.4187316894531 136.5"
" Q 146.4187316894531 139.8508911132812 146.4066772460938 143.19775390625"
" Q 146.3946533203125 146.5446166992188 146.3705749511719 149.8793640136719"
" Q 146.3465270996094 153.214111328125 146.3104858398438 156.5287170410156"
" Q 146.2744750976562 159.8433227539062 146.2265930175781 163.1298217773438"
" Q 146.1786804199219 166.4163208007812 146.1190490722656 169.6668090820312"
" Q 146.0593872070312 172.9172973632812 145.9881286621094 176.1238708496094"
" Q 145.9168701171875 179.3304443359375 145.8341674804688 182.4854736328125"
" Q 145.75146484375 185.6404418945312 145.6575317382812 188.7362670898438"
" Q 145.5635681152344 191.8320922851562 145.4586181640625 194.8612365722656"
" Q 145.3536682128906 197.8904113769531 145.2379455566406 200.8456420898438"
" Q 145.1222229003906 203.8008422851562 144.9960021972656 206.6750183105469"
" Q 144.8698120117188 209.5491638183594 144.7334289550781 212.3353271484375"
" Q 144.5970458984375 215.1214599609375 144.4508056640625 217.8129272460938"
" Q 144.3045654296875 220.50439453125 144.1488342285156 223.0946655273438"
" Q 143.9931030273438 225.6849365234375 143.8282470703125 228.1677856445312"
" Q 143.6633911132812 230.650634765625 143.4898071289062 233.0200805664062"
" Q 143.3162231445312 235.3894958496094 143.1343078613281 237.6398315429688"
" Q 142.9524230957031 239.89013671875 142.7626647949219 242.0159301757812"
" Q 142.5729064941406 244.1417236328125 142.375732421875 246.1378173828125"
" Q 142.1785583496094 248.1339111328125 141.9744262695312 249.99560546875"
" Q 141.7703247070312 251.8572387695312 141.5597534179688 253.5799255371094"
" Q 141.3492126464844 255.3026428222656 141.1326904296875 256.8822326660156"
" Q 140.9161987304688 258.4617919921875 140.6942749023438 259.8945007324219"
" Q 140.4723510742188 261.3271789550781 140.2455749511719 262.6095275878906"
" Q 140.0187683105469 263.8918762207031 139.7876281738281 265.020751953125"
" Q 139.5564880371094 266.1496276855469 139.3215637207031 267.1223449707031"
" Q 139.086669921875 268.0950622558594 138.8485412597656 268.9092712402344"
" Q 138.6104125976562 269.7234497070312 138.36962890625 270.3771667480469"
" Q 138.1288757324219 271.0308837890625 137.8860473632812 271.5225830078125"
" Q 137.6432189941406 272.0142822265625 137.3988952636719 272.3427124023438"
" Q 137.1546020507812 272.6711730957031 136.9093933105469 272.8355712890625"
" Q 136.6642150878906 272.9999694824219 136.4187316894531 273"
" Q 136.1732482910156 272.9999694824219 135.9280700683594 272.8355712890625"
" Q 135.682861328125 272.6711730957031 135.4385681152344 272.3427124023438"
" Q 135.1942443847656 272.0142822265625 134.951416015625 271.5225830078125"
" Q 134.7085876464844 271.0308837890625 134.4678344726562 270.3771667480469"
" Q 134.22705078125 269.7234497070312 133.9889221191406 268.9092407226562"
" Q 133.7507934570312 268.0950622558594 133.5158996582031 267.122314453125"
" Q 133.2809753417969 266.1495971679688 133.0498352050781 265.020751953125"
" Q 132.8186950683594 263.8918762207031 132.5918884277344 262.6095275878906"
" Q 132.3651123046875 261.3271789550781 132.1431884765625 259.8945007324219"
" Q 131.9212646484375 258.4617919921875 131.7047729492188 256.8822326660156"
" Q 131.4882507324219 255.3026428222656 131.2777099609375 253.5799560546875"
" Q 131.067138671875 251.8572387695312 130.863037109375 249.99560546875"
" Q 130.6589050292969 248.1339111328125 130.4617309570312 246.1378173828125"
" Q 130.2645568847656 244.1417236328125 130.0747985839844 242.0159301757812"
" Q 129.8850402832031 239.89013671875 129.7031555175781 237.6398315429688"
" Q 129.521240234375 235.3894958496094 129.34765625 233.0200805664062"
" Q 129.174072265625 230.650634765625 129.0092163085938 228.1677856445312"
" Q 128.8443603515625 225.6849365234375 128.6886291503906 223.0946655273438"
" Q 128.5328979492188 220.50439453125 128.3866577148438 217.8129272460938"
" Q 128.2404174804688 215.1214599609375 128.1040344238281 212.3353271484375"
" Q 127.9676513671875 209.5491333007812 127.8414306640625 206.6749877929688"
" Q 127.7152404785156 203.8008422851562 127.5995178222656 200.8456420898438"
" Q 127.4837951660156 197.8904113769531 127.3788452148438 194.8612365722656"
" Q 127.2738647460938 191.8320922851562 127.179931640625 188.7362670898438"
" Q 127.0859985351562 185.6404418945312 127.0032958984375 182.4854431152344"
" Q 126.9205932617188 179.3304443359375 126.8493347167969 176.1238708496094"
" Q 126.778076171875 172.9172973632812 126.7184143066406 169.6668090820312"
" Q 126.6587829589844 166.4163208007812 126.6108703613281 163.1298217773438"
" Q 126.56298828125 159.8433227539062 126.5269775390625 156.5287170410156"
" Q 126.4909362792969 153.214111328125 126.4668884277344 149.8793640136719"
" Q 126.4428100585938 146.5446166992188 126.4307861328125 143.19775390625"
" Q 126.4187316894531 139.8508911132812 126.4187316894531 136.5"
" Q 126.4187316894531 133.1491088867188 126.4307861328125 129.8022613525391"
" Q 126.4428100585938 126.4554138183594 126.4668884277344 123.1206665039062"
" Q 126.4909362792969 119.7859039306641 126.5269775390625 116.4712829589844"
" Q 126.56298828125 113.1566619873047 126.6108703613281 109.8701629638672"
" Q 126.6587829589844 106.5836639404297 126.7184143066406 103.3331909179688"
" Q 126.778076171875 100.0827331542969 126.8493347167969 96.87612915039062"
" Q 126.9205932617188 93.66952514648438 127.0032958984375 90.5145263671875"
" Q 127.0859985351562 87.35951232910156 127.179931640625 84.26370239257812"
" Q 127.2738647460938 81.16787719726562 127.3788452148438 78.13871765136719"
" Q 127.4837951660156 75.10955810546875 127.5995178222656 72.15434265136719"
" Q 127.7152404785156 69.19912719726562 127.8414306640625 66.32498168945312"
" Q 127.9676513671875 63.45082092285156 128.1040344238281 60.66465759277344"
" Q 128.2404174804688 57.87849426269531 128.3866577148438 55.18704223632812"
" Q 128.5328979492188 52.49559020996094 128.6886291503906 49.90530395507812"
" Q 128.8443603515625 47.31504821777344 129.0092163085938 44.83219909667969"
" Q 129.174072265625 42.349365234375 129.34765625 39.97991943359375"
" Q 129.521240234375 37.61048889160156 129.7031555175781 35.36016845703125"
" Q 129.8850402832031 33.10984802246094 130.0747985839844 30.98406982421875"
" Q 130.2645568847656 28.85829162597656 130.4617309570312 26.86216735839844"
" Q 130.6589050292969 24.86604309082031 130.863037109375 23.00439453125"
" Q 131.067138671875 21.14274597167969 131.2777099609375 19.4200439453125"
" Q 131.4882507324219 17.69734191894531 131.7047729492188 16.11775207519531"
" Q 131.9212646484375 14.53814697265625 132.1431884765625 13.10545349121094"
" Q 132.3651123046875 11.67277526855469 132.5918884277344 10.39044189453125"
" Q 132.8186950683594 9.108108520507812 133.0498352050781 7.979232788085938"
" Q 133.2809753417969 6.850357055664062 133.5158996582031 5.877639770507812"
" Q 133.7507934570312 4.904937744140625 133.9889221191406 4.090728759765625"
" Q 134.22705078125 3.276535034179688 134.4678344726562 2.622810363769531"
" Q 134.7085876464844 1.969085693359375 134.951416015625 1.477409362792969"
" Q 135.1942443847656 0.9857254028320312 135.4385681152344 0.65728759765625"
" Q 135.682861328125 0.3288421630859375 135.9280700683594 0.1644210815429688"
" Q 136.1732482910156 0 136.4187316894531 0"
" Q 136.6642150878906 0 136.9093933105469 0.1644210815429688"
" Q 137.1546020507812 0.3288421630859375 137.3988952636719 0.65728759765625"
" Q 137.6432189941406 0.9857254028320312 137.8860473632812 1.477409362792969"
" Q 138.1288757324219 1.969085693359375 138.36962890625 2.622810363769531"
" Q 138.6104125976562 3.276535034179688 138.8485412597656 4.090728759765625"
" Q 139.086669921875 4.904937744140625 139.3215637207031 5.877639770507812"
" Q 139.5564880371094 6.850357055664062 139.7876281738281 7.979232788085938"
" Q 140.0187683105469 9.108108520507812 140.2455749511719 10.39044189453125"
" Q 140.4723510742188 11.67277526855469 140.6942749023438 13.10545349121094"
" Q 140.9161987304688 14.53814697265625 141.1326904296875 16.11775207519531"
" Q 141.3492126464844 17.69734191894531 141.5597534179688 19.4200439453125"
" Q 141.7703247070312 21.14274597167969 141.9744262695312 23.00439453125"
" Q 142.1785583496094 24.86604309082031 142.375732421875 26.86216735839844"
" Q 142.5729064941406 28.85829162597656 142.7626647949219 30.98406982421875"
" Q 142.9524230957031 33.10984802246094 143.1343078613281 35.36016845703125"
" Q 143.3162231445312 37.61048889160156 143.4898071289062 39.97991943359375"
" Q 143.6633911132812 42.349365234375 143.8282470703125 44.83219909667969"
" Q 143.9931030273438 47.31504821777344 144.1488342285156 49.90531921386719"
" Q 144.3045654296875 52.49559020996094 144.4508056640625 55.18704223632812"
" Q 144.5970458984375 57.87849426269531 144.7334289550781 60.66465759277344"
" Q 144.8698120117188 63.45082092285156 144.9960021972656 66.32498168945312"
" Q 145.1222229003906 69.19912719726562 145.2379455566406 72.15434265136719"
" Q 145.3536682128906 75.10955810546875 145.4586181640625 78.13871765136719"
" Q 145.5635681152344 81.16787719726562 145.6575317382812 84.26370239257812"
" Q 145.75146484375 87.35951232910156 145.8341674804688 90.5145263671875"
" Q 145.9168701171875 93.66952514648438 145.9881286621094 96.87614440917969"
" Q 146.0593872070312 100.0827331542969 146.1190490722656 103.3332061767578"
" Q 146.1786804199219 106.5836639404297 146.2265930175781 109.8701629638672"
" Q 146.2744750976562 113.1566619873047 146.3104858398438 116.4712829589844"
" Q 146.3465270996094 119.785888671875 146.3705749511719 123.1206665039062"
" Q 146.3946533203125 126.4554138183594 146.4066772460938 129.8022613525391"
" Q 146.4187316894531 133.1491088867188 146.4187316894531 136.5 Z";

#include "SkParsePath.h"

DEF_TEST(PathOpsOvalsAsQuads, reporter) {
    return; // don't execute this for now
    SkPath path;
    SkParsePath::FromSVGString(ovalsAsQuads, &path);
    Simplify(path, &path);
}

DEF_TEST(PathOps64OvalsAsQuads, reporter) {
    return; // don't execute this for now
    SkPath path, result;
    SkOpBuilder builder;
    SkParsePath::FromSVGString(ovalsAsQuads, &path);
    OvalSet set = {{0, 0, 0, 0}, 2, 3, 9, 100, 100};
    testOvalSet(set, path, &builder, nullptr, &result);
}
