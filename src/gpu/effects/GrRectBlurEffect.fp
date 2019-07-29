/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
    #include "include/core/SkScalar.h"
    #include "src/core/SkBlurMask.h"
    #include "src/gpu/GrProxyProvider.h"
    #include "src/gpu/GrShaderCaps.h"
}

in uniform float4 rect;
in float sigma;
in uniform sampler2D blurProfile;

@constructorParams {
    GrSamplerState samplerParams
}

@samplerParams(blurProfile) {
    samplerParams
}

// in OpenGL ES, mediump floats have a minimum range of 2^14. If we have coordinates bigger than
// that, the shader math will end up with infinities and result in the blur effect not working
// correctly. To avoid this, we switch into highp when the coordinates are too big. As 2^14 is the
// minimum range but the actual range can be bigger, we might end up switching to highp sooner than
// strictly necessary, but most devices that have a bigger range for mediump also have mediump being
// exactly the same as highp (e.g. all non-OpenGL ES devices), and thus incur no additional penalty
// for the switch.
layout(key) bool highPrecision = abs(rect.x) > 16000 || abs(rect.y) > 16000 ||
                                 abs(rect.z) > 16000 || abs(rect.w) > 16000 ||
                                 abs(rect.z - rect.x) > 16000 || abs(rect.w - rect.y) > 16000;

layout(when=!highPrecision) uniform half4 proxyRectHalf;
layout(when=highPrecision) uniform float4 proxyRectFloat;
uniform half profileSize;


@class {
    static sk_sp<GrTextureProxy> CreateBlurProfileTexture(GrProxyProvider* proxyProvider,
                                                          float sigma) {
        unsigned int profileSize = SkScalarCeilToInt(6 * sigma);

        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
        builder[0] = profileSize;
        builder.finish();

        sk_sp<GrTextureProxy> blurProfile(proxyProvider->findOrCreateProxyByUniqueKey(
                                                                    key, kTopLeft_GrSurfaceOrigin));
        if (!blurProfile) {
            SkImageInfo ii = SkImageInfo::MakeA8(profileSize, 1);

            SkBitmap bitmap;
            if (!bitmap.tryAllocPixels(ii)) {
                return nullptr;
            }

            SkBlurMask::ComputeBlurProfile(bitmap.getAddr8(0, 0), profileSize, sigma);
            bitmap.setImmutable();

            sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
            if (!image) {
                return nullptr;
            }

            blurProfile = proxyProvider->createTextureProxy(std::move(image), GrRenderable::kNo,
                                                            1, SkBudgeted::kYes,
                                                            SkBackingFit::kExact);
            if (!blurProfile) {
                return nullptr;
            }

            SkASSERT(blurProfile->origin() == kTopLeft_GrSurfaceOrigin);
            proxyProvider->assignUniqueKeyToProxy(key, blurProfile.get());
        }

        return blurProfile;
    }
}

@make {
     static std::unique_ptr<GrFragmentProcessor> Make(GrProxyProvider* proxyProvider,
                                                      const GrShaderCaps& caps,
                                                      const SkRect& rect, float sigma) {
         if (!caps.floatIs32Bits()) {
             // We promote the rect uniform from half to float when it has large values for
             // precision. If we don't have full float then fail.
             if (SkScalarAbs(rect.fLeft) > 16000.f || SkScalarAbs(rect.fTop) > 16000.f ||
                 SkScalarAbs(rect.fRight) > 16000.f || SkScalarAbs(rect.fBottom) > 16000.f ||
                 SkScalarAbs(rect.width()) > 16000.f || SkScalarAbs(rect.height()) > 16000.f) {
                 return nullptr;
             }
         }
         int doubleProfileSize = SkScalarCeilToInt(12*sigma);

         if (doubleProfileSize >= rect.width() || doubleProfileSize >= rect.height()) {
             // if the blur sigma is too large so the gaussian overlaps the whole
             // rect in either direction, fall back to CPU path for now.
             return nullptr;
         }

         sk_sp<GrTextureProxy> blurProfile(CreateBlurProfileTexture(proxyProvider, sigma));
         if (!blurProfile) {
            return nullptr;
         }

         return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(
            rect, sigma, std::move(blurProfile),
            GrSamplerState(GrSamplerState::WrapMode::kClamp, GrSamplerState::Filter::kBilerp)));
     }
}

void main() {
    @if (highPrecision) {
        float2 translatedPos = sk_FragCoord.xy - rect.xy;
        float width = rect.z - rect.x;
        float height = rect.w - rect.y;
        float2 smallDims = float2(width - profileSize, height - profileSize);
        float center = 2 * floor(profileSize / 2 + 0.25) - 1;
        float2 wh = smallDims - float2(center, center);
        half hcoord = half(((abs(translatedPos.x - 0.5 * width) - 0.5 * wh.x)) / profileSize);
        half hlookup = sample(blurProfile, float2(hcoord, 0.5)).a;
        half vcoord = half(((abs(translatedPos.y - 0.5 * height) - 0.5 * wh.y)) / profileSize);
        half vlookup = sample(blurProfile, float2(vcoord, 0.5)).a;
        sk_OutColor = sk_InColor * hlookup * vlookup;
    } else {
        half2 translatedPos = half2(sk_FragCoord.xy - rect.xy);
        half width = half(rect.z - rect.x);
        half height = half(rect.w - rect.y);
        half2 smallDims = half2(width - profileSize, height - profileSize);
        half center = 2 * floor(profileSize / 2 + 0.25) - 1;
        half2 wh = smallDims - half2(center, center);
        half hcoord = ((half(abs(translatedPos.x - 0.5 * width)) - 0.5 * wh.x)) / profileSize;
        half hlookup = sample(blurProfile, float2(hcoord, 0.5)).a;
        half vcoord = ((half(abs(translatedPos.y - 0.5 * height)) - 0.5 * wh.y)) / profileSize;
        half vlookup = sample(blurProfile, float2(vcoord, 0.5)).a;
        sk_OutColor = sk_InColor * hlookup * vlookup;
    }
}

@setData(pdman) {
    pdman.set1f(profileSize, SkScalarCeilToScalar(6 * sigma));
}

@optimizationFlags { kCompatibleWithCoverageAsAlpha_OptimizationFlag }

@test(data) {
    float sigma = data->fRandom->nextRangeF(3,8);
    float width = data->fRandom->nextRangeF(200,300);
    float height = data->fRandom->nextRangeF(200,300);
    return GrRectBlurEffect::Make(data->proxyProvider(), *data->caps()->shaderCaps(),
                                  SkRect::MakeWH(width, height), sigma);
}
