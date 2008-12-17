#include "SkKernel33MaskFilter.h"
#include "SkColorPriv.h"

SkMask::Format SkKernel33ProcMaskFilter::getFormat()
{
    return SkMask::kA8_Format;
}

bool SkKernel33ProcMaskFilter::filterMask(SkMask* dst, const SkMask& src, const SkMatrix&, SkIPoint* margin)
{
    // margin???
    dst->fImage = NULL;
    dst->fBounds = src.fBounds;
    dst->fBounds.inset(-1, -1);
    dst->fFormat = SkMask::kA8_Format;
    
    if (NULL == src.fImage)
        return true;
    
    dst->fRowBytes = dst->fBounds.width();
    size_t size = dst->computeImageSize();
    dst->fImage = SkMask::AllocImage(size);
    
    const int h = src.fBounds.height();
    const int w = src.fBounds.width();
    const int srcRB = src.fRowBytes;
    const uint8_t* srcImage = src.fImage;
    uint8_t* dstImage = dst->fImage;

    uint8_t* srcRows[3];
    uint8_t storage[3][3];
    
    srcRows[0] = storage[0];
    srcRows[1] = storage[1];
    srcRows[2] = storage[2];

    unsigned scale = fPercent256;
    
    for (int y = -1; y <= h; y++)
    {
        uint8_t* dstRow = dstImage;
        for (int x = -1; x <= w; x++)
        {
            memset(storage, 0, sizeof(storage));
            uint8_t* storagePtr = &storage[0][0];

            for (int ky = y - 1; ky <= y + 1; ky++)
            {
                const uint8_t* srcRow = srcImage + ky * srcRB;  // may be out-of-range
                for (int kx = x - 1; kx <= x + 1; kx++)
                {
                    if ((unsigned)ky < (unsigned)h && (unsigned)kx < (unsigned)w)
                        *storagePtr = srcRow[kx];
                    storagePtr++;
                }
            }            
            int value = this->computeValue(srcRows);
            
            if (scale < 256)
                value = SkAlphaBlend(value, srcRows[1][1], scale);
            *dstRow++ = SkToU8(value);
        }
        dstImage += dst->fRowBytes;
    }
    return true;
}

void SkKernel33ProcMaskFilter::flatten(SkFlattenableWriteBuffer& wb)
{
    this->INHERITED::flatten(wb);
    wb.write32(fPercent256);
}

SkKernel33ProcMaskFilter::SkKernel33ProcMaskFilter(SkFlattenableReadBuffer& rb)
    : SkMaskFilter(rb)
{
    fPercent256 = rb.readS32();
}

///////////////////////////////////////////////////////////////////////////////

uint8_t SkKernel33MaskFilter::computeValue(uint8_t* const* srcRows)
{
    int value = 0;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            value += fKernel[i][j] * srcRows[i][j];
    
    value >>= fShift;

    if (value < 0)
        value = 0;
    else if (value > 255)
        value = 255;
    return (uint8_t)value;
}

void SkKernel33MaskFilter::flatten(SkFlattenableWriteBuffer& wb)
{
    this->INHERITED::flatten(wb);
    wb.writeMul4(fKernel, 9 * sizeof(int));
    wb.write32(fShift);
}

SkFlattenable::Factory SkKernel33MaskFilter::getFactory()
{
    return Create;
}

SkFlattenable* SkKernel33MaskFilter::Create(SkFlattenableReadBuffer& rb)
{
    return new SkKernel33MaskFilter(rb);
}

SkKernel33MaskFilter::SkKernel33MaskFilter(SkFlattenableReadBuffer& rb)
    : SkKernel33ProcMaskFilter(rb)
{
    rb.read(fKernel, 9 * sizeof(int));
    fShift = rb.readS32();
}

