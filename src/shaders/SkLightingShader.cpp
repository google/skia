/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBitmapProcShader.h"
#include "SkBitmapProcState.h"
#include "SkColor.h"
#include "SkColorSpaceXformer.h"
#include "SkEmptyShader.h"
#include "SkLightingShader.h"
#include "SkMathPriv.h"
#include "SkNormalSource.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkShaderBase.h"
#include "SkWriteBuffer.h"

////////////////////////////////////////////////////////////////////////////

/*
   SkLightingShader TODOs:
        support different light types
        support multiple lights
        fix non-opaque diffuse textures

    To Test:
        A8 diffuse textures
        down & upsampled draws
*/



/** \class SkLightingShaderImpl
    This subclass of shader applies lighting.
*/
class SkLightingShaderImpl : public SkShaderBase {
public:
    /** Create a new lighting shader that uses the provided normal map and
        lights to light the diffuse bitmap.
        @param diffuseShader     the shader that provides the diffuse colors
        @param normalSource      the source of normals for lighting computation
        @param lights            the lights applied to the geometry
    */
    SkLightingShaderImpl(sk_sp<SkShader> diffuseShader,
                         sk_sp<SkNormalSource> normalSource,
                         sk_sp<SkLights> lights)
        : fDiffuseShader(std::move(diffuseShader))
        , fNormalSource(std::move(normalSource))
        , fLights(std::move(lights)) {}

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    class LightingShaderContext : public Context {
    public:
        // The context takes ownership of the context and provider. It will call their destructors
        // and then indirectly free their memory by calling free() on heapAllocated
        LightingShaderContext(const SkLightingShaderImpl&, const ContextRec&,
                              SkShaderBase::Context* diffuseContext, SkNormalSource::Provider*,
                              void* heapAllocated);

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        uint32_t getFlags() const override { return fFlags; }

    private:
        SkShaderBase::Context*    fDiffuseContext;
        SkNormalSource::Provider* fNormalProvider;
        SkColor                   fPaintColor;
        uint32_t                  fFlags;

        typedef Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

private:
    sk_sp<SkShader> fDiffuseShader;
    sk_sp<SkNormalSource> fNormalSource;
    sk_sp<SkLights> fLights;

    friend class SkLightingShader;

    typedef SkShaderBase INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkGr.h"

// This FP expects a premul'd color input for its diffuse color. Premul'ing of the paint's color is
// handled by the asFragmentProcessor() factory, but shaders providing diffuse color must output it
// premul'd.
class LightingFP : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor> normalFP,
                                           sk_sp<SkLights> lights) {
        return sk_sp<GrFragmentProcessor>(new LightingFP(std::move(normalFP), std::move(lights)));
    }

    class GLSLLightingFP : public GrGLSLFragmentProcessor {
    public:
        GLSLLightingFP() {
            fAmbientColor.fX = 0.0f;
        }

        void emitCode(EmitArgs& args) override {

            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
            const LightingFP& lightingFP = args.fFp.cast<LightingFP>();

            const char *lightDirsUniName = nullptr;
            const char *lightColorsUniName = nullptr;
            if (lightingFP.fDirectionalLights.count() != 0) {
                fLightDirsUni = uniformHandler->addUniformArray(
                        kFragment_GrShaderFlag,
                        kVec3f_GrSLType,
                        kDefault_GrSLPrecision,
                        "LightDir",
                        lightingFP.fDirectionalLights.count(),
                        &lightDirsUniName);
                fLightColorsUni = uniformHandler->addUniformArray(
                        kFragment_GrShaderFlag,
                        kVec3f_GrSLType,
                        kDefault_GrSLPrecision,
                        "LightColor",
                        lightingFP.fDirectionalLights.count(),
                        &lightColorsUniName);
            }

            const char* ambientColorUniName = nullptr;
            fAmbientColorUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                          kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                          "AmbientColor", &ambientColorUniName);

            fragBuilder->codeAppendf("vec4 diffuseColor = %s;", args.fInputColor);

            SkString dstNormalName("dstNormal");
            this->emitChild(0, &dstNormalName, args);

            fragBuilder->codeAppendf("vec3 normal = %s.xyz;", dstNormalName.c_str());

            fragBuilder->codeAppend( "vec3 result = vec3(0.0);");

            // diffuse light
            if (lightingFP.fDirectionalLights.count() != 0) {
                fragBuilder->codeAppendf("for (int i = 0; i < %d; i++) {",
                                         lightingFP.fDirectionalLights.count());
                // TODO: modulate the contribution from each light based on the shadow map
                fragBuilder->codeAppendf("    float NdotL = clamp(dot(normal, %s[i]), 0.0, 1.0);",
                                         lightDirsUniName);
                fragBuilder->codeAppendf("    result += %s[i]*diffuseColor.rgb*NdotL;",
                                         lightColorsUniName);
                fragBuilder->codeAppend("}");
            }

            // ambient light
            fragBuilder->codeAppendf("result += %s * diffuseColor.rgb;", ambientColorUniName);

