
#ifndef NOFILTER_BITMAP_SHADER_PREAMBLE
	#define NOFILTER_BITMAP_SHADER_PREAMBLE(bitmap, rb)
#endif
#ifndef NOFILTER_BITMAP_SHADER_POSTAMBLE
	#define NOFILTER_BITMAP_SHADER_POSTAMBLE(bitmap)
#endif
#ifndef NOFILTER_BITMAP_SHADER_PREAMBLE16
	#define NOFILTER_BITMAP_SHADER_PREAMBLE16(bitmap, rb)
#endif
#ifndef NOFILTER_BITMAP_SHADER_POSTAMBLE16
	#define NOFILTER_BITMAP_SHADER_POSTAMBLE16(bitmap)
#endif

class NOFILTER_BITMAP_SHADER_CLASS : public HasSpan16_Sampler_BitmapShader {
public:
	NOFILTER_BITMAP_SHADER_CLASS(const SkBitmap& src, bool transferOwnershipOfPixels)
		: HasSpan16_Sampler_BitmapShader(src, transferOwnershipOfPixels, SkPaint::kNo_FilterType,
                                         NOFILTER_BITMAP_SHADER_TILEMODE, NOFILTER_BITMAP_SHADER_TILEMODE)
	{
	}
    
#ifdef NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
    virtual bool setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix)
    {
        if (this->INHERITED::setContext(device, paint, matrix)) {
            this->computeUnitInverse();
            return true;
        }
        return false;
    }
