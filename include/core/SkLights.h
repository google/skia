
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLights_DEFINED
#define SkLights_DEFINED

#include "../private/SkTArray.h"
#include "SkImage.h"
#include "SkPoint3.h"
#include "SkRefCnt.h"

class SkReadBuffer;
class SkWriteBuffer;

class SK_API SkLights  : public SkRefCnt {
public:
    class Light {
    public:
        enum LightType {
            kDirectional_LightType,
            kPoint_LightType
        };

        Light(const Light& other)
            : fType(other.fType)
            , fColor(other.fColor)
            , fDirOrPos(other.fDirOrPos)
            , fIntensity(other.fIntensity)
            , fShadowMap(other.fShadowMap)
            , fIsRadial(other.fIsRadial) {
        }

        Light(Light&& other)
            : fType(other.fType)
            , fColor(other.fColor)
            , fDirOrPos(other.fDirOrPos)
            , fIntensity(other.fIntensity)
            , fShadowMap(std::move(other.fShadowMap))
            , fIsRadial(other.fIsRadial)  {
        }

        static Light MakeDirectional(const SkColor3f& color, const SkVector3& dir,
                                     bool isRadial = false) {
            Light light(kDirectional_LightType, color, dir, isRadial);
            if (!light.fDirOrPos.normalize()) {
                light.fDirOrPos.set(0.0f, 0.0f, 1.0f);
            }
            return light;
        }

        static Light MakePoint(const SkColor3f& color, const SkPoint3& pos, SkScalar intensity,
                               bool isRadial = false) {
            return Light(kPoint_LightType, color, pos, intensity, isRadial);
        }

        LightType type() const { return fType; }
        const SkColor3f& color() const { return fColor; }
        const SkVector3& dir() const {
            SkASSERT(kDirectional_LightType == fType);
            return fDirOrPos;
        }
        const SkPoint3& pos() const {
            SkASSERT(kPoint_LightType == fType);
            return fDirOrPos;
        }
        SkScalar intensity() const {
            SkASSERT(kPoint_LightType == fType);
            return fIntensity;
        }

        void setShadowMap(sk_sp<SkImage> shadowMap) {
            fShadowMap = std::move(shadowMap);
        }

        SkImage* getShadowMap() const {
            return fShadowMap.get();
        }

        bool isRadial() const { return fIsRadial; }

        Light& operator= (const Light& b) {
            if (this == &b) {
                return *this;
            }

            fColor = b.fColor;
            fType = b.fType;
            fDirOrPos = b.fDirOrPos;
            fIntensity = b.fIntensity;
            fShadowMap = b.fShadowMap;
            fIsRadial = b.fIsRadial;
            return *this;
        }

        bool operator== (const Light& b) {
            if (this == &b) {
                return true;
            }

            return (fColor     == b.fColor) &&
                   (fType      == b.fType) &&
                   (fDirOrPos  == b.fDirOrPos) &&
                   (fShadowMap == b.fShadowMap) &&
                   (fIntensity == b.fIntensity) &&
                   (fIsRadial  == b.fIsRadial);
        }

        bool operator!= (const Light& b) { return !(this->operator==(b)); }

    private:
        friend class SkLights;

        LightType   fType;
        SkColor3f   fColor;           // linear (unpremul) color. Range is 0..1 in each channel.

        SkVector3   fDirOrPos;        // For directional lights, holds the direction towards the
                                      // light (+Z is out of the screen).
                                      // If degenerate, it will be replaced with (0, 0, 1).
                                      // For point lights, holds location of point light

        SkScalar    fIntensity;       // For point lights, dictates the light intensity.
                                      // Simply a multiplier to the final light output value.
        sk_sp<SkImage> fShadowMap;
        bool        fIsRadial;        // Whether the light is radial or not. Radial lights will
                                      // cast shadows and lights radially outwards.

        Light(LightType type, const SkColor3f& color, const SkVector3& dirOrPos,
              SkScalar intensity = 0.0f, bool isRadial = false) {
            fType = type;
            fColor = color;
            fDirOrPos = dirOrPos;
            fIntensity = intensity;
            fIsRadial = isRadial;
        }
    };

    class Builder {
    public:
        Builder() : fLights(new SkLights) {}

        void add(const Light& light) {
            if (fLights) {
                fLights->fLights.push_back(light);
            }
        }

        void add(Light&& light) {
            if (fLights) {
                fLights->fLights.push_back(std::move(light));
            }
        }

        void setAmbientLightColor(const SkColor3f& color) {
            if (fLights) {
                fLights->fAmbientLightColor = color;
            }
        }

        sk_sp<SkLights> finish() {
            return std::move(fLights);
        }

    private:
        sk_sp<SkLights> fLights;
    };

    int numLights() const {
        return fLights.count();
    }

    const Light& light(int index) const {
        return fLights[index];
    }

    Light& light(int index) {
        return fLights[index];
    }

    const SkColor3f& ambientLightColor() const {
        return fAmbientLightColor;
    }

    static sk_sp<SkLights> MakeFromBuffer(SkReadBuffer& buf);

    void flatten(SkWriteBuffer& buf) const;

private:
    SkLights() {
        fAmbientLightColor.set(0.0f, 0.0f, 0.0f);
    }

    friend class SkLightingShaderImpl;
    sk_sp<SkLights> makeColorSpace(SkColorSpaceXformer* xformer) const;

    SkTArray<Light> fLights;
    SkColor3f fAmbientLightColor;
    typedef SkRefCnt INHERITED;
};

#endif
