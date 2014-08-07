/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_DEFINED
#define SkTextureCompressor_DEFINED

#include "SkBitmapProcShader.h"
#include "SkImageInfo.h"

class SkBitmap;
class SkBlitter;
class SkData;

namespace SkTextureCompressor {
    // Various texture compression formats that we support.
    enum Format {
        // Alpha only formats.
        kLATC_Format,       // 4x4 blocks, (de)compresses A8
        kR11_EAC_Format,    // 4x4 blocks, (de)compresses A8

        // RGB only formats
        kETC1_Format,       // 4x4 blocks, compresses RGB 565, decompresses 8-bit RGB
                            //    NOTE: ETC1 supports 8-bit RGB compression, but we
                            //    currently don't have any RGB8 SkColorTypes. We could
                            //    support 8-bit RGBA but we would have to preprocess the
                            //    bitmap to insert alphas.

        // Multi-purpose formats
        kASTC_4x4_Format,   // 4x4 blocks, no compression, decompresses RGBA
        kASTC_5x4_Format,   // 5x4 blocks, no compression, decompresses RGBA
        kASTC_5x5_Format,   // 5x5 blocks, no compression, decompresses RGBA
        kASTC_6x5_Format,   // 6x5 blocks, no compression, decompresses RGBA
        kASTC_6x6_Format,   // 6x6 blocks, no compression, decompresses RGBA
        kASTC_8x5_Format,   // 8x5 blocks, no compression, decompresses RGBA
        kASTC_8x6_Format,   // 8x6 blocks, no compression, decompresses RGBA
        kASTC_8x8_Format,   // 8x8 blocks, no compression, decompresses RGBA
        kASTC_10x5_Format,  // 10x5 blocks, no compression, decompresses RGBA
        kASTC_10x6_Format,  // 10x6 blocks, no compression, decompresses RGBA
        kASTC_10x8_Format,  // 10x8 blocks, no compression, decompresses RGBA
        kASTC_10x10_Format, // 10x10 blocks, no compression, decompresses RGBA
        kASTC_12x10_Format, // 12x10 blocks, no compression, decompresses RGBA
        kASTC_12x12_Format, // 12x12 blocks, compresses A8, decompresses RGBA

        kLast_Format = kASTC_12x12_Format
    };
    static const int kFormatCnt = kLast_Format + 1;

    // Returns the size of the compressed data given the width, height, and
    // desired compression format. If the width and height are not an appropriate
    // multiple of the block size, then this function returns an error (-1).
    int GetCompressedDataSize(Format fmt, int width, int height);

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

    // Decompresses the given src data from the format specified into the
    // destination buffer. The width and height of the data passed corresponds
    // to the width and height of the uncompressed image. The destination buffer (dst)
    // is assumed to be large enough to hold the entire decompressed image. The
    // decompressed image colors are determined based on the passed format.
    //
    // Note, CompressBufferToFormat compresses A8 data into ASTC. However,
    // general ASTC data encodes RGBA data, so that is what the decompressor
    // operates on.
    //
    // Returns true if successfully decompresses the src data.
    bool DecompressBufferFromFormat(uint8_t* dst, int dstRowBytes, const uint8_t* src,
                                    int width, int height, Format format);

    // This typedef defines what the nominal aspects of a compression function
    // are. The typedef is not meant to be used by clients of the API, but rather
    // allows SIMD optimized compression functions to be implemented.
    typedef bool (*CompressionProc)(uint8_t* dst, const uint8_t* src,
                                    int width, int height, int rowBytes);

    // Returns true if there exists a blitter for the specified format.
    inline bool ExistsBlitterForFormat(Format format) {
        switch (format) {
            case kLATC_Format:
            case kR11_EAC_Format:
            case kASTC_12x12_Format:
                return true;

            default:
                return false;
        }
    }

    // Returns the blitter for the given compression format. Note, the blitter
    // is intended to be used with the proper input. I.e. if you try to blit
    // RGB source data into an R11 EAC texture, you're gonna have a bad time.
    SkBlitter* CreateBlitterForFormat(int width, int height, void* compressedBuffer,
                                      SkTBlitterAllocator *allocator, Format format);

    // Returns the desired dimensions of the block size for the given format. These dimensions
    // don't necessarily correspond to the specification's dimensions, since there may
    // be specialized algorithms that operate on multiple blocks at once. If the
    // flag 'matchSpec' is true, then the actual dimensions from the specification are
    // returned. If the flag is false, then these dimensions reflect the appropriate operable
    // dimensions of the compression functions.
    void GetBlockDimensions(Format format, int* dimX, int* dimY, bool matchSpec = false);
}

#endif
