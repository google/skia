/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "Test.h"

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkStream.h"

namespace {
// This class wraps another SkStream. It does not own the underlying stream, so
// that the underlying stream can be reused starting from where the first
// client left off. This mimics Android's JavaInputStreamAdaptor.
class UnowningStream : public SkStream {
public:
    explicit UnowningStream(SkStream* stream)
        : fStream(stream)
    {}

    size_t read(void* buf, size_t bytes) override {
        return fStream->read(buf, bytes);
    }

    bool rewind() override {
        return fStream->rewind();
    }

    bool isAtEnd() const override {
        return fStream->isAtEnd();
    }
private:
    SkStream* fStream; // Unowned.
};
} // namespace

// Test that some SkCodecs do not attempt to read input beyond the logical
// end of the data. Some other SkCodecs do, but some Android apps rely on not
// doing so for PNGs. Test on other formats that work.
DEF_TEST(Codec_end, r) {
    for (const char* path : { "plane.png",
                              "yellow_rose.png",
                              "plane_interlaced.png",
                              "google_chrome.ico",
                              "color_wheel.ico",
                              "mandrill.wbmp",
                              "randPixels.bmp",
                              }) {
        sk_sp<SkData> data = GetResourceAsData(path);
        if (!data) {
            continue;
        }

        const int kNumImages = 2;
        const size_t size = data->size();
        sk_sp<SkData> multiData = SkData::MakeUninitialized(size * kNumImages);
        void* dst = multiData->writable_data();
        for (int i = 0; i < kNumImages; i++) {
            memcpy(SkTAddOffset<void>(dst, size * i), data->data(), size);
        }
        data.reset();

        SkMemoryStream stream(std::move(multiData));
        for (int i = 0; i < kNumImages; ++i) {
            std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(new UnowningStream(&stream)));
            if (!codec) {
                ERRORF(r, "Failed to create a codec from %s, iteration %i", path, i);
                continue;
            }

            auto info = codec->getInfo().makeColorType(kN32_SkColorType);
            SkBitmap bm;
            bm.allocPixels(info);

            auto result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes());
            if (result != SkCodec::kSuccess) {
                ERRORF(r, "Failed to getPixels from %s, iteration %i error %i", path, i, result);
                continue;
            }
        }
    }
}
