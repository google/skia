/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLightingImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "effects/GrSingleTextureEffect.h"
#include "gl/GrGLEffect.h"
#include "GrEffect.h"
#include "GrTBackendEffectFactory.h"

class GrGLDiffuseLightingEffect;
class GrGLSpecularLightingEffect;

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;
#endif

namespace {

const SkScalar gOneThird = SkScalarInvert(SkIntToScalar(3));
const SkScalar gTwoThirds = SkScalarDiv(SkIntToScalar(2), SkIntToScalar(3));
const SkScalar gOneHalf = SkFloatToScalar(0.5f);
const SkScalar gOneQuarter = SkFloatToScalar(0.25f);

#if SK_SUPPORT_GPU
void setUniformPoint3(const GrGLUniformManager& uman, UniformHandle uni, const SkPoint3& point) {
    GR_STATIC_ASSERT(sizeof(SkPoint3) == 3 * sizeof(GrGLfloat));
    uman.set3fv(uni, 0, 1, &point.fX);
}

void setUniformNormal3(const GrGLUniformManager& uman, UniformHandle uni, const SkPoint3& point) {
    setUniformPoint3(uman, uni, SkPoint3(point.fX, point.fY, point.fZ));
}
#endif

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
                            SkClampMax(SkScalarFloorToInt(color.fX), 255),
                            SkClampMax(SkScalarFloorToInt(color.fY), 255),
                            SkClampMax(SkScalarFloorToInt(color.fZ), 255));
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
        return SkPackARGB32(SkClampMax(SkScalarFloorToInt(color.maxComponent()), 255),
                            SkClampMax(SkScalarFloorToInt(color.fX), 255),
                            SkClampMax(SkScalarFloorToInt(color.fY), 255),
                            SkClampMax(SkScalarFloorToInt(color.fZ), 255));
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

