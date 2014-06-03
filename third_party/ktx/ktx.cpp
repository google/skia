
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ktx.h"
#include "SkStream.h"
#include "SkEndian.h"

#include "gl/GrGLDefines.h"

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
    this->fKey.set(key, bytesRead);
    if (bytesLeft > 0) {
        this->fValue.set(value, bytesLeft);
    } else {
        return false;
    }

    return true;
}

uint32_t SkKTXFile::readInt(const uint8_t** buf, size_t* bytesLeft) const {
    SkASSERT(NULL != buf && NULL != bytesLeft);

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

bool SkKTXFile::isETC1() const {
    return this->valid() && GR_GL_COMPRESSED_RGB8_ETC1 == fHeader.fGLInternalFormat;
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

bool SkKTXFile::is_ktx(const uint8_t *data) {
    return 0 == memcmp(KTX_FILE_IDENTIFIER, data, KTX_FILE_IDENTIFIER_SIZE);
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
    return is_ktx(buf);
}
