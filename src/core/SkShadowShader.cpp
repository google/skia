/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkShadowShader.h"

////////////////////////////////////////////////////////////////////////////
#ifdef SK_EXPERIMENTAL_SHADOWING


/** \class SkShadowShaderImpl
    This subclass of shader applies shadowing
*/
class SkShadowShaderImpl : public SkShader {
public:
    /** Create a new shadowing shader that shadows
        @param to do        to do
    */
    SkShadowShaderImpl(sk_sp<SkShader> povDepthShader,
                       sk_sp<SkShader> diffuseShader,
                       sk_sp<SkLights> lights,
                       int diffuseWidth, int diffuseHeight,
                       const SkShadowParams& params)
            : fPovDepthShader(std::move(povDepthShader))
            , fDiffuseShader(std::move(diffuseShader))
            , fLights(std::move(lights))
            , fDiffuseWidth(diffuseWidth)
            , fDiffuseHeight(diffuseHeight)
            , fShadowParams(params) { }

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    class ShadowShaderContext : public SkShader::Context {
    public:
        // The context takes ownership of the states. It will call their destructors
        // but will NOT free the memory.
        ShadowShaderContext(const SkShadowShaderImpl&, const ContextRec&,
                            SkShader::Context* povDepthContext,
                            SkShader::Context* diffuseContext,
                            void* heapAllocated);

        ~ShadowShaderContext() override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        uint32_t getFlags() const override { return fFlags; }

    private:
        SkShader::Context*        fPovDepthContext;
        SkShader::Context*        fDiffuseContext;
        uint32_t                  fFlags;

        void* fHeapAllocated;

        int fNonAmbLightCnt;
        SkPixmap* fShadowMapPixels;


        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkShadowShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void*) const override;

private:
    sk_sp<SkShader> fPovDepthShader;
    sk_sp<SkShader> fDiffuseShader;
    sk_sp<SkLights> fLights;

    int fDiffuseWidth;
    int fDiffuseHeight;

    SkShadowParams fShadowParams;

    friend class SkShadowShader;

    typedef SkShader INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"
#include "SkSpecialImage.h"
#include "SkImage_Base.h"
#include "GrContext.h"

class ShadowFP : public GrFragmentProcessor {
public:
    ShadowFP(sk_sp<GrFragmentProcessor> povDepth,
             sk_sp<GrFragmentProcessor> diffuse,
             sk_sp<SkLights> lights,
             int diffuseWidth, int diffuseHeight,
             const SkShadowParams& params,
             GrContext* context) {

        fAmbientColor = lights->ambientLightColor();

        fNumNonAmbLights = 0; // count of non-ambient lights
        for (int i = 0; i < lights->numLights(); ++i) {
            if (fNumNonAmbLights < SkShadowShader::kMaxNonAmbientLights) {
                fLightColor[fNumNonAmbLights] = lights->light(i).color();

                if (SkLights::Light::kPoint_LightType == lights->light(i).type()) {
                    fLightDirOrPos[fNumNonAmbLights] = lights->light(i).pos();
                    fLightColor[fNumNonAmbLights].scale(lights->light(i).intensity());
                } else {
                    fLightDirOrPos[fNumNonAmbLights] = lights->light(i).dir();
                }

                fIsPointLight[fNumNonAmbLights] =
                        SkLights::Light::kPoint_LightType == lights->light(i).type();

                fIsRadialLight[fNumNonAmbLights] = lights->light(i).isRadial();

                SkImage_Base* shadowMap = ((SkImage_Base*)lights->light(i).getShadowMap());

                // gets deleted when the ShadowFP is destroyed, and frees the GrTexture*
                fTexture[fNumNonAmbLights] = sk_sp<GrTexture>(shadowMap->asTextureRef(context,
                                                           GrSamplerParams::ClampNoFilter(),
                                                           SkDestinationSurfaceColorMode::kLegacy,
                                                           nullptr));
                fDepthMapSampler[fNumNonAmbLights].reset(fTexture[fNumNonAmbLights].get());
                this->addTextureSampler(&fDepthMapSampler[fNumNonAmbLights]);

                fDepthMapHeight[fNumNonAmbLights] = shadowMap->height();
                fDepthMapWidth[fNumNonAmbLights] = shadowMap->width();

                fNumNonAmbLights++;
            }
        }

        fWidth = diffuseWidth;
        fHeight = diffuseHeight;

        fShadowParams = params;

        this->registerChildProcessor(std::move(povDepth));
        this->registerChildProcessor(std::move(diffuse));
        this->initClassID<ShadowFP>();
    }

