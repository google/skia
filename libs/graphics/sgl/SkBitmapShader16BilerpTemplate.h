

class BILERP_BITMAP16_SHADER_CLASS : public HasSpan16_Sampler_BitmapShader {
public:
	BILERP_BITMAP16_SHADER_CLASS(const SkBitmap& src, bool transferOwnershipOfPixels)
		: HasSpan16_Sampler_BitmapShader(src, transferOwnershipOfPixels, SkPaint::kBilinear_FilterType,
                                         SkShader::kClamp_TileMode, SkShader::kClamp_TileMode)
	{
	}

	virtual void shadeSpanOpaque16(int x, int y, U16 dstC[], int count)
	{
		SkASSERT(count > 0);
		SkASSERT(this->getInverseClass() != kPerspective_MatrixClass);
		SkASSERT(this->getPaintAlpha() == 0xFF);

		const SkMatrix&	inv = this->getTotalInverse();
		const SkBitmap&	srcBitmap = this->getSrcBitmap();
		unsigned		srcMaxX = srcBitmap.width() - 1;
		unsigned		srcMaxY = srcBitmap.height() - 1;
		unsigned		srcRB = srcBitmap.rowBytes();
		SkFixed			fx, fy, dx, dy;

		// now init fx, fy, dx, dy
		{
			SkPoint	srcPt;
			this->getInverseMapPtProc()(inv, SkIntToScalar(x), SkIntToScalar(y), &srcPt);

			fx = SkScalarToFixed(srcPt.fX);
			fy = SkScalarToFixed(srcPt.fY);

			if (this->getInverseClass() == kFixedStepInX_MatrixClass)
				(void)inv.fixedStepInX(SkIntToScalar(y), &dx, &dy);
			else
			{
				dx = SkScalarToFixed(inv.getScaleX());
				dy = SkScalarToFixed(inv.getSkewY());
			}
		}

		BILERP_BITMAP16_SHADER_PREAMBLE(srcBitmap);

		const U32* coeff_table = gBilerpPackedCoeff;
		const BILERP_BITMAP16_SHADER_TYPE* srcPixels = (const BILERP_BITMAP16_SHADER_TYPE*)srcBitmap.getPixels();
		U16CPU rbMask = gRBMask_Bilerp_BitmapShader;

		if (dy == 0)
		{
			fy = SkClampMax(fy, srcMaxY << 16);
			coeff_table += SK_BILERP_GET_BITS(fy) << 2;	// jump the table to the correct section (so we can just use fx to index it)

			unsigned y = fy >> 16;
			SkASSERT((int)y >= 0 && y <= srcMaxY);
			// pre-bias srcPixels since y won't change
			srcPixels = (const BILERP_BITMAP16_SHADER_TYPE*)((const char*)srcPixels + y * srcRB);
			// now make y the step from one row to the next
			y = srcRB;
			if (y == srcMaxY)
				y = 0;

			do {
				unsigned fx_clamped = SkClampMax(fx, srcMaxX << 16);
				unsigned x = fx_clamped >> 16;
				SkASSERT((int)x >= 0 && x <= srcMaxX);

				const BILERP_BITMAP16_SHADER_TYPE *p00, *p01, *p10, *p11;

				p00 = p01 = srcPixels + x;
				if (x < srcMaxX)
					p01 += 1;
				p10 = (const BILERP_BITMAP16_SHADER_TYPE*)((const char*)p00 + y);
				p11 = (const BILERP_BITMAP16_SHADER_TYPE*)((const char*)p01 + y);

				*dstC++ = SkToU16(sk_bilerp16(	BILERP_BITMAP16_SHADER_PIXEL(*p00),
												BILERP_BITMAP16_SHADER_PIXEL(*p01),
												BILERP_BITMAP16_SHADER_PIXEL(*p10),
												BILERP_BITMAP16_SHADER_PIXEL(*p11),
												coeff_table[SK_BILERP_GET_BITS(fx_clamped)],
												rbMask));

				fx += dx;
			} while (--count != 0);
		}
		else
		{
			do {
				unsigned x = SkClampMax(fx, srcMaxX << 16) >> 16;
				unsigned y = SkClampMax(fy, srcMaxY << 16) >> 16;

				SkASSERT((int)x >= 0 && x <= srcMaxX);
				SkASSERT((int)y >= 0 && y <= srcMaxY);

				const BILERP_BITMAP16_SHADER_TYPE *p00, *p01, *p10, *p11;

				p00 = p01 = ((const BILERP_BITMAP16_SHADER_TYPE*)((const char*)srcPixels + y * srcRB)) + x;
				if (x < srcMaxX)
					p01 += 1;
				p10 = p00;
				p11 = p01;
				if (y < srcMaxY)
				{
					p10 = (const BILERP_BITMAP16_SHADER_TYPE*)((const char*)p10 + srcRB);
					p11 = (const BILERP_BITMAP16_SHADER_TYPE*)((const char*)p11 + srcRB);
				}

				*dstC++ = SkToU16(sk_bilerp16(	BILERP_BITMAP16_SHADER_PIXEL(*p00),
												BILERP_BITMAP16_SHADER_PIXEL(*p01),
												BILERP_BITMAP16_SHADER_PIXEL(*p10),
												BILERP_BITMAP16_SHADER_PIXEL(*p11),
												sk_find_bilerp_coeff(coeff_table, fx, fy),
												rbMask));

				fx += dx;
				fy += dy;
			} while (--count != 0);
		}

		BILERP_BITMAP16_SHADER_POSTAMBLE(srcBitmap);
	}
};

#undef BILERP_BITMAP16_SHADER_CLASS
#undef BILERP_BITMAP16_SHADER_TYPE
#undef BILERP_BITMAP16_SHADER_PREAMBLE
#undef BILERP_BITMAP16_SHADER_PIXEL
#undef BILERP_BITMAP16_SHADER_POSTAMBLE
