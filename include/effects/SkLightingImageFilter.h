/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLightingImageFilter_DEFINED
#define SkLightingImageFilter_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"

struct SkPoint3;

class SK_API SkLightingImageFilter {
public:
    static sk_sp<SkImageFilter> MakeDistantLitDiffuse(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const SkImageFilter::CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakePointLitDiffuse(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const SkImageFilter::CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeSpotLitDiffuse(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const SkImageFilter::CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeDistantLitSpecular(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks, SkScalar shininess,
        sk_sp<SkImageFilter> input, const SkImageFilter::CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakePointLitSpecular(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks, SkScalar shininess,
        sk_sp<SkImageFilter> input, const SkImageFilter::CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeSpotLitSpecular(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks, SkScalar shininess,
        sk_sp<SkImageFilter> input, const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkLightingImageFilter() = delete;
};

#endif
