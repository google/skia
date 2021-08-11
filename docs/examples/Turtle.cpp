// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(Turtle, 256, 256, false, 0, 2) {
// Simple turtle based graphics. The turtle starts out at 128, 128, looking North, pen down.
// The input string is read left to right (but see 'r' below). Commands are a single character,
// sometimes followed by additional arguments:
//
// u : Raises the pen
// d : Lowers the pen
// + : Reads integer N, rotates turtle N degrees clockwise
// - : Reads integer N, rotates turtle N degrees counterclockwise
// f : Reads integer N, moves turtle forwards N units
// r : Reads integer N, then separator character C. C should be some non-digit, non-command
//     character. Repeats all commands after C until the next instance of C, N times. Can be
//     nested.
//const char* input = "r2[r3(f50+90f50+90f50+90f50(+45uf50d[";
//const char* input = "r360|f1+1|";
const char* input = "uf100+91dr180|f3+2|+89uf60+90r2$f20-90dr60|f1+6|u-90f20$-90f50-90f30d+180f60uf1000";

struct Turtle { float x; float y; float h; bool p; } t;

const SkPaint& p() {
  static SkPaint paint;
  paint.setColor(SK_ColorBLACK);
  paint.setAntiAlias(true);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(0);
  return paint;
}

const char* eval(SkCanvas* canvas, const char* s, char e, float& dist, float& l, bool pt) {
  while (*s != e) {
    switch(*s++) {
      case 'u': t.p = false; break;
      case 'd': t.p = true; break;
      case '+': t.h += atoi(s); break;
      case '-': t.h -= atoi(s); break;
      case 'f': {
        float d = atoi(s);
        d = std::min(d, l);
        dist += d; l -= d;
        float r = t.h * 0.01745329f;
        Turtle nt = { t.x + sinf(r) * d, t.y - cosf(r) * d, t.h, t.p };
        if (pt && t.p) canvas->drawLine(t.x, t.y, nt.x, nt.y, p());
        t = nt;
        break;
      }
      case 'r': {
        int c = atoi(s);
        while (*s >= '0' && *s <= '9') { ++s; }
        auto n = s+1;
        for (int i = 0; i < c; ++i) { n = eval(canvas, s+1, *s, dist, l, pt); }
        s = n;
      }
    }
  }
  return s+1;
}

void draw(SkCanvas* canvas) {
  canvas->clear(SK_ColorWHITE);

  t = { 128, 128, 0, true };
  float totalDist = 0;
  float l = 1E9f;
  eval(canvas, input, 0, totalDist, l, false);

  l = frame * totalDist;
  t = { 128, 128, 0, true };
  eval(canvas, input, 0, totalDist, l, true);
}
}  // END FIDDLE
