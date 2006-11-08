/* libs/graphics/effects/SkGradientShader.cpp
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

#include "SkGradientShader.h"
#include "SkColorPriv.h"
#include "SkUnitMapper.h"
#include "SkUtils.h"

/*
    ToDo

    - not sure we still need the full Rec struct, now that we're using a cache
    - detect const-alpha (but not opaque) in getFlags()
*/

/* dither seems to look better, but not stuningly yet, and it slows us down a little
    so its not on by default yet.
*/
#define TEST_GRADIENT_DITHER

///////////////////////////////////////////////////////////////////////////

typedef SkFixed (*TileProc)(SkFixed);

static SkFixed clamp_tileproc(SkFixed x)
{
    return SkClampMax(x, 0xFFFF);
}

static SkFixed repeat_tileproc(SkFixed x)
{
    return x & 0xFFFF;
}

static inline SkFixed mirror_tileproc(SkFixed x)
{
    int s = x << 15 >> 31;
    return (x ^ s) & 0xFFFF;
}

static const TileProc gTileProcs[] = {
    clamp_tileproc,
    repeat_tileproc,
    mirror_tileproc
};

//////////////////////////////////////////////////////////////////////////////

static inline int repeat_6bits(int x)
{
    return x & 63;
}

static inline int mirror_6bits(int x)
{
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
    if (x & 64)
        x = ~x;
    return x & 63;
#else
    int s = x << 25 >> 31;
    return (x ^ s) & 63;
#endif
}

static inline int repeat_8bits(int x)
{
    return x & 0xFF;
}

static inline int mirror_8bits(int x)
{
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
    if (x & 256)
        x = ~x;
    return x & 255;
#else
    int s = x << 23 >> 31;
    return (x ^ s) & 0xFF;
#endif
}

//////////////////////////////////////////////////////////////////////////////

class Gradient_Shader : public SkShader {
public:
    Gradient_Shader(const SkColor colors[], const SkScalar pos[],
                    int colorCount, SkShader::TileMode mode, SkUnitMapper* mapper);
    virtual ~Gradient_Shader();

    // overrides
    virtual bool setContext(const SkBitmap&, const SkPaint&, const SkMatrix&);
    virtual uint32_t getFlags() { return fFlags; }

protected:
    SkUnitMapper* fMapper;
    SkMatrix    fPtsToUnit;     // set by subclass
    SkMatrix    fDstToIndex;
    SkMatrix::MapPtProc fDstToIndexProc;
    SkPMColor*  fARGB32;
    TileProc    fTileProc;
    uint16_t    fColorCount;
    uint8_t     fDstToIndexClass;
    uint8_t     fFlags;

    struct Rec {
        SkFixed     fPos;   // 0...1
        uint32_t    fScale; // (1 << 24) / range
    };
    Rec*        fRecs;

    enum {
        kCache16Bits    = 6,    // seems like enough for visual accuracy
        kCache16Count   = 1 << kCache16Bits,
        kCache32Bits    = 8,    // pretty much should always be 8
        kCache32Count   = 1 << kCache32Bits
    };
    const uint16_t*     getCache16();
    const SkPMColor*    getCache32();

private:
    enum {
        kColorStorageCount = 4, // more than this many colors, and we'll use sk_malloc for the space

        kStorageSize = kColorStorageCount * (sizeof(SkColor) + sizeof(SkPMColor) + sizeof(Rec))
    };
    SkColor     fStorage[(kStorageSize + 3) >> 2];
    SkColor*    fOrigColors;

    uint16_t*   fCache16;   // working ptr. If this is NULL, we need to recompute the cache values
    SkPMColor*  fCache32;   // working ptr. If this is NULL, we need to recompute the cache values

    uint16_t*   fCache16Storage;    // storage for fCache16, allocated on demand
    SkPMColor*  fCache32Storage;    // storage for fCache32, allocated on demand
    unsigned    fCacheAlpha;        // the alpha value we used when we computed the cache. larger than 8bits so we can store uninitialized value

    typedef SkShader INHERITED;
};

static inline unsigned scalarToU16(SkScalar x)
{
    SkASSERT(x >= 0 && x <= SK_Scalar1);

#ifdef SK_SCALAR_IS_FLOAT
    return (unsigned)(x * 0xFFFF);
#else
    return x - (x >> 16);   // probably should be x - (x > 0x7FFF) but that is slower
#endif
}

