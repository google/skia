/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_DEFINED
#define SkTextureCompressor_DEFINED

#include "SkImageInfo.h"

class SkBitmap;
class SkData;

namespace SkTextureCompressor {
    // Various texture compression formats that we support.
    enum Format {
        // Alpha only formats.
        kLATC_Format,
        kR11_EAC_Format,

        kLast_Format = kR11_EAC_Format
    };
    static const int kFormatCnt = kLast_Format + 1;

    // Returns an SkData holding a blob of compressed data that corresponds
    // to the bitmap. If the bitmap colorType cannot be compressed using the 
    // associated format, then we return NULL. The caller is responsible for
    // calling unref() on the returned data.
    SkData* CompressBitmapToFormat(const SkBitmap& bitmap, Format format);

    // Compresses the given src data into dst. The src data is assumed to be
    // large enough to hold width*height pixels. The dst data is expected to
    // be large enough to hold the compressed data according to the format.
    bool CompressBufferToFormat(uint8_t* dst, const uint8_t* src, SkColorType srcColorType,
                                int width, int height, int rowBytes, Format format,
                                bool opt = true /* Use optimization if available */);

    // This typedef defines what the nominal aspects of a compression function
    // are. The typedef is not meant to be used by clients of the API, but rather
    // allows SIMD optimized compression functions to be implemented.
    typedef bool (*CompressionProc)(uint8_t* dst, const uint8_t* src,
                                    int width, int height, int rowBytes);
}

#endif
