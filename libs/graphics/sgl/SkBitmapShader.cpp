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

SkBitmapShader::SkBitmapShader(const SkBitmap& src,
							   bool transferOwnershipOfPixels,
							   SkPaint::FilterType filterType,
							   TileMode tmx, TileMode tmy)
    :
#ifdef SK_SUPPORT_MIPMAP
    fMipLevel(0), fMipSrcBitmap(src),
#endif
    fOrigSrcBitmap(src)
    
{
	if (transferOwnershipOfPixels)
	{
		fOrigSrcBitmap.setOwnsPixels(src.getOwnsPixels());
		((SkBitmap*)&src)->setOwnsPixels(false);
		// do the same for mipmap ownership???
	}
	fFilterType = SkToU8(filterType);
	fTileModeX = SkToU8(tmx);
	fTileModeY = SkToU8(tmy);
}

bool SkBitmapShader::setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix)
{
	// do this first, so we have a correct inverse matrix
	if (!this->INHERITED::setContext(device, paint, matrix))
		return false;

	uint32_t flags = fOrigSrcBitmap.isOpaque() ? kOpaqueAlpha_Flag : 0;

	if (flags == kOpaqueAlpha_Flag && paint.getAlpha() != 0xFF)
		flags = kConstAlpha_Flag;
	
	fFlags = SkToU8(flags);

#ifdef SK_SUPPORT_MIPMAP
	if (fOrigSrcBitmap.countMipLevels())
	{
		const SkMatrix& inv = this->getTotalInverse();

		fMipLevel = SkMin32(find_mip_level(	SkScalarToFixed(inv.getScaleX()),
											SkScalarToFixed(inv.getSkewY())),
							SkIntToFixed(fOrigSrcBitmap.countMipLevels() - 1));

//        SkDEBUGF(("BitmapShader miplevel=%x\n", fMipLevel));

        const SkBitmap::MipLevel* mm = fOrigSrcBitmap.getMipLevel(fMipLevel >> 16);
        
        fMipSrcBitmap.setConfig(fOrigSrcBitmap.getConfig(),
                                mm->fWidth,
                                mm->fHeight,
                                mm->fRowBytes);
        fMipSrcBitmap.setPixels(mm->fPixels);
    }
    else
    {
        fMipLevel = 0;
        fMipSrcBitmap = fOrigSrcBitmap;
    }
#endif
	return true;
}

///////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"
#include "SkBitmapSampler.h"

class Sampler_BitmapShader : public SkBitmapShader {
public:
	Sampler_BitmapShader(const SkBitmap& src,
						 bool transferOwnershipOfPixels,
						 SkPaint::FilterType ftype,
						 TileMode tmx, TileMode tmy)
		: SkBitmapShader(src, transferOwnershipOfPixels, ftype, tmx, tmy)
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
            fSampler = SkBitmapSampler::Create(this->getSrcBitmap(), this->getFilterType(),
                                               this->getTileModeX(), this->getTileModeY());
            return true;
        }
        return false;
    }

	virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count)
	{
		unsigned			scale = SkAlpha255To256(this->getPaintAlpha());
		const SkMatrix&		inv = this->getTotalInverse();
		SkMatrix::MapPtProc	proc = this->getInverseMapPtProc();
		SkBitmapSampler*	 sampler = fSampler;
		MatrixClass			mc = this->getInverseClass();

		SkPoint	srcPt;

		if (mc != kPerspective_MatrixClass)
		{
			proc(inv, SkIntToScalar(x), SkIntToScalar(y), &srcPt);

			SkFixed	fx = SkScalarToFixed(srcPt.fX);
			SkFixed fy = SkScalarToFixed(srcPt.fY);
			SkFixed	dx, dy;

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
					uint32_t	c = sampler->sample(fx, fy);
					dstC[i] = SkAlphaMulQ(c, scale);
					fx += dx;
					fy += dy;
				}
			}
		}
		else
		{
			SkScalar	dstX = SkIntToScalar(x);
			SkScalar	dstY = SkIntToScalar(y);

			for (int i = 0; i < count; i++)
			{
				proc(inv, dstX, dstY, &srcPt);
				uint32_t	c = sampler->sample(SkScalarToFixed(srcPt.fX), SkScalarToFixed(srcPt.fY));

				if (scale != 256)
					c = SkAlphaMulQ(c, scale);
				dstC[i] = c;
				dstX += SK_Scalar1;
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
		fUnitInverse.postScale(SK_Scalar1 / src.width(), SK_Scalar1 / src.height(), 0, 0);
        fUnitInverseProc = fUnitInverse.getMapPtProc();
	}

private:
	SkBitmapSampler*	fSampler;
	SkMatrix            fUnitInverse;
    SkMatrix::MapPtProc fUnitInverseProc;
    
    typedef SkBitmapShader INHERITED;
};