Gradient_Shader::Gradient_Shader(const SkColor colors[], const SkScalar pos[], int colorCount,
                                 SkShader::TileMode mode, SkUnitMapper* mapper)
{
    SkASSERT(colorCount > 1);

    fCacheAlpha = 256;  // init to a value that paint.getAlpha() can't return

    fMapper = mapper;
    mapper->safeRef();

    fCache16 = fCache16Storage = NULL;
    fCache32 = fCache32Storage = NULL;

    fColorCount = SkToU16(colorCount);
    if (colorCount > kColorStorageCount)
        fOrigColors = (SkColor*)sk_malloc_throw((sizeof(SkColor) + sizeof(SkPMColor) + sizeof(Rec)) * colorCount);
    else
        fOrigColors = fStorage;
    memcpy(fOrigColors, colors, colorCount * sizeof(SkColor));
    // our premul colors point to the 2nd half of the array
    // these are assigned each time in setContext
    fARGB32 = fOrigColors + colorCount;

    SkASSERT((unsigned)mode < SkShader::kTileModeCount);
    SkASSERT(SkShader::kTileModeCount == SK_ARRAY_COUNT(gTileProcs));
    fTileProc = gTileProcs[mode];

    fRecs = (Rec*)(fARGB32 + colorCount);
    if (colorCount > 2)
    {
        Rec* recs = fRecs;

        recs[0].fPos = 0;
    //  recs[0].fScale = 0; // unused;
        if (pos)
        {
            for (int i = 1; i < colorCount; i++)
            {
                SkASSERT(pos[i] > pos[i-1] && pos[i] <= SK_Scalar1);
                recs[i].fPos    = SkScalarToFixed(pos[i]);
                recs[i].fScale  = (1 << 24) / SkScalarToFixed(pos[i] - pos[i-1]);
            }
        }
        else    // assume even distribution
        {
            SkFixed dp = SK_Fixed1 / (colorCount - 1);
            SkFixed p = dp;
            SkFixed scale = (colorCount - 1) << 8;  // (1 << 24) / dp
            for (int i = 1; i < colorCount; i++)
            {
                recs[i].fPos    = p;
                recs[i].fScale  = scale;
                p += dp;
            }
        }
    }
}

Gradient_Shader::~Gradient_Shader()
{
    if (fCache16Storage)
        sk_free(fCache16Storage);
    if (fCache32Storage)
        sk_free(fCache32Storage);
    if (fOrigColors != fStorage)
        sk_free(fOrigColors);
    fMapper->safeUnref();
}

bool Gradient_Shader::setContext(const SkBitmap& device,
                                 const SkPaint& paint,
                                 const SkMatrix& matrix)
{
    if (!this->INHERITED::setContext(device, paint, matrix))
        return false;

    const SkMatrix& inverse = this->getTotalInverse();

    if (!fDstToIndex.setConcat(fPtsToUnit, inverse))
        return false;

    fDstToIndexProc = fDstToIndex.getMapPtProc();
    fDstToIndexClass = (uint8_t)SkShader::ComputeMatrixClass(fDstToIndex);

    // now convert our colors in to PMColors
    int pa = this->getPaintAlpha();
    int scale = SkAlpha255To256(pa);
    int origColorAlpha = 0xFF;
    int prevAlpha = -1;

    for (unsigned i = 0; i < fColorCount; i++)
    {
        SkColor src = fOrigColors[i];
        int sa = SkColorGetA(src);
        origColorAlpha &= sa;

        sa = SkAlphaMul(sa, scale);
        pa &= sa;

        int localScale = SkAlpha255To256(sa);

        if (i == 0)
            prevAlpha = sa;
        else
        {
            if (prevAlpha != sa)
                prevAlpha = -1;
        }

        fARGB32[i] = SkPackARGB32(  sa,
                                    SkAlphaMul(SkColorGetR(src), localScale),
                                    SkAlphaMul(SkColorGetG(src), localScale),
                                    SkAlphaMul(SkColorGetB(src), localScale));
    }

    fFlags = this->INHERITED::getFlags();
    if (pa == 0xFF)
    {
        SkASSERT(prevAlpha >= 0);
        fFlags |= kOpaqueAlpha_Flag;
    }
    // we can do span16 as long as our individual colors are opaque, regardless of the paint's alpha
    if (origColorAlpha == 0xFF)
        fFlags |= kHasSpan16_Flag;

    // if the new alpha differs from the previous time we were called, inval our cache
    // this will trigger the cache to be rebuilt.
    // we don't care about the first time, since the cache ptrs will already be NULL
    if (fCacheAlpha != paint.getAlpha())
    {
        fCache16 = NULL;                 // inval the cache
        fCache32 = NULL;                 // inval the cache
        fCacheAlpha = paint.getAlpha(); // record the new alpha
    }
    return true;
}

