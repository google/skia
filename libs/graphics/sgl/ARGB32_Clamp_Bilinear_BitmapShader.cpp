
class ARGB32_Clamp_Bilinear_BitmapShader : public SkBitmapShader {
public:
    ARGB32_Clamp_Bilinear_BitmapShader(const SkBitmap& src, bool transferOwnership)
        : SkBitmapShader(src, transferOwnership, SkPaint::kBilinear_FilterType,
                         SkShader::kClamp_TileMode, SkShader::kClamp_TileMode)
    {}

    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count);
};

SkPMColor sample_bilerp(SkFixed fx, SkFixed fy, int srcMaxX, int srcMaxY,
                        const SkPMColor* srcPixels, int srcRB, const SkFilterPtrProc* proc_table);
SkPMColor sample_bilerp(SkFixed fx, SkFixed fy, int srcMaxX, int srcMaxY,
                        const SkPMColor* srcPixels, int srcRB, const SkFilterPtrProc* proc_table)
{
    int x = fx >> 16;
    int y = fy >> 16;
    
    if (x < 0) x = 0;
    if (x > srcMaxX) x = srcMaxX;
    if (y < 0) y = 0;
    if (y > srcMaxY) y = srcMaxY;

    SkASSERT(x <= srcMaxX);
    SkASSERT(y <= srcMaxY);

    const SkPMColor *p00, *p01, *p10, *p11;

    p00 = p01 = ((const SkPMColor*)((const char*)srcPixels + y * srcRB)) + x;
    if (x < srcMaxX)
        p01 += 1;
    p10 = p00;
    p11 = p01;
    if (y < srcMaxY)
    {
        p10 = (const SkPMColor*)((const char*)p10 + srcRB);
        p11 = (const SkPMColor*)((const char*)p11 + srcRB);
    }

    SkFilterPtrProc proc = SkGetBilinearFilterPtrProc(proc_table, fx, fy);
    return proc(p00, p01, p10, p11);
}

static inline SkPMColor sample_bilerp(int x, int srcMaxX, const SkPMColor* srcPixels, int srcRB, SkFilterPtrProc proc)
{
    const SkPMColor *p00, *p01, *p10, *p11;
    
    // sure wish I could pull this check out of the loop that calls this function
    if ((unsigned)x < (unsigned)srcMaxX)
    {
        p00 = srcPixels + x;
        p01 = p00 + 1;
        p10 = (const SkPMColor*)((const char*)p00 + srcRB);
        p11 = p10 + 1;
        return proc(p00, p01, p10, p11);
    }

    if (x < 0) x = 0;
    if (x > srcMaxX) x = srcMaxX;

    p00 = p01 = srcPixels + x;
    p10 = p11 = (const SkPMColor*)((const char*)p00 + srcRB);
    if (x < srcMaxX)
    {
        p01 += 1;
        p11 += 1;
    }
    return proc(p00, p01, p10, p11);
}

void ARGB32_Clamp_Bilinear_BitmapShader::shadeSpan(int x, int y, SkPMColor dstC[], int count)
{
    SkASSERT(count > 0);
    
    unsigned srcScale = SkAlpha255To256(this->getPaintAlpha());

    const SkMatrix& inv = this->getTotalInverse();
    const SkBitmap& srcBitmap = this->getSrcBitmap();
    unsigned        srcMaxX = srcBitmap.width() - 1;
    unsigned        srcMaxY = srcBitmap.height() - 1;
    unsigned        srcRB = srcBitmap.rowBytes();

    const SkFilterPtrProc* proc_table = SkGetBilinearFilterPtrProcTable();
    const SkPMColor* srcPixels = (const SkPMColor*)srcBitmap.getPixels();

    if (this->getInverseClass() == kPerspective_MatrixClass)
    {
        SkPerspIter   iter(inv, SkIntToScalar(x), SkIntToScalar(y), count);

        if (256 == srcScale)
        {
            while ((count = iter.next()) != 0)
            {
                const SkFixed* srcXY = iter.getXY();
                while (--count >= 0)
                {
                    SkFixed fx = *srcXY++;
                    SkFixed fy = *srcXY++;
                    *dstC++ = sample_bilerp(fx, fy, srcMaxX, srcMaxY, srcPixels, srcRB, proc_table);
                }
            }
        }
        else    // scale by srcScale
        {
            while ((count = iter.next()) != 0)
            {
                const SkFixed* srcXY = iter.getXY();
                while (--count >= 0)
                {
                    SkFixed fx = *srcXY++;
                    SkFixed fy = *srcXY++;
                    SkPMColor c = sample_bilerp(fx, fy, srcMaxX, srcMaxY, srcPixels, srcRB, proc_table);
                    *dstC++ = SkAlphaMulQ(c, srcScale);
                }
            }
        }
    }
    else    // linear case
    {
        SkFixed fx, fy, dx, dy;

        // now init fx, fy, dx, dy
        {
            SkPoint srcPt;
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

        if (dy == 0 && (unsigned)(fy >> 16) < srcMaxY)
        {
            srcPixels = (const SkPMColor*)((const char*)srcPixels + (fy >> 16) * srcRB);
            proc_table = SkGetBilinearFilterPtrProcYTable(proc_table, fy);
            if (256 == srcScale)
            {
                do {
                    *dstC++ = sample_bilerp(fx >> 16, srcMaxX, srcPixels, srcRB, SkGetBilinearFilterPtrXProc(proc_table, fx));
                    fx += dx;
                } while (--count != 0);
            }
            else
            {
                do {
                    SkPMColor c = sample_bilerp(fx >> 16, srcMaxX, srcPixels, srcRB, SkGetBilinearFilterPtrXProc(proc_table, fx));
                    *dstC++ = SkAlphaMulQ(c, srcScale);
                    fx += dx;
                } while (--count != 0);
            }
        }
        else    // dy is != 0
        {
            if (256 == srcScale)
            {
                do {
                    *dstC++ = sample_bilerp(fx, fy, srcMaxX, srcMaxY, srcPixels, srcRB, proc_table);
                    fx += dx;
                    fy += dy;
                } while (--count != 0);
            }
            else
            {
                do {
                    SkPMColor c = sample_bilerp(fx, fy, srcMaxX, srcMaxY, srcPixels, srcRB, proc_table);
                    *dstC++ = SkAlphaMulQ(c, srcScale);
                    fx += dx;
                    fy += dy;
                } while (--count != 0);
            }
        }
    }
}

