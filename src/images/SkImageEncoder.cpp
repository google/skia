/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "src/images/SkImageEncoderPriv.h"
#include <vector>

struct EncoderProc {
    SkEncodedImageFormat format;
    bool (*encode)(SkWStream*, const SkPixmap&, int quality);
};

static std::vector<EncoderProc>* encoders() {
    static auto* encoders = new std::vector<EncoderProc> {
    #ifdef SK_HAS_JPEG_LIBRARY
        { SkEncodedImageFormat::kJPEG, SkJpegEncoder::Encode },
    #endif

    #ifdef SK_HAS_PNG_LIBRARY
        { SkEncodedImageFormat::kPNG, SkPngEncoder::Encode },
    #endif

    #ifdef SK_HAS_WEBP_LIBRARY
        { SkEncodedImageFormat::kWEBP, SkWebpEncoder::Encode },
    #endif
    };
    return encoders;
}

void SkRegisterEncoder(SkEncodedImageFormat format,
                       bool (*encode)(SkWStream*, const SkPixmap&, int)) {
    encoders()->push_back({format, encode});
}

bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                   SkEncodedImageFormat format, int quality) {
    #ifdef SK_USE_CG_ENCODER
        (void)quality;
        return SkEncodeImageWithCG(dst, src, format);
    #elif SK_USE_WIC_ENCODER
        return SkEncodeImageWithWIC(dst, src, format, quality);
    #else
        for (auto encoder : *encoders()) {
            if (encoder.format == format) {
                return encoder.encode(dst, src, quality);
            }
        }
        return false;
    #endif
}

bool SkEncoder::encodeRows(int numRows) {
    SkASSERT(numRows > 0 && fCurrRow < fSrc.height());
    if (numRows <= 0 || fCurrRow >= fSrc.height()) {
        return false;
    }

    if (fCurrRow + numRows > fSrc.height()) {
        numRows = fSrc.height() - fCurrRow;
    }

    if (!this->onEncodeRows(numRows)) {
        // If we fail, short circuit any future calls.
        fCurrRow = fSrc.height();
        return false;
    }

    return true;
}

sk_sp<SkData> SkEncodePixmap(const SkPixmap& src, SkEncodedImageFormat format, int quality) {
    SkDynamicMemoryWStream stream;
    return SkEncodeImage(&stream, src, format, quality) ? stream.detachAsData() : nullptr;
}

sk_sp<SkData> SkEncodeBitmap(const SkBitmap& src, SkEncodedImageFormat format, int quality) {
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) ? SkEncodePixmap(pixmap, format, quality) : nullptr;
}
