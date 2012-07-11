/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLightingImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "GrProgramStageFactory.h"
#include "gl/GrGLProgramStage.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrCustomStage.h"

class GrGLDiffuseLightingEffect;
class GrGLSpecularLightingEffect;

// FIXME:  Eventually, this should be implemented properly, and put in
// SkScalar.h.
#define SkScalarPow(x, y) SkFloatToScalar(powf(SkScalarToFloat(x), SkScalarToFloat(y)))
namespace {

const SkScalar gOneThird = SkScalarInvert(SkIntToScalar(3));
const SkScalar gTwoThirds = SkScalarDiv(SkIntToScalar(2), SkIntToScalar(3));
const SkScalar gOneHalf = SkFloatToScalar(0.5f);
const SkScalar gOneQuarter = SkFloatToScalar(0.25f);

void setUniformPoint3(const GrGLInterface* gl, GrGLint location, const SkPoint3& point) {
    float x = SkScalarToFloat(point.fX);
    float y = SkScalarToFloat(point.fY);
    float z = SkScalarToFloat(point.fZ);
    GR_GL_CALL(gl, Uniform3f(location, x, y, z));
}

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

    virtual bool asNewCustomStage(GrCustomStage** stage) const SK_OVERRIDE;
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

    virtual bool asNewCustomStage(GrCustomStage** stage) const SK_OVERRIDE;
    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

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


class GrLightingEffect : public GrCustomStage {
public:
    GrLightingEffect(const SkLight* light, SkScalar surfaceScale);
    virtual ~GrLightingEffect();

    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    const SkLight* light() const { return fLight; }
    SkScalar surfaceScale() const { return fSurfaceScale; }
private:
    typedef GrCustomStage INHERITED;
    const SkLight* fLight;
    SkScalar fSurfaceScale;
};

class GrDiffuseLightingEffect : public GrLightingEffect {
public:
    GrDiffuseLightingEffect(const SkLight* light, SkScalar surfaceScale, SkScalar kd);

    static const char* Name() { return "DiffuseLighting"; }

    typedef GrGLDiffuseLightingEffect GLProgramStage;

    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;
    SkScalar kd() const { return fKD; }
private:
    typedef GrLightingEffect INHERITED;
    SkScalar fKD;
};

class GrSpecularLightingEffect : public GrLightingEffect {
public:
    GrSpecularLightingEffect(const SkLight* light, SkScalar surfaceScale, SkScalar ks, SkScalar shininess);

    static const char* Name() { return "SpecularLighting"; }