    class GLSLShadowFP : public GrGLSLFragmentProcessor {
    public:
        GLSLShadowFP() { }

        void emitCode(EmitArgs& args) override {
            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
            const ShadowFP& shadowFP = args.fFp.cast<ShadowFP>();

            SkASSERT(shadowFP.fNumNonAmbLights <= SkShadowShader::kMaxNonAmbientLights);

            // add uniforms
            int32_t numLights = shadowFP.fNumNonAmbLights;
            SkASSERT(numLights <= SkShadowShader::kMaxNonAmbientLights);

            int blurAlgorithm = shadowFP.fShadowParams.fType;

            const char* lightDirOrPosUniName[SkShadowShader::kMaxNonAmbientLights] = {nullptr};
            const char* lightColorUniName[SkShadowShader::kMaxNonAmbientLights] = {nullptr};
            const char* ambientColorUniName = nullptr;

            const char* depthMapWidthUniName[SkShadowShader::kMaxNonAmbientLights] = {nullptr};
            const char* depthMapHeightUniName[SkShadowShader::kMaxNonAmbientLights] = {nullptr};
            const char* widthUniName = nullptr; // dimensions of povDepth
            const char* heightUniName = nullptr;

            const char* shBiasUniName = nullptr;
            const char* minVarianceUniName = nullptr;

            // setting uniforms
            for (int i = 0; i < shadowFP.fNumNonAmbLights; i++) {
                SkString lightDirOrPosUniNameStr("lightDir");
                lightDirOrPosUniNameStr.appendf("%d", i);
                SkString lightColorUniNameStr("lightColor");
                lightColorUniNameStr.appendf("%d", i);
                SkString lightIntensityUniNameStr("lightIntensity");
                lightIntensityUniNameStr.appendf("%d", i);

                SkString depthMapWidthUniNameStr("dmapWidth");
                depthMapWidthUniNameStr.appendf("%d", i);
                SkString depthMapHeightUniNameStr("dmapHeight");
                depthMapHeightUniNameStr.appendf("%d", i);

                fLightDirOrPosUni[i] = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                             kVec3f_GrSLType,
                                                             kDefault_GrSLPrecision,
                                                             lightDirOrPosUniNameStr.c_str(),
                                                             &lightDirOrPosUniName[i]);
                fLightColorUni[i] = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                               kVec3f_GrSLType,
                                                               kDefault_GrSLPrecision,
                                                               lightColorUniNameStr.c_str(),
                                                               &lightColorUniName[i]);

