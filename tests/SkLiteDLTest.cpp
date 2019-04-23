/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRSXform.h"
#include "src/core/SkLiteDL.h"
#include "src/core/SkLiteRecorder.h"
#include "tests/Test.h"

DEF_TEST(SkLiteDL_basics, r) {
    SkLiteDL p;
    p.save();
        p.clipRect(SkRect{2,3,4,5}, kIntersect_SkClipOp, true);
        p.drawRect(SkRect{0,0,9,9}, SkPaint{});
    p.restore();
}

DEF_TEST(SkLiteDL_unbalanced, r) {
    SkLiteRecorder rec;
    SkCanvas* c = &rec;

    SkLiteDL p;
    rec.reset(&p, {2,2,3,3});
    c->save();
        c->scale(2,2);
        c->save();
            c->translate(1,1);
        // missing restore() but SkLiteDL::draw should balance it for us
    c->restore();

    // reinit the recorder so we can playback the original SkLiteDL
    SkLiteDL p2;
    rec.reset(&p2, {2,2,3,3});

    REPORTER_ASSERT(r, 1 == rec.getSaveCount());
    p.draw(c);
    REPORTER_ASSERT(r, 1 == rec.getSaveCount());
}

DEF_TEST(SkLiteRecorder, r) {
    SkLiteDL p;
    SkLiteRecorder rec;
    SkCanvas* c = &rec;

    rec.reset(&p, {2,2,3,3});

    c->save();
        c->clipRect(SkRect{2,3,4,5}, kIntersect_SkClipOp, true);
        c->drawRect(SkRect{0,0,9,9}, SkPaint{});
    c->restore();
}

DEF_TEST(SkLiteRecorder_RecordsFlush, r) {
    SkLiteDL dl;

    SkLiteRecorder canvas;
    canvas.reset(&dl, {0,0,100,100});

    REPORTER_ASSERT(r,  dl.empty());
    canvas.flush();
    REPORTER_ASSERT(r, !dl.empty());
}
