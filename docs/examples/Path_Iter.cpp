#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2f53df9201769ab7e7c0e164a1334309
REG_FIDDLE(Path_Iter, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(256);
    SkPath asterisk, path;
    paint.getTextPath("*", 1, 50, 192, &asterisk);
    SkPath::Iter iter(asterisk, true);
    SkPoint start[4], pts[4];
    iter.next(start);  // skip moveTo
    iter.next(start);  // first quadTo
    path.moveTo((start[0] + start[1]) * 0.5f);
    while (SkPath::kClose_Verb != iter.next(pts)) {
        path.quadTo(pts[0], (pts[0] + pts[1]) * 0.5f);
    }
    path.quadTo(start[0], (start[0] + start[1]) * 0.5f);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
