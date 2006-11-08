/* libs/graphics/images/SkImageDecoder_zbm.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkImageDecoder.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkEndian.h"
#include "SkMath.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkEndian.h"

extern "C" {
    #include "zlib.h"
}
#define DEF_MEM_LEVEL 8                // normally in zutil.h?

static uint16_t* endian_swap16(uint16_t dst[], const uint16_t src[], int count)
{
    for (int i = 0; i < count; i++)
        dst[i] = SkEndianSwap16(src[i]);
    return dst;
}

static void endian_swap16(uint16_t data[], int count)
{
    (void)endian_swap16(data, data, count);
}

class SkZBMImageDecoder : public SkImageDecoder {
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, SkBitmap::Config pref);
};

#define SIGNATURE   "ZlibImag"

#define VERSION 0

enum Format {
    k565_LEndian
};

enum Compression {
    kNone_Compression,
    kZLib_Compression
};

struct Header {
    uint8_t     fSignature[8];

    uint32_t    fCompressedSize;
    uint16_t    fWidth, fHeight;
    uint8_t     fVersion;
    uint8_t     fFormat;
    uint8_t     fCompression;
    uint8_t     fPad;
    // [] compressed data
};
    
SkImageDecoder* SkImageDecoder_ZBM_Factory(SkStream* stream)
{
    uint8_t sig[8];
    
    if (stream->read(sig, sizeof(sig)) == sizeof(sig) &&
        memcmp(sig, SIGNATURE, sizeof(sig)) == 0)
    {
        return SkNEW(SkZBMImageDecoder);
    }
    return NULL;
}

bool SkZBMImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, SkBitmap::Config prefConfig)
{
    Header  head;
    
    if (stream->read(&head, sizeof(head)) != sizeof(head))
    {
        printf("--------------- SkZBMImageDecoder: cannot read header\n");
        return false;
    }

    //  Now convert (as necessary) the endianness of our fields (they are stored in BEndian)
    size_t   compressedSize = SkEndian_SwapBE32(head.fCompressedSize);
    unsigned width          = SkEndian_SwapBE16(head.fWidth);
    unsigned height         = SkEndian_SwapBE16(head.fHeight);

    if (0 == width || 0 == height ||
        VERSION != head.fVersion ||
        k565_LEndian != head.fFormat ||
        kZLib_Compression != head.fCompression)
    {
        printf("--------------- SkZBMImageDecoder: bad header\n");
        return false;
    }

    SkAutoMalloc    storage(compressedSize, SK_MALLOC_TEMP);
    void*           src = storage.get();
    
    if (NULL == src || stream->read(src, compressedSize) != compressedSize)
    {
        printf("--------------- SkZBMImageDecoder: cannot read stream\n");
        return false;
    }

    z_stream    z;

    memset(&z, 0, sizeof(z));
    z.next_in = (Bytef*)src;
    z.avail_in = compressedSize;
    z.data_type = Z_UNKNOWN;
    if (inflateInit2(&z, -MAX_WBITS) != 0)
    {
        printf("--------------- SkZBMImageDecoder: inflateInit2 failed\n");
        return false;
    }

    bool        success = false;
    SkBitmap    tmp;
    tmp.setConfig(SkBitmap::kRGB_565_Config, width, height, width << 1);
    tmp.allocPixels();

    for (unsigned y = 0; y < height; y++)
    {
        z.next_out = (Bytef*)tmp.getAddr16(0, y);
        z.avail_out = width << 1;
        int result = inflate(&z, Z_NO_FLUSH);
        if (Z_STREAM_END == result)
        {
            if (y < height - 1) // we terminated early
                goto DONE;
            break;
        }
        if (Z_OK != result)
            goto DONE;
    }
    success = true;

#ifdef SK_CPU_BENDIAN
    endian_swap16(tmp.getAddr16(0, 0), height * tmp.rowBytes() >> 1);
#endif
    bm->swap(tmp);

DONE:
    inflateEnd(&z);
    return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_IMAGE_ENCODE

class SkZBMImageEncoder : public SkImageEncoder {
protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality);
};

bool SkZBMImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bm, int /*quality*/)
{
    SkBitmap::Config config = bm.getConfig();
    
    if (config != SkBitmap::kRGB_565_Config)
    {
        SkDEBUGF(("SkZLibImageEncoder::onEncode can't encode %d config\n", config));
        return false;
    }
    
    //////////// Compress the bitmap image
    
    int             width = bm.width();
    int             height = bm.height();
    size_t          dstSize = width * height << 1;  // worst case for compression
    SkAutoMalloc    storage(dstSize, SK_MALLOC_TEMP);
    void*           dst = storage.get();
    
    if (NULL == dst)
        return false;

    z_stream    z;
    
    memset(&z, 0, sizeof(z));
    if (deflateInit2(&z, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS,
                     DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
        return false;

// we want to always create LEndian images for now
#ifdef SK_CPU_BENDIAN
    SkAutoTMalloc<uint16_t> scanline(width);
    uint16_t*               scan = scanline.get();
#endif

    z.avail_out = dstSize;
    z.next_out  = (Bytef*)dst;
    for (int y = 0; y < height - 1; y++)
    {
        z.avail_in  = width << 1;
#ifdef SK_CPU_BENDIAN
        z.next_in   = (Bytef*)endian_swap16(scan, bm.getAddr16(0, y), width);
#else
        z.next_in   = (Bytef*)bm.getAddr16(0, y);
#endif
        if (deflate(&z, Z_NO_FLUSH) != Z_OK)
        {
            deflateEnd(&z);
            return false;
        }
    }

    // compress the last scanline
    z.avail_in  = width << 1;
#ifdef SK_CPU_BENDIAN
    z.next_in   = (Bytef*)endian_swap16(scan, bm.getAddr16(0, height - 1), width);
#else
    z.next_in   = (Bytef*)bm.getAddr16(0, height - 1);
#endif
    if (deflate(&z, Z_FINISH) != Z_STREAM_END)
    {
        deflateEnd(&z);
        return false;
    }

    size_t  compressedSize = z.total_out;
    deflateEnd(&z);

    //////////// Now write the data to the stream
    
    Header head;
    
    memcpy(head.fSignature, SIGNATURE, sizeof(head.fSignature));
    head.fCompressedSize = SkEndian_SwapBE32(compressedSize);
    head.fWidth = SkEndian_SwapBE16(bm.width());
    head.fHeight = SkEndian_SwapBE16(bm.height());
    head.fVersion = VERSION;
    head.fFormat = k565_LEndian;
    head.fCompression = kZLib_Compression;
    head.fPad = 0;

    return stream->write(&head, sizeof(head)) &&
           stream->write(dst, compressedSize);
}

SkImageEncoder* SkImageEncoder_ZBM_Factory();
SkImageEncoder* SkImageEncoder_ZBM_Factory()
{
    return SkNEW(SkZBMImageEncoder);
}

#endif /* SK_SUPPORT_IMAGE_ENCODE */


