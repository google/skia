/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRect.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGroup.h"
#include "SkSGInvalidationController.h"
#include "SkSGRect.h"
#include "SkSGTransform.h"

#include "Test.h"

#include <vector>

static void check_inval(skiatest::Reporter* reporter, const sk_sp<sksg::Node>& root,
                        const SkRect& expected_bounds,
                        const SkRect& expected_inval_bounds,
                        const std::vector<SkRect>* expected_damage) {
    sksg::InvalidationController ic;
    const auto bbox = root->revalidate(&ic, SkMatrix::I());

    if (0) {
        printf("** bbox: [%f %f %f %f], ibbox: [%f %f %f %f]\n",
               bbox.fLeft, bbox.fTop, bbox.fRight, bbox.fBottom,
               ic.bounds().left(), ic.bounds().top(), ic.bounds().right(), ic.bounds().bottom());
    }

    REPORTER_ASSERT(reporter, bbox == expected_bounds);
    REPORTER_ASSERT(reporter, ic.bounds() == expected_inval_bounds);

    if (expected_damage) {
        REPORTER_ASSERT(reporter, expected_damage->size() == SkTo<size_t>(ic.end() - ic.begin()));
        for (size_t i = 0; i < expected_damage->size(); ++i) {
            const auto r1 = (*expected_damage)[i],
                       r2 = ic.begin()[i];
            if (0) {
                printf("*** expected inval: [%f %f %f %f], actual: [%f %f %f %f]\n",
                       r1.left(), r1.top(), r1.right(), r1.bottom(),
                       r2.left(), r2.top(), r2.right(), r2.bottom());
            }
            REPORTER_ASSERT(reporter, r1 == r2);
        }
    }
}

DEF_TEST(SGInvalidation, reporter) {
    auto color = sksg::Color::Make(0xff000000);
    auto r1    = sksg::Rect::Make(SkRect::MakeWH(100, 100)),
         r2    = sksg::Rect::Make(SkRect::MakeWH(100, 100));
    auto grp   = sksg::Group::Make();
    auto tr    = sksg::Transform::Make(grp, SkMatrix::I());

    grp->addChild(sksg::Draw::Make(r1, color));
    grp->addChild(sksg::Draw::Make(r2, color));

    {
        // Initial revalidation.
        check_inval(reporter, tr,
                    SkRect::MakeWH(100, 100),
                    SkRect::MakeLargestS32(),
                    nullptr);
    }

    {
        // Move r2 to (200 100).
        r2->setL(200); r2->setT(100); r2->setR(300); r2->setB(200);
        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 200, 100, 300, 200} };
        check_inval(reporter, tr,
                    SkRect::MakeWH(300, 200),
                    SkRect::MakeWH(300, 200),
                    &damage);
    }

    {
        // Update the common color.
        // TODO: this doesn't work ATM as expected; fix and enable.
//        color->setColor(0xffff0000);
//        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 200, 100, 300, 200} };
//        check_inval(reporter, tr,
//                    SkRect::MakeWH(300, 200),
//                    SkRect::MakeWH(300, 200),
//                    &damage);
    }

    {
        // Shrink r1.
        r1->setR(50);
        std::vector<SkRect> damage = { {0, 0, 100, 100}, { 0, 0, 50, 100} };
        check_inval(reporter, tr,
                    SkRect::MakeWH(300, 200),
                    SkRect::MakeWH(100, 100),
                    &damage);
    }

    {
        // Update transform.
        tr->setMatrix(SkMatrix::MakeScale(2, 2));
        std::vector<SkRect> damage = { {0, 0, 300, 200}, { 0, 0, 600, 400} };
        check_inval(reporter, tr,
                    SkRect::MakeWH(600, 400),
                    SkRect::MakeWH(600, 400),
                    &damage);
    }

    {
        // Shrink r2 under transform.
        r2->setR(250);
        std::vector<SkRect> damage = { {400, 200, 600, 400}, { 400, 200, 500, 400} };
        check_inval(reporter, tr,
                    SkRect::MakeWH(500, 400),
                    SkRect::MakeLTRB(400, 200, 600, 400),
                    &damage);
    }
}
