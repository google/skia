/* libs/graphics/effects/SkGradientShader.cpp
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

#include "SkGradientShader.h"
#include "SkColorPriv.h"
#include "SkMallocPixelRef.h"
#include "SkUnitMapper.h"
#include "SkUtils.h"
#include "SkTemplates.h"
#include "SkBitmapCache.h"

#ifndef SK_DISABLE_DITHER_32BIT_GRADIENT
    #define USE_DITHER_32BIT_GRADIENT
#endif

static void sk_memset32_dither(uint32_t dst[], uint32_t v0, uint32_t v1,
                               int count) {
    if (count > 0) {
        if (v0 == v1) {
            sk_memset32(dst, v0, count);
        } else {
            int pairs = count >> 1;
            for (int i = 0; i < pairs; i++) {
                *dst++ = v0;
                *dst++ = v1;
            }
            if (count & 1) {
                *dst = v0;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Can't use a two-argument function with side effects like this in a
// constructor's initializer's argument list because the order of
// evaluations in that context is undefined (and backwards on linux/gcc).
static SkPoint unflatten_point(SkReader32& buffer) {
    SkPoint retval;
    retval.fX = buffer.readScalar();
    retval.fY = buffer.readScalar();
    return retval;
}

///////////////////////////////////////////////////////////////////////////////

typedef SkFixed (*TileProc)(SkFixed);

static SkFixed clamp_tileproc(SkFixed x) {
    return SkClampMax(x, 0xFFFF);
}

static SkFixed repeat_tileproc(SkFixed x) {
    return x & 0xFFFF;
}

static inline SkFixed mirror_tileproc(SkFixed x) {
    int s = x << 15 >> 31;
    return (x ^ s) & 0xFFFF;
}

static const TileProc gTileProcs[] = {
    clamp_tileproc,
    repeat_tileproc,
    mirror_tileproc
};

///////////////////////////////////////////////////////////////////////////////

static inline int repeat_bits(int x, const int bits) {
    return x & ((1 << bits) - 1);
}

static inline int mirror_bits(int x, const int bits) {
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
    if (x & (1 << bits))
        x = ~x;
    return x & ((1 << bits) - 1);
#else
    int s = x << (31 - bits) >> 31;
    return (x ^ s) & ((1 << bits) - 1);
#endif
}

static inline int repeat_8bits(int x) {
    return x & 0xFF;
}

static inline int mirror_8bits(int x) {
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
    if (x & 256) {
        x = ~x;
    }
    return x & 255;
#else
    int s = x << 23 >> 31;
    return (x ^ s) & 0xFF;
#endif
}

///////////////////////////////////////////////////////////////////////////////

class Gradient_Shader : public SkShader {
public:
    Gradient_Shader(const SkColor colors[], const SkScalar pos[],
                int colorCount, SkShader::TileMode mode, SkUnitMapper* mapper);
    virtual ~Gradient_Shader();

    // overrides
    virtual bool setContext(const SkBitmap&, const SkPaint&, const SkMatrix&);
    virtual uint32_t getFlags() { return fFlags; }

protected:
    Gradient_Shader(SkFlattenableReadBuffer& );
    SkUnitMapper* fMapper;
    SkMatrix    fPtsToUnit;     // set by subclass
    SkMatrix    fDstToIndex;
    SkMatrix::MapXYProc fDstToIndexProc;
    TileMode    fTileMode;
    TileProc    fTileProc;
    int         fColorCount;
    uint8_t     fDstToIndexClass;
    uint8_t     fFlags;

    struct Rec {
        SkFixed     fPos;   // 0...1
        uint32_t    fScale; // (1 << 24) / range
    };
    Rec*        fRecs;

    enum {
        kCache16Bits    = 8,    // seems like enough for visual accuracy
        kCache16Count   = 1 << kCache16Bits,
        kCache16Mask    = kCache16Count - 1,
        kCache16Shift   = 16 - kCache16Bits,

        kCache32Bits    = 8,    // pretty much should always be 8
        kCache32Count   = 1 << kCache32Bits
    };
    virtual void flatten(SkFlattenableWriteBuffer& );
    const uint16_t*     getCache16() const;
    const SkPMColor*    getCache32() const;

    void commonAsABitmap(SkBitmap*) const;
    void commonAsAGradient(GradientInfo*) const;

private:
    enum {
        kColorStorageCount = 4, // more than this many colors, and we'll use sk_malloc for the space

        kStorageSize = kColorStorageCount * (sizeof(SkColor) + sizeof(Rec))
    };
    SkColor     fStorage[(kStorageSize + 3) >> 2];
    SkColor*    fOrigColors;

    mutable uint16_t*   fCache16;   // working ptr. If this is NULL, we need to recompute the cache values
    mutable SkPMColor*  fCache32;   // working ptr. If this is NULL, we need to recompute the cache values

    mutable uint16_t*   fCache16Storage;    // storage for fCache16, allocated on demand
    mutable SkMallocPixelRef* fCache32PixelRef;
    mutable unsigned    fCacheAlpha;        // the alpha value we used when we computed the cache. larger than 8bits so we can store uninitialized value

    static void Build16bitCache(uint16_t[], SkColor c0, SkColor c1, int count);
    static void Build32bitCache(SkPMColor[], SkColor c0, SkColor c1, int count,
                                U8CPU alpha);
    void setCacheAlpha(U8CPU alpha) const;

    typedef SkShader INHERITED;
};

static inline unsigned scalarToU16(SkScalar x) {
    SkASSERT(x >= 0 && x <= SK_Scalar1);

#ifdef SK_SCALAR_IS_FLOAT
    return (unsigned)(x * 0xFFFF);
#else
    return x - (x >> 16);   // probably should be x - (x > 0x7FFF) but that is slower
#endif
}

Gradient_Shader::Gradient_Shader(const SkColor colors[], const SkScalar pos[],
             int colorCount, SkShader::TileMode mode, SkUnitMapper* mapper) {
    SkASSERT(colorCount > 1);

    fCacheAlpha = 256;  // init to a value that paint.getAlpha() can't return

    fMapper = mapper;
    SkSafeRef(mapper);

    SkASSERT((unsigned)mode < SkShader::kTileModeCount);
    SkASSERT(SkShader::kTileModeCount == SK_ARRAY_COUNT(gTileProcs));
    fTileMode = mode;
    fTileProc = gTileProcs[mode];

    fCache16 = fCache16Storage = NULL;
    fCache32 = NULL;
    fCache32PixelRef = NULL;

    /*  Note: we let the caller skip the first and/or last position.
        i.e. pos[0] = 0.3, pos[1] = 0.7
        In these cases, we insert dummy entries to ensure that the final data
        will be bracketed by [0, 1].
        i.e. our_pos[0] = 0, our_pos[1] = 0.3, our_pos[2] = 0.7, our_pos[3] = 1

        Thus colorCount (the caller's value, and fColorCount (our value) may
        differ by up to 2. In the above example:
            colorCount = 2
            fColorCount = 4
     */
    fColorCount = colorCount;
    // check if we need to add in dummy start and/or end position/colors
    bool dummyFirst = false;
    bool dummyLast = false;
    if (pos) {
        dummyFirst = pos[0] != 0;
        dummyLast = pos[colorCount - 1] != SK_Scalar1;
        fColorCount += dummyFirst + dummyLast;
    }

    if (fColorCount > kColorStorageCount) {
        size_t size = sizeof(SkColor) + sizeof(Rec);
        fOrigColors = reinterpret_cast<SkColor*>(
                                        sk_malloc_throw(size * fColorCount));
    }
    else {
        fOrigColors = fStorage;
    }

    // Now copy over the colors, adding the dummies as needed
    {
        SkColor* origColors = fOrigColors;
        if (dummyFirst) {
            *origColors++ = colors[0];
        }
        memcpy(origColors, colors, colorCount * sizeof(SkColor));
        if (dummyLast) {
            origColors += colorCount;
            *origColors = colors[colorCount - 1];
        }
    }

    fRecs = (Rec*)(fOrigColors + fColorCount);
    if (fColorCount > 2) {
        Rec* recs = fRecs;
        recs->fPos = 0;
        //  recs->fScale = 0; // unused;
        recs += 1;
        if (pos) {
            /*  We need to convert the user's array of relative positions into
                fixed-point positions and scale factors. We need these results
                to be strictly monotonic (no two values equal or out of order).
                Hence this complex loop that just jams a zero for the scale
                value if it sees a segment out of order, and it assures that
                we start at 0 and end at 1.0
            */
            SkFixed prev = 0;
            int startIndex = dummyFirst ? 0 : 1;
            int count = colorCount + dummyLast;
            for (int i = startIndex; i < count; i++) {
                // force the last value to be 1.0
                SkFixed curr;
                if (i == colorCount) {  // we're really at the dummyLast
                    curr = SK_Fixed1;
                } else {
                    curr = SkScalarToFixed(pos[i]);
                }
                // pin curr withing range
                if (curr < 0) {
                    curr = 0;
                } else if (curr > SK_Fixed1) {
                    curr = SK_Fixed1;
                }
                recs->fPos = curr;
                if (curr > prev) {
                    recs->fScale = (1 << 24) / (curr - prev);
                } else {
                    recs->fScale = 0; // ignore this segment
                }
                // get ready for the next value
                prev = curr;
                recs += 1;
            }
        } else {    // assume even distribution
            SkFixed dp = SK_Fixed1 / (colorCount - 1);
            SkFixed p = dp;
            SkFixed scale = (colorCount - 1) << 8;  // (1 << 24) / dp
            for (int i = 1; i < colorCount; i++) {
                recs->fPos   = p;
                recs->fScale = scale;
                recs += 1;
                p += dp;
            }
        }
    }
    fFlags = 0;
}

