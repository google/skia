/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

DEF_SIMPLE_GM(PlusMergesAA, canvas, 256, 256) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);  //  <-- crucial to the test that we use AA

    // Draw a two red squares.
    canvas->drawRect(SkRect::MakeWH(100, 100), p);
    canvas->drawRect(SkRect::MakeXYWH(150, 0, 100, 100), p);

    p.setColor(0xf000ff00);

    // We'll draw a green square on top of each using two triangles.
    SkPath upperLeft;
    upperLeft.lineTo(100, 0);
    upperLeft.lineTo(0, 100);
    upperLeft.lineTo(0, 0);

    SkPath bottomRight;
    bottomRight.moveTo(100, 0);
    bottomRight.lineTo(100, 100);
    bottomRight.lineTo(0, 100);
    bottomRight.lineTo(100, 0);

    // The left square is drawn simply with SrcOver.  It will show a red seam.
    canvas->drawPath(upperLeft, p);
    canvas->drawPath(bottomRight, p);

    // Using Plus on the right should merge the AA of seam together completely covering the red.
    canvas->saveLayer(nullptr, nullptr);
      p.setBlendMode(SkBlendMode::kPlus);
      canvas->translate(150, 0);
      canvas->drawPath(upperLeft, p);
      canvas->drawPath(bottomRight, p);
    canvas->restore();
}