///////////////////////////////////////////////////////////////////////////

class HasSpan16_Sampler_BitmapShader : public Sampler_BitmapShader {
public:
	HasSpan16_Sampler_BitmapShader(const SkBitmap& src, bool transferOwnershipOfPixels,
									SkPaint::FilterType ft, TileMode tmx, TileMode tmy)
		: Sampler_BitmapShader(src, transferOwnershipOfPixels, ft, tmx, tmy)
	{
	}

	virtual uint32_t getFlags()
	{
		uint32_t flags = this->INHERITED::getFlags();

		if (this->getPaintAlpha() == 0xFF && this->getInverseClass() != kPerspective_MatrixClass)
			flags |= SkShader::kHasSpan16_Flag;
		else
			flags &= ~SkShader::kHasSpan16_Flag;

		return flags;
	}
private:
	typedef Sampler_BitmapShader INHERITED;
};

///////////////////////////////////////////////////////////////////////////

#define NOFILTER_BITMAP_SHADER_CLASS					Index8_NoFilter_ClampTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE					SkShader::kClamp_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)			SkClampMax((x >> 16), max)
#define NOFILTER_BITMAP_SHADER_TYPE						uint8_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)			colors32[p[x]]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)	colors32[p[x + y * rb]]
#define NOFILTER_BITMAP_SHADER_PREAMBLE(bitmap, rb)		const SkPMColor* colors32 = bitmap.getColorTable()->lockColors()
#define NOFILTER_BITMAP_SHADER_POSTAMBLE(bitmap)		bitmap.getColorTable()->unlockColors(false)
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)			colors16[p[x]]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb)	colors16[p[x + y * rb]]
#define NOFILTER_BITMAP_SHADER_PREAMBLE16(bitmap, rb)	const uint16_t* colors16 = bitmap.getColorTable()->lock16BitCache()
#define NOFILTER_BITMAP_SHADER_POSTAMBLE16(bitmap)		bitmap.getColorTable()->unlock16BitCache()
#include "SkBitmapShaderTemplate.h"

#define NOFILTER_BITMAP_SHADER_CLASS					Index8_NoFilter_RepeatTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE					SkShader::kRepeat_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)			(fixed_repeat(x) * (max + 1) >> 16)
#define NOFILTER_BITMAP_SHADER_TYPE						uint8_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)			colors32[p[x]]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)	colors32[p[x + y * rb]]
#define NOFILTER_BITMAP_SHADER_PREAMBLE(bitmap, rb)		const SkPMColor* colors32 = bitmap.getColorTable()->lockColors()
#define NOFILTER_BITMAP_SHADER_POSTAMBLE(bitmap)		bitmap.getColorTable()->unlockColors(false)
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)			colors16[p[x]]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb)	colors16[p[x + y * rb]]
#define NOFILTER_BITMAP_SHADER_PREAMBLE16(bitmap, rb)	const uint16_t* colors16 = bitmap.getColorTable()->lock16BitCache()
#define NOFILTER_BITMAP_SHADER_POSTAMBLE16(bitmap)		bitmap.getColorTable()->unlock16BitCache()
#define NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
#include "SkBitmapShaderTemplate.h"

#define NOFILTER_BITMAP_SHADER_CLASS					U16_NoFilter_ClampTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE					SkShader::kClamp_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)			SkClampMax((x >> 16), max)
#define NOFILTER_BITMAP_SHADER_TYPE						uint16_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)			SkPixel16ToPixel32(p[x])
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)	SkPixel16ToPixel32(*(const uint16_t*)((const char*)p + y * rb + (x << 1)))
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)			p[x]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb)	*(const uint16_t*)((const char*)p + y * rb + (x << 1))
#include "SkBitmapShaderTemplate.h"

#define NOFILTER_BITMAP_SHADER_CLASS					U16_NoFilter_RepeatTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE					SkShader::kRepeat_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)			(fixed_repeat(x) * (max + 1) >> 16)
#define NOFILTER_BITMAP_SHADER_TYPE						uint16_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)			SkPixel16ToPixel32(p[x])
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)	SkPixel16ToPixel32(*(const uint16_t*)((const char*)p + y * rb + (x << 1)))
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)			p[x]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb)	*(const uint16_t*)((const char*)p + y * rb + (x << 1))
#define NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
#include "SkBitmapShaderTemplate.h"

