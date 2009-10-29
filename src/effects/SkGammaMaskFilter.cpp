#include "SkGammaMaskFilter.h"

SkGammaMaskFilter::SkGammaMaskFilter() {
    for (int i = 0; i < 256; i++) {
        fTable[i] = i;
    }
}

SkGammaMaskFilter::SkGammaMaskFilter(SkScalar gamma) {
    this->setGamma(gamma);
}

SkGammaMaskFilter::SkGammaMaskFilter(const uint8_t table[256]) {
    this->setGammaTable(table);
}

SkGammaMaskFilter::~SkGammaMaskFilter() {}

void SkGammaMaskFilter::setGamma(SkScalar gamma) {
    float x = 0;
    const float dx = 1 / 255.0f;
    for (int i = 0; i < 256; i++) {
        fTable[i] = SkPin32(SkScalarRound(powf(x, gamma) * 255), 0, 255);
        x += dx;
    }
}

void SkGammaMaskFilter::setGammaTable(const uint8_t table[256]) {
    memcpy(fTable, table, 256);
}

SkMask::Format SkGammaMaskFilter::getFormat() {
    return SkMask::kA8_Format;
}

bool SkGammaMaskFilter::filterMask(SkMask* dst, const SkMask& src,
                                 const SkMatrix&, SkIPoint* margin) {
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    dst->fBounds = src.fBounds;
    dst->fRowBytes = SkAlign4(dst->fBounds.width());
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = NULL;
    
    if (src.fImage) {
        dst->fImage = SkMask::AllocImage(dst->computeImageSize());
        
        const uint8_t* srcP = src.fImage;
        uint8_t* dstP = dst->fImage;
        const uint8_t* table = fTable;
        int dstWidth = dst->fBounds.width();
        int extraZeros = dst->fRowBytes - dstWidth;
        
        for (int y = dst->fBounds.height() - 1; y >= 0; --y) {
            for (int x = dstWidth - 1; x >= 0; --x) {
                dstP[x] = table[srcP[x]];
            }
            srcP += src.fRowBytes;
            // we can't just inc dstP by rowbytes, because if it has any
            // padding between its width and its rowbytes, we need to zero those
            // so that the bitters can read those safely if that is faster for
            // them
            dstP += dstWidth;
            for (int i = extraZeros - 1; i >= 0; --i) {
                *dstP++ = 0;
            }
        }
    }

    if (margin) {
        margin->set(0, 0);
    }
    return true;
}

void SkGammaMaskFilter::flatten(SkFlattenableWriteBuffer& wb) {
    this->INHERITED::flatten(wb);
    wb.writePad(fTable, 256);
}

SkGammaMaskFilter::SkGammaMaskFilter(SkFlattenableReadBuffer& rb)
        : INHERITED(rb) {
    rb.read(fTable, 256);
}

SkFlattenable* SkGammaMaskFilter::Factory(SkFlattenableReadBuffer& rb) {
    return SkNEW_ARGS(SkGammaMaskFilter, (rb));
}

SkFlattenable::Factory SkGammaMaskFilter::getFactory() {
    return SkGammaMaskFilter::Factory;
}