static inline int blend8(int a, int b, int scale)
{
    SkASSERT(a == SkToU8(a));
    SkASSERT(b == SkToU8(b));
    SkASSERT(scale >= 0 && scale <= 256);

    return a + ((b - a) * scale >> 8);
}

static inline uint32_t dot8_blend_packed32(uint32_t s0, uint32_t s1, int blend)
{
#if 0
    int a = blend8(SkGetPackedA32(s0), SkGetPackedA32(s1), blend);
    int r = blend8(SkGetPackedR32(s0), SkGetPackedR32(s1), blend);
    int g = blend8(SkGetPackedG32(s0), SkGetPackedG32(s1), blend);
    int b = blend8(SkGetPackedB32(s0), SkGetPackedB32(s1), blend);

    return SkPackARGB32(a, r, g, b);
#else
    int otherBlend = 256 - blend;

#if 0
    U32 t0 = (((s0 & 0xFF00FF) * blend + (s1 & 0xFF00FF) * otherBlend) >> 8) & 0xFF00FF;
    U32 t1 = (((s0 >> 8) & 0xFF00FF) * blend + ((s1 >> 8) & 0xFF00FF) * otherBlend) & 0xFF00FF00;
    SkASSERT((t0 & t1) == 0);
    return t0 | t1;
#else
    return  ((((s0 & 0xFF00FF) * blend + (s1 & 0xFF00FF) * otherBlend) >> 8) & 0xFF00FF) |
            ((((s0 >> 8) & 0xFF00FF) * blend + ((s1 >> 8) & 0xFF00FF) * otherBlend) & 0xFF00FF00);
#endif

#endif
}

#define Fixed_To_Dot8(x)        (((x) + 0x80) >> 8)

/** We take the original colors, not our premultiplied PMColors, since we can build a 16bit table
    as long as the original colors are opaque, even if the paint specifies a non-opaque alpha.
*/
static void build_16bit_cache(uint16_t cache[], SkColor c0, SkColor c1, int count)
{
    SkASSERT(count > 1);
    SkASSERT(SkColorGetA(c0) == 0xFF);
    SkASSERT(SkColorGetA(c1) == 0xFF);

    SkFixed r = SkColorGetR(c0);
    SkFixed g = SkColorGetG(c0);
    SkFixed b = SkColorGetB(c0);

    SkFixed dr = SkIntToFixed(SkColorGetR(c1) - r) / (count - 1);
    SkFixed dg = SkIntToFixed(SkColorGetG(c1) - g) / (count - 1);
    SkFixed db = SkIntToFixed(SkColorGetB(c1) - b) / (count - 1);

    r = SkIntToFixed(r) + 0x8000;
    g = SkIntToFixed(g) + 0x8000;
    b = SkIntToFixed(b) + 0x8000;

    do {
        unsigned rr = r >> 16;
        unsigned gg = g >> 16;
        unsigned bb = b >> 16;
        cache[0] = SkPackRGB16(SkR32ToR16(rr), SkG32ToG16(gg), SkB32ToB16(bb));
        cache[64] = SkDitherPack888ToRGB16(rr, gg, bb);
        cache += 1;
        r += dr;
        g += dg;
        b += db;
    } while (--count != 0);
}

static void build_32bit_cache(SkPMColor cache[], SkPMColor c0, SkPMColor c1, int count)
{
    SkASSERT(count > 1);

    SkFixed a = SkGetPackedA32(c0);
    SkFixed r = SkGetPackedR32(c0);
    SkFixed g = SkGetPackedG32(c0);
    SkFixed b = SkGetPackedB32(c0);

    SkFixed da = SkIntToFixed(SkGetPackedA32(c1) - a) / (count - 1);
    SkFixed dr = SkIntToFixed(SkGetPackedR32(c1) - r) / (count - 1);
    SkFixed dg = SkIntToFixed(SkGetPackedG32(c1) - g) / (count - 1);
    SkFixed db = SkIntToFixed(SkGetPackedB32(c1) - b) / (count - 1);

    a = SkIntToFixed(a) + 0x8000;
    r = SkIntToFixed(r) + 0x8000;
    g = SkIntToFixed(g) + 0x8000;
    b = SkIntToFixed(b) + 0x8000;

    do {
        *cache++ = SkPackARGB32(a >> 16, r >> 16, g >> 16, b >> 16);
        a += da;
        r += dr;
        g += dg;
        b += db;
    } while (--count != 0);
}

