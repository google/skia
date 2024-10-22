// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

REG_FIDDLE(overdrawcolorfilter_grid, 256, 256, false, 0) {

// This colorfilter turns alpha values into a color from the list
// (e.g. alpha 3 would produce a yellow color).
sk_sp<SkColorFilter> color_filter() {
  static const SkColor colors[SkOverdrawColorFilter::kNumColors] = {
      0x80FF0000, // red
      0x8000FF00, // green
      0x800000FF, // blue
      0x80FFFF00, // yellow
      0x8000FFFF, // cyan
      0x80FF00FF, // magenta
  };
  return SkOverdrawColorFilter::MakeWithSkColors(colors);
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
