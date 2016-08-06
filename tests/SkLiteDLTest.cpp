/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkLiteDL.h"
#include "SkLiteRecorder.h"

#if 0   // This test doesn't make sense when run in a threaded environment.  It tests global state.
DEF_TEST(SkLiteDL_freelisting, r) {
    // TODO: byte and count limit tests
    sk_sp<SkLiteDL> sp1 = SkLiteDL::New({1,1,10,10}),
                    sp2 = SkLiteDL::New({2,2,20,20});

    SkLiteDL* p1 = sp1.get();
    SkLiteDL* p2 = sp2.get();
    REPORTER_ASSERT(r, p1 != p2);
    REPORTER_ASSERT(r, p1->getBounds().left() == 1);
    REPORTER_ASSERT(r, p2->getBounds().left() == 2);

    sp2.reset();

    sk_sp<SkLiteDL> sp3 = SkLiteDL::New({3,3,30,30});
    SkLiteDL* p3 = sp3.get();
    REPORTER_ASSERT(r, p1 != p3);
    REPORTER_ASSERT(r, p2 == p3);
    REPORTER_ASSERT(r, p1->getBounds().left() == 1);
    REPORTER_ASSERT(r, p3->getBounds().left() == 3);

    sp3.reset();
    sp1.reset();

    sk_sp<SkLiteDL> sp4 = SkLiteDL::New({4,4,40,40});
    SkLiteDL* p4 = sp4.get();
    REPORTER_ASSERT(r, p4 == p1);  // Checks that we operate in stack order.  Nice, not essential.
    REPORTER_ASSERT(r, p4->getBounds().left() == 4);
}
#endif

DEF_TEST(SkLiteDL_basics, r) {
    sk_sp<SkLiteDL> p { SkLiteDL::New({2,2,3,3}) };

    p->save();
        p->clipRect(SkRect{2,3,4,5}, SkRegion::kIntersect_Op, true);
        p->drawRect(SkRect{0,0,9,9}, SkPaint{});
    p->restore();
}

DEF_TEST(SkLiteRecorder, r) {
    sk_sp<SkLiteDL> p { SkLiteDL::New({2,2,3,3}) };

    SkLiteRecorder rec;
    SkCanvas* c = &rec;

    rec.reset(p.get());

    c->save();
        c->clipRect(SkRect{2,3,4,5}, SkRegion::kIntersect_Op, true);
        c->drawRect(SkRect{0,0,9,9}, SkPaint{});
    c->restore();
}
