/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLightingImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"

// FIXME:  Eventually, this should be implemented properly, and put in
// SkScalar.h.
#define SkScalarPow(x, y) SkFloatToScalar(powf(SkScalarToFloat(x), SkScalarToFloat(y)))
namespace {

const SkScalar gOneThird = SkScalarInvert(SkIntToScalar(3));
const SkScalar gTwoThirds = SkScalarDiv(SkIntToScalar(2), SkIntToScalar(3));
const SkScalar gOneHalf = SkFloatToScalar(0.5f);
const SkScalar gOneQuarter = SkFloatToScalar(0.25f);

// Shift matrix components to the left, as we advance pixels to the right.
inline void shiftMatrixLeft(int m[9]) {
    m[0] = m[1];
    m[3] = m[4];
    m[6] = m[7];
    m[1] = m[2];
    m[4] = m[5];
    m[7] = m[8];
}

class DiffuseLightingType {
public:
    DiffuseLightingType(SkScalar kd)
        : fKD(kd) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight, const SkPoint3& lightColor) const {
        SkScalar colorScale = SkScalarMul(fKD, normal.dot(surfaceTolight));
        colorScale = SkScalarClampMax(colorScale, SK_Scalar1);
        SkPoint3 color(lightColor * colorScale);
        return SkPackARGB32(255,
                            SkScalarFloorToInt(color.fX),
                            SkScalarFloorToInt(color.fY),
                            SkScalarFloorToInt(color.fZ));
    }
private:
    SkScalar fKD;
};

class SpecularLightingType {
public:
    SpecularLightingType(SkScalar ks, SkScalar shininess)
        : fKS(ks), fShininess(shininess) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight, const SkPoint3& lightColor) const {
        SkPoint3 halfDir(surfaceTolight);
        halfDir.fZ += SK_Scalar1;        // eye position is always (0, 0, 1)
        halfDir.normalize();
        SkScalar colorScale = SkScalarMul(fKS,
            SkScalarPow(normal.dot(halfDir), fShininess));
        colorScale = SkScalarClampMax(colorScale, SK_Scalar1);
        SkPoint3 color(lightColor * colorScale);
        return SkPackARGB32(SkScalarFloorToInt(color.maxComponent()),
                            SkScalarFloorToInt(color.fX),
                            SkScalarFloorToInt(color.fY),
                            SkScalarFloorToInt(color.fZ));
    }
private:
    SkScalar fKS;
    SkScalar fShininess;
};

inline SkScalar sobel(int a, int b, int c, int d, int e, int f, SkScalar scale) {
    return SkScalarMul(SkIntToScalar(-a + b - 2 * c + 2 * d -e + f), scale);
}

inline SkPoint3 pointToNormal(SkScalar x, SkScalar y, SkScalar surfaceScale) {
    SkPoint3 vector(SkScalarMul(-x, surfaceScale),
                    SkScalarMul(-y, surfaceScale),
                    SK_Scalar1);
    vector.normalize();
    return vector;
}

inline SkPoint3 topLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(0, 0, m[4], m[5], m[7], m[8], gTwoThirds),
                         sobel(0, 0, m[4], m[7], m[5], m[8], gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 topNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[5], m[6], m[8], gOneThird),
                         sobel(m[3], m[6], m[4], m[7], m[5], m[8], gOneHalf),
                         surfaceScale);
}

inline SkPoint3 topRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[4], m[6], m[7], gTwoThirds),
                         sobel(m[3], m[6], m[4], m[7],    0,    0, gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 leftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5], m[7], m[8], gOneHalf),
                         sobel(   0,    0, m[1], m[7], m[2], m[8], gOneThird),
                         surfaceScale);
}


inline SkPoint3 interiorNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5], m[6], m[8], gOneQuarter),
                         sobel(m[0], m[6], m[1], m[7], m[2], m[8], gOneQuarter),
                         surfaceScale);
}

inline SkPoint3 rightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], m[6], m[7], gOneHalf),
                         sobel(m[0], m[6], m[1], m[7],    0,    0, gOneThird),
                         surfaceScale);
}

inline SkPoint3 bottomLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5],    0,    0, gTwoThirds),
                         sobel(   0,    0, m[1], m[4], m[2], m[5], gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 bottomNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5],    0,    0, gOneThird),
                         sobel(m[0], m[3], m[1], m[4], m[2], m[5], gOneHalf),
                         surfaceScale);
}

inline SkPoint3 bottomRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], 0,  0, gTwoThirds),
                         sobel(m[0], m[3], m[1], m[4], 0,  0, gTwoThirds),
                         surfaceScale);
}

