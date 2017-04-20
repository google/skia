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
     * |fQuality| must be in [0, 100] where 0 corresponds to the lowest quality.
     *
     */
    struct JPEG { int fQuality; };

    /**
     * Options that only affect WEBP encodes.
     * |fQuality| must be in [0.0f, 100.0f] where 0.0f corresponds to the lowest quality.
     */
    struct WEBP { float fQuality; };

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

    SkEncodeOptions(SkEncodedImageFormat format)
        : fUnpremulBehavior(SkTransferFunctionBehavior::kRespect)
    {
        switch (format) {
            case SkEncodedImageFormat::kJPEG:
                fFormat.fJPEG.fQuality = 100;
                break;
            case SkEncodedImageFormat::kWEBP:
                fFormat.fWEBP.fQuality = 100.0f;
                break;
            default:
                break;
        }
    }
};

/**
 * Encode SkPixmap in the given binary image format.
 *
 * @param  dst     results are written to this stream.
 * @param  src     source pixels.
 * @param  format  image format, not all formats are supported.
 * @param  options switches to enable encoder-specific behaviors.
 *
 * @return false iff input is bad or format is unsupported.
 *
 * Will always return false if Skia is compiled without image
 * encoders.
 */
SK_API bool SkEncodeImage(SkWStream* dst, const SkPixmap& src, SkEncodedImageFormat format,
                          const SkEncodeOptions& options);

SK_API sk_sp<SkData> SkEncodeImage(const SkPixmap& src, SkEncodedImageFormat format,
                                   const SkEncodeOptions& options);

static inline bool SkEncodeImage(SkWStream* dst, const SkPixmap& src, SkEncodedImageFormat format,
                                 int quality) {
    SkEncodeOptions opts(format);
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

static inline bool SkEncodeImage(SkWStream* dst, const SkBitmap& src, SkEncodedImageFormat format,
                                 int quality) {
    SkAutoLockPixels autoLockPixels(src);
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) && SkEncodeImage(dst, pixmap, f, q);
}

#endif  // SkImageEncoder_DEFINED