static inline int SkFixedToFFFF(SkFixed x)
{
    SkASSERT((unsigned)x <= SK_Fixed1);
    return x - (x >> 16);
}

static inline U16CPU dot6to16(unsigned x)
{
    SkASSERT(x < 64);
    return (x << 10) | (x << 4) | (x >> 2);
}

const uint16_t* Gradient_Shader::getCache16()
{
    if (fCache16 == NULL)
    {
        if (fCache16Storage == NULL) // set the storage and our working ptr
#ifdef TEST_GRADIENT_DITHER
            fCache16Storage = (uint16_t*)sk_malloc_throw(sizeof(uint16_t) * kCache16Count * 2);
#else
            fCache16Storage = (uint16_t*)sk_malloc_throw(sizeof(uint16_t) * kCache16Count);
#endif
        fCache16 = fCache16Storage;
        if (fColorCount == 2)
            build_16bit_cache(fCache16, fOrigColors[0], fOrigColors[1], kCache16Count);
        else
        {
            Rec* rec = fRecs;
            int prevIndex = 0;
            for (unsigned i = 1; i < fColorCount; i++)
            {
                int nextIndex = SkFixedToFFFF(rec[i].fPos) >> (16 - kCache16Bits);
                SkASSERT(nextIndex < kCache16Count);

                if (nextIndex > prevIndex)
                    build_16bit_cache(fCache16 + prevIndex, fOrigColors[i-1], fOrigColors[i], nextIndex - prevIndex + 1);
                prevIndex = nextIndex;
            }
            SkASSERT(prevIndex == kCache16Count - 1);
        }

        if (fMapper)
        {
#ifdef TEST_GRADIENT_DITHER
            fCache16Storage = (uint16_t*)sk_malloc_throw(sizeof(uint16_t) * kCache16Count * 2);
#else
            fCache16Storage = (uint16_t*)sk_malloc_throw(sizeof(uint16_t) * kCache16Count);
#endif
            uint16_t* linear = fCache16;         // just computed linear data
            uint16_t* mapped = fCache16Storage;  // storage for mapped data
            SkUnitMapper* map = fMapper;
            for (int i = 0; i < 64; i++)
            {
                int index = map->mapUnit16(dot6to16(i)) >> 10;
                mapped[i] = linear[index];
#ifdef TEST_GRADIENT_DITHER
                mapped[i + 64] = linear[index + 64];
#endif
            }
            sk_free(fCache16);
            fCache16 = fCache16Storage;
        }
    }
    return fCache16;
}

const SkPMColor* Gradient_Shader::getCache32()
{
    if (fCache32 == NULL)
    {
        if (fCache32Storage == NULL) // set the storage and our working ptr
            fCache32Storage = (SkPMColor*)sk_malloc_throw(sizeof(SkPMColor) * kCache32Count);

        fCache32 = fCache32Storage;
        if (fColorCount == 2)
            build_32bit_cache(fCache32, fARGB32[0], fARGB32[1], kCache32Count);
        else
        {
            Rec* rec = fRecs;
            int prevIndex = 0;
            for (unsigned i = 1; i < fColorCount; i++)
            {
                int nextIndex = SkFixedToFFFF(rec[i].fPos) >> (16 - kCache32Bits);
                SkASSERT(nextIndex < kCache32Count);

                if (nextIndex > prevIndex)
                    build_32bit_cache(fCache32 + prevIndex, fARGB32[i-1], fARGB32[i], nextIndex - prevIndex + 1);
                prevIndex = nextIndex;
            }
            SkASSERT(prevIndex == kCache32Count - 1);
        }

        if (fMapper)
        {
            fCache32Storage = (SkPMColor*)sk_malloc_throw(sizeof(SkPMColor) * kCache32Count);
            SkPMColor* linear = fCache32;           // just computed linear data
            SkPMColor* mapped = fCache32Storage;    // storage for mapped data
            SkUnitMapper* map = fMapper;
            for (int i = 0; i < 256; i++)
                mapped[i] = linear[map->mapUnit16((i << 8) | i) >> 8];
            sk_free(fCache32);
            fCache32 = fCache32Storage;
        }
    }
    return fCache32;
}

