
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmapSampler.h"

static SkTileModeProc get_tilemode_proc(SkShader::TileMode mode)
{
    switch (mode) {
    case SkShader::kClamp_TileMode:
        return do_clamp;
    case SkShader::kRepeat_TileMode:
        return do_repeat_mod;
    case SkShader::kMirror_TileMode:
        return do_mirror_mod;
    default:
        SkDEBUGFAIL("unknown mode");
        return NULL;
    }
}

SkBitmapSampler::SkBitmapSampler(const SkBitmap& bm, bool filter,
                                 SkShader::TileMode tmx, SkShader::TileMode tmy)
    : fBitmap(bm), fFilterBitmap(filter), fTileModeX(tmx), fTileModeY(tmy)
{
    SkASSERT(bm.width() > 0 && bm.height() > 0);

    fMaxX = SkToU16(bm.width() - 1);
    fMaxY = SkToU16(bm.height() - 1);

    fTileProcX = get_tilemode_proc(tmx);
    fTileProcY = get_tilemode_proc(tmy);
}

void SkBitmapSampler::setPaint(const SkPaint& paint)
{
}

class SkNullBitmapSampler : public SkBitmapSampler {
public:
    SkNullBitmapSampler(const SkBitmap& bm, bool filter,
                        SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, filter, tmx, tmy) {}

    virtual SkPMColor sample(SkFixed x, SkFixed y) const { return 0; }
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#define BITMAP_CLASSNAME_PREFIX(name)           ARGB32##name
#define BITMAP_PIXEL_TO_PMCOLOR(bitmap, x, y)   *bitmap.getAddr32(x, y)
#include "SkBitmapSamplerTemplate.h"

#include "SkColorPriv.h"

#define BITMAP_CLASSNAME_PREFIX(name)           RGB16##name
#define BITMAP_PIXEL_TO_PMCOLOR(bitmap, x, y)   SkPixel16ToPixel32(*bitmap.getAddr16(x, y))
#include "SkBitmapSamplerTemplate.h"

#define BITMAP_CLASSNAME_PREFIX(name)           Index8##name
#define BITMAP_PIXEL_TO_PMCOLOR(bitmap, x, y)   bitmap.getIndex8Color(x, y)
#include "SkBitmapSamplerTemplate.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///////////////// The Bilinear versions

#include "SkFilterProc.h"

class ARGB32_Bilinear_Sampler : public SkBitmapSampler {
public:
    ARGB32_Bilinear_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, true, tmx, tmy)
    {
        fPtrProcTable = SkGetBilinearFilterPtrProcTable();
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        const uint32_t *p00, *p01, *p10, *p11;

        // turn pixel centers into the top-left of our filter-box
        x -= SK_FixedHalf;
        y -= SK_FixedHalf;

        // compute our pointers
        {
            const SkBitmap* bitmap = &fBitmap;
            int ix = x >> 16;
            int iy = y >> 16;

            int             maxX = fMaxX;
            SkTileModeProc  procX = fTileProcX;
            int             maxY = fMaxY;
            SkTileModeProc  procY = fTileProcY;

            int tmpx = procX(ix, maxX);
            int tmpy = procY(iy, maxY);
            p00 = bitmap->getAddr32(tmpx, tmpy);

            int tmpx1 = procX(ix + 1, maxX);
            p01 = bitmap->getAddr32(tmpx1, tmpy);

            int tmpy1 = procY(iy + 1, maxY);
            p10 = bitmap->getAddr32(tmpx, tmpy1);

            p11 = bitmap->getAddr32(tmpx1, tmpy1);
        }

        SkFilterPtrProc proc = SkGetBilinearFilterPtrProc(fPtrProcTable, x, y);
        return proc(p00, p01, p10, p11);
    }

private:
    const SkFilterPtrProc* fPtrProcTable;
};

class RGB16_Bilinear_Sampler : public SkBitmapSampler {
public:
    RGB16_Bilinear_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, true, tmx, tmy)
    {
        fProcTable = SkGetBilinearFilterProcTable();
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        const uint16_t *p00, *p01, *p10, *p11;

        // turn pixel centers into the top-left of our filter-box
        x -= SK_FixedHalf;
        y -= SK_FixedHalf;

        // compute our pointers
        {
            const SkBitmap* bitmap = &fBitmap;
            int ix = x >> 16;
            int iy = y >> 16;

            int             maxX = fMaxX;
            SkTileModeProc  procX = fTileProcX;
            int             maxY = fMaxY;
            SkTileModeProc  procY = fTileProcY;

            int tmpx = procX(ix, maxX);
            int tmpy = procY(iy, maxY);
            p00 = bitmap->getAddr16(tmpx, tmpy);

            int tmpx1 = procX(ix + 1, maxX);
            p01 = bitmap->getAddr16(tmpx1, tmpy);

            int tmpy1 = procY(iy + 1, maxY);
            p10 = bitmap->getAddr16(tmpx, tmpy1);

            p11 = bitmap->getAddr16(tmpx1, tmpy1);
        }

        SkFilterProc proc = SkGetBilinearFilterProc(fProcTable, x, y);
        uint32_t c = proc(SkExpand_rgb_16(*p00), SkExpand_rgb_16(*p01),
                          SkExpand_rgb_16(*p10), SkExpand_rgb_16(*p11));

        return SkPixel16ToPixel32((uint16_t)SkCompact_rgb_16(c));
    }