                fDepthMapWidthUni[i]  = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kInt_GrSLType,
                                                   kDefault_GrSLPrecision,
                                                   depthMapWidthUniNameStr.c_str(),
                                                   &depthMapWidthUniName[i]);
                fDepthMapHeightUni[i] = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kInt_GrSLType,
                                                   kDefault_GrSLPrecision,
                                                   depthMapHeightUniNameStr.c_str(),
                                                   &depthMapHeightUniName[i]);
            }

            fBiasingConstantUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                             kFloat_GrSLType,
                                                             kDefault_GrSLPrecision,
                                                             "shadowBias", &shBiasUniName);
            fMinVarianceUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                         kFloat_GrSLType,
                                                         kDefault_GrSLPrecision,
                                                         "minVariance", &minVarianceUniName);

            fWidthUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kInt_GrSLType,
                                                   kDefault_GrSLPrecision,
                                                   "width", &widthUniName);
            fHeightUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kInt_GrSLType,
                                                    kDefault_GrSLPrecision,
                                                    "height", &heightUniName);

            fAmbientColorUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                          kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                          "AmbientColor", &ambientColorUniName);

            SkString povDepthSampler("_povDepth");
            SkString povDepth("povDepth");
            this->emitChild(0, nullptr, &povDepthSampler, args);
            fragBuilder->codeAppendf("vec4 %s = %s;", povDepth.c_str(), povDepthSampler.c_str());

            SkString diffuseColorSampler("_inDiffuseColor");
            SkString diffuseColor("inDiffuseColor");
            this->emitChild(1, nullptr, &diffuseColorSampler, args);
            fragBuilder->codeAppendf("vec4 %s = %s;", diffuseColor.c_str(),
                                     diffuseColorSampler.c_str());

            SkString depthMaps[SkShadowShader::kMaxNonAmbientLights];

            fragBuilder->codeAppendf("vec4 resultDiffuseColor = %s;", diffuseColor.c_str());
            fragBuilder->codeAppend ("vec3 totalLightColor = vec3(0);");

            // probability that a fragment is lit. For each light, we multiply this by the
            // light's color to get its contribution to totalLightColor.
            fragBuilder->codeAppend ("float lightProbability;");

            // coordinates of current fragment in world space
            fragBuilder->codeAppend ("vec3 worldCor;");

            // Multiply by 255 to transform from sampler coordinates to world
            // coordinates (since 1 channel is 0xFF)
            // Note: vMatrixCoord_0_1_Stage0 is the texture sampler coordinates.
            fragBuilder->codeAppendf("worldCor = vec3(vMatrixCoord_0_1_Stage0 * "
                                                "vec2(%s, %s), %s.b * 255);",
                                     widthUniName, heightUniName, povDepth.c_str());

            // Applies the offset indexing that goes from our view space into the light's space.
            for (int i = 0; i < shadowFP.fNumNonAmbLights; i++) {
                SkString povCoord("povCoord");
                povCoord.appendf("%d", i);

                SkString offset("offset");
                offset.appendf("%d", i);
                fragBuilder->codeAppendf("vec2 %s;", offset.c_str());

                if (shadowFP.fIsPointLight[i]) {
                    fragBuilder->codeAppendf("vec3 fragToLight%d = %s - worldCor;",
                                             i, lightDirOrPosUniName[i]);
                    fragBuilder->codeAppendf("float dist%d = length(fragToLight%d);",
                                             i, i);
                    fragBuilder->codeAppendf("%s = vec2(-fragToLight%d) * povDepth.b;",
                                             offset.c_str(), i);
                    fragBuilder->codeAppendf("fragToLight%d = normalize(fragToLight%d);",
                                             i, i);
                }

                if (shadowFP.fIsRadialLight[i]) {
                    fragBuilder->codeAppendf("vec2 %s = vec2(vMatrixCoord_0_1_Stage0.x, "
                                                            "1 - vMatrixCoord_0_1_Stage0.y);\n",
                                             povCoord.c_str());

                    fragBuilder->codeAppendf("%s = (%s) * 2.0 - 1.0 + (vec2(%s)/vec2(%s,%s) - 0.5)"
                                                                      "* vec2(-2.0, 2.0);\n",
                                             povCoord.c_str(), povCoord.c_str(),
                                             lightDirOrPosUniName[i],
                                             widthUniName, heightUniName);

                    fragBuilder->codeAppendf("float theta = atan(%s.y, %s.x);",
                                             povCoord.c_str(), povCoord.c_str());
                    fragBuilder->codeAppendf("float r = length(%s);", povCoord.c_str());

                    // map output of atan to [0, 1]
                    fragBuilder->codeAppendf("%s.x = (theta + 3.1415) / (2.0 * 3.1415);",
                                             povCoord.c_str());
                    fragBuilder->codeAppendf("%s.y = 0.0;", povCoord.c_str());
                } else {
                    // note that we flip the y-coord of the offset and then later add
                    // a value just to the y-coord of povCoord. This is to account for
                    // the shifted origins from switching from raster into GPU.
                    if (shadowFP.fIsPointLight[i]) {
                        // the 0.375s are precalculated transform values, given that the depth
                        // maps for pt lights are 4x the size (linearly) as diffuse maps.
                        // The vec2(0.375, -0.375) is used to transform us to
                        // the center of the map.
                        fragBuilder->codeAppendf("vec2 %s = ((vec2(%s, %s) *"
                                                         "vMatrixCoord_0_1_Stage0 +"
                                                         "vec2(0,%s - %s)"
                                                         "+ %s) / (vec2(%s, %s))) +"
                                                         "vec2(0.375, -0.375);",
                                                 povCoord.c_str(),
                                                 widthUniName, heightUniName,
                                                 depthMapHeightUniName[i], heightUniName,
                                                 offset.c_str(),
                                                 depthMapWidthUniName[i],
                                                 depthMapWidthUniName[i]);
                    } else {
                        fragBuilder->codeAppendf("%s = vec2(%s) * povDepth.b * "
                                                      "vec2(255.0, -255.0);",
                                                 offset.c_str(), lightDirOrPosUniName[i]);

                        fragBuilder->codeAppendf("vec2 %s = ((vec2(%s, %s) *"
                                                         "vMatrixCoord_0_1_Stage0 +"
                                                         "vec2(0,%s - %s)"
                                                         "+ %s) / vec2(%s, %s));",
                                                 povCoord.c_str(),
                                                 widthUniName, heightUniName,
                                                 depthMapHeightUniName[i], heightUniName,
                                                 offset.c_str(),
                                                 depthMapWidthUniName[i],
                                                 depthMapWidthUniName[i]);
                    }
                }

                fragBuilder->appendTextureLookup(&depthMaps[i], args.fTexSamplers[i],
                                                 povCoord.c_str(),
                                                 kVec2f_GrSLType);
            }

            // helper variables for calculating shadowing

            // variance of depth at this fragment in the context of surrounding area
            // (area size and weighting dependent on blur size and type)
            fragBuilder->codeAppendf("float variance;");

            // the difference in depth between the user POV and light POV.
            fragBuilder->codeAppendf("float d;");

            // add up light contributions from all lights to totalLightColor
            for (int i = 0; i < numLights; i++) {
                fragBuilder->codeAppendf("lightProbability = 1;");

                if (shadowFP.fIsRadialLight[i]) {
                    fragBuilder->codeAppend("totalLightColor = vec3(0);");

                    fragBuilder->codeAppend("vec2 tc = vec2(povCoord0.x, 0.0);");
                    fragBuilder->codeAppend("float depth = texture(uTextureSampler0_Stage1,"
                                                                  "povCoord0).b * 2.0;");

                    fragBuilder->codeAppendf("lightProbability = step(r, depth);");

                    // 2 is the maximum depth. If this is reached, probably we have
                    // not intersected anything. So values after this should be unshadowed.
                    fragBuilder->codeAppendf("if (%s.b != 0 || depth == 2) {"
                                                     "lightProbability = 1.0; }",
                                             povDepth.c_str());
                } else {
                    // 1/512 == .00195... is less than half a pixel; imperceptible
                    fragBuilder->codeAppendf("if (%s.b <= %s.b + .001953125) {",
                                             povDepth.c_str(), depthMaps[i].c_str());
                    if (blurAlgorithm == SkShadowParams::kVariance_ShadowType) {
                        // We mess with depth and depth^2 in their given scales.
                        // (i.e. between 0 and 1)
                        fragBuilder->codeAppendf("vec2 moments%d = vec2(%s.b, %s.g);",
                                                 i, depthMaps[i].c_str(), depthMaps[i].c_str());

                        // variance biasing lessens light bleeding
                        fragBuilder->codeAppendf("variance = max(moments%d.y - "
                                                         "(moments%d.x * moments%d.x),"
                                                         "%s);", i, i, i,
                                                 minVarianceUniName);

                        fragBuilder->codeAppendf("d = (%s.b) - moments%d.x;",
                                                 povDepth.c_str(), i);
                        fragBuilder->codeAppendf("lightProbability = "
                                                         "(variance / (variance + d * d));");

                        SkString clamp("clamp");
                        clamp.appendf("%d", i);

                        // choosing between light artifacts or correct shape shadows
                        // linstep
                        fragBuilder->codeAppendf("float %s = clamp((lightProbability - %s) /"
                                                         "(1 - %s), 0, 1);",
                                                 clamp.c_str(), shBiasUniName, shBiasUniName);

                        fragBuilder->codeAppendf("lightProbability = %s;", clamp.c_str());
                    } else {
                        fragBuilder->codeAppendf("if (%s.b >= %s.b) {",
                                                 povDepth.c_str(), depthMaps[i].c_str());
                        fragBuilder->codeAppendf("lightProbability = 1;");
                        fragBuilder->codeAppendf("} else { lightProbability = 0; }");
                    }

                    // VSM: The curved shadows near plane edges are artifacts from blurring
                    // lightDir.z is equal to the lightDir dot the surface normal.
                    fragBuilder->codeAppendf("}");
                }

                if (shadowFP.isPointLight(i)) {
                    fragBuilder->codeAppendf("totalLightColor += max(fragToLight%d.z, 0) * %s /"
                                                     "(1 + dist%d) * lightProbability;",
                                             i, lightColorUniName[i], i);
                } else {
                    fragBuilder->codeAppendf("totalLightColor += %s.z * %s * lightProbability;",
                                             lightDirOrPosUniName[i],
                                             lightColorUniName[i]);
                }

                fragBuilder->codeAppendf("totalLightColor += %s;", ambientColorUniName);
                fragBuilder->codeAppendf("%s = resultDiffuseColor * vec4(totalLightColor, 1);",
                                         args.fOutputColor);
            }

        }

        static void GenKey(const GrProcessor& proc, const GrShaderCaps&,
                           GrProcessorKeyBuilder* b) {
            const ShadowFP& shadowFP = proc.cast<ShadowFP>();
            b->add32(shadowFP.fNumNonAmbLights);
            int isPLR = 0;
            for (int i = 0; i < SkShadowShader::kMaxNonAmbientLights; i++) {
                isPLR = isPLR | ((shadowFP.fIsPointLight[i] ? 1 : 0) << i);
                isPLR = isPLR | ((shadowFP.fIsRadialLight[i] ? 1 : 0) << (i+4));
            }
            b->add32(isPLR);
            b->add32(shadowFP.fShadowParams.fType);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const ShadowFP &shadowFP = proc.cast<ShadowFP>();

            for (int i = 0; i < shadowFP.numLights(); i++) {
                const SkVector3& lightDirOrPos = shadowFP.lightDirOrPos(i);
                if (lightDirOrPos != fLightDirOrPos[i]) {
                    pdman.set3fv(fLightDirOrPosUni[i], 1, &lightDirOrPos.fX);
                    fLightDirOrPos[i] = lightDirOrPos;
                }

                const SkColor3f& lightColor = shadowFP.lightColor(i);
                if (lightColor != fLightColor[i]) {
                    pdman.set3fv(fLightColorUni[i], 1, &lightColor.fX);
                    fLightColor[i] = lightColor;
                }

                int depthMapWidth = shadowFP.depthMapWidth(i);
                if (depthMapWidth != fDepthMapWidth[i]) {
                    pdman.set1i(fDepthMapWidthUni[i], depthMapWidth);
                    fDepthMapWidth[i] = depthMapWidth;
                }
                int depthMapHeight = shadowFP.depthMapHeight(i);
                if (depthMapHeight != fDepthMapHeight[i]) {
                    pdman.set1i(fDepthMapHeightUni[i], depthMapHeight);
                    fDepthMapHeight[i] = depthMapHeight;
                }
            }

            SkScalar biasingConstant = shadowFP.shadowParams().fBiasingConstant;
            if (biasingConstant != fBiasingConstant) {
                pdman.set1f(fBiasingConstantUni, biasingConstant);
                fBiasingConstant = biasingConstant;
            }

            SkScalar minVariance = shadowFP.shadowParams().fMinVariance;
            if (minVariance != fMinVariance) {
                // transform variance from pixel-scale to normalized scale
                pdman.set1f(fMinVarianceUni, minVariance / 65536.0f);
                fMinVariance = minVariance / 65536.0f;
            }

            int width = shadowFP.width();
            if (width != fWidth) {
                pdman.set1i(fWidthUni, width);
                fWidth = width;
            }
            int height = shadowFP.height();
            if (height != fHeight) {
                pdman.set1i(fHeightUni, height);
                fHeight = height;
            }

            const SkColor3f& ambientColor = shadowFP.ambientColor();
            if (ambientColor != fAmbientColor) {
                pdman.set3fv(fAmbientColorUni, 1, &ambientColor.fX);
                fAmbientColor = ambientColor;
            }
        }

    private:
        SkVector3 fLightDirOrPos[SkShadowShader::kMaxNonAmbientLights];
        GrGLSLProgramDataManager::UniformHandle
                fLightDirOrPosUni[SkShadowShader::kMaxNonAmbientLights];

        SkColor3f fLightColor[SkShadowShader::kMaxNonAmbientLights];
        GrGLSLProgramDataManager::UniformHandle
                fLightColorUni[SkShadowShader::kMaxNonAmbientLights];

        int fDepthMapWidth[SkShadowShader::kMaxNonAmbientLights];
        GrGLSLProgramDataManager::UniformHandle
                fDepthMapWidthUni[SkShadowShader::kMaxNonAmbientLights];

        int fDepthMapHeight[SkShadowShader::kMaxNonAmbientLights];
        GrGLSLProgramDataManager::UniformHandle
                fDepthMapHeightUni[SkShadowShader::kMaxNonAmbientLights];

        int fWidth;
        GrGLSLProgramDataManager::UniformHandle fWidthUni;
        int fHeight;
        GrGLSLProgramDataManager::UniformHandle fHeightUni;

        SkScalar fBiasingConstant;
        GrGLSLProgramDataManager::UniformHandle fBiasingConstantUni;
        SkScalar fMinVariance;
        GrGLSLProgramDataManager::UniformHandle fMinVarianceUni;

        SkColor3f fAmbientColor;
        GrGLSLProgramDataManager::UniformHandle fAmbientColorUni;
    };

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLShadowFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "shadowFP"; }

    int32_t numLights() const { return fNumNonAmbLights; }
    const SkColor3f& ambientColor() const { return fAmbientColor; }
    bool isPointLight(int i) const {
        SkASSERT(i < fNumNonAmbLights);
        return fIsPointLight[i];
    }
    bool isRadialLight(int i) const {
        SkASSERT(i < fNumNonAmbLights);
        return fIsRadialLight[i];
    }
    const SkVector3& lightDirOrPos(int i) const {
        SkASSERT(i < fNumNonAmbLights);
        return fLightDirOrPos[i];
    }
    const SkVector3& lightColor(int i) const {
        SkASSERT(i < fNumNonAmbLights);
        return fLightColor[i];
    }
    int depthMapWidth(int i) const {
        SkASSERT(i < fNumNonAmbLights);
        return fDepthMapWidth[i];
    }
    int depthMapHeight(int i) const {
        SkASSERT(i < fNumNonAmbLights);
        return fDepthMapHeight[i];
    }
    int width() const {return fWidth; }
    int height() const {return fHeight; }

    const SkShadowParams& shadowParams() const {return fShadowParams; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLShadowFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const ShadowFP& shadowFP = proc.cast<ShadowFP>();
        if (fAmbientColor != shadowFP.fAmbientColor ||
            fNumNonAmbLights != shadowFP.fNumNonAmbLights) {
            return false;
        }

        if (fWidth != shadowFP.fWidth || fHeight != shadowFP.fHeight) {
            return false;
        }

        for (int i = 0; i < fNumNonAmbLights; i++) {
            if (fLightDirOrPos[i] != shadowFP.fLightDirOrPos[i] ||
                fLightColor[i] != shadowFP.fLightColor[i] ||
                fIsPointLight[i] != shadowFP.fIsPointLight[i] ||
                fIsRadialLight[i] != shadowFP.fIsRadialLight[i]) {
                return false;
            }

            if (fDepthMapWidth[i] != shadowFP.fDepthMapWidth[i] ||
                fDepthMapHeight[i] != shadowFP.fDepthMapHeight[i]) {
                return false;
            }
        }

        return true;
    }

    int              fNumNonAmbLights;

    bool             fIsPointLight[SkShadowShader::kMaxNonAmbientLights];
    bool             fIsRadialLight[SkShadowShader::kMaxNonAmbientLights];
    SkVector3        fLightDirOrPos[SkShadowShader::kMaxNonAmbientLights];
    SkColor3f        fLightColor[SkShadowShader::kMaxNonAmbientLights];
    TextureSampler   fDepthMapSampler[SkShadowShader::kMaxNonAmbientLights];
    sk_sp<GrTexture> fTexture[SkShadowShader::kMaxNonAmbientLights];

    int              fDepthMapWidth[SkShadowShader::kMaxNonAmbientLights];
    int              fDepthMapHeight[SkShadowShader::kMaxNonAmbientLights];

    int              fHeight;
    int              fWidth;

    SkShadowParams   fShadowParams;

    SkColor3f        fAmbientColor;
};