            // Clamping to alpha (equivalent to an unpremul'd clamp to 1.0)
            fragBuilder->codeAppendf("%s = vec4(clamp(result.rgb, 0.0, diffuseColor.a), "
                                               "diffuseColor.a);", args.fOutputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
            const LightingFP& lightingFP = proc.cast<LightingFP>();
            b->add32(lightingFP.fDirectionalLights.count());
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const LightingFP& lightingFP = proc.cast<LightingFP>();

            const SkTArray<SkLights::Light>& directionalLights = lightingFP.directionalLights();
            if (directionalLights != fDirectionalLights) {
                SkTArray<SkColor3f> lightDirs(directionalLights.count());
                SkTArray<SkVector3> lightColors(directionalLights.count());
                for (const SkLights::Light& light : directionalLights) {
                    lightDirs.push_back(light.dir());
                    lightColors.push_back(light.color());
                }

                pdman.set3fv(fLightDirsUni, directionalLights.count(), &(lightDirs[0].fX));
                pdman.set3fv(fLightColorsUni, directionalLights.count(), &(lightColors[0].fX));

                fDirectionalLights = directionalLights;
            }

            const SkColor3f& ambientColor = lightingFP.ambientColor();
            if (ambientColor != fAmbientColor) {
                pdman.set3fv(fAmbientColorUni, 1, &ambientColor.fX);
                fAmbientColor = ambientColor;
            }
        }

    private:
        SkTArray<SkLights::Light> fDirectionalLights;
        GrGLSLProgramDataManager::UniformHandle fLightDirsUni;
        GrGLSLProgramDataManager::UniformHandle fLightColorsUni;

        SkColor3f fAmbientColor;
        GrGLSLProgramDataManager::UniformHandle fAmbientColorUni;
    };

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLLightingFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "LightingFP"; }

    const SkTArray<SkLights::Light>& directionalLights() const { return fDirectionalLights; }
    const SkColor3f& ambientColor() const { return fAmbientColor; }

private:
    LightingFP(sk_sp<GrFragmentProcessor> normalFP, sk_sp<SkLights> lights)
            : INHERITED(kPreservesOpaqueInput_OptimizationFlag) {
        // fuse all ambient lights into a single one
        fAmbientColor = lights->ambientLightColor();
        for (int i = 0; i < lights->numLights(); ++i) {
            if (SkLights::Light::kDirectional_LightType == lights->light(i).type()) {
                fDirectionalLights.push_back(lights->light(i));
                // TODO get the handle to the shadow map if there is one
            } else {
                SkDEBUGFAIL("Unimplemented Light Type passed to LightingFP");
            }
        }

        this->registerChildProcessor(std::move(normalFP));
        this->initClassID<LightingFP>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLLightingFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const LightingFP& lightingFP = proc.cast<LightingFP>();
        return fDirectionalLights == lightingFP.fDirectionalLights &&
               fAmbientColor == lightingFP.fAmbientColor;
    }

    SkTArray<SkLights::Light> fDirectionalLights;
    SkColor3f                 fAmbientColor;

    typedef GrFragmentProcessor INHERITED;
};

////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkLightingShaderImpl::asFragmentProcessor(const AsFPArgs& args) const {
    sk_sp<GrFragmentProcessor> normalFP(fNormalSource->asFragmentProcessor(args));
    if (!normalFP) {
        return nullptr;
    }

    if (fDiffuseShader) {
        sk_sp<GrFragmentProcessor> fpPipeline[] = {
            as_SB(fDiffuseShader)->asFragmentProcessor(args),
            LightingFP::Make(std::move(normalFP), fLights)
        };
        if (!fpPipeline[0] || !fpPipeline[1]) {
            return nullptr;
        }

        sk_sp<GrFragmentProcessor> innerLightFP = GrFragmentProcessor::RunInSeries(fpPipeline, 2);
        // FP is wrapped because paint's alpha needs to be applied to output
        return GrFragmentProcessor::MulOutputByInputAlpha(std::move(innerLightFP));
    } else {
        // FP is wrapped because paint comes in unpremul'd to fragment shader, but LightingFP
        // expects premul'd color.
        return GrFragmentProcessor::PremulInput(LightingFP::Make(std::move(normalFP), fLights));
    }
}

#endif

////////////////////////////////////////////////////////////////////////////

bool SkLightingShaderImpl::isOpaque() const {
    return (fDiffuseShader ? fDiffuseShader->isOpaque() : false);
}

