/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkImageDecoder.h"
#include "SkPixelRef.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkTypes.h"

#include "ktx.h"
#include "etc1.h"

/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////

// KTX Image decoder
// ---
// KTX is a general texture data storage file format ratified by the Khronos Group. As an
// overview, a KTX file contains all of the appropriate values needed to fully specify a
// texture in an OpenGL application, including the use of compressed data.
//
// This decoder is meant to be used with an SkDiscardablePixelRef so that GPU backends
// can sniff the data before creating a texture. If they encounter a compressed format
// that they understand, they can then upload the data directly to the GPU. Otherwise,
// they will decode the data into a format that Skia supports.

class SkKTXImageDecoder : public SkImageDecoder {
public:
    SkKTXImageDecoder() { }

    Format getFormat() const override {
        return kKTX_Format;
    }

protected:
    Result onDecode(SkStream* stream, SkBitmap* bm, Mode) override;

private:
    typedef SkImageDecoder INHERITED;
};

SkImageDecoder::Result SkKTXImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {
    // TODO: Implement SkStream::copyToData() that's cheap for memory and file streams
    SkAutoDataUnref data(SkCopyStreamToData(stream));
    if (NULL == data) {
        return kFailure;
    }

    SkKTXFile ktxFile(data);
    if (!ktxFile.valid()) {
        return kFailure;
    }

    const unsigned short width = ktxFile.width();
    const unsigned short height = ktxFile.height();

    // Set a flag if our source is premultiplied alpha
    const SkString premulKey("KTXPremultipliedAlpha");
    const bool bSrcIsPremul = ktxFile.getValueForKey(premulKey) == SkString("True");

    // Setup the sampler...
    SkScaledBitmapSampler sampler(width, height, this->getSampleSize());

    // Determine the alpha of the bitmap...
    SkAlphaType alphaType = kOpaque_SkAlphaType;
    if (ktxFile.isRGBA8()) {
        if (this->getRequireUnpremultipliedColors()) {
            alphaType = kUnpremul_SkAlphaType;
            // If the client wants unpremul colors and we only have
            // premul, then we cannot honor their wish.
            if (bSrcIsPremul) {
                return kFailure;
            }
        } else {
            alphaType = kPremul_SkAlphaType;
        }
    }

    // Search through the compressed formats to see if the KTX file is holding
    // compressed data
    bool ktxIsCompressed = false;
    SkTextureCompressor::Format ktxCompressedFormat;
    for (int i = 0; i < SkTextureCompressor::kFormatCnt; ++i) {
        SkTextureCompressor::Format fmt = static_cast<SkTextureCompressor::Format>(i);
        if (ktxFile.isCompressedFormat(fmt)) {
            ktxIsCompressed = true;
            ktxCompressedFormat = fmt;
            break;
        }
    }

    // If the compressed format is a grayscale image, then setup the bitmap properly...
    bool isCompressedAlpha = ktxIsCompressed &&
        ((SkTextureCompressor::kLATC_Format == ktxCompressedFormat) ||
         (SkTextureCompressor::kR11_EAC_Format == ktxCompressedFormat));

    // Set the image dimensions and underlying pixel type.
    if (isCompressedAlpha) {
        const int w = sampler.scaledWidth();
        const int h = sampler.scaledHeight();
        bm->setInfo(SkImageInfo::MakeA8(w, h));
    } else {
        const int w = sampler.scaledWidth();
        const int h = sampler.scaledHeight();
        bm->setInfo(SkImageInfo::MakeN32(w, h, alphaType));
    }
    
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return kSuccess;
    }

    // If we've made it this far, then we know how to grok the data.
    if (!this->allocPixelRef(bm, NULL)) {
        return kFailure;
    }

    // Lock the pixels, since we're about to write to them...
    SkAutoLockPixels alp(*bm);

    if (isCompressedAlpha) {
        if (!sampler.begin(bm, SkScaledBitmapSampler::kGray, *this)) {
            return kFailure;
        }

        // Alpha data is only a single byte per pixel.
        int nPixels = width * height;
        SkAutoMalloc outRGBData(nPixels);
        uint8_t *outRGBDataPtr = reinterpret_cast<uint8_t *>(outRGBData.get());

        // Decode the compressed format
        const uint8_t *buf = reinterpret_cast<const uint8_t *>(ktxFile.pixelData());
        if (!SkTextureCompressor::DecompressBufferFromFormat(
                outRGBDataPtr, width, buf, width, height, ktxCompressedFormat)) {
            return kFailure;
        }

        // Set each of the pixels...
        const int srcRowBytes = width;
        const int dstHeight = sampler.scaledHeight();
        const uint8_t *srcRow = reinterpret_cast<uint8_t *>(outRGBDataPtr);
        srcRow += sampler.srcY0() * srcRowBytes;
        for (int y = 0; y < dstHeight; ++y) {
            sampler.next(srcRow);
            srcRow += sampler.srcDY() * srcRowBytes;
        }

        return kSuccess;

    } else if (ktxFile.isCompressedFormat(SkTextureCompressor::kETC1_Format)) {
        if (!sampler.begin(bm, SkScaledBitmapSampler::kRGB, *this)) {
            return kFailure;
        }

        // ETC1 Data is encoded as RGB pixels, so we should extract it as such
        int nPixels = width * height;
        SkAutoMalloc outRGBData(nPixels * 3);
        uint8_t *outRGBDataPtr = reinterpret_cast<uint8_t *>(outRGBData.get());

        // Decode ETC1
        const uint8_t *buf = reinterpret_cast<const uint8_t *>(ktxFile.pixelData());
        if (!SkTextureCompressor::DecompressBufferFromFormat(
                outRGBDataPtr, width*3, buf, width, height, SkTextureCompressor::kETC1_Format)) {
            return kFailure;
        }

        // Set each of the pixels...
        const int srcRowBytes = width * 3;
        const int dstHeight = sampler.scaledHeight();
        const uint8_t *srcRow = reinterpret_cast<uint8_t *>(outRGBDataPtr);
        srcRow += sampler.srcY0() * srcRowBytes;
        for (int y = 0; y < dstHeight; ++y) {
            sampler.next(srcRow);
            srcRow += sampler.srcDY() * srcRowBytes;
        }

        return kSuccess;

    } else if (ktxFile.isRGB8()) {

        // Uncompressed RGB data (without alpha)
        if (!sampler.begin(bm, SkScaledBitmapSampler::kRGB, *this)) {
            return kFailure;
        }

        // Just need to read RGB pixels
        const int srcRowBytes = width * 3;
        const int dstHeight = sampler.scaledHeight();
        const uint8_t *srcRow = reinterpret_cast<const uint8_t *>(ktxFile.pixelData());
        srcRow += sampler.srcY0() * srcRowBytes;
        for (int y = 0; y < dstHeight; ++y) {
            sampler.next(srcRow);
            srcRow += sampler.srcDY() * srcRowBytes;
        }

        return kSuccess;

    } else if (ktxFile.isRGBA8()) {

        // Uncompressed RGBA data

        // If we know that the image contains premultiplied alpha, then
        // we need to turn off the premultiplier
        SkScaledBitmapSampler::Options opts (*this);
        if (bSrcIsPremul) {
            SkASSERT(bm->alphaType() == kPremul_SkAlphaType);
            SkASSERT(!this->getRequireUnpremultipliedColors());

            opts.fPremultiplyAlpha = false;
        } 

        if (!sampler.begin(bm, SkScaledBitmapSampler::kRGBA, opts)) {
            return kFailure;
        }

        // Just need to read RGBA pixels
        const int srcRowBytes = width * 4;
        const int dstHeight = sampler.scaledHeight();
        const uint8_t *srcRow = reinterpret_cast<const uint8_t *>(ktxFile.pixelData());
        srcRow += sampler.srcY0() * srcRowBytes;
        for (int y = 0; y < dstHeight; ++y) {
            sampler.next(srcRow);
            srcRow += sampler.srcDY() * srcRowBytes;
        }

        return kSuccess;
    }

    return kFailure;
}