////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkShadowShaderImpl::asFragmentProcessor(const AsFPArgs& fpargs) const {

    sk_sp<GrFragmentProcessor> povDepthFP = fPovDepthShader->asFragmentProcessor(fpargs);

    sk_sp<GrFragmentProcessor> diffuseFP = fDiffuseShader->asFragmentProcessor(fpargs);

    sk_sp<GrFragmentProcessor> shadowfp = sk_make_sp<ShadowFP>(std::move(povDepthFP),
                                                               std::move(diffuseFP),
                                                               std::move(fLights),
                                                               fDiffuseWidth, fDiffuseHeight,
                                                               fShadowParams, fpargs.fContext);
    return shadowfp;
}


#endif

////////////////////////////////////////////////////////////////////////////

bool SkShadowShaderImpl::isOpaque() const {
    return fDiffuseShader->isOpaque();
}

SkShadowShaderImpl::ShadowShaderContext::ShadowShaderContext(
        const SkShadowShaderImpl& shader, const ContextRec& rec,
        SkShader::Context* povDepthContext,
        SkShader::Context* diffuseContext,
        void* heapAllocated)
        : INHERITED(shader, rec)
        , fPovDepthContext(povDepthContext)
        , fDiffuseContext(diffuseContext)
        , fHeapAllocated(heapAllocated) {
    bool isOpaque = shader.isOpaque();

    // update fFlags
    uint32_t flags = 0;
    if (isOpaque && (255 == this->getPaintAlpha())) {
        flags |= kOpaqueAlpha_Flag;
    }

    fFlags = flags;

    const SkShadowShaderImpl& lightShader = static_cast<const SkShadowShaderImpl&>(fShader);

    fNonAmbLightCnt = lightShader.fLights->numLights();
    fShadowMapPixels = new SkPixmap[fNonAmbLightCnt];

    for (int i = 0; i < fNonAmbLightCnt; i++) {
        if (lightShader.fLights->light(i).type() == SkLights::Light::kDirectional_LightType) {
            lightShader.fLights->light(i).getShadowMap()->
                    peekPixels(&fShadowMapPixels[i]);
        }
    }
}