Gradient_Shader::Gradient_Shader(SkFlattenableReadBuffer& buffer) :
    INHERITED(buffer) {
    fCacheAlpha = 256;

    fMapper = static_cast<SkUnitMapper*>(buffer.readFlattenable());

    fCache16 = fCache16Storage = NULL;
    fCache32 = NULL;
    fCache32PixelRef = NULL;

    int colorCount = fColorCount = buffer.readU32();
    if (colorCount > kColorStorageCount) {
        size_t size = sizeof(SkColor) + sizeof(SkPMColor) + sizeof(Rec);
        fOrigColors = (SkColor*)sk_malloc_throw(size * colorCount);
    } else {
        fOrigColors = fStorage;
    }
    buffer.read(fOrigColors, colorCount * sizeof(SkColor));

    fTileMode = (TileMode)buffer.readU8();
    fTileProc = gTileProcs[fTileMode];
    fRecs = (Rec*)(fOrigColors + colorCount);
    if (colorCount > 2) {
        Rec* recs = fRecs;
        recs[0].fPos = 0;
        for (int i = 1; i < colorCount; i++) {
            recs[i].fPos = buffer.readS32();
            recs[i].fScale = buffer.readU32();
        }
    }
    SkReadMatrix(&buffer, &fPtsToUnit);
    fFlags = 0;
}

Gradient_Shader::~Gradient_Shader() {
    if (fCache16Storage) {
        sk_free(fCache16Storage);
    }
    SkSafeUnref(fCache32PixelRef);
    if (fOrigColors != fStorage) {
        sk_free(fOrigColors);
    }
    SkSafeUnref(fMapper);
}

void Gradient_Shader::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fMapper);
    buffer.write32(fColorCount);
    buffer.writeMul4(fOrigColors, fColorCount * sizeof(SkColor));
    buffer.write8(fTileMode);
    if (fColorCount > 2) {
        Rec* recs = fRecs;
        for (int i = 1; i < fColorCount; i++) {
            buffer.write32(recs[i].fPos);
            buffer.write32(recs[i].fScale);
        }
    }
    SkWriteMatrix(&buffer, fPtsToUnit);
}

bool Gradient_Shader::setContext(const SkBitmap& device,
                                 const SkPaint& paint,
                                 const SkMatrix& matrix) {
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    const SkMatrix& inverse = this->getTotalInverse();

    if (!fDstToIndex.setConcat(fPtsToUnit, inverse)) {
        return false;
    }

    fDstToIndexProc = fDstToIndex.getMapXYProc();
    fDstToIndexClass = (uint8_t)SkShader::ComputeMatrixClass(fDstToIndex);

    // now convert our colors in to PMColors
    unsigned paintAlpha = this->getPaintAlpha();
    unsigned colorAlpha = 0xFF;

    // FIXME: record colorAlpha in constructor, since this is not affected
    // by setContext()
    for (int i = 0; i < fColorCount; i++) {
        SkColor src = fOrigColors[i];
        unsigned sa = SkColorGetA(src);
        colorAlpha &= sa;
    }

    fFlags = this->INHERITED::getFlags();
    if ((colorAlpha & paintAlpha) == 0xFF) {
        fFlags |= kOpaqueAlpha_Flag;
    }
    // we can do span16 as long as our individual colors are opaque,
    // regardless of the paint's alpha
    if (0xFF == colorAlpha) {
        fFlags |= kHasSpan16_Flag;
    }

    this->setCacheAlpha(paintAlpha);
    return true;
}

void Gradient_Shader::setCacheAlpha(U8CPU alpha) const {
    // if the new alpha differs from the previous time we were called, inval our cache
    // this will trigger the cache to be rebuilt.
    // we don't care about the first time, since the cache ptrs will already be NULL
    if (fCacheAlpha != alpha) {
        fCache16 = NULL;            // inval the cache
        fCache32 = NULL;            // inval the cache
        fCacheAlpha = alpha;        // record the new alpha
        // inform our subclasses
        if (fCache32PixelRef) {
            fCache32PixelRef->notifyPixelsChanged();
        }
    }
}

static inline int blend8(int a, int b, int scale) {
    SkASSERT(a == SkToU8(a));
    SkASSERT(b == SkToU8(b));
    SkASSERT(scale >= 0 && scale <= 256);
    return a + ((b - a) * scale >> 8);
}

static inline uint32_t dot8_blend_packed32(uint32_t s0, uint32_t s1,
                                           int blend) {
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

/** We take the original colors, not our premultiplied PMColors, since we can
    build a 16bit table as long as the original colors are opaque, even if the
    paint specifies a non-opaque alpha.
*/
void Gradient_Shader::Build16bitCache(uint16_t cache[], SkColor c0, SkColor c1,
                                      int count) {
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
        cache[kCache16Count] = SkDitherPack888ToRGB16(rr, gg, bb);
        cache += 1;
        r += dr;
        g += dg;
        b += db;
    } while (--count != 0);
}

/*
 *  2x2 dither a fixed-point color component (8.16) down to 8, matching the
 *  semantics of how we 2x2 dither 32->16
 */
static inline U8CPU dither_fixed_to_8(SkFixed n) {
    n >>= 8;
    return ((n << 1) - ((n >> 8 << 8) | (n >> 8))) >> 8;
}

/*
 *  For dithering with premultiply, we want to ceiling the alpha component,
 *  to ensure that it is always >= any color component.
 */
static inline U8CPU dither_ceil_fixed_to_8(SkFixed n) {
    n >>= 8;
    return ((n << 1) - (n | (n >> 8))) >> 8;
}

void Gradient_Shader::Build32bitCache(SkPMColor cache[], SkColor c0, SkColor c1,
                                      int count, U8CPU paintAlpha) {
    SkASSERT(count > 1);

    // need to apply paintAlpha to our two endpoints
    SkFixed a = SkMulDiv255Round(SkColorGetA(c0), paintAlpha);
    SkFixed da;
    {
        int tmp = SkMulDiv255Round(SkColorGetA(c1), paintAlpha);
        da = SkIntToFixed(tmp - a) / (count - 1);
    }

    SkFixed r = SkColorGetR(c0);
    SkFixed g = SkColorGetG(c0);
    SkFixed b = SkColorGetB(c0);
    SkFixed dr = SkIntToFixed(SkColorGetR(c1) - r) / (count - 1);
    SkFixed dg = SkIntToFixed(SkColorGetG(c1) - g) / (count - 1);
    SkFixed db = SkIntToFixed(SkColorGetB(c1) - b) / (count - 1);

    a = SkIntToFixed(a) + 0x8000;
    r = SkIntToFixed(r) + 0x8000;
    g = SkIntToFixed(g) + 0x8000;
    b = SkIntToFixed(b) + 0x8000;

    do {
        cache[0] = SkPremultiplyARGBInline(a >> 16, r >> 16, g >> 16, b >> 16);
        cache[kCache32Count] = SkPremultiplyARGBInline(dither_ceil_fixed_to_8(a),
                                                       dither_fixed_to_8(r),
                                                       dither_fixed_to_8(g),
                                                       dither_fixed_to_8(b));
        cache += 1;
        a += da;
        r += dr;
        g += dg;
        b += db;
    } while (--count != 0);
}

