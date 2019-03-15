#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d70194c9c51e700335f95de91846d023
REG_FIDDLE(Image_016, 256, 156, false, 5) {
void draw(SkCanvas* canvas) {
    sk_sp<SkImage> subset = image->makeSubset({10, 20, 90, 100});
    canvas->drawImage(image, 0, 0);
    canvas->drawImage(subset, 128, 0);
    SkPaint paint;
    SkString s;
    s.printf("original id: %d", image->uniqueID());
    canvas->drawString(s, 20, image->height() + 20, paint);
    s.printf("subset id: %d", subset->uniqueID());
    canvas->drawString(s, 148, subset->height() + 20, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
