/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED
#define skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <cmath>
#include <optional>

namespace skgpu::graphite {

class Recorder;
class Transform;

class AnalyticRRectBlurMask {
public:
    AnalyticRRectBlurMask() = delete;

    static std::optional<AnalyticRRectBlurMask> Make(Recorder*,
                                                     const Transform& localToDevice,
                                                     SkV2 localSigma,
                                                     const SkRRect& srcRRect);

    const SkRRect& rrect() const { return fRRect; }
    SkV2 localSigma() const { return fLocalSigma; }
    sk_sp<TextureProxy> refCdfProxy() const { return fCdfLut; }
    Rect bounds() const {
        return Rect(fRRect.getBounds()).makeOutset(skvx::float2{this->drawPadX(),
                                                                this->drawPadY()});
    }
    float drawPadX() const { return std::ceil(3.f * fLocalSigma.x); }
    float drawPadY() const { return std::ceil(3.f * fLocalSigma.y); }
    SkV2 drawPad() const { return {this->drawPadX(), this->drawPadY()}; }

    // True when the rrect is a circle or very close to one, blurred by an isotropic local space
    // sigma. Needed so we can assume radial symmetry of the blur in local space.
    bool isCircleBlur() const;

    // Only valid when isCircleBlur() is true. Returns:
    //   x: the v texture coordinate of the CDF LUT for this circle's sigma / radius ratio, which
    //      lies between two rows that the sampler linearly interpolates.
    //   y: the coverage at the center of the blurred circle, 1 - exp(-radius^2 / (2 * sigma^2)),
    //      which the normalized circle profile is multiplied by.
    SkV2 circleParams() const;

private:
    AnalyticRRectBlurMask(const SkRRect& rrect,
                          SkV2 localSigma,
                          sk_sp<TextureProxy> cdfLut)
            : fRRect(rrect)
            , fLocalSigma(localSigma)
            , fCdfLut(std::move(cdfLut)) {}

    SkRRect fRRect;
    SkV2 fLocalSigma;
    sk_sp<TextureProxy> fCdfLut;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED
