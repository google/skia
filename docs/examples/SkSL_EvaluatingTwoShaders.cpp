// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_EvaluatingTwoShaders, 128, 128, false, 5) {
// Create a linear gradient from white (left) to black (right)
sk_sp<SkShader> makeGradientShader() {
  const SkPoint pts[] = { { 0, 0 }, { 128, 0 } };
  const SkColor colors[] = { SK_ColorWHITE, SK_ColorBLACK };
  return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

void draw(SkCanvas* canvas) {
  // Turn `image` into an SkShader:
  sk_sp<SkShader> imageShader = image->makeShader(SkSamplingOptions(SkFilterMode::kLinear));

  const char* sksl =
    "uniform shader input_1;"
    "uniform shader input_2;"
    "half4 main(float2 coord) {"
    "  return input_1.eval(coord) * input_2.eval(coord);"
    "}";
  SkRuntimeEffect::ChildPtr children[] = { /*input_1=*/ imageShader,
                                           /*input_2=*/ makeGradientShader() };

  // Create SkShader from SkSL, then fill surface: // SK_FOLD_START

  // Create an SkShader from our SkSL, with `children` bound to the inputs:
  auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(sksl));
  sk_sp<SkShader> myShader = effect->makeShader(/*uniforms=*/ nullptr,
                                                /*children=*/ { children, 2 });

  // Fill the surface with `myShader`:
  SkPaint p;
  p.setShader(myShader);
  canvas->drawPaint(p);
  // SK_FOLD_END
}
}  // END FIDDLE