#define NOFILTER_BITMAP_SHADER_CLASS					U32_NoFilter_ClampTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE					SkShader::kClamp_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)			SkClampMax((x >> 16), max)
#define NOFILTER_BITMAP_SHADER_TYPE						uint32_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)			p[x]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)	*(const uint32_t*)((const char*)p + y * rb + (x << 2))
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)			SkPixel32ToPixel16_ToU16(p[x])
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb)	SkPixel32ToPixel16_ToU16(*(const uint32_t*)((const char*)p + y * rb + (x << 2)))
#include "SkBitmapShaderTemplate.h"

#define NOFILTER_BITMAP_SHADER_CLASS					U32_NoFilter_RepeatTile_BitmapShader
#define NOFILTER_BITMAP_SHADER_TILEMODE					SkShader::kRepeat_TileMode
#define NOFILTER_BITMAP_SHADER_TILEPROC(x, max)			(fixed_repeat(x) * (max + 1) >> 16)
#define NOFILTER_BITMAP_SHADER_TYPE						uint32_t
#define NOFILTER_BITMAP_SHADER_SAMPLE_X(p, x)			p[x]
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY(p, x, y, rb)	*(const uint32_t*)((const char*)p + y * rb + (x << 2))
#define NOFILTER_BITMAP_SHADER_SAMPLE_X16(p, x)			SkPixel32ToPixel16_ToU16(p[x])
#define NOFILTER_BITMAP_SHADER_SAMPLE_XY16(p, x, y, rb)	SkPixel32ToPixel16_ToU16(*(const uint32_t*)((const char*)p + y * rb + (x << 2)))
#define NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
#include "SkBitmapShaderTemplate.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////

#define Pack4Bytes(c00, c01, c10, c11)	(((c00) << 24) | ((c01) << 16) | ((c10) << 8) | (c11))
/*	Each long stores 4 coefficients, each in a byte.
	coeff >> 24	-> [0][0]
	coeff >> 16	-> [0][1]
	coeff >>  8	-> [1][0]
	coeff >>  0	-> [1][1]
*/
static const uint32_t gBilerpPackedCoeff[] = {
	/* y == 0 */
	Pack4Bytes(16,  0,  0,  0),	// x == 0
	Pack4Bytes(12,  4,  0,  0),	// x == 1/4
	Pack4Bytes( 8,  8,  0,  0),	// x == 1/2
	Pack4Bytes( 4, 12,  0,  0),	// x == 3/4

	/* y == 1/4 */
	Pack4Bytes(12,  0,  4,  0),
	Pack4Bytes( 9,  3,  3,  1),
	Pack4Bytes( 6,  6,  2,  2),
	Pack4Bytes( 3,  9,  1,  3),

	/* y == 1/2 */
	Pack4Bytes( 8,  0,  8,  0),
	Pack4Bytes( 6,  2,  6,  2),
	Pack4Bytes( 4,  4,  4,  4),
	Pack4Bytes( 2,  6,  2,  6),

	/* y == 3/4 */
	Pack4Bytes( 4,  0, 12,  0),
	Pack4Bytes( 3,  1,  9,  3),
	Pack4Bytes( 2,  2,  6,  6),
	Pack4Bytes( 1,  3,  3,  9)
};

// extract the high two bits in the fractional part of the fixed
#define SK_BILERP_GET_BITS(x)	(((x) >> 14) & 3)

static inline uint32_t sk_find_bilerp_coeff(const uint32_t coeff[], SkFixed fx, SkFixed fy)
{
#ifdef SK_DEBUG
	uint32_t c = coeff[(SK_BILERP_GET_BITS(fy) << 2) | SK_BILERP_GET_BITS(fx)];
	SkASSERT((c >> 24) + ((c >> 16) & 0xFF) + ((c >> 8) & 0xFF) + (c & 0xFF) == 16);
#endif
	return coeff[(SK_BILERP_GET_BITS(fy) << 2) | SK_BILERP_GET_BITS(fx)];
}

static inline uint32_t expand_rgb_16(U16CPU c, U16CPU rbMask)
{
	return ((c & SK_G16_MASK_IN_PLACE) << 16) | (c & rbMask);
}

static inline U16CPU compact_rgb_16(uint32_t c, U16CPU rbMask)
{
	return ((c >> 16) & SK_G16_MASK_IN_PLACE) | (c & rbMask);
}

static inline U16CPU sk_bilerp16(U16CPU c00, U16CPU c01, U16CPU c10, U16CPU c11, uint32_t coeff, U16CPU rbMask)
{
//	U16CPU rbMask = SK_R16B16_MASK_IN_PLACE;

	c00 =	expand_rgb_16(c00, rbMask) * (coeff >> 24) +
			expand_rgb_16(c01, rbMask) * ((coeff >> 16) & 0xFF) +
			expand_rgb_16(c10, rbMask) * ((coeff >> 8) & 0xFF) +
			expand_rgb_16(c11, rbMask) * (coeff & 0xFF);

	return compact_rgb_16(c00 >> 4, rbMask);
}

