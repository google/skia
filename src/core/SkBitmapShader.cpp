/* Copyright 2006, The Android Open Source Project
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

#if 0

#include "SkBitmapShader.h"
#include "SkBitmapSampler.h"

#ifdef SK_SUPPORT_MIPMAP
static SkFixed find_mip_level(SkFixed dx, SkFixed dy)
{
    dx = SkAbs32(dx);
    dy = SkAbs32(dy);
    if (dx < dy)
        dx = dy;
    
    if (dx < SK_Fixed1)
        return 0;
    
    int clz = SkCLZ(dx);
    SkASSERT(clz >= 1 && clz <= 15);
    return SkIntToFixed(15 - clz) + ((unsigned)(dx << (clz + 1)) >> 16);
}
#endif

SkBitmapShader::SkBitmapShader(const SkBitmap& src, bool doFilter,
                               TileMode tmx, TileMode tmy)
    :
#ifdef SK_SUPPORT_MIPMAP
    fMipLevel(0), fMipSrcBitmap(src),
#endif
    fOrigSrcBitmap(src)
    
{
    fFilterBitmap = doFilter;
    fTileModeX = SkToU8(tmx);
    fTileModeY = SkToU8(tmy);
}

SkBitmapShader::SkBitmapShader(SkFlattenableReadBuffer& buffer) :
    INHERITED(buffer)
{
    Bitmap src;
    buffer.readBitmap(&src); 
#ifdef SK_SUPPORT_MIPMAP
    fMipLevel = 0;
    fMipSrcBitmap = src;
#endif
    fOrigSrcBitmap = src;
    fFilterBitmap = buffer.readU8();
    fTileModeX = buffer.readU8();
    fTileModeY = buffer.readU8();
}

void SkBitmapShader::flatten(SkFlattenableWriteBuffer& buffer)
{
    this->INHERITED::flatten(buffer);
    buffer.writeBitmap(&fOrigSrcBitmap);
    buffer.write8(fFilterBitmap);
    buffer.write8(fTileModeX);
    buffer.write8(fTileModeY);
}

bool SkBitmapShader::setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix)
{
    // do this first, so we have a correct inverse matrix
    if (!this->INHERITED::setContext(device, paint, matrix))
        return false;

    if (fOrigSrcBitmap.getConfig() == SkBitmap::kNo_Config ||
        fOrigSrcBitmap.width() == 0 ||
        fOrigSrcBitmap.height() == 0)
        return false;

    SkBitmap& bm = fOrigSrcBitmap;

#ifdef SK_SUPPORT_MIPMAP
    if (fOrigSrcBitmap.countMipLevels())
    {
        const SkMatrix& inv = this->getTotalInverse();

        fMipLevel = SkMin32(find_mip_level( SkScalarToFixed(inv.getScaleX()),
                                            SkScalarToFixed(inv.getSkewY())),
                            SkIntToFixed(fOrigSrcBitmap.countMipLevels() - 1));

//        SkDEBUGF(("BitmapShader miplevel=%x\n", fMipLevel));

        const SkBitmap::MipLevel* mm = fOrigSrcBitmap.getMipLevel(fMipLevel >> 16);
        
        fMipSrcBitmap.setConfig(fOrigSrcBitmap.getConfig(),
                                mm->fWidth,
                                mm->fHeight,
                                mm->fRowBytes);
        fMipSrcBitmap.setPixels(mm->fPixels);
        bm = fMipSrcBitmap;
    }
    else
    {
        fMipLevel = 0;
        fMipSrcBitmap = fOrigSrcBitmap;
    }
#endif

    fFlags = 0;
    if (paint.getAlpha() == 255 && bm.isOpaque())
        fFlags |= kOpaqueAlpha_Flag;

    return true;
}

///////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"
#include "SkBitmapSampler.h"
#include "SkPerspIter.h"

class Sampler_BitmapShader : public SkBitmapShader {
public:
    Sampler_BitmapShader(const SkBitmap& src, bool doFilter,
                         TileMode tmx, TileMode tmy)
        : SkBitmapShader(src, doFilter, tmx, tmy)
    {
        // make sure to pass our copy of the src bitmap to the sampler, and not the
        // original parameter (which might go away).
        fSampler = NULL;
    }

    virtual ~Sampler_BitmapShader()
    {
        SkDELETE(fSampler);
    }
    
    virtual bool setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix)
    {
        if (this->INHERITED::setContext(device, paint, matrix))
        {
            SkDELETE(fSampler);
            fSampler = SkBitmapSampler::Create(this->getSrcBitmap(), this->getFilterBitmap(),
                                               this->getTileModeX(), this->getTileModeY());
            fSampler->setPaint(paint);
            return true;
        }
        return false;
    }
    
    enum {
        kMaxPointStorageCount = 32
    };

    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count)
    {
        unsigned            scale = SkAlpha255To256(this->getPaintAlpha());
        const SkMatrix&     inv = this->getTotalInverse();
        SkMatrix::MapPtProc proc = this->getInverseMapPtProc();
        SkBitmapSampler*     sampler = fSampler;
        MatrixClass         mc = this->getInverseClass();

        SkPoint srcPt;

        if (mc != kPerspective_MatrixClass)
        {
            proc(inv, SkIntToScalar(x) + SK_ScalarHalf,
                      SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

            SkFixed fx = SkScalarToFixed(srcPt.fX);
            SkFixed fy = SkScalarToFixed(srcPt.fY);
            SkFixed dx, dy;

            if (mc == kLinear_MatrixClass)
            {
                dx = SkScalarToFixed(inv.getScaleX());
                dy = SkScalarToFixed(inv.getSkewY());
            }
            else
                (void)inv.fixedStepInX(SkIntToScalar(y), &dx, &dy);

#if defined(SK_SUPPORT_MIPMAP)
            {   int level = this->getMipLevel() >> 16;
                fx >>= level;
                fy >>= level;
                dx >>= level;
                dy >>= level;
            }
#endif
            if (scale == 256)
            {
                for (int i = 0; i < count; i++)
                {
                    dstC[i] = sampler->sample(fx, fy);
                    fx += dx;
                    fy += dy;
                }
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    uint32_t    c = sampler->sample(fx, fy);
                    dstC[i] = SkAlphaMulQ(c, scale);
                    fx += dx;
                    fy += dy;
                }
            }
        }
        else
        {
            SkPerspIter   iter(inv, SkIntToScalar(x) + SK_ScalarHalf,
                                    SkIntToScalar(y) + SK_ScalarHalf, count);
            if (scale == 256)
            {
                while ((count = iter.next()) != 0)
                {
                    const SkFixed* src = iter.getXY();
                    for (int i = 0; i < count; i++)
                    {
                        *dstC++ = sampler->sample(src[0], src[1]);
                        src += 2;
                    }
                }
            }
            else
            {
                while ((count = iter.next()) != 0)
                {
                    const SkFixed* src = iter.getXY();
                    for (int i = 0; i < count; i++)
                    {
                        uint32_t c = sampler->sample(src[0] - SK_FixedHalf, src[1] - SK_FixedHalf);
                        *dstC++ = SkAlphaMulQ(c, scale);
                        src += 2;
                    }
                }
            }
        }
    }

protected:

    const SkMatrix& getUnitInverse() const { return fUnitInverse; }
    SkMatrix::MapPtProc getUnitInverseProc() const { return fUnitInverseProc; }

    /* takes computed inverse (from setContext) and computes fUnitInverse,
        taking srcBitmap width/height into account, so that fUnitInverse
        walks 0...1, allowing the tile modes to all operate in a fast 16bit
        space (no need for mod). The resulting coords need to be scaled by
        width/height to get back into src space (coord * width >> 16).
    */
    void computeUnitInverse()
    {
        const SkBitmap& src = getSrcBitmap();
        fUnitInverse = this->getTotalInverse();
        fUnitInverse.postIDiv(src.width(), src.height());
        fUnitInverseProc = fUnitInverse.getMapPtProc();
    }

