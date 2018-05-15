/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDFMaskFilter.h"
#include "../../src/gpu/text/GrSDFMaskFilter.h"
#include "SkMaskFilterBase.h"
#include "SkPerlinNoiseShader.h"
#include "SkReadBuffer.h"
#include "SkSafeMath.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

#if 0
static float scurve(float x) {
    SkASSERT(x >= 0 && x <= 1);
    return x*x*(3 - 2*x);
}
#endif

static uint8_t convert_sdf_to_a8(uint8_t distanceByte, float window, float offset) {
    float distance = ((int)distanceByte - 128) * (1.0f/32);
    float radius = window * 0.5f;
    distance += offset;
    if (distance > radius) {
        return 0xFF;
    }
    if (distance < -radius) {
        return 0;
    }
    distance += radius;
    distance *= 255 / window;
    return distance + 0.5;
}

static void ConvertDFToA8(const SkMask& src, int dx, int dy, const SkMask& dst,
                          float window, float offset) {
    SkASSERT(dst.fFormat == SkMask::kA8_Format);
    SkASSERT(src.fBounds.contains(dst.fBounds));

    const uint8_t* srcP = src.getAddr8(src.fBounds.fLeft + dx, src.fBounds.fTop + dy);
    uint8_t* dstP = dst.getAddr8(dst.fBounds.fLeft, dst.fBounds.fTop);
    for (int y = 0; y < dst.fBounds.height(); ++y) {
        for (int x = 0; x < dst.fBounds.width(); ++x) {
            dstP[x] = convert_sdf_to_a8(srcP[x], window, offset);
        }
        srcP += src.fRowBytes;
        dstP += dst.fRowBytes;
    }
}

class SkDFMaskFilterImpl : public SkMaskFilterBase {
public:
    SkDFMaskFilterImpl(const SkDFMaskFilter::Rec&);

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }

    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void computeFastBounds(const SkRect&, SkRect*) const override;

    void toString(SkString* str) const override;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDFMaskFilterImpl)

private:
    SkDFMaskFilter::Rec fRec;
    sk_sp<SkMaskFilter> fSDF;
    sk_sp<SkShader> fPN;

    typedef SkMaskFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkSurface.h"

static sk_sp<SkImage> make_perlin_image(const SkIRect& r, const SkMatrix& ctm, sk_sp<SkShader> sh) {
    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());
    SkCanvas* canvas = s->getCanvas();
    canvas->concat(ctm);
    SkPaint paint;
    paint.setShader(sh);
    canvas->drawPaint(paint);
    return s->makeImageSnapshot();
}

SkDFMaskFilterImpl::SkDFMaskFilterImpl(const SkDFMaskFilter::Rec& rec) {
    fRec = rec;
    fPN = SkPerlinNoiseShader::MakeImprovedNoise(rec.fBaseX, rec.fBaseY, rec.fOctaves, rec.fZ);
    fSDF = GrSDFMaskFilter::Make();
}

static float extract_noise(SkPMColor c) {
    return SkGetPackedR32(c) / 255.0f;
}

bool SkDFMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                     const SkMatrix& matrix, SkIPoint* margin) const {
    *dst = src;
    dst->fFormat = SkMask::kA8_Format;

    if (src.fImage == nullptr) {
        return true;
    }

    size_t size = src.computeImageSize();
    dst->fImage = SkMask::AllocImage(size);
    memcpy(dst->fImage, src.fImage, size);

    auto img = make_perlin_image(src.fBounds, matrix, fPN);
    SkPixmap pm;
    if (!img->peekPixels(&pm)) {
        return false;
    }

    uint8_t* d = dst->fImage;
    for (int y = 0; y < src.fBounds.height(); ++y) {
        for (int x = 0; x < src.fBounds.width(); ++x) {
            float noise = extract_noise(*pm.addr32(x, y));
            d[x] = (int)(d[x] * noise);
        }
        d += dst->fRowBytes;
    }
    return true;

    if (0) {
    SkMask sdf;
    if (!as_MFB(fSDF)->filterMask(&sdf, src, matrix, margin)) {
        return false;
    }

    *dst = src;
    dst->fImage = SkMask::AllocImage(src.computeImageSize());
    ConvertDFToA8(sdf,
                  src.fBounds.fLeft - sdf.fBounds.fLeft, src.fBounds.fTop - sdf.fBounds.fTop,
                  *dst, fRec.fBaseX, fRec.fBaseY);
    }
    return true;
}

void SkDFMaskFilterImpl::computeFastBounds(const SkRect& src, SkRect* dst) const {
    *dst = src;
}

sk_sp<SkFlattenable> SkDFMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    return SkDFMaskFilter::Make({1, 1, 1, 1});
}

void SkDFMaskFilterImpl::toString(SkString* str) const {
    str->append("SkDFMaskFilterImpl: ()");
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SkDFMaskFilter::Make(const Rec& rec) {
    return sk_sp<SkMaskFilter>(new SkDFMaskFilterImpl(rec));
}