SkLightingShaderImpl::LightingShaderContext::LightingShaderContext(
        const SkLightingShaderImpl& shader, const ContextRec& rec,
        SkShaderBase::Context* diffuseContext, SkNormalSource::Provider* normalProvider,
        void* heapAllocated)
    : INHERITED(shader, rec)
    , fDiffuseContext(diffuseContext)
    , fNormalProvider(normalProvider) {
    bool isOpaque = shader.isOpaque();

    // update fFlags
    uint32_t flags = 0;
    if (isOpaque && (255 == this->getPaintAlpha())) {
        flags |= kOpaqueAlpha_Flag;
    }

    fPaintColor = rec.fPaint->getColor();
    fFlags = flags;
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
void SkLightingShaderImpl::LightingShaderContext::shadeSpan(int x, int y,
                                                            SkPMColor result[], int count) {
    const SkLightingShaderImpl& lightShader = static_cast<const SkLightingShaderImpl&>(fShader);

    SkPMColor diffuse[BUFFER_MAX];
    SkPoint3 normals[BUFFER_MAX];

    SkColor diffColor = fPaintColor;

    do {
        int n = SkTMin(count, BUFFER_MAX);

        fNormalProvider->fillScanLine(x, y, normals, n);

        if (fDiffuseContext) {
            fDiffuseContext->shadeSpan(x, y, diffuse, n);
        }

        for (int i = 0; i < n; ++i) {
            if (fDiffuseContext) {
                diffColor = SkUnPreMultiply::PMColorToColor(diffuse[i]);
            }

            SkColor3f accum = SkColor3f::Make(0.0f, 0.0f, 0.0f);

            // Adding ambient light
            accum.fX += lightShader.fLights->ambientLightColor().fX * SkColorGetR(diffColor);
            accum.fY += lightShader.fLights->ambientLightColor().fY * SkColorGetG(diffColor);
            accum.fZ += lightShader.fLights->ambientLightColor().fZ * SkColorGetB(diffColor);

            // This is all done in linear unpremul color space (each component 0..255.0f though)
            for (int l = 0; l < lightShader.fLights->numLights(); ++l) {
                const SkLights::Light& light = lightShader.fLights->light(l);

                SkScalar illuminanceScalingFactor = 1.0f;

                if (SkLights::Light::kDirectional_LightType == light.type()) {
                    illuminanceScalingFactor = normals[i].dot(light.dir());
                    if (illuminanceScalingFactor < 0.0f) {
                        illuminanceScalingFactor = 0.0f;
                    }
                }

                accum.fX += light.color().fX * SkColorGetR(diffColor) * illuminanceScalingFactor;
                accum.fY += light.color().fY * SkColorGetG(diffColor) * illuminanceScalingFactor;
                accum.fZ += light.color().fZ * SkColorGetB(diffColor) * illuminanceScalingFactor;
            }

            // convert() premultiplies the accumulate color with alpha
            result[i] = convert(accum, SkColorGetA(diffColor));
        }

        result += n;
        x += n;
        count -= n;
    } while (count > 0);
}

////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkLightingShaderImpl::toString(SkString* str) const {
    str->appendf("LightingShader: ()");
}
#endif

sk_sp<SkFlattenable> SkLightingShaderImpl::CreateProc(SkReadBuffer& buf) {

    // Discarding SkShader flattenable params
    bool hasLocalMatrix = buf.readBool();
    SkAssertResult(!hasLocalMatrix);

    sk_sp<SkLights> lights = SkLights::MakeFromBuffer(buf);

    sk_sp<SkNormalSource> normalSource(buf.readFlattenable<SkNormalSource>());

    bool hasDiffuse = buf.readBool();
    sk_sp<SkShader> diffuseShader = nullptr;
    if (hasDiffuse) {
        diffuseShader = buf.readFlattenable<SkShaderBase>();
    }

    return sk_make_sp<SkLightingShaderImpl>(std::move(diffuseShader), std::move(normalSource),
                                            std::move(lights));
}

void SkLightingShaderImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    fLights->flatten(buf);

    buf.writeFlattenable(fNormalSource.get());
    buf.writeBool(fDiffuseShader);
    if (fDiffuseShader) {
        buf.writeFlattenable(fDiffuseShader.get());
    }
}

SkShaderBase::Context* SkLightingShaderImpl::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    SkShaderBase::Context *diffuseContext = nullptr;
    if (fDiffuseShader) {
        diffuseContext = as_SB(fDiffuseShader)->makeContext(rec, alloc);
        if (!diffuseContext) {
            return nullptr;
        }
    }

    SkNormalSource::Provider* normalProvider = fNormalSource->asProvider(rec, alloc);
    if (!normalProvider) {
        return nullptr;
    }

    return alloc->make<LightingShaderContext>(*this, rec, diffuseContext, normalProvider, nullptr);
}

sk_sp<SkShader> SkLightingShaderImpl::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    sk_sp<SkShader> xformedDiffuseShader =
            fDiffuseShader ? xformer->apply(fDiffuseShader.get()) : nullptr;
    return SkLightingShader::Make(std::move(xformedDiffuseShader), fNormalSource,
                                  fLights->makeColorSpace(xformer));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkLightingShader::Make(sk_sp<SkShader> diffuseShader,
                                       sk_sp<SkNormalSource> normalSource,
                                       sk_sp<SkLights> lights) {
    SkASSERT(lights);
    if (!normalSource) {
        normalSource = SkNormalSource::MakeFlat();
    }

    return sk_make_sp<SkLightingShaderImpl>(std::move(diffuseShader), std::move(normalSource),
                                            std::move(lights));
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
