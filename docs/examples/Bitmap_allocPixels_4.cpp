// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1b2800d23c9ea249b45c2c21a34b6d14
REG_FIDDLE(Bitmap_allocPixels_4, 256, 32, false, 0) {
class TinyAllocator : public SkBitmap::Allocator {
public:
    bool allocPixelRef(SkBitmap* bitmap) override {
        const SkImageInfo& info = bitmap->info();
        if (info.height() * info.minRowBytes() > sizeof(storage)) {
            return false;
        }
        sk_sp<SkPixelRef> pr = sk_sp<SkPixelRef>(
                new SkPixelRef(info.width(), info.height(), storage, info.minRowBytes()));
        bitmap->setPixelRef(std::move(pr), 0, 0);
        return true;
    }
    char storage[16];
};

void draw(SkCanvas* canvas) {
   TinyAllocator tinyAllocator;
   SkBitmap bitmap;
   bitmap.setInfo(SkImageInfo::MakeN32(2, 2, kOpaque_SkAlphaType));
   if (bitmap.tryAllocPixels(&tinyAllocator)) {
       bitmap.eraseColor(0xff55aa33);
       bitmap.erase(0xffaa3355, SkIRect::MakeXYWH(1, 1, 1, 1));
       canvas->scale(16, 16);
       canvas->drawBitmap(bitmap, 0, 0);
   }
}
}  // END FIDDLE
