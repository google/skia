/* libs/graphics/sgl/SkSpriteBlitter_RGB16.cpp
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
#include "SkBlitRow.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "SkColorPriv.h"

#define D16_S32A_Opaque_Pixel(dst, sc)                                        \
do {                                                                          \
    if (sc) {                                                                 \
        *dst = SkSrcOver32To16(sc, *dst);                                     \
    }                                                                         \
} while (0)

static inline void D16_S32A_Blend_Pixel_helper(uint16_t* dst, SkPMColor sc,
                                               unsigned src_scale) {
    uint16_t dc = *dst;
    unsigned sa = SkGetPackedA32(sc);
    unsigned dr, dg, db;

    if (255 == sa) {
        dr = SkAlphaBlend(SkPacked32ToR16(sc), SkGetPackedR16(dc), src_scale);
        dg = SkAlphaBlend(SkPacked32ToG16(sc), SkGetPackedG16(dc), src_scale);
        db = SkAlphaBlend(SkPacked32ToB16(sc), SkGetPackedB16(dc), src_scale);
    } else {
        unsigned dst_scale = 255 - SkAlphaMul(sa, src_scale);
        dr = (SkPacked32ToR16(sc) * src_scale +
              SkGetPackedR16(dc) * dst_scale) >> 8;
        dg = (SkPacked32ToG16(sc) * src_scale +
              SkGetPackedG16(dc) * dst_scale) >> 8;
        db = (SkPacked32ToB16(sc) * src_scale +
              SkGetPackedB16(dc) * dst_scale) >> 8;
    }
    *dst = SkPackRGB16(dr, dg, db);
}

#define D16_S32A_Blend_Pixel(dst, sc, src_scale) \
    do { if (sc) D16_S32A_Blend_Pixel_helper(dst, sc, src_scale); } while (0)


///////////////////////////////////////////////////////////////////////////////

class Sprite_D16_S16_Opaque : public SkSpriteBlitter {
public:
    Sprite_D16_S16_Opaque(const SkBitmap& source)
        : SkSpriteBlitter(source) {}

    // overrides
    virtual void blitRect(int x, int y, int width, int height) {
        SK_RESTRICT uint16_t* dst = fDevice->getAddr16(x, y);
        const SK_RESTRICT uint16_t* src = fSource->getAddr16(x - fLeft,
                                                             y - fTop);
        unsigned dstRB = fDevice->rowBytes();
        unsigned srcRB = fSource->rowBytes();

        while (--height >= 0) {
            memcpy(dst, src, width << 1);
            dst = (uint16_t*)((char*)dst + dstRB);
            src = (const uint16_t*)((const char*)src + srcRB);
        }
    }
};

#define D16_S16_Blend_Pixel(dst, sc, scale)     \
    do {                                        \
        uint16_t dc = *dst;                     \
        *dst = SkBlendRGB16(sc, dc, scale);     \
    } while (0)

#define SkSPRITE_CLASSNAME                  Sprite_D16_S16_Blend
#define SkSPRITE_ARGS                       , uint8_t alpha
#define SkSPRITE_FIELDS                     uint8_t  fSrcAlpha;
#define SkSPRITE_INIT                       fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE                   uint16_t
#define SkSPRITE_SRC_TYPE                   uint16_t
#define SkSPRITE_DST_GETADDR                getAddr16
#define SkSPRITE_SRC_GETADDR                getAddr16
#define SkSPRITE_PREAMBLE(srcBM, x, y)      int scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)       D16_S16_Blend_Pixel(dst, src, scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

///////////////////////////////////////////////////////////////////////////////

#define D16_S4444_Opaque(dst, sc)           \
    do {                                    \
        uint16_t dc = *dst;                 \
        *dst = SkSrcOver4444To16(sc, dc);   \
    } while (0)

#define SkSPRITE_CLASSNAME                  Sprite_D16_S4444_Opaque
#define SkSPRITE_ARGS                       
#define SkSPRITE_FIELDS                     
#define SkSPRITE_INIT                       
#define SkSPRITE_DST_TYPE                   uint16_t
#define SkSPRITE_SRC_TYPE                   SkPMColor16
#define SkSPRITE_DST_GETADDR                getAddr16
#define SkSPRITE_SRC_GETADDR                getAddr16
#define SkSPRITE_PREAMBLE(srcBM, x, y)      
#define SkSPRITE_BLIT_PIXEL(dst, src)       D16_S4444_Opaque(dst, src)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

#define D16_S4444_Blend(dst, sc, scale16)           \
    do {                                            \
        uint16_t dc = *dst;                         \
        *dst = SkBlend4444To16(sc, dc, scale16);    \
    } while (0)


#define SkSPRITE_CLASSNAME                  Sprite_D16_S4444_Blend
#define SkSPRITE_ARGS                       , uint8_t alpha
#define SkSPRITE_FIELDS                     uint8_t  fSrcAlpha;
#define SkSPRITE_INIT                       fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE                   uint16_t
#define SkSPRITE_SRC_TYPE                   uint16_t
#define SkSPRITE_DST_GETADDR                getAddr16
#define SkSPRITE_SRC_GETADDR                getAddr16
#define SkSPRITE_PREAMBLE(srcBM, x, y)      int scale = SkAlpha15To16(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)       D16_S4444_Blend(dst, src, scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)
#include "SkSpriteBlitterTemplate.h"

///////////////////////////////////////////////////////////////////////////////

#define SkSPRITE_CLASSNAME                  Sprite_D16_SIndex8A_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE                   uint16_t
#define SkSPRITE_SRC_TYPE                   uint8_t
#define SkSPRITE_DST_GETADDR                getAddr16
#define SkSPRITE_SRC_GETADDR                getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)      const SkPMColor* ctable = srcBM.getColorTable()->lockColors()
#define SkSPRITE_BLIT_PIXEL(dst, src)       D16_S32A_Opaque_Pixel(dst, ctable[src])
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)           srcBM.getColorTable()->unlockColors(false)
#include "SkSpriteBlitterTemplate.h"

#define SkSPRITE_CLASSNAME                  Sprite_D16_SIndex8A_Blend
#define SkSPRITE_ARGS                       , uint8_t alpha
#define SkSPRITE_FIELDS                     uint8_t fSrcAlpha;
#define SkSPRITE_INIT                       fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE                   uint16_t
#define SkSPRITE_SRC_TYPE                   uint8_t
#define SkSPRITE_DST_GETADDR                getAddr16
#define SkSPRITE_SRC_GETADDR                getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)      const SkPMColor* ctable = srcBM.getColorTable()->lockColors(); unsigned src_scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)       D16_S32A_Blend_Pixel(dst, ctable[src], src_scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)           srcBM.getColorTable()->unlockColors(false);
#include "SkSpriteBlitterTemplate.h"

///////////////////////////////////////////////////////////////////////////////

#define SkSPRITE_CLASSNAME                  Sprite_D16_SIndex8_Opaque
#define SkSPRITE_ARGS
#define SkSPRITE_FIELDS
#define SkSPRITE_INIT
#define SkSPRITE_DST_TYPE                   uint16_t
#define SkSPRITE_SRC_TYPE                   uint8_t
#define SkSPRITE_DST_GETADDR                getAddr16
#define SkSPRITE_SRC_GETADDR                getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)      const uint16_t* ctable = srcBM.getColorTable()->lock16BitCache()
#define SkSPRITE_BLIT_PIXEL(dst, src)       *dst = ctable[src]
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)           srcBM.getColorTable()->unlock16BitCache()
#include "SkSpriteBlitterTemplate.h"

#define SkSPRITE_CLASSNAME                  Sprite_D16_SIndex8_Blend
#define SkSPRITE_ARGS                       , uint8_t alpha
#define SkSPRITE_FIELDS                     uint8_t fSrcAlpha;
#define SkSPRITE_INIT                       fSrcAlpha = alpha;
#define SkSPRITE_DST_TYPE                   uint16_t
#define SkSPRITE_SRC_TYPE                   uint8_t
#define SkSPRITE_DST_GETADDR                getAddr16
#define SkSPRITE_SRC_GETADDR                getAddr8
#define SkSPRITE_PREAMBLE(srcBM, x, y)      const uint16_t* ctable = srcBM.getColorTable()->lock16BitCache(); unsigned src_scale = SkAlpha255To256(fSrcAlpha);
#define SkSPRITE_BLIT_PIXEL(dst, src)       D16_S16_Blend_Pixel(dst, ctable[src], src_scale)
#define SkSPRITE_NEXT_ROW
#define SkSPRITE_POSTAMBLE(srcBM)           srcBM.getColorTable()->unlock16BitCache();
#include "SkSpriteBlitterTemplate.h"

///////////////////////////////////////////////////////////////////////////////

class Sprite_D16_S32_BlitRowProc : public SkSpriteBlitter {
public:
    Sprite_D16_S32_BlitRowProc(const SkBitmap& source)
        : SkSpriteBlitter(source) {}
    
    // overrides
    
    virtual void setup(const SkBitmap& device, int left, int top,
                       const SkPaint& paint) {
        this->INHERITED::setup(device, left, top, paint);
        
        unsigned flags = 0;
        
        if (paint.getAlpha() < 0xFF) {
            flags |= SkBlitRow::kGlobalAlpha_Flag;
        }
        if (!fSource->isOpaque()) {
            flags |= SkBlitRow::kSrcPixelAlpha_Flag;
        }
        if (paint.isDither()) {
            flags |= SkBlitRow::kDither_Flag;
        }
        fProc = SkBlitRow::Factory(flags, SkBitmap::kRGB_565_Config);
    }
    
    virtual void blitRect(int x, int y, int width, int height) {
        SK_RESTRICT uint16_t* dst = fDevice->getAddr16(x, y);
        const SK_RESTRICT SkPMColor* src = fSource->getAddr32(x - fLeft,
                                                              y - fTop);
        unsigned dstRB = fDevice->rowBytes();
        unsigned srcRB = fSource->rowBytes();
        SkBlitRow::Proc proc = fProc;
        U8CPU alpha = fPaint->getAlpha();
        
        while (--height >= 0) {
            proc(dst, src, width, alpha, x, y);
            y += 1;
            dst = (SK_RESTRICT uint16_t*)((char*)dst + dstRB);
            src = (const SK_RESTRICT SkPMColor*)((const char*)src + srcRB);
        }
    }
    
private:
    SkBlitRow::Proc fProc;
    
    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

#include "SkTemplatesPriv.h"

SkSpriteBlitter* SkSpriteBlitter::ChooseD16(const SkBitmap& source,
                                            const SkPaint& paint,
                                            void* storage, size_t storageSize) {
    if (paint.getMaskFilter() != NULL) { // may add cases for this
        return NULL;
    }
    if (paint.getXfermode() != NULL) { // may add cases for this
        return NULL;
    }
    if (paint.getColorFilter() != NULL) { // may add cases for this
        return NULL;
    }

    SkSpriteBlitter* blitter = NULL;
    unsigned alpha = paint.getAlpha();

    switch (source.getConfig()) {
        case SkBitmap::kARGB_8888_Config:
            SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S32_BlitRowProc,
                                  storage, storageSize, (source));
            break;
        case SkBitmap::kARGB_4444_Config:
            if (255 == alpha) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S4444_Opaque,
                                      storage, storageSize, (source));
            } else {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S4444_Blend,
                                    storage, storageSize, (source, alpha >> 4));
            }
            break;
        case SkBitmap::kRGB_565_Config:
            if (255 == alpha) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S16_Opaque,
                                      storage, storageSize, (source));
            } else {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_S16_Blend,
                                      storage, storageSize, (source, alpha));
            }
            break;
        case SkBitmap::kIndex8_Config:
            if (source.isOpaque()) {
                if (255 == alpha) {
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8_Opaque,
                                          storage, storageSize, (source));
                } else {
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8_Blend,
                                         storage, storageSize, (source, alpha));
                }
            } else {
                if (255 == alpha) {
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8A_Opaque,
                                          storage, storageSize, (source));
                } else {
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D16_SIndex8A_Blend,
                                         storage, storageSize, (source, alpha));
                }
            }
            break;
        default:
            break;
    }
    return blitter;
}

