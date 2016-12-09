/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkLiteDL.h"
#include "SkLiteRecorder.h"

DEF_TEST(SkLiteDL_basics, r) {
    sk_sp<SkLiteDL> p { SkLiteDL::New({2,2,3,3}) };

    p->save();
        p->clipRect(SkRect{2,3,4,5}, SkCanvas::kIntersect_Op, true);
        p->drawRect(SkRect{0,0,9,9}, SkPaint{});
    p->restore();
}

DEF_TEST(SkLiteRecorder, r) {
    sk_sp<SkLiteDL> p { SkLiteDL::New({2,2,3,3}) };

    SkLiteRecorder rec;
    SkCanvas* c = &rec;

    rec.reset(p.get());

    c->save();
        c->clipRect(SkRect{2,3,4,5}, SkCanvas::kIntersect_Op, true);
        c->drawRect(SkRect{0,0,9,9}, SkPaint{});
    c->restore();
}
