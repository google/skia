// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3ba94448a4ba48f926e643baeb5b1016
REG_FIDDLE(Path_079, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
      SkPaint conicPaint;
      conicPaint.setAntiAlias(true);
      conicPaint.setStyle(SkPaint::kStroke_Style);
      SkPaint quadPaint(conicPaint);
      quadPaint.setColor(SK_ColorRED);
      SkPoint conic[] = { {20, 170}, {80, 170}, {80, 230} };
      for (auto weight : { .25f, .5f, .707f, .85f, 1.f } ) {
          SkPoint quads[5];
          SkPath::ConvertConicToQuads(conic[0], conic[1], conic[2], weight, quads, 1);
          SkPath path;
          path.moveTo(conic[0]);
          path.conicTo(conic[1], conic[2], weight);
          canvas->drawPath(path, conicPaint);
          path.rewind();
          path.moveTo(quads[0]);
          path.quadTo(quads[1], quads[2]);
          path.quadTo(quads[3], quads[4]);
          canvas->drawPath(path, quadPaint);
          canvas->translate(50, -50);
      }
}
}  // END FIDDLE