private:
    SkBitmapSampler*    fSampler;
    SkMatrix            fUnitInverse;
    SkMatrix::MapPtProc fUnitInverseProc;
    
    typedef SkBitmapShader INHERITED;
};

///////////////////////////////////////////////////////////////////////////

class HasSpan16_Sampler_BitmapShader : public Sampler_BitmapShader {
public:
    HasSpan16_Sampler_BitmapShader(const SkBitmap& src, bool doFilter,
                                   TileMode tmx, TileMode tmy)
        : Sampler_BitmapShader(src, doFilter, tmx, tmy)
    {
    }

    virtual uint32_t getFlags()
    {
        uint32_t flags = this->INHERITED::getFlags();
        
        switch (this->getSrcBitmap().getConfig()) {
        case SkBitmap::kRGB_565_Config:
            flags |= kHasSpan16_Flag;
            break;
        case SkBitmap::kIndex8_Config:
        case SkBitmap::kARGB_8888_Config:
            if (this->getSrcBitmap().isOpaque())
                flags |= kHasSpan16_Flag;
            break;
        default:
            break;
        }
        return flags;
    }
    
    const SkBitmap& revealSrcBitmap() const { return this->getSrcBitmap(); }
    uint8_t         revealPaintAlpha() const { return this->getPaintAlpha(); }
    const SkMatrix& revealTotalInverse() const { return this->getTotalInverse(); }

private:
    typedef Sampler_BitmapShader INHERITED;
};

