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

// Currently, SkModeColorFilter is the only color filter that holds a color.  And
// SkComposeColorFilter is the only color filter that holds another color filter.  If this
// changes, this function will need updating.
sk_sp<SkColorFilter> SkColorSpaceXformer::apply(const SkColorFilter* colorFilter) {
    SkColor color;
    SkBlendMode mode;
    if (colorFilter->asColorMode(&color, &mode)) {
        return SkColorFilter::MakeModeFilter(this->apply(color), mode);
    }

    SkColorFilter* outer;
    SkColorFilter* inner;
    if (colorFilter->asACompose(&outer, &inner)) {
        return SkColorFilter::MakeComposeFilter(this->apply(outer), this->apply(inner));
    }

    return sk_ref_sp(const_cast<SkColorFilter*>(colorFilter));
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

const SkPaint& SkColorSpaceXformer::apply(const SkPaint& src) {
    const SkPaint* result = &src;
    auto get_dst = [&] {
        if (result == &src) {
            fDstPaint = src;
            result = &fDstPaint;
        }
        return &fDstPaint;
    };

    // All SkColorSpaces have the same black point.
    if (src.getColor() & 0xffffff) {
        get_dst()->setColor(this->apply(src.getColor()));
    }

    if (auto shader = src.getShader()) {
        auto replacement = shader->makeColorSpace(this);
        if (replacement.get() != shader) {
            get_dst()->setShader(std::move(replacement));
        }
    }

    if (auto cf = src.getColorFilter()) {
        auto replacement = this->apply(cf);
        if (replacement.get() != cf) {
            get_dst()->setColorFilter(std::move(replacement));
        }
    }

    if (auto looper = src.getDrawLooper()) {
        auto replacement = looper->makeColorSpace(this);
        if (replacement.get() != looper) {
            get_dst()->setDrawLooper(std::move(replacement));
        }
    }

    if (auto imageFilter = src.getImageFilter()) {
        auto replacement = imageFilter->makeColorSpace(this);
        if (replacement.get() != imageFilter) {
            get_dst()->setImageFilter(std::move(replacement));
        }
    }

    return *result;
}

const SkPaint* SkColorSpaceXformer::apply(const SkPaint* src) {
    return src ? &this->apply(*src) : nullptr;
}
