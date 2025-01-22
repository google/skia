/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFJPEGHelpers_DEFINED
#define SkPDFJPEGHelpers_DEFINED

#include "include/codec/SkJpegDecoder.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/docs/SkPDFDocument.h"
#include "include/encode/SkJpegEncoder.h"

class SkPixmap;
class SkWStream;

#include <memory>

namespace SkPDF::JPEG {
inline std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data) {
    return SkJpegDecoder::Decode(data, nullptr, nullptr);
}

inline bool Encode(SkWStream* dst, const SkPixmap& src, int quality) {
    SkJpegEncoder::Options jOpts;
    jOpts.fQuality = quality;
    return SkJpegEncoder::Encode(dst, src, jOpts);
}

inline SkPDF::Metadata MetadataWithCallbacks() {
    SkPDF::Metadata m;
    m.jpegDecoder = SkPDF::JPEG::Decode;
    m.jpegEncoder = SkPDF::JPEG::Encode;
    return m;
}

}  // namespace SkPDF::JPEG

#endif