SkShadowShaderImpl::ShadowShaderContext::~ShadowShaderContext() {
    delete[] fShadowMapPixels;

    // The dependencies have been created outside of the context on memory that was allocated by
    // the onCreateContext() method. Call the destructors and free the memory.
    fPovDepthContext->~Context();
    fDiffuseContext->~Context();

    sk_free(fHeapAllocated);
}

static inline SkPMColor convert(SkColor3f color, U8CPU a) {
    if (color.fX <= 0.0f) {
        color.fX = 0.0f;
    } else if (color.fX >= 255.0f) {
        color.fX = 255.0f;
    }

    if (color.fY <= 0.0f) {
        color.fY = 0.0f;
    } else if (color.fY >= 255.0f) {
        color.fY = 255.0f;
    }

    if (color.fZ <= 0.0f) {
        color.fZ = 0.0f;
    } else if (color.fZ >= 255.0f) {
        color.fZ = 255.0f;
    }

    return SkPreMultiplyARGB(a, (int) color.fX,  (int) color.fY, (int) color.fZ);
}

// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each one here costs 16 bytes)
#define BUFFER_MAX 16
void SkShadowShaderImpl::ShadowShaderContext::shadeSpan(int x, int y,
                                                        SkPMColor result[], int count) {
    const SkShadowShaderImpl& lightShader = static_cast<const SkShadowShaderImpl&>(fShader);

    SkPMColor diffuse[BUFFER_MAX];
    SkPMColor povDepth[BUFFER_MAX];

    do {
        int n = SkTMin(count, BUFFER_MAX);

        fDiffuseContext->shadeSpan(x, y, diffuse, n);
        fPovDepthContext->shadeSpan(x, y, povDepth, n);

        for (int i = 0; i < n; ++i) {
            SkColor diffColor = SkUnPreMultiply::PMColorToColor(diffuse[i]);
            SkColor povDepthColor = povDepth[i];

            SkColor3f totalLight = lightShader.fLights->ambientLightColor();
            // This is all done in linear unpremul color space (each component 0..255.0f though)

            for (int l = 0; l < lightShader.fLights->numLights(); ++l) {
                const SkLights::Light& light = lightShader.fLights->light(l);

                int pvDepth = SkColorGetB(povDepthColor); // depth stored in blue channel

                if (light.type() == SkLights::Light::kDirectional_LightType) {

                    int xOffset = SkScalarRoundToInt(light.dir().fX * pvDepth);
                    int yOffset = SkScalarRoundToInt(light.dir().fY * pvDepth);

                    int shX = SkClampMax(x + i + xOffset, light.getShadowMap()->width() - 1);
                    int shY = SkClampMax(y + yOffset, light.getShadowMap()->height() - 1);

                    int shDepth = 0;
                    int shDepthsq = 0;

                    // pixmaps that point to things have nonzero heights
                    if (fShadowMapPixels[l].height() > 0) {
                        uint32_t pix = *fShadowMapPixels[l].addr32(shX, shY);
                        SkColor shColor(pix);

                        shDepth = SkColorGetB(shColor);
                        shDepthsq = SkColorGetG(shColor) * 256;
                    } else {
                        // Make lights w/o a shadow map receive the full light contribution
                        shDepth = pvDepth;
                    }

                    SkScalar lightProb = 1.0f;
                    if (pvDepth < shDepth) {
                        if (lightShader.fShadowParams.fType ==
                            SkShadowParams::ShadowType::kVariance_ShadowType) {
                            int variance = SkMaxScalar(shDepthsq - shDepth * shDepth,
                                                       lightShader.fShadowParams.fMinVariance);
                            int d = pvDepth - shDepth;

                            lightProb = (SkScalar) variance / ((SkScalar) (variance + d * d));

                            SkScalar bias = lightShader.fShadowParams.fBiasingConstant;

                            lightProb = SkMaxScalar((lightProb - bias) / (1.0f - bias), 0.0f);
                        } else {
                            lightProb = 0.0f;
                        }
                    }

                    // assume object normals are pointing straight up
                    totalLight.fX += light.dir().fZ * light.color().fX * lightProb;
                    totalLight.fY += light.dir().fZ * light.color().fY * lightProb;
                    totalLight.fZ += light.dir().fZ * light.color().fZ * lightProb;

                } else {
                    // right now we only expect directional and point light types.
                    SkASSERT(light.type() == SkLights::Light::kPoint_LightType);

                    int height = lightShader.fDiffuseHeight;

                    SkVector3 fragToLight = SkVector3::Make(light.pos().fX - x - i,
                                                            light.pos().fY - (height - y),
                                                            light.pos().fZ - pvDepth);

                    SkScalar dist = fragToLight.length();
                    SkScalar normalizedZ = fragToLight.fZ / dist;

                    SkScalar distAttenuation = light.intensity() / (1.0f + dist);

                    // assume object normals are pointing straight up
                    totalLight.fX += normalizedZ * light.color().fX * distAttenuation;
                    totalLight.fY += normalizedZ * light.color().fY * distAttenuation;
                    totalLight.fZ += normalizedZ * light.color().fZ * distAttenuation;
                }
            }

            SkColor3f totalColor = SkColor3f::Make(SkColorGetR(diffColor) * totalLight.fX,
                                                   SkColorGetG(diffColor) * totalLight.fY,
                                                   SkColorGetB(diffColor) * totalLight.fZ);

            result[i] = convert(totalColor, SkColorGetA(diffColor));
        }

        result += n;
        x += n;
        count -= n;
    } while (count > 0);
}

