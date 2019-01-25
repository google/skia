
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

class SkColorSpaceXformer;
class SkReadBuffer;
class SkWriteBuffer;

/** \class SkLights
    SkLights encapsulates a set of directional, point and ambient lights for use with the
    SkLightingShader.
*/
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
                , fIntensity(other.fIntensity) {}

        Light(Light&& other)
                : fType(other.fType)
                , fColor(other.fColor)
                , fDirOrPos(other.fDirOrPos)
                , fIntensity(other.fIntensity) {}

        static Light MakeDirectional(const SkColor3f& color, const SkVector3& dir) {
            Light light(kDirectional_LightType, color, dir, 0.0f);
            if (!light.fDirOrPos.normalize()) {
                light.fDirOrPos.set(0.0f, 0.0f, 1.0f);
            }
            return light;
        }

        static Light MakePoint(const SkColor3f& color, const SkPoint3& pos, SkScalar intensity) {
            return Light(kPoint_LightType, color, pos, intensity);
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

        Light& operator=(const Light& other) {
            if (this == &other) {
                return *this;
            }

            fType = other.fType;
            fColor = other.fColor;
            fDirOrPos = other.fDirOrPos;
            fIntensity = other.fIntensity;
            return *this;
        }

        bool operator==(const Light& other) {
            return (fType      == other.fType) &&
                   (fColor     == other.fColor) &&
                   (fDirOrPos  == other.fDirOrPos) &&
                   (fIntensity == other.fIntensity);
        }

        bool operator!=(const Light& other) { return !(this->operator==(other)); }

    private:
        friend class SkLights;

        Light(LightType type, const SkColor3f& color, const SkVector3& dirOrPos,
              SkScalar intensity)
                : fType(type)
                , fColor(color)
                , fDirOrPos(dirOrPos)
                , fIntensity(intensity) {}

        LightType   fType;
        SkColor3f   fColor;           // linear (unpremul) color. Range is 0..1 in each channel.

        SkVector3   fDirOrPos;        // For directional lights, holds the direction towards the
                                      // light (+Z is out of the screen).
                                      // If degenerate, it will be replaced with (0, 0, 1).
                                      // For point lights, holds location of point light

        SkScalar    fIntensity;       // For point lights, dictates the light intensity.
                                      // Simply a multiplier to the final light output value.
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

    /** Returns number of lights not including the ambient light.

        @return number of lights not including the ambient light
    */
    int numLights() const { return fLights.count(); }

    /** Returns the index-th light.

        @param index  the index of the desired light
        @return       the index-th light
    */
    const Light& light(int index) const { return fLights[index]; }

    /** Returns the ambient light.

        @return the ambient light
    */
    const SkColor3f& ambientLightColor() const {
        return fAmbientLightColor;
    }

    /**
     *  Recreate an SkLights object that was serialized into a buffer.
     *
     *  @param  SkReadBuffer Serialized blob data.
     *  @return A new SkLights representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static sk_sp<SkLights> MakeFromBuffer(SkReadBuffer& buf);

    /**
     *  Serialize to a buffer.
     *
     *  @param  buffer the write buffer to write out to
     */
    void flatten(SkWriteBuffer& buf) const;

private:
    friend class SkLightingShaderImpl;

    SkLights() : fAmbientLightColor(SkColor3f::Make(0.0f, 0.0f, 0.0f)) {}

    sk_sp<SkLights> makeColorSpace(SkColorSpaceXformer* xformer) const;

    SkTArray<Light> fLights;
    SkColor3f       fAmbientLightColor;

    typedef SkRefCnt INHERITED;
};

#endif
