/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoder_DEFINED
#define SkImageEncoder_DEFINED

#include "SkBitmap.h"
#include "SkEncodedImageFormat.h"
#include "SkStream.h"

/**
 * Various options to control encode behavior.
 */
struct SkEncodeOptions {
    /**
     * Options that only affect JPEG encodes.
     */
    struct JPEG { int fQuality; };

    /**
     * Options that only affect WEBP encodes.
     */
    struct WEBP { int fQuality; };

    /**
     * Options that only affect PNG encodes.
     */
    struct PNG {};

    union Format {
        JPEG fJPEG;
        WEBP fWEBP;
        PNG  fPNG;
    };

    /**
     * Container for format specific options.
     */
    Format                     fFormat;

    /**
     * If the input is premultiplied, this controls the unpremultiplication behavior.
     * The encoder can convert to linear before unpremultiplying or ignore the transfer
     * function and unpremultiply the input as is.
     */
    SkTransferFunctionBehavior fUnpremulBehavior;

    SkEncodeOptions()
        : fUnpremulBehavior(SkTransferFunctionBehavior::kRespect)
    {
        sk_bzero(&fFormat, sizeof(fFormat));
    }
};

/**
 * Encode SkPixmap in the given binary image format.
 *
 * @param  dst     results are written to this stream.
 * @param  src     source pixels.
 * @param  format  image format, not all formats are supported.
 * @param  options switches to enable specific encode behaviors.
 *
 * @return false iff input is bad or format is unsupported.
 *
 * Will always return false if Skia is compiled without image
 * encoders.
 */
SK_API bool SkEncodeImage(SkWStream* dst, const SkPixmap& src, SkEncodedImageFormat format,
                          const SkEncodeOptions& options);

SK_API inline sk_sp<SkData> SkEncodeImage(const SkPixmap& src, SkEncodedImageFormat format,
                                          const SkEncodeOptions& options) {
    SkDynamicMemoryWStream dst;
    if (SkEncodeImage(&dst, src, format, options)) {
        return dst.detachAsData();
    }

    return nullptr;
}

SK_API inline bool SkEncodeImage(SkWStream* dst, const SkPixmap& src, SkEncodedImageFormat format,
                                 int quality) {
    SkEncodeOptions opts;
    opts.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            opts.fFormat.fJPEG.fQuality = quality;
            break;
        case SkEncodedImageFormat::kWEBP:
            opts.fFormat.fWEBP.fQuality = (float) quality;
            break;
        default:
            break;
    }

    return SkEncodeImage(dst, src, format, opts);
}

inline bool SkEncodeImage(SkWStream* dst, const SkBitmap& src, SkEncodedImageFormat f, int q) {
    SkAutoLockPixels autoLockPixels(src);
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) && SkEncodeImage(dst, pixmap, f, q);
}

#endif  // SkImageEncoder_DEFINED