template <class LightingType, class LightType> void lightBitmap(const LightingType& lightingType, const SkLight* light, const SkBitmap& src, SkBitmap* dst, SkScalar surfaceScale, const SkIRect& bounds) {
    SkASSERT(dst->width() == bounds.width() && dst->height() == bounds.height());
    const LightType* l = static_cast<const LightType*>(light);
    int left = bounds.left(), right = bounds.right();
    int bottom = bounds.bottom();
    int y = bounds.top();
    SkPMColor* dptr = dst->getAddr32(0, 0);
    {
        int x = left;
        const SkPMColor* row1 = src.getAddr32(x, y);
        const SkPMColor* row2 = src.getAddr32(x, y + 1);
        int m[9];
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        m[7] = SkGetPackedA32(*row2++);
        m[8] = SkGetPackedA32(*row2++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topLeftNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x)
        {
            shiftMatrixLeft(m);
            m[5] = SkGetPackedA32(*row1++);
            m[8] = SkGetPackedA32(*row2++);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(topNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topRightNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
    }

    for (++y; y < bottom - 1; ++y) {
        int x = left;
        const SkPMColor* row0 = src.getAddr32(x, y - 1);
        const SkPMColor* row1 = src.getAddr32(x, y);
        const SkPMColor* row2 = src.getAddr32(x, y + 1);
        int m[9];
        m[1] = SkGetPackedA32(*row0++);
        m[2] = SkGetPackedA32(*row0++);
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        m[7] = SkGetPackedA32(*row2++);
        m[8] = SkGetPackedA32(*row2++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(leftNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x) {
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
        int x = left;
        const SkPMColor* row0 = src.getAddr32(x, bottom - 2);
        const SkPMColor* row1 = src.getAddr32(x, bottom - 1);
        int m[9];
        m[1] = SkGetPackedA32(*row0++);
        m[2] = SkGetPackedA32(*row0++);
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(bottomLeftNormal(m, surfaceScale), surfaceToLight, l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x)
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
    SkDiffuseLightingImageFilter(SkLight* light, SkScalar surfaceScale,
                                 SkScalar kd, SkImageFilter* input, const CropRect* cropRect);
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDiffuseLightingImageFilter)

#if SK_SUPPORT_GPU
    virtual bool asNewEffect(GrEffectRef** effect, GrTexture*, const SkMatrix& matrix) const SK_OVERRIDE;
#endif
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
    SkSpecularLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkScalar ks, SkScalar shininess, SkImageFilter* input, const CropRect* cropRect);
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSpecularLightingImageFilter)

#if SK_SUPPORT_GPU
    virtual bool asNewEffect(GrEffectRef** effect, GrTexture*, const SkMatrix& matrix) const SK_OVERRIDE;
#endif

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

#if SK_SUPPORT_GPU

class GrLightingEffect : public GrSingleTextureEffect {
public:
    GrLightingEffect(GrTexture* texture, const SkLight* light, SkScalar surfaceScale, const SkMatrix& matrix);
    virtual ~GrLightingEffect();

    const SkLight* light() const { return fLight; }
    SkScalar surfaceScale() const { return fSurfaceScale; }
    const SkMatrix& filterMatrix() const { return fFilterMatrix; }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        // lighting shaders are complicated. We just throw up our hands.
        *validFlags = 0;
    }

protected:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

private:
    typedef GrSingleTextureEffect INHERITED;
    const SkLight* fLight;
    SkScalar fSurfaceScale;
    SkMatrix fFilterMatrix;
};

class GrDiffuseLightingEffect : public GrLightingEffect {
public:
    static GrEffectRef* Create(GrTexture* texture,
                               const SkLight* light,
                               SkScalar surfaceScale,
                               const SkMatrix& matrix,
                               SkScalar kd) {
        AutoEffectUnref effect(SkNEW_ARGS(GrDiffuseLightingEffect, (texture,
                                                                    light,
                                                                    surfaceScale,
                                                                    matrix,
                                                                    kd)));
        return CreateEffectRef(effect);
    }

    static const char* Name() { return "DiffuseLighting"; }

    typedef GrGLDiffuseLightingEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    SkScalar kd() const { return fKD; }

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GrDiffuseLightingEffect(GrTexture* texture,
                            const SkLight* light,
                            SkScalar surfaceScale,
                            const SkMatrix& matrix,
                            SkScalar kd);

    GR_DECLARE_EFFECT_TEST;
    typedef GrLightingEffect INHERITED;
    SkScalar fKD;
};

class GrSpecularLightingEffect : public GrLightingEffect {
public:
    static GrEffectRef* Create(GrTexture* texture,
                               const SkLight* light,
                               SkScalar surfaceScale,
                               const SkMatrix& matrix,
                               SkScalar ks,
                               SkScalar shininess) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSpecularLightingEffect, (texture,
                                                                     light,
                                                                     surfaceScale,
                                                                     matrix,
                                                                     ks,
                                                                     shininess)));
        return CreateEffectRef(effect);
    }
    static const char* Name() { return "SpecularLighting"; }

    typedef GrGLSpecularLightingEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GrSpecularLightingEffect(GrTexture* texture,
                             const SkLight* light,
                             SkScalar surfaceScale,
                             const SkMatrix& matrix,
                             SkScalar ks,
                             SkScalar shininess);

    GR_DECLARE_EFFECT_TEST;
    typedef GrLightingEffect INHERITED;
    SkScalar fKS;
    SkScalar fShininess;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLLight {
public:
    virtual ~GrGLLight() {}

    /**
     * This is called by GrGLLightingEffect::emitCode() before either of the two virtual functions
     * below. It adds a vec3f uniform visible in the FS that represents the constant light color.
     */
    void emitLightColorUniform(GrGLShaderBuilder*);

    /**
     * These two functions are called from GrGLLightingEffect's emitCode() function.
     * emitSurfaceToLight places an expression in param out that is the vector from the surface to
     * the light. The expression will be used in the FS. emitLightColor writes an expression into
     * the FS that is the color of the light. Either function may add functions and/or uniforms to
     * the FS. The default of emitLightColor appends the name of the constant light color uniform
     * and so this function only needs to be overridden if the light color varies spatially.
     */
    virtual void emitSurfaceToLight(GrGLShaderBuilder*, const char* z) = 0;
    virtual void emitLightColor(GrGLShaderBuilder*, const char *surfaceToLight);

    // This is called from GrGLLightingEffect's setData(). Subclasses of GrGLLight must call
    // INHERITED::setData().
    virtual void setData(const GrGLUniformManager&,
                         const SkLight* light) const;

protected:
    /**
     * Gets the constant light color uniform. Subclasses can use this in their emitLightColor
     * function.
     */
    UniformHandle lightColorUni() const { return fColorUni; }

private:
    UniformHandle fColorUni;

    typedef SkRefCnt INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDistantLight : public GrGLLight {
public:
    virtual ~GrGLDistantLight() {}
    virtual void setData(const GrGLUniformManager&,
                         const SkLight* light) const SK_OVERRIDE;
    virtual void emitSurfaceToLight(GrGLShaderBuilder*, const char* z) SK_OVERRIDE;

private:
    typedef GrGLLight INHERITED;
    UniformHandle fDirectionUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLPointLight : public GrGLLight {
public:
    virtual ~GrGLPointLight() {}
    virtual void setData(const GrGLUniformManager&,
                         const SkLight* light) const SK_OVERRIDE;
    virtual void emitSurfaceToLight(GrGLShaderBuilder*, const char* z) SK_OVERRIDE;

private:
    typedef GrGLLight INHERITED;
    UniformHandle fLocationUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpotLight : public GrGLLight {
public:
    virtual ~GrGLSpotLight() {}
    virtual void setData(const GrGLUniformManager&,
                         const SkLight* light) const SK_OVERRIDE;
    virtual void emitSurfaceToLight(GrGLShaderBuilder*, const char* z) SK_OVERRIDE;
    virtual void emitLightColor(GrGLShaderBuilder*, const char *surfaceToLight) SK_OVERRIDE;

private:
    typedef GrGLLight INHERITED;

    SkString        fLightColorFunc;
    UniformHandle   fLocationUni;
    UniformHandle   fExponentUni;
    UniformHandle   fCosOuterConeAngleUni;
    UniformHandle   fCosInnerConeAngleUni;
    UniformHandle   fConeScaleUni;
    UniformHandle   fSUni;
};
#else

class GrGLLight;

#endif

};

///////////////////////////////////////////////////////////////////////////////

class SkLight : public SkRefCnt {
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
    // Called to know whether the generated GrGLLight will require access to the fragment position.
    virtual bool requiresFragmentPosition() const = 0;
    virtual SkLight* transform(const SkMatrix& matrix) const = 0;

    // Defined below SkLight's subclasses.
    void flattenLight(SkFlattenableWriteBuffer& buffer) const;
    static SkLight* UnflattenLight(SkFlattenableReadBuffer& buffer);

protected:
    SkLight(SkColor color)
      : fColor(SkIntToScalar(SkColorGetR(color)),
               SkIntToScalar(SkColorGetG(color)),
               SkIntToScalar(SkColorGetB(color))) {}
    SkLight(const SkPoint3& color)
      : fColor(color) {}
    SkLight(SkFlattenableReadBuffer& buffer) {
        fColor = readPoint3(buffer);
    }

    virtual void onFlattenLight(SkFlattenableWriteBuffer& buffer) const = 0;


private:
    typedef SkRefCnt INHERITED;
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
    virtual GrGLLight* createGLLight() const SK_OVERRIDE {
#if SK_SUPPORT_GPU
        return SkNEW(GrGLDistantLight);
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return NULL;
#endif
    }
    virtual bool requiresFragmentPosition() const SK_OVERRIDE { return false; }

    virtual bool isEqual(const SkLight& other) const SK_OVERRIDE {
        if (other.type() != kDistant_LightType) {
            return false;
        }

        const SkDistantLight& o = static_cast<const SkDistantLight&>(other);
        return INHERITED::isEqual(other) &&
               fDirection == o.fDirection;
    }

    SkDistantLight(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fDirection = readPoint3(buffer);
    }

protected:
    SkDistantLight(const SkPoint3& direction, const SkPoint3& color)
      : INHERITED(color), fDirection(direction) {
    }
    virtual SkLight* transform(const SkMatrix& matrix) const {
        return new SkDistantLight(direction(), color());
    }
    virtual void onFlattenLight(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
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
#if SK_SUPPORT_GPU
        return SkNEW(GrGLPointLight);
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return NULL;
#endif
    }
    virtual bool requiresFragmentPosition() const SK_OVERRIDE { return true; }
    virtual bool isEqual(const SkLight& other) const SK_OVERRIDE {
        if (other.type() != kPoint_LightType) {
            return false;
        }
        const SkPointLight& o = static_cast<const SkPointLight&>(other);
        return INHERITED::isEqual(other) &&
               fLocation == o.fLocation;
    }
    virtual SkLight* transform(const SkMatrix& matrix) const {
        SkPoint location2 = SkPoint::Make(fLocation.fX, fLocation.fY);
        matrix.mapPoints(&location2, 1);
        SkPoint3 location(location2.fX, location2.fY, fLocation.fZ);
        return new SkPointLight(location, color());
    }

    SkPointLight(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
    }

protected:
    SkPointLight(const SkPoint3& location, const SkPoint3& color)
     : INHERITED(color), fLocation(location) {}
    virtual void onFlattenLight(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
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
       fSpecularExponent(SkScalarPin(specularExponent, kSpecularExponentMin, kSpecularExponentMax))
    {
       fS = target - location;
       fS.normalize();
       fCosOuterConeAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
       const SkScalar antiAliasThreshold = SkFloatToScalar(0.016f);
       fCosInnerConeAngle = fCosOuterConeAngle + antiAliasThreshold;
       fConeScale = SkScalarInvert(antiAliasThreshold);
    }

    virtual SkLight* transform(const SkMatrix& matrix) const {
        SkPoint location2 = SkPoint::Make(fLocation.fX, fLocation.fY);
        matrix.mapPoints(&location2, 1);
        SkPoint3 location(location2.fX, location2.fY, fLocation.fZ);
        SkPoint target2 = SkPoint::Make(fTarget.fX, fTarget.fY);
        matrix.mapPoints(&target2, 1);
        SkPoint3 target(target2.fX, target2.fY, fTarget.fZ);
        return new SkSpotLight(location, target, fSpecularExponent, fCosOuterConeAngle, fCosInnerConeAngle, fConeScale, fS, color());
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
#if SK_SUPPORT_GPU
        return SkNEW(GrGLSpotLight);
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return NULL;
#endif
    }
    virtual bool requiresFragmentPosition() const SK_OVERRIDE { return true; }
    virtual LightType type() const { return kSpot_LightType; }
    const SkPoint3& location() const { return fLocation; }
    const SkPoint3& target() const { return fTarget; }
    SkScalar specularExponent() const { return fSpecularExponent; }
    SkScalar cosInnerConeAngle() const { return fCosInnerConeAngle; }
    SkScalar cosOuterConeAngle() const { return fCosOuterConeAngle; }
    SkScalar coneScale() const { return fConeScale; }
    const SkPoint3& s() const { return fS; }

    SkSpotLight(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
        fTarget = readPoint3(buffer);
        fSpecularExponent = buffer.readScalar();
        fCosOuterConeAngle = buffer.readScalar();
        fCosInnerConeAngle = buffer.readScalar();
        fConeScale = buffer.readScalar();
        fS = readPoint3(buffer);
    }
protected:
    SkSpotLight(const SkPoint3& location, const SkPoint3& target, SkScalar specularExponent, SkScalar cosOuterConeAngle, SkScalar cosInnerConeAngle, SkScalar coneScale, const SkPoint3& s, const SkPoint3& color)
     : INHERITED(color),
       fLocation(location),
       fTarget(target),
       fSpecularExponent(specularExponent),
       fCosOuterConeAngle(cosOuterConeAngle),
       fCosInnerConeAngle(cosInnerConeAngle),
       fConeScale(coneScale),
       fS(s)
    {
    }
    virtual void onFlattenLight(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
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
    static const SkScalar kSpecularExponentMin;
    static const SkScalar kSpecularExponentMax;

    typedef SkLight INHERITED;
    SkPoint3 fLocation;
    SkPoint3 fTarget;
    SkScalar fSpecularExponent;
    SkScalar fCosOuterConeAngle;
    SkScalar fCosInnerConeAngle;
    SkScalar fConeScale;
    SkPoint3 fS;
};

// According to the spec, the specular term should be in the range [1, 128] :
// http://www.w3.org/TR/SVG/filters.html#feSpecularLightingSpecularExponentAttribute
const SkScalar SkSpotLight::kSpecularExponentMin = SkFloatToScalar(1.0f);
const SkScalar SkSpotLight::kSpecularExponentMax = SkFloatToScalar(128.0f);

///////////////////////////////////////////////////////////////////////////////

void SkLight::flattenLight(SkFlattenableWriteBuffer& buffer) const {
    // Write type first, then baseclass, then subclass.
    buffer.writeInt(this->type());
    writePoint3(fColor, buffer);
    this->onFlattenLight(buffer);
}

/*static*/ SkLight* SkLight::UnflattenLight(SkFlattenableReadBuffer& buffer) {
    // Read type first.
    const SkLight::LightType type = (SkLight::LightType)buffer.readInt();
    switch (type) {
        // Each of these constructors must first call SkLight's, so we'll read the baseclass
        // then subclass, same order as flattenLight.
        case SkLight::kDistant_LightType: return SkNEW_ARGS(SkDistantLight, (buffer));
        case SkLight::kPoint_LightType:   return SkNEW_ARGS(SkPointLight, (buffer));
        case SkLight::kSpot_LightType:    return SkNEW_ARGS(SkSpotLight, (buffer));
        default:
            SkDEBUGFAIL("Unknown LightType.");
            return NULL;
    }
}
///////////////////////////////////////////////////////////////////////////////

SkLightingImageFilter::SkLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkImageFilter* input, const CropRect* cropRect)
  : INHERITED(input, cropRect),
    fLight(light),
    fSurfaceScale(SkScalarDiv(surfaceScale, SkIntToScalar(255)))
{
    SkASSERT(fLight);
    // our caller knows that we take ownership of the light, so we don't
    // need to call ref() here.
}

SkImageFilter* SkLightingImageFilter::CreateDistantLitDiffuse(
    const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale,
    SkScalar kd, SkImageFilter* input, const CropRect* cropRect) {
    return SkNEW_ARGS(SkDiffuseLightingImageFilter,
        (SkNEW_ARGS(SkDistantLight, (direction, lightColor)), surfaceScale, kd,
        input, cropRect));
}

SkImageFilter* SkLightingImageFilter::CreatePointLitDiffuse(
    const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale,
    SkScalar kd, SkImageFilter* input, const CropRect* cropRect) {
    return SkNEW_ARGS(SkDiffuseLightingImageFilter,
        (SkNEW_ARGS(SkPointLight, (location, lightColor)), surfaceScale, kd,
        input, cropRect));
}

SkImageFilter* SkLightingImageFilter::CreateSpotLitDiffuse(
    const SkPoint3& location, const SkPoint3& target,
    SkScalar specularExponent, SkScalar cutoffAngle,
    SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
    SkImageFilter* input, const CropRect* cropRect) {
    return SkNEW_ARGS(SkDiffuseLightingImageFilter,
        (SkNEW_ARGS(SkSpotLight, (location, target, specularExponent,
                                  cutoffAngle, lightColor)),
                    surfaceScale, kd, input, cropRect));
}

SkImageFilter* SkLightingImageFilter::CreateDistantLitSpecular(
    const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale,
    SkScalar ks, SkScalar shininess, SkImageFilter* input, const CropRect* cropRect) {
    return SkNEW_ARGS(SkSpecularLightingImageFilter,
        (SkNEW_ARGS(SkDistantLight, (direction, lightColor)),
        surfaceScale, ks, shininess, input, cropRect));
}

SkImageFilter* SkLightingImageFilter::CreatePointLitSpecular(
    const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale,
    SkScalar ks, SkScalar shininess, SkImageFilter* input, const CropRect* cropRect) {
    return SkNEW_ARGS(SkSpecularLightingImageFilter,
        (SkNEW_ARGS(SkPointLight, (location, lightColor)),
        surfaceScale, ks, shininess, input, cropRect));
}

SkImageFilter* SkLightingImageFilter::CreateSpotLitSpecular(
    const SkPoint3& location, const SkPoint3& target,
    SkScalar specularExponent, SkScalar cutoffAngle,
    SkColor lightColor, SkScalar surfaceScale,
    SkScalar ks, SkScalar shininess, SkImageFilter* input, const CropRect* cropRect) {
    return SkNEW_ARGS(SkSpecularLightingImageFilter,
        (SkNEW_ARGS(SkSpotLight, (location, target, specularExponent, cutoffAngle, lightColor)),
        surfaceScale, ks, shininess, input, cropRect));
}

SkLightingImageFilter::~SkLightingImageFilter() {
    fLight->unref();
}

SkLightingImageFilter::SkLightingImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fLight = SkLight::UnflattenLight(buffer);
    fSurfaceScale = buffer.readScalar();
}

void SkLightingImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    fLight->flattenLight(buffer);
    buffer.writeScalar(fSurfaceScale);
}

///////////////////////////////////////////////////////////////////////////////

SkDiffuseLightingImageFilter::SkDiffuseLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkScalar kd, SkImageFilter* input, const CropRect* cropRect = NULL)
  : SkLightingImageFilter(light, surfaceScale, input, cropRect),
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
                                                 const SkMatrix& ctm,
                                                 SkBitmap* dst,
                                                 SkIPoint* offset) {
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }
    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }

    if (bounds.width() < 2 || bounds.height() < 2) {
        return false;
    }

    dst->setConfig(src.config(), bounds.width(), bounds.height());
    dst->allocPixels();

    SkAutoTUnref<SkLight> transformedLight(light()->transform(ctm));

    DiffuseLightingType lightingType(fKD);
    switch (transformedLight->type()) {
        case SkLight::kDistant_LightType:
            lightBitmap<DiffuseLightingType, SkDistantLight>(lightingType, transformedLight, src, dst, surfaceScale(), bounds);
            break;
        case SkLight::kPoint_LightType:
            lightBitmap<DiffuseLightingType, SkPointLight>(lightingType, transformedLight, src, dst, surfaceScale(), bounds);
            break;
        case SkLight::kSpot_LightType:
            lightBitmap<DiffuseLightingType, SkSpotLight>(lightingType, transformedLight, src, dst, surfaceScale(), bounds);
            break;
    }

    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
}