///////////////////////////////////////////////////////////////////////////

static void Index8_RepeatTile_Sprite16(HasSpan16_Sampler_BitmapShader* shader,
                                       int x, int y, uint16_t dstC[], int count)
{
    const SkMatrix& inv = shader->revealTotalInverse();
    const SkBitmap& srcBitmap = shader->revealSrcBitmap();
    int             width = srcBitmap.width();
    int             height = srcBitmap.height();

    SkColorTable* ctable = srcBitmap.getColorTable();
    const uint16_t* colors = ctable->lock16BitCache();

    x += SkScalarRound(inv[SkMatrix::kMTransX]);
    y += SkScalarRound(inv[SkMatrix::kMTransY]);
    
    x = do_repeat_mod(x, width - 1);
    y = do_repeat_mod(y, height - 1);
    const uint8_t* row = srcBitmap.getAddr8(0, y);
    const uint8_t* src = row + x;

    // do the first partial run
    int n = width - x;
    if (n > count) n = count;
    count -= n;
    SkASSERT(n > 0);
    do {
        *dstC++ = colors[*src++];
    } while (--n > 0);

    // do 1 complete run
    if (count >= width)
    {
        uint16_t* baseDstC = dstC;  // remember the first complete run start
        n = width;
        count -= width;
        src = row;
        do {
            *dstC++ = colors[*src++];
        } while (--n > 0);

        // do the rest of the complete runs
        while (count >= width)
        {
            count -= width;
            memcpy(dstC, baseDstC, width << 1);
            dstC += width;
        }
        // do final partial run
        if (count > 0)
            memcpy(dstC, baseDstC, count << 1);
    }
    else // do final partial
    {
        if (count > 0)
        {
            src = row;
            do {
                *dstC++ = colors[*src++];
            } while (--count > 0);
        }
    }
    
    ctable->unlock16BitCache();
}

