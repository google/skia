// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=eb5fa5bea23059ce538e883502f828f5
REG_FIDDLE(Path_RawIter_peek, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.quadTo(10, 20, 30, 40);
    path.conicTo(1, 2, 3, 4, .5f);
    path.cubicTo(1, 2, 3, 4, .5, 6);
    SkPath::RawIter iter(path);
    SkPath::Verb verb, peek = iter.peek();
    const char* verbStr[] =  { "Move", "Line", "Quad", "Conic", "Cubic", "Close", "Done" };
    do {
        SkPoint points[4];
        verb = iter.next(points);
        SkDebugf("peek %s %c= verb %s\n", verbStr[peek], peek == verb ? '=' : '!', verbStr[verb]);
        peek = iter.peek();
    } while (SkPath::kDone_Verb != verb);
    SkDebugf("peek %s %c= verb %s\n", verbStr[peek], peek == verb ? '=' : '!', verbStr[verb]);
}
}  // END FIDDLE