static inline int SkFixedToFFFF(SkFixed x) {
    SkASSERT((unsigned)x <= SK_Fixed1);
    return x - (x >> 16);
}

static inline U16CPU bitsTo16(unsigned x, const unsigned bits) {
    SkASSERT(x < (1U << bits));
    if (6 == bits) {
        return (x << 10) | (x << 4) | (x >> 2);
    }
    if (8 == bits) {
        return (x << 8) | x;
    }
    sk_throw();
    return 0;
}

const uint16_t* Gradient_Shader::getCache16() const {
    if (fCache16 == NULL) {
        // double the count for dither entries
        const int entryCount = kCache16Count * 2;
        const size_t allocSize = sizeof(uint16_t) * entryCount;

        if (fCache16Storage == NULL) { // set the storage and our working ptr
            fCache16Storage = (uint16_t*)sk_malloc_throw(allocSize);
        }
        fCache16 = fCache16Storage;
        if (fColorCount == 2) {
            Build16bitCache(fCache16, fOrigColors[0], fOrigColors[1], kCache16Count);
        } else {
            Rec* rec = fRecs;
            int prevIndex = 0;
            for (int i = 1; i < fColorCount; i++) {
                int nextIndex = SkFixedToFFFF(rec[i].fPos) >> kCache16Shift;
                SkASSERT(nextIndex < kCache16Count);

                if (nextIndex > prevIndex)
                    Build16bitCache(fCache16 + prevIndex, fOrigColors[i-1], fOrigColors[i], nextIndex - prevIndex + 1);
                prevIndex = nextIndex;
            }
            SkASSERT(prevIndex == kCache16Count - 1);
        }

        if (fMapper) {
            fCache16Storage = (uint16_t*)sk_malloc_throw(allocSize);
            uint16_t* linear = fCache16;         // just computed linear data
            uint16_t* mapped = fCache16Storage;  // storage for mapped data
            SkUnitMapper* map = fMapper;
            for (int i = 0; i < kCache16Count; i++) {
                int index = map->mapUnit16(bitsTo16(i, kCache16Bits)) >> kCache16Shift;
                mapped[i] = linear[index];
                mapped[i + kCache16Count] = linear[index + kCache16Count];
            }
            sk_free(fCache16);
            fCache16 = fCache16Storage;
        }
    }
    return fCache16;
}

const SkPMColor* Gradient_Shader::getCache32() const {
    if (fCache32 == NULL) {
        // double the count for dither entries
        const int entryCount = kCache32Count * 2;
        const size_t allocSize = sizeof(SkPMColor) * entryCount;

        if (NULL == fCache32PixelRef) {
            fCache32PixelRef = SkNEW_ARGS(SkMallocPixelRef,
                                          (NULL, allocSize, NULL));
        }
        fCache32 = (SkPMColor*)fCache32PixelRef->getAddr();
        if (fColorCount == 2) {
            Build32bitCache(fCache32, fOrigColors[0], fOrigColors[1],
                            kCache32Count, fCacheAlpha);
        } else {
            Rec* rec = fRecs;
            int prevIndex = 0;
            for (int i = 1; i < fColorCount; i++) {
                int nextIndex = SkFixedToFFFF(rec[i].fPos) >> (16 - kCache32Bits);
                SkASSERT(nextIndex < kCache32Count);

                if (nextIndex > prevIndex)
                    Build32bitCache(fCache32 + prevIndex, fOrigColors[i-1],
                                    fOrigColors[i],
                                    nextIndex - prevIndex + 1, fCacheAlpha);
                prevIndex = nextIndex;
            }
            SkASSERT(prevIndex == kCache32Count - 1);
        }

        if (fMapper) {
            SkMallocPixelRef* newPR = SkNEW_ARGS(SkMallocPixelRef,
                                                 (NULL, allocSize, NULL));
            SkPMColor* linear = fCache32;           // just computed linear data
            SkPMColor* mapped = (SkPMColor*)newPR->getAddr();    // storage for mapped data
            SkUnitMapper* map = fMapper;
            for (int i = 0; i < kCache32Count; i++) {
                int index = map->mapUnit16((i << 8) | i) >> 8;
                mapped[i] = linear[index];
                mapped[i + kCache32Count] = linear[index + kCache32Count];
            }
            fCache32PixelRef->unref();
            fCache32PixelRef = newPR;
            fCache32 = (SkPMColor*)newPR->getAddr();
        }
    }
    return fCache32;
}

/*
 *  Because our caller might rebuild the same (logically the same) gradient
 *  over and over, we'd like to return exactly the same "bitmap" if possible,
 *  allowing the client to utilize a cache of our bitmap (e.g. with a GPU).
 *  To do that, we maintain a private cache of built-bitmaps, based on our
 *  colors and positions. Note: we don't try to flatten the fMapper, so if one
 *  is present, we skip the cache for now.
 */
void Gradient_Shader::commonAsABitmap(SkBitmap* bitmap) const {
    // our caller assumes no external alpha, so we ensure that our cache is
    // built with 0xFF
    this->setCacheAlpha(0xFF);

    // don't have a way to put the mapper into our cache-key yet
    if (fMapper) {
        // force our cahce32pixelref to be built
        (void)this->getCache32();
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, kCache32Count, 1);
        bitmap->setPixelRef(fCache32PixelRef);
        return;
    }

    // build our key: [numColors + colors[] + {positions[]} ]
    int count = 1 + fColorCount;
    if (fColorCount > 2) {
        count += fColorCount - 1;    // fRecs[].fPos
    }

    SkAutoSTMalloc<16, int32_t> storage(count);
    int32_t* buffer = storage.get();

    *buffer++ = fColorCount;
    memcpy(buffer, fOrigColors, fColorCount * sizeof(SkColor));
    buffer += fColorCount;
    if (fColorCount > 2) {
        for (int i = 1; i < fColorCount; i++) {
            *buffer++ = fRecs[i].fPos;
        }
    }
    SkASSERT(buffer - storage.get() == count);

    ///////////////////////////////////

    static SkMutex gMutex;
    static SkBitmapCache* gCache;
    // each cache cost 1K of RAM, since each bitmap will be 1x256 at 32bpp
    static const int MAX_NUM_CACHED_GRADIENT_BITMAPS = 32;
    SkAutoMutexAcquire ama(gMutex);

    if (NULL == gCache) {
        gCache = new SkBitmapCache(MAX_NUM_CACHED_GRADIENT_BITMAPS);
    }
    size_t size = count * sizeof(int32_t);

    if (!gCache->find(storage.get(), size, bitmap)) {
        // force our cahce32pixelref to be built
        (void)this->getCache32();
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, kCache32Count, 1);
        bitmap->setPixelRef(fCache32PixelRef);

        gCache->add(storage.get(), size, *bitmap);
    }
}

void Gradient_Shader::commonAsAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColorCount >= fColorCount) {
            if (info->fColors) {
                memcpy(info->fColors, fOrigColors,
                       fColorCount * sizeof(SkColor));
            }
            if (info->fColorOffsets) {
                if (fColorCount == 2) {
                    info->fColorOffsets[0] = 0;
                    info->fColorOffsets[1] = SK_Scalar1;
                } else if (fColorCount > 2) {
                    for (int i = 0; i < fColorCount; i++)
                        info->fColorOffsets[i] = SkFixedToScalar(fRecs[i].fPos);
                }
            }
        }
        info->fColorCount = fColorCount;
        info->fTileMode = fTileMode;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void pts_to_unit_matrix(const SkPoint pts[2], SkMatrix* matrix) {
    SkVector    vec = pts[1] - pts[0];
    SkScalar    mag = vec.length();
    SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    matrix->setSinCos(-vec.fY, vec.fX, pts[0].fX, pts[0].fY);
    matrix->postTranslate(-pts[0].fX, -pts[0].fY);
    matrix->postScale(inv, inv);
}

///////////////////////////////////////////////////////////////////////////////

class Linear_Gradient : public Gradient_Shader {
public:
    Linear_Gradient(const SkPoint pts[2],
                    const SkColor colors[], const SkScalar pos[], int colorCount,
                    SkShader::TileMode mode, SkUnitMapper* mapper)
        : Gradient_Shader(colors, pos, colorCount, mode, mapper),
          fStart(pts[0]),
          fEnd(pts[1])
    {
        pts_to_unit_matrix(pts, &fPtsToUnit);
    }

