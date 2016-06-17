/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ktx.h"
#include "SkBitmap.h"
#include "SkStream.h"
#include "SkEndian.h"

#include "gl/GrGLDefines.h"
#include "GrConfig.h"

#include "etc1.h"

static inline uint32_t compressed_fmt_to_gl_define(SkTextureCompressor::Format fmt) {
    static const uint32_t kGLDefineMap[SkTextureCompressor::kFormatCnt] = {
        GR_GL_COMPRESSED_LUMINANCE_LATC1,      // kLATC_Format
        GR_GL_COMPRESSED_R11_EAC,              // kR11_EAC_Format
        GR_GL_COMPRESSED_ETC1_RGB8,            // kETC1_Format
        GR_GL_COMPRESSED_RGBA_ASTC_4x4,        // kASTC_4x4_Format
        GR_GL_COMPRESSED_RGBA_ASTC_5x4,        // kASTC_5x4_Format
        GR_GL_COMPRESSED_RGBA_ASTC_5x5,        // kASTC_5x5_Format
        GR_GL_COMPRESSED_RGBA_ASTC_6x5,        // kASTC_6x5_Format
        GR_GL_COMPRESSED_RGBA_ASTC_6x6,        // kASTC_6x6_Format
        GR_GL_COMPRESSED_RGBA_ASTC_8x5,        // kASTC_8x5_Format
        GR_GL_COMPRESSED_RGBA_ASTC_8x6,        // kASTC_8x6_Format
        GR_GL_COMPRESSED_RGBA_ASTC_8x8,        // kASTC_8x8_Format
        GR_GL_COMPRESSED_RGBA_ASTC_10x5,       // kASTC_10x5_Format
        GR_GL_COMPRESSED_RGBA_ASTC_10x6,       // kASTC_10x6_Format
        GR_GL_COMPRESSED_RGBA_ASTC_10x8,       // kASTC_10x8_Format
        GR_GL_COMPRESSED_RGBA_ASTC_10x10,      // kASTC_10x10_Format
        GR_GL_COMPRESSED_RGBA_ASTC_12x10,      // kASTC_12x10_Format
        GR_GL_COMPRESSED_RGBA_ASTC_12x12,      // kASTC_12x12_Format
    };

    GR_STATIC_ASSERT(0 == SkTextureCompressor::kLATC_Format);
    GR_STATIC_ASSERT(1 == SkTextureCompressor::kR11_EAC_Format);
    GR_STATIC_ASSERT(2 == SkTextureCompressor::kETC1_Format);
    GR_STATIC_ASSERT(3 == SkTextureCompressor::kASTC_4x4_Format);
    GR_STATIC_ASSERT(4 == SkTextureCompressor::kASTC_5x4_Format);
    GR_STATIC_ASSERT(5 == SkTextureCompressor::kASTC_5x5_Format);
    GR_STATIC_ASSERT(6 == SkTextureCompressor::kASTC_6x5_Format);
    GR_STATIC_ASSERT(7 == SkTextureCompressor::kASTC_6x6_Format);
    GR_STATIC_ASSERT(8 == SkTextureCompressor::kASTC_8x5_Format);
    GR_STATIC_ASSERT(9 == SkTextureCompressor::kASTC_8x6_Format);
    GR_STATIC_ASSERT(10 == SkTextureCompressor::kASTC_8x8_Format);
    GR_STATIC_ASSERT(11 == SkTextureCompressor::kASTC_10x5_Format);
    GR_STATIC_ASSERT(12 == SkTextureCompressor::kASTC_10x6_Format);
    GR_STATIC_ASSERT(13 == SkTextureCompressor::kASTC_10x8_Format);
    GR_STATIC_ASSERT(14 == SkTextureCompressor::kASTC_10x10_Format);
    GR_STATIC_ASSERT(15 == SkTextureCompressor::kASTC_12x10_Format);
    GR_STATIC_ASSERT(16 == SkTextureCompressor::kASTC_12x12_Format);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kGLDefineMap) == SkTextureCompressor::kFormatCnt);

    return kGLDefineMap[fmt];
}

