
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLights_DEFINED
#define SkLights_DEFINED

#include "SkPoint3.h"
#include "SkRefCnt.h"
#include "../private/SkTArray.h"
#include "SkImage.h"

class SK_API SkLights  : public SkRefCnt {
public:
    class Light {
    public:
        enum LightType {
            kAmbient_LightType,       // only 'fColor' is used
            kDirectional_LightType
        };

        Light(const Light& other)
            : fType(other.fType)
            , fColor(other.fColor)
            , fDirection(other.fDirection)
            , fShadowMap(other.fShadowMap) {
        }

        Light(Light&& other)
            : fType(other.fType)
            , fColor(other.fColor)
            , fDirection(other.fDirection)
            , fShadowMap(std::move(other.fShadowMap)) {
        }

        Light(const SkColor3f& color)
            : fType(kAmbient_LightType)
            , fColor(color) {
            fDirection.set(0.0f, 0.0f, 1.0f);
        }

        Light(const SkColor3f& color, const SkVector3& dir)
            : fType(kDirectional_LightType)
            , fColor(color)
            , fDirection(dir) {
            if (!fDirection.normalize()) {
                fDirection.set(0.0f, 0.0f, 1.0f);
            }
        }

        LightType type() const { return fType; }
        const SkColor3f& color() const { return fColor; }
        const SkVector3& dir() const { 
            SkASSERT(kAmbient_LightType != fType);
            return fDirection; 
        }

        void setShadowMap(sk_sp<SkImage> shadowMap) {
            fShadowMap = std::move(shadowMap);
        }

        SkImage* getShadowMap() const {
            return fShadowMap.get();
        }

        Light& operator= (const Light& b) {
            if (this == &b) {
                return *this;
            }

            fColor = b.fColor;
            fType = b.fType;
            fDirection = b.fDirection;
            fShadowMap = b.fShadowMap;
            return *this;
        }

    private:
        LightType   fType;
        SkColor3f   fColor;           // linear (unpremul) color. Range is 0..1 in each channel.
        SkVector3   fDirection;       // direction towards the light (+Z is out of the screen).
                                      // If degenerate, it will be replaced with (0, 0, 1).
        sk_sp<SkImage> fShadowMap;
    };

    class Builder {
    public:
        Builder() : fLights(new SkLights) { }
    
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

private:
    SkLights() {}

    SkTArray<Light> fLights;

    typedef SkRefCnt INHERITED;
};

#endif