#endif

	virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count)
	{
		if (this->getInverseClass() == kPerspective_MatrixClass)
		{
			this->INHERITED::shadeSpan(x, y, dstC, count);
			return;
		}

		unsigned		scale = SkAlpha255To256(this->getPaintAlpha());
#ifdef NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
        const SkMatrix& inv = this->getUnitInverse();
        SkMatrix::MapPtProc invProc = this->getUnitInverseProc();
#else
		const SkMatrix&	inv = this->getTotalInverse();
        SkMatrix::MapPtProc invProc = this->getInverseMapPtProc();
#endif
		const SkBitmap&	srcBitmap = this->getSrcBitmap();
		unsigned		srcMaxX = srcBitmap.width() - 1;
		unsigned		srcMaxY = srcBitmap.height() - 1;
		unsigned		srcRB = srcBitmap.rowBytes();
		SkFixed			fx, fy, dx, dy;

		// now init fx, fy, dx, dy
		{
			SkPoint	srcPt;
			invProc(inv, SkIntToScalar(x), SkIntToScalar(y), &srcPt);

			fx = SkScalarToFixed(srcPt.fX);
			fy = SkScalarToFixed(srcPt.fY);
#ifndef NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
            fx += SK_Fixed1/2;
            fy += SK_Fixed1/2;
#endif

			if (this->getInverseClass() == kFixedStepInX_MatrixClass)
				(void)inv.fixedStepInX(SkIntToScalar(y), &dx, &dy);
			else
			{
				dx = SkScalarToFixed(inv.getScaleX());
				dy = SkScalarToFixed(inv.getSkewY());
			}
		}

		const NOFILTER_BITMAP_SHADER_TYPE* srcPixels = (const NOFILTER_BITMAP_SHADER_TYPE*)srcBitmap.getPixels();
		NOFILTER_BITMAP_SHADER_PREAMBLE(srcBitmap, srcRB);

#if defined(SK_SUPPORT_MIPMAP) && !defined(NOFILTER_BITMAP_SHADER_USE_UNITINVERSE)
        {   int level = this->getMipLevel() >> 16;
            fx >>= level;
            fy >>= level;
            dx >>= level;
            dy >>= level;
		}
#endif

		if (dy == 0)
		{
			int y_index = NOFILTER_BITMAP_SHADER_TILEPROC(fy, srcMaxY);
//			SkDEBUGF(("fy = %g, srcMaxY = %d, y_index = %d\n", SkFixedToFloat(fy), srcMaxY, y_index));
			srcPixels = (const NOFILTER_BITMAP_SHADER_TYPE*)((const char*)srcPixels + y_index * srcRB);
			if (scale == 256)
				while (--count >= 0)
				{
					unsigned x = NOFILTER_BITMAP_SHADER_TILEPROC(fx, srcMaxX);
					fx += dx;
					*dstC++ = NOFILTER_BITMAP_SHADER_SAMPLE_X(srcPixels, x);
				}
			else
				while (--count >= 0)
				{
					unsigned x = NOFILTER_BITMAP_SHADER_TILEPROC(fx, srcMaxX);
					U32 c = NOFILTER_BITMAP_SHADER_SAMPLE_X(srcPixels, x);
					fx += dx;
					*dstC++ = SkAlphaMulQ(c, scale);
				}
		}
		else	// dy != 0
		{
			if (scale == 256)
				while (--count >= 0)
				{
					unsigned x = NOFILTER_BITMAP_SHADER_TILEPROC(fx, srcMaxX);
					unsigned y = NOFILTER_BITMAP_SHADER_TILEPROC(fy, srcMaxY);
					fx += dx;
					fy += dy;
					*dstC++ = NOFILTER_BITMAP_SHADER_SAMPLE_XY(srcPixels, x, y, srcRB);
				}
			else
				while (--count >= 0)
				{
					unsigned x = NOFILTER_BITMAP_SHADER_TILEPROC(fx, srcMaxX);
					unsigned y = NOFILTER_BITMAP_SHADER_TILEPROC(fy, srcMaxY);
					U32 c = NOFILTER_BITMAP_SHADER_SAMPLE_XY(srcPixels, x, y, srcRB);
					fx += dx;
					fy += dy;
					*dstC++ = SkAlphaMulQ(c, scale);
				}
		}

		NOFILTER_BITMAP_SHADER_POSTAMBLE(srcBitmap);
	}

	virtual void shadeSpanOpaque16(int x, int y, U16 dstC[], int count)
	{
		SkASSERT(count > 0);
		SkASSERT(this->getInverseClass() != kPerspective_MatrixClass);
		SkASSERT(this->getFlags() & SkShader::kHasSpan16_Flag);
		SkASSERT(this->getFlags() & (SkShader::kOpaqueAlpha_Flag | SkShader::kConstAlpha_Flag));

#ifdef NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
        const SkMatrix& inv = this->getUnitInverse();
        SkMatrix::MapPtProc invProc = this->getUnitInverseProc();
#else
		const SkMatrix&	inv = this->getTotalInverse();
        SkMatrix::MapPtProc invProc = this->getInverseMapPtProc();
#endif
		const SkBitmap&	srcBitmap = this->getSrcBitmap();
		unsigned		srcMaxX = srcBitmap.width() - 1;
		unsigned		srcMaxY = srcBitmap.height() - 1;
		unsigned		srcRB = srcBitmap.rowBytes();
		SkFixed			fx, fy, dx, dy;

		// now init fx, fy, dx, dy
		{
			SkPoint	srcPt;
			invProc(inv, SkIntToScalar(x), SkIntToScalar(y), &srcPt);

			fx = SkScalarToFixed(srcPt.fX);
			fy = SkScalarToFixed(srcPt.fY);
#ifndef NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
            fx += SK_Fixed1/2;
            fy += SK_Fixed1/2;
#endif

			if (this->getInverseClass() == kFixedStepInX_MatrixClass)
				(void)inv.fixedStepInX(SkIntToScalar(y), &dx, &dy);
			else
			{
				dx = SkScalarToFixed(inv.getScaleX());
				dy = SkScalarToFixed(inv.getSkewY());
			}
		}

		const NOFILTER_BITMAP_SHADER_TYPE* srcPixels = (const NOFILTER_BITMAP_SHADER_TYPE*)srcBitmap.getPixels();
		NOFILTER_BITMAP_SHADER_PREAMBLE16(srcBitmap, srcRB);

#if defined(SK_SUPPORT_MIPMAP) && !defined(NOFILTER_BITMAP_SHADER_USE_UNITINVERSE)
        {   int level = this->getMipLevel() >> 16;
            fx >>= level;
            fy >>= level;
            dx >>= level;
            dy >>= level;
		}
#endif

		if (dy == 0)
		{
			srcPixels = (const NOFILTER_BITMAP_SHADER_TYPE*)((const char*)srcPixels + NOFILTER_BITMAP_SHADER_TILEPROC(fy, srcMaxY) * srcRB);
			do {
				unsigned x = NOFILTER_BITMAP_SHADER_TILEPROC(fx, srcMaxX);
				fx += dx;
				*dstC++ = NOFILTER_BITMAP_SHADER_SAMPLE_X16(srcPixels, x);
			} while (--count != 0);
		}
		else	// dy != 0
		{
			do {
				int ix = fx >> 16;
				unsigned x = NOFILTER_BITMAP_SHADER_TILEPROC(ix, srcMaxX);
				ix = fy >> 16;
				unsigned y = NOFILTER_BITMAP_SHADER_TILEPROC(ix, srcMaxY);
				fx += dx;
				fy += dy;
				*dstC++ = NOFILTER_BITMAP_SHADER_SAMPLE_XY16(srcPixels, x, y, srcRB);
			} while (--count != 0);
		}

		NOFILTER_BITMAP_SHADER_POSTAMBLE16(srcBitmap);
	}
private:
	typedef HasSpan16_Sampler_BitmapShader INHERITED;
};

#undef NOFILTER_BITMAP_SHADER_CLASS
#undef NOFILTER_BITMAP_SHADER_TYPE
#undef NOFILTER_BITMAP_SHADER_PREAMBLE
#undef NOFILTER_BITMAP_SHADER_POSTAMBLE
#undef NOFILTER_BITMAP_SHADER_SAMPLE_X		//(x)
#undef NOFILTER_BITMAP_SHADER_SAMPLE_XY		//(x, y, rowBytes)
#undef NOFILTER_BITMAP_SHADER_TILEMODE
#undef NOFILTER_BITMAP_SHADER_TILEPROC

#undef NOFILTER_BITMAP_SHADER_PREAMBLE16
#undef NOFILTER_BITMAP_SHADER_POSTAMBLE16
#undef NOFILTER_BITMAP_SHADER_SAMPLE_X16		//(x)
#undef NOFILTER_BITMAP_SHADER_SAMPLE_XY16		//(x, y, rowBytes)

#undef NOFILTER_BITMAP_SHADER_USE_UNITINVERSE
