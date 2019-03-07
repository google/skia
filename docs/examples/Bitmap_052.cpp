// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=eb6f861ca1839146d26e40d56c2a001c
REG_FIDDLE(Bitmap_052, 256, 100, false, 0) {
class LargePixelRef : public SkPixelRef {
public:
    LargePixelRef(const SkImageInfo& info, char* storage, size_t rowBytes)
        : SkPixelRef(info.width(), info.height(), storage, rowBytes) {
    }
    ~LargePixelRef() override {
        delete[] (char* ) this->pixels();
    }
};
class LargeAllocator : public SkBitmap::Allocator {
public:
    bool allocPixelRef(SkBitmap* bitmap) override {
        const SkImageInfo& info = bitmap->info();
        uint64_t rowBytes = info.minRowBytes64();
        uint64_t size = info.height() * rowBytes;
        char* addr = new char[size];
        if (nullptr == addr) {
            return false;
        }
        sk_sp<SkPixelRef> pr = sk_sp<SkPixelRef>(new LargePixelRef(info, addr, rowBytes));
        if (!pr) {
            return false;
        }
        bitmap->setPixelRef(std::move(pr), 0, 0);
        return true;
    }
};

void draw(SkCanvas* canvas) {
   LargeAllocator largeAllocator;
   SkBitmap bitmap;
   int width = 100; // make this 20000
   int height = 100; // and this 100000 to allocate 8 gigs on a 64-bit platform
   bitmap.setInfo(SkImageInfo::MakeN32(width, height, kOpaque_SkAlphaType));
   if (bitmap.tryAllocPixels(&largeAllocator)) {
       bitmap.eraseColor(0xff55aa33);
       canvas->drawBitmap(bitmap, 0, 0);
   }
}
}  // END FIDDLE
