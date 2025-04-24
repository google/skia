/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/SkShaderMaskFilterImpl.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "src/core/SkMask.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>
#include <cstring>
#include <utility>

class SkMatrix;

sk_sp<SkFlattenable> SkShaderMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    return SkShaderMaskFilter::Make(buffer.readShader());
}

void SkShaderMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
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

bool SkShaderMaskFilterImpl::filterMask(SkMaskBuilder* dst, const SkMask& src, const SkMatrix& ctm,
                                        SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    if (margin) {
        margin->set(0, 0);
    }
    dst->bounds()   = src.fBounds;
    dst->rowBytes() = src.fBounds.width();   // need alignment?
    dst->format()   = SkMask::kA8_Format;

    if (src.fImage == nullptr) {
        dst->image() = nullptr;
        return true;
    }
    size_t size = dst->computeImageSize();
    if (0 == size) {
        return false;   // too big to allocate, abort
    }

    // Allocate and initialize dst image with a copy of the src image
    dst->image() = SkMaskBuilder::AllocImage(size);
    rect_memcpy(dst->image(), dst->fRowBytes, src.fImage, src.fRowBytes,
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

std::pair<sk_sp<SkImageFilter>, bool> SkShaderMaskFilterImpl::asImageFilter(const SkMatrix&,
                                                                            const SkPaint&) const {
    sk_sp<SkImageFilter> filter =  SkImageFilters::Shader(fShader);
    return {SkImageFilters::Blend(SkBlendMode::kDstIn, std::move(filter), nullptr), false};
}

sk_sp<SkMaskFilter> SkShaderMaskFilter::Make(sk_sp<SkShader> shader) {
    return shader ? sk_sp<SkMaskFilter>(new SkShaderMaskFilterImpl(std::move(shader))) : nullptr;
}

void SkShaderMaskFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkShaderMaskFilterImpl);
    // Previous name
    SkFlattenable::Register("SkShaderMF", SkShaderMaskFilterImpl::CreateProc);
}
