// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkImage_to_PPM_binary, 256, 256, true, 4) {
void print_data(const SkData* data, const char* name) {
    if (data) {
        SkDebugf("\nxxd -r -p > %s << EOF", name);
        size_t s = data->size();
        const uint8_t* d = data->bytes();
        for (size_t i = 0; i < s; ++i) {
            if (i % 40 == 0) {
                SkDebugf("\n");
            }
            SkDebugf("%02x", d[i]);
        }
        SkDebugf("\nEOF\n\n");
    }
}

sk_sp<SkData> Encode_PPM_B(const SkPixmap& src) {
    if (src.width() <= 0 || src.height() <= 0 || !src.addr() ||
        src.colorType() == kUnknown_SkColorType) {
        return nullptr;
    }
    SkString s = SkStringPrintf("P6\n%d %d\n255\n", src.width(), src.height());
    auto result = SkData::MakeUninitialized(s.size() + 3 * src.width() * src.height());
    uint8_t* ptr = static_cast<uint8_t*>(result->writable_data());
    memcpy(ptr, s.c_str(), s.size());
    ptr += s.size();
    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.height(); ++x) {
            SkColor c = src.getColor(x, y);
            *ptr++ = SkColorGetR(c);
            *ptr++ = SkColorGetG(c);
            *ptr++ = SkColorGetB(c);
        }
    }
    return result;
}

SkBitmap ToBitmap(SkImage * img) {
    SkBitmap bitmap;
    (void)img->asLegacyBitmap(&bitmap, SkImage::kRO_LegacyBitmapMode);
    return bitmap;
}

void draw(SkCanvas*) {
    SkBitmap bitmap = ToBitmap(image.get());
    sk_sp<SkData> data = Encode_PPM_B(bitmap.pixmap());
    print_data(data.get(), "foo.ppm");
}
}  // END FIDDLE
