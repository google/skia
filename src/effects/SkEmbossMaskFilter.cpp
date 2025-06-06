/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/SkEmbossMaskFilter.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/SkEmbossMask.h"

#if defined(SK_SUPPORT_LEGACY_EMBOSSMASKFILTER)
#include "include/effects/SkBlurMaskFilter.h"
#endif

#include <cstring>

sk_sp<SkMaskFilter> SkEmbossMaskFilter::Make(SkScalar blurSigma, const Light& light) {
    if (!SkIsFinite(blurSigma) || blurSigma <= 0) {
        return nullptr;
    }

    SkPoint3 lightDir{light.fDirection[0], light.fDirection[1], light.fDirection[2]};
    if (!lightDir.normalize()) {
        return nullptr;
    }
    Light newLight = light;
    newLight.fDirection[0] = lightDir.x();
    newLight.fDirection[1] = lightDir.y();
    newLight.fDirection[2] = lightDir.z();

    return sk_sp<SkMaskFilter>(new SkEmbossMaskFilter(blurSigma, newLight));
}

#ifdef SK_SUPPORT_LEGACY_EMBOSSMASKFILTER
sk_sp<SkMaskFilter> SkBlurMaskFilter::MakeEmboss(SkScalar blurSigma, const SkScalar direction[3],
                                                 SkScalar ambient, SkScalar specular) {
    if (direction == nullptr) {
        return nullptr;
    }

    SkEmbossMaskFilter::Light   light;

    memcpy(light.fDirection, direction, sizeof(light.fDirection));
    // ambient should be 0...1 as a scalar
    light.fAmbient = SkUnitScalarClampToByte(ambient);
    // specular should be 0..15.99 as a scalar
    static const SkScalar kSpecularMultiplier = SkIntToScalar(255) / 16;
    light.fSpecular = static_cast<U8CPU>(SkTPin(specular, 0.0f, 16.0f) * kSpecularMultiplier + 0.5);

    return SkEmbossMaskFilter::Make(blurSigma, light);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkEmbossMaskFilter::SkEmbossMaskFilter(SkScalar blurSigma, const Light& light)
    : fLight(light), fBlurSigma(blurSigma)
{
    SkASSERT(fBlurSigma > 0);
    SkASSERT(SkIsFinite(fLight.fDirection, 3));
}

SkMask::Format SkEmbossMaskFilter::getFormat() const {
    return SkMask::k3D_Format;
}

bool SkEmbossMaskFilter::filterMask(SkMaskBuilder* dst, const SkMask& src,
                                    const SkMatrix& matrix, SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    SkScalar sigma = matrix.mapRadius(fBlurSigma);

    if (!SkBlurMask::BoxBlur(dst, src, sigma, kInner_SkBlurStyle)) {
        return false;
    }

    dst->format() = SkMask::k3D_Format;
    if (margin) {
        margin->set(SkScalarCeilToInt(3*sigma), SkScalarCeilToInt(3*sigma));
    }

    if (src.fImage == nullptr) {
        return true;
    }

    // create a larger buffer for the other two channels (should force fBlur to do this for us)

    {
        uint8_t* alphaPlane = dst->image();
        size_t totalSize = dst->computeTotalImageSize();
        if (totalSize == 0) {
            return false;  // too big to allocate, abort
        }
        size_t planeSize = dst->computeImageSize();
        SkASSERT(planeSize != 0);  // if totalSize didn't overflow, this can't either
        dst->image() = SkMaskBuilder::AllocImage(totalSize);
        memcpy(dst->image(), alphaPlane, planeSize);
        SkMaskBuilder::FreeImage(alphaPlane);
    }

    // run the light direction through the matrix...
    Light   light = fLight;
    matrix.mapVectors({(SkVector*)(void*)light.fDirection, 1},
                      {(SkVector*)(void*)fLight.fDirection, 1});

    // now restore the length of the XY component
    // cast to SkVector so we can call setLength (this double cast silences alias warnings)
    SkVector* vec = (SkVector*)(void*)light.fDirection;
    vec->setLength(light.fDirection[0],
                   light.fDirection[1],
                   SkPoint::Length(fLight.fDirection[0], fLight.fDirection[1]));

    SkEmbossMask::Emboss(dst, light);

    // restore original alpha
    memcpy(dst->image(), src.fImage, src.computeImageSize());

    return true;
}

sk_sp<SkFlattenable> SkEmbossMaskFilter::CreateProc(SkReadBuffer& buffer) {
    Light light;
    if (buffer.readByteArray(&light, sizeof(Light))) {
        light.fPad = 0; // for the font-cache lookup to be clean
        const SkScalar sigma = buffer.readScalar();
        return Make(sigma, light);
    }
    return nullptr;
}

void SkEmbossMaskFilter::flatten(SkWriteBuffer& buffer) const {
    Light tmpLight = fLight;
    tmpLight.fPad = 0;    // for the font-cache lookup to be clean
    buffer.writeByteArray(&tmpLight, sizeof(tmpLight));
    buffer.writeScalar(fBlurSigma);
}

// This image filter uses coverage masks for operations but affects shading properties
// of a draw using the paint parameter, and returning true to indicate appliesShading.
std::pair<sk_sp<SkImageFilter>, bool> SkEmbossMaskFilter::asImageFilter(
        const SkMatrix& ctm, const SkPaint& paint) const {
    // Here the original bitmap we are operating on (nullptr for imageFilters) should be
    // our coverage mask, as a white RGBA8 image where the alpha corresponds to the coverage.
    sk_sp<SkImageFilter> coverageBlurred = SkImageFilters::Blur(fBlurSigma, fBlurSigma, nullptr);

    // The paint should have the original shading properties that we want to apply.
    sk_sp<SkShader> srcShader = SkShaders::Color(paint.getColor4f(), /*cs=*/nullptr);
    if (paint.getShader()) {
        srcShader = SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(), std::move(srcShader));
    }
    srcShader = srcShader->makeWithColorFilter(paint.refColorFilter());
    sk_sp<SkImageFilter> srcColor = SkImageFilters::Shader(
        std::move(srcShader), paint.isDither() ? SkImageFilters::Dither::kYes
                                               : SkImageFilters::Dither::kNo);

    // ka = fLight.fAmbient
    float ambientf = fLight.fAmbient / 255.f;
    SkColor4f ambientColor = {ambientf, ambientf, ambientf, 1};
    sk_sp<SkImageFilter> ambient = SkImageFilters::Shader(SkShaders::Color(ambientColor, nullptr));

    // L = fLight.fDirection
    SkPoint3 lightDirection = SkPoint3::Make(fLight.fDirection[0],
                                             fLight.fDirection[1],
                                             fLight.fDirection[2]);


    // Amount to scale the alpha by to calculate N, set this way to mimic the legacy
    // emboss mask filter implementation.
    // Made negative to match functionality of legacy emboss mask filter which calculates
    // the normal "into" the monitor, away from the user, whereas all other documentation
    // points normals towards negative directions (towards user).
    const float surfaceScale = -255.f/ 32.f;

    // diffuse = kd * dot(L, N)
    sk_sp<SkImageFilter> diffuseCF = SkImageFilters::DistantLitDiffuse(lightDirection,
                                                                       SK_ColorWHITE,
                                                                       surfaceScale,
                                                                       1,
                                                                       coverageBlurred);
    // mul = ka + diffuse
    sk_sp<SkImageFilter> ambientdiffuse = SkImageFilters::Blend(SkBlendMode::kPlus,
                                                                diffuseCF,
                                                                ambient);
    // ambientdiffuseColor = srcColor * mul
    sk_sp<SkImageFilter> ambientdiffuseBlend = SkImageFilters::Blend(
        SkBlendMode::kModulate, srcColor, ambientdiffuse);

    // fLight.fSpecular is in a fixed 4.4 format.
    // This uses the legacy implementation for emboss which calculates the specular
    // lighting differently than standard specular functions.
    //
    // specular = ks * pow((2 * (L * N) - L_z) * L_z), shininess)
    float shininess = ((fLight.fSpecular >> 4) + 1);

    sk_sp<SkImageFilter> specular = LegacySpecular(lightDirection,
                                                   SK_ColorWHITE,
                                                   surfaceScale,
                                                   1,
                                                   shininess,
                                                   coverageBlurred);

    // dstColor = ambientdiffuseColor + specular
    //          = srcColor * (ka + kd * dot(L, N)) + ks * pow((2 * (L * N) - L_z) * L_z), shininess)
    sk_sp<SkImageFilter> finalFilter = SkImageFilters::Blend(SkBlendMode::kPlus,
                                                             ambientdiffuseBlend,
                                                             specular);
    // Mask by original coverage mask, it remains unchanged.
    // Return true to indicate applies shading.
    return {SkImageFilters::Blend(SkBlendMode::kDstIn, finalFilter, nullptr), true};
}
