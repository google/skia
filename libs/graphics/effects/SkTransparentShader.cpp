#include "SkTransparentShader.h"
#include "SkColorPriv.h"

bool SkTransparentShader::setContext(const SkBitmap& device,
									 const SkPaint& paint,
									 const SkMatrix& matrix)
{
	fDevice = device;
	fAlpha = paint.getAlpha();

	return this->INHERITED::setContext(device, paint, matrix);
}

U32 SkTransparentShader::getFlags()
{
	U32	flags = 0;

	switch (fDevice.getConfig()) {
	case SkBitmap::kRGB_565_Config:
		flags |= kHasSpan16_Flag;
		// fall through
	case SkBitmap::kARGB_8888_Config:
		flags |= (fAlpha == 255 ? kOpaqueAlpha_Flag : kConstAlpha_Flag);
	default:
		break;
	}
	return flags;
}

void SkTransparentShader::shadeSpan(int x, int y, SkPMColor span[], int count)
{
	unsigned scale = SkAlpha255To256(fAlpha);

	switch (fDevice.getConfig()) {
	case SkBitmap::kARGB_8888_Config:
		if (scale == 256)
			memcpy(span, fDevice.getAddr32(x, y), count * sizeof(SkPMColor));
		else
		{
			const SkPMColor* src = fDevice.getAddr32(x, y);
			for (int i = count - 1; i >= 0; --i)
				span[i] = SkAlphaMulQ(src[i], scale);
		}
		break;
	case SkBitmap::kRGB_565_Config:
		{
			const U16* src = fDevice.getAddr16(x, y);
			if (scale == 256)
			{
				for (int i = count - 1; i >= 0; --i)
					span[i] = SkPixel16ToPixel32(src[i]);
			}
			else
			{
				unsigned alpha = fAlpha;
				for (int i = count - 1; i >= 0; --i)
				{
					U16		 c = src[i];
					unsigned r = SkPacked16ToR32(c);
					unsigned g = SkPacked16ToG32(c);
					unsigned b = SkPacked16ToB32(c);

					span[i] = SkPackARGB32(	alpha,
											SkAlphaMul(r, scale),
											SkAlphaMul(g, scale),
											SkAlphaMul(b, scale));
				}
			}
		}
		break;
	case SkBitmap::kIndex8_Config:
		SkASSERT(!"index8 not supported as a destination device");
		break;
	case SkBitmap::kA8_Config:
		{
			const U8* src = fDevice.getAddr8(x, y);
			if (scale == 256)
			{
				for (int i = count - 1; i >= 0; --i)
					span[i] = SkPackARGB32(src[i], 0, 0, 0);
			}
			else
			{
				for (int i = count - 1; i >= 0; --i)
					span[i] = SkPackARGB32(SkAlphaMul(src[i], scale), 0, 0, 0);
			}
		}
		break;
	case SkBitmap::kA1_Config:
		SkASSERT(!"kA1_Config umimplemented at this time");
		break;
	default:	// to avoid warnings
		break;
	}
}

void SkTransparentShader::shadeSpanOpaque16(int x, int y, U16 span[], int count)
{
	SkASSERT(fAlpha == 255);
	SkASSERT(fDevice.getConfig() == SkBitmap::kRGB_565_Config);

	memcpy(span, fDevice.getAddr16(x, y), count << 1);
}

