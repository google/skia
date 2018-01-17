/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawShadowInfo_DEFINED
#define SkDrawShadowInfo_DEFINED

#include "SkColor.h"
#include "SkPoint.h"
#include "SkPoint3.h"

class SkMatrix;
class SkPath;
struct SkRect;

struct SkDrawShadowRec {
    SkPoint3    fZPlaneParams;
    SkPoint3    fLightPos;
    SkScalar    fLightRadius;
    SkColor     fAmbientColor;
    SkColor     fSpotColor;
    uint32_t    fFlags;
};

namespace SkDrawShadowMetrics {

static constexpr auto kAmbientHeightFactor = 1.0f / 128.0f;
static constexpr auto kAmbientGeomFactor = 64.0f;

inline SkScalar AmbientBlurRadius(SkScalar height) {
    return height*kAmbientHeightFactor*kAmbientGeomFactor;
}

inline SkScalar AmbientRecipAlpha(SkScalar height) {
    return 1.0f + SkTMax(height*kAmbientHeightFactor, 0.0f);
}

inline SkScalar SpotBlurRadius(SkScalar occluderZ, SkScalar lightZ, SkScalar lightRadius) {
    return lightRadius*SkTPin(occluderZ / (lightZ - occluderZ), 0.0f, 0.95f);
}

inline void GetSpotParams(SkScalar occluderZ, SkScalar lightX, SkScalar lightY, SkScalar lightZ,
                          SkScalar lightRadius,
                          SkScalar* blurRadius, SkScalar* scale, SkVector* translate) {
    SkScalar zRatio = SkTPin(occluderZ / (lightZ - occluderZ), 0.0f, 0.95f);
    *blurRadius = lightRadius*zRatio;
    *scale = SkTMax(lightZ / (lightZ - occluderZ), 1.0f);
    *translate = SkVector::Make(-zRatio * lightX, -zRatio * lightY);
}

// get bounds prior to the ctm being applied
void GetLocalBounds(const SkPath&, const SkDrawShadowRec&, const SkMatrix& ctm, SkRect* bounds);

}

#endif
