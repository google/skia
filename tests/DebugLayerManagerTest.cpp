/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "tests/Test.h"
#include "tools/debugger/DebugLayerManager.h"

// Adds one full update, one partial update, and requests one image a few frames later.
static void test_basic(skiatest::Reporter* reporter) {
  // prepare supporting objects
  int layerWidth = 100;
  int layerHeight = 100;

  // make a picture that fully updates the layer
  SkPictureRecorder rec;
  SkCanvas* canvas = rec.beginRecording(layerWidth, layerHeight);
  canvas->clear(0x00000000);
  SkPaint paint;
  paint.setColor(SK_ColorBLUE);
  canvas->drawOval(SkRect::MakeLTRB(1,1,99,99), paint);
  auto picture1 = rec.finishRecordingAsPicture();
  SkIRect dirtyRectFull = SkIRect::MakeLTRB(0, 0, layerWidth, layerHeight);

  // make a second picture that acts as a partial update.
  SkPictureRecorder rec2;
  canvas = rec2.beginRecording(layerWidth, layerHeight);
  paint.setColor(SK_ColorGREEN);
  canvas->drawOval(SkRect::MakeLTRB(40,40,60,60), paint);
  auto picture2 = rec2.finishRecordingAsPicture();
  SkIRect dirtyRectPartial = SkIRect::MakeLTRB(40,40,60,60);

  int node = 2;

  // create and exercise DebugLayerManager
  DebugLayerManager dlm;
  dlm.storeSkPicture(node, 0, picture1, dirtyRectFull);
  dlm.storeSkPicture(node, 10, picture2, dirtyRectPartial);
  auto frames = dlm.listFramesForNode(node);

  // Confirm the layer manager stored these at the right places.
  REPORTER_ASSERT(reporter, frames.size() == 2);
  REPORTER_ASSERT(reporter, frames[0] == 0);
  REPORTER_ASSERT(reporter, frames[1] == 10);

  SkPixmap pixmap;
  // request an image of the layer between the two updates.
  for (int i=0; i<10; i++) {
    auto image = dlm.getLayerAsImage(node, i);
    REPORTER_ASSERT(reporter, image->width() == layerWidth);
    REPORTER_ASSERT(reporter, image->height() == layerHeight);
    // confirm center is blue, proving only first pic was drawn.
    image->peekPixels(&pixmap);
    SkColor paintColor = pixmap.getColor(50, 50);
    REPORTER_ASSERT(reporter, paintColor == SK_ColorBLUE);
  }

  // For any images after the second draw, confirm the center is green, but the area just outside
  // that smaller circle is still blue, proving dlm drew both pictures.
  for (int i=10; i<12; i++) {
    auto image = dlm.getLayerAsImage(node, i);
    REPORTER_ASSERT(reporter, image->width() == layerWidth);
    REPORTER_ASSERT(reporter, image->height() == layerHeight);
    image->peekPixels(&pixmap);
    auto innerColor = pixmap.getColor(50, 50);
    REPORTER_ASSERT(reporter, innerColor == SK_ColorGREEN);
    auto outerColor = pixmap.getColor(10, 50);
    REPORTER_ASSERT(reporter, outerColor == SK_ColorBLUE);
  }


}

DEF_TEST(DebugLayerManagerTest, reporter) {
  test_basic(reporter);
}