///////////////////////////////////////////////////////////////////////////

static void pts_to_unit_matrix(const SkPoint pts[2], SkMatrix* matrix)
{
    SkVector    vec = pts[1] - pts[0];
    SkScalar    mag = vec.length();
    SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    matrix->setSinCos(-vec.fY, vec.fX, pts[0].fX, pts[0].fY);
    matrix->postTranslate(-pts[0].fX, -pts[0].fY);
    matrix->postScale(inv, inv);
}

class Linear_Gradient : public Gradient_Shader {
public:
    Linear_Gradient(const SkPoint pts[2],
                    const SkColor colors[], const SkScalar pos[], int colorCount,
                    SkShader::TileMode mode, SkUnitMapper* mapper)
        : Gradient_Shader(colors, pos, colorCount, mode, mapper)
    {
        pts_to_unit_matrix(pts, &fPtsToUnit);
    }
    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count);
    virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count);

private:
    typedef Gradient_Shader INHERITED;
};

void Linear_Gradient::shadeSpan(int x, int y, SkPMColor dstC[], int count)
{
    SkASSERT(count > 0);

    SkPoint             srcPt;
    SkMatrix::MapPtProc dstProc = fDstToIndexProc;
    TileProc            proc = fTileProc;
    const SkPMColor*    cache = this->getCache32();

    if (fDstToIndexClass != kPerspective_MatrixClass)
    {
        dstProc(fDstToIndex, SkIntToScalar(x), SkIntToScalar(y), &srcPt);
        SkFixed dx, fx = SkScalarToFixed(srcPt.fX);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass)
        {
            SkFixed dxStorage[1];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), dxStorage, NULL);
            dx = dxStorage[0];
        }
        else
        {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToFixed(fDstToIndex.getScaleX());
        }

        if (SkFixedNearlyZero(dx))  // we're a vertical gradient, so no change in a span
        {
            unsigned fi = proc(fx);
            SkASSERT(fi <= 0xFFFF);
            sk_memset32(dstC, cache[fi >> (16 - kCache32Bits)], count);
        }
        else if (proc == clamp_tileproc)
        {
            do {
                unsigned fi = SkClampMax(fx >> 8, 0xFF);
                SkASSERT(fi <= 0xFF);
                fx += dx;
                *dstC++ = cache[fi];
            } while (--count != 0);
        }
        else if (proc == mirror_tileproc)
        {
            do {
                unsigned fi = mirror_8bits(fx >> 8);
                SkASSERT(fi <= 0xFF);
                fx += dx;
                *dstC++ = cache[fi];
            } while (--count != 0);
        }
        else
        {
            SkASSERT(proc == repeat_tileproc);
            do {
                unsigned fi = repeat_8bits(fx >> 8);
                SkASSERT(fi <= 0xFF);
                fx += dx;
                *dstC++ = cache[fi];
            } while (--count != 0);
        }
    }
    else
    {
        SkScalar    dstX = SkIntToScalar(x);
        SkScalar    dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.fX));
            SkASSERT(fi <= 0xFFFF);
            *dstC++ = cache[fi >> (16 - kCache32Bits)];
            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

#ifdef TEST_GRADIENT_DITHER
static void dither_memset16(uint16_t dst[], uint16_t value, uint16_t other, int count)
{
    if ((unsigned)dst & 2)
    {
        *dst++ = value;
        count -= 1;
        SkTSwap(value, other);
    }

    sk_memset32((uint32_t*)dst, (value << 16) | other, count >> 1);
    
    if (count & 1)
        dst[count - 1] = value;
}
#endif

