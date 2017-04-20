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
// This class emits a skiatest failure if a client attempts to read beyond its
// end. Since it is used with complete, valid images, and contains nothing
// after the encoded image data, it will emit a failure if the client attempts
// to read beyond the logical end of the data.
class MyStream : public SkStream {
public:
    MyStream(sk_sp<SkData> data, skiatest::Reporter* r)
        : fStream(std::move(data))
        , fReporter(r)
    {}

    size_t read(void* buf, size_t bytes) override {
        const size_t remaining = fStream.getLength() - fStream.getPosition();
        REPORTER_ASSERT(fReporter, bytes <= remaining);
        return fStream.read(buf, bytes);
    }

    bool rewind() override {
        return fStream.rewind();
    }

    bool isAtEnd() const override {
        return fStream.isAtEnd();
    }
private:
    SkMemoryStream fStream;
    skiatest::Reporter* fReporter;  // Unowned
};
} // namespace

// Test that SkPngCodec does not attempt to read its input beyond the logical
// end of its data. Some other SkCodecs do, but some Android apps rely on not
// doing so for PNGs.
DEF_TEST(Codec_end, r) {
    for (const char* path : { "plane.png",
                              "yellow_rose.png",
                              "plane_interlaced.png" }) {
        sk_sp<SkData> data(GetResourceAsData(path));
        if (!data) {
            return;
        }

        std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(new MyStream(std::move(data), r)));
        if (!codec) {
            ERRORF(r, "Failed to create a codec from %s\n", path);
            return;
        }

        auto info = codec->getInfo().makeColorType(kN32_SkColorType);
        SkBitmap bm;
        bm.allocPixels(info);

        auto result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes());
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);

        // Rewind and do an incremental decode.
        result = codec->startIncrementalDecode(bm.info(), bm.getPixels(), bm.rowBytes());
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);

        result = codec->incrementalDecode();
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    }
}
