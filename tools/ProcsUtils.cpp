/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/ProcsUtils.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "src/image/SkImage_Base.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
#include "include/encode/SkPngRustEncoder.h"
#else
#include "include/encode/SkPngEncoder.h"
#endif
#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
#include "include/codec/SkPngRustDecoder.h"
#else
#include "include/codec/SkPngDecoder.h"
#endif

namespace ToolUtils {

SkDeserialProcs default_deserial_procs() {
    SkDeserialProcs procs;
    procs.fImageDataProc =
            [](sk_sp<SkData> data, std::optional<SkAlphaType> at, void*) -> sk_sp<SkImage> {
#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
        std::unique_ptr<SkStream> stream = SkMemoryStream::Make(data);
        auto codec = SkPngRustDecoder::Decode(std::move(stream), nullptr, nullptr);
#else
        auto codec = SkPngDecoder::Decode(data, nullptr, nullptr);
#endif
        if (!codec) {
            SkDebugf("Invalid png data detected\n");
            return nullptr;
        }
        if (auto lazyImage = SkCodecs::DeferredImage(std::move(codec), at)) {
            return lazyImage->makeRasterImage(/*GrDirectContext=*/nullptr);
        }
        return nullptr;
    };

    // SKPs may have typefaces encoded in them (e.g. with FreeType). We can try falling back
    // to the Test FontMgr (possibly a native one) if we have do not have FreeType built-in.
    procs.fTypefaceStreamProc = [](SkStream& stream, void*) -> sk_sp<SkTypeface> {
        return SkTypeface::MakeDeserialize(&stream, ToolUtils::TestFontMgr());
    };
    return procs;
}

SkSerialProcs default_serial_procs() {
    static SkSerialProcs procs;

    procs.fImageProc = [](SkImage* img, void*) -> sk_sp<const SkData> {
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
        return SkPngRustEncoder::Encode(nullptr, img, {});
#else
        // TODO: This catches SkImageEncoder_NDK (or other).
        return SkPngEncoder::Encode(nullptr, img, {});
#endif
    };

    return procs;
}

}  // namespace ToolUtils
