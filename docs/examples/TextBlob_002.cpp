#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e9d4eb8ece521b1329e7433d4b243fdf
REG_FIDDLE(TextBlob_getIntercepts, 256, 143, false, 0) {
void draw(SkCanvas* canvas) {
    SkFont font;
    font.setSize(120);
    SkPoint textPos = { 20, 110 };
    int len = 3;
    SkTextBlobBuilder textBlobBuilder;
    const SkTextBlobBuilder::RunBuffer& run =
            textBlobBuilder.allocRun(font, len, textPos.fX, textPos.fY);
    run.glyphs[0] = 10;
    run.glyphs[1] = 20;
    run.glyphs[2] = 30;
    sk_sp<const SkTextBlob> blob = textBlobBuilder.make();
    SkPaint paint;
    SkScalar bounds[] = { 116, 134 };
    int count = blob->getIntercepts(bounds, nullptr);
    std::vector<SkScalar> intervals;
    intervals.resize(count);
    (void) paint.getTextBlobIntercepts(blob.get(), bounds, &intervals.front());
    canvas->drawTextBlob(blob.get(), 0, 0, paint);
    paint.setColor(0xFFFF7777);
    SkScalar x = textPos.fX;
    for (int i = 0; i < count; i+= 2) {
        canvas->drawRect({x, bounds[0], intervals[i], bounds[1]}, paint);
        x = intervals[i + 1];
    }
    canvas->drawRect({intervals[count - 1], bounds[0], 180, bounds[1]}, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