#if SK_SUPPORT_GPU
bool SkDiffuseLightingImageFilter::asNewEffect(GrEffectRef** effect, GrTexture* texture, const SkMatrix& matrix) const {
    if (effect) {
        SkScalar scale = SkScalarMul(surfaceScale(), SkIntToScalar(255));
        *effect = GrDiffuseLightingEffect::Create(texture, light(), scale, matrix, kd());
    }
    return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkSpecularLightingImageFilter::SkSpecularLightingImageFilter(SkLight* light, SkScalar surfaceScale, SkScalar ks, SkScalar shininess, SkImageFilter* input, const CropRect* cropRect)
  : SkLightingImageFilter(light, surfaceScale, input, cropRect),
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
                                                  const SkMatrix& ctm,
                                                  SkBitmap* dst,
                                                  SkIPoint* offset) {
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }
    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }

    if (bounds.width() < 2 || bounds.height() < 2) {
        return false;
    }

    dst->setConfig(src.config(), bounds.width(), bounds.height());
    dst->allocPixels();

    SpecularLightingType lightingType(fKS, fShininess);
    SkAutoTUnref<SkLight> transformedLight(light()->transform(ctm));
    switch (transformedLight->type()) {
        case SkLight::kDistant_LightType:
            lightBitmap<SpecularLightingType, SkDistantLight>(lightingType, transformedLight, src, dst, surfaceScale(), bounds);
            break;
        case SkLight::kPoint_LightType:
            lightBitmap<SpecularLightingType, SkPointLight>(lightingType, transformedLight, src, dst, surfaceScale(), bounds);
            break;
        case SkLight::kSpot_LightType:
            lightBitmap<SpecularLightingType, SkSpotLight>(lightingType, transformedLight, src, dst, surfaceScale(), bounds);
            break;
    }
    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
}

