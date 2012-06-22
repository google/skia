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

class SK_API SkPoint3 {
public:
    SkPoint3() {}
    SkPoint3(SkScalar x, SkScalar y, SkScalar z)
      : fX(x), fY(y), fZ(z) {}
    SkScalar dot(const SkPoint3& other) const {
        return SkScalarMul(fX, other.fX)
             + SkScalarMul(fY, other.fY)
             + SkScalarMul(fZ, other.fZ);
    }
    SkScalar maxComponent() const {
        return fX > fY ? (fX > fZ ? fX : fZ) : (fY > fZ ? fY : fZ);
    }
    void normalize() {
        SkScalar scale = SkScalarInvert(SkScalarSqrt(dot(*this)));
        fX = SkScalarMul(fX, scale);
        fY = SkScalarMul(fY, scale);
        fZ = SkScalarMul(fZ, scale);
    }
    SkPoint3 operator*(SkScalar scalar) const {
        return SkPoint3(SkScalarMul(fX, scalar),
                        SkScalarMul(fY, scalar),
                        SkScalarMul(fZ, scalar));
    }
    SkPoint3 operator-(const SkPoint3& other) const {
        return SkPoint3(fX - other.fX, fY - other.fY, fZ - other.fZ);
    }
    SkScalar fX, fY, fZ;
};

class SkLight;

class SK_API SkLightingImageFilter : public SkImageFilter {
public:
    static SkImageFilter* CreateDistantLitDiffuse(const SkPoint3& direction,
        const SkColor& lightColor, SkScalar surfaceScale, SkScalar kd);
    static SkImageFilter* CreatePointLitDiffuse(SkPoint3& location,
        const SkColor& lightColor, SkScalar surfaceScale, SkScalar kd);
    static SkImageFilter* CreateSpotLitDiffuse(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        const SkColor& lightColor, SkScalar surfaceScale, SkScalar kd);
    static SkImageFilter* CreateDistantLitSpecular(const SkPoint3& direction,
        const SkColor& lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess);
    static SkImageFilter* CreatePointLitSpecular(SkPoint3& location,
        const SkColor& lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess);
    static SkImageFilter* CreateSpotLitSpecular(const SkPoint3& location,
        const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle,
        const SkColor& lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess);
    ~SkLightingImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingImageFilter)

protected:
    SkLightingImageFilter(SkLight* light, const SkColor& lightColor,
                          SkScalar surfaceScale);
    explicit SkLightingImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    const SkLight* light() const { return fLight; }
    const SkPoint3& lightColor() const { return fLightColor; }
    SkScalar surfaceScale() const { return fSurfaceScale; }

private:
    typedef SkImageFilter INHERITED;
    SkLight* fLight;
    SkPoint3 fLightColor;
    SkScalar fSurfaceScale;
};

#endif