    virtual bool setContext(const SkBitmap&, const SkPaint&, const SkMatrix&);
    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count);
    virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count);
    virtual BitmapType asABitmap(SkBitmap*, SkMatrix*,
                             TileMode*, SkScalar* twoPointRadialParams) const;
    virtual GradientType asAGradient(GradientInfo* info) const;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(Linear_Gradient, (buffer));
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.writeScalar(fStart.fX);
        buffer.writeScalar(fStart.fY);
        buffer.writeScalar(fEnd.fX);
        buffer.writeScalar(fEnd.fY);
    }

protected:
    Linear_Gradient(SkFlattenableReadBuffer& buffer)
        : Gradient_Shader(buffer),
          fStart(unflatten_point(buffer)),
          fEnd(unflatten_point(buffer)) {
    }
    virtual Factory getFactory() { return CreateProc; }

private:
    typedef Gradient_Shader INHERITED;
    const SkPoint fStart;
    const SkPoint fEnd;
};

bool Linear_Gradient::setContext(const SkBitmap& device, const SkPaint& paint,
                                 const SkMatrix& matrix) {
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    unsigned mask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;
    if ((fDstToIndex.getType() & ~mask) == 0) {
        fFlags |= SkShader::kConstInY32_Flag;
        if ((fFlags & SkShader::kHasSpan16_Flag) && !paint.isDither()) {
            // only claim this if we do have a 16bit mode (i.e. none of our
            // colors have alpha), and if we are not dithering (which obviously
            // is not const in Y).
            fFlags |= SkShader::kConstInY16_Flag;
        }
    }
    return true;
}

//  Return true if fx, fx+dx, fx+2*dx, ... is always in range
static inline bool no_need_for_clamp(int fx, int dx, int count) {
    SkASSERT(count > 0);
    return (unsigned)((fx | (fx + (count - 1) * dx)) >> 8) <= 0xFF;
}

#include "SkClampRange.h"

#define NO_CHECK_ITER               \
    do {                            \
    unsigned fi = fx >> 8;          \
    SkASSERT(fi <= 0xFF);           \
    fx += dx;                       \
    *dstC++ = cache[toggle + fi];   \
    toggle ^= TOGGLE_MASK;          \
    } while (0)


void Linear_Gradient::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    SkASSERT(count > 0);

    SkPoint             srcPt;
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = fTileProc;
    const SkPMColor*    cache = this->getCache32();
#ifdef USE_DITHER_32BIT_GRADIENT
    int                 toggle = ((x ^ y) & 1) << kCache32Bits;
    const int           TOGGLE_MASK = (1 << kCache32Bits);
#else
    int toggle = 0;
    const int TOGGLE_MASK = 0;
#endif

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkFixed dx, fx = SkScalarToFixed(srcPt.fX);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed dxStorage[1];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), dxStorage, NULL);
            dx = dxStorage[0];
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToFixed(fDstToIndex.getScaleX());
        }

        if (SkFixedNearlyZero(dx)) {
            // we're a vertical gradient, so no change in a span
            unsigned fi = proc(fx);
            SkASSERT(fi <= 0xFFFF);
            // TODO: dither version
            sk_memset32(dstC, cache[fi >> (16 - kCache32Bits)], count);
        } else if (proc == clamp_tileproc) {
            SkClampRange range;
            range.init(fx, dx, count, 0, 0xFF);

            if ((count = range.fCount0) > 0) {
                sk_memset32_dither(dstC,
                                   cache[toggle + range.fV0],
                                   cache[(toggle ^ TOGGLE_MASK) + range.fV0],
                                   count);
                dstC += count;
            }
            if ((count = range.fCount1) > 0) {
                int unroll = count >> 3;
                fx = range.fFx1;
                for (int i = 0; i < unroll; i++) {
                    NO_CHECK_ITER;  NO_CHECK_ITER;
                    NO_CHECK_ITER;  NO_CHECK_ITER;
                    NO_CHECK_ITER;  NO_CHECK_ITER;
                    NO_CHECK_ITER;  NO_CHECK_ITER;
                }
                if ((count &= 7) > 0) {
                    do {
                        NO_CHECK_ITER;
                    } while (--count != 0);
                }
            }
            if ((count = range.fCount2) > 0) {
                sk_memset32_dither(dstC,
                                   cache[toggle + range.fV1],
                                   cache[(toggle ^ TOGGLE_MASK) + range.fV1],
                                   count);
            }
        } else if (proc == mirror_tileproc) {
            do {
                unsigned fi = mirror_8bits(fx >> 8);
                SkASSERT(fi <= 0xFF);
                fx += dx;
                *dstC++ = cache[toggle + fi];
                toggle ^= TOGGLE_MASK;
            } while (--count != 0);
        } else {
            SkASSERT(proc == repeat_tileproc);
            do {
                unsigned fi = repeat_8bits(fx >> 8);
                SkASSERT(fi <= 0xFF);
                fx += dx;
                *dstC++ = cache[toggle + fi];
                toggle ^= TOGGLE_MASK;
            } while (--count != 0);
        }
    } else {
        SkScalar    dstX = SkIntToScalar(x);
        SkScalar    dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.fX));
            SkASSERT(fi <= 0xFFFF);
            *dstC++ = cache[toggle + (fi >> (16 - kCache32Bits))];
            toggle ^= TOGGLE_MASK;
            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

SkShader::BitmapType Linear_Gradient::asABitmap(SkBitmap* bitmap,
                                                SkMatrix* matrix,
                                                TileMode xy[],
                                        SkScalar* twoPointRadialParams) const {
    if (bitmap) {
        this->commonAsABitmap(bitmap);
    }
    if (matrix) {
        matrix->setScale(SkIntToScalar(kCache32Count), SK_Scalar1);
        matrix->preConcat(fPtsToUnit);
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kDefault_BitmapType;
}

SkShader::GradientType Linear_Gradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fStart;
        info->fPoint[1] = fEnd;
    }
    return kLinear_GradientType;
}

static void dither_memset16(uint16_t dst[], uint16_t value, uint16_t other,
                            int count) {
    if (reinterpret_cast<uintptr_t>(dst) & 2) {
        *dst++ = value;
        count -= 1;
        SkTSwap(value, other);
    }

    sk_memset32((uint32_t*)dst, (value << 16) | other, count >> 1);

    if (count & 1) {
        dst[count - 1] = value;
    }
}

#define NO_CHECK_ITER_16                \
    do {                                \
    unsigned fi = fx >> kCache16Shift;  \
    SkASSERT(fi <= kCache16Mask);       \
    fx += dx;                           \
    *dstC++ = cache[toggle + fi];       \
    toggle ^= TOGGLE_MASK;              \
    } while (0)


void Linear_Gradient::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    SkASSERT(count > 0);

    SkPoint             srcPt;
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = fTileProc;
    const uint16_t*     cache = this->getCache16();
    int                 toggle = ((x ^ y) & 1) << kCache16Bits;
    const int           TOGGLE_MASK = (1 << kCache32Bits);

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkFixed dx, fx = SkScalarToFixed(srcPt.fX);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed dxStorage[1];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), dxStorage, NULL);
            dx = dxStorage[0];
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToFixed(fDstToIndex.getScaleX());
        }

        if (SkFixedNearlyZero(dx)) {
            // we're a vertical gradient, so no change in a span
            unsigned fi = proc(fx) >> kCache16Shift;
            SkASSERT(fi <= kCache16Mask);
            dither_memset16(dstC, cache[toggle + fi],
                            cache[(toggle ^ TOGGLE_MASK) + fi], count);
        } else if (proc == clamp_tileproc) {
            SkClampRange range;
            range.init(fx, dx, count, 0, kCache16Mask);

            if ((count = range.fCount0) > 0) {
                dither_memset16(dstC,
                                cache[toggle + range.fV0],
                                cache[(toggle ^ TOGGLE_MASK) + range.fV0],
                                count);
                dstC += count;
            }
            if ((count = range.fCount1) > 0) {
                int unroll = count >> 3;
                fx = range.fFx1;
                for (int i = 0; i < unroll; i++) {
                    NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
                    NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
                    NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
                    NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
                }
                if ((count &= 7) > 0) {
                    do {
                        NO_CHECK_ITER_16;
                    } while (--count != 0);
                }
            }
            if ((count = range.fCount2) > 0) {
                dither_memset16(dstC,
                                cache[toggle + range.fV1],
                                cache[(toggle ^ TOGGLE_MASK) + range.fV1],
                                count);
            }
        } else if (proc == mirror_tileproc) {
            do {
                unsigned fi = mirror_bits(fx >> kCache16Shift, kCache16Bits);
                SkASSERT(fi <= kCache16Mask);
                fx += dx;
                *dstC++ = cache[toggle + fi];
                toggle ^= TOGGLE_MASK;
            } while (--count != 0);
        } else {
            SkASSERT(proc == repeat_tileproc);
            do {
                unsigned fi = repeat_bits(fx >> kCache16Shift, kCache16Bits);
                SkASSERT(fi <= kCache16Mask);
                fx += dx;
                *dstC++ = cache[toggle + fi];
                toggle ^= TOGGLE_MASK;
            } while (--count != 0);
        }
    } else {
        SkScalar    dstX = SkIntToScalar(x);
        SkScalar    dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.fX));
            SkASSERT(fi <= 0xFFFF);

            int index = fi >> kCache16Shift;
            *dstC++ = cache[toggle + index];
            toggle ^= TOGGLE_MASK;

            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

