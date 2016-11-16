/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoder_DEFINED
#define SkImageEncoder_DEFINED

#include "SkBitmap.h"
#include "SkEncodedFormat.h"
#include "SkPixelSerializer.h"
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
 * Will always return false if Skia is compiles without image
 * encoders.
 */
SK_API bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                          SkEncodedFormat format, int quality);
/**
 * The following five helper functions wrap SkEncodeImage().
 */
inline bool SkEncodeImage(SkWStream* dst, const SkBitmap& src, SkEncodedFormat f, int q) {
    SkAutoLockPixels autoLockPixels(src);
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) && SkEncodeImage(dst, pixmap, f, q);
}
inline bool SkEncodeImageToFile(const char* path, const SkPixmap& src, SkEncodedFormat f, int q) {
    SkFILEWStream file(path);
    return file.isValid() && SkEncodeImage(&file, src, f, q);
}
inline bool SkEncodeImageToFile(const char* path, const SkBitmap& src, SkEncodedFormat f, int q) {
    SkFILEWStream file(path);
    return file.isValid() && SkEncodeImage(&file, src, f, q);
}
inline sk_sp<SkData> SkEncodeImageToData(const SkPixmap& src, SkEncodedFormat f, int q) {
    SkDynamicMemoryWStream buf;
    return SkEncodeImage(&buf, src , f, q) ? buf.detachAsData() : nullptr;
}
inline sk_sp<SkData> SkEncodeImageToData(const SkBitmap& src, SkEncodedFormat f, int q) {
    SkDynamicMemoryWStream buf;
    return SkEncodeImage(&buf, src, f, q) ? buf.detachAsData() : nullptr;
}

/**
 * Uses SkEncodeImage to serialize images that are not already
 * encoded as kPNG_SkEncodedFormat images.
 */
inline sk_sp<SkPixelSerializer> SkMakePixelSerializer() {
    struct EncodeImagePixelSerializer final : SkPixelSerializer {
        bool onUseEncodedData(const void*, size_t) override { return true; }
        SkData* onEncode(const SkPixmap& pmap) override {
            return SkEncodeImageToData(pmap, kPNG_SkEncodedFormat, 100).release();
        }
    };
    return sk_make_sp<EncodeImagePixelSerializer>();
}
//#define SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS
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
    // TODO (scroggo): Merge with SkEncodedFormat.
    enum Type : uint8_t {
        kUnknown_Type = kUnknown_SkEncodedFormat,
        kBMP_Type     = kBMP_SkEncodedFormat,
        kGIF_Type     = kGIF_SkEncodedFormat,
        kICO_Type     = kICO_SkEncodedFormat,
        kJPEG_Type    = kJPEG_SkEncodedFormat,
        kPNG_Type     = kPNG_SkEncodedFormat,
        kWBMP_Type    = kWBMP_SkEncodedFormat,
        kWEBP_Type    = kWEBP_SkEncodedFormat,
        kKTX_Type     = kKTX_SkEncodedFormat,
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
        return SkEncodeImageToData(pixmap, (SkEncodedFormat)t, quality).release();
    }
    static SkData* EncodeData(const SkBitmap& src, Type t, int quality) {
        return SkEncodeImageToData(src, (SkEncodedFormat)t, quality).release();
    }

    static SkData* EncodeData(const SkPixmap&, Type t, int quality);

    static bool EncodeFile(const char file[], const SkBitmap& src, Type t, int quality) {
        return SkEncodeImageToFile(file, src, (SkEncodedFormat)t, quality);
    }
    static bool EncodeStream(SkWStream* dst, const SkBitmap& bm, Type t, int quality) {
        return SkEncodeImage(dst, bm, (SkEncodedFormat)t, quality);
    }

    /** Uses SkImageEncoder to serialize images that are not already
        encoded as kPNG_SkEncodedFormat images. */
    static SkPixelSerializer* CreatePixelSerializer() {
        return SkMakePixelSerializer().release();
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
SkImageEncoder* CreateImageEncoder_CG(SkEncodedFormat type);
#endif

#if defined(SK_BUILD_FOR_WIN)
SkImageEncoder* CreateImageEncoder_WIC(SkEncodedFormat type);
#endif

// Typedef to make registering encoder callback easier
// This has to be defined outside SkImageEncoder. :(
typedef SkTRegistry<SkImageEncoder*(*)(SkEncodedFormat)> SkImageEncoder_EncodeReg;

#endif  // SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS
#endif  // SkImageEncoder_DEFINED
