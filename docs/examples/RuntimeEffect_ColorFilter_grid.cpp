// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

REG_FIDDLE(runtimeeffect_colorfilter_grid, 256, 256, false, 0) {
constexpr size_t kNumColors = 8;
// This colorfilter turns alpha values into a color from the list
// (e.g. alpha 3 would produce a yellow color).
sk_sp<SkColorFilter> color_filter() {
  static const SkColor colors[kNumColors] = {
      0x80FF0000, // red
      0x8000FF00, // green
      0x800000FF, // blue
      0x80FFFF00, // yellow
      0x8000FFFF, // cyan
      0x80FF00FF, // magenta
      0x80DDDDDD, // light grey
      0x80333333, // dark grey
  };
  static constexpr char kSKSL[] =
    "uniform half4 color0, color1, color2, color3, color4, color5, color6, color7;"

    "half4 main(half4 color) {"
       "return color.a < (0.5 / 255.) ? color0"
         ": color.a < (1.5 / 255.) ? color1"
         ": color.a < (2.5 / 255.) ? color2"
         ": color.a < (3.5 / 255.) ? color3"
         ": color.a < (4.5 / 255.) ? color4"
         ": color.a < (5.5 / 255.) ? color5"
         ": color.a < (6.5 / 255.) ? color6"
                                  ": color7;"
    "}";
  auto [effect, error] = SkRuntimeEffect::MakeForColorFilter(SkString(kSKSL));
  if (!effect) {
    SkDebugf("RuntimeShader error: %s\n", error.c_str());
  }
  using Half4Color = SkRGBA4f<kPremul_SkAlphaType>;
  auto data = SkData::MakeUninitialized(kNumColors * sizeof(Half4Color));
  Half4Color* premul = (Half4Color*)data->writable_data();
  for (size_t i = 0; i < kNumColors; i++) {
      premul[i] = SkColor4f::FromColor(colors[i]).premul();
  }
  return effect->makeColorFilter(std::move(data));
}

constexpr int WIDTH = 80;

// Make an alpha only bitmap where every pixel is set to the same alpha value
SkBitmap make_bitmap(uint8_t alpha) {
  SkImageInfo info = SkImageInfo::MakeA8(WIDTH, WIDTH);
  SkBitmap bitmap;
  bitmap.allocPixels(info);
  bitmap.eraseARGB(alpha, 0, 0, 0);
  return bitmap;
}

void draw(SkCanvas* canvas) {
  SkSamplingOptions sampling;
  SkPaint paint;
  paint.setColorFilter(color_filter());

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      SkBitmap bitmap = make_bitmap(row * 3 + col);
      canvas->drawImage(bitmap.asImage(), col * WIDTH, row * WIDTH, sampling, &paint);
    }
  }
}
}  // END FIDDLE
