/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_DEFINED
#define SkTextureCompressor_DEFINED

class SkBitmap;
class SkData;

namespace SkTextureCompressor {
    // Various texture compression formats that we support.
    enum Format {
        // Alpha only format.
        kLATC_Format,

        kLast_Format = kLATC_Format
    };
    static const int kFormatCnt = kLast_Format + 1;

    // Returns an SkData holding a blob of compressed data that corresponds
    // to the bitmap. If the bitmap colorType cannot be compressed using the 
    // associated format, then we return NULL. The caller is responsible for
    // calling unref() on the returned data.
    SkData* CompressBitmapToFormat(const SkBitmap& bitmap, Format format);
}

#endif
