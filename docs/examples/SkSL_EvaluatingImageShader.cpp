// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_EvaluatingImageShader, 128, 128, false, 5) {
void draw(SkCanvas* canvas) {
  // Turn `image` into an SkShader:
  sk_sp<SkShader> imageShader = image->makeShader(SkSamplingOptions(SkFilterMode::kLinear));

  const char* sksl =
    "uniform shader image;"
    "half4 main(float2 coord) {"
    "  return image.eval(coord).bgra;"  // Sample 'image', then swap red and blue
    "}";

  // Parse the SkSL, and create an SkRuntimeEffect object:
  auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(sksl));

  // SkRuntimeEffect::makeShader expects an SkSpan<ChildPtr>, one per `uniform shader`:
  SkRuntimeEffect::ChildPtr children[] = { imageShader };

  // Create an SkShader from our SkSL, with `imageShader` bound to `image`:
  sk_sp<SkShader> myShader = effect->makeShader(/*uniforms=*/ nullptr,
                                                /*children=*/ { children, 1 });

  // Fill the surface with `myShader`:
  SkPaint p;
  p.setShader(myShader);
  canvas->drawPaint(p);
}
}  // END FIDDLE
