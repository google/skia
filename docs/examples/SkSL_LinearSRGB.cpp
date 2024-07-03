// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_LinearSRGB, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
  // Make a simple lighting effect:
  auto litEffect = SkRuntimeEffect::MakeForShader(SkString(R"(
    layout(color) uniform vec3 surfaceColor;
    uniform int doLinearLighting;

    vec3 normal_at(vec2 p) {
      p = (p / 128) * 2 - 1;
      float len2 = dot(p, p);
      vec3 n = (len2 > 1) ? vec3(0, 0, 1) : vec3(p, sqrt(1 - len2));
      return normalize(n);
    }

    vec4 main(vec2 p) {
      vec3 n = normal_at(p);
      vec3 l = normalize(vec3(-1, -1, 0.5));
      vec3 C = surfaceColor;

      if (doLinearLighting != 0) { C = toLinearSrgb(C); }
      C *= saturate(dot(n, l));
      if (doLinearLighting != 0) { C = fromLinearSrgb(C); }

      return C.rgb1;
    })")).effect;
  SkRuntimeShaderBuilder builder(litEffect);
  builder.uniform("surfaceColor") = SkV3{0.8, 0.8, 0.8};
  SkPaint paint;

  // FIRST: Draw the lit sphere without converting to linear sRGB.
  // This produces INCORRECT light falloff.
  builder.uniform("doLinearLighting") = 0;
  paint.setShader(builder.makeShader());
  canvas->drawRect({0,0,128,128}, paint);

  // SECOND: Draw the lit sphere with math done in linear sRGB.
  // This produces sharper falloff, which is CORRECT.
  builder.uniform("doLinearLighting") = 1;
  paint.setShader(builder.makeShader());
  canvas->translate(128, 0);
  canvas->drawRect({0,0,128,128}, paint);
}
}  // END FIDDLE