    typedef GrGLSpecularLightingEffect GLProgramStage;

    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;
    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

private:
    typedef GrLightingEffect INHERITED;
    SkScalar fKS;
    SkScalar fShininess;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLLight {
public:
    virtual void setupVariables(GrGLShaderBuilder* state, int stage);
    virtual void emitVS(SkString* builder) const {}
    virtual void emitFuncs(SkString* builder) const {}
    virtual void emitSurfaceToLight(SkString* builder, const char* z) const = 0;
    virtual void emitLightColor(SkString* builder, const char *surfaceToLight) const;
    virtual void initUniforms(const GrGLInterface* gl, int programID);
    virtual void setData(const GrGLInterface*, const SkLight* light) const;

private:
    typedef SkRefCnt INHERITED;

protected:
    const GrGLShaderVar* fColorVar;
    int fColorVarLocation;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDistantLight : public GrGLLight {
public:
    virtual void setupVariables(GrGLShaderBuilder* state, int stage) SK_OVERRIDE;
    virtual void initUniforms(const GrGLInterface* gl, int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface* gl, const SkLight* light) const SK_OVERRIDE;
    virtual void emitSurfaceToLight(SkString* builder, const char* z) const SK_OVERRIDE;

private:
    typedef GrGLLight INHERITED;
    const GrGLShaderVar* fDirectionVar;
    int fDirectionLocation;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLPointLight : public GrGLLight {
public:
    virtual void setupVariables(GrGLShaderBuilder* state, int stage);
    virtual void initUniforms(const GrGLInterface* gl, int programID);
    virtual void setData(const GrGLInterface* gl, const SkLight* light) const SK_OVERRIDE;
    virtual void emitVS(SkString* builder) const;
    virtual void emitSurfaceToLight(SkString* builder, const char* z) const SK_OVERRIDE;

private:
    typedef GrGLLight INHERITED;
    SkPoint3 fLocation;
    const GrGLShaderVar* fLocationVar;
    int fLocationLocation;
    const char* fHeightVaryingName;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpotLight : public GrGLLight {
public:
    virtual void setupVariables(GrGLShaderBuilder* state, int stage);
    virtual void initUniforms(const GrGLInterface* gl, int programID);
    virtual void setData(const GrGLInterface* gl, const SkLight* light) const SK_OVERRIDE;
    virtual void emitVS(SkString* builder) const;
    virtual void emitFuncs(SkString* builder) const;
    virtual void emitSurfaceToLight(SkString* builder, const char* z) const SK_OVERRIDE;
    virtual void emitLightColor(SkString* builder, const char *surfaceToLight) const;

private:
    typedef GrGLLight INHERITED;

    const GrGLShaderVar* fLocationVar;
    int                  fLocationLocation;
    const GrGLShaderVar* fExponentVar;
    int                  fExponentLocation;
    const GrGLShaderVar* fCosOuterConeAngleVar;
    int                  fCosOuterConeAngleLocation;
    const GrGLShaderVar* fCosInnerConeAngleVar;
    int                  fCosInnerConeAngleLocation;
    const GrGLShaderVar* fConeScaleVar;
    int                  fConeScaleLocation;
    const GrGLShaderVar* fSVar;
    int                  fSLocation;
    const char*          fHeightVaryingName;
};

};

///////////////////////////////////////////////////////////////////////////////

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
    virtual GrGLLight* createGLLight() const = 0;
    virtual bool isEqual(const SkLight& other) const {
        return fColor == other.fColor;
    }

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

///////////////////////////////////////////////////////////////////////////////

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
    const SkPoint3& direction() const { return fDirection; }
    virtual GrGLLight* createGLLight() const SK_OVERRIDE;

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

///////////////////////////////////////////////////////////////////////////////

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
    const SkPoint3& location() const { return fLocation; }
    virtual GrGLLight* createGLLight() const SK_OVERRIDE {
        return new GrGLPointLight();
    }
    bool isEqual(const SkLight& other) const SK_OVERRIDE {
        if (other.type() != kPoint_LightType) {
            return false;
        }
        const SkPointLight& o = static_cast<const SkPointLight&>(other);
        return INHERITED::isEqual(other) &&
               fLocation == o.fLocation;
    }

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

///////////////////////////////////////////////////////////////////////////////

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
    virtual GrGLLight* createGLLight() const SK_OVERRIDE {
        return new GrGLSpotLight();
    }
    virtual LightType type() const { return kSpot_LightType; }
    const SkPoint3& location() const { return fLocation; }
    const SkPoint3& target() const { return fTarget; }
    SkScalar specularExponent() const { return fSpecularExponent; }
    SkScalar cosInnerConeAngle() const { return fCosInnerConeAngle; }
    SkScalar cosOuterConeAngle() const { return fCosOuterConeAngle; }
    SkScalar coneScale() const { return fConeScale; }
    SkPoint3 s() const { return fS; }

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

    virtual bool isEqual(const SkLight& other) const SK_OVERRIDE {
        if (other.type() != kSpot_LightType) {
            return false;
        }

        const SkSpotLight& o = static_cast<const SkSpotLight&>(other);
        return INHERITED::isEqual(other) &&
               fLocation == o.fLocation &&
               fTarget == o.fTarget &&
               fSpecularExponent == o.fSpecularExponent &&
               fCosOuterConeAngle == o.fCosOuterConeAngle;
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

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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

bool SkDiffuseLightingImageFilter::asNewCustomStage(GrCustomStage** stage) const {
    if (stage) {
        SkScalar scale = SkScalarMul(surfaceScale(), SkIntToScalar(255));
        *stage = new GrDiffuseLightingEffect(light(), scale, kd());
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

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

bool SkSpecularLightingImageFilter::asNewCustomStage(GrCustomStage** stage) const {
    if (stage) {
        SkScalar scale = SkScalarMul(surfaceScale(), SkIntToScalar(255));
        *stage = new GrSpecularLightingEffect(light(), scale, ks(), shininess());
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

class GrGLLightingEffect  : public GrGLProgramStage {
public:
    GrGLLightingEffect(const GrProgramStageFactory& factory,
                       const GrCustomStage& stage);
    virtual ~GrGLLightingEffect();

    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    virtual void emitLightFunc(SkString* funcs) = 0;

    static inline StageKey GenKey(const GrCustomStage& s);

    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface*, 
                         const GrGLTexture&,
                         const GrCustomStage&,
                         int stageNum) SK_OVERRIDE;

private:
    typedef GrGLProgramStage INHERITED;

    const GrGLShaderVar*               fImageIncrementVar;
    GrGLint                            fImageIncrementLocation;
    const GrGLShaderVar*               fSurfaceScaleVar;
    GrGLint                            fSurfaceScaleLocation;
    GrGLLight*                         fLight;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDiffuseLightingEffect  : public GrGLLightingEffect {
public:
    GrGLDiffuseLightingEffect(const GrProgramStageFactory& factory,
                              const GrCustomStage& stage);
    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE;
    virtual void emitLightFunc(SkString* funcs) SK_OVERRIDE;
    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface*, 
                         const GrGLTexture&,
                         const GrCustomStage&,
                         int stageNum) SK_OVERRIDE;

private:
    typedef GrGLLightingEffect INHERITED;

    const GrGLShaderVar*               fKDVar;
    GrGLint                            fKDLocation;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpecularLightingEffect  : public GrGLLightingEffect {
public:
    GrGLSpecularLightingEffect(const GrProgramStageFactory& factory,
                               const GrCustomStage& stage);
    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE;
    virtual void emitLightFunc(SkString* funcs) SK_OVERRIDE;
    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface*, 
                         const GrGLTexture&,
                         const GrCustomStage&,
                         int stageNum) SK_OVERRIDE;

private:
    typedef GrGLLightingEffect INHERITED;

    const GrGLShaderVar*               fKSVar;
    GrGLint                            fKSLocation;
    const GrGLShaderVar*               fShininessVar;
    GrGLint                            fShininessLocation;
};

///////////////////////////////////////////////////////////////////////////////

GrLightingEffect::GrLightingEffect(const SkLight* light, SkScalar surfaceScale)
    : fLight(light)
    , fSurfaceScale(surfaceScale) {
    fLight->ref();
}

GrLightingEffect::~GrLightingEffect() {
    fLight->unref();
}

bool GrLightingEffect::isEqual(const GrCustomStage& sBase) const {
    const GrLightingEffect& s =
        static_cast<const GrLightingEffect&>(sBase);
    return fLight->isEqual(*s.fLight) &&
           fSurfaceScale == s.fSurfaceScale;
}

///////////////////////////////////////////////////////////////////////////////

GrDiffuseLightingEffect::GrDiffuseLightingEffect(const SkLight* light, SkScalar surfaceScale, SkScalar kd)
    : INHERITED(light, surfaceScale), fKD(kd) {
}

const GrProgramStageFactory& GrDiffuseLightingEffect::getFactory() const {
    return GrTProgramStageFactory<GrDiffuseLightingEffect>::getInstance();
}

bool GrDiffuseLightingEffect::isEqual(const GrCustomStage& sBase) const {
    const GrDiffuseLightingEffect& s =
        static_cast<const GrDiffuseLightingEffect&>(sBase);
    return INHERITED::isEqual(sBase) &&
            this->kd() == s.kd();
}

///////////////////////////////////////////////////////////////////////////////

GrGLLightingEffect::GrGLLightingEffect(const GrProgramStageFactory& factory,
                                       const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fImageIncrementVar(NULL)
    , fImageIncrementLocation(0)
    , fSurfaceScaleVar(NULL)
    , fSurfaceScaleLocation(0) {
    const GrLightingEffect& m = static_cast<const GrLightingEffect&>(stage);
    fLight = m.light()->createGLLight();
}

GrGLLightingEffect::~GrGLLightingEffect() {
    delete fLight;
}

void GrGLLightingEffect::setupVariables(GrGLShaderBuilder* state, int stage) {
    fImageIncrementVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kVec2f_GrSLType, "uImageIncrement", stage);
    fSurfaceScaleVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uSurfaceScale", stage);
    fLight->setupVariables(state, stage);
}

void GrGLLightingEffect::emitVS(GrGLShaderBuilder* state,
                                    const char* vertexCoords) {
    fLight->emitVS(&state->fVSCode);
}

void GrGLLightingEffect::initUniforms(const GrGLInterface* gl,
                                        int programID) {
    GR_GL_CALL_RET(gl, fSurfaceScaleLocation,
        GetUniformLocation(programID,
            fSurfaceScaleVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fImageIncrementLocation,
        GetUniformLocation(programID,
            fImageIncrementVar->getName().c_str()));
    fLight->initUniforms(gl, programID);
}

void GrGLLightingEffect::emitFS(GrGLShaderBuilder* state,
                                       const char* outputColor,
                                       const char* inputColor,
                                       const char* samplerName) {
    SkString* code = &state->fFSCode;
    SkString* funcs = &state->fFSFunctions;
    fLight->emitFuncs(funcs);
    emitLightFunc(funcs);
    funcs->appendf("float sobel(float a, float b, float c, float d, float e, float f, float scale) {\n");
    funcs->appendf("\treturn (-a + b - 2 * c + 2 * d -e + f) * scale;\n");
    funcs->appendf("}\n");
    funcs->appendf("vec3 pointToNormal(float x, float y, float scale) {\n");
    funcs->appendf("\treturn normalize(vec3(-x * scale, -y * scale, 1));\n");
    funcs->appendf("}\n");
    funcs->append("\n\
vec3 interiorNormal(float m[9], float surfaceScale) {\n\
    return pointToNormal(sobel(m[0], m[2], m[3], m[5], m[6], m[8], 0.25),\n\
                         sobel(m[0], m[6], m[1], m[7], m[2], m[8], 0.25),\n\
                         surfaceScale);\n}\n");

    code->appendf("\t\tvec2 coord = %s;\n", state->fSampleCoords.c_str());
    code->appendf("\t\tfloat m[9];\n");
    int index = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            SkString texCoords;
            texCoords.appendf("coord + vec2(%d, %d) * %s", dx, dy, fImageIncrementVar->getName().c_str());
            code->appendf("\t\tm[%d] = ", index++);
            state->emitTextureLookup(samplerName, texCoords.c_str());
            code->appendf(".a;\n");
        }
    }
    code->appendf("\t\tvec3 surfaceToLight = ");
    SkString arg;
    arg.appendf("%s * m[4]", fSurfaceScaleVar->getName().c_str());
    fLight->emitSurfaceToLight(code, arg.c_str());
    code->appendf(";\n");
    code->appendf("\t\t%s = light(interiorNormal(m, %s), surfaceToLight, ", outputColor, fSurfaceScaleVar->getName().c_str());
    fLight->emitLightColor(code, "surfaceToLight");
    code->appendf(")%s;\n", state->fModulate.c_str());
}

GrGLProgramStage::StageKey GrGLLightingEffect::GenKey(
  const GrCustomStage& s) {
    return static_cast<const GrLightingEffect&>(s).light()->type();
}

void GrGLLightingEffect::setData(const GrGLInterface* gl,
                                 const GrGLTexture& texture,
                                 const GrCustomStage& data,
                                 int stageNum) {
    const GrLightingEffect& effect =
        static_cast<const GrLightingEffect&>(data);
    GR_GL_CALL(gl, Uniform2f(fImageIncrementLocation, 1.0f / texture.width(), 1.0f / texture.height()));
    GR_GL_CALL(gl, Uniform1f(fSurfaceScaleLocation, effect.surfaceScale()));
    fLight->setData(gl, effect.light());
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

GrGLDiffuseLightingEffect::GrGLDiffuseLightingEffect(const GrProgramStageFactory& factory,
                                            const GrCustomStage& stage)
    : INHERITED(factory, stage)
    , fKDVar(NULL)
    , fKDLocation(0) {
}

void GrGLDiffuseLightingEffect::setupVariables(GrGLShaderBuilder* state, int stage) {
    INHERITED::setupVariables(state, stage);
    fKDVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uKD", stage);
}

void GrGLDiffuseLightingEffect::initUniforms(const GrGLInterface* gl,
                                             int programID) {
    INHERITED::initUniforms(gl, programID);
    GR_GL_CALL_RET(gl, fKDLocation,
        GetUniformLocation(programID,
            fKDVar->getName().c_str()));
}

void GrGLDiffuseLightingEffect::emitLightFunc(SkString* funcs) {
    funcs->appendf("vec4 light(vec3 normal, vec3 surfaceToLight, vec3 lightColor) {\n");
    funcs->appendf("\tfloat colorScale = %s * dot(normal, surfaceToLight);\n", fKDVar->getName().c_str());
    funcs->appendf("\treturn vec4(lightColor * clamp(colorScale, 0, 1), 1);\n");
    funcs->appendf("}\n");
}

void GrGLDiffuseLightingEffect::setData(const GrGLInterface* gl,
                                        const GrGLTexture& texture,
                                        const GrCustomStage& data,
                                        int stageNum) {
    INHERITED::setData(gl, texture, data, stageNum);
    const GrDiffuseLightingEffect& effect =
        static_cast<const GrDiffuseLightingEffect&>(data);
    GR_GL_CALL(gl, Uniform1f(fKDLocation, effect.kd()));
}

///////////////////////////////////////////////////////////////////////////////

GrSpecularLightingEffect::GrSpecularLightingEffect(const SkLight* light, SkScalar surfaceScale, SkScalar ks, SkScalar shininess)
    : INHERITED(light, surfaceScale),
      fKS(ks),
      fShininess(shininess) {
}

const GrProgramStageFactory& GrSpecularLightingEffect::getFactory() const {
    return GrTProgramStageFactory<GrSpecularLightingEffect>::getInstance();
}

bool GrSpecularLightingEffect::isEqual(const GrCustomStage& sBase) const {
    const GrSpecularLightingEffect& s =
        static_cast<const GrSpecularLightingEffect&>(sBase);
    return INHERITED::isEqual(sBase) &&
           this->ks() == s.ks() &&
           this->shininess() == s.shininess();
}

///////////////////////////////////////////////////////////////////////////////

GrGLSpecularLightingEffect::GrGLSpecularLightingEffect(const GrProgramStageFactory& factory,
                                            const GrCustomStage& stage)
    : GrGLLightingEffect(factory, stage)
    , fKSVar(NULL)
    , fKSLocation(0)
    , fShininessVar(NULL)
    , fShininessLocation(0) {
}

void GrGLSpecularLightingEffect::setupVariables(GrGLShaderBuilder* state, int stage) {
    INHERITED::setupVariables(state, stage);
    fKSVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uKS", stage);
    fShininessVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uShininess", stage);
}

void GrGLSpecularLightingEffect::initUniforms(const GrGLInterface* gl,
                                        int programID) {
    INHERITED::initUniforms(gl, programID);
    GR_GL_CALL_RET(gl, fKSLocation,
        GetUniformLocation(programID, fKSVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fShininessLocation,
        GetUniformLocation(programID, fShininessVar->getName().c_str()));
}

void GrGLSpecularLightingEffect::emitLightFunc(SkString* funcs) {
    funcs->appendf("vec4 light(vec3 normal, vec3 surfaceToLight, vec3 lightColor) {\n");
    funcs->appendf("\tvec3 halfDir = vec3(normalize(surfaceToLight + vec3(0, 0, 1)));\n");

    funcs->appendf("\tfloat colorScale = %s * pow(dot(normal, halfDir), %s);\n",
        fKSVar->getName().c_str(), fShininessVar->getName().c_str());
    funcs->appendf("\treturn vec4(lightColor * clamp(colorScale, 0, 1), 1);\n");
    funcs->appendf("}\n");
}

void GrGLSpecularLightingEffect::setData(const GrGLInterface* gl,
                                        const GrGLTexture& texture,
                                        const GrCustomStage& data,
                                        int stageNum) {
    INHERITED::setData(gl, texture, data, stageNum);
    const GrSpecularLightingEffect& effect =
        static_cast<const GrSpecularLightingEffect&>(data);
    GR_GL_CALL(gl, Uniform1f(fKSLocation, effect.ks()));
    GR_GL_CALL(gl, Uniform1f(fShininessLocation, effect.shininess()));
}

///////////////////////////////////////////////////////////////////////////////

void GrGLLight::emitLightColor(SkString* builder, const char *surfaceToLight) const {
    builder->append(fColorVar->getName().c_str());
}

void GrGLLight::setupVariables(GrGLShaderBuilder* state, int stage) {
    fColorVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kVec3f_GrSLType, "uLightColor", stage);
}

void GrGLLight::initUniforms(const GrGLInterface* gl, int programID) {
    GR_GL_CALL_RET(gl, fColorVarLocation,
        GetUniformLocation(programID, fColorVar->getName().c_str()));
}

void GrGLLight::setData(const GrGLInterface* gl, const SkLight* light) const {
    setUniformPoint3(gl, fColorVarLocation, light->color() * SkScalarInvert(SkIntToScalar(255)));
}

GrGLLight* SkDistantLight::createGLLight() const {
    return new GrGLDistantLight();
}

///////////////////////////////////////////////////////////////////////////////

void GrGLDistantLight::setupVariables(GrGLShaderBuilder* state, int stage) {
    INHERITED::setupVariables(state, stage);
    fDirectionVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType, kVec3f_GrSLType,
        "uLightDirection", stage);
}

void GrGLDistantLight::initUniforms(const GrGLInterface* gl, int programID) {
    INHERITED::initUniforms(gl, programID);
    GR_GL_CALL_RET(gl, fDirectionLocation,
        GetUniformLocation(programID, fDirectionVar->getName().c_str()));
}

void GrGLDistantLight::setData(const GrGLInterface* gl, const SkLight* light) const {
    INHERITED::setData(gl, light);
    SkASSERT(light->type() == SkLight::kDistant_LightType);
    const SkDistantLight* distantLight = static_cast<const SkDistantLight*>(light);
    setUniformPoint3(gl, fDirectionLocation, distantLight->direction());
}

void GrGLDistantLight::emitSurfaceToLight(SkString* builder,
                                          const char* z) const {
    builder->append(fDirectionVar->getName().c_str());
}

///////////////////////////////////////////////////////////////////////////////

void GrGLPointLight::setupVariables(GrGLShaderBuilder* state, int stage) {
    INHERITED::setupVariables(state, stage);
    fLocationVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType, kVec3f_GrSLType,
        "uLightLocation", stage);
    state->addVarying(kFloat_GrSLType, "Height", stage, &fHeightVaryingName);
}

void GrGLPointLight::initUniforms(const GrGLInterface* gl, int programID) {
    INHERITED::initUniforms(gl, programID);
    GR_GL_CALL_RET(gl, fLocationLocation,
        GetUniformLocation(programID, fLocationVar->getName().c_str()));
}

void GrGLPointLight::setData(const GrGLInterface* gl, const SkLight* light) const {
    INHERITED::setData(gl, light);
    SkASSERT(light->type() == SkLight::kPoint_LightType);
    const SkPointLight* pointLight = static_cast<const SkPointLight*>(light);
    setUniformPoint3(gl, fLocationLocation, pointLight->location());
}

void GrGLPointLight::emitVS(SkString* builder) const {
    // Compute viewport height from the Y scale of the matrix.
    builder->appendf("\t\t%s = -2.0 / uViewM[1][1];\n", fHeightVaryingName);
}

void GrGLPointLight::emitSurfaceToLight(SkString* builder,
                                      const char* z) const {
    builder->appendf(
        "normalize(%s - vec3(gl_FragCoord.x, %s - gl_FragCoord.y, %s))",
        fLocationVar->getName().c_str(), fHeightVaryingName, z);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLSpotLight::setupVariables(GrGLShaderBuilder* state, int stage) {
    INHERITED::setupVariables(state, stage);
    fLocationVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kVec3f_GrSLType, "uLightLocation", stage);
    fExponentVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uExponent", stage);
    fCosInnerConeAngleVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uCosInnerConeAngle", stage);
    fCosOuterConeAngleVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uCosOuterConeAngle", stage);
    fConeScaleVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kFloat_GrSLType, "uConeScale", stage);
    fSVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_ShaderType,
        kVec3f_GrSLType, "uS", stage);
    state->addVarying(kFloat_GrSLType, "Height", stage, &fHeightVaryingName);
}

void GrGLSpotLight::initUniforms(const GrGLInterface* gl, int programID) {
    INHERITED::initUniforms(gl, programID);
    GR_GL_CALL_RET(gl, fLocationLocation,
        GetUniformLocation(programID, fLocationVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fExponentLocation,
        GetUniformLocation(programID, fExponentVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fCosInnerConeAngleLocation,
        GetUniformLocation(programID, fCosInnerConeAngleVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fCosOuterConeAngleLocation,
        GetUniformLocation(programID, fCosOuterConeAngleVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fCosOuterConeAngleLocation,
        GetUniformLocation(programID, fCosOuterConeAngleVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fConeScaleLocation,
        GetUniformLocation(programID, fConeScaleVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fSLocation,
        GetUniformLocation(programID, fSVar->getName().c_str()));
}

void GrGLSpotLight::setData(const GrGLInterface* gl, const SkLight* light) const {
    INHERITED::setData(gl, light);
    SkASSERT(light->type() == SkLight::kSpot_LightType);
    const SkSpotLight* spotLight = static_cast<const SkSpotLight *>(light);
    setUniformPoint3(gl, fLocationLocation, spotLight->location());
    GR_GL_CALL(gl, Uniform1f(fExponentLocation, spotLight->specularExponent()));
    GR_GL_CALL(gl, Uniform1f(fCosInnerConeAngleLocation, spotLight->cosInnerConeAngle()));
    GR_GL_CALL(gl, Uniform1f(fCosOuterConeAngleLocation, spotLight->cosOuterConeAngle()));
    GR_GL_CALL(gl, Uniform1f(fConeScaleLocation, spotLight->coneScale()));
    setUniformPoint3(gl, fSLocation, spotLight->s());
}

void GrGLSpotLight::emitVS(SkString* builder) const {
    // Compute viewport height from the Y scale of the matrix.
    builder->appendf("\t\t%s = -2.0 / uViewM[1][1];\n", fHeightVaryingName);
}

void GrGLSpotLight::emitFuncs(SkString* builder) const {
    builder->appendf("vec3 lightColor(vec3 surfaceToLight) {\n");
    builder->appendf("\tfloat cosAngle = -dot(surfaceToLight, %s);\n", fSVar->getName().c_str());
    builder->appendf("\tif (cosAngle < %s) {\n", fCosOuterConeAngleVar->getName().c_str());
    builder->appendf("\t\treturn vec3(0);\n");
    builder->appendf("\t}\n");
    builder->appendf("\tfloat scale = pow(cosAngle, %s);\n", fExponentVar->getName().c_str());
    builder->appendf("\tif (cosAngle < %s) {\n", fCosInnerConeAngleVar->getName().c_str());
    builder->appendf("\t\treturn %s * scale * (cosAngle - %s) * %s;\n", fColorVar->getName().c_str(), fCosOuterConeAngleVar->getName().c_str(), fConeScaleVar->getName().c_str());
    builder->appendf("\t}\n");
    builder->appendf("\treturn %s;\n", fColorVar->getName().c_str());
    builder->appendf("}\n");
}

void GrGLSpotLight::emitSurfaceToLight(SkString* builder, const char* z) const {
    builder->appendf("normalize(%s - vec3(gl_FragCoord.x, %s - gl_FragCoord.y, %s))", fLocationVar->getName().c_str(), fHeightVaryingName, z);
}

void GrGLSpotLight::emitLightColor(SkString* builder, const char *surfaceToLight) const {
    builder->appendf("lightColor(%s)", surfaceToLight);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiffuseLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpecularLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDistantLight)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPointLight)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpotLight)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
