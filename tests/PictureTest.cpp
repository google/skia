/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkStream.h"

#ifdef SK_DEBUG
// Ensure that deleting SkPicturePlayback does not assert. Asserts only fire in debug mode, so only
// run in debug mode.
static void test_deleting_empty_playback() {
    SkPicture picture;
    // Creates an SkPictureRecord
    picture.beginRecording(0, 0);
    // Turns that into an SkPicturePlayback
    picture.endRecording();
    // Deletes the old SkPicturePlayback, and creates a new SkPictureRecord
    picture.beginRecording(0, 0);
}

// Ensure that serializing an empty picture does not assert. Likewise only runs in debug mode.
static void test_serializing_empty_picture() {
    SkPicture picture;
    picture.beginRecording(0, 0);
    picture.endRecording();
    SkDynamicMemoryWStream stream;
    picture.serialize(&stream);
}
#endif

static void rand_op(SkCanvas* canvas, SkRandom& rand) {
    SkPaint paint;
    SkRect rect = SkRect::MakeWH(50, 50);

    SkScalar unit = rand.nextUScalar1();
    if (unit <= 0.3) {
//        SkDebugf("save\n");
        canvas->save();
    } else if (unit <= 0.6) {
//        SkDebugf("restore\n");
        canvas->restore();
    } else if (unit <= 0.9) {
//        SkDebugf("clip\n");
        canvas->clipRect(rect);
    } else {
//        SkDebugf("draw\n");
        canvas->drawPaint(paint);
    }
}

static void test_peephole(skiatest::Reporter* reporter) {
    SkRandom rand;

    for (int j = 0; j < 100; j++) {
        SkRandom rand2(rand.getSeed()); // remember the seed

        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(100, 100);

        for (int i = 0; i < 1000; ++i) {
            rand_op(canvas, rand);
        }
        picture.endRecording();
    }

    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(100, 100);
        SkRect rect = SkRect::MakeWH(50, 50);

        for (int i = 0; i < 100; ++i) {
            canvas->save();
        }
        while (canvas->getSaveCount() > 1) {
            canvas->clipRect(rect);
            canvas->restore();
        }
        picture.endRecording();
    }
}

static void TestPicture(skiatest::Reporter* reporter) {
#ifdef SK_DEBUG
    test_deleting_empty_playback();
    test_serializing_empty_picture();
#endif
    test_peephole(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Pictures", PictureTestClass, TestPicture)