template <class LightingType, class LightType> void lightBitmap(const LightingType& lightingType, const SkLight* light, const SkBitmap& src, SkBitmap* dst, SkScalar surfaceScale) {
    const LightType* l = static_cast<const LightType*>(light);
    int y = 0;
    {
        const SkPMColor* row1 = src.getAddr32(0, 0);
        const SkPMColor* row2 = src.getAddr32(0, 1);
        SkPMColor* dptr = dst->getAddr32(0, 0);
        int m[9];
        int x = 0;
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        m[7] = SkGetPackedA32(*row2++);
        m[8] = SkGetPackedA32(*row2++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topLeftNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        for (x = 1; x < src.width() - 1; ++x)
        {
            shiftMatrixLeft(m);
            m[5] = SkGetPackedA32(*row1++);
            m[8] = SkGetPackedA32(*row2++);
            surfaceToLight = l->surfaceToLight(x, 0, m[4], surfaceScale);
            *dptr++ = lightingType.light(topNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topRightNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
    }

    for (++y; y < src.height() - 1; ++y) {
        const SkPMColor* row0 = src.getAddr32(0, y - 1);
        const SkPMColor* row1 = src.getAddr32(0, y);
        const SkPMColor* row2 = src.getAddr32(0, y + 1);
        SkPMColor* dptr = dst->getAddr32(0, y);
        int m[9];
        int x = 0;
        m[1] = SkGetPackedA32(*row0++);
        m[2] = SkGetPackedA32(*row0++);
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        m[7] = SkGetPackedA32(*row2++);
        m[8] = SkGetPackedA32(*row2++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(leftNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        for (x = 1; x < src.width() - 1; ++x) {
            shiftMatrixLeft(m);
            m[2] = SkGetPackedA32(*row0++);
            m[5] = SkGetPackedA32(*row1++);
            m[8] = SkGetPackedA32(*row2++);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(interiorNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(rightNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
    }

    {
        const SkPMColor* row0 = src.getAddr32(0, src.height() - 2);
        const SkPMColor* row1 = src.getAddr32(0, src.height() - 1);
        int x = 0;
        SkPMColor* dptr = dst->getAddr32(0, src.height() - 1);
        int m[9];
        m[1] = SkGetPackedA32(*row0++);
        m[2] = SkGetPackedA32(*row0++);
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(bottomLeftNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        for (x = 1; x < src.width() - 1; ++x)
        {
            shiftMatrixLeft(m);
            m[2] = SkGetPackedA32(*row0++);
            m[5] = SkGetPackedA32(*row1++);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(bottomNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(bottomRightNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
    }
}

SkPoint3 readPoint3(SkFlattenableReadBuffer& buffer) {
    SkPoint3 point;
    point.fX = buffer.readScalar();
    point.fY = buffer.readScalar();
    point.fZ = buffer.readScalar();
    return point;
};

void writePoint3(const SkPoint3& point, SkFlattenableWriteBuffer& buffer) {
    buffer.writeScalar(point.fX);
    buffer.writeScalar(point.fY);
    buffer.writeScalar(point.fZ);
};

class SkDiffuseLightingImageFilter : public SkLightingImageFilter {
public:
    SkDiffuseLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkScalar kd);
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDiffuseLightingImageFilter)

    SkScalar kd() const { return fKD; }

protected:
    explicit SkDiffuseLightingImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;


private:
    typedef SkLightingImageFilter INHERITED;
    SkScalar fKD;
};

class SkSpecularLightingImageFilter : public SkLightingImageFilter {
public:
    SkSpecularLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkScalar ks, SkScalar shininess);
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSpecularLightingImageFilter)

protected:
    explicit SkSpecularLightingImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;

private:
    typedef SkLightingImageFilter INHERITED;
    SkScalar fKS;
    SkScalar fShininess;
};

};

class SkLight : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkLight)

    enum LightType {
        kDistant_LightType,
        kPoint_LightType,
        kSpot_LightType,
    };
    virtual LightType type() const = 0;
    const SkPoint3& color() const { return fColor; }

protected:
    SkLight(SkColor color)
      : fColor(SkIntToScalar(SkColorGetR(color)),
               SkIntToScalar(SkColorGetG(color)),
               SkIntToScalar(SkColorGetB(color))) {}
    SkLight(SkFlattenableReadBuffer& buffer)
      : INHERITED(buffer) {
        fColor = readPoint3(buffer);
    }
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        INHERITED::flatten(buffer);
        writePoint3(fColor, buffer);
    }

private:
    typedef SkFlattenable INHERITED;
    SkPoint3 fColor;
};

SK_DEFINE_INST_COUNT(SkLight)

class SkDistantLight : public SkLight {
public:
    SkDistantLight(const SkPoint3& direction, SkColor color)
      : INHERITED(color), fDirection(direction) {
    }

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        return fDirection;
    };
    SkPoint3 lightColor(const SkPoint3&) const { return color(); }
    virtual LightType type() const { return kDistant_LightType; }
    SkPoint3 direction() const { return fDirection; }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDistantLight)

protected:
    SkDistantLight(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fDirection = readPoint3(buffer);
    }
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const {
        INHERITED::flatten(buffer);
        writePoint3(fDirection, buffer);
    }

private:
    typedef SkLight INHERITED;
    SkPoint3 fDirection;
};

class SkPointLight : public SkLight {
public:
    SkPointLight(const SkPoint3& location, SkColor color)
     : INHERITED(color), fLocation(location) {}

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        SkPoint3 direction(fLocation.fX - SkIntToScalar(x),
                           fLocation.fY - SkIntToScalar(y),
                           fLocation.fZ - SkScalarMul(SkIntToScalar(z), surfaceScale));
        direction.normalize();
        return direction;
    };
    SkPoint3 lightColor(const SkPoint3&) const { return color(); }
    virtual LightType type() const { return kPoint_LightType; }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPointLight)

protected:
    SkPointLight(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
    }
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const {
        INHERITED::flatten(buffer);
        writePoint3(fLocation, buffer);
    }

private:
    typedef SkLight INHERITED;
    SkPoint3 fLocation;
};