static void Index8_RepeatTile_Sprite32(HasSpan16_Sampler_BitmapShader* shader,
                                       int x, int y, SkPMColor dstC[], int count)
{
    const SkMatrix& inv = shader->revealTotalInverse();
    const SkBitmap& srcBitmap = shader->revealSrcBitmap();
    int             width = srcBitmap.width();
    int             height = srcBitmap.height();

    SkColorTable* ctable = srcBitmap.getColorTable();
    const SkPMColor* colors = ctable->lockColors();

    x += SkScalarRound(inv[SkMatrix::kMTransX]);
    y += SkScalarRound(inv[SkMatrix::kMTransY]);
    
    x = do_repeat_mod(x, width - 1);
    y = do_repeat_mod(y, height - 1);

    const uint8_t* row = srcBitmap.getAddr8(0, y);
    const uint8_t* src = row + x;

    // do the first partial run
    int n = width - x;
    if (n > count) n = count;
    count -= n;
    SkASSERT(n > 0);
    do {
        *dstC++ = colors[*src++];
    } while (--n > 0);

    // do 1 complete run
    if (count >= width)
    {
        SkPMColor* baseDstC = dstC;  // remember the first complete run start
        n = width;
        count -= width;
        src = row;
        do {
            *dstC++ = colors[*src++];
        } while (--n > 0);

        // do the rest of the complete runs
        while (count >= width)
        {
            count -= width;
            memcpy(dstC, baseDstC, width << 2);
            dstC += width;
        }
        // do final partial run
        if (count > 0)
            memcpy(dstC, baseDstC, count << 2);
    }
    else // do final partial
    {
        if (count > 0)
        {
            src = row;
            do {
                *dstC++ = colors[*src++];
            } while (--count > 0);
        }
    }
    
    ctable->unlockColors(false);
}

static void RGB16_RepeatTile_Sprite16(HasSpan16_Sampler_BitmapShader* shader,
                                       int x, int y, uint16_t dstC[], int count)
{
    SkASSERT(count > 0);

    const SkMatrix& inv = shader->revealTotalInverse();
    const SkBitmap& srcBitmap = shader->revealSrcBitmap();
    int             width = srcBitmap.width();
    int             height = srcBitmap.height();
    
    SkASSERT(width > 0 && height > 0);

    x += SkScalarRound(inv[SkMatrix::kMTransX]);
    y += SkScalarRound(inv[SkMatrix::kMTransY]);
    
    x = do_repeat_mod(x, width - 1);
    y = do_repeat_mod(y, height - 1);

    const uint16_t* row = srcBitmap.getAddr16(0, y);
    const uint16_t* src = row + x;

    int n = SkMin32(width - x, count);

    for (;;)
    {
        SkASSERT(n > 0 && count >= n);
        memcpy(dstC, src, n << 1);
        count -= n;
        if (count == 0)
            break;
        dstC += n;
        src = row;
        n = SkMin32(width, count);
    }
}

static void RGB16_RepeatTile_Sprite32(HasSpan16_Sampler_BitmapShader* shader,
                                       int x, int y, SkPMColor dstC[], int count)
{
    SkASSERT(count > 0);

    const SkMatrix& inv = shader->revealTotalInverse();
    const SkBitmap& srcBitmap = shader->revealSrcBitmap();
    int             width = srcBitmap.width();
    int             height = srcBitmap.height();
    
    SkASSERT(width > 0 && height > 0);

    x += SkScalarRound(inv[SkMatrix::kMTransX]);
    y += SkScalarRound(inv[SkMatrix::kMTransY]);
    
    x = do_repeat_mod(x, width - 1);
    y = do_repeat_mod(y, height - 1);

    const uint16_t* row = srcBitmap.getAddr16(0, y);
    const uint16_t* src = row + x;

    int n = SkMin32(width - x, count);

    // do the first partial run
    count -= n;
    SkASSERT(n > 0);
    do {
        *dstC++ = SkPixel16ToPixel32(*src++);
    } while (--n > 0);

    // do 1 complete run
    if (count >= width)
    {
        SkPMColor* baseDstC = dstC;  // remember the first complete run start
        n = width;
        count -= width;
        src = row;
        do {
            *dstC++ = SkPixel16ToPixel32(*src++);
        } while (--n > 0);

        // do the rest of the complete runs
        while (count >= width)
        {
            count -= width;
            memcpy(dstC, baseDstC, width << 2);
            dstC += width;
        }
        // do final partial run
        if (count > 0)
            memcpy(dstC, baseDstC, count << 2);
    }
    else // do final partial
    {
        if (count > 0)
        {
            src = row;
            do {
                *dstC++ = SkPixel16ToPixel32(*src++);;
            } while (--count > 0);
        }
    }
}

