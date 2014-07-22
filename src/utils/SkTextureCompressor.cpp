/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor.h"
#include "SkTextureCompressor_R11EAC.h"
#include "SkTextureCompressor_LATC.h"

#include "SkBitmap.h"
#include "SkData.h"
#include "SkEndian.h"

#include "SkTextureCompression_opts.h"

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

int GetCompressedDataSize(Format fmt, int width, int height) {
    switch (fmt) {
        // These formats are 64 bits per 4x4 block.
        case kR11_EAC_Format:
        case kLATC_Format:
        {
            static const int kBlockDimension = 4;
            static const int kEncodedBlockSize = 8;

            if(((width % kBlockDimension) == 0) && ((height % kBlockDimension) == 0)) {

                const int blocksX = width / kBlockDimension;
                const int blocksY = height / kBlockDimension;

                return blocksX * blocksY * kEncodedBlockSize;
            }

            return -1;
        }

        default:
            SkFAIL("Unknown compressed format!");
            return -1;
    }
}

bool CompressBufferToFormat(uint8_t* dst, const uint8_t* src, SkColorType srcColorType,
                            int width, int height, int rowBytes, Format format, bool opt) {
    CompressionProc proc = NULL;
    if (opt) {
        proc = SkTextureCompressorGetPlatformProc(srcColorType, format);
    }

    if (NULL == proc) {
        switch (srcColorType) {
            case kAlpha_8_SkColorType:
            {
                switch (format) {
                    case kLATC_Format:
                        proc = CompressA8ToLATC;
                        break;
                    case kR11_EAC_Format:
                        proc = CompressA8ToR11EAC;
                        break;
                    default:
                        // Do nothing...
                        break;
                }
            }
            break;

            default:
                // Do nothing...
                break;
        }
    }

    if (NULL != proc) {
        return proc(dst, src, width, height, rowBytes);
    }

    return false;
}

SkData *CompressBitmapToFormat(const SkBitmap &bitmap, Format format) {
    SkAutoLockPixels alp(bitmap);

    int compressedDataSize = GetCompressedDataSize(format, bitmap.width(), bitmap.height());
    if (compressedDataSize < 0) {
        return NULL;
    }

    const uint8_t* src = reinterpret_cast<const uint8_t*>(bitmap.getPixels());
    uint8_t* dst = reinterpret_cast<uint8_t*>(sk_malloc_throw(compressedDataSize));

    if (CompressBufferToFormat(dst, src, bitmap.colorType(), bitmap.width(), bitmap.height(),
                               bitmap.rowBytes(), format)) {
        return SkData::NewFromMalloc(dst, compressedDataSize);
    }

    sk_free(dst);
    return NULL;
}

SkBlitter* CreateBlitterForFormat(int width, int height, void* compressedBuffer, Format format) {
    switch(format) {
        case kLATC_Format:
            return CreateLATCBlitter(width, height, compressedBuffer);

        case kR11_EAC_Format:
            return CreateR11EACBlitter(width, height, compressedBuffer);

        default:
            return NULL;
    }

    return NULL;
}

}  // namespace SkTextureCompressor
