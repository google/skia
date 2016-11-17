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
#include "SkPixelSerializer.h"
#include "SkStream.h"

// #ifndef SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS
// #define SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS
// #endif

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

#ifdef SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS

////////////////////////////////////////////////////////////////////////////////

#include "SkImageInfo.h"
#include "SkTRegistry.h"

class SkBitmap;
class SkPixelSerializer;
class SkPixmap;
class SkData;
class SkWStream;

class SkImageEncoder {
public:
    // TODO (scroggo): Merge with SkEncodedImageFormat.
    enum Type : uint8_t {
        kBMP_Type     = (uint8_t)SkEncodedImageFormat::kBMP,
        kGIF_Type     = (uint8_t)SkEncodedImageFormat::kGIF,
        kICO_Type     = (uint8_t)SkEncodedImageFormat::kICO,
        kJPEG_Type    = (uint8_t)SkEncodedImageFormat::kJPEG,
        kPNG_Type     = (uint8_t)SkEncodedImageFormat::kPNG,
        kWBMP_Type    = (uint8_t)SkEncodedImageFormat::kWBMP,
        kWEBP_Type    = (uint8_t)SkEncodedImageFormat::kWEBP,
        kKTX_Type     = (uint8_t)SkEncodedImageFormat::kKTX,
    };
    static SkImageEncoder* Create(Type);

    virtual ~SkImageEncoder() {}

    /*  Quality ranges from 0..100 */
    enum {
        kDefaultQuality = 80
    };

    /**
     *  Encode bitmap 'bm', returning the results in an SkData, at quality level
     *  'quality' (which can be in range 0-100). If the bitmap cannot be
     *  encoded, return null. On success, the caller is responsible for
     *  calling unref() on the data when they are finished.
     */
    SkData* encodeData(const SkBitmap& bm, int quality) {
        SkDynamicMemoryWStream buffer;
        return this->encodeStream(&buffer, bm, quality)
               ? buffer.detachAsData().release()
               : nullptr;
    }

    /**
     * Encode bitmap 'bm' in the desired format, writing results to
     * file 'file', at quality level 'quality' (which can be in range
     * 0-100). Returns false on failure.
     */
    bool encodeFile(const char path[], const SkBitmap& bm, int quality) {
        SkFILEWStream file(path);
        return this->encodeStream(&file, bm, quality);
    }

    /**
     * Encode bitmap 'bm' in the desired format, writing results to
     * stream 'stream', at quality level 'quality' (which can be in
     * range 0-100). Returns false on failure.
     */
    bool encodeStream(SkWStream* dst, const SkBitmap& src, int quality) {
        return this->onEncode(dst, src, SkMin32(100, SkMax32(0, quality)));
    }

    static SkData* EncodeData(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                              Type t, int quality) {
        SkPixmap pixmap(info, pixels, rowBytes, nullptr);
        return SkImageEncoder::EncodeData(pixmap, t, quality);
    }

    static SkData* EncodeData(const SkBitmap& src, Type t, int quality) {
        SkDynamicMemoryWStream buf;
        return SkEncodeImage(&buf, src, (SkEncodedImageFormat)t, quality)
            ? buf.detachAsData().release() : nullptr;
    }

    static SkData* EncodeData(const SkPixmap& pixmap, Type t, int quality) {
        SkDynamicMemoryWStream buf;
        return SkEncodeImage(&buf, pixmap, (SkEncodedImageFormat)t, quality)
            ? buf.detachAsData().release() : nullptr;
    }

    static bool EncodeFile(const char path[], const SkBitmap& src, Type t, int quality) {
        SkFILEWStream file(path);
        return SkEncodeImage(&file, src, (SkEncodedImageFormat)t, quality);
    }
    static bool EncodeStream(SkWStream* dst, const SkBitmap& bm, Type t, int quality) {
        return SkEncodeImage(dst, bm, (SkEncodedImageFormat)t, quality);
    }

protected:
    /**
     * Encode bitmap 'bm' in the desired format, writing results to
     * stream 'stream', at quality level 'quality' (which can be in
     * range 0-100).
     *
     * This must be overridden by each SkImageEncoder implementation.
     */
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) = 0;
};

// This macro declares a global (i.e., non-class owned) creation entry point
// for each encoder (e.g., CreateJPEGImageEncoder)
#define DECLARE_ENCODER_CREATOR(codec)          \
    SK_API SkImageEncoder *Create ## codec ();

// This macro defines the global creation entry point for each encoder. Each
// encoder implementation that registers with the encoder factory must call it.
#define DEFINE_ENCODER_CREATOR(codec) \
    SkImageEncoder* Create##codec() { return new Sk##codec; }

// All the encoders known by Skia. Note that, depending on the compiler settings,
// not all of these will be available
DECLARE_ENCODER_CREATOR(JPEGImageEncoder);
DECLARE_ENCODER_CREATOR(PNGImageEncoder);
DECLARE_ENCODER_CREATOR(KTXImageEncoder);
DECLARE_ENCODER_CREATOR(WEBPImageEncoder);

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
SkImageEncoder* CreateImageEncoder_CG(SkImageEncoder::Type type);
#endif

#if defined(SK_BUILD_FOR_WIN)
SkImageEncoder* CreateImageEncoder_WIC(SkImageEncoder::Type type);
#endif

// Typedef to make registering encoder callback easier
// This has to be defined outside SkImageEncoder. :(
typedef SkTRegistry<SkImageEncoder*(*)(SkImageEncoder::Type)> SkImageEncoder_EncodeReg;

#endif  // SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS
#endif  // SkImageEncoder_DEFINED