void Linear_Gradient::shadeSpan16(int x, int y, uint16_t dstC[], int count)
{
    SkASSERT(count > 0);

    SkPoint             srcPt;
    SkMatrix::MapPtProc dstProc = fDstToIndexProc;
    TileProc            proc = fTileProc;
    const uint16_t*     cache = this->getCache16();
#ifdef TEST_GRADIENT_DITHER
    int                 toggle = ((x ^ y) & 1) << kCache16Bits;
#endif

    if (fDstToIndexClass != kPerspective_MatrixClass)
    {
        dstProc(fDstToIndex, SkIntToScalar(x), SkIntToScalar(y), &srcPt);
        SkFixed dx, fx = SkScalarToFixed(srcPt.fX);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass)
        {
            SkFixed dxStorage[1];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), dxStorage, NULL);
            dx = dxStorage[0];
        }
        else
        {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToFixed(fDstToIndex.getScaleX());
        }

        if (SkFixedNearlyZero(dx))  // we're a vertical gradient, so no change in a span
        {
            unsigned fi = proc(fx) >> 10;
            SkASSERT(fi <= 63);
#ifdef TEST_GRADIENT_DITHER
            dither_memset16(dstC, cache[toggle + fi], cache[(toggle ^ (1 << kCache16Bits)) + fi], count);
#else
            sk_memset16(dstC, cache[fi], count);
#endif
        }
        else if (proc == clamp_tileproc)
        {
            do {
                unsigned fi = SkClampMax(fx >> 10, 63);
                SkASSERT(fi <= 63);
                fx += dx;
#ifdef TEST_GRADIENT_DITHER
                *dstC++ = cache[toggle + fi];
                toggle ^= (1 << kCache16Bits);
#else
                *dstC++ = cache[fi];
#endif
            } while (--count != 0);
        }
        else if (proc == mirror_tileproc)
        {
            do {
                unsigned fi = mirror_6bits(fx >> 10);
                SkASSERT(fi <= 0x3F);
                fx += dx;
#ifdef TEST_GRADIENT_DITHER
                *dstC++ = cache[toggle + fi];
                toggle ^= (1 << kCache16Bits);
#else
                *dstC++ = cache[fi];
#endif
            } while (--count != 0);
        }
        else
        {
            SkASSERT(proc == repeat_tileproc);
            do {
                unsigned fi = repeat_6bits(fx >> 10);
                SkASSERT(fi <= 0x3F);
                fx += dx;
#ifdef TEST_GRADIENT_DITHER
                *dstC++ = cache[toggle + fi];
                toggle ^= (1 << kCache16Bits);
#else
                *dstC++ = cache[fi];
#endif
            } while (--count != 0);
        }
    }
    else
    {
        SkScalar    dstX = SkIntToScalar(x);
        SkScalar    dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.fX));
            SkASSERT(fi <= 0xFFFF);

            int index = fi >> (16 - kCache16Bits);
#ifdef TEST_GRADIENT_DITHER
            *dstC++ = cache[toggle + index];
            toggle ^= (1 << kCache16Bits);
#else
            *dstC++ = cache[index];
#endif

            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#define kSQRT_TABLE_BITS    11
#define kSQRT_TABLE_SIZE    (1 << kSQRT_TABLE_BITS)

#include "SkRadialGradient_Table.h"

#if defined(SK_BUILD_FOR_WIN32) && defined(SK_DEBUG)

#include <stdio.h>

void SkRadialGradient_BuildTable()
{
    // build it 0..127 x 0..127, so we use 2^15 - 1 in the numerator for our "fixed" table

    FILE* file = ::fopen("SkRadialGradient_Table.h", "w");
    SkASSERT(file);
    ::fprintf(file, "static const U8 gSqrt8Table[] = {\n");

    for (int i = 0; i < kSQRT_TABLE_SIZE; i++)
    {
        if ((i & 15) == 0)
            ::fprintf(file, "\t");

        U8 value = SkToU8(SkFixedSqrt(i * SK_Fixed1 / kSQRT_TABLE_SIZE) >> 8);

        ::fprintf(file, "0x%02X", value);
        if (i < kSQRT_TABLE_SIZE-1)
            ::fprintf(file, ", ");
        if ((i & 15) == 15)
            ::fprintf(file, "\n");
    }
    ::fprintf(file, "};\n");
    ::fclose(file);
}

#endif


static void rad_to_unit_matrix(const SkPoint& center, SkScalar radius, SkMatrix* matrix)
{
    SkScalar    inv = SkScalarInvert(radius);

    matrix->setTranslate(-center.fX, -center.fY);
    matrix->postScale(inv, inv);
}

