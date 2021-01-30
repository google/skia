/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/shaders/SkShaderBase.h"

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

protected:
#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(const GrFPArgs&) const override;
    bool onHasFragmentProcessor() const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkShaderMF)

    sk_sp<SkShader> fShader;

    SkShaderMF(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkShaderMaskFilter;

    using INHERITED = SkMaskFilter;
};

sk_sp<SkFlattenable> SkShaderMF::CreateProc(SkReadBuffer& buffer) {
    return SkShaderMaskFilter::Make(buffer.readShader());
}

void SkShaderMF::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
}

static void rect_memcpy(void* dst, size_t dstRB, const void* src, size_t srcRB,
                        size_t copyBytes, int rows) {
    for (int i = 0; i < rows; ++i) {
        memcpy(dst, src, copyBytes);
        dst = (char*)dst + dstRB;
        src = (const char*)src + srcRB;
    }
}

bool SkShaderMF::filterMask(SkMask* dst, const SkMask& src, const SkMatrix& ctm,
                            SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

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

    // Allocate and initialize dst image with a copy of the src image
    dst->fImage = SkMask::AllocImage(size);
    rect_memcpy(dst->fImage, dst->fRowBytes, src.fImage, src.fRowBytes,
                src.fBounds.width() * sizeof(uint8_t), src.fBounds.height());

    // Now we have a dst-mask, just need to setup a canvas and draw into it
    SkBitmap bitmap;
    if (!bitmap.installMaskPixels(*dst)) {
        return false;
    }

    SkPaint paint;
    paint.setShader(fShader);
    // this blendmode is the trick: we only draw the shader where the mask is
    paint.setBlendMode(SkBlendMode::kSrcIn);

    SkCanvas canvas(bitmap);
    canvas.translate(-SkIntToScalar(dst->fBounds.fLeft), -SkIntToScalar(dst->fBounds.fTop));
    canvas.concat(ctm);
    canvas.drawPaint(paint);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"

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

void SkShaderMaskFilter::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkShaderMF); }
