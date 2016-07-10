/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkBitmapProcState.h"
#include "SkColor.h"
#include "SkEmptyShader.h"
#include "SkErrorInternals.h"
#include "SkLightingShader.h"
#include "SkMathPriv.h"
#include "SkNormalSource.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

////////////////////////////////////////////////////////////////////////////

/*
   SkLightingShader TODOs:
        support other than clamp mode
        allow 'diffuse' & 'normal' to be of different dimensions?
        support different light types
        support multiple lights
        enforce normal map is 4 channel
        use SkImages instead if SkBitmaps

    To Test:
        non-opaque diffuse textures
        A8 diffuse textures
        down & upsampled draws
*/



/** \class SkLightingShaderImpl
    This subclass of shader applies lighting.
*/
class SkLightingShaderImpl : public SkShader {
public:
    /** Create a new lighting shader that uses the provided normal map and
        lights to light the diffuse bitmap.
        @param diffuseShader     the shader that provides the diffuse colors
        @param normalSource      the source of normals for lighting computation
        @param lights            the lights applied to the geometry
    */
    SkLightingShaderImpl(sk_sp<SkShader> diffuseShader,
                         sk_sp<SkNormalSource> normalSource,
                         const sk_sp<SkLights> lights)
        : fDiffuseShader(std::move(diffuseShader))
        , fNormalSource(std::move(normalSource))
        , fLights(std::move(lights)) {}

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix* localMatrix,
                                                   SkFilterQuality,
                                                   SkSourceGammaTreatment) const override;
#endif

    class LightingShaderContext : public SkShader::Context {
    public:
        // The context takes ownership of the states. It will call their destructors
        // but will NOT free the memory.
        LightingShaderContext(const SkLightingShaderImpl&, const ContextRec&,
                              SkShader::Context* diffuseContext, SkNormalSource::Provider*,
                              void* heapAllocated);

        ~LightingShaderContext() override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        uint32_t getFlags() const override { return fFlags; }

    private:
        SkShader::Context*        fDiffuseContext;
        SkNormalSource::Provider* fNormalProvider;
        uint32_t                  fFlags;

        void* fHeapAllocated;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void*) const override;

private:
    sk_sp<SkShader> fDiffuseShader;
    sk_sp<SkNormalSource> fNormalSource;
    sk_sp<SkLights> fLights;

    friend class SkLightingShader;

    typedef SkShader INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrTextureAccess.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkGr.h"
#include "SkGrPriv.h"

class LightingFP : public GrFragmentProcessor {
public:
    LightingFP(sk_sp<GrFragmentProcessor> normalFP, sk_sp<SkLights> lights) {

        // fuse all ambient lights into a single one
        fAmbientColor.set(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < lights->numLights(); ++i) {
            if (SkLights::Light::kAmbient_LightType == lights->light(i).type()) {
                fAmbientColor += lights->light(i).color();
            } else {
                // TODO: handle more than one of these
                fLightColor = lights->light(i).color();
                fLightDir = lights->light(i).dir();
            }
        }

        this->registerChildProcessor(std::move(normalFP));
        this->initClassID<LightingFP>();
    }

    class GLSLLightingFP : public GrGLSLFragmentProcessor {
    public:
        GLSLLightingFP() {
            fLightDir.fX = 10000.0f;
            fLightColor.fX = 0.0f;
            fAmbientColor.fX = 0.0f;
        }

        void emitCode(EmitArgs& args) override {

            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // add uniforms
            const char* lightDirUniName = nullptr;
            fLightDirUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                      kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                      "LightDir", &lightDirUniName);

            const char* lightColorUniName = nullptr;
            fLightColorUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                        kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                        "LightColor", &lightColorUniName);