#define KTX_FILE_IDENTIFIER_SIZE 12
static const uint8_t KTX_FILE_IDENTIFIER[KTX_FILE_IDENTIFIER_SIZE] = {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

static const uint32_t kKTX_ENDIANNESS_CODE = 0x04030201;

bool SkKTXFile::KeyValue::readKeyAndValue(const uint8_t* data) {
    const char *key = reinterpret_cast<const char *>(data);
    const char *value = key;

    size_t bytesRead = 0;
    while (*value != '\0' && bytesRead < this->fDataSz) {
        ++bytesRead;
        ++value;
    }

    // Error of some sort..
    if (bytesRead >= this->fDataSz) {
        return false;
    }

    // Read the zero terminator
    ++bytesRead;
    ++value;

    size_t bytesLeft = this->fDataSz - bytesRead;

    // We ignore the null terminator when setting the string value.
    this->fKey.set(key, bytesRead - 1);
    if (bytesLeft > 0) {
        this->fValue.set(value, bytesLeft - 1);
    } else {
        return false;
    }

    return true;
}

bool SkKTXFile::KeyValue::writeKeyAndValueForKTX(SkWStream* strm) {
    size_t bytesWritten = 0;
    if (!strm->write(&(this->fDataSz), 4)) {
        return false;
    }

    bytesWritten += 4;

    // Here we know that C-strings must end with a null terminating
    // character, so when we get a c_str(), it will have as many
    // bytes of data as size() returns plus a zero, so we just
    // write size() + 1 bytes into the stream.

    size_t keySize = this->fKey.size() + 1;
    if (!strm->write(this->fKey.c_str(), keySize)) {
        return false;
    }

    bytesWritten += keySize;

    size_t valueSize = this->fValue.size() + 1;
    if (!strm->write(this->fValue.c_str(), valueSize)) {
        return false;
    }

    bytesWritten += valueSize;

    size_t bytesWrittenPadFour = (bytesWritten + 3) & ~3;
    uint8_t nullBuf[4] = { 0, 0, 0, 0 };

    size_t padding = bytesWrittenPadFour - bytesWritten;
    SkASSERT(padding < 4);

    return strm->write(nullBuf, padding);
}

uint32_t SkKTXFile::readInt(const uint8_t** buf, size_t* bytesLeft) const {
    SkASSERT(buf && bytesLeft);

    uint32_t result;

    if (*bytesLeft < 4) {
        SkASSERT(false);
        return 0;
    }

    memcpy(&result, *buf, 4);
    *buf += 4;

    if (fSwapBytes) {
        SkEndianSwap32(result);
    }

    *bytesLeft -= 4;

    return result;
}

SkString SkKTXFile::getValueForKey(const SkString& key) const {
    const KeyValue *begin = this->fKeyValuePairs.begin();
    const KeyValue *end = this->fKeyValuePairs.end();
    for (const KeyValue *kv = begin; kv != end; ++kv) {
        if (kv->key() == key) {
            return kv->value();
        }
    }
    return SkString();
}

bool SkKTXFile::isCompressedFormat(SkTextureCompressor::Format fmt) const {
    if (!this->valid()) {
        return false;
    }

    // This has many aliases
    bool isFmt = false;
    if (fmt == SkTextureCompressor::kLATC_Format) {
        isFmt = GR_GL_COMPRESSED_RED_RGTC1 == fHeader.fGLInternalFormat ||
                GR_GL_COMPRESSED_3DC_X == fHeader.fGLInternalFormat;
    }

    return isFmt || compressed_fmt_to_gl_define(fmt) == fHeader.fGLInternalFormat;
}

bool SkKTXFile::isRGBA8() const {
    return this->valid() && GR_GL_RGBA8 == fHeader.fGLInternalFormat;
}

bool SkKTXFile::isRGB8() const {
    return this->valid() && GR_GL_RGB8 == fHeader.fGLInternalFormat;
}

bool SkKTXFile::readKTXFile(const uint8_t* data, size_t dataLen) {
    const uint8_t *buf = data;
    size_t bytesLeft = dataLen;

    // Make sure original KTX header is there... this should have been checked
    // already by a call to is_ktx()
    SkASSERT(bytesLeft > KTX_FILE_IDENTIFIER_SIZE);
    SkASSERT(0 == memcmp(KTX_FILE_IDENTIFIER, buf, KTX_FILE_IDENTIFIER_SIZE));
    buf += KTX_FILE_IDENTIFIER_SIZE;
    bytesLeft -= KTX_FILE_IDENTIFIER_SIZE;

    // Read header, but first make sure that we have the proper space: we need
    // two 32-bit ints: 1 for endianness, and another for the mandatory image
    // size after the header.
    if (bytesLeft < 8 + sizeof(Header)) {
        return false;
    }

    uint32_t endianness = this->readInt(&buf, &bytesLeft);
    fSwapBytes = kKTX_ENDIANNESS_CODE != endianness;

    // Read header values
    fHeader.fGLType                = this->readInt(&buf, &bytesLeft);
    fHeader.fGLTypeSize            = this->readInt(&buf, &bytesLeft);
    fHeader.fGLFormat              = this->readInt(&buf, &bytesLeft);
    fHeader.fGLInternalFormat      = this->readInt(&buf, &bytesLeft);
    fHeader.fGLBaseInternalFormat  = this->readInt(&buf, &bytesLeft);
    fHeader.fPixelWidth            = this->readInt(&buf, &bytesLeft);
    fHeader.fPixelHeight           = this->readInt(&buf, &bytesLeft);
    fHeader.fPixelDepth            = this->readInt(&buf, &bytesLeft);
    fHeader.fNumberOfArrayElements = this->readInt(&buf, &bytesLeft);
    fHeader.fNumberOfFaces         = this->readInt(&buf, &bytesLeft);
    fHeader.fNumberOfMipmapLevels  = this->readInt(&buf, &bytesLeft);
    fHeader.fBytesOfKeyValueData   = this->readInt(&buf, &bytesLeft);

    // Check for things that we understand...
    {
        // First, we only support compressed formats and single byte
        // representations at the moment. If the internal format is
        // compressed, the the GLType field in the header must be zero.
        // In the future, we may support additional data types (such
        // as GL_UNSIGNED_SHORT_5_6_5)
        if (fHeader.fGLType != 0 && fHeader.fGLType != GR_GL_UNSIGNED_BYTE) {
            return false;
        }

        // This means that for well-formatted KTX files, the glTypeSize
        // field must be one...
        if (fHeader.fGLTypeSize != 1) {
            return false;
        }

        // We don't support 3D textures.
        if (fHeader.fPixelDepth > 1) {
            return false;
        }

        // We don't support texture arrays
        if (fHeader.fNumberOfArrayElements > 1) {
            return false;
        }

        // We don't support cube maps
        if (fHeader.fNumberOfFaces > 1) {
            return false;
        }

        // We don't support width and/or height <= 0
        if (fHeader.fPixelWidth <= 0 || fHeader.fPixelHeight <= 0) {
            return false;
        }
    }

    // Make sure that we have enough bytes left for the key/value
    // data according to what was said in the header.
    if (bytesLeft < fHeader.fBytesOfKeyValueData) {
        return false;
    }

    // Next read the key value pairs
    size_t keyValueBytesRead = 0;
    while (keyValueBytesRead < fHeader.fBytesOfKeyValueData) {
        uint32_t keyValueBytes = this->readInt(&buf, &bytesLeft);
        keyValueBytesRead += 4;

        if (keyValueBytes > bytesLeft) {
            return false;
        }

        KeyValue kv(keyValueBytes);
        if (!kv.readKeyAndValue(buf)) {
            return false;
        }

        fKeyValuePairs.push_back(kv);

        uint32_t keyValueBytesPadded = (keyValueBytes + 3) & ~3;
        buf += keyValueBytesPadded;
        keyValueBytesRead += keyValueBytesPadded;
        bytesLeft -= keyValueBytesPadded;
    }

    // Read the pixel data...
    int mipmaps = SkMax32(fHeader.fNumberOfMipmapLevels, 1);
    SkASSERT(mipmaps == 1);

    int arrayElements = SkMax32(fHeader.fNumberOfArrayElements, 1);
    SkASSERT(arrayElements == 1);

    int faces = SkMax32(fHeader.fNumberOfFaces, 1);
    SkASSERT(faces == 1);

    int depth = SkMax32(fHeader.fPixelDepth, 1);
    SkASSERT(depth == 1);

    for (int mipmap = 0; mipmap < mipmaps; ++mipmap) {
        // Make sure that we have at least 4 more bytes for the first image size
        if (bytesLeft < 4) {
            return false;
        }

        uint32_t imgSize = this->readInt(&buf, &bytesLeft);

        // Truncated file.
        if (bytesLeft < imgSize) {
            return false;
        }

        // !FIXME! If support is ever added for cube maps then the padding
        // needs to be taken into account here.
        for (int arrayElement = 0; arrayElement < arrayElements; ++arrayElement) {
            for (int face = 0; face < faces; ++face) {
                for (int z = 0; z < depth; ++z) {
                    PixelData pd(buf, imgSize);
                    fPixelData.append(1, &pd);
                }
            }
        }

        uint32_t imgSizePadded = (imgSize + 3) & ~3;
        buf += imgSizePadded;
        bytesLeft -= imgSizePadded;
    }

    return bytesLeft == 0;
}

bool SkKTXFile::is_ktx(const uint8_t data[], size_t size) {
    return size >= KTX_FILE_IDENTIFIER_SIZE &&
           0 == memcmp(KTX_FILE_IDENTIFIER, data, KTX_FILE_IDENTIFIER_SIZE);
}

bool SkKTXFile::is_ktx(SkStreamRewindable* stream) {
    // Read the KTX header and make sure it's valid.
    unsigned char buf[KTX_FILE_IDENTIFIER_SIZE];
    bool largeEnough =
        stream->read((void*)buf, KTX_FILE_IDENTIFIER_SIZE) == KTX_FILE_IDENTIFIER_SIZE;
    stream->rewind();
    if (!largeEnough) {
        return false;
    }
    return is_ktx(buf, KTX_FILE_IDENTIFIER_SIZE);
}

SkKTXFile::KeyValue SkKTXFile::CreateKeyValue(const char *cstrKey, const char *cstrValue) {
    SkString key(cstrKey);
    SkString value(cstrValue);

    // Size of buffer is length of string plus the null terminators...
    size_t size = key.size() + 1 + value.size() + 1;

    SkAutoSMalloc<256> buf(size);
    uint8_t* kvBuf = reinterpret_cast<uint8_t*>(buf.get());
    memcpy(kvBuf, key.c_str(), key.size() + 1);
    memcpy(kvBuf + key.size() + 1, value.c_str(), value.size() + 1);

    KeyValue kv(size);
    SkAssertResult(kv.readKeyAndValue(kvBuf));
    return kv;
}

bool SkKTXFile::WriteETC1ToKTX(SkWStream* stream, const uint8_t *etc1Data,
                               uint32_t width, uint32_t height) {
    // First thing's first, write out the magic identifier and endianness...
    if (!stream->write(KTX_FILE_IDENTIFIER, KTX_FILE_IDENTIFIER_SIZE)) {
        return false;
    }

    if (!stream->write(&kKTX_ENDIANNESS_CODE, 4)) {
        return false;
    }

    Header hdr;
    hdr.fGLType = 0;
    hdr.fGLTypeSize = 1;
    hdr.fGLFormat = 0;
    hdr.fGLInternalFormat = GR_GL_COMPRESSED_ETC1_RGB8;
    hdr.fGLBaseInternalFormat = GR_GL_RGB;
    hdr.fPixelWidth = width;
    hdr.fPixelHeight = height;
    hdr.fNumberOfArrayElements = 0;
    hdr.fNumberOfFaces = 1;
    hdr.fNumberOfMipmapLevels = 1;

    // !FIXME! The spec suggests that we put KTXOrientation as a
    // key value pair in the header, but that means that we'd have to
    // pipe through the bitmap's orientation to properly do that.
    hdr.fBytesOfKeyValueData = 0;

    // Write the header
    if (!stream->write(&hdr, sizeof(hdr))) {
        return false;
    }

    // Write the size of the image data
    etc1_uint32 dataSize = etc1_get_encoded_data_size(width, height);
    if (!stream->write(&dataSize, 4)) {
        return false;
    }

    // Write the actual image data
    if (!stream->write(etc1Data, dataSize)) {
        return false;
    }

    return true;
}

bool SkKTXFile::WriteBitmapToKTX(SkWStream* stream, const SkBitmap& bitmap) {
    const SkColorType ct = bitmap.colorType();
    SkAutoLockPixels alp(bitmap);

    const int width = bitmap.width();
    const int height = bitmap.width();
    const uint8_t* src = reinterpret_cast<uint8_t*>(bitmap.getPixels());
    if (NULL == bitmap.getPixels()) {
        return false;
    }

    // First thing's first, write out the magic identifier and endianness...
    if (!stream->write(KTX_FILE_IDENTIFIER, KTX_FILE_IDENTIFIER_SIZE) ||
        !stream->write(&kKTX_ENDIANNESS_CODE, 4)) {
        return false;
    }

    // Collect our key/value pairs...
    SkTArray<KeyValue> kvPairs;

    // Next, write the header based on the bitmap's config.
    Header hdr;
    switch (ct) {
        case kIndex_8_SkColorType:
            // There is a compressed format for this, but we don't support it yet.
            SkDebugf("Writing indexed bitmap to KTX unsupported.\n");
            // VVV fall through VVV
        default:
        case kUnknown_SkColorType:
            // Bitmap hasn't been configured.
            return false;

        case kAlpha_8_SkColorType:
            hdr.fGLType = GR_GL_UNSIGNED_BYTE;
            hdr.fGLTypeSize = 1;
            hdr.fGLFormat = GR_GL_RED;
            hdr.fGLInternalFormat = GR_GL_R8;
            hdr.fGLBaseInternalFormat = GR_GL_RED;
            break;

        case kRGB_565_SkColorType:
            hdr.fGLType = GR_GL_UNSIGNED_SHORT_5_6_5;
            hdr.fGLTypeSize = 2;
            hdr.fGLFormat = GR_GL_RGB;
            hdr.fGLInternalFormat = GR_GL_RGB;
            hdr.fGLBaseInternalFormat = GR_GL_RGB;
            break;

        case kARGB_4444_SkColorType:
            hdr.fGLType = GR_GL_UNSIGNED_SHORT_4_4_4_4;
            hdr.fGLTypeSize = 2;
            hdr.fGLFormat = GR_GL_RGBA;
            hdr.fGLInternalFormat = GR_GL_RGBA4;
            hdr.fGLBaseInternalFormat = GR_GL_RGBA;
            kvPairs.push_back(CreateKeyValue("KTXPremultipliedAlpha", "True"));
            break;

        case kN32_SkColorType:
            hdr.fGLType = GR_GL_UNSIGNED_BYTE;
            hdr.fGLTypeSize = 1;
            hdr.fGLFormat = GR_GL_RGBA;
            hdr.fGLInternalFormat = GR_GL_RGBA8;
            hdr.fGLBaseInternalFormat = GR_GL_RGBA;
            kvPairs.push_back(CreateKeyValue("KTXPremultipliedAlpha", "True"));
            break;
    }

    // Everything else in the header is shared.
    hdr.fPixelWidth = width;
    hdr.fPixelHeight = height;
    hdr.fNumberOfArrayElements = 0;
    hdr.fNumberOfFaces = 1;
    hdr.fNumberOfMipmapLevels = 1;

    // Calculate the key value data size
    hdr.fBytesOfKeyValueData = 0;
    for (KeyValue *kv = kvPairs.begin(); kv != kvPairs.end(); ++kv) {
        // Key value size is the size of the key value data,
        // four bytes for saying how big the key value size is
        // and then additional bytes for padding to four byte boundary
        size_t kvsize = kv->size();
        kvsize += 4;
        kvsize = (kvsize + 3) & ~3;
        hdr.fBytesOfKeyValueData = SkToU32(hdr.fBytesOfKeyValueData + kvsize);
    }

    // Write the header
    if (!stream->write(&hdr, sizeof(hdr))) {
        return false;
    }

    // Write out each key value pair
    for (KeyValue *kv = kvPairs.begin(); kv != kvPairs.end(); ++kv) {
        if (!kv->writeKeyAndValueForKTX(stream)) {
            return false;
        }
    }

    // Calculate the size of the data
    int bpp = bitmap.bytesPerPixel();
    uint32_t dataSz = bpp * width * height;

    if (0 >= bpp) {
        return false;
    }

    // Write it into the buffer
    if (!stream->write(&dataSz, 4)) {
        return false;
    }

    // Write the pixel data...
    const uint8_t* rowPtr = src;
    if (kN32_SkColorType == ct) {
        for (int j = 0; j < height; ++j) {
            const uint32_t* pixelsPtr = reinterpret_cast<const uint32_t*>(rowPtr);
            for (int i = 0; i < width; ++i) {
                uint32_t pixel = pixelsPtr[i];
                uint8_t dstPixel[4];
                dstPixel[0] = pixel >> SK_R32_SHIFT;
                dstPixel[1] = pixel >> SK_G32_SHIFT;
                dstPixel[2] = pixel >> SK_B32_SHIFT;
                dstPixel[3] = pixel >> SK_A32_SHIFT;
                if (!stream->write(dstPixel, 4)) {
                    return false;
                }
            }
            rowPtr += bitmap.rowBytes();
        }
    } else {
        for (int i = 0; i < height; ++i) {
            if (!stream->write(rowPtr, bpp*width)) {
                return false;
            }
            rowPtr += bitmap.rowBytes();
        }
    }

    return true;
}
