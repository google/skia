// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_PremultipliedAlpha, 200, 200, false, 0) {
// Interactive version of this demo available at:
// https://shaders.skia.org/?id=8a80c2a7bcd9fb6b39460fd6acc1e828e5725d0210b72719ef696b02e1557435
void draw(SkCanvas* canvas) {
  const char* sksl =

  "const half3 iColor = half3(0, 0.5, 0.75);"
  "half4 main(float2 coord) {"
  "  float alpha = 1 - (coord.y / 150);"
  "  if (coord.x < 100) {"
  "    /* Correctly premultiplied version of color */"
  "    return iColor.rgb1 * alpha;"
  "  } else {"
  "    /* Returning an unpremultiplied color (just setting alpha) leads to over-bright colors. */"
  "    return half4(iColor, alpha);"
  "  }"
  "}";

  auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(sksl));
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
