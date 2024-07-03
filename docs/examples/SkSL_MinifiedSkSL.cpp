// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_MinifiedSkSL, 200, 200, false, 0) {
static constexpr char SKSL_MINIFIED_GradientShader[] =
"half4 main(float2 a){float b=1.-a.y*.006666667;if(a.x<100.)return half4(float4"
"(0.,.5,.75,1.)*b);else return half4(half3(0.,.5,.75),half(b));}";

void draw(SkCanvas* canvas) {
  auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(SKSL_MINIFIED_GradientShader));
  sk_sp<SkShader> myShader = effect->makeShader(/*uniforms=*/ nullptr,
                                                /*children=*/ {});

  // Fill canvas with gray, first:
  canvas->drawColor(SK_ColorGRAY);

  // Blend our test shader on top of that:
  SkPaint p;
  p.setShader(myShader);
  canvas->drawPaint(p);
}
}  // END FIDDLE