///////////////////////////////////////////////////////////////////////////////

// KTX Image Encoder
//
// This encoder takes a best guess at how to encode the bitmap passed to it. If
// there is an installed discardable pixel ref with existing PKM data, then we
// will repurpose the existing ETC1 data into a KTX file. If the data contains
// KTX data, then we simply return a copy of the same data. For all other files,
// the underlying KTX library tries to do its best to encode the appropriate
// data specified by the bitmap based on the config. (i.e. kAlpha8_Config will
// be represented as a full resolution 8-bit image dump with the appropriate
// OpenGL defines in the header).

class SkKTXImageEncoder : public SkImageEncoder {
protected:
    bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) override;

private:
    virtual bool encodePKM(SkWStream* stream, const SkData *data);
    typedef SkImageEncoder INHERITED;
};

bool SkKTXImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bitmap, int) {
    if (!bitmap.pixelRef()) {
        return false;
    }
    SkAutoDataUnref data(bitmap.pixelRef()->refEncodedData());

    // Is this even encoded data?
    if (data) {
        const uint8_t *bytes = data->bytes();
        if (etc1_pkm_is_valid(bytes)) {
            return this->encodePKM(stream, data);
        }

        // Is it a KTX file??
        if (SkKTXFile::is_ktx(bytes)) {
            return stream->write(bytes, data->size());
        }
        
        // If it's neither a KTX nor a PKM, then we need to
        // get at the actual pixels, so fall through and decompress...
    }

    return SkKTXFile::WriteBitmapToKTX(stream, bitmap);
}

bool SkKTXImageEncoder::encodePKM(SkWStream* stream, const SkData *data) {
    const uint8_t* bytes = data->bytes();
    SkASSERT(etc1_pkm_is_valid(bytes));

    etc1_uint32 width = etc1_pkm_get_width(bytes);
    etc1_uint32 height = etc1_pkm_get_height(bytes);

    // ETC1 Data is stored as compressed 4x4 pixel blocks, so we must make sure
    // that our dimensions are valid.
    if (width == 0 || (width & 3) != 0 || height == 0 || (height & 3) != 0) {
        return false;
    }

    // Advance pointer to etc1 data.
    bytes += ETC_PKM_HEADER_SIZE;

    return SkKTXFile::WriteETC1ToKTX(stream, bytes, width, height);
}

/////////////////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(KTXImageDecoder);
DEFINE_ENCODER_CREATOR(KTXImageEncoder);
/////////////////////////////////////////////////////////////////////////////////////////

static SkImageDecoder* sk_libktx_dfactory(SkStreamRewindable* stream) {
    if (SkKTXFile::is_ktx(stream)) {
        return SkNEW(SkKTXImageDecoder);
    }
    return NULL;
}

static SkImageDecoder::Format get_format_ktx(SkStreamRewindable* stream) {
    if (SkKTXFile::is_ktx(stream)) {
        return SkImageDecoder::kKTX_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

SkImageEncoder* sk_libktx_efactory(SkImageEncoder::Type t) {
    return (SkImageEncoder::kKTX_Type == t) ? SkNEW(SkKTXImageEncoder) : NULL;
}

static SkImageDecoder_DecodeReg gReg(sk_libktx_dfactory);
static SkImageDecoder_FormatReg gFormatReg(get_format_ktx);
static SkImageEncoder_EncodeReg gEReg(sk_libktx_efactory);