class SkSpotLight : public SkLight {
public:
    SkSpotLight(const SkPoint3& location, const SkPoint3& target, SkScalar specularExponent, SkScalar cutoffAngle, SkColor color)
     : INHERITED(color),
       fLocation(location),
       fTarget(target),
       fSpecularExponent(specularExponent)
    {
       fS = target - location;
       fS.normalize();
       fCosOuterConeAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
       const SkScalar antiAliasThreshold = SkFloatToScalar(0.016f);
       fCosInnerConeAngle = fCosOuterConeAngle + antiAliasThreshold;
       fConeScale = SkScalarInvert(antiAliasThreshold);
    }

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        SkPoint3 direction(fLocation.fX - SkIntToScalar(x),
                           fLocation.fY - SkIntToScalar(y),
                           fLocation.fZ - SkScalarMul(SkIntToScalar(z), surfaceScale));
        direction.normalize();
        return direction;
    };
    SkPoint3 lightColor(const SkPoint3& surfaceToLight) const {
        SkScalar cosAngle = -surfaceToLight.dot(fS);
        if (cosAngle < fCosOuterConeAngle) {
            return SkPoint3(0, 0, 0);
        }
        SkScalar scale = SkScalarPow(cosAngle, fSpecularExponent);
        if (cosAngle < fCosInnerConeAngle) {
            scale = SkScalarMul(scale, cosAngle - fCosOuterConeAngle);
            return color() * SkScalarMul(scale, fConeScale);
        }
        return color() * scale;
    }

    virtual LightType type() const { return kSpot_LightType; }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSpotLight)

protected:
    SkSpotLight(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
        fTarget = readPoint3(buffer);
        fSpecularExponent = buffer.readScalar();
        fCosOuterConeAngle = buffer.readScalar();
        fCosInnerConeAngle = buffer.readScalar();
        fConeScale = buffer.readScalar();
        fS = readPoint3(buffer);
    }
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const {
        INHERITED::flatten(buffer);
        writePoint3(fLocation, buffer);
        writePoint3(fTarget, buffer);
        buffer.writeScalar(fSpecularExponent);
        buffer.writeScalar(fCosOuterConeAngle);
        buffer.writeScalar(fCosInnerConeAngle);
        buffer.writeScalar(fConeScale);
        writePoint3(fS, buffer);
    }

private:
    typedef SkLight INHERITED;
    SkPoint3 fLocation;
    SkPoint3 fTarget;
    SkScalar fSpecularExponent;
    SkScalar fCosOuterConeAngle;
    SkScalar fCosInnerConeAngle;
    SkScalar fConeScale;
    SkPoint3 fS;
};

SkLightingImageFilter::SkLightingImageFilter(SkLight* light, SkScalar surfaceScale)
  : fLight(light),
    fSurfaceScale(SkScalarDiv(surfaceScale, SkIntToScalar(255)))
{
    SkASSERT(fLight);
    // our caller knows that we take ownership of the light, so we don't
    // need to call ref() here.
}

SkImageFilter* SkLightingImageFilter::CreateDistantLitDiffuse(
    const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale,
    SkScalar kd) {
    return new SkDiffuseLightingImageFilter(
        new SkDistantLight(direction, lightColor), surfaceScale, kd);
}

SkImageFilter* SkLightingImageFilter::CreatePointLitDiffuse(
    const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale,
    SkScalar kd) {
    return new SkDiffuseLightingImageFilter(
        new SkPointLight(location, lightColor), surfaceScale, kd);
}

