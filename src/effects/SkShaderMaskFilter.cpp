/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkMaskFilterBase.h"
#include "SkNx.h"
#include "SkReadBuffer.h"
#include "SkShaderMaskFilter.h"
#include "SkShaderBase.h"
#include "SkString.h"

class SkShaderMF : public SkMaskFilterBase {
public:
    SkShaderMF(sk_sp<SkShader> shader) : fShader(std::move(shader)) {}

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }

    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void computeFastBounds(const SkRect& src, SkRect* dst) const override {
        *dst = src;
    }

    bool asABlur(BlurRec*) const override { return false; }

    void toString(SkString* str) const override;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkShaderMF)

protected:
#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(const GrFPArgs&) const override;
    bool onHasFragmentProcessor() const override;
#endif

private:
    sk_sp<SkShader> fShader;

    SkShaderMF(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkShaderMaskFilter;

    typedef SkMaskFilter INHERITED;
};

void SkShaderMF::toString(SkString* str) const {
    str->set("SkShaderMF:");
}

sk_sp<SkFlattenable> SkShaderMF::CreateProc(SkReadBuffer& buffer) {
    return SkShaderMaskFilter::Make(buffer.readShader());
}

void SkShaderMF::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
}

static void modulate_mask(const SkMask& src, const SkBitmap& mask, SkMask* dst) {
    SkASSERT(src.fBounds == dst->fBounds);
    SkASSERT(mask.rowBytes() == mask.info().minRowBytes());
    SkASSERT(dst->fRowBytes ==
             SkImageInfo::MakeA8(src.fBounds.width(), src.fBounds.height()).minRowBytes());

    auto s = src.fImage;
    auto d = dst->fImage;
    auto m = mask.getAddr32(0, 0);

    for (int y = 0; y < src.fBounds.height(); ++y) {
        auto w = src.fBounds.width();
        for (; w >= 8; w -= 8) {
            const auto s16 = SkNx_cast<uint16_t>(Sk8b::Load(s));
            const auto m16 = Sk8h({
                                static_cast<uint16_t>(SkGetPackedA32(m[0])),
                                static_cast<uint16_t>(SkGetPackedA32(m[1])),
                                static_cast<uint16_t>(SkGetPackedA32(m[2])),
                                static_cast<uint16_t>(SkGetPackedA32(m[3])),
                                static_cast<uint16_t>(SkGetPackedA32(m[4])),
                                static_cast<uint16_t>(SkGetPackedA32(m[5])),
                                static_cast<uint16_t>(SkGetPackedA32(m[6])),
                                static_cast<uint16_t>(SkGetPackedA32(m[7]))
                              }),
                    prod16 = s16 * m16 + 128,
                     dst16 = (prod16 + (prod16 >> 8)) >> 8;
            SkNx_cast<uint8_t>(dst16).store(d);

            s += 8;
            d += 8;
            m += 8;
        }

        for (; w > 0; --w) {
            *d++ = SkMulDiv255Round(*s++, SkGetPackedA32(*m++));
        }

        s += src.fRowBytes - src.fBounds.width();
    }
}

bool SkShaderMF::filterMask(SkMask* dst, const SkMask& src, const SkMatrix& ctm,
                            SkIPoint* margin) const {
    SkASSERT(src.fFormat == SkMask::kA8_Format);

    if (margin) {
        margin->set(0, 0);
    }
    dst->fBounds   = src.fBounds;
    dst->fRowBytes = src.fBounds.width();   // need alignment?
    dst->fFormat   = SkMask::kA8_Format;

    if (src.fImage == nullptr) {
        dst->fImage = nullptr;
        return true;
    }
    size_t size = dst->computeImageSize();
    if (0 == size) {
        return false;   // too big to allocate, abort
    }

    dst->fImage = SkMask::AllocImage(size);

    SkBitmap dstBM;
    if (!dstBM.installMaskPixels(*dst)) {
        return false;
    }

    // We draw the mask into an N32 temp buffer to avoid slow A8 rasterization.
    SkBitmap mask;
    mask.allocN32Pixels(dst->fBounds.width(), dst->fBounds.height());
    mask.eraseColor(0);

    SkPaint paint;
    paint.setShader(fShader);
    paint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);

    SkCanvas canvas(mask);
    canvas.translate(-SkIntToScalar(dst->fBounds.fLeft), -SkIntToScalar(dst->fBounds.fTop));
    canvas.concat(ctm);
    canvas.drawPaint(paint);

    // Modulate src x mask => dst
    modulate_mask(src, mask, dst);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkShaderMF::onAsFragmentProcessor(const GrFPArgs& args) const {
    return GrFragmentProcessor::MulInputByChildAlpha(as_SB(fShader)->asFragmentProcessor(args));
}

bool SkShaderMF::onHasFragmentProcessor() const {
    return true;
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SkShaderMaskFilter::Make(sk_sp<SkShader> shader) {
    return shader ? sk_sp<SkMaskFilter>(new SkShaderMF(std::move(shader))) : nullptr;
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkShaderMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkShaderMF)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