////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkShadowShaderImpl::toString(SkString* str) const {
    str->appendf("ShadowShader: ()");
}
#endif

sk_sp<SkFlattenable> SkShadowShaderImpl::CreateProc(SkReadBuffer& buf) {

    // Discarding SkShader flattenable params
    bool hasLocalMatrix = buf.readBool();
    SkAssertResult(!hasLocalMatrix);

    sk_sp<SkLights> lights = SkLights::MakeFromBuffer(buf);

    SkShadowParams params;
    params.fMinVariance = buf.readScalar();
    params.fBiasingConstant = buf.readScalar();
    params.fType = (SkShadowParams::ShadowType) buf.readInt();
    params.fShadowRadius = buf.readScalar();

    int diffuseWidth = buf.readInt();
    int diffuseHeight = buf.readInt();

    sk_sp<SkShader> povDepthShader(buf.readFlattenable<SkShader>());
    sk_sp<SkShader> diffuseShader(buf.readFlattenable<SkShader>());

    return sk_make_sp<SkShadowShaderImpl>(std::move(povDepthShader),
                                          std::move(diffuseShader),
                                          std::move(lights),
                                          diffuseWidth, diffuseHeight,
                                          params);
}

void SkShadowShaderImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    fLights->flatten(buf);

    buf.writeScalar(fShadowParams.fMinVariance);
    buf.writeScalar(fShadowParams.fBiasingConstant);
    buf.writeInt(fShadowParams.fType);
    buf.writeScalar(fShadowParams.fShadowRadius);

    buf.writeInt(fDiffuseWidth);
    buf.writeInt(fDiffuseHeight);

    buf.writeFlattenable(fPovDepthShader.get());
    buf.writeFlattenable(fDiffuseShader.get());
}

