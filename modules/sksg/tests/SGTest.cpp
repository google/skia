/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if !defined(SK_BUILD_FOR_GOOGLE3)

#include "SkRect.h"
#include "SkRectPriv.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGroup.h"
#include "SkSGInvalidationController.h"
#include "SkSGRect.h"
#include "SkSGTransform.h"
#include "SkTo.h"

#include "Test.h"

#include <vector>

static void check_inval(skiatest::Reporter* reporter, const sk_sp<sksg::Node>& root,
                        const SkRect& expected_bounds,
                        const SkRect& expected_inval_bounds,
                        const std::vector<SkRect>* expected_damage) {
    sksg::InvalidationController ic;
    const auto bbox = root->revalidate(&ic, SkMatrix::I());

    if (0) {
        SkDebugf("** bbox: [%f %f %f %f], ibbox: [%f %f %f %f]\n",
                 bbox.fLeft, bbox.fTop, bbox.fRight, bbox.fBottom,
                 ic.bounds().left(), ic.bounds().top(), ic.bounds().right(), ic.bounds().bottom());
    }

    REPORTER_ASSERT(reporter, bbox == expected_bounds);
    REPORTER_ASSERT(reporter, ic.bounds() == expected_inval_bounds);

    if (expected_damage) {
        const auto damage_count = SkTo<size_t>(ic.end() - ic.begin());
        REPORTER_ASSERT(reporter, expected_damage->size() == damage_count);
        for (size_t i = 0; i < std::min(expected_damage->size(), damage_count); ++i) {
            const auto r1 = (*expected_damage)[i],
                       r2 = ic.begin()[i];
            if (0) {
                SkDebugf("*** expected inval: [%f %f %f %f], actual: [%f %f %f %f]\n",
                         r1.left(), r1.top(), r1.right(), r1.bottom(),
                         r2.left(), r2.top(), r2.right(), r2.bottom());
            }
            REPORTER_ASSERT(reporter, r1 == r2);
        }
    }
}

static void inval_test1(skiatest::Reporter* reporter) {
    auto color  = sksg::Color::Make(0xff000000);
    auto r1     = sksg::Rect::Make(SkRect::MakeWH(100, 100)),
         r2     = sksg::Rect::Make(SkRect::MakeWH(100, 100));
    auto grp    = sksg::Group::Make();
    auto matrix = sksg::Matrix::Make(SkMatrix::I());
    auto root   = sksg::Transform::Make(grp, matrix);

    grp->addChild(sksg::Draw::Make(r1, color));
    grp->addChild(sksg::Draw::Make(r2, color));

    {
        // Initial revalidation.
        check_inval(reporter, root,
                    SkRect::MakeWH(100, 100),
                    SkRectPriv::MakeLargeS32(),
                    nullptr);
    }

    {
        // Move r2 to (200 100).
        r2->setL(200); r2->setT(100); r2->setR(300); r2->setB(200);
        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 200, 100, 300, 200} };
        check_inval(reporter, root,
                    SkRect::MakeWH(300, 200),
                    SkRect::MakeWH(300, 200),
                    &damage);
    }

    {
        // Update the common color.
        color->setColor(0xffff0000);
        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 200, 100, 300, 200} };
        check_inval(reporter, root,
                    SkRect::MakeWH(300, 200),
                    SkRect::MakeWH(300, 200),
                    &damage);
    }

    {
        // Shrink r1.
        r1->setR(50);
        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 0, 0, 50, 100} };
        check_inval(reporter, root,
                    SkRect::MakeWH(300, 200),
                    SkRect::MakeWH(100, 100),
                    &damage);
    }

    {
        // Update transform.
        matrix->setMatrix(SkMatrix::MakeScale(2, 2));
        std::vector<SkRect> damage = { {0, 0, 300, 200}, { 0, 0, 600, 400} };
        check_inval(reporter, root,
                    SkRect::MakeWH(600, 400),
                    SkRect::MakeWH(600, 400),
                    &damage);
    }

    {
        // Shrink r2 under transform.
        r2->setR(250);
        std::vector<SkRect> damage = { {400, 200, 600, 400}, { 400, 200, 500, 400} };
        check_inval(reporter, root,
                    SkRect::MakeWH(500, 400),
                    SkRect::MakeLTRB(400, 200, 600, 400),
                    &damage);
    }
}

static void inval_test2(skiatest::Reporter* reporter) {
    auto color = sksg::Color::Make(0xff000000);
    auto rect  = sksg::Rect::Make(SkRect::MakeWH(100, 100));
    auto m1    = sksg::Matrix::Make(SkMatrix::I()),
         m2    = sksg::Matrix::Make(SkMatrix::I(), m1);
    auto t1    = sksg::Transform::Make(sksg::Draw::Make(rect, color), m2),
         t2    = sksg::Transform::Make(sksg::Draw::Make(rect, color), m1);
    auto root  = sksg::Group::Make();
    root->addChild(t1);
    root->addChild(t2);

    {
        // Initial revalidation.
        check_inval(reporter, root,
                    SkRect::MakeWH(100, 100),
                    SkRectPriv::MakeLargeS32(),
                    nullptr);
    }

    {
        // Update the shared color.
        color->setColor(0xffff0000);
        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 0, 0, 100, 100} };
        check_inval(reporter, root,
                    SkRect::MakeWH(100, 100),
                    SkRect::MakeWH(100, 100),
                    &damage);
    }

    {
        // Update m2.
        m2->setMatrix(SkMatrix::MakeScale(2, 2));
        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 0, 0, 200, 200} };
        check_inval(reporter, root,
                    SkRect::MakeWH(200, 200),
                    SkRect::MakeWH(200, 200),
                    &damage);
    }

    {
        // Update shared m1.
        m1->setMatrix(SkMatrix::MakeTrans(100, 100));
        std::vector<SkRect> damage = { {   0,   0, 200, 200},   // draw1 prev bounds
                                       { 100, 100, 300, 300},   // draw1 new bounds
                                       {   0,   0, 100, 100},   // draw2 prev bounds
                                       { 100, 100, 200, 200} }; // draw2 new bounds
        check_inval(reporter, root,
                    SkRect::MakeLTRB(100, 100, 300, 300),
                    SkRect::MakeLTRB(  0,   0, 300, 300),
                    &damage);
    }

    {
        // Update shared rect.
        rect->setR(50);
        std::vector<SkRect> damage = { { 100, 100, 300, 300},   // draw1 prev bounds
                                       { 100, 100, 200, 300},   // draw1 new bounds
                                       { 100, 100, 200, 200},   // draw2 prev bounds
                                       { 100, 100, 150, 200} }; // draw2 new bounds
        check_inval(reporter, root,
                    SkRect::MakeLTRB(100, 100, 200, 300),
                    SkRect::MakeLTRB(100, 100, 300, 300),
                    &damage);
    }
}

DEF_TEST(SGInvalidation, reporter) {
    inval_test1(reporter);
    inval_test2(reporter);
}

#endif // !defined(SK_BUILD_FOR_GOOGLE3)
