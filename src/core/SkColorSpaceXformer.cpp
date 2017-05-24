/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkColorSpaceXformer.h"
#include "SkColorSpaceXform_Base.h"
#include "SkDrawLooper.h"
#include "SkGradientShader.h"
#include "SkImage_Base.h"
#include "SkImageFilter.h"
#include "SkImagePriv.h"
#include "SkMakeUnique.h"

std::unique_ptr<SkColorSpaceXformer> SkColorSpaceXformer::Make(sk_sp<SkColorSpace> dst) {
    std::unique_ptr<SkColorSpaceXform> fromSRGB = SkColorSpaceXform_Base::New(
            SkColorSpace::MakeSRGB().get(), dst.get(), SkTransferFunctionBehavior::kIgnore);
    if (!fromSRGB) {
        return nullptr;
    }

    auto xformer = std::unique_ptr<SkColorSpaceXformer>(new SkColorSpaceXformer());
    xformer->fDst      = std::move(dst);
    xformer->fFromSRGB = std::move(fromSRGB);
    return xformer;
}

sk_sp<SkImage> SkColorSpaceXformer::apply(const SkImage* src) {
    return src->makeColorSpace(fDst, SkTransferFunctionBehavior::kIgnore);
}

sk_sp<SkImage> SkColorSpaceXformer::apply(const SkBitmap& src) {
    sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(src, kNever_SkCopyPixelsMode);
    if (!image) {
        return nullptr;
    }

    sk_sp<SkImage> xformed = image->makeColorSpace(fDst, SkTransferFunctionBehavior::kIgnore);
    // We want to be sure we don't let the kNever_SkCopyPixelsMode image escape this stack frame.
    SkASSERT(xformed != image);
    return xformed;
}

sk_sp<SkColorFilter> SkColorSpaceXformer::apply(const SkColorFilter* colorFilter) {
    return colorFilter->makeColorSpace(this);
}

sk_sp<SkImageFilter> SkColorSpaceXformer::apply(const SkImageFilter* imageFilter) {
    return imageFilter->makeColorSpace(this);
}

sk_sp<SkShader> SkColorSpaceXformer::apply(const SkShader* shader) {
    return shader->makeColorSpace(this);
}

void SkColorSpaceXformer::apply(SkColor* xformed, const SkColor* srgb, int n) {
    SkAssertResult(fFromSRGB->apply(SkColorSpaceXform::kBGRA_8888_ColorFormat, xformed,
                                    SkColorSpaceXform::kBGRA_8888_ColorFormat, srgb,
                                    n, kUnpremul_SkAlphaType));
}

SkColor SkColorSpaceXformer::apply(SkColor srgb) {
    SkColor xformed;
    this->apply(&xformed, &srgb, 1);
    return xformed;
}

SkPaint SkColorSpaceXformer::apply(const SkPaint& src) {
    SkPaint dst = src;

    // All SkColorSpaces have the same black point.
    if (src.getColor() & 0xffffff) {
        dst.setColor(this->apply(src.getColor()));
    }

    if (auto shader = src.getShader()) {
        dst.setShader(this->apply(shader));
    }

    if (auto cf = src.getColorFilter()) {
        dst.setColorFilter(this->apply(cf));
    }

    if (auto looper = src.getDrawLooper()) {
        dst.setDrawLooper(looper->makeColorSpace(this));
    }

    if (auto imageFilter = src.getImageFilter()) {
        dst.setImageFilter(this->apply(imageFilter));
    }

    return dst;
}
