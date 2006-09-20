#include "SkShader.h"
#include "SkPaint.h"

SkShader::SkShader() : fLocalMatrix(nil)
{
}

SkShader::~SkShader()
{
	sk_free(fLocalMatrix);
}

void SkShader::setLocalMatrix(const SkMatrix& matrix)
{
	if (matrix.isIdentity())
	{
		if (fLocalMatrix)
		{
			sk_free(fLocalMatrix);
			fLocalMatrix = nil;
		}
	}
	else
	{
		if (fLocalMatrix == nil)
			fLocalMatrix = (SkMatrix*)sk_malloc_throw(sizeof(SkMatrix));
		*fLocalMatrix = matrix;
	}
}

bool SkShader::setContext(const SkBitmap& device,
						  const SkPaint& paint,
						  const SkMatrix& matrix)
{
	const SkMatrix*	m = &matrix;
	SkMatrix		total;

	fDeviceConfig = SkToU8(device.getConfig());
	fPaintAlpha = paint.getAlpha();
	if (fLocalMatrix)
	{
		total.setConcat(matrix, *fLocalMatrix);
		m = &total;
	}
	if (m->invert(&fTotalInverse))
	{
		fInverseMapPtProc = fTotalInverse.getMapPtProc();
		fTotalInverseClass = (U8)SkShader::ComputeMatrixClass(fTotalInverse);
		return true;
	}
	return false;
}

U32 SkShader::getFlags()
{
	return 0;
}

#include "SkColorPriv.h"

void SkShader::shadeSpanOpaque16(int x, int y, U16 span16[], int count)
{
	SkASSERT(span16);
	SkASSERT(count > 0);
	SkASSERT(this->canCallShadeSpanOpaque16());

	// basically, if we get here, the subclass screwed up
	SkASSERT(!"kHasSpan16 flag is set, but shadeSpanOpaque16() not implemented");
}

#define kTempColorQuadCount	6	// balance between speed (larger) and saving stack-space
#define kTempColorCount		(kTempColorQuadCount << 2)	

#ifdef SK_CPU_BENDIAN
	#define SkU32BitShiftToByteOffset(shift)	(3 - ((shift) >> 3))
#else
	#define SkU32BitShiftToByteOffset(shift)	((shift) >> 3)
#endif