size_t SkShadowShaderImpl::onContextSize(const ContextRec& rec) const {
    return sizeof(ShadowShaderContext);
}

SkShader::Context* SkShadowShaderImpl::onCreateContext(const ContextRec& rec,
                                                       void* storage) const {
    size_t heapRequired = fPovDepthShader->contextSize(rec) +
                          fDiffuseShader->contextSize(rec);

    void* heapAllocated = sk_malloc_throw(heapRequired);

    void* povDepthContextStorage = heapAllocated;

    SkShader::Context* povDepthContext =
            fPovDepthShader->createContext(rec, povDepthContextStorage);

    if (!povDepthContext) {
        sk_free(heapAllocated);
        return nullptr;
    }

    void* diffuseContextStorage = (char*)heapAllocated + fPovDepthShader->contextSize(rec);

    SkShader::Context* diffuseContext = fDiffuseShader->createContext(rec, diffuseContextStorage);
    if (!diffuseContext) {
        sk_free(heapAllocated);
        return nullptr;
    }

    return new (storage) ShadowShaderContext(*this, rec, povDepthContext, diffuseContext,
                                             heapAllocated);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkShadowShader::Make(sk_sp<SkShader> povDepthShader,
                                     sk_sp<SkShader> diffuseShader,
                                     sk_sp<SkLights> lights,
                                     int diffuseWidth, int diffuseHeight,
                                     const SkShadowParams& params) {
    if (!povDepthShader || !diffuseShader) {
        // TODO: Use paint's color in absence of a diffuseShader
        // TODO: Use a default implementation of normalSource instead
        return nullptr;
    }

    return sk_make_sp<SkShadowShaderImpl>(std::move(povDepthShader),
                                          std::move(diffuseShader),
                                          std::move(lights),
                                          diffuseWidth, diffuseHeight,
                                          params);
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkShadowShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkShadowShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////

#endif
