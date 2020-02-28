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

#ifndef SK_ENCODE_JPEG
bool SkJpegEncoder::Encode(SkWStream*, const SkPixmap&, const Options&) { return false; }
std::unique_ptr<SkEncoder> SkJpegEncoder::Make(SkWStream*, const SkPixmap&, const Options&) {
    return nullptr;
}
#endif

#ifndef SK_ENCODE_PNG
bool SkPngEncoder::Encode(SkWStream*, const SkPixmap&, const Options&) { return false; }
std::unique_ptr<SkEncoder> SkPngEncoder::Make(SkWStream*, const SkPixmap&, const Options&) {
    return nullptr;
}
#endif

#ifndef SK_ENCODE_WEBP
bool SkWebpEncoder::Encode(SkWStream*, const SkPixmap&, const Options&) { return false; }
#endif

bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                   SkEncodedImageFormat format, int quality) {
    #ifdef SK_USE_CG_ENCODER
        (void)quality;
        return SkEncodeImageWithCG(dst, src, format);
    #elif SK_USE_WIC_ENCODER
        return SkEncodeImageWithWIC(dst, src, format, quality);
    #else
        switch(format) {
            case SkEncodedImageFormat::kJPEG: {
                SkJpegEncoder::Options opts;
                opts.fQuality = quality;
                return SkJpegEncoder::Encode(dst, src, opts);
            }
            case SkEncodedImageFormat::kPNG: {
                SkPngEncoder::Options opts;
                return SkPngEncoder::Encode(dst, src, opts);
            }
            case SkEncodedImageFormat::kWEBP: {
                SkWebpEncoder::Options opts;
                if (quality == 100) {
                    opts.fCompression = SkWebpEncoder::Compression::kLossless;
                    // Note: SkEncodeImage treats 0 quality as the lowest quality
                    // (greatest compression) and 100 as the highest quality (least
                    // compression). For kLossy, this matches libwebp's
                    // interpretation, so it is passed directly to libwebp. But
                    // with kLossless, libwebp always creates the highest quality
                    // image. In this case, fQuality is reinterpreted as how much
                    // effort (time) to put into making a smaller file. This API
                    // does not provide a way to specify this value (though it can
                    // be specified by using SkWebpEncoder::Encode) so we have to
                    // pick one arbitrarily. This value matches that chosen by
                    // blink::ImageEncoder::ComputeWebpOptions as well
                    // WebPConfigInit.
                    opts.fQuality = 75;
                } else {
                    opts.fCompression = SkWebpEncoder::Compression::kLossy;
                    opts.fQuality = quality;
                }
                return SkWebpEncoder::Encode(dst, src, opts);
            }
            default:
                return false;
        }
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