#define kSQRT_TABLE_BITS    11
#define kSQRT_TABLE_SIZE    (1 << kSQRT_TABLE_BITS)

#include "SkRadialGradient_Table.h"

#if defined(SK_BUILD_FOR_WIN32) && defined(SK_DEBUG)

#include <stdio.h>

void SkRadialGradient_BuildTable() {
    // build it 0..127 x 0..127, so we use 2^15 - 1 in the numerator for our "fixed" table

    FILE* file = ::fopen("SkRadialGradient_Table.h", "w");
    SkASSERT(file);
    ::fprintf(file, "static const uint8_t gSqrt8Table[] = {\n");

    for (int i = 0; i < kSQRT_TABLE_SIZE; i++) {
        if ((i & 15) == 0) {
            ::fprintf(file, "\t");
        }

        uint8_t value = SkToU8(SkFixedSqrt(i * SK_Fixed1 / kSQRT_TABLE_SIZE) >> 8);

        ::fprintf(file, "0x%02X", value);
        if (i < kSQRT_TABLE_SIZE-1) {
            ::fprintf(file, ", ");
        }
        if ((i & 15) == 15) {
            ::fprintf(file, "\n");
        }
    }
    ::fprintf(file, "};\n");
    ::fclose(file);
}

#endif


static void rad_to_unit_matrix(const SkPoint& center, SkScalar radius,
                               SkMatrix* matrix) {
    SkScalar    inv = SkScalarInvert(radius);

    matrix->setTranslate(-center.fX, -center.fY);
    matrix->postScale(inv, inv);
}

class Radial_Gradient : public Gradient_Shader {
public:
    Radial_Gradient(const SkPoint& center, SkScalar radius,
                    const SkColor colors[], const SkScalar pos[], int colorCount,
                    SkShader::TileMode mode, SkUnitMapper* mapper)
        : Gradient_Shader(colors, pos, colorCount, mode, mapper),
          fCenter(center),
          fRadius(radius)
    {
        // make sure our table is insync with our current #define for kSQRT_TABLE_SIZE
        SkASSERT(sizeof(gSqrt8Table) == kSQRT_TABLE_SIZE);

        rad_to_unit_matrix(center, radius, &fPtsToUnit);
    }

    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count) {
        SkASSERT(count > 0);

        SkPoint             srcPt;
        SkMatrix::MapXYProc dstProc = fDstToIndexProc;
        TileProc            proc = fTileProc;
        const SkPMColor*    cache = this->getCache32();

        if (fDstToIndexClass != kPerspective_MatrixClass) {
            dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                                 SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
            SkFixed dx, fx = SkScalarToFixed(srcPt.fX);
            SkFixed dy, fy = SkScalarToFixed(srcPt.fY);

            if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
                SkFixed storage[2];
                (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), &storage[0], &storage[1]);
                dx = storage[0];
                dy = storage[1];
            } else {
                SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
                dx = SkScalarToFixed(fDstToIndex.getScaleX());
                dy = SkScalarToFixed(fDstToIndex.getSkewY());
            }

            if (proc == clamp_tileproc) {
                const uint8_t* sqrt_table = gSqrt8Table;
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
            } else if (proc == mirror_tileproc) {
                do {
                    SkFixed magnitudeSquared = SkFixedSquare(fx) + SkFixedSquare(fy);
                    if (magnitudeSquared < 0) // Overflow.
                        magnitudeSquared = SK_FixedMax;
                    SkFixed dist = SkFixedSqrt(magnitudeSquared);
                    unsigned fi = mirror_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    *dstC++ = cache[fi >> (16 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                } while (--count != 0);
            } else {
                SkASSERT(proc == repeat_tileproc);
                do {
                    SkFixed magnitudeSquared = SkFixedSquare(fx) + SkFixedSquare(fy);
                    if (magnitudeSquared < 0) // Overflow.
                        magnitudeSquared = SK_FixedMax;
                    SkFixed dist = SkFixedSqrt(magnitudeSquared);
                    unsigned fi = repeat_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    *dstC++ = cache[fi >> (16 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                } while (--count != 0);
            }
        } else {    // perspective case
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

    virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count) {
        SkASSERT(count > 0);

        SkPoint             srcPt;
        SkMatrix::MapXYProc dstProc = fDstToIndexProc;
        TileProc            proc = fTileProc;
        const uint16_t*     cache = this->getCache16();
        int                 toggle = ((x ^ y) & 1) << kCache16Bits;

        if (fDstToIndexClass != kPerspective_MatrixClass) {
            dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                                 SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
            SkFixed dx, fx = SkScalarToFixed(srcPt.fX);
            SkFixed dy, fy = SkScalarToFixed(srcPt.fY);

            if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
                SkFixed storage[2];
                (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), &storage[0], &storage[1]);
                dx = storage[0];
                dy = storage[1];
            } else {
                SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
                dx = SkScalarToFixed(fDstToIndex.getScaleX());
                dy = SkScalarToFixed(fDstToIndex.getSkewY());
            }

            if (proc == clamp_tileproc) {
                const uint8_t* sqrt_table = gSqrt8Table;

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
                if (dy == 0) {    // might perform this check for the other modes, but the win will be a smaller % of the total
                    fy = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
                    fy *= fy;
                    do {
                        unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
                        unsigned fi = (xx * xx + fy) >> (14 + 16 - kSQRT_TABLE_BITS);
                        fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
                        fx += dx;
                        *dstC++ = cache[toggle + (sqrt_table[fi] >> (8 - kCache16Bits))];
                        toggle ^= (1 << kCache16Bits);
                    } while (--count != 0);
                } else {
                    do {
                        unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
                        unsigned fi = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
                        fi = (xx * xx + fi * fi) >> (14 + 16 - kSQRT_TABLE_BITS);
                        fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
                        fx += dx;
                        fy += dy;
                        *dstC++ = cache[toggle + (sqrt_table[fi] >> (8 - kCache16Bits))];
                        toggle ^= (1 << kCache16Bits);
                    } while (--count != 0);
                }
            } else if (proc == mirror_tileproc) {
                do {
                    SkFixed dist = SkFixedSqrt(SkFixedSquare(fx) + SkFixedSquare(fy));
                    unsigned fi = mirror_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    fx += dx;
                    fy += dy;
                    *dstC++ = cache[toggle + (fi >> (16 - kCache16Bits))];
                    toggle ^= (1 << kCache16Bits);
                } while (--count != 0);
            } else {
                SkASSERT(proc == repeat_tileproc);
                do {
                    SkFixed dist = SkFixedSqrt(SkFixedSquare(fx) + SkFixedSquare(fy));
                    unsigned fi = repeat_tileproc(dist);
                    SkASSERT(fi <= 0xFFFF);
                    fx += dx;
                    fy += dy;
                    *dstC++ = cache[toggle + (fi >> (16 - kCache16Bits))];
                    toggle ^= (1 << kCache16Bits);
                } while (--count != 0);
            }
        } else {    // perspective case
            SkScalar dstX = SkIntToScalar(x);
            SkScalar dstY = SkIntToScalar(y);
            do {
                dstProc(fDstToIndex, dstX, dstY, &srcPt);
                unsigned fi = proc(SkScalarToFixed(srcPt.length()));
                SkASSERT(fi <= 0xFFFF);

                int index = fi >> (16 - kCache16Bits);
                *dstC++ = cache[toggle + index];
                toggle ^= (1 << kCache16Bits);

                dstX += SK_Scalar1;
            } while (--count != 0);
        }
    }

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy,
                                 SkScalar* twoPointRadialParams) const {
        if (bitmap) {
            this->commonAsABitmap(bitmap);
        }
        if (matrix) {
            matrix->setScale(SkIntToScalar(kCache32Count), SkIntToScalar(kCache32Count));
            matrix->preConcat(fPtsToUnit);
        }
        if (xy) {
            xy[0] = fTileMode;
            xy[1] = kClamp_TileMode;
        }
        return kRadial_BitmapType;
    }
    virtual GradientType asAGradient(GradientInfo* info) const {
        if (info) {
            commonAsAGradient(info);
            info->fPoint[0] = fCenter;
            info->fRadius[0] = fRadius;
        }
        return kRadial_GradientType;
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(Radial_Gradient, (buffer));
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.writeScalar(fCenter.fX);
        buffer.writeScalar(fCenter.fY);
        buffer.writeScalar(fRadius);
    }

