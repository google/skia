// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(subset_example, 512, 512, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);
    const int N = 8;
    int shuffle[N * N];
    for (int i = 0; i < (N * N); ++i) {
        shuffle[i] = i;
    }
    srand(0);
    for (int i = 0; i < (N * N); ++i) {
        std::swap(shuffle[i], shuffle[rand() % (N * N - i) + i]);
    }
    int w = (source.width() - 1) / N + 1;
    int h = (source.height() - 1) / N + 1;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int x = shuffle[(N * i) + j] % N;
            int y = shuffle[(N * i) + j] / N;
            SkBitmap subset;
            source.extractSubset(&subset, SkIRect::MakeXYWH(w * x, h * y, w, h));
            canvas->drawImage(subset.asImage(), w * i, h * j);
        }
    }
}
}  // END FIDDLE