#if SK_SUPPORT_GPU
bool SkSpecularLightingImageFilter::asNewEffect(GrEffectRef** effect, GrTexture* texture, const SkMatrix& matrix) const {
    if (effect) {
        SkScalar scale = SkScalarMul(surfaceScale(), SkIntToScalar(255));
        *effect = GrSpecularLightingEffect::Create(texture, light(), scale, matrix, ks(), shininess());
    }
    return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

namespace {
SkPoint3 random_point3(SkRandom* random) {
    return SkPoint3(SkScalarToFloat(random->nextSScalar1()),
                    SkScalarToFloat(random->nextSScalar1()),
                    SkScalarToFloat(random->nextSScalar1()));
}

SkLight* create_random_light(SkRandom* random) {
    int type = random->nextULessThan(3);
    switch (type) {
        case 0: {
            return SkNEW_ARGS(SkDistantLight, (random_point3(random), random->nextU()));
        }
        case 1: {
            return SkNEW_ARGS(SkPointLight, (random_point3(random), random->nextU()));
        }
        case 2: {
            return SkNEW_ARGS(SkSpotLight, (random_point3(random),
                                            random_point3(random),
                                            random->nextUScalar1(),
                                            random->nextUScalar1(),
                                            random->nextU()));
        }
        default:
            GrCrash();
            return NULL;
    }
}

}

class GrGLLightingEffect  : public GrGLEffect {
public:
    GrGLLightingEffect(const GrBackendEffectFactory& factory,
                       const GrDrawEffect& effect);
    virtual ~GrGLLightingEffect();

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    /**
     * Subclasses of GrGLLightingEffect must call INHERITED::setData();
     */
    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

protected:
    virtual void emitLightFunc(GrGLShaderBuilder*, SkString* funcName) = 0;

private:
    typedef GrGLEffect INHERITED;

    UniformHandle       fImageIncrementUni;
    UniformHandle       fSurfaceScaleUni;
    GrGLLight*          fLight;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDiffuseLightingEffect  : public GrGLLightingEffect {
public:
    GrGLDiffuseLightingEffect(const GrBackendEffectFactory& factory,
                              const GrDrawEffect& drawEffect);
    virtual void emitLightFunc(GrGLShaderBuilder*, SkString* funcName) SK_OVERRIDE;
    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLLightingEffect INHERITED;

    UniformHandle   fKDUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpecularLightingEffect  : public GrGLLightingEffect {
public:
    GrGLSpecularLightingEffect(const GrBackendEffectFactory& factory,
                               const GrDrawEffect& effect);
    virtual void emitLightFunc(GrGLShaderBuilder*, SkString* funcName) SK_OVERRIDE;
    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLLightingEffect INHERITED;

    UniformHandle   fKSUni;
    UniformHandle   fShininessUni;
};

///////////////////////////////////////////////////////////////////////////////

GrLightingEffect::GrLightingEffect(GrTexture* texture,
                                   const SkLight* light,
                                   SkScalar surfaceScale,
                                   const SkMatrix& matrix)
    : INHERITED(texture, MakeDivByTextureWHMatrix(texture))
    , fLight(light)
    , fSurfaceScale(surfaceScale)
    , fFilterMatrix(matrix) {
    fLight->ref();
    if (light->requiresFragmentPosition()) {
        this->setWillReadFragmentPosition();
    }
}

GrLightingEffect::~GrLightingEffect() {
    fLight->unref();
}

bool GrLightingEffect::onIsEqual(const GrEffect& sBase) const {
    const GrLightingEffect& s = CastEffect<GrLightingEffect>(sBase);
    return this->texture(0) == s.texture(0) &&
           fLight->isEqual(*s.fLight) &&
           fSurfaceScale == s.fSurfaceScale;
}

///////////////////////////////////////////////////////////////////////////////

GrDiffuseLightingEffect::GrDiffuseLightingEffect(GrTexture* texture,
                                                 const SkLight* light,
                                                 SkScalar surfaceScale,
                                                 const SkMatrix& matrix,
                                                 SkScalar kd)
    : INHERITED(texture, light, surfaceScale, matrix), fKD(kd) {
}

const GrBackendEffectFactory& GrDiffuseLightingEffect::getFactory() const {
    return GrTBackendEffectFactory<GrDiffuseLightingEffect>::getInstance();
}

bool GrDiffuseLightingEffect::onIsEqual(const GrEffect& sBase) const {
    const GrDiffuseLightingEffect& s = CastEffect<GrDiffuseLightingEffect>(sBase);
    return INHERITED::onIsEqual(sBase) &&
            this->kd() == s.kd();
}

GR_DEFINE_EFFECT_TEST(GrDiffuseLightingEffect);

GrEffectRef* GrDiffuseLightingEffect::TestCreate(SkRandom* random,
                                                 GrContext* context,
                                                 const GrDrawTargetCaps&,
                                                 GrTexture* textures[]) {
    SkScalar surfaceScale = random->nextSScalar1();
    SkScalar kd = random->nextUScalar1();
    SkAutoTUnref<SkLight> light(create_random_light(random));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = random->nextUScalar1();
    }
    return GrDiffuseLightingEffect::Create(textures[GrEffectUnitTest::kAlphaTextureIdx],
                                           light, surfaceScale, matrix, kd);
}


///////////////////////////////////////////////////////////////////////////////

GrGLLightingEffect::GrGLLightingEffect(const GrBackendEffectFactory& factory,
                                       const GrDrawEffect& drawEffect)
    : INHERITED(factory) {
    const GrLightingEffect& m = drawEffect.castEffect<GrLightingEffect>();
    fLight = m.light()->createGLLight();
}

GrGLLightingEffect::~GrGLLightingEffect() {
    delete fLight;
}

void GrGLLightingEffect::emitCode(GrGLShaderBuilder* builder,
                                  const GrDrawEffect&,
                                  EffectKey key,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const TransformedCoordsArray& coords,
                                  const TextureSamplerArray& samplers) {
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);

    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                              kVec2f_GrSLType,
                                             "ImageIncrement");
    fSurfaceScaleUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                           kFloat_GrSLType,
                                           "SurfaceScale");
    fLight->emitLightColorUniform(builder);
    SkString lightFunc;
    this->emitLightFunc(builder, &lightFunc);
    static const GrGLShaderVar gSobelArgs[] =  {
        GrGLShaderVar("a", kFloat_GrSLType),
        GrGLShaderVar("b", kFloat_GrSLType),
        GrGLShaderVar("c", kFloat_GrSLType),
        GrGLShaderVar("d", kFloat_GrSLType),
        GrGLShaderVar("e", kFloat_GrSLType),
        GrGLShaderVar("f", kFloat_GrSLType),
        GrGLShaderVar("scale", kFloat_GrSLType),
    };
    SkString sobelFuncName;
    builder->fsEmitFunction(kFloat_GrSLType,
                            "sobel",
                            SK_ARRAY_COUNT(gSobelArgs),
                            gSobelArgs,
                            "\treturn (-a + b - 2.0 * c + 2.0 * d -e + f) * scale;\n",
                            &sobelFuncName);
    static const GrGLShaderVar gPointToNormalArgs[] =  {
        GrGLShaderVar("x", kFloat_GrSLType),
        GrGLShaderVar("y", kFloat_GrSLType),
        GrGLShaderVar("scale", kFloat_GrSLType),
    };
    SkString pointToNormalName;
    builder->fsEmitFunction(kVec3f_GrSLType,
                            "pointToNormal",
                            SK_ARRAY_COUNT(gPointToNormalArgs),
                            gPointToNormalArgs,
                            "\treturn normalize(vec3(-x * scale, y * scale, 1));\n",
                            &pointToNormalName);

    static const GrGLShaderVar gInteriorNormalArgs[] =  {
        GrGLShaderVar("m", kFloat_GrSLType, 9),
        GrGLShaderVar("surfaceScale", kFloat_GrSLType),
    };
    SkString interiorNormalBody;
    interiorNormalBody.appendf("\treturn %s(%s(m[0], m[2], m[3], m[5], m[6], m[8], 0.25),\n"
                               "\t       %s(m[0], m[6], m[1], m[7], m[2], m[8], 0.25),\n"
                               "\t       surfaceScale);\n",
                                pointToNormalName.c_str(),
                                sobelFuncName.c_str(),
                                sobelFuncName.c_str());
    SkString interiorNormalName;
    builder->fsEmitFunction(kVec3f_GrSLType,
                            "interiorNormal",
                            SK_ARRAY_COUNT(gInteriorNormalArgs),
                            gInteriorNormalArgs,
                            interiorNormalBody.c_str(),
                            &interiorNormalName);

    builder->fsCodeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    builder->fsCodeAppend("\t\tfloat m[9];\n");

    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    const char* surfScale = builder->getUniformCStr(fSurfaceScaleUni);

    int index = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            SkString texCoords;
            texCoords.appendf("coord + vec2(%d, %d) * %s", dx, dy, imgInc);
            builder->fsCodeAppendf("\t\tm[%d] = ", index++);
            builder->fsAppendTextureLookup(samplers[0], texCoords.c_str());
            builder->fsCodeAppend(".a;\n");
        }
    }
    builder->fsCodeAppend("\t\tvec3 surfaceToLight = ");
    SkString arg;
    arg.appendf("%s * m[4]", surfScale);
    fLight->emitSurfaceToLight(builder, arg.c_str());
    builder->fsCodeAppend(";\n");
    builder->fsCodeAppendf("\t\t%s = %s(%s(m, %s), surfaceToLight, ",
                           outputColor, lightFunc.c_str(), interiorNormalName.c_str(), surfScale);
    fLight->emitLightColor(builder, "surfaceToLight");
    builder->fsCodeAppend(");\n");
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    builder->fsCodeAppend(modulate.c_str());
}

