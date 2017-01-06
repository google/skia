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
 * Encode SkPixmap in the given binary image format.
 *
 * @param  dst     results are written to this stream.
 * @param  src     source pixels.
 * @param  format  image format, not all formats are supported.
 * @param  quality range from 0-100, not all formats respect quality.
 *
 * @return false iff input is bad or format is unsupported.
 *
 * Will always return false if Skia is compiled without image
 * encoders.
 *
 * For examples of encoding an image to a file or to a block of memory,
 * see tools/sk_tool_utils.h.
 */
SK_API bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                          SkEncodedImageFormat format, int quality);
/**
 * The following helper function wraps SkEncodeImage().
 */
inline bool SkEncodeImage(SkWStream* dst, const SkBitmap& src, SkEncodedImageFormat f, int q) {
    SkAutoLockPixels autoLockPixels(src);
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) && SkEncodeImage(dst, pixmap, f, q);
}

SK_API struct SkEncodeOptions {
    enum class Fidelity {
        // If the selected image format supports lossless encoding, choose a lossless encode.
        // If the format is lossy, encode at the highest quality.
        kPreferLossless,

        // Use lossy encoding if it is supported by the selected image format.
        kPreferLossy,
    };

    enum class BitDepth {
        // If possible, maintain the bit depth of the input.
        kAsIs,

        // Encode at a maximum bit depth of 8.
        kShrinkTo8,
    };

    enum class PremulBehavior {
         // Convert to a linear space before premultiplying or unpremultiplying.
        kGammaCorrect,

        // Ignore the transfer function when premultiplying or unpremultiplying.
        kLegacy,
    };

    SkEncodedImageFormat fFormat;
    Fidelity             fFidelity;
    BitDepth             fBitDepth;
    PremulBehavior       fPremulBehavior;

    SkEncodeOptions()
        : fFormat(SkEncodedImageFormat::kPNG)
        , fFidelity(Fidelity::kPreferLossless)
        , fBitDepth(BitDepth::kAsIs)
        , fPremulBehavior(PremulBehavior::kGammaCorrect)
    {}
};

/**
 * Encode |src| in the given binary image format.
 *
 * @param  src     Pixels to encode.
 * @param  opts    Options for the encode.
 *
 * @return         On success, returns the encoded data.  On failure, returns nullptr.
 */
SK_API sk_sp<SkData> SkEncodeImage(const SkPixmap& src, const SkEncodeOptions& opts);

//TODO(halcanary):  remove this code once all changes land.
#ifdef SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS
class SkImageEncoder {
public:
    enum Type {
#ifdef GOOGLE3
        kUnknown_Type = (int)SkEncodedImageFormat::kUnknown,
#endif
        kBMP_Type     = (int)SkEncodedImageFormat::kBMP,
        kGIF_Type     = (int)SkEncodedImageFormat::kGIF,
        kICO_Type     = (int)SkEncodedImageFormat::kICO,
        kJPEG_Type    = (int)SkEncodedImageFormat::kJPEG,
        kPNG_Type     = (int)SkEncodedImageFormat::kPNG,
        kWBMP_Type    = (int)SkEncodedImageFormat::kWBMP,
        kWEBP_Type    = (int)SkEncodedImageFormat::kWEBP,
        kKTX_Type     = (int)SkEncodedImageFormat::kKTX,
    };
    static SkData* EncodeData(const SkBitmap& src, Type t, int quality) {
        SkDynamicMemoryWStream buf;
        return SkEncodeImage(&buf, src, (SkEncodedImageFormat)t, quality)
            ? buf.detachAsData().release() : nullptr;
    }
    static bool EncodeFile(const char path[], const SkBitmap& src, Type t, int quality) {
        SkFILEWStream file(path);
        return SkEncodeImage(&file, src, (SkEncodedImageFormat)t, quality);
    }
    static bool EncodeStream(SkWStream* dst, const SkBitmap& bm, Type t, int quality) {
        return SkEncodeImage(dst, bm, (SkEncodedImageFormat)t, quality);
    }
};
#endif  // SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS

#endif  // SkImageEncoder_DEFINED
