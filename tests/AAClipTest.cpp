/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkAAClip.h"
#include "SkPath.h"
#include "SkRandom.h"

static const SkRegion::Op gRgnOps[] = {
//    SkRegion::kDifference_Op,
    SkRegion::kIntersect_Op,
    SkRegion::kUnion_Op,
    SkRegion::kXOR_Op,
//    SkRegion::kReverseDifference_Op,
    SkRegion::kReplace_Op
};

static const char* gRgnOpNames[] = {
//    "DIFF",
    "SECT", "UNION", "XOR",
//    "RDIFF",
    "REPLACE"
};

static void imoveTo(SkPath& path, int x, int y) {
    path.moveTo(SkIntToScalar(x), SkIntToScalar(y));
}

static void icubicTo(SkPath& path, int x0, int y0, int x1, int y1, int x2, int y2) {
    path.cubicTo(SkIntToScalar(x0), SkIntToScalar(y0),
                 SkIntToScalar(x1), SkIntToScalar(y1),
                 SkIntToScalar(x2), SkIntToScalar(y2));
}

static void test_path_bounds(skiatest::Reporter* reporter) {
    SkPath path;
    SkAAClip clip;
    const int height = 40;
    const SkScalar sheight = SkIntToScalar(height);

    path.addOval(SkRect::MakeWH(sheight, sheight));
    REPORTER_ASSERT(reporter, sheight == path.getBounds().height());
    clip.setPath(path, NULL, true);
    REPORTER_ASSERT(reporter, height == clip.getBounds().height());

    // this is the trimmed height of this cubic (with aa). The critical thing
    // for this test is that it is less than height, which represents just
    // the bounds of the path's control-points.
    //
    // This used to fail until we tracked the MinY in the BuilderBlitter.
    //
    const int teardrop_height = 12;
    path.reset();
    imoveTo(path, 0, 20);
    icubicTo(path, 40, 40, 40, 0, 0, 20);
    REPORTER_ASSERT(reporter, sheight == path.getBounds().height());
    clip.setPath(path, NULL, true);
    REPORTER_ASSERT(reporter, teardrop_height == clip.getBounds().height());
}

static void test_empty(skiatest::Reporter* reporter) {
    SkAAClip clip0, clip1;

    REPORTER_ASSERT(reporter, clip0.isEmpty());
    REPORTER_ASSERT(reporter, clip0.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, clip1 == clip0);

    clip0.translate(10, 10);    // should have no effect on empty
    REPORTER_ASSERT(reporter, clip0.isEmpty());
    REPORTER_ASSERT(reporter, clip0.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, clip1 == clip0);

    SkIRect r = { 10, 10, 40, 50 };
    clip0.setRect(r);
    REPORTER_ASSERT(reporter, !clip0.isEmpty());
    REPORTER_ASSERT(reporter, !clip0.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, clip0 != clip1);
    REPORTER_ASSERT(reporter, clip0.getBounds() == r);

    clip0.setEmpty();
    REPORTER_ASSERT(reporter, clip0.isEmpty());
    REPORTER_ASSERT(reporter, clip0.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, clip1 == clip0);

    SkMask mask;
    mask.fImage = NULL;
    clip0.copyToMask(&mask);
    REPORTER_ASSERT(reporter, NULL == mask.fImage);
    REPORTER_ASSERT(reporter, mask.fBounds.isEmpty());
}

static void rand_irect(SkIRect* r, int N, SkRandom& rand) {
    r->setXYWH(0, 0, rand.nextU() % N, rand.nextU() % N);
    int dx = rand.nextU() % (2*N);
    int dy = rand.nextU() % (2*N);
    // use int dx,dy to make the subtract be signed
    r->offset(N - dx, N - dy);
}

static void test_irect(skiatest::Reporter* reporter) {
    SkRandom rand;

    for (int i = 0; i < 100; i++) {
        SkAAClip clip0, clip1;
        SkRegion rgn0, rgn1;
        SkIRect r0, r1;

        rand_irect(&r0, 10, rand);
        rand_irect(&r1, 10, rand);
        clip0.setRect(r0);
        clip1.setRect(r1);
        rgn0.setRect(r0);
        rgn1.setRect(r1);
        for (size_t j = 0; j < SK_ARRAY_COUNT(gRgnOps); ++j) {
            SkRegion::Op op = gRgnOps[j];
            SkAAClip clip2;
            SkRegion rgn2;
            bool nonEmptyAA = clip2.op(clip0, clip1, op);
            bool nonEmptyBW = rgn2.op(rgn0, rgn1, op);
            if (nonEmptyAA != nonEmptyBW || clip2.getBounds() != rgn2.getBounds()) {
                SkDebugf("[%d %d %d %d] %s [%d %d %d %d] = BW:[%d %d %d %d] AA:[%d %d %d %d]\n",
                         r0.fLeft, r0.fTop, r0.right(), r0.bottom(),
                         gRgnOpNames[j],
                         r1.fLeft, r1.fTop, r1.right(), r1.bottom(),
                         rgn2.getBounds().fLeft, rgn2.getBounds().fTop,
                         rgn2.getBounds().right(), rgn2.getBounds().bottom(),
                         clip2.getBounds().fLeft, clip2.getBounds().fTop,
                         clip2.getBounds().right(), clip2.getBounds().bottom());
            }
            REPORTER_ASSERT(reporter, nonEmptyAA == nonEmptyBW);
            REPORTER_ASSERT(reporter, clip2.getBounds() == rgn2.getBounds());
        }
    }
}

static void TestAAClip(skiatest::Reporter* reporter) {
    test_empty(reporter);
    test_path_bounds(reporter);
    test_irect(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("AAClip", AAClipTestClass, TestAAClip)
