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

// As far as I know, SkModeColorFilter is the only color filter that holds a color.
sk_sp<SkColorFilter> SkColorSpaceXformer::apply(const SkColorFilter* colorFilter) {
    SkColor color;
    SkBlendMode mode;
    if (colorFilter->asColorMode(&color, &mode)) {
        return SkColorFilter::MakeModeFilter(this->apply(color), mode);
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

// TODO: Is this introspection going to be enough, or do we need a new SkShader method?
sk_sp<SkShader> SkColorSpaceXformer::apply(const SkShader* shader) {
    SkColor color;
    if (shader->isConstant() && shader->asLuminanceColor(&color)) {
        return SkShader::MakeColorShader(this->apply(color))
                ->makeWithLocalMatrix(shader->getLocalMatrix());
    }

    SkShader::TileMode xy[2];
    SkMatrix local;
    if (auto img = shader->isAImage(&local, xy)) {
        return this->apply(img)->makeShader(xy[0], xy[1], &local);
    }

    SkShader::ComposeRec compose;
    if (shader->asACompose(&compose)) {
        auto A = this->apply(compose.fShaderA),
             B = this->apply(compose.fShaderB);
        if (A && B) {
            return SkShader::MakeComposeShader(std::move(A), std::move(B), compose.fBlendMode)
                    ->makeWithLocalMatrix(shader->getLocalMatrix());
        }
    }

    SkShader::GradientInfo gradient;
    sk_bzero(&gradient, sizeof(gradient));
    if (auto type = shader->asAGradient(&gradient)) {
        SkSTArray<8, SkColor>  colors(gradient.fColorCount);
        SkSTArray<8, SkScalar>    pos(gradient.fColorCount);

        gradient.fColors       = colors.begin();
        gradient.fColorOffsets =    pos.begin();
        shader->asAGradient(&gradient);

        SkSTArray<8, SkColor> xformed(gradient.fColorCount);
        this->apply(xformed.begin(), gradient.fColors, gradient.fColorCount);

        switch (type) {
            case SkShader::kNone_GradientType:
            case SkShader::kColor_GradientType:
                SkASSERT(false);  // Should be unreachable.
                break;

            case SkShader::kLinear_GradientType:
                return SkGradientShader::MakeLinear(gradient.fPoint,
                                                    xformed.begin(),
                                                    gradient.fColorOffsets,
                                                    gradient.fColorCount,
                                                    gradient.fTileMode,
                                                    gradient.fGradientFlags,
                                                    &shader->getLocalMatrix());
            case SkShader::kRadial_GradientType:
                return SkGradientShader::MakeRadial(gradient.fPoint[0],
                                                    gradient.fRadius[0],
                                                    xformed.begin(),
                                                    gradient.fColorOffsets,
                                                    gradient.fColorCount,
                                                    gradient.fTileMode,
                                                    gradient.fGradientFlags,
                                                    &shader->getLocalMatrix());
            case SkShader::kSweep_GradientType:
                return SkGradientShader::MakeSweep(gradient.fPoint[0].fX,
                                                   gradient.fPoint[0].fY,
                                                   xformed.begin(),
                                                   gradient.fColorOffsets,
                                                   gradient.fColorCount,
                                                   gradient.fGradientFlags,
                                                   &shader->getLocalMatrix());
            case SkShader::kConical_GradientType:
                return SkGradientShader::MakeTwoPointConical(gradient.fPoint[0],
                                                             gradient.fRadius[0],
                                                             gradient.fPoint[1],
                                                             gradient.fRadius[1],
                                                             xformed.begin(),
                                                             gradient.fColorOffsets,
                                                             gradient.fColorCount,
                                                             gradient.fTileMode,
                                                             gradient.fGradientFlags,
                                                             &shader->getLocalMatrix());
        }
    }

    return sk_ref_sp(const_cast<SkShader*>(shader));
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
        if (auto replacement = this->apply(shader)) {
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
        get_dst()->setDrawLooper(looper->makeColorSpace(this));
    }

    if (auto imageFilter = src.getImageFilter()) {
        get_dst()->setImageFilter(imageFilter->makeColorSpace(this));
    }

    return *result;
}

const SkPaint* SkColorSpaceXformer::apply(const SkPaint* src) {
    return src ? &this->apply(*src) : nullptr;
}