private:
    const SkFilterProc* fProcTable;
};

// If we had a init/term method on sampler, we could avoid the per-pixel
// call to lockColors/unlockColors

class Index8_Bilinear_Sampler : public SkBitmapSampler {
public:
    Index8_Bilinear_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, true, tmx, tmy)
    {
        fPtrProcTable = SkGetBilinearFilterPtrProcTable();
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        const SkBitmap* bitmap = &fBitmap;

        const uint8_t *p00, *p01, *p10, *p11;

         // turn pixel centers into the top-left of our filter-box
        x -= SK_FixedHalf;
        y -= SK_FixedHalf;

       // compute our pointers
        {
            int ix = x >> 16;
            int iy = y >> 16;

            int             maxX = fMaxX;
            SkTileModeProc  procX = fTileProcX;
            int             maxY = fMaxY;
            SkTileModeProc  procY = fTileProcY;

            int tmpx = procX(ix, maxX);
            int tmpy = procY(iy, maxY);
            p00 = bitmap->getAddr8(tmpx, tmpy);

            int tmpx1 = procX(ix + 1, maxX);
            p01 = bitmap->getAddr8(tmpx1, tmpy);

            int tmpy1 = procY(iy + 1, maxY);
            p10 = bitmap->getAddr8(tmpx, tmpy1);

            p11 = bitmap->getAddr8(tmpx1, tmpy1);
        }

        const SkPMColor* colors = bitmap->getColorTable()->lockColors();

        SkFilterPtrProc proc = SkGetBilinearFilterPtrProc(fPtrProcTable, x, y);
        uint32_t c = proc(&colors[*p00], &colors[*p01], &colors[*p10], &colors[*p11]);

        bitmap->getColorTable()->unlockColors(false);

        return c;
    }

private:
    const SkFilterPtrProc* fPtrProcTable;
};

class A8_Bilinear_Sampler : public SkBitmapSampler {
public:
    A8_Bilinear_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, true, tmx, tmy)
        , fColor(0)
    {
        fProcTable = SkGetBilinearFilterProcTable();
    }

    virtual void setPaint(const SkPaint& paint)
    {
        fColor = SkPreMultiplyColor(paint.getColor());
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        const uint8_t *p00, *p01, *p10, *p11;

        // turn pixel centers into the top-left of our filter-box
        x -= SK_FixedHalf;
        y -= SK_FixedHalf;

        // compute our pointers
        {
            const SkBitmap* bitmap = &fBitmap;
            int ix = x >> 16;
            int iy = y >> 16;

            int             maxX = fMaxX;
            SkTileModeProc  procX = fTileProcX;
            int             maxY = fMaxY;
            SkTileModeProc  procY = fTileProcY;

            int tmpx = procX(ix, maxX);
            int tmpy = procY(iy, maxY);
            p00 = bitmap->getAddr8(tmpx, tmpy);

            int tmpx1 = procX(ix + 1, maxX);
            p01 = bitmap->getAddr8(tmpx1, tmpy);

            int tmpy1 = procY(iy + 1, maxY);
            p10 = bitmap->getAddr8(tmpx, tmpy1);

            p11 = bitmap->getAddr8(tmpx1, tmpy1);
        }

        SkFilterProc proc = SkGetBilinearFilterProc(fProcTable, x, y);
        int alpha = proc(*p00, *p01, *p10, *p11);
        return SkAlphaMulQ(fColor, SkAlpha255To256(alpha));
    }

private:
    const SkFilterProc* fProcTable;
    SkPMColor           fColor;
};

class A8_NoFilter_Sampler : public SkBitmapSampler {
public:
    A8_NoFilter_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, false, tmx, tmy)
    {
    }

    virtual void setPaint(const SkPaint& paint)
    {
        fColor = SkPreMultiplyColor(paint.getColor());
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        int ix = SkFixedFloor(x);
        int iy = SkFixedFloor(y);

        int alpha = *fBitmap.getAddr8(fTileProcX(ix, fMaxX), fTileProcY(iy, fMaxY));
        return SkAlphaMulQ(fColor, SkAlpha255To256(alpha));
    }

