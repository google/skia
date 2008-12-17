/* libs/graphics/sgl/SkSpriteBlitter_ARGB32.cpp
**
** Copyright 2006, The Android Open Source Project
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

#include "SkSpriteBlitter.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "SkColorPriv.h"

#define D32_S32A_Opaque_Pixel(dst, sc) \
do {                                                                  \
    if (sc) {                                                         \
        unsigned srcA = SkGetPackedA32(sc);                           \
        uint32_t result = sc;                                         \
        if (srcA != 0xFF) {                                           \
            result += SkAlphaMulQ(*dst, SkAlpha255To256(255 - srcA)); \
        }                                                             \
        *dst = result;                                                \
    }                                                                 \
} while (0)

#define SkSPRITE_CLASSNAME                  Sprite_D32_S32A_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE                   uint32_t
#define SkSPRITE_SRC_TYPE                   uint32_t
#define SkSPRITE_DST_GETADDR                getAddr32
#define SkSPRITE_SRC_GETADDR                getAddr32
#define SkSPRITE_PREAMBLE(srcBM, x, y)
#define SkSPRITE_BLIT_PIXEL(dst, src)       D32_S32A_Opaque_Pixel(dst, src)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_S32_Opaque : public SkSpriteBlitter {
public:
    Sprite_D32_S32_Opaque(const SkBitmap& source) : SkSpriteBlitter(source) {}

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SK_RESTRICT uint32_t* dst = fDevice->getAddr32(x, y);
        const SK_RESTRICT uint32_t* src = fSource->getAddr32(x - fLeft,
                                                             y - fTop);
        unsigned dstRB = fDevice->rowBytes();
        unsigned srcRB = fSource->rowBytes();
        size_t size = width * sizeof(uint32_t);

        do {
            memcpy(dst, src, size);
            dst = (SK_RESTRICT uint32_t*)((char*)dst + dstRB);
            src = (const SK_RESTRICT uint32_t*)((const char*)src + srcRB);
        } while (--height != 0);
    }
};

///////////////////////////////////////////////////////////////////////////////

#include "SkColorFilter.h"
#include "SkXfermode.h"

class Sprite_D32_XferFilter : public SkSpriteBlitter {
public:
    Sprite_D32_XferFilter(const SkBitmap& source, const SkPaint& paint)
        : SkSpriteBlitter(source) {
        fColorFilter = paint.getColorFilter();
        fColorFilter->safeRef();
        
        fXfermode = paint.getXfermode();
        fXfermode->safeRef();
        
        fBufferSize = 0;
        fBuffer = NULL;
    }
    
    virtual ~Sprite_D32_XferFilter() {
        delete[] fBuffer;
        fXfermode->safeUnref();
        fColorFilter->safeUnref();
    }
    
    virtual void setup(const SkBitmap& device, int left, int top,
                       const SkPaint& paint) {
        this->INHERITED::setup(device, left, top, paint);
        
        int width = device.width();
        if (width > fBufferSize) {
            fBufferSize = width;
            delete[] fBuffer;
            fBuffer = new SkPMColor[width];
        }
    }

protected:
    SkColorFilter*  fColorFilter;
    SkXfermode*     fXfermode;
    int             fBufferSize;
    SkPMColor*      fBuffer;
    
private:
    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_S32A_XferFilter : public Sprite_D32_XferFilter {
public:
    Sprite_D32_S32A_XferFilter(const SkBitmap& source, const SkPaint& paint)
        : Sprite_D32_XferFilter(source, paint) {}

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SK_RESTRICT uint32_t* dst = fDevice->getAddr32(x, y);
        const SK_RESTRICT uint32_t* src = fSource->getAddr32(x - fLeft,
                                                             y - fTop);
        unsigned dstRB = fDevice->rowBytes();
        unsigned srcRB = fSource->rowBytes();
        SkColorFilter* colorFilter = fColorFilter;
        SkXfermode* xfermode = fXfermode;

        do {
            const SkPMColor* tmp = src;
            
            if (NULL != colorFilter) {
                colorFilter->filterSpan(src, width, fBuffer);
                tmp = fBuffer;
            }
            
            if (NULL != xfermode) {
                xfermode->xfer32(dst, tmp, width, NULL);
            } else {
                for (int i = 0; i < width; i++) {
                    dst[i] = SkPMSrcOver(tmp[i], dst[i]);
                }
            }

            dst = (SK_RESTRICT uint32_t*)((char*)dst + dstRB);
            src = (const SK_RESTRICT uint32_t*)((const char*)src + srcRB);
        } while (--height != 0);
    }
    
private:
    typedef Sprite_D32_XferFilter INHERITED;
};

static void fillbuffer(SK_RESTRICT SkPMColor dst[],
                       const SK_RESTRICT SkPMColor16 src[], int count) {
    SkASSERT(count > 0);
    
    do {
        *dst++ = SkPixel4444ToPixel32(*src++);
    } while (--count != 0);
}

class Sprite_D32_S4444_XferFilter : public Sprite_D32_XferFilter {
public:
    Sprite_D32_S4444_XferFilter(const SkBitmap& source, const SkPaint& paint)
        : Sprite_D32_XferFilter(source, paint) {}
    
    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SK_RESTRICT SkPMColor* dst = fDevice->getAddr32(x, y);
        const SK_RESTRICT SkPMColor16* src = fSource->getAddr16(x - fLeft,
                                                                y - fTop);
        unsigned dstRB = fDevice->rowBytes();
        unsigned srcRB = fSource->rowBytes();
        SK_RESTRICT SkPMColor* buffer = fBuffer;
        SkColorFilter* colorFilter = fColorFilter;
        SkXfermode* xfermode = fXfermode;

        do {
            fillbuffer(buffer, src, width);
            
            if (NULL != colorFilter) {
                colorFilter->filterSpan(buffer, width, buffer);
            }
            if (NULL != xfermode) {
                xfermode->xfer32(dst, buffer, width, NULL);
            } else {
                for (int i = 0; i < width; i++) {
                    dst[i] = SkPMSrcOver(buffer[i], dst[i]);
                }
            }
            
            dst = (SK_RESTRICT SkPMColor*)((char*)dst + dstRB);
            src = (const SK_RESTRICT SkPMColor16*)((const char*)src + srcRB);
        } while (--height != 0);
    }
    
private:
    typedef Sprite_D32_XferFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static void src_row(SK_RESTRICT SkPMColor dst[],
                    const SK_RESTRICT SkPMColor16 src[], int count) {
    do {
        *dst = SkPixel4444ToPixel32(*src);
        src += 1;
        dst += 1;
    } while (--count != 0);
}

class Sprite_D32_S4444_Opaque : public SkSpriteBlitter {
public:
    Sprite_D32_S4444_Opaque(const SkBitmap& source) : SkSpriteBlitter(source) {}
    
    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SK_RESTRICT SkPMColor* dst = fDevice->getAddr32(x, y);
        const SK_RESTRICT SkPMColor16* src = fSource->getAddr16(x - fLeft,
                                                                y - fTop);
        unsigned dstRB = fDevice->rowBytes();
        unsigned srcRB = fSource->rowBytes();
        
        do {
            src_row(dst, src, width);
            dst = (SK_RESTRICT SkPMColor*)((char*)dst + dstRB);
            src = (const SK_RESTRICT SkPMColor16*)((const char*)src + srcRB);
        } while (--height != 0);
    }
};

static void srcover_row(SK_RESTRICT SkPMColor dst[],
                        const SK_RESTRICT SkPMColor16 src[], int count) {
    do {
        *dst = SkPMSrcOver(SkPixel4444ToPixel32(*src), *dst);
        src += 1;
        dst += 1;
    } while (--count != 0);
}

class Sprite_D32_S4444 : public SkSpriteBlitter {
public:
    Sprite_D32_S4444(const SkBitmap& source) : SkSpriteBlitter(source) {}
    
    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SK_RESTRICT SkPMColor* dst = fDevice->getAddr32(x, y);
        const SK_RESTRICT SkPMColor16* src = fSource->getAddr16(x - fLeft,
                                                                y - fTop);
        unsigned dstRB = fDevice->rowBytes();
        unsigned srcRB = fSource->rowBytes();
        
        do {
            srcover_row(dst, src, width);
            dst = (SK_RESTRICT SkPMColor*)((char*)dst + dstRB);
            src = (const SK_RESTRICT SkPMColor16*)((const char*)src + srcRB);
        } while (--height != 0);
    }
};

///////////////////////////////////////////////////////////////////////////////

#include "SkTemplatesPriv.h"

SkSpriteBlitter* SkSpriteBlitter::ChooseD32(const SkBitmap& source,
                                            const SkPaint& paint,
                                            void* storage, size_t storageSize) {
    if (paint.getMaskFilter() != NULL || paint.getAlpha() != 0xFF) {
        return NULL;
    }

    SkXfermode* xfermode = paint.getXfermode();
    SkColorFilter* filter = paint.getColorFilter();
    SkSpriteBlitter* blitter = NULL;

    switch (source.getConfig()) {
        case SkBitmap::kARGB_4444_Config:
            if (xfermode || filter) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S4444_XferFilter,
                                      storage, storageSize, (source, paint));
            } else if (source.isOpaque()) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S4444_Opaque,
                                      storage, storageSize, (source));
            } else {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S4444,
                                      storage, storageSize, (source));
            }
            break;
        case SkBitmap::kARGB_8888_Config:
            if (xfermode || filter) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S32A_XferFilter,
                                      storage, storageSize, (source, paint));
            } else if (source.isOpaque()) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S32_Opaque,
                                      storage, storageSize, (source));
            } else {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S32A_Opaque,
                                      storage, storageSize, (source));
            }
            break;
        default:
            break;
    }
    return blitter;
}

