/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/EncodeUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/encode/SkPngEncoder.h"
#include "src/base/SkBase64.h"

#include <cstddef>
#include <cstdint>

namespace ToolUtils {

bool BitmapToBase64DataURI(const SkBitmap& bitmap, SkString* dst) {
    SkPixmap pm;
    if (!bitmap.peekPixels(&pm)) {
        dst->set("peekPixels failed");
        return false;
    }

    // We're going to embed this PNG in a data URI, so make it as small as possible
    SkPngEncoder::Options options;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
    options.fZLibLevel = 9;

    SkDynamicMemoryWStream wStream;
    if (!SkPngEncoder::Encode(&wStream, pm, options)) {
        dst->set("SkPngEncoder::Encode failed");
        return false;
    }

    sk_sp<SkData> pngData = wStream.detachAsData();
    size_t len = SkBase64::EncodedSize(pngData->size());

    // The PNG can be almost arbitrarily large. We don't want to fill our logs with enormous URLs.
    // Infra says these can be pretty big, as long as we're only outputting them on failure.
    static const size_t kMaxBase64Length = 1024 * 1024;
    if (len > kMaxBase64Length) {
        dst->printf("Encoded image too large (%u bytes)", static_cast<uint32_t>(len));
        return false;
    }

    dst->resize(len);
    SkBase64::Encode(pngData->data(), pngData->size(), dst->data());
    dst->prepend("data:image/png;base64,");
    return true;
}

bool EncodeImageToPngFile(const char* path, const SkBitmap& src) {
    SkFILEWStream file(path);
    return file.isValid() && SkPngEncoder::Encode(&file, src.pixmap(), {});
}

bool EncodeImageToPngFile(const char* path, const SkPixmap& src) {
    SkFILEWStream file(path);
    return file.isValid() && SkPngEncoder::Encode(&file, src, {});
}

}  // namespace ToolUtils
