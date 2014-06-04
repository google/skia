/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBHFactory.h"
#include "SkCanvas.h"
#include "SkPictureRecorder.h"
#include "SkPictureStateTree.h"
#include "Test.h"

static SkPicture* draw_scene(SkBBHFactory* bbhFactory) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(200, 200, bbhFactory, 0);

    SkPaint p1, p2;
    p1.setStyle(SkPaint::kFill_Style);
    p1.setARGB(0x80, 0, 0xff, 0);
    p2.setStyle(SkPaint::kFill_Style);
    p2.setARGB(0x80, 0xff, 0, 0);

    canvas->drawColor(SK_ColorWHITE);

    // This is intended to exercise some tricky SkPictureStateTree code paths when
    // played back with various clips:
    //
    //   * cleanup/rewind when the last draw is not at the root of the state tree.
    //   * state nodes with both kSave_Flag & kSaveLayer_Flag set.
    //   * state tree transitions which implicitly reset the matrix via. restore().

    canvas->save();
      canvas->translate(10, 10);

      canvas->drawRect(SkRect::MakeWH(100, 50), p1);
      canvas->drawRect(SkRect::MakeWH(50, 100), p2);

      SkRect layerBounds = SkRect::MakeXYWH(0, 0, 90, 90);
      canvas->saveLayer(&layerBounds, NULL);
        canvas->save();
          canvas->clipRect(layerBounds);

          canvas->save();
            canvas->clipRect(SkRect::MakeWH(25, 25));
            canvas->rotate(90);
            canvas->drawRect(SkRect::MakeWH(100, 50), p1);
          canvas->restore();

          canvas->save();
            canvas->clipRect(SkRect::MakeWH(25, 25));
            canvas->save();
              canvas->rotate(90);
              canvas->drawRect(SkRect::MakeWH(50, 100), p2);
            canvas->restore();
            canvas->drawRect(SkRect::MakeWH(100, 50), p1);
          canvas->restore();
          canvas->drawRect(SkRect::MakeXYWH(99, 99, 1, 1), p1);
        canvas->restore();
      canvas->restore();

    canvas->restore();

    return recorder.endRecording();
}

static void check_bms(skiatest::Reporter* reporter, const SkBitmap& bm1, const SkBitmap& bm2) {
    SkASSERT(bm1.getSize() == bm2.getSize());
    REPORTER_ASSERT(reporter, 0 == memcmp(bm1.getAddr(0, 0), bm2.getAddr(0, 0), bm1.getSize()));
}

static void test_reference_picture(skiatest::Reporter* reporter) {
    SkRTreeFactory bbhFactory;

    SkAutoTUnref<SkPicture> bbhPicture(draw_scene(&bbhFactory));
    SkAutoTUnref<SkPicture> referencePicture(draw_scene(NULL));

    SkBitmap referenceBitmap;
    referenceBitmap.allocN32Pixels(100, 100);
    SkCanvas referenceCanvas(referenceBitmap);

    SkBitmap bbhBitmap;
    bbhBitmap.allocN32Pixels(100, 100);
    SkCanvas bbhCanvas(bbhBitmap);

    referenceCanvas.drawColor(SK_ColorTRANSPARENT);
    referenceCanvas.drawPicture(referencePicture.get());
    bbhCanvas.drawColor(SK_ColorTRANSPARENT);
    bbhCanvas.drawPicture(bbhPicture.get());
    REPORTER_ASSERT(reporter,
                    referenceCanvas.getSaveCount() == bbhCanvas.getSaveCount());
    REPORTER_ASSERT(reporter,
                    referenceCanvas.getTotalMatrix() == bbhCanvas.getTotalMatrix());
    check_bms(reporter, referenceBitmap, bbhBitmap);

    referenceCanvas.drawColor(SK_ColorTRANSPARENT);
    referenceCanvas.clipRect(SkRect::MakeWH(50, 50));
    referenceCanvas.drawPicture(referencePicture.get());
    bbhCanvas.drawColor(SK_ColorTRANSPARENT);
    bbhCanvas.clipRect(SkRect::MakeWH(50, 50));
    bbhCanvas.drawPicture(bbhPicture.get());
    REPORTER_ASSERT(reporter,
                    referenceCanvas.getSaveCount() == bbhCanvas.getSaveCount());
    REPORTER_ASSERT(reporter,
                    referenceCanvas.getTotalMatrix() == bbhCanvas.getTotalMatrix());
    check_bms(reporter, referenceBitmap, bbhBitmap);

    referenceCanvas.drawColor(SK_ColorTRANSPARENT);
    referenceCanvas.clipRect(SkRect::MakeWH(10, 10));
    referenceCanvas.drawPicture(referencePicture.get());
    bbhCanvas.drawColor(SK_ColorTRANSPARENT);
    bbhCanvas.clipRect(SkRect::MakeWH(10, 10));
    bbhCanvas.drawPicture(bbhPicture.get());
    REPORTER_ASSERT(reporter,
                    referenceCanvas.getSaveCount() == bbhCanvas.getSaveCount());
    REPORTER_ASSERT(reporter,
                    referenceCanvas.getTotalMatrix() == bbhCanvas.getTotalMatrix());
    check_bms(reporter, referenceBitmap, bbhBitmap);
}

DEF_TEST(PictureStateTree, reporter) {
    test_reference_picture(reporter);
}