GrGLEffect::EffectKey GrGLLightingEffect::GenKey(const GrDrawEffect& drawEffect,
                                                 const GrGLCaps& caps) {
    return drawEffect.castEffect<GrLightingEffect>().light()->type();
}

void GrGLLightingEffect::setData(const GrGLUniformManager& uman,
                                 const GrDrawEffect& drawEffect) {
    const GrLightingEffect& lighting = drawEffect.castEffect<GrLightingEffect>();
    GrTexture* texture = lighting.texture(0);
    float ySign = texture->origin() == kTopLeft_GrSurfaceOrigin ? -1.0f : 1.0f;
    uman.set2f(fImageIncrementUni, 1.0f / texture->width(), ySign / texture->height());
    uman.set1f(fSurfaceScaleUni, lighting.surfaceScale());
    SkAutoTUnref<SkLight> transformedLight(lighting.light()->transform(lighting.filterMatrix()));
    fLight->setData(uman, transformedLight);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

GrGLDiffuseLightingEffect::GrGLDiffuseLightingEffect(const GrBackendEffectFactory& factory,
                                                     const GrDrawEffect& drawEffect)
    : INHERITED(factory, drawEffect) {
}

void GrGLDiffuseLightingEffect::emitLightFunc(GrGLShaderBuilder* builder, SkString* funcName) {
    const char* kd;
    fKDUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                 kFloat_GrSLType,
                                 "KD",
                                 &kd);

    static const GrGLShaderVar gLightArgs[] = {
        GrGLShaderVar("normal", kVec3f_GrSLType),
        GrGLShaderVar("surfaceToLight", kVec3f_GrSLType),
        GrGLShaderVar("lightColor", kVec3f_GrSLType)
    };
    SkString lightBody;
    lightBody.appendf("\tfloat colorScale = %s * dot(normal, surfaceToLight);\n", kd);
    lightBody.appendf("\treturn vec4(lightColor * clamp(colorScale, 0.0, 1.0), 1.0);\n");
    builder->fsEmitFunction(kVec4f_GrSLType,
                            "light",
                            SK_ARRAY_COUNT(gLightArgs),
                            gLightArgs,
                            lightBody.c_str(),
                            funcName);
}

