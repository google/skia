/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/AnalyticRRectBlurMask.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cmath>

namespace skgpu::graphite {

std::optional<AnalyticRRectBlurMask> AnalyticRRectBlurMask::Make(
                                            Recorder* recorder,
                                            const Transform& localToDeviceTransform,
                                            SkV2 localSigma,
                                            const SkRRect& srcRRect) {
    UniqueKey key;
    {
        static const UniqueKey::Domain kCdfLutDomain = UniqueKey::GenerateDomain();
        UniqueKey::Builder builder(&key, kCdfLutDomain, 0, "AnalyticRRectBlurCdfLut");
    }
    sk_sp<TextureProxy> cdfLut = recorder->priv().proxyCache()->findOrCreateCachedProxy(
            recorder, key, nullptr,
            [](const void*) -> SkBitmap {
                constexpr int kCdfLutSize = 256;
                constexpr float kCdfLutRange = 4.0f;
                SkBitmap bmp;
                if (!bmp.tryAllocPixels(SkImageInfo::Make(kCdfLutSize, 1,
                                                          kR8_unorm_SkColorType,
                                                          kOpaque_SkAlphaType))) {
                    return bmp;
                }

                // Generate our gaussian CDF values from x in [-kCdfLutRange, kCdfLutRange] to be
                // sampled with a half pixel offset.
                uint8_t* pixels = static_cast<uint8_t*>(bmp.getPixels());
                for (int i = 0; i < kCdfLutSize; i++) {
                    float x = -kCdfLutRange + 2.0f * kCdfLutRange * (i + 0.5f) / kCdfLutSize;
                    float cdf = 0.5f + 0.5f * std::erf(x);
                    pixels[i] = static_cast<uint8_t>(std::round(cdf * 255.0f));
                }
                bmp.setImmutable();
                return bmp;
            });

    if (!cdfLut) {
        return std::nullopt;
    }

    return AnalyticRRectBlurMask(srcRRect,
                                 localSigma,
                                 std::move(cdfLut));
}

}  // namespace skgpu::graphite
