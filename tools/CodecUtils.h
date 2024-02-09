/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef CodecUtils_DEFINED
#define CodecUtils_DEFINED
#include "include/codec/SkCodec.h"

#if defined(SK_CODEC_DECODES_AVIF)
#include "include/codec/SkAvifDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_BMP)
#include "include/codec/SkBmpDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_GIF)
#include "include/codec/SkGifDecoder.h"
#endif

#if defined(SK_HAS_HEIF_LIBRARY)
#include "include/android/SkHeifDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_ICO)
#include "include/codec/SkIcoDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_JPEG)
#include "include/codec/SkJpegDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_JPEGXL)
#include "include/codec/SkJpegxlDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_PNG)
#include "include/codec/SkPngDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_RAW)
#include "include/codec/SkRawDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_WBMP)
#include "include/codec/SkWbmpDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_WEBP)
#include "include/codec/SkWebpDecoder.h"
#endif

namespace CodecUtils {
// Register all codecs which were compiled in. Our modular codecs set a define to signal if they
// were compiled in or not. It is safe to call this more than once, as the SkCodecs::Register
// function is idempotent. This function *cannot* go in src/ (e.g. as part of Skia proper) because
// then Skia itself would need to depend on codecs, which we want to avoid.
inline void RegisterAllAvailable() {
#if defined(SK_CODEC_DECODES_AVIF)
    SkCodecs::Register(SkAvifDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_BMP)
    SkCodecs::Register(SkBmpDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_GIF)
    SkCodecs::Register(SkGifDecoder::Decoder());
#endif
#if defined(SK_HAS_HEIF_LIBRARY)
    SkCodecs::Register(SkHeifDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_ICO)
    SkCodecs::Register(SkIcoDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_JPEG)
    SkCodecs::Register(SkJpegDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_JPEGXL)
    SkCodecs::Register(SkJpegxlDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_PNG)
    SkCodecs::Register(SkPngDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_RAW)
    SkCodecs::Register(SkRawDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_WBMP)
    SkCodecs::Register(SkWbmpDecoder::Decoder());
#endif
#if defined(SK_CODEC_DECODES_WEBP)
    SkCodecs::Register(SkWebpDecoder::Decoder());
#endif
}

}  // namespace CodecUtils

#endif
