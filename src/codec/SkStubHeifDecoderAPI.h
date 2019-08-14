/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStubHeifDecoderAPI_DEFINED
#define SkStubHeifDecoderAPI_DEFINED

// This stub implementation of HeifDecoderAPI.h lets us compile SkHeifCodec.cpp
// even when libheif is not available.  It, of course, does nothing and fails to decode.

#include <memory>
#include <stddef.h>
#include <stdint.h>

enum HeifColorFormat {
    kHeifColorFormat_RGB565,
    kHeifColorFormat_RGBA_8888,
    kHeifColorFormat_BGRA_8888,
};

struct HeifStream {
    virtual ~HeifStream() {}

    virtual size_t read(void*, size_t) = 0;
    virtual bool   rewind()            = 0;
    virtual bool   seek(size_t)        = 0;
    virtual bool   hasLength() const   = 0;
    virtual size_t getLength() const   = 0;
};

struct HeifFrameInfo {
#ifdef SK_LEGACY_HEIF_API
    int mRotationAngle;
    int mWidth;
    int mHeight;
    int mBytesPerPixel;

    size_t                  mIccSize;
    std::unique_ptr<char[]> mIccData;
#else
    uint32_t mWidth;
    uint32_t mHeight;
    int32_t  mRotationAngle;           // Rotation angle, clockwise, should be multiple of 90
    uint32_t mBytesPerPixel;           // Number of bytes for one pixel
    int64_t mDurationUs;               // Duration of the frame in us
    std::vector<uint8_t> mIccData;     // ICC data array
#endif
};

struct HeifDecoder {
    bool init(HeifStream* stream, HeifFrameInfo*) {
        delete stream;
        return false;
    }

#ifndef SK_LEGACY_HEIF_API
    bool getSequenceInfo(HeifFrameInfo* frameInfo, size_t *frameCount) {
        return false;
    }
#endif

    bool decode(HeifFrameInfo*) {
        return false;
    }

#ifndef SK_LEGACY_HEIF_API
    bool decodeSequence(int frameIndex, HeifFrameInfo* frameInfo) {
        return false;
    }
#endif

    bool setOutputColor(HeifColorFormat) {
        return false;
    }

    bool getScanline(uint8_t*) {
        return false;
    }

    int skipScanlines(int) {
        return 0;
    }
};

static inline HeifDecoder* createHeifDecoder() { return new HeifDecoder; }

#endif//SkStubHeifDecoderAPI_DEFINED