static void ARGB32_RepeatTile_Sprite16(HasSpan16_Sampler_BitmapShader* shader,
                                       int x, int y, uint16_t dstC[], int count)
{
    SkASSERT(count > 0);

    const SkMatrix& inv = shader->revealTotalInverse();
    const SkBitmap& srcBitmap = shader->revealSrcBitmap();
    int             width = srcBitmap.width();
    int             height = srcBitmap.height();
    
    SkASSERT(width > 0 && height > 0);

    x += SkScalarRound(inv[SkMatrix::kMTransX]);
    y += SkScalarRound(inv[SkMatrix::kMTransY]);
    
    x = do_repeat_mod(x, width - 1);
    y = do_repeat_mod(y, height - 1);

    const SkPMColor* row = srcBitmap.getAddr32(0, y);
    const SkPMColor* src = row + x;

    int n = SkMin32(width - x, count);

    // do the first partial run
    count -= n;
    SkASSERT(n > 0);
    do {
        *dstC++ = SkPixel32ToPixel16(*src++);
    } while (--n > 0);

    // do 1 complete run
    if (count >= width)
    {
        uint16_t* baseDstC = dstC;  // remember the first complete run start
        n = width;
        count -= width;
        src = row;
        do {
            *dstC++ = SkPixel32ToPixel16(*src++);
        } while (--n > 0);

        // do the rest of the complete runs
        while (count >= width)
        {
            count -= width;
            memcpy(dstC, baseDstC, width << 1);
            dstC += width;
        }
        // do final partial run
        if (count > 0)
            memcpy(dstC, baseDstC, count << 1);
    }
    else // do final partial
    {
        if (count > 0)
        {
            src = row;
            do {
                *dstC++ = SkPixel32ToPixel16(*src++);;
            } while (--count > 0);
        }
    }
}

static void ARGB32_RepeatTile_Sprite32(HasSpan16_Sampler_BitmapShader* shader,
                                       int x, int y, SkPMColor dstC[], int count)
{
    SkASSERT(count > 0);

    const SkMatrix& inv = shader->revealTotalInverse();
    const SkBitmap& srcBitmap = shader->revealSrcBitmap();
    int             width = srcBitmap.width();
    int             height = srcBitmap.height();
    
    SkASSERT(width > 0 && height > 0);

    x += SkScalarRound(inv[SkMatrix::kMTransX]);
    y += SkScalarRound(inv[SkMatrix::kMTransY]);
    
    x = do_repeat_mod(x, width - 1);
    y = do_repeat_mod(y, height - 1);

    const SkPMColor* row = srcBitmap.getAddr32(0, y);
    const SkPMColor* src = row + x;

    int n = SkMin32(width - x, count);

    for (;;)
    {
        SkASSERT(n > 0 && count >= n);
        memcpy(dstC, src, n << 2);
        count -= n;
        if (count == 0)
            break;
        dstC += n;
        src = row;
        n = SkMin32(width, count);
    }
}

///////////////////////////////////////////////////////////////////////////

