// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(l_system_plant, 256, 256, false, 0, 3) {
// L-System
// https://en.wikipedia.org/wiki/L-system#Example_7:_Fractal_plant

struct rules_t {
    char c;
    std::string s;
};

rules_t rules[6] = {
    {'X', "F-[[X]+X]+F[+FX]-X"},
    {'F', "FF"},
    {'+', "+"},
    {'-', "-"},
    {'[', "["},
    {']', "]"},
};

std::string E(std::string s) {
    if (s.size() == 0) {
        return "";
    }
    for (int i=0; i<6; i++) {
        if (rules[i].c == s[0]) {
            return rules[i].s + E(s.substr(1));
        }
    }
    return "";
}

struct Pt {
  SkScalar x;
  SkScalar y;
  SkScalar a;
};

void draw(SkCanvas* canvas) {
  canvas->drawColor(SK_ColorLTGRAY);

  SkPaint p;
  p.setColor(0xFFA6761D);
  p.setAntiAlias(true);
  p.setStyle(SkPaint::kStroke_Style);
  p.setStrokeWidth(1);

  std::vector<struct Pt> ptstack;
  std::string plant = E(E(E(E(E("X")))));
  const double len = 2.5;
  struct Pt pt = {128, 256, 3.14};
  SkPath path;
  path.moveTo(pt.x, pt.y);

  for (std::string::iterator it=plant.begin(); it!=plant.end(); ++it) {
    if (*it == 'F') {
      pt.x += len*sin(pt.a);
      pt.y += len*cos(pt.a);
      path.lineTo(pt.x, pt.y);
    } else if (*it == '+') {
      pt.a += (0.15 + sin(frame*2.0*3.14159)*0.05);
    } else if (*it == '-') {
      pt.a += (-0.15 + sin(frame*2.0*3.14159)*0.05);
    } else if (*it == '[') {
      ptstack.push_back(pt);
    } else if (*it == ']') {
      pt = ptstack.back();
      ptstack.pop_back();
      path.moveTo(pt.x, pt.y);
    }
  }
  canvas->drawPath(path, p);
}
}  // END FIDDLE
