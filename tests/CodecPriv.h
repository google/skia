/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef CodecPriv_DEFINED
#define CodecPriv_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace SkCodecs {
// Defined in src/codec/SkCodec.cpp without a header declaration. Forward-declared here so tests can
// save and restore the global decoder registry without modifying production headers.
const std::vector<Decoder>& get_decoders();
}  // namespace SkCodecs

// Saves the global SkCodecs decoder registry on construction and restores it on destruction so
// tests that modify decoder registrations are isolated and order-independent.
class ScopedCodecDecoders {
public:
    ScopedCodecDecoders() : fSaved(SkCodecs::get_decoders()) {}
    ~ScopedCodecDecoders() {
        const_cast<std::vector<SkCodecs::Decoder>&>(SkCodecs::get_decoders()) = std::move(fSaved);
    }

    void clear() { const_cast<std::vector<SkCodecs::Decoder>&>(SkCodecs::get_decoders()).clear(); }

private:
    std::vector<SkCodecs::Decoder> fSaved;
};

static DEFINE_string(codecWritePath, "",
                     "Dump image decodes from codec unit tests here.");

inline bool decode_memory(const void* mem, size_t size, SkBitmap* bm) {
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(SkData::MakeWithoutCopy(mem, size)));
    if (!codec) {
        return false;
    }

    bm->allocPixels(codec->getInfo());
    const SkCodec::Result result = codec->getPixels(codec->getInfo(), bm->getPixels(),
            bm->rowBytes());
    return result == SkCodec::kSuccess || result == SkCodec::kIncompleteInput;
}

inline void write_bm(const char* name, const SkBitmap& bm) {
    if (FLAGS_codecWritePath.isEmpty()) {
        return;
    }

    SkString filename = SkOSPath::Join(FLAGS_codecWritePath[0], name);
    filename.appendf(".png");
    SkFILEWStream file(filename.c_str());
    if (!SkPngEncoder::Encode(&file, bm.pixmap(), {})) {
        SkDebugf("failed to write '%s'\n", filename.c_str());
    }
}

inline sk_sp<SkData> make_ico_with_png(const void* pngBytes, size_t pngSize) {
    constexpr size_t kHeaderSize = 22;
    SkDynamicMemoryWStream stream;

    const uint8_t icoHeader[6] = {
            0x00,
            0x00,  // Reserved
            0x01,
            0x00,  // Image type: 1 for ICO
            0x01,
            0x00,  // Number of images: 1
    };
    stream.write(icoHeader, sizeof(icoHeader));

    const uint32_t size32 = static_cast<uint32_t>(pngSize);
    const uint32_t offset32 = kHeaderSize;
    const uint8_t dirEntry[16] = {
            0,
            0,  // Width, Height (0 means 256)
            0,
            0,  // Color count, Reserved
            0x01,
            0x00,  // Color planes: 1
            0x20,
            0x00,  // Bits per pixel: 32
            static_cast<uint8_t>(size32 & 0xFF),
            static_cast<uint8_t>((size32 >> 8) & 0xFF),
            static_cast<uint8_t>((size32 >> 16) & 0xFF),
            static_cast<uint8_t>((size32 >> 24) & 0xFF),
            static_cast<uint8_t>(offset32 & 0xFF),
            static_cast<uint8_t>((offset32 >> 8) & 0xFF),
            static_cast<uint8_t>((offset32 >> 16) & 0xFF),
            static_cast<uint8_t>((offset32 >> 24) & 0xFF),
    };
    stream.write(dirEntry, sizeof(dirEntry));
    stream.write(pngBytes, pngSize);
    return stream.detachAsData();
}

inline sk_sp<SkData> make_ico_from_png_resource(skiatest::Reporter* r, const char* resource) {
    sk_sp<SkData> pngData = GetResourceAsData(resource);
    REPORTER_ASSERT(r, pngData);
    if (!pngData) {
        return nullptr;
    }
    sk_sp<SkData> icoData = make_ico_with_png(pngData->data(), pngData->size());
    REPORTER_ASSERT(r, icoData);
    return icoData;
}

#endif  // CodecPriv_DEFINED