// this wacky line is to force the compiler to put this contant into a register
// rather than try to construct it each time it is referenced in the inner-loop
extern const uint16_t gRBMask_Bilerp_BitmapShader;

#define BILERP_BITMAP16_SHADER_CLASS			U16_Bilerp_BitmapShader
#define BILERP_BITMAP16_SHADER_TYPE				uint16_t
#define BILERP_BITMAP16_SHADER_PREAMBLE(bm)
#define BILERP_BITMAP16_SHADER_PIXEL(c)			(c)
#define BILERP_BITMAP16_SHADER_POSTAMBLE(bm)
#include "SkBitmapShader16BilerpTemplate.h"

#define BILERP_BITMAP16_SHADER_CLASS			Index8_Bilerp_BitmapShader
#define BILERP_BITMAP16_SHADER_TYPE				uint8_t
#define BILERP_BITMAP16_SHADER_PREAMBLE(bm)		SkColorTable* ctable = (bm).getColorTable(); const uint16_t* colors16 = ctable->lock16BitCache()
#define BILERP_BITMAP16_SHADER_PIXEL(c)			colors16[c]
#define BILERP_BITMAP16_SHADER_POSTAMBLE(bm)	ctable->unlock16BitCache()
#include "SkBitmapShader16BilerpTemplate.h"

// we define it below all the includes, so they won't try to inline the value
// (which doesn't fit in an immediate register load)
const uint16_t gRBMask_Bilerp_BitmapShader = SK_R16B16_MASK_IN_PLACE;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include "SkTemplatesPriv.h"

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src,
									   bool transferOwnershipOfPixels,
									   SkPaint::FilterType filterType,
									   TileMode tmx, TileMode tmy,
									   void* storage, size_t storageSize)
{
	SkShader* shader = NULL;

	if (filterType == SkPaint::kNo_FilterType)
	{
		switch (src.getConfig()) {
		case SkBitmap::kIndex8_Config:
			if (kClamp_TileMode == tmx && kClamp_TileMode == tmy)
				SK_PLACEMENT_NEW_ARGS(shader, Index8_NoFilter_ClampTile_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
            else if (kRepeat_TileMode == tmx && kRepeat_TileMode == tmy)
				SK_PLACEMENT_NEW_ARGS(shader, Index8_NoFilter_RepeatTile_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
			break;
		case SkBitmap::kRGB_565_Config:
            if (kClamp_TileMode == tmx && kClamp_TileMode == tmy)
                SK_PLACEMENT_NEW_ARGS(shader, U16_NoFilter_ClampTile_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
            else if (kRepeat_TileMode == tmx && kRepeat_TileMode == tmy)
                SK_PLACEMENT_NEW_ARGS(shader, U16_NoFilter_RepeatTile_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
			break;
		case SkBitmap::kARGB_8888_Config:
			if (kClamp_TileMode == tmx && kClamp_TileMode == tmy)
				SK_PLACEMENT_NEW_ARGS(shader, U32_NoFilter_ClampTile_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
            else if (kRepeat_TileMode == tmx && kRepeat_TileMode == tmy)
				SK_PLACEMENT_NEW_ARGS(shader, U32_NoFilter_RepeatTile_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
			break;
		default:
			break;
		}
	}
	else if (filterType == SkPaint::kBilinear_FilterType
             && kClamp_TileMode == tmx
             && kClamp_TileMode == tmy)
	{
		switch (src.getConfig()) {
		case SkBitmap::kIndex8_Config:
			SK_PLACEMENT_NEW_ARGS(shader, Index8_Bilerp_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
			break;
		case SkBitmap::kRGB_565_Config:
			SK_PLACEMENT_NEW_ARGS(shader, U16_Bilerp_BitmapShader, storage, storageSize, (src, transferOwnershipOfPixels));
			break;
        default:
            break;
		}
	}
    
    // if shader is null, then none of the special cases could handle the request
    // so fall through to our slow-general case
	if (shader == NULL)
		SK_PLACEMENT_NEW_ARGS(shader, Sampler_BitmapShader, storage, storageSize,
                              (src, transferOwnershipOfPixels, filterType, tmx, tmy));
	return shader;
}

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src,
									   bool transferOwnershipOfPixels,
									   SkPaint::FilterType filterType,
									   TileMode tmx, TileMode tmy)
{
	return SkShader::CreateBitmapShader(src, transferOwnershipOfPixels, filterType, tmx, tmy, NULL, 0);
}