            const char* ambientColorUniName = nullptr;
            fAmbientColorUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                          kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                          "AmbientColor", &ambientColorUniName);

            fragBuilder->codeAppendf("vec4 diffuseColor = %s;", args.fInputColor);

            SkString dstNormalName("dstNormal");
            this->emitChild(0, nullptr, &dstNormalName, args);

            fragBuilder->codeAppendf("vec3 normal = %s.xyz;", dstNormalName.c_str());
            fragBuilder->codeAppendf("float NdotL = clamp(dot(normal, %s), 0.0, 1.0);",
                                     lightDirUniName);
            // diffuse light
            fragBuilder->codeAppendf("vec3 result = %s*diffuseColor.rgb*NdotL;", lightColorUniName);
            // ambient light
            fragBuilder->codeAppendf("result += %s;", ambientColorUniName);
            fragBuilder->codeAppendf("%s = vec4(result.rgb, diffuseColor.a);", args.fOutputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
//            const LightingFP& lightingFP = proc.cast<LightingFP>();
            // only one shader generated currently
            b->add32(0x0);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
            const LightingFP& lightingFP = proc.cast<LightingFP>();

            const SkVector3& lightDir = lightingFP.lightDir();
            if (lightDir != fLightDir) {
                pdman.set3fv(fLightDirUni, 1, &lightDir.fX);
                fLightDir = lightDir;
            }

            const SkColor3f& lightColor = lightingFP.lightColor();
            if (lightColor != fLightColor) {
                pdman.set3fv(fLightColorUni, 1, &lightColor.fX);
                fLightColor = lightColor;
            }

            const SkColor3f& ambientColor = lightingFP.ambientColor();
            if (ambientColor != fAmbientColor) {
                pdman.set3fv(fAmbientColorUni, 1, &ambientColor.fX);
                fAmbientColor = ambientColor;
            }
        }

    private:
        SkVector3 fLightDir;
        GrGLSLProgramDataManager::UniformHandle fLightDirUni;

        SkColor3f fLightColor;
        GrGLSLProgramDataManager::UniformHandle fLightColorUni;

        SkColor3f fAmbientColor;
        GrGLSLProgramDataManager::UniformHandle fAmbientColorUni;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLLightingFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "LightingFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->mulByUnknownFourComponents();
    }

    const SkVector3& lightDir() const { return fLightDir; }
    const SkColor3f& lightColor() const { return fLightColor; }
    const SkColor3f& ambientColor() const { return fAmbientColor; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLLightingFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const LightingFP& lightingFP = proc.cast<LightingFP>();
        return fLightDir == lightingFP.fLightDir &&
               fLightColor == lightingFP.fLightColor &&
               fAmbientColor == lightingFP.fAmbientColor;
    }

    SkVector3        fLightDir;
    SkColor3f        fLightColor;
    SkColor3f        fAmbientColor;
};

////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkLightingShaderImpl::asFragmentProcessor(
                                                     GrContext* context,
                                                     const SkMatrix& viewM,
                                                     const SkMatrix* localMatrix,
                                                     SkFilterQuality filterQuality,
                                                     SkSourceGammaTreatment gammaTreatment) const {
    sk_sp<GrFragmentProcessor> normalFP(
            fNormalSource->asFragmentProcessor(context, viewM, localMatrix, filterQuality,
                                               gammaTreatment));
    if (!normalFP) {
        return nullptr;
    }

    sk_sp<GrFragmentProcessor> fpPipeline[] = {
            fDiffuseShader->asFragmentProcessor(context, viewM, localMatrix, filterQuality,
                                                gammaTreatment),
            sk_make_sp<LightingFP>(std::move(normalFP), fLights)
    };
    if(!fpPipeline[0]) {
        return nullptr;
    }

    sk_sp<GrFragmentProcessor> inner(GrFragmentProcessor::RunInSeries(fpPipeline, 2));

    return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
}

#endif

////////////////////////////////////////////////////////////////////////////

bool SkLightingShaderImpl::isOpaque() const {
    return fDiffuseShader->isOpaque();
}

SkLightingShaderImpl::LightingShaderContext::LightingShaderContext(
        const SkLightingShaderImpl& shader, const ContextRec& rec,
        SkShader::Context* diffuseContext, SkNormalSource::Provider* normalProvider,
        void* heapAllocated)
    : INHERITED(shader, rec)
    , fDiffuseContext(diffuseContext)
    , fNormalProvider(normalProvider)
    , fHeapAllocated(heapAllocated) {
    bool isOpaque = shader.isOpaque();

    // update fFlags
    uint32_t flags = 0;
    if (isOpaque && (255 == this->getPaintAlpha())) {
        flags |= kOpaqueAlpha_Flag;
    }

    fFlags = flags;
}

