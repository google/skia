/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkShadowShader_DEFINED
#define SkShadowShader_DEFINED

#ifdef SK_EXPERIMENTAL_SHADOWING

class SkLights;
class SkShader;

class SK_API SkShadowShader {
public:
    /** This shader combines the diffuse color in 'diffuseShader' with the shadows
     *  determined by the 'povDepthShader' and the shadow maps stored in each of the
     *  lights in 'lights'
     *
     *  Please note that the shadow shader is required to be in Stage0, otherwise
     *  the texture coords will be wrong within the shader.
     */
    static sk_sp<SkShader> Make(sk_sp<SkShader> povDepthShader,
                                sk_sp<SkShader> diffuseShader,
                                sk_sp<SkLights> lights,
                                int diffuseWidth, int diffuseHeight,
                                const SkShadowParams& params);

    // The shadow shader supports any number of ambient lights, but only
    // 4 non-ambient lights (currently just refers to directional lights).
    static constexpr int kMaxNonAmbientLights = 4;

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
#endif
