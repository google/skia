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
    static MyStream* Make(const char* path, skiatest::Reporter* r) {
        SkASSERT(path);
        sk_sp<SkData> data(GetResourceAsData(path));
        if (!data) {
            return nullptr;
        }

        return new MyStream(path, std::move(data), r);
    }

    size_t read(void* buf, size_t bytes) override {
        const size_t remaining = fStream.getLength() - fStream.getPosition();
        if (bytes > remaining) {
            ERRORF(fReporter, "Tried to read %lu bytes (only %lu remaining) from %s",
                   bytes, remaining, fPath);
        }
        return fStream.read(buf, bytes);
    }

    bool rewind() override {
        return fStream.rewind();
    }

    bool isAtEnd() const override {
        return fStream.isAtEnd();
    }
private:
    const char* fPath;
    SkMemoryStream fStream;
    skiatest::Reporter* fReporter;  // Unowned

    MyStream(const char* path, sk_sp<SkData> data, skiatest::Reporter* r)
        : fPath(path)
        , fStream(std::move(data))
        , fReporter(r)
    {}
};
} // namespace

// Test that SkPngCodec does not attempt to read its input beyond the logical
// end of its data. Some other SkCodecs do, but some Android apps rely on not
// doing so for PNGs.
DEF_TEST(Codec_end, r) {
    for (const char* path : { "plane.png",
                              "yellow_rose.png",
                              "plane_interlaced.png" }) {
        std::unique_ptr<MyStream> stream(MyStream::Make(path, r));
        if (!stream) {
            continue;
        }

        std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(stream.release()));
        if (!codec) {
            ERRORF(r, "Failed to create a codec from %s\n", path);
            continue;
        }

        auto info = codec->getInfo().makeColorType(kN32_SkColorType);
        SkBitmap bm;
        bm.allocPixels(info);

        auto result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes());
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to getPixels from %s. error %i", path, result);
            continue;
        }

        // Rewind and do an incremental decode.
        result = codec->startIncrementalDecode(bm.info(), bm.getPixels(), bm.rowBytes());
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to startIncrementalDecode from %s. error %i", path, result);
            continue;
        }

        result = codec->incrementalDecode();
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to incrementalDecode from %s. error %i", path, result);
        }
    }
}