private:
    SkPMColor           fColor;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkBitmapSampler* SkBitmapSampler::Create(const SkBitmap& bm, bool doFilter,
                                         SkShader::TileMode tmx,
                                         SkShader::TileMode tmy)
{
    switch (bm.getConfig()) {
    case SkBitmap::kARGB_8888_Config:
        if (doFilter)
            return SkNEW_ARGS(ARGB32_Bilinear_Sampler, (bm, tmx, tmy));

        if (tmx == tmy) {
            switch (tmx) {
            case SkShader::kClamp_TileMode:
                return SkNEW_ARGS(ARGB32_Point_Clamp_Sampler, (bm));
            case SkShader::kRepeat_TileMode:
                if (is_pow2(bm.width()) && is_pow2(bm.height()))
                    return SkNEW_ARGS(ARGB32_Point_Repeat_Pow2_Sampler, (bm));
                else
                    return SkNEW_ARGS(ARGB32_Point_Repeat_Mod_Sampler, (bm));
            case SkShader::kMirror_TileMode:
                if (is_pow2(bm.width()) && is_pow2(bm.height()))
                    return SkNEW_ARGS(ARGB32_Point_Mirror_Pow2_Sampler, (bm));
                else
                    return SkNEW_ARGS(ARGB32_Point_Mirror_Mod_Sampler, (bm));
            default:
                SkDEBUGFAIL("unknown mode");
            }
        }
        else {  // tmx != tmy
            return SkNEW_ARGS(ARGB32_Point_Sampler, (bm, tmx, tmy));
        }
        break;

    case SkBitmap::kRGB_565_Config:
        if (doFilter)
            return SkNEW_ARGS(RGB16_Bilinear_Sampler, (bm, tmx, tmy));

        if (tmx == tmy) {
            switch (tmx) {
            case SkShader::kClamp_TileMode:
                return SkNEW_ARGS(RGB16_Point_Clamp_Sampler, (bm));
            case SkShader::kRepeat_TileMode:
                if (is_pow2(bm.width()) && is_pow2(bm.height()))
                    return SkNEW_ARGS(RGB16_Point_Repeat_Pow2_Sampler, (bm));
                else
                    return SkNEW_ARGS(RGB16_Point_Repeat_Mod_Sampler, (bm));
            case SkShader::kMirror_TileMode:
                if (is_pow2(bm.width()) && is_pow2(bm.height()))
                    return SkNEW_ARGS(RGB16_Point_Mirror_Pow2_Sampler, (bm));
                else
                    return SkNEW_ARGS(RGB16_Point_Mirror_Mod_Sampler, (bm));
            default:
                SkDEBUGFAIL("unknown mode");
            }
        }
        else {  // tmx != tmy
            return SkNEW_ARGS(RGB16_Point_Sampler, (bm, tmx, tmy));
        }
        break;

    case SkBitmap::kIndex8_Config:
        if (doFilter)
            return SkNEW_ARGS(Index8_Bilinear_Sampler, (bm, tmx, tmy));

        if (tmx == tmy) {
            switch (tmx) {
            case SkShader::kClamp_TileMode:
                return SkNEW_ARGS(Index8_Point_Clamp_Sampler, (bm));
            case SkShader::kRepeat_TileMode:
                if (is_pow2(bm.width()) && is_pow2(bm.height()))
                    return SkNEW_ARGS(Index8_Point_Repeat_Pow2_Sampler, (bm));
                else
                    return SkNEW_ARGS(Index8_Point_Repeat_Mod_Sampler, (bm));
            case SkShader::kMirror_TileMode:
                if (is_pow2(bm.width()) && is_pow2(bm.height()))
                    return SkNEW_ARGS(Index8_Point_Mirror_Pow2_Sampler, (bm));
                else
                    return SkNEW_ARGS(Index8_Point_Mirror_Mod_Sampler, (bm));
            default:
                SkDEBUGFAIL("unknown mode");
            }
        }
        else {  // tmx != tmy
            return SkNEW_ARGS(Index8_Point_Sampler, (bm, tmx, tmy));
        }
        break;

    case SkBitmap::kA8_Config:
        if (doFilter)
            return SkNEW_ARGS(A8_Bilinear_Sampler, (bm, tmx, tmy));
        else
            return SkNEW_ARGS(A8_NoFilter_Sampler, (bm, tmx, tmy));
        break;

    default:
        SkDEBUGFAIL("unknown device");
    }
    return SkNEW_ARGS(SkNullBitmapSampler, (bm, doFilter, tmx, tmy));
}
