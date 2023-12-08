// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=80856fe921ce36f8d5a32d8672bccbfc
REG_FIDDLE(Image_refEncodedData, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext) {
        return;
    }

    struct {
        const char* name;
        sk_sp<SkImage> image;
    } tests[] = {{"image", image},
                 {"bitmap", source.asImage()},
                 {"texture",
                  SkImages::BorrowTextureFrom(dContext,
                                              backEndTexture,
                                              kTopLeft_GrSurfaceOrigin,
                                              kRGBA_8888_SkColorType,
                                              kOpaque_SkAlphaType,
                                              nullptr)}};
    SkString string;
    SkPaint paint;
    SkFont font = SkFont(fontMgr->matchFamilyStyle(nullptr, {}));

    for (const auto& test : tests ) {
        if (!test.image) {
            string.printf("no %s", test.name);
        } else {
            string.printf("%s" "encoded %s", test.image->refEncodedData() ? "" : "no ", test.name);
        }
        canvas->drawString(string, 10, 20, font, paint);
        canvas->translate(0, 20);
    }
}
}  // END FIDDLE