SkImageFilter* SkLightingImageFilter::CreateSpotLitDiffuse(
    const SkPoint3& location, const SkPoint3& target,
    SkScalar specularExponent, SkScalar cutoffAngle,
    SkColor lightColor, SkScalar surfaceScale, SkScalar kd) {
    return new SkDiffuseLightingImageFilter(
        new SkSpotLight(location, target, specularExponent, cutoffAngle, lightColor),
        surfaceScale, kd);
}

SkImageFilter* SkLightingImageFilter::CreateDistantLitSpecular(
    const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale,
    SkScalar ks, SkScalar shininess) {
    return new SkSpecularLightingImageFilter(
        new SkDistantLight(direction, lightColor), surfaceScale, ks, shininess);
}

SkImageFilter* SkLightingImageFilter::CreatePointLitSpecular(
    const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale,
    SkScalar ks, SkScalar shininess) {
    return new SkSpecularLightingImageFilter(
        new SkPointLight(location, lightColor), surfaceScale, ks, shininess);
}

SkImageFilter* SkLightingImageFilter::CreateSpotLitSpecular(
    const SkPoint3& location, const SkPoint3& target,
    SkScalar specularExponent, SkScalar cutoffAngle,
    SkColor lightColor, SkScalar surfaceScale,
    SkScalar ks, SkScalar shininess) {
    return new SkSpecularLightingImageFilter(
        new SkSpotLight(location, target, specularExponent, cutoffAngle, lightColor),
        surfaceScale, ks, shininess);
}

SkLightingImageFilter::~SkLightingImageFilter() {
    fLight->unref();
}

SkLightingImageFilter::SkLightingImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer)
{
    fLight = (SkLight*)buffer.readFlattenable();
    fSurfaceScale = buffer.readScalar();
}

void SkLightingImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fLight);
    buffer.writeScalar(fSurfaceScale);
}

SkDiffuseLightingImageFilter::SkDiffuseLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkScalar kd)
  : SkLightingImageFilter(light, surfaceScale),
    fKD(kd)
{
}

SkDiffuseLightingImageFilter::SkDiffuseLightingImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer)
{
    fKD = buffer.readScalar();
}

void SkDiffuseLightingImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKD);
}

bool SkDiffuseLightingImageFilter::onFilterImage(Proxy*,
                                                 const SkBitmap& src,
                                                 const SkMatrix&,
                                                 SkBitmap* dst,
                                                 SkIPoint*) {
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }
    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }
    if (src.width() < 2 || src.height() < 2) {
        return false;
    }
    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();

    DiffuseLightingType lightingType(fKD);
    switch (light()->type()) {
        case SkLight::kDistant_LightType:
            lightBitmap<DiffuseLightingType, SkDistantLight>(lightingType, light(), src, dst, surfaceScale());
            break;
        case SkLight::kPoint_LightType:
            lightBitmap<DiffuseLightingType, SkPointLight>(lightingType, light(), src, dst, surfaceScale());
            break;
        case SkLight::kSpot_LightType:
            lightBitmap<DiffuseLightingType, SkSpotLight>(lightingType, light(), src, dst, surfaceScale());
            break;
    }
    return true;
}

SkSpecularLightingImageFilter::SkSpecularLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkScalar ks, SkScalar shininess)
  : SkLightingImageFilter(light, surfaceScale),
    fKS(ks),
    fShininess(shininess)
{
}

SkSpecularLightingImageFilter::SkSpecularLightingImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer)
{
    fKS = buffer.readScalar();
    fShininess = buffer.readScalar();
}

void SkSpecularLightingImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKS);
    buffer.writeScalar(fShininess);
}

bool SkSpecularLightingImageFilter::onFilterImage(Proxy*,
                                                  const SkBitmap& src,
                                                  const SkMatrix&,
                                                  SkBitmap* dst,
                                                  SkIPoint*) {
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }
    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }
    if (src.width() < 2 || src.height() < 2) {
        return false;
    }
    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();

    SpecularLightingType lightingType(fKS, fShininess);
    switch (light()->type()) {
        case SkLight::kDistant_LightType:
            lightBitmap<SpecularLightingType, SkDistantLight>(lightingType, light(), src, dst, surfaceScale());
            break;
        case SkLight::kPoint_LightType:
            lightBitmap<SpecularLightingType, SkPointLight>(lightingType, light(), src, dst, surfaceScale());
            break;
        case SkLight::kSpot_LightType:
            lightBitmap<SpecularLightingType, SkSpotLight>(lightingType, light(), src, dst, surfaceScale());
            break;
    }
    return true;
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiffuseLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpecularLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDistantLight)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPointLight)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpotLight)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