protected:
    Radial_Gradient(SkFlattenableReadBuffer& buffer)
        : Gradient_Shader(buffer),
          fCenter(unflatten_point(buffer)),
          fRadius(buffer.readScalar()) {
    }
    virtual Factory getFactory() { return CreateProc; }

private:
    typedef Gradient_Shader INHERITED;
    const SkPoint fCenter;
    const SkScalar fRadius;
};

/* Two-point radial gradients are specified by two circles, each with a center
   point and radius.  The gradient can be considered to be a series of
   concentric circles, with the color interpolated from the start circle
   (at t=0) to the end circle (at t=1).

   For each point (x, y) in the span, we want to find the
   interpolated circle that intersects that point.  The center
   of the desired circle (Cx, Cy) falls at some distance t
   along the line segment between the start point (Sx, Sy) and
   end point (Ex, Ey):

      Cx = (1 - t) * Sx + t * Ex        (0 <= t <= 1)
      Cy = (1 - t) * Sy + t * Ey

   The radius of the desired circle (r) is also a linear interpolation t
   between the start and end radii (Sr and Er):

      r = (1 - t) * Sr + t * Er

   But

      (x - Cx)^2 + (y - Cy)^2 = r^2

   so

     (x - ((1 - t) * Sx + t * Ex))^2
   + (y - ((1 - t) * Sy + t * Ey))^2
   = ((1 - t) * Sr + t * Er)^2

   Solving for t yields

     [(Sx - Ex)^2 + (Sy - Ey)^2 - (Er - Sr)^2)] * t^2
   + [2 * (Sx - Ex)(x - Sx) + 2 * (Sy - Ey)(y - Sy) - 2 * (Er - Sr) * Sr] * t
   + [(x - Sx)^2 + (y - Sy)^2 - Sr^2] = 0

   To simplify, let Dx = Sx - Ex, Dy = Sy - Ey, Dr = Er - Sr, dx = x - Sx, dy = y - Sy

     [Dx^2 + Dy^2 - Dr^2)] * t^2
   + 2 * [Dx * dx + Dy * dy - Dr * Sr] * t
   + [dx^2 + dy^2 - Sr^2] = 0

   A quadratic in t.  The two roots of the quadratic reflect the two
   possible circles on which the point may fall.  Solving for t yields
   the gradient value to use.

   If a<0, the start circle is entirely contained in the
   end circle, and one of the roots will be <0 or >1 (off the line
   segment).  If a>0, the start circle falls at least partially
   outside the end circle (or vice versa), and the gradient
   defines a "tube" where a point may be on one circle (on the
   inside of the tube) or the other (outside of the tube).  We choose
   one arbitrarily.

   In order to keep the math to within the limits of fixed point,
   we divide the entire quadratic by Dr^2, and replace
   (x - Sx)/Dr with x' and (y - Sy)/Dr with y', giving

   [Dx^2 / Dr^2 + Dy^2 / Dr^2 - 1)] * t^2
   + 2 * [x' * Dx / Dr + y' * Dy / Dr - Sr / Dr] * t
   + [x'^2 + y'^2 - Sr^2/Dr^2] = 0

   (x' and y' are computed by appending the subtract and scale to the
   fDstToIndex matrix in the constructor).

   Since the 'A' component of the quadratic is independent of x' and y', it
   is precomputed in the constructor.  Since the 'B' component is linear in
   x' and y', if x and y are linear in the span, 'B' can be computed
   incrementally with a simple delta (db below).  If it is not (e.g.,
   a perspective projection), it must be computed in the loop.

*/

static inline SkFixed two_point_radial(SkScalar b, SkScalar fx, SkScalar fy,
                                       SkScalar sr2d2, SkScalar foura,
                                       SkScalar oneOverTwoA, bool posRoot) {
    SkScalar c = SkScalarSquare(fx) + SkScalarSquare(fy) - sr2d2;
    if (0 == foura) {
        return SkScalarToFixed(SkScalarDiv(-c, b));
    }

    SkScalar discrim = SkScalarSquare(b) - SkScalarMul(foura, c);
    if (discrim < 0) {
        discrim = -discrim;
    }
    SkScalar rootDiscrim = SkScalarSqrt(discrim);
    SkScalar result;
    if (posRoot) {
        result = SkScalarMul(-b + rootDiscrim, oneOverTwoA);
    } else {
        result = SkScalarMul(-b - rootDiscrim, oneOverTwoA);
    }
    return SkScalarToFixed(result);
}

class Two_Point_Radial_Gradient : public Gradient_Shader {
public:
    Two_Point_Radial_Gradient(const SkPoint& start, SkScalar startRadius,
                              const SkPoint& end, SkScalar endRadius,
                              const SkColor colors[], const SkScalar pos[],
                              int colorCount, SkShader::TileMode mode,
                              SkUnitMapper* mapper)
            : Gradient_Shader(colors, pos, colorCount, mode, mapper),
              fCenter1(start),
              fCenter2(end),
              fRadius1(startRadius),
              fRadius2(endRadius) {
        init();
    }

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy,
                                 SkScalar* twoPointRadialParams) const {
        if (bitmap) {
            this->commonAsABitmap(bitmap);
        }
        SkScalar diffL = 0; // just to avoid gcc warning
        if (matrix || twoPointRadialParams) {
            diffL = SkScalarSqrt(SkScalarSquare(fDiff.fX) +
                                 SkScalarSquare(fDiff.fY));
        }
        if (matrix) {
            if (diffL) {
                SkScalar invDiffL = SkScalarInvert(diffL);
                matrix->setSinCos(-SkScalarMul(invDiffL, fDiff.fY),
                                  SkScalarMul(invDiffL, fDiff.fX));
            } else {
                matrix->reset();
            }
            matrix->preConcat(fPtsToUnit);
        }
        if (xy) {
            xy[0] = fTileMode;
            xy[1] = kClamp_TileMode;
        }
        if (NULL != twoPointRadialParams) {
            twoPointRadialParams[0] = diffL;
            twoPointRadialParams[1] = fStartRadius;
            twoPointRadialParams[2] = fDiffRadius;
        }
        return kTwoPointRadial_BitmapType;
    }

    virtual GradientType asAGradient(GradientInfo* info) const {
        if (info) {
            commonAsAGradient(info);
            info->fPoint[0] = fCenter1;
            info->fPoint[1] = fCenter2;
            info->fRadius[0] = fRadius1;
            info->fRadius[1] = fRadius2;
        }
        return kRadial2_GradientType;
    }

    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count) {
        SkASSERT(count > 0);

        // Zero difference between radii:  fill with transparent black.
        if (fDiffRadius == 0) {
          sk_bzero(dstC, count * sizeof(*dstC));
          return;
        }
        SkMatrix::MapXYProc dstProc = fDstToIndexProc;
        TileProc            proc = fTileProc;
        const SkPMColor*    cache = this->getCache32();

        SkScalar foura = fA * 4;
        bool posRoot = fDiffRadius < 0;
        if (fDstToIndexClass != kPerspective_MatrixClass) {
            SkPoint srcPt;
            dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                                 SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
            SkScalar dx, fx = srcPt.fX;
            SkScalar dy, fy = srcPt.fY;

            if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
                SkFixed fixedX, fixedY;
                (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), &fixedX, &fixedY);
                dx = SkFixedToScalar(fixedX);
                dy = SkFixedToScalar(fixedY);
            } else {
                SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
                dx = fDstToIndex.getScaleX();
                dy = fDstToIndex.getSkewY();
            }
            SkScalar b = (SkScalarMul(fDiff.fX, fx) +
                         SkScalarMul(fDiff.fY, fy) - fStartRadius) * 2;
            SkScalar db = (SkScalarMul(fDiff.fX, dx) +
                          SkScalarMul(fDiff.fY, dy)) * 2;
            if (proc == clamp_tileproc) {
                for (; count > 0; --count) {
                    SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura, fOneOverTwoA, posRoot);
                    SkFixed index = SkClampMax(t, 0xFFFF);
                    SkASSERT(index <= 0xFFFF);
                    *dstC++ = cache[index >> (16 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                    b += db;
                }
            } else if (proc == mirror_tileproc) {
                for (; count > 0; --count) {
                    SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura, fOneOverTwoA, posRoot);
                    SkFixed index = mirror_tileproc(t);
                    SkASSERT(index <= 0xFFFF);
                    *dstC++ = cache[index >> (16 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                    b += db;
                }
            } else {
                SkASSERT(proc == repeat_tileproc);
                for (; count > 0; --count) {
                    SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura, fOneOverTwoA, posRoot);
                    SkFixed index = repeat_tileproc(t);
                    SkASSERT(index <= 0xFFFF);
                    *dstC++ = cache[index >> (16 - kCache32Bits)];
                    fx += dx;
                    fy += dy;
                    b += db;
                }
            }
        } else {    // perspective case
            SkScalar dstX = SkIntToScalar(x);
            SkScalar dstY = SkIntToScalar(y);
            for (; count > 0; --count) {
                SkPoint             srcPt;
                dstProc(fDstToIndex, dstX, dstY, &srcPt);
                SkScalar fx = srcPt.fX;
                SkScalar fy = srcPt.fY;
                SkScalar b = (SkScalarMul(fDiff.fX, fx) +
                             SkScalarMul(fDiff.fY, fy) - fStartRadius) * 2;
                SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura, fOneOverTwoA, posRoot);
                SkFixed index = proc(t);
                SkASSERT(index <= 0xFFFF);
                *dstC++ = cache[index >> (16 - kCache32Bits)];
                dstX += SK_Scalar1;
            }
        }
    }

    virtual bool setContext(const SkBitmap& device,
                            const SkPaint& paint,
                            const SkMatrix& matrix) {
        if (!this->INHERITED::setContext(device, paint, matrix)) {
            return false;
        }

        // we don't have a span16 proc
        fFlags &= ~kHasSpan16_Flag;
        return true;
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(Two_Point_Radial_Gradient, (buffer));
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.writeScalar(fCenter1.fX);
        buffer.writeScalar(fCenter1.fY);
        buffer.writeScalar(fCenter2.fX);
        buffer.writeScalar(fCenter2.fY);
        buffer.writeScalar(fRadius1);
        buffer.writeScalar(fRadius2);
    }

