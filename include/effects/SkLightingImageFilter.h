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
    static SkImageFilter* CreateDistantLitDiffuse(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        SkImageFilter* input = NULL, const CropRect* cropRect = NULL);
    static SkImageFilter* CreatePointLitDiffuse(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        SkImageFilter* input = NULL, const CropRect* cropRect = NULL);
    static SkImageFilter* CreateSpotLitDiffuse(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        SkImageFilter* input = NULL, const CropRect* cropRect = NULL);
    static SkImageFilter* CreateDistantLitSpecular(const SkPoint3& direction,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, SkImageFilter* input = NULL, const CropRect* cropRect = NULL);
    static SkImageFilter* CreatePointLitSpecular(const SkPoint3& location,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, SkImageFilter* input = NULL, const CropRect* cropRect = NULL);
    static SkImageFilter* CreateSpotLitSpecular(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, SkImageFilter* input = NULL, const CropRect* cropRect = NULL);
    ~SkLightingImageFilter();

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

protected:
    SkLightingImageFilter(SkImageFilterLight* light,
                          SkScalar surfaceScale,
                          SkImageFilter* input,
                          const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;
    const SkImageFilterLight* light() const { return fLight.get(); }
    SkScalar surfaceScale() const { return fSurfaceScale; }
    bool affectsTransparentBlack() const override { return true; }

private:
    typedef SkImageFilter INHERITED;
    SkAutoTUnref<SkImageFilterLight> fLight;
    SkScalar fSurfaceScale;
};

#endif
