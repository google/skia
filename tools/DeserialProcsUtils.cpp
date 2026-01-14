/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/DeserialProcsUtils.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
#include "include/codec/SkPngRustDecoder.h"
#else
#include "include/codec/SkPngDecoder.h"
#endif

namespace ToolUtils {

SkDeserialProcs get_default_skp_deserial_procs() {
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

}  // namespace ToolUtils