#define NOFILTER_BITMAP_SHADER_CLASS                    Index8_NoFilter_RepeatTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE                 SkShader::kRepeat_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)         (fixed_repeat(x) * (max + 1) >> 16)
#define NOFILTER_BITMAP_SHADER_TYPE                     uint8_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)           colors32[p[x]]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)   colors32[p[x + y * rb]]
#define NOFILTER_BITMAP_SHADER_PREAMBLE(bitmap, rb)     const SkPMColor* colors32 = bitmap.getColorTable()->lockColors()
#define NOFILTER_BITMAP_SHADER_POSTAMBLE(bitmap)        bitmap.getColorTable()->unlockColors(false)
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)         colors16[p[x]]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb) colors16[p[x + y * rb]]
#define NOFILTER_BITMAP_SHADER_PREAMBLE16(bitmap, rb)   const uint16_t* colors16 = bitmap.getColorTable()->lock16BitCache()
#define NOFILTER_BITMAP_SHADER_POSTAMBLE16(bitmap)      bitmap.getColorTable()->unlock16BitCache()
#define NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
#define NOFILTER_BITMAP_SHADER_SPRITEPROC16             Index8_RepeatTile_Sprite16
#define NOFILTER_BITMAP_SHADER_SPRITEPROC32             Index8_RepeatTile_Sprite32
#include "SkBitmapShaderTemplate.h"

#define NOFILTER_BITMAP_SHADER_CLASS                    U16_NoFilter_RepeatTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE                 SkShader::kRepeat_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)         (fixed_repeat(x) * (max + 1) >> 16)
#define NOFILTER_BITMAP_SHADER_TYPE                     uint16_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)           SkPixel16ToPixel32(p[x])
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)   SkPixel16ToPixel32(*(const uint16_t*)((const char*)p + y * rb + (x << 1)))
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)         p[x]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb) *(const uint16_t*)((const char*)p + y * rb + (x << 1))
#define NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
#define NOFILTER_BITMAP_SHADER_SPRITEPROC16             RGB16_RepeatTile_Sprite16
#define NOFILTER_BITMAP_SHADER_SPRITEPROC32             RGB16_RepeatTile_Sprite32
#include "SkBitmapShaderTemplate.h"

#define NOFILTER_BITMAP_SHADER_CLASS                    U32_NoFilter_RepeatTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE                 SkShader::kRepeat_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)         (fixed_repeat(x) * (max + 1) >> 16)
#define NOFILTER_BITMAP_SHADER_TYPE                     uint32_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)           p[x]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)   *(const uint32_t*)((const char*)p + y * rb + (x << 2))
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)         SkPixel32ToPixel16_ToU16(p[x])
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb) SkPixel32ToPixel16_ToU16(*(const uint32_t*)((const char*)p + y * rb + (x << 2)))
#define NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
#define NOFILTER_BITMAP_SHADER_SPRITEPROC16             ARGB32_RepeatTile_Sprite16
#define NOFILTER_BITMAP_SHADER_SPRITEPROC32             ARGB32_RepeatTile_Sprite32
#include "SkBitmapShaderTemplate.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////

static inline SkPMColor expanded_rgb16_to_8888(uint32_t c, U8CPU alpha)
{
//    GGGG Gggg gggR RRRR rrrr r|BB BBBb bbbb
    SkASSERT(alpha <= 255);

#if 1
    int scale = SkAlpha255To256(alpha);
    int r = (c & 0xF800) * scale >> 16;
    int g = ((c >> 21) & 0x3F) * scale >> 6;
    int b = (c & 0x1F) * scale >> 5;
    return SkPackARGB32(alpha, r, g, b);
#else
    int scale = SkAlpha255To256(alpha) >> 3;
    c &= 0x07E0F81F;
    c = c * scale;
    int r = (c >> 13) & 0xFF;
    int g = (c >> 24) & 0xFF;
    int b = (c >> 2) & 0xFF;
    return SkPackARGB32(alpha, r, g, b);
#endif
}

#define BILERP_BITMAP16_SHADER_CLASS            U16_Bilerp_BitmapShader
#define BILERP_BITMAP16_SHADER_TYPE             uint16_t
#define BILERP_BITMAP16_SHADER_PREAMBLE(bm)
#define BILERP_BITMAP16_SHADER_PIXEL(c)         (c)
#define BILERP_BITMAP16_SHADER_POSTAMBLE(bm)
#include "SkBitmapShader16BilerpTemplate.h"