SkLightingShaderImpl::LightingShaderContext::~LightingShaderContext() {
    // The dependencies have been created outside of the context on memory that was allocated by
    // the onCreateContext() method. Call the destructors and free the memory.
    fDiffuseContext->~Context();
    fNormalProvider->~Provider();

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
void SkLightingShaderImpl::LightingShaderContext::shadeSpan(int x, int y,
                                                            SkPMColor result[], int count) {
    const SkLightingShaderImpl& lightShader = static_cast<const SkLightingShaderImpl&>(fShader);

    SkPMColor diffuse[BUFFER_MAX];
    SkPoint3 normals[BUFFER_MAX];

    do {
        int n = SkTMin(count, BUFFER_MAX);

        fDiffuseContext->shadeSpan(x, y, diffuse, n);
        fNormalProvider->fillScanLine(x, y, normals, n);

        for (int i = 0; i < n; ++i) {

            SkColor diffColor = SkUnPreMultiply::PMColorToColor(diffuse[i]);

            SkColor3f accum = SkColor3f::Make(0.0f, 0.0f, 0.0f);
            // This is all done in linear unpremul color space (each component 0..255.0f though)
            for (int l = 0; l < lightShader.fLights->numLights(); ++l) {
                const SkLights::Light& light = lightShader.fLights->light(l);

                if (SkLights::Light::kAmbient_LightType == light.type()) {
                    accum += light.color().makeScale(255.0f);
                } else {
                    SkScalar NdotL = normals[i].dot(light.dir());
                    if (NdotL < 0.0f) {
                        NdotL = 0.0f;
                    }

                    accum.fX += light.color().fX * SkColorGetR(diffColor) * NdotL;
                    accum.fY += light.color().fY * SkColorGetG(diffColor) * NdotL;
                    accum.fZ += light.color().fZ * SkColorGetB(diffColor) * NdotL;
                }
            }

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

    int numLights = buf.readInt();

    SkLights::Builder builder;

    for (int l = 0; l < numLights; ++l) {
        bool isAmbient = buf.readBool();

        SkColor3f color;
        if (!buf.readScalarArray(&color.fX, 3)) {
            return nullptr;
        }

        if (isAmbient) {
            builder.add(SkLights::Light(color));
        } else {
            SkVector3 dir;
            if (!buf.readScalarArray(&dir.fX, 3)) {
                return nullptr;
            }
            builder.add(SkLights::Light(color, dir));
        }
    }

    sk_sp<SkLights> lights(builder.finish());

    sk_sp<SkNormalSource> normalSource(buf.readFlattenable<SkNormalSource>());
    sk_sp<SkShader> diffuseShader(buf.readFlattenable<SkShader>());

    return sk_make_sp<SkLightingShaderImpl>(std::move(diffuseShader), std::move(normalSource),
                                            std::move(lights));
}

void SkLightingShaderImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    buf.writeInt(fLights->numLights());
    for (int l = 0; l < fLights->numLights(); ++l) {
        const SkLights::Light& light = fLights->light(l);

        bool isAmbient = SkLights::Light::kAmbient_LightType == light.type();

        buf.writeBool(isAmbient);
        buf.writeScalarArray(&light.color().fX, 3);
        if (!isAmbient) {
            buf.writeScalarArray(&light.dir().fX, 3);
        }
    }

    buf.writeFlattenable(fNormalSource.get());
    buf.writeFlattenable(fDiffuseShader.get());
}

size_t SkLightingShaderImpl::onContextSize(const ContextRec& rec) const {
    return sizeof(LightingShaderContext);
}

SkShader::Context* SkLightingShaderImpl::onCreateContext(const ContextRec& rec,
                                                         void* storage) const {
    size_t heapRequired = fDiffuseShader->contextSize(rec) +
                          fNormalSource->providerSize(rec);
    void* heapAllocated = sk_malloc_throw(heapRequired);

    void* diffuseContextStorage = heapAllocated;
    SkShader::Context* diffuseContext = fDiffuseShader->createContext(rec, diffuseContextStorage);
    if (!diffuseContext) {
        sk_free(heapAllocated);
        return nullptr;
    }

    void* normalProviderStorage = (char*)heapAllocated + fDiffuseShader->contextSize(rec);
    SkNormalSource::Provider* normalProvider = fNormalSource->asProvider(rec,
                                                                         normalProviderStorage);
    if (!normalProvider) {
        diffuseContext->~Context();
        sk_free(heapAllocated);
        return nullptr;
    }

    return new (storage) LightingShaderContext(*this, rec, diffuseContext, normalProvider,
                                               heapAllocated);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkLightingShader::Make(const SkBitmap& diffuse,
                                       sk_sp<SkLights> lights,
                                       const SkMatrix* diffLocalM,
                                       sk_sp<SkNormalSource> normalSource) {
    if (diffuse.isNull() || SkBitmapProcShader::BitmapIsTooBig(diffuse)) {
        return nullptr;
    }

    if (!normalSource) {
        // TODO: Use a default implementation of normalSource instead
        return nullptr;
    }

    // TODO: support other tile modes
    sk_sp<SkShader> diffuseShader = SkBitmapProcShader::MakeBitmapShader(diffuse,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, diffLocalM);

    return sk_make_sp<SkLightingShaderImpl>(std::move(diffuseShader), std::move(normalSource),
                                            std::move(lights));
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