class Radial_Gradient : public Gradient_Shader {
public:
    Radial_Gradient(const SkPoint& center, SkScalar radius,
                    const SkColor colors[], const SkScalar pos[], int colorCount,
                    SkShader::TileMode mode, SkUnitMapper* mapper)
        : Gradient_Shader(colors, pos, colorCount, mode, mapper)
    {
        // make sure our table is insync with our current #define for kSQRT_TABLE_SIZE
        SkASSERT(sizeof(gSqrt8Table) == kSQRT_TABLE_SIZE);

        rad_to_unit_matrix(center, radius, &fPtsToUnit);
    }
    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count)
    {
        SkASSERT(count > 0);

        SkPoint             srcPt;
        SkMatrix::MapPtProc dstProc = fDstToIndexProc;
        TileProc            proc = fTileProc;
        const SkPMColor*    cache = this->getCache32();

        if (fDstToIndexClass != kPerspective_MatrixClass)
        {
            dstProc(fDstToIndex, SkIntToScalar(x), SkIntToScalar(y), &srcPt);
            SkFixed dx, fx = SkScalarToFixed(srcPt.fX);
            SkFixed dy, fy = SkScalarToFixed(srcPt.fY);

            if (fDstToIndexClass == kFixedStepInX_MatrixClass)
            {
                SkFixed storage[2];
                (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), &storage[0], &storage[1]);
                dx = storage[0];
                dy = storage[1];
            }
            else
            {
                SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
                dx = SkScalarToFixed(fDstToIndex.getScaleX());
                dy = SkScalarToFixed(fDstToIndex.getSkewY());
            }

            if (proc == clamp_tileproc)
            {
                const U8* sqrt_table = gSqrt8Table;
                fx >>= 1;
                dx >>= 1;
                fy >>= 1;
                dy >>= 1;
                do {
                    unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
                    unsigned fi = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
                    fi = (xx * xx + fi * fi) >> (14 + 16 - kSQRT_TABLE_BITS);
                    fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
                    *dstC++ = cache[sqrt_table[fi] >> (8 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                } while (--count != 0);
            }
            else if (proc == mirror_tileproc)
            {
                do {
                    SkFixed dist = SkFixedSqrt(SkFixedSquare(fx) + SkFixedSquare(fy));
                    unsigned fi = mirror_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    *dstC++ = cache[fi >> (16 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                } while (--count != 0);
            }
            else
            {
                SkASSERT(proc == repeat_tileproc);
                do {
                    SkFixed dist = SkFixedSqrt(SkFixedSquare(fx) + SkFixedSquare(fy));
                    unsigned fi = repeat_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    *dstC++ = cache[fi >> (16 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                } while (--count != 0);
            }
        }
        else    // perspective case
        {
            SkScalar dstX = SkIntToScalar(x);
            SkScalar dstY = SkIntToScalar(y);
            do {
                dstProc(fDstToIndex, dstX, dstY, &srcPt);
                unsigned fi = proc(SkScalarToFixed(srcPt.length()));
                SkASSERT(fi <= 0xFFFF);
                *dstC++ = cache[fi >> (16 - kCache32Bits)];
                dstX += SK_Scalar1;
            } while (--count != 0);
        }
    }
    virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count)
    {
        SkASSERT(count > 0);

        SkPoint             srcPt;
        SkMatrix::MapPtProc dstProc = fDstToIndexProc;
        TileProc            proc = fTileProc;
        const uint16_t*     cache = this->getCache16();
#ifdef TEST_GRADIENT_DITHER
        int                 toggle = ((x ^ y) & 1) << kCache16Bits;
#endif

        if (fDstToIndexClass != kPerspective_MatrixClass)
        {
            dstProc(fDstToIndex, SkIntToScalar(x), SkIntToScalar(y), &srcPt);
            SkFixed dx, fx = SkScalarToFixed(srcPt.fX);
            SkFixed dy, fy = SkScalarToFixed(srcPt.fY);

            if (fDstToIndexClass == kFixedStepInX_MatrixClass)
            {
                SkFixed storage[2];
                (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), &storage[0], &storage[1]);
                dx = storage[0];
                dy = storage[1];
            }
            else
            {
                SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
                dx = SkScalarToFixed(fDstToIndex.getScaleX());
                dy = SkScalarToFixed(fDstToIndex.getSkewY());
            }

            if (proc == clamp_tileproc)
            {
                const U8* sqrt_table = gSqrt8Table;

                /* knock these down so we can pin against +- 0x7FFF, which is an immediate load,
                    rather than 0xFFFF which is slower. This is a compromise, since it reduces our
                    precision, but that appears to be visually OK. If we decide this is OK for
                    all of our cases, we could (it seems) put this scale-down into fDstToIndex,
                    to avoid having to do these extra shifts each time.
                */
                fx >>= 1;
                dx >>= 1;
                fy >>= 1;
                dy >>= 1;
                if (dy == 0)    // might perform this check for the other modes, but the win will be a smaller % of the total
                {
                    fy = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
                    fy *= fy;
                    do {
                        unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
                        unsigned fi = (xx * xx + fy) >> (14 + 16 - kSQRT_TABLE_BITS);
                        fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
                        fx += dx;
#ifdef TEST_GRADIENT_DITHER
                        *dstC++ = cache[toggle + (sqrt_table[fi] >> (8 - kCache16Bits))];
                        toggle ^= (1 << kCache16Bits);
#else
                        *dstC++ = cache[sqrt_table[fi] >> (8 - kCache16Bits)];
#endif
                    } while (--count != 0);
                }
                else
                {
                    do {
                        unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
                        unsigned fi = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
                        fi = (xx * xx + fi * fi) >> (14 + 16 - kSQRT_TABLE_BITS);
                        fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
                        fx += dx;
                        fy += dy;
#ifdef TEST_GRADIENT_DITHER
                        *dstC++ = cache[toggle + (sqrt_table[fi] >> (8 - kCache16Bits))];
                        toggle ^= (1 << kCache16Bits);
#else
                        *dstC++ = cache[sqrt_table[fi] >> (8 - kCache16Bits)];
#endif
                    } while (--count != 0);
                }
            }
            else if (proc == mirror_tileproc)
            {
                do {
                    SkFixed dist = SkFixedSqrt(SkFixedSquare(fx) + SkFixedSquare(fy));
                    unsigned fi = mirror_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    fx += dx;
                    fy += dy;
#ifdef TEST_GRADIENT_DITHER
                    *dstC++ = cache[toggle + (fi >> (16 - kCache16Bits))];
                    toggle ^= (1 << kCache16Bits);
#else
                    *dstC++ = cache[fi >> (16 - kCache16Bits)];
#endif
                } while (--count != 0);
            }
            else
            {
                SkASSERT(proc == repeat_tileproc);
                do {
                    SkFixed dist = SkFixedSqrt(SkFixedSquare(fx) + SkFixedSquare(fy));
                    unsigned fi = repeat_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    fx += dx;
                    fy += dy;
#ifdef TEST_GRADIENT_DITHER
                    *dstC++ = cache[toggle + (fi >> (16 - kCache16Bits))];
                    toggle ^= (1 << kCache16Bits);
#else
                    *dstC++ = cache[fi >> (16 - kCache16Bits)];
#endif
                } while (--count != 0);
            }
        }
        else    // perspective case
        {
            SkScalar dstX = SkIntToScalar(x);
            SkScalar dstY = SkIntToScalar(y);
            do {
                dstProc(fDstToIndex, dstX, dstY, &srcPt);
                unsigned fi = proc(SkScalarToFixed(srcPt.length()));
                SkASSERT(fi <= 0xFFFF);

                int index = fi >> (16 - kCache16Bits);
#ifdef TEST_GRADIENT_DITHER
                *dstC++ = cache[toggle + index];
                toggle ^= (1 << kCache16Bits);
#else
                *dstC++ = cache[index];
#endif

                dstX += SK_Scalar1;
            } while (--count != 0);
        }
    }
private:
    typedef Gradient_Shader INHERITED;
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

SkDiscreteMapper::SkDiscreteMapper(unsigned segments)
{
    if (segments < 2)
    {
        fSegments = 0;
        fScale = 0;
    }
    else
    {
        fSegments = segments;
        fScale = SK_Fract1 / (segments - 1);
    }
}

U16CPU SkDiscreteMapper::mapUnit16(U16CPU x)
{
    x = x * fSegments >> 16;
    return x * fScale >> 14;
}

U16CPU SkFlipCosineMapper::mapUnit16(U16CPU x)
{
    x = SkFixedCos(x * (SK_FixedPI >> 2) >> 15);
    x += x << 15 >> 31; // map 0x10000 to 0xFFFF
    x = 0xFFFF - x;
    return x;
}

SkShader* SkGradientShader::CreateLinear(   const SkPoint pts[2],
                                            const SkColor colors[], const SkScalar pos[], int colorCount,
                                            TileMode mode, SkUnitMapper* mapper)
{
    SkASSERT(pts && colors && colorCount >= 2);

    return SkNEW_ARGS(Linear_Gradient, (pts, colors, pos, colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateRadial(   const SkPoint& center, SkScalar radius,
                                            const SkColor colors[], const SkScalar pos[], int colorCount,
                                            TileMode mode, SkUnitMapper* mapper)
{
    SkASSERT(radius > 0 && colors && colorCount >= 2);

    return SkNEW_ARGS(Radial_Gradient, (center, radius, colors, pos, colorCount, mode, mapper));
}