#define BILERP_BITMAP16_SHADER_CLASS            Index8_Bilerp_BitmapShader
#define BILERP_BITMAP16_SHADER_TYPE             uint8_t
#define BILERP_BITMAP16_SHADER_PREAMBLE(bm)     SkColorTable* ctable = (bm).getColorTable(); const uint16_t* colors16 = ctable->lock16BitCache()
#define BILERP_BITMAP16_SHADER_PIXEL(c)         colors16[c]
#define BILERP_BITMAP16_SHADER_POSTAMBLE(bm)    ctable->unlock16BitCache()
#include "SkBitmapShader16BilerpTemplate.h"

#include "ARGB32_Clamp_Bilinear_BitmapShader.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include "SkBitmapProcShader.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include "SkTemplatesPriv.h"

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src,
                                       bool doFilter,
                                       TileMode tmx, TileMode tmy,
                                       void* storage, size_t storageSize)
{
#if 1
    
    SkShader* shader;
    SK_PLACEMENT_NEW_ARGS(shader, SkBitmapProcShader, storage,
                          storageSize, (src, doFilter, tmx, tmy));
    return shader;
#else

    if (!doFilter)
    {
        if (kClamp_TileMode == tmx && kClamp_TileMode == tmy)
        {
            SK_PLACEMENT_NEW_ARGS(shader, SkBitmapProcShader, storage,
                                  storageSize, (src, doFilter, tmx, tmy));
        }
        else if (kRepeat_TileMode == tmx && kRepeat_TileMode == tmy)
        {
    #if 1
            SK_PLACEMENT_NEW_ARGS(shader, SkBitmapProcShader, storage,
                                  storageSize, (src, doFilter, tmx, tmy));
    #else
            switch (src.getConfig()) {
            case SkBitmap::kIndex8_Config:
                SK_PLACEMENT_NEW_ARGS(shader, Index8_NoFilter_RepeatTile_BitmapShader, storage, storageSize, (src));
                break;
            case SkBitmap::kRGB_565_Config:
                SK_PLACEMENT_NEW_ARGS(shader, U16_NoFilter_RepeatTile_BitmapShader, storage, storageSize, (src));
                break;
            case SkBitmap::kARGB_8888_Config:
                SK_PLACEMENT_NEW_ARGS(shader, U32_NoFilter_RepeatTile_BitmapShader, storage, storageSize, (src));
                break;
            default:
                break;
            }
    #endif
        }
    }
    else if (kClamp_TileMode == tmx && kClamp_TileMode == tmy)
    {
#if 1
        if (SkBitmapProcShader::CanDo(src, tmx, tmy))
        {
            SK_PLACEMENT_NEW_ARGS(shader, SkBitmapProcShader, storage,
                                  storageSize, (src, doFilter, tmx, tmy));
        }
#else
        switch (src.getConfig()) {
        case SkBitmap::kIndex8_Config:
            if (src.isOpaque())
                SK_PLACEMENT_NEW_ARGS(shader, Index8_Bilerp_BitmapShader, storage, storageSize, (src));
            break;
        case SkBitmap::kRGB_565_Config:
            SK_PLACEMENT_NEW_ARGS(shader, U16_Bilerp_BitmapShader, storage, storageSize, (src));
            break;
        case SkBitmap::kARGB_8888_Config:
            SK_PLACEMENT_NEW_ARGS(shader, ARGB32_Clamp_Bilinear_BitmapShader, storage, storageSize, (src));
            break;
        default:
            break;
        }
#endif
    }
    
    // if shader is null, then none of the special cases could handle the request
    // so fall through to our slow-general case
    if (shader == NULL)
        SK_PLACEMENT_NEW_ARGS(shader, Sampler_BitmapShader, storage, storageSize,
                              (src, doFilter, tmx, tmy));
    return shader;
#endif
}

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src, bool doFilter,
                                       TileMode tmx, TileMode tmy)
{
    return SkShader::CreateBitmapShader(src, doFilter, tmx, tmy, NULL, 0);
}

#endif