protected:
    Two_Point_Radial_Gradient(SkFlattenableReadBuffer& buffer)
            : Gradient_Shader(buffer),
              fCenter1(unflatten_point(buffer)),
              fCenter2(unflatten_point(buffer)),
              fRadius1(buffer.readScalar()),
              fRadius2(buffer.readScalar()) {
        init();
    };
    virtual Factory getFactory() { return CreateProc; }

private:
    typedef Gradient_Shader INHERITED;
    const SkPoint fCenter1;
    const SkPoint fCenter2;
    const SkScalar fRadius1;
    const SkScalar fRadius2;
    SkPoint fDiff;
    SkScalar fStartRadius, fDiffRadius, fSr2D2, fA, fOneOverTwoA;

    void init() {
        fDiff = fCenter1 - fCenter2;
        fDiffRadius = fRadius2 - fRadius1;
        SkScalar inv = SkScalarInvert(fDiffRadius);
        fDiff.fX = SkScalarMul(fDiff.fX, inv);
        fDiff.fY = SkScalarMul(fDiff.fY, inv);
        fStartRadius = SkScalarMul(fRadius1, inv);
        fSr2D2 = SkScalarSquare(fStartRadius);
        fA = SkScalarSquare(fDiff.fX) + SkScalarSquare(fDiff.fY) - SK_Scalar1;
        fOneOverTwoA = fA ? SkScalarInvert(fA * 2) : 0;

        fPtsToUnit.setTranslate(-fCenter1.fX, -fCenter1.fY);
        fPtsToUnit.postScale(inv, inv);
    }
};

///////////////////////////////////////////////////////////////////////////////

class Sweep_Gradient : public Gradient_Shader {
public:
    Sweep_Gradient(SkScalar cx, SkScalar cy, const SkColor colors[],
                   const SkScalar pos[], int count, SkUnitMapper* mapper)
    : Gradient_Shader(colors, pos, count, SkShader::kClamp_TileMode, mapper),
      fCenter(SkPoint::Make(cx, cy))
    {
        fPtsToUnit.setTranslate(-cx, -cy);
    }
    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count);
    virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count);

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy,
                                 SkScalar* twoPointRadialParams) const {
        if (bitmap) {
            this->commonAsABitmap(bitmap);
        }
        if (matrix) {
            *matrix = fPtsToUnit;
        }
        if (xy) {
            xy[0] = fTileMode;
            xy[1] = kClamp_TileMode;
        }
        return kSweep_BitmapType;
    }

    virtual GradientType asAGradient(GradientInfo* info) const {
        if (info) {
            commonAsAGradient(info);
            info->fPoint[0] = fCenter;
        }
        return kSweep_GradientType;
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(Sweep_Gradient, (buffer));
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.writeScalar(fCenter.fX);
        buffer.writeScalar(fCenter.fY);
    }

protected:
    Sweep_Gradient(SkFlattenableReadBuffer& buffer)
        : Gradient_Shader(buffer),
          fCenter(unflatten_point(buffer)) {
    }

    virtual Factory getFactory() { return CreateProc; }

private:
    typedef Gradient_Shader INHERITED;
    const SkPoint fCenter;
};

#ifdef COMPUTE_SWEEP_TABLE
#define PI  3.14159265
static bool gSweepTableReady;
static uint8_t gSweepTable[65];

/*  Our table stores precomputed values for atan: [0...1] -> [0..PI/4]
    We scale the results to [0..32]
*/
static const uint8_t* build_sweep_table() {
    if (!gSweepTableReady) {
        const int N = 65;
        const double DENOM = N - 1;

        for (int i = 0; i < N; i++)
        {
            double arg = i / DENOM;
            double v = atan(arg);
            int iv = (int)round(v * DENOM * 2 / PI);
//            printf("[%d] atan(%g) = %g %d\n", i, arg, v, iv);
            printf("%d, ", iv);
            gSweepTable[i] = iv;
        }
        gSweepTableReady = true;
    }
    return gSweepTable;
}
#else
static const uint8_t gSweepTable[] = {
    0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9,
    10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 16, 16, 17, 17, 18, 18,
    19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26,
    26, 27, 27, 27, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32,
    32
};
static const uint8_t* build_sweep_table() { return gSweepTable; }
#endif

// divide numer/denom, with a bias of 6bits. Assumes numer <= denom
// and denom != 0. Since our table is 6bits big (+1), this is a nice fit.
// Same as (but faster than) SkFixedDiv(numer, denom) >> 10

//unsigned div_64(int numer, int denom);
static unsigned div_64(int numer, int denom) {
    SkASSERT(numer <= denom);
    SkASSERT(numer > 0);
    SkASSERT(denom > 0);

    int nbits = SkCLZ(numer);
    int dbits = SkCLZ(denom);
    int bits = 6 - nbits + dbits;
    SkASSERT(bits <= 6);

    if (bits < 0) {  // detect underflow
        return 0;
    }

    denom <<= dbits - 1;
    numer <<= nbits - 1;

    unsigned result = 0;

    // do the first one
    if ((numer -= denom) >= 0) {
        result = 1;
    } else {
        numer += denom;
    }

    // Now fall into our switch statement if there are more bits to compute
    if (bits > 0) {
        // make room for the rest of the answer bits
        result <<= bits;
        switch (bits) {
        case 6:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 32;
            else
                numer += denom;
        case 5:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 16;
            else
                numer += denom;
        case 4:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 8;
            else
                numer += denom;
        case 3:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 4;
            else
                numer += denom;
        case 2:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 2;
            else
                numer += denom;
        case 1:
        default:    // not strictly need, but makes GCC make better ARM code
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 1;
            else
                numer += denom;
        }
    }
    return result;
}

