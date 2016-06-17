/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLightingShader_DEFINED
#define SkLightingShader_DEFINED

#include "SkFlattenable.h"
#include "SkLight.h"
#include "SkShader.h"
#include "SkTDArray.h"

class SkBitmap;
class SkMatrix;

class SK_API SkLightingShader {
public:
    class Lights  : public SkRefCnt {
    public:
        class Builder {
        public:
            Builder(const SkLight lights[], int numLights)
                : fLights(new Lights(lights, numLights)) {}

            Builder() : fLights(new Lights) {}

            // TODO: limit the number of lights here or just ignore those
            // above some maximum?
            void add(const SkLight& light) {
                if (fLights) {
                    *fLights->fLights.push() = light;
                }
            }

            const Lights* finish() {
                return fLights.release();
            }

        private:
            SkAutoTUnref<Lights> fLights;
        };

        int numLights() const {
            return fLights.count();
        }

        const SkLight& light(int index) const {
            return fLights[index];
        }

    private:
        Lights() {}
        Lights(const SkLight lights[], int numLights) : fLights(lights, numLights) {}

        SkTDArray<SkLight> fLights;

        typedef SkRefCnt INHERITED;
    };

    /** Returns a shader that lights the diffuse and normal maps with a single light.

        It returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  diffuse     the diffuse bitmap
        @param  normal      the normal map
        @param  light       the light applied to the normal map
        @param  ambient     the linear (unpremul) ambient light color. Range is 0..1/channel.
        @param  localMatrix the matrix mapping the textures to the dest rect

        nullptr will be returned if:
            either 'diffuse' or 'normal' are empty
            either 'diffuse' or 'normal' are too big (> 65535 on a side)
            'diffuse' and 'normal' aren't the same size

        The lighting equation is currently:
            result = LightColor * DiffuseColor * (Normal * LightDir) + AmbientColor

        The normal map is currently assumed to be an 8888 image where the normal at a texel
        is retrieved by:
            N.x = R-127;
            N.y = G-127;
            N.z = B-127;
            N.normalize();
        The +Z axis is thus encoded in RGB as (127, 127, 255) while the -Z axis is
        (127, 127, 0).
    */
    static sk_sp<SkShader> Make(const SkBitmap& diffuse, const SkBitmap& normal,
                                const Lights* lights, const SkVector& invNormRotation,
                                const SkMatrix* diffLocalMatrix, const SkMatrix* normLocalMatrix);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
