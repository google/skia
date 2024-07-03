// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkSL_RawImageShaders, 384, 128, false, 0) {
static sk_sp<SkImage> make_image(sk_sp<SkRuntimeEffect> effect,
                                 const SkImageInfo& info) {
  sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
  SkCanvas* canvas = surface->getCanvas();
  auto shader = effect->makeShader(/*uniforms=*/ nullptr, /*children=*/ {});
  if (!shader) {
      return nullptr;
  }
  SkPaint paint;
  paint.setShader(std::move(shader));
  paint.setBlendMode(SkBlendMode::kSrc);
  canvas->drawPaint(paint);
  return surface->makeImageSnapshot();
}

void draw(SkCanvas* canvas) {
  // Make a hemispherical normal map image:
  auto imageInfo = SkImageInfo::MakeN32Premul(128, 128);
  auto imageShader = SkRuntimeEffect::MakeForShader(SkString(R"(
    vec4 main(vec2 p) {
      p = (p / 128) * 2 - 1;
      float len2 = dot(p, p);
      vec3 v = (len2 > 1) ? vec3(0, 0, 1) : vec3(p, sqrt(1 - len2));
      return (v * 0.5 + 0.5).xyz1;
    })")).effect;
  auto normalImage = make_image(imageShader, imageInfo);

  // Make a simple lighting effect:
  auto litEffect = SkRuntimeEffect::MakeForShader(SkString(R"(
    uniform shader normals;
    vec4 main(vec2 p) {
      vec3 n = normalize(normals.eval(p).xyz * 2 - 1);
      vec3 l = normalize(vec3(-1, -1, 0.5));
      return saturate(dot(n, l)).xxx1;
    })")).effect;
  SkRuntimeShaderBuilder builder(litEffect);
  SkPaint paint;

  // FIRST: Draw the lighting to our (not color managed) canvas.
  // This is our CORRECT, reference result:
  builder.child("normals") = normalImage->makeShader(SkSamplingOptions{});
  paint.setShader(builder.makeShader());
  canvas->drawRect({0,0,128,128}, paint);

  // Make an offscreen surface with a wide gamut:
  auto rec2020 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                       SkNamedGamut::kRec2020);
  auto info = SkImageInfo::Make(128, 128, kRGBA_F16_SkColorType,
                                kPremul_SkAlphaType, rec2020);
  auto surface = SkSurfaces::Raster(info);

  // SECOND: Draw the lighting to the offscreen surface. Color management
  // changes the normals, producing INCORRECT (wrong direction) lighting:
  surface->getCanvas()->drawPaint(paint);
  canvas->drawImage(surface->makeImageSnapshot(), 128, 0);

  // THIRD: Convert the normals to a raw image shader. This ignores color
  // management for that image, so we get CORRECT lighting again:
  builder.child("normals") = normalImage->makeRawShader(SkSamplingOptions{});
  paint.setShader(builder.makeShader());
  surface->getCanvas()->drawPaint(paint);
  canvas->drawImage(surface->makeImageSnapshot(), 256, 0);
}
}  // END FIDDLE
