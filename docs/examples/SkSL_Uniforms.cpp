// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_Uniforms, 128, 64, false, 0) {
void draw(SkCanvas* canvas) {
  // Make a very wide-gamut offscreen surface:
  auto rec2020 = SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear,
                                       SkNamedGamut::kRec2020);
  auto info = SkImageInfo::Make(128, 64, kRGBA_F16_SkColorType,
                                kPremul_SkAlphaType, rec2020);
  auto surface = SkSurfaces::Raster(info);

  // Effect draws horizontal gradients. Top half uses the simple uniform,
  // bottom half uses a uniform tagged as a color:
  const char* sksl = R"(
                  uniform vec4 not_a_color;
    layout(color) uniform vec4 color;

    vec4 main(vec2 xy) {
      vec4 c = xy.y < 32 ? not_a_color : color;
      return (c * (xy.x / 128)).rgb1;
    })";

  auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(sksl));

  // Set both uniforms to be "red":
  SkRuntimeShaderBuilder builder(effect);
  builder.uniform("not_a_color") = SkV4{ 1, 0, 0, 1 };  // Red?
  builder.uniform("color")       = SkV4{ 1, 0, 0, 1 };  // sRGB Red

  // Fill the offscreen surface:
  SkPaint paint;
  paint.setShader(builder.makeShader());
  surface->getCanvas()->drawPaint(paint);

  // Draw our offscreen image back to the original canvas:
  canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
}
}  // END FIDDLE
