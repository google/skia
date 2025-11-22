/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkPngRustEncoder.h"

#include <memory>

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/encode/SkEncoder.h"
#include "src/encode/SkPngRustEncoderImpl.h"
#include "src/image/SkImage_Base.h"

namespace SkPngRustEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    std::unique_ptr<SkEncoder> encoder = Make(dst, src, options);
    return encoder && encoder->encodeRows(src.height());
}

sk_sp<SkData> Encode(const SkPixmap& src, const Options& options) {
    SkDynamicMemoryWStream stream;
    if (!Encode(&stream, src, options)) {
        return nullptr;
    }
    return stream.detachAsData();
}

sk_sp<SkData> Encode(GrDirectContext* ctx, const SkImage* img, const Options& options) {
    if (!img) {
        return nullptr;
    }

    SkBitmap bm;
    if (!as_IB(img)->getROPixels(ctx, &bm)) {
        return nullptr;
    }

    return Encode(bm.pixmap(), options);
}

std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src, const Options& options) {
    return SkPngRustEncoderImpl::Make(dst, src, options);
}

}  // namespace SkPngRustEncoder
