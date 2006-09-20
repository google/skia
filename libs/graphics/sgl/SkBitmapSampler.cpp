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

#define BITMAP_CLASSNAME_PREFIX(name)			ARGB32##name
#define BITMAP_PIXEL_TO_PMCOLOR(bitmap, x, y)	*bitmap.getAddr32(x, y)
#include "SkBitmapSamplerTemplate.h"

#include "SkColorPriv.h"

#define BITMAP_CLASSNAME_PREFIX(name)			RGB16##name
#define BITMAP_PIXEL_TO_PMCOLOR(bitmap, x, y)	SkPixel16ToPixel32(*bitmap.getAddr16(x, y))
#include "SkBitmapSamplerTemplate.h"

#define BITMAP_CLASSNAME_PREFIX(name)			Index8##name
#define BITMAP_PIXEL_TO_PMCOLOR(bitmap, x, y)	bitmap.getIndex8Color(x, y)
#include "SkBitmapSamplerTemplate.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///////////////// The Bilinear versions

static void assert_valid_pmcolor(uint32_t c)
{
	SkASSERT(SkGetPackedA32(c) >= SkGetPackedR32(c));
	SkASSERT(SkGetPackedA32(c) >= SkGetPackedG32(c));
	SkASSERT(SkGetPackedA32(c) >= SkGetPackedB32(c));
}

#include "SkFilterProc.h"

class ARGB32_Bilinear_Sampler : public SkBitmapSampler {
public:
	ARGB32_Bilinear_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
		: SkBitmapSampler(bm, SkPaint::kBilinear_FilterType, tmx, tmy)
	{
		fProcTable = SkGetBilinearFilterProcTable();
	}

	virtual SkPMColor sample(SkFixed x, SkFixed y) const
	{
		int	ix = x >> 16;
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

		uint32_t c00 = *p00;
		uint32_t c01 = *p01;
		uint32_t c10 = *p10;
		uint32_t c11 = *p11;

		assert_valid_pmcolor(c00);
		assert_valid_pmcolor(c01);
		assert_valid_pmcolor(c01);
		assert_valid_pmcolor(c11);

		SkFilterProc proc = SkGetBilinearFilterProc(fProcTable, x, y);
		uint32_t c = SkPackARGB32(	proc(SkGetPackedA32(c00), SkGetPackedA32(c01), SkGetPackedA32(c10), SkGetPackedA32(c11)),
								proc(SkGetPackedR32(c00), SkGetPackedR32(c01), SkGetPackedR32(c10), SkGetPackedR32(c11)),
								proc(SkGetPackedG32(c00), SkGetPackedG32(c01), SkGetPackedG32(c10), SkGetPackedG32(c11)),
								proc(SkGetPackedB32(c00), SkGetPackedB32(c01), SkGetPackedB32(c10), SkGetPackedB32(c11)));

		assert_valid_pmcolor(c);
		return c;
	}
	
private:
	const SkFilterProc*   fProcTable;
};

class Index8_Bilinear_Sampler : public SkBitmapSampler {
public:
	Index8_Bilinear_Sampler(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
		: SkBitmapSampler(bm, SkPaint::kBilinear_FilterType, tmx, tmy)
	{
		fProcTable = SkGetBilinearFilterProcTable();
	}

	virtual SkPMColor sample(SkFixed x, SkFixed y) const
	{
		int	ix = x >> 16;
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

		uint32_t c00 = colors[*p00];
		uint32_t c01 = colors[*p01];
		uint32_t c10 = colors[*p10];
		uint32_t c11 = colors[*p11];

		assert_valid_pmcolor(c00);
		assert_valid_pmcolor(c01);
		assert_valid_pmcolor(c01);
		assert_valid_pmcolor(c11);

		SkFilterProc proc = SkGetBilinearFilterProc(fProcTable, x, y);
		uint32_t c = SkPackARGB32(	proc(SkGetPackedA32(c00), SkGetPackedA32(c01), SkGetPackedA32(c10), SkGetPackedA32(c11)),
								proc(SkGetPackedR32(c00), SkGetPackedR32(c01), SkGetPackedR32(c10), SkGetPackedR32(c11)),
								proc(SkGetPackedG32(c00), SkGetPackedG32(c01), SkGetPackedG32(c10), SkGetPackedG32(c11)),
								proc(SkGetPackedB32(c00), SkGetPackedB32(c01), SkGetPackedB32(c10), SkGetPackedB32(c11)));

		assert_valid_pmcolor(c);

		fBitmap.getColorTable()->unlockColors(false);

		return c;
	}
	
private:
	const SkFilterProc*   fProcTable;
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
		default:	// to avoid warnings
			break;
		}
		break;
	default:
		SkASSERT(!"unknown device");
	}
	return SkNEW_ARGS(SkNullBitmapSampler, (bm, ftype, tmx, tmy));
}