// Given x,y in the first quadrant, return 0..63 for the angle [0..90]
static unsigned atan_0_90(SkFixed y, SkFixed x) {
#ifdef SK_DEBUG
    {
        static bool gOnce;
        if (!gOnce) {
            gOnce = true;
            SkASSERT(div_64(55, 55) == 64);
            SkASSERT(div_64(128, 256) == 32);
            SkASSERT(div_64(2326528, 4685824) == 31);
            SkASSERT(div_64(753664, 5210112) == 9);
            SkASSERT(div_64(229376, 4882432) == 3);
            SkASSERT(div_64(2, 64) == 2);
            SkASSERT(div_64(1, 64) == 1);
            // test that we handle underflow correctly
            SkASSERT(div_64(12345, 0x54321234) == 0);
        }
    }
#endif

    SkASSERT(y > 0 && x > 0);
    const uint8_t* table = build_sweep_table();

    unsigned result;
    bool swap = (x < y);
    if (swap) {
        // first part of the atan(v) = PI/2 - atan(1/v) identity
        // since our div_64 and table want v <= 1, where v = y/x
        SkTSwap<SkFixed>(x, y);
    }

    result = div_64(y, x);

#ifdef SK_DEBUG
    {
        unsigned result2 = SkDivBits(y, x, 6);
        SkASSERT(result2 == result ||
                 (result == 1 && result2 == 0));
    }
#endif

    SkASSERT(result < SK_ARRAY_COUNT(gSweepTable));
    result = table[result];

    if (swap) {
        // complete the atan(v) = PI/2 - atan(1/v) identity
        result = 64 - result;
        // pin to 63
        result -= result >> 6;
    }

    SkASSERT(result <= 63);
    return result;
}

//  returns angle in a circle [0..2PI) -> [0..255]
static unsigned SkATan2_255(SkFixed y, SkFixed x) {
    if (x == 0) {
        if (y == 0) {
            return 0;
        }
        return y < 0 ? 192 : 64;
    }
    if (y == 0) {
        return x < 0 ? 128 : 0;
    }

    /*  Find the right quadrant for x,y
        Since atan_0_90 only handles the first quadrant, we rotate x,y
        appropriately before calling it, and then add the right amount
        to account for the real quadrant.
        quadrant 0 : add 0                  | x > 0 && y > 0
        quadrant 1 : add 64 (90 degrees)    | x < 0 && y > 0
        quadrant 2 : add 128 (180 degrees)  | x < 0 && y < 0
        quadrant 3 : add 192 (270 degrees)  | x > 0 && y < 0

        map x<0 to (1 << 6)
        map y<0 to (3 << 6)
        add = map_x ^ map_y
    */
    int xsign = x >> 31;
    int ysign = y >> 31;
    int add = ((-xsign) ^ (ysign & 3)) << 6;

#ifdef SK_DEBUG
    if (0 == add)
        SkASSERT(x > 0 && y > 0);
    else if (64 == add)
        SkASSERT(x < 0 && y > 0);
    else if (128 == add)
        SkASSERT(x < 0 && y < 0);
    else if (192 == add)
        SkASSERT(x > 0 && y < 0);
    else
        SkASSERT(!"bad value for add");
#endif

    /*  This ^ trick makes x, y positive, and the swap<> handles quadrants
        where we need to rotate x,y by 90 or -90
    */
    x = (x ^ xsign) - xsign;
    y = (y ^ ysign) - ysign;
    if (add & 64) {             // quads 1 or 3 need to swap x,y
        SkTSwap<SkFixed>(x, y);
    }

    unsigned result = add + atan_0_90(y, x);
    SkASSERT(result < 256);
    return result;
}

void Sweep_Gradient::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const SkPMColor*    cache = this->getCache32();
    SkPoint             srcPt;

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkFixed dx, fx = SkScalarToFixed(srcPt.fX);
        SkFixed dy, fy = SkScalarToFixed(srcPt.fY);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)matrix.fixedStepInX(SkIntToScalar(y) + SK_ScalarHalf,
                                      &storage[0], &storage[1]);
            dx = storage[0];
            dy = storage[1];
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToFixed(matrix.getScaleX());
            dy = SkScalarToFixed(matrix.getSkewY());
        }

        for (; count > 0; --count) {
            *dstC++ = cache[SkATan2_255(fy, fx)];
            fx += dx;
            fy += dy;
        }
    } else {  // perspective case
        for (int stop = x + count; x < stop; x++) {
            proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                 SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

            int index = SkATan2_255(SkScalarToFixed(srcPt.fY),
                                    SkScalarToFixed(srcPt.fX));
            *dstC++ = cache[index];
        }
    }
}

void Sweep_Gradient::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const uint16_t*     cache = this->getCache16();
    int                 toggle = ((x ^ y) & 1) << kCache16Bits;
    SkPoint             srcPt;

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkFixed dx, fx = SkScalarToFixed(srcPt.fX);
        SkFixed dy, fy = SkScalarToFixed(srcPt.fY);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)matrix.fixedStepInX(SkIntToScalar(y) + SK_ScalarHalf,
                                      &storage[0], &storage[1]);
            dx = storage[0];
            dy = storage[1];
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToFixed(matrix.getScaleX());
            dy = SkScalarToFixed(matrix.getSkewY());
        }

        for (; count > 0; --count) {
            int index = SkATan2_255(fy, fx) >> (8 - kCache16Bits);
            *dstC++ = cache[toggle + index];
            toggle ^= (1 << kCache16Bits);
            fx += dx;
            fy += dy;
        }
    } else {  // perspective case
        for (int stop = x + count; x < stop; x++) {
            proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                         SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

            int index = SkATan2_255(SkScalarToFixed(srcPt.fY),
                                    SkScalarToFixed(srcPt.fX));
            index >>= (8 - kCache16Bits);
            *dstC++ = cache[toggle + index];
            toggle ^= (1 << kCache16Bits);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// assumes colors is SkColor* and pos is SkScalar*
#define EXPAND_1_COLOR(count)               \
    SkColor tmp[2];                         \
    do {                                    \
        if (1 == count) {                   \
            tmp[0] = tmp[1] = colors[0];    \
            colors = tmp;                   \
            pos = NULL;                     \
            count = 2;                      \
        }                                   \
    } while (0)

SkShader* SkGradientShader::CreateLinear(const SkPoint pts[2],
                                         const SkColor colors[],
                                         const SkScalar pos[], int colorCount,
                                         SkShader::TileMode mode,
                                         SkUnitMapper* mapper) {
    if (NULL == pts || NULL == colors || colorCount < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(colorCount);

    return SkNEW_ARGS(Linear_Gradient,
                      (pts, colors, pos, colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateRadial(const SkPoint& center, SkScalar radius,
                                         const SkColor colors[],
                                         const SkScalar pos[], int colorCount,
                                         SkShader::TileMode mode,
                                         SkUnitMapper* mapper) {
    if (radius <= 0 || NULL == colors || colorCount < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(colorCount);

    return SkNEW_ARGS(Radial_Gradient,
                      (center, radius, colors, pos, colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateTwoPointRadial(const SkPoint& start,
                                                 SkScalar startRadius,
                                                 const SkPoint& end,
                                                 SkScalar endRadius,
                                                 const SkColor colors[],
                                                 const SkScalar pos[],
                                                 int colorCount,
                                                 SkShader::TileMode mode,
                                                 SkUnitMapper* mapper) {
    if (startRadius < 0 || endRadius < 0 || NULL == colors || colorCount < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(colorCount);

    return SkNEW_ARGS(Two_Point_Radial_Gradient,
                      (start, startRadius, end, endRadius, colors, pos,
                       colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateSweep(SkScalar cx, SkScalar cy,
                                        const SkColor colors[],
                                        const SkScalar pos[],
                                        int count, SkUnitMapper* mapper) {
    if (NULL == colors || count < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(count);

    return SkNEW_ARGS(Sweep_Gradient, (cx, cy, colors, pos, count, mapper));
}

static SkFlattenable::Registrar gLinearGradientReg("Linear_Gradient",
                                                   Linear_Gradient::CreateProc);

static SkFlattenable::Registrar gRadialGradientReg("Radial_Gradient",
                                                   Radial_Gradient::CreateProc);

static SkFlattenable::Registrar gSweepGradientReg("Sweep_Gradient",
                                                   Sweep_Gradient::CreateProc);

static SkFlattenable::Registrar
    gTwoPointRadialGradientReg("Two_Point_Radial_Gradient",
                               Two_Point_Radial_Gradient::CreateProc);
