#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a8b8bd4bfe968e2c63085f867665227f
REG_FIDDLE(Image_isLazyGenerated_a, 256, 80, false, 0) {
class TestImageGenerator : public SkImageGenerator {
public:
    TestImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(10, 10)) {}
    ~TestImageGenerator() override {}
protected:
    bool onGetPixels(const SkImageInfo& info, void* pixelPtr, size_t rowBytes,
                     const Options& options) override {
        SkPMColor* pixels = static_cast<SkPMColor*>(pixelPtr);
        for (int y = 0; y < info.height(); ++y) {
            for (int x = 0; x < info.width(); ++x) {
                pixels[y * info.width() + x] = 0xff223344 + y * 0x000C0811;
            }
        }
        return true;
    }
};

void draw(SkCanvas* canvas) {
    auto gen = std::unique_ptr<TestImageGenerator>(new TestImageGenerator());
    sk_sp<SkImage> image(SkImage::MakeFromGenerator(std::move(gen)));
    SkString lazy(image->isLazyGenerated() ? "is lazy" : "not lazy");
    canvas->scale(8, 8);
    canvas->drawImage(image, 0, 0, nullptr);
    SkPaint paint;
    paint.setTextSize(4);
    canvas->drawString(lazy, 2, 5, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