void GrGLDiffuseLightingEffect::setData(const GrGLUniformManager& uman,
                                        const GrDrawEffect& drawEffect) {
    INHERITED::setData(uman, drawEffect);
    const GrDiffuseLightingEffect& diffuse = drawEffect.castEffect<GrDiffuseLightingEffect>();
    uman.set1f(fKDUni, diffuse.kd());
}

///////////////////////////////////////////////////////////////////////////////

GrSpecularLightingEffect::GrSpecularLightingEffect(GrTexture* texture,
                                                   const SkLight* light,
                                                   SkScalar surfaceScale,
                                                   const SkMatrix& matrix,
                                                   SkScalar ks,
                                                   SkScalar shininess)
    : INHERITED(texture, light, surfaceScale, matrix),
      fKS(ks),
      fShininess(shininess) {
}

const GrBackendEffectFactory& GrSpecularLightingEffect::getFactory() const {
    return GrTBackendEffectFactory<GrSpecularLightingEffect>::getInstance();
}

bool GrSpecularLightingEffect::onIsEqual(const GrEffect& sBase) const {
    const GrSpecularLightingEffect& s = CastEffect<GrSpecularLightingEffect>(sBase);
    return INHERITED::onIsEqual(sBase) &&
           this->ks() == s.ks() &&
           this->shininess() == s.shininess();
}

