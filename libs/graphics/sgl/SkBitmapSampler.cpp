/* libs/graphics/sgl/SkBitmapSampler.cpp
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
        SkASSERT(!"unknown mode");
        return NULL;
    }
}

SkBitmapSampler::SkBitmapSampler(const SkBitmap& bm, SkPaint::FilterType ftype,
                                 SkShader::TileMode tmx, SkShader::TileMode tmy)
    : fBitmap(bm), fFilterType(ftype), fTileModeX(tmx), fTileModeY(tmy)
{
    SkASSERT(bm.width() > 0 && bm.height() > 0);

    fMaxX = SkToU16(bm.width() - 1);
    fMaxY = SkToU16(bm.height() - 1);
    
    fTileProcX = get_tilemode_proc(tmx);
    fTileProcY = get_tilemode_proc(tmy);
}

class SkNullBitmapSampler : public SkBitmapSampler {
public:
    SkNullBitmapSampler(const SkBitmap& bm, SkPaint::FilterType ft,
                        SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, ft, tmx, tmy) {}

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
        : SkBitmapSampler(bm, SkPaint::kBilinear_FilterType, tmx, tmy)
    {
        fPtrProcTable = SkGetBilinearFilterPtrProcTable();
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        int ix = x >> 16;
        int iy = y >> 16;

        ix = fTileProcX(ix, fMaxX);
        iy = fTileProcY(iy, fMaxY);

        const uint32_t *p00, *p01, *p10, *p11;

        p00 = p01 = fBitmap.getAddr32(ix, iy);
        if (ix < fMaxX)
            p01 += 1;
        p10 = p00;
        p11 = p01;
        if (iy < fMaxY)
        {
            p10 = (const uint32_t*)((const char*)p10 + fBitmap.rowBytes());
            p11 = (const uint32_t*)((const char*)p11 + fBitmap.rowBytes());
        }

        SkFilterPtrProc proc = SkGetBilinearFilterPtrProc(fPtrProcTable, x, y);
        return proc(p00, p01, p10, p11);
    }
    
private:
    const SkFilterPtrProc* fPtrProcTable;
};

// If we had a init/term method on sampler, we could avoid the per-pixel
// call to lockColors/unlockColors

class Index8_Bilinear_Sampler : public SkBitmapSampler {
public:
    Index8_Bilinear_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, SkPaint::kBilinear_FilterType, tmx, tmy)
    {
        fPtrProcTable = SkGetBilinearFilterPtrProcTable();
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        int ix = x >> 16;
        int iy = y >> 16;

        ix = fTileProcX(ix, fMaxX);
        iy = fTileProcY(iy, fMaxY);

        const U8 *p00, *p01, *p10, *p11;

        p00 = p01 = fBitmap.getAddr8(ix, iy);
        if (ix < fMaxX)
            p01 += 1;
        p10 = p00;
        p11 = p01;
        if (iy < fMaxY)
        {
            p10 = (const U8*)((const char*)p10 + fBitmap.rowBytes());
            p11 = (const U8*)((const char*)p11 + fBitmap.rowBytes());
        }

        const SkPMColor* colors = fBitmap.getColorTable()->lockColors();

        SkFilterPtrProc proc = SkGetBilinearFilterPtrProc(fPtrProcTable, x, y);
        uint32_t c = proc(&colors[*p00], &colors[*p01], &colors[*p10], &colors[*p11]);

        fBitmap.getColorTable()->unlockColors(false);

        return c;
    }
    
private:
    const SkFilterPtrProc* fPtrProcTable;
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

SkBitmapSampler* SkBitmapSampler::Create(const SkBitmap& bm, SkPaint::FilterType ftype,
                                         SkShader::TileMode tmx, SkShader::TileMode tmy)
{
    switch (bm.getConfig()) {
    case SkBitmap::kARGB_8888_Config:
        switch (ftype) {
        case SkPaint::kNo_FilterType:
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
                    SkASSERT(!"unknown mode");
                }
            }
            else {  // tmx != tmy
                return SkNEW_ARGS(ARGB32_Point_Sampler, (bm, tmx, tmy));
            }
            break;

        case SkPaint::kBilinear_FilterType:
            return SkNEW_ARGS(ARGB32_Bilinear_Sampler, (bm, tmx, tmy));

        default:
            SkASSERT(!"unknown filter type");
        }
        break;
    case SkBitmap::kRGB_565_Config:
        // we ignore ftype, since we haven't implemented bilinear for 16bit bitmaps yet
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
                SkASSERT(!"unknown mode");
            }
        }
        else {  // tmx != tmy
            return SkNEW_ARGS(RGB16_Point_Sampler, (bm, tmx, tmy));
        }
        break;
    case SkBitmap::kIndex8_Config:
        switch (ftype) {
        case SkPaint::kNo_FilterType:
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
                    SkASSERT(!"unknown mode");
                }
            }
            else {  // tmx != tmy
                return SkNEW_ARGS(Index8_Point_Sampler, (bm, tmx, tmy));
            }
            break;
        case SkPaint::kBilinear_FilterType:
            return SkNEW_ARGS(Index8_Bilinear_Sampler, (bm, tmx, tmy));
        default:    // to avoid warnings
            break;
        }
        break;
    default:
        SkASSERT(!"unknown device");
    }
    return SkNEW_ARGS(SkNullBitmapSampler, (bm, ftype, tmx, tmy));
}

