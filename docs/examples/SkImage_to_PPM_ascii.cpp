// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkImage_to_PPM_ascii, 256, 256, true, 4) {
void dump_txt(const SkData* data, const char* name) {
    if (data) {
        SkDebugf("\ncat > %s << EOF\n", name);
        size_t s = data->size();
        const char* d = (const char*)data->bytes();
        while (s > 0) {
            int l = (int)std::min(s, (size_t)1024);
            SkDebugf("%.*s", l, d);
            s -= l;
            d += l;
        }
        SkDebugf("\nEOF\n\n");
    }
}

sk_sp<SkData> Encode_PPM_A(const SkPixmap& src) {
    if (src.width() <= 0 || src.height() <= 0 || !src.addr() ||
        src.colorType() == kUnknown_SkColorType) {
        return nullptr;
    }
    SkDynamicMemoryWStream buf;
    SkString s = SkStringPrintf("P3\n%d %d\n255\n", src.width(), src.height());
    buf.write(s.c_str(), s.size());
    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.height(); ++x) {
            char buffer[13];
            SkColor c = src.getColor(x, y);
            int n = snprintf(buffer, sizeof(buffer), "%u %u %u\n", SkColorGetR(c),
                             SkColorGetG(c), SkColorGetB(c));
            if (n < 6 || n + 1 > (int)sizeof(buffer)) {
                return nullptr;
            }
            buf.write(buffer, n);
        }
    }
    return buf.detachAsData();
}

SkBitmap ToBitmap(SkImage * img) {
    SkBitmap bitmap;
    (void)img->asLegacyBitmap(&bitmap, SkImage::kRO_LegacyBitmapMode);
    return bitmap;
}

void draw(SkCanvas*) {
    SkBitmap bitmap = ToBitmap(image.get());
    sk_sp<SkData> data = Encode_PPM_A(bitmap.pixmap());
    dump_txt(data.get(), "fooa.ppm");
}
}  // END FIDDLE
