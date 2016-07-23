/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLightingImageFilter_DEFINED
#define SkLightingImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkColor.h"


class SkImageFilterLight;
struct SkPoint3;

class SK_API SkLightingImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> MakeDistantLitDiffuse(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakePointLitDiffuse(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeSpotLitDiffuse(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeDistantLitSpecular(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakePointLitSpecular(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeSpotLitSpecular(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect* cropRect = nullptr);
    ~SkLightingImageFilter() override;

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
     static SkImageFilter* CreateDistantLitDiffuse(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        SkImageFilter* input = NULL, const CropRect* cropRect = NULL) {
        return MakeDistantLitDiffuse(direction, lightColor, surfaceScale, kd,
                                     sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
    static SkImageFilter* CreatePointLitDiffuse(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        SkImageFilter* input = NULL, const CropRect* cropRect = NULL) {
        return MakePointLitDiffuse(location, lightColor, surfaceScale, kd,
                                   sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
    static SkImageFilter* CreateSpotLitDiffuse(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        SkImageFilter* input = NULL, const CropRect* cropRect = NULL) {
        return MakeSpotLitDiffuse(location, target, specularExponent, cutoffAngle,
                                  lightColor, surfaceScale, kd,
                                  sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
    static SkImageFilter* CreateDistantLitSpecular(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, SkImageFilter* input = NULL, const CropRect* cropRect = NULL) {
        return MakeDistantLitSpecular(direction, lightColor, surfaceScale, ks, shininess,
                                      sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
    static SkImageFilter* CreatePointLitSpecular(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, SkImageFilter* input = NULL, const CropRect* cropRect = NULL) {
        return MakePointLitSpecular(location, lightColor, surfaceScale, ks, shininess,
                                    sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
    static SkImageFilter* CreateSpotLitSpecular(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, SkImageFilter* input = NULL, const CropRect* cropRect = NULL) {
        return MakeSpotLitSpecular(location, target, specularExponent, cutoffAngle,
                                   lightColor, surfaceScale, ks, shininess,
                                   sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
#endif

protected:
    SkLightingImageFilter(sk_sp<SkImageFilterLight> light,
                          SkScalar surfaceScale,
                          sk_sp<SkImageFilter> input,
                          const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;
    const SkImageFilterLight* light() const { return fLight.get(); }
    SkScalar surfaceScale() const { return fSurfaceScale; }
    bool affectsTransparentBlack() const override { return true; }

private:
    sk_sp<SkImageFilterLight> fLight;
    SkScalar fSurfaceScale;

    typedef SkImageFilter INHERITED;
};

#endif
