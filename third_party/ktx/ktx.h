/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkKTXFile_DEFINED
#define SkKTXFile_DEFINED

#include "SkData.h"
#include "SkTextureCompressor.h"
#include "SkTypes.h"
#include "SkTDArray.h"
#include "SkString.h"
#include "SkRefCnt.h"

class SkBitmap;
class SkStreamRewindable;
class SkWStream;

// KTX Image File
// ---
// KTX is a general texture data storage file format ratified by the Khronos Group. As an
// overview, a KTX file contains all of the appropriate values needed to fully specify a
// texture in an OpenGL application, including the use of compressed data.
//
// A full format specification can be found here:
// http://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/

class SkKTXFile {
public:
    // The ownership of the data remains with the caller. This class is intended
    // to be used as a logical wrapper around the data in order to properly
    // access the pixels.
    SkKTXFile(SkData* data) : fData(data), fSwapBytes(false) {
        data->ref();
        fValid = this->readKTXFile(fData->bytes(), fData->size());
    }

    bool valid() const { return fValid; }

    int width() const { return static_cast<int>(fHeader.fPixelWidth); }
    int height() const { return static_cast<int>(fHeader.fPixelHeight); }

    const uint8_t *pixelData(int mipmap = 0) const {
        SkASSERT(!this->valid() || mipmap < fPixelData.count());
        return this->valid() ? fPixelData[mipmap].data() : NULL;
    }

    // If the decoded KTX file has the following key, then it will
    // return the associated value. If not found, the empty string
    // is returned.
    SkString getValueForKey(const SkString& key) const;

    int numMipmaps() const { return static_cast<int>(fHeader.fNumberOfMipmapLevels); }

    bool isCompressedFormat(SkTextureCompressor::Format fmt) const;
    bool isRGBA8() const;
    bool isRGB8() const;

    static bool is_ktx(const uint8_t data[], size_t size);
    static bool is_ktx(SkStreamRewindable* stream);

    static bool WriteETC1ToKTX(SkWStream* stream, const uint8_t *etc1Data,
                               uint32_t width, uint32_t height);
    static bool WriteBitmapToKTX(SkWStream* stream, const SkBitmap& bitmap);
private:

    // The blob holding the file data.
    SkAutoTUnref<SkData> fData;

    // This header captures all of the data that describes the format
    // of the image data in a KTX file.
    struct Header {
        uint32_t fGLType;
        uint32_t fGLTypeSize;
        uint32_t fGLFormat;
        uint32_t fGLInternalFormat;
        uint32_t fGLBaseInternalFormat;
        uint32_t fPixelWidth;
        uint32_t fPixelHeight;
        uint32_t fPixelDepth;
        uint32_t fNumberOfArrayElements;
        uint32_t fNumberOfFaces;
        uint32_t fNumberOfMipmapLevels;
        uint32_t fBytesOfKeyValueData;

        Header() { memset(this, 0, sizeof(*this)); }
    } fHeader;

    // A Key Value pair stored in the KTX file. There may be
    // arbitrarily many of these.
    class KeyValue {
    public:
        KeyValue(size_t size) : fDataSz(size) { }
        bool readKeyAndValue(const uint8_t *data);
        size_t size() const { return fDataSz; }
        const SkString& key() const { return fKey; }
        const SkString& value() const { return fValue; }
        bool writeKeyAndValueForKTX(SkWStream* strm);
    private:
        const size_t fDataSz;
        SkString     fKey;
        SkString     fValue;
    };

    static KeyValue CreateKeyValue(const char *key, const char *value);

    // The pixel data for a single mipmap level in an image. Based on how
    // the rest of the data is stored, this may be compressed, a cubemap, etc.
    // The header will describe the format of this data.
    class PixelData {
    public:
        PixelData(const uint8_t *ptr, size_t sz) : fDataSz(sz), fDataPtr(ptr) { }
        const uint8_t *data() const { return fDataPtr; }
        size_t dataSize() const { return fDataSz; }
    private:
        const size_t fDataSz;
        const uint8_t *fDataPtr;
    };

    // This function is only called once from the constructor. It loads the data
    // and populates the appropriate fields of this class
    // (fKeyValuePairs, fPixelData, fSwapBytes)
    bool readKTXFile(const uint8_t *data, size_t dataLen);

    SkTArray<KeyValue> fKeyValuePairs;
    SkTDArray<PixelData> fPixelData;
    bool fValid;

    // If the endianness of the platform is different than the file,
    // then we need to do proper byte swapping.
    bool fSwapBytes;

    // Read an integer from a buffer, advance the buffer, and swap
    // bytes if fSwapBytes is set
    uint32_t readInt(const uint8_t** buf, size_t* bytesLeft) const;
};

#endif  // SkKTXFile_DEFINED