GR_DEFINE_EFFECT_TEST(GrSpecularLightingEffect);

GrEffectRef* GrSpecularLightingEffect::TestCreate(SkRandom* random,
                                                  GrContext* context,
                                                  const GrDrawTargetCaps&,
                                                  GrTexture* textures[]) {
    SkScalar surfaceScale = random->nextSScalar1();
    SkScalar ks = random->nextUScalar1();
    SkScalar shininess = random->nextUScalar1();
    SkAutoTUnref<SkLight> light(create_random_light(random));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = random->nextUScalar1();
    }
    return GrSpecularLightingEffect::Create(textures[GrEffectUnitTest::kAlphaTextureIdx],
                                            light, surfaceScale, matrix, ks, shininess);
}

///////////////////////////////////////////////////////////////////////////////

GrGLSpecularLightingEffect::GrGLSpecularLightingEffect(const GrBackendEffectFactory& factory,
                                                       const GrDrawEffect& drawEffect)
    : INHERITED(factory, drawEffect) {
}

void GrGLSpecularLightingEffect::emitLightFunc(GrGLShaderBuilder* builder, SkString* funcName) {
    const char* ks;
    const char* shininess;

    fKSUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                 kFloat_GrSLType, "KS", &ks);
    fShininessUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                        kFloat_GrSLType, "Shininess", &shininess);

    static const GrGLShaderVar gLightArgs[] = {
        GrGLShaderVar("normal", kVec3f_GrSLType),
        GrGLShaderVar("surfaceToLight", kVec3f_GrSLType),
        GrGLShaderVar("lightColor", kVec3f_GrSLType)
    };
    SkString lightBody;
    lightBody.appendf("\tvec3 halfDir = vec3(normalize(surfaceToLight + vec3(0, 0, 1)));\n");
    lightBody.appendf("\tfloat colorScale = %s * pow(dot(normal, halfDir), %s);\n", ks, shininess);
    lightBody.appendf("\tvec3 color = lightColor * clamp(colorScale, 0.0, 1.0);\n");
    lightBody.appendf("\treturn vec4(color, max(max(color.r, color.g), color.b));\n");
    builder->fsEmitFunction(kVec4f_GrSLType,
                            "light",
                            SK_ARRAY_COUNT(gLightArgs),
                            gLightArgs,
                            lightBody.c_str(),
                            funcName);
}