void SkShader::shadeSpanAlpha(int x, int y, U8 alpha[], int count)
{
	SkASSERT(count > 0);

	SkPMColor	colors[kTempColorCount];

	while ((count -= kTempColorCount) >= 0)
	{
		this->shadeSpan(x, y, colors, kTempColorCount);
		x += kTempColorCount;

		const U8* srcA = (const U8*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
		int quads = kTempColorQuadCount;
		do {
			U8CPU a0 = srcA[0];
			U8CPU a1 = srcA[4];
			U8CPU a2 = srcA[8];
			U8CPU a3 = srcA[12];
			srcA += 4*4;
			*alpha++ = SkToU8(a0);
			*alpha++ = SkToU8(a1);
			*alpha++ = SkToU8(a2);
			*alpha++ = SkToU8(a3);
		} while (--quads != 0);
	}
	SkASSERT(count < 0);
	SkASSERT(count + kTempColorCount >= 0);
	if (count += kTempColorCount)
	{
		this->shadeSpan(x, y, colors, count);

		const U8* srcA = (const U8*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
		do {
			*alpha++ = *srcA;
			srcA += 4;
		} while (--count != 0);
	}
#if 0
	do {
		int n = count;
		if (n > kTempColorCount)
			n = kTempColorCount;
		SkASSERT(n > 0);

		this->shadeSpan(x, y, colors, n);
		x += n;
		count -= n;

		const U8* srcA = (const U8*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
		do {
			*alpha++ = *srcA;
			srcA += 4;
		} while (--n != 0);
	} while (count > 0);
#endif
}

SkShader::MatrixClass SkShader::ComputeMatrixClass(const SkMatrix& mat)
{
	MatrixClass	mc = kLinear_MatrixClass;

	if (mat.getType() & SkMatrix::kPerspective_Mask)
	{
		if (mat.fixedStepInX(0, nil, nil))
			mc = kFixedStepInX_MatrixClass;
		else
			mc = kPerspective_MatrixClass;
	}
	return mc;
}

////////////////////////////////////////////////////////////////////////////////////////

#if 0
SkPairShader::SkPairShader(SkShader* s0, SkShader* s1)
	: fShader0(s0), fShader1(s1)
{
	s0->safeRef();
	s1->safeRef();
}

SkPairShader::~SkPairShader()
{
	fShader1->safeUnref();
	fShader0->safeUnref();
}

U32 SkPairShader::getFlags()
{
	SkASSERT(fShader0 || fShader1);

	U32	flags = 0-1U;

	if (fShader0)
		flags &= fShader0->getFlags();
	if (fShader1)
		flags &= fShader1->getFlags();
	return flags;
}

bool SkPairShader::setContext(	const SkBitmap& device,
								const SkPaint& paint,
								const SkMatrix& matrix)
{
	if (fShader0 == nil && fShader1 == nil)
		return false;

	const SkMatrix* localM = this->getLocalMatrix();
	SkMatrix		tmp;

	if (localM)
	{
		tmp.setConcat(matrix, *localM);
		localM = &tmp;
	}
	else
		localM = &matrix;

	//	wonder if some subclasses will want OR instead of AND?

	if (fShader0 && !fShader0->setContext(device, paint, *localM))
		return false;
	if (fShader1 && !fShader1->setContext(device, paint, *localM))
		return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

void SkComposeShader::shadeSpan(int x, int y, SkPMColor span[], int count)
{
	SkShader*	s0 = this->getShader0();
	SkShader*	s1 = this->getShader1();

	if (s1)
		s1->shadeSpan(x, y, span, count);
	if (s0)
		s0->shadeSpan(x, y, span, count);
}

void SkComposeShader::shadeSpanOpaque16(int x, int y, U16 span[], int count)
{
	SkShader*	s0 = this->getShader0();
	SkShader*	s1 = this->getShader1();

	if (s1)
		s1->shadeSpanOpaque16(x, y, span, count);
	if (s0)
		s0->shadeSpanOpaque16(x, y, span, count);
}

void SkComposeShader::shadeSpanAlpha(int x, int y, U8 span[], int count)
{
	SkShader*	s0 = this->getShader0();
	SkShader*	s1 = this->getShader1();

	if (s1)
		s1->shadeSpanAlpha(x, y, span, count);
	if (s0)
		s0->shadeSpanAlpha(x, y, span, count);
}

////////////////////////////////////////////////////////////////////////////////////////

#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkBitmap.h"

SkSumShader::SkSumShader(SkShader* s0, SkShader* s1, U8CPU weight)
	: SkPairShader(s0, s1), fBuffer(nil), fMode(nil), fWeight(SkToU8(weight))
{
}

SkSumShader::SkSumShader(SkShader* s0, SkShader* s1, SkXfermode* mode)
	: SkPairShader(s0, s1), fBuffer(nil), fMode(mode), fWeight(0xFF)
{
	mode->safeRef();
}

SkSumShader::~SkSumShader()
{
	fMode->safeUnref();
	sk_free(fBuffer);
}

bool SkSumShader::setContext(const SkBitmap& device,
							 const SkPaint& paint,
							 const SkMatrix& matrix)
{
	if (!this->INHERITED::setContext(device, paint, matrix))
		return false;

	if (fBuffer == nil)
		fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * sizeof(SkPMColor));
	else
		fBuffer = (SkPMColor*)sk_realloc_throw(fBuffer, device.width() * sizeof(SkPMColor));
	return true;
}

void SkSumShader::shadeSpan(int x, int y, SkPMColor dst[], int count)
{
	SkShader*	s0 = this->getShader0();
	SkShader*	s1 = this->getShader1();
	SkASSERT(s0 || s1);

	if (s0 == nil)
		s1->shadeSpan(x, y, dst, count);
	else
	{
		s0->shadeSpan(x, y, dst, count);
		if (s1)
		{
			SkPMColor* src = fBuffer;
			s1->shadeSpan(x, y, src, count);

			if (fMode)
				fMode->xfer32(dst, src, count, nil);
			else
			{
				unsigned weight = fWeight;
				for (int i = 0; i < count; i++)
					dst[i] = SkBlendARGB32(src[i], dst[i], weight);
			}
		}
	}
}

void SkSumShader::shadeSpanOpaque16(int x, int y, U16 dst[], int count)
{
	SkShader*	s0 = this->getShader0();
	SkShader*	s1 = this->getShader1();
	SkASSERT(s0 || s1);

	if (s0 == nil)
		s1->shadeSpanOpaque16(x, y, dst, count);
	else
	{
		s0->shadeSpanOpaque16(x, y, dst, count);
		if (s1)
		{
			if (fMode)
			{
				SkPMColor* src = fBuffer;
				s1->shadeSpan(x, y, src, count);
				fMode->xfer16(dst, src, count, nil);
			}
			else
			{
				U16* src = (U16*)fBuffer;
				s1->shadeSpanOpaque16(x, y, src, count);

				unsigned scale = SkAlpha255To256(fWeight);
				for (int i = 0; i < count; i++)
					dst[i] = (U16)SkBlendRGB16(src[i], dst[i], scale);
			}
		}
	}
}

void SkSumShader::shadeSpanAlpha(int x, int y, U8 dst[], int count)
{
	SkShader*	s0 = this->getShader0();
	SkShader*	s1 = this->getShader1();
	SkASSERT(s0 || s1);

	if (s0 == nil)
		s1->shadeSpanAlpha(x, y, dst, count);
	else
	{
		s0->shadeSpanAlpha(x, y, dst, count);
		if (s1)
		{
			if (fMode)
			{
				SkPMColor* src = fBuffer;
				s1->shadeSpan(x, y, src, count);
				fMode->xferA8(dst, src, count, nil);
			}
			else
			{
				U8* src = (U8*)fBuffer;
				s1->shadeSpanAlpha(x, y, src, count);

				unsigned scale = SkAlpha255To256(fWeight);
				for (int i = 0; i < count; i++)
					dst[i] = (U8)SkAlphaBlend(src[i], dst[i], scale);
			}
		}
	}
}
#endif