void GrGLSpecularLightingEffect::setData(const GrGLUniformManager& uman,
                                         const GrDrawEffect& drawEffect) {
    INHERITED::setData(uman, drawEffect);
    const GrSpecularLightingEffect& spec = drawEffect.castEffect<GrSpecularLightingEffect>();
    uman.set1f(fKSUni, spec.ks());
    uman.set1f(fShininessUni, spec.shininess());
}

///////////////////////////////////////////////////////////////////////////////
void GrGLLight::emitLightColorUniform(GrGLShaderBuilder* builder) {
    fColorUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                    kVec3f_GrSLType, "LightColor");
}

void GrGLLight::emitLightColor(GrGLShaderBuilder* builder,
                               const char *surfaceToLight) {
    builder->fsCodeAppend(builder->getUniformCStr(this->lightColorUni()));
}

void GrGLLight::setData(const GrGLUniformManager& uman,
                        const SkLight* light) const {
    setUniformPoint3(uman, fColorUni, light->color() * SkScalarInvert(SkIntToScalar(255)));
}

///////////////////////////////////////////////////////////////////////////////

void GrGLDistantLight::setData(const GrGLUniformManager& uman,
                               const SkLight* light) const {
    INHERITED::setData(uman, light);
    SkASSERT(light->type() == SkLight::kDistant_LightType);
    const SkDistantLight* distantLight = static_cast<const SkDistantLight*>(light);
    setUniformNormal3(uman, fDirectionUni, distantLight->direction());
}

void GrGLDistantLight::emitSurfaceToLight(GrGLShaderBuilder* builder, const char* z) {
    const char* dir;
    fDirectionUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility, kVec3f_GrSLType,
                                        "LightDirection", &dir);
    builder->fsCodeAppend(dir);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLPointLight::setData(const GrGLUniformManager& uman,
                             const SkLight* light) const {
    INHERITED::setData(uman, light);
    SkASSERT(light->type() == SkLight::kPoint_LightType);
    const SkPointLight* pointLight = static_cast<const SkPointLight*>(light);
    setUniformPoint3(uman, fLocationUni, pointLight->location());
}

void GrGLPointLight::emitSurfaceToLight(GrGLShaderBuilder* builder, const char* z) {
    const char* loc;
    fLocationUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility, kVec3f_GrSLType,
                                       "LightLocation", &loc);
    builder->fsCodeAppendf("normalize(%s - vec3(%s.xy, %s))", loc, builder->fragmentPosition(), z);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLSpotLight::setData(const GrGLUniformManager& uman,
                            const SkLight* light) const {
    INHERITED::setData(uman, light);
    SkASSERT(light->type() == SkLight::kSpot_LightType);
    const SkSpotLight* spotLight = static_cast<const SkSpotLight *>(light);
    setUniformPoint3(uman, fLocationUni, spotLight->location());
    uman.set1f(fExponentUni, spotLight->specularExponent());
    uman.set1f(fCosInnerConeAngleUni, spotLight->cosInnerConeAngle());
    uman.set1f(fCosOuterConeAngleUni, spotLight->cosOuterConeAngle());
    uman.set1f(fConeScaleUni, spotLight->coneScale());
    setUniformNormal3(uman, fSUni, spotLight->s());
}

void GrGLSpotLight::emitSurfaceToLight(GrGLShaderBuilder* builder, const char* z) {
    const char* location;
    fLocationUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                       kVec3f_GrSLType, "LightLocation", &location);
    builder->fsCodeAppendf("normalize(%s - vec3(%s.xy, %s))",
                           location, builder->fragmentPosition(), z);
}

void GrGLSpotLight::emitLightColor(GrGLShaderBuilder* builder,
                                   const char *surfaceToLight) {

    const char* color = builder->getUniformCStr(this->lightColorUni()); // created by parent class.

    const char* exponent;
    const char* cosInner;
    const char* cosOuter;
    const char* coneScale;
    const char* s;
    fExponentUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                       kFloat_GrSLType, "Exponent", &exponent);
    fCosInnerConeAngleUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                kFloat_GrSLType, "CosInnerConeAngle", &cosInner);
    fCosOuterConeAngleUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                kFloat_GrSLType, "CosOuterConeAngle", &cosOuter);
    fConeScaleUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                        kFloat_GrSLType, "ConeScale", &coneScale);
    fSUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                kVec3f_GrSLType, "S", &s);

    static const GrGLShaderVar gLightColorArgs[] = {
        GrGLShaderVar("surfaceToLight", kVec3f_GrSLType)
    };
    SkString lightColorBody;
    lightColorBody.appendf("\tfloat cosAngle = -dot(surfaceToLight, %s);\n", s);
    lightColorBody.appendf("\tif (cosAngle < %s) {\n", cosOuter);
    lightColorBody.appendf("\t\treturn vec3(0);\n");
    lightColorBody.appendf("\t}\n");
    lightColorBody.appendf("\tfloat scale = pow(cosAngle, %s);\n", exponent);
    lightColorBody.appendf("\tif (cosAngle < %s) {\n", cosInner);
    lightColorBody.appendf("\t\treturn %s * scale * (cosAngle - %s) * %s;\n",
                           color, cosOuter, coneScale);
    lightColorBody.appendf("\t}\n");
    lightColorBody.appendf("\treturn %s;\n", color);
    builder->fsEmitFunction(kVec3f_GrSLType,
                            "lightColor",
                            SK_ARRAY_COUNT(gLightColorArgs),
                            gLightColorArgs,
                            lightColorBody.c_str(),
                            &fLightColorFunc);

    builder->fsCodeAppendf("%s(%s)", fLightColorFunc.c_str(), surfaceToLight);
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiffuseLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpecularLightingImageFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
