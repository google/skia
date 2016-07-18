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
        @param diffuse           the diffuse bitmap
        @param normal            the normal map
        @param lights            the lights applied to the normal map
        @param invNormRotation   rotation applied to the normal map's normals
        @param diffLocalM        the local matrix for the diffuse coordinates
        @param normLocalM        the local matrix for the normal coordinates
        @param normalSource      the normal source for GPU computations
    */
    SkLightingShaderImpl(const SkBitmap& diffuse, const SkBitmap& normal,
                         const sk_sp<SkLights> lights,
                         const SkVector& invNormRotation,
                         const SkMatrix* diffLocalM, const SkMatrix* normLocalM,
                         sk_sp<SkNormalSource> normalSource)
        : INHERITED(diffLocalM)
        , fDiffuseMap(diffuse)
        , fNormalMap(normal)
        , fLights(std::move(lights))
        , fInvNormRotation(invNormRotation) {

        if (normLocalM) {
            fNormLocalMatrix = *normLocalM;
        } else {
            fNormLocalMatrix.reset();
        }
        // Pre-cache so future calls to fNormLocalMatrix.getType() are threadsafe.
        (void)fNormLocalMatrix.getType();

        fNormalSource = std::move(normalSource);
    }

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
                              SkBitmapProcState* diffuseState, SkNormalSource::Provider*,
                              void* heapAllocated);
        ~LightingShaderContext() override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        uint32_t getFlags() const override { return fFlags; }

    private:
        SkBitmapProcState*        fDiffuseState;
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
    SkBitmap        fDiffuseMap;
    SkBitmap        fNormalMap;

    sk_sp<SkLights> fLights;

    SkMatrix        fNormLocalMatrix;
    SkVector        fInvNormRotation;

    sk_sp<SkNormalSource> fNormalSource;

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
    LightingFP(GrTexture* diffuse, const SkMatrix& diffMatrix, const GrTextureParams& diffParams,
               sk_sp<SkLights> lights, sk_sp<GrFragmentProcessor> normalFP)
        : fDiffDeviceTransform(kLocal_GrCoordSet, diffMatrix, diffuse, diffParams.filterMode())
        , fDiffuseTextureAccess(diffuse, diffParams) {
        this->addCoordTransform(&fDiffDeviceTransform);
        this->addTextureAccess(&fDiffuseTextureAccess);

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

            fragBuilder->codeAppend("vec4 diffuseColor = ");
            fragBuilder->appendTextureLookupAndModulate(args.fInputColor, args.fTexSamplers[0],
                                                args.fCoords[0].c_str(),
                                                args.fCoords[0].getType());
            fragBuilder->codeAppend(";");

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
        return fDiffDeviceTransform == lightingFP.fDiffDeviceTransform &&
               fDiffuseTextureAccess == lightingFP.fDiffuseTextureAccess &&
               fLightDir == lightingFP.fLightDir &&
               fLightColor == lightingFP.fLightColor &&
               fAmbientColor == lightingFP.fAmbientColor;
    }

    GrCoordTransform fDiffDeviceTransform;
    GrTextureAccess  fDiffuseTextureAccess;
    SkVector3        fLightDir;
    SkColor3f        fLightColor;
    SkColor3f        fAmbientColor;
};

////////////////////////////////////////////////////////////////////////////

static bool make_mat(const SkBitmap& bm,
                     const SkMatrix& localMatrix1,
                     const SkMatrix* localMatrix2,
                     SkMatrix* result) {

    result->setIDiv(bm.width(), bm.height());

    SkMatrix lmInverse;
    if (!localMatrix1.invert(&lmInverse)) {
        return false;
    }
    if (localMatrix2) {
        SkMatrix inv;
        if (!localMatrix2->invert(&inv)) {
            return false;
        }
        lmInverse.postConcat(inv);
    }
    result->preConcat(lmInverse);

    return true;
}

sk_sp<GrFragmentProcessor> SkLightingShaderImpl::asFragmentProcessor(
                                                     GrContext* context,
                                                     const SkMatrix& viewM,
                                                     const SkMatrix* localMatrix,
                                                     SkFilterQuality filterQuality,
                                                     SkSourceGammaTreatment gammaTreatment) const {
    // we assume diffuse and normal maps have same width and height
    // TODO: support different sizes, will be addressed when diffuse maps are factored out of
    //       SkLightingShader in a future CL
    SkASSERT(fDiffuseMap.width() == fNormalMap.width() &&
             fDiffuseMap.height() == fNormalMap.height());
    SkMatrix diffM;

    if (!make_mat(fDiffuseMap, this->getLocalMatrix(), localMatrix, &diffM)) {
        return nullptr;
    }

    bool doBicubic;
    GrTextureParams::FilterMode diffFilterMode = GrSkFilterQualityToGrFilterMode(
                                                    SkTMin(filterQuality, kMedium_SkFilterQuality),
                                                    viewM,
                                                    this->getLocalMatrix(),
                                                    &doBicubic);
    SkASSERT(!doBicubic);

    // TODO: support other tile modes
    GrTextureParams diffParams(kClamp_TileMode, diffFilterMode);
    SkAutoTUnref<GrTexture> diffuseTexture(GrRefCachedBitmapTexture(context, fDiffuseMap,
                                                                    diffParams, gammaTreatment));
    if (!diffuseTexture) {
        SkErrorInternals::SetError(kInternalError_SkError, "Couldn't convert bitmap to texture.");
        return nullptr;
    }

    sk_sp<GrFragmentProcessor> normalFP(
            fNormalSource->asFragmentProcessor(context, viewM, localMatrix, filterQuality,
                                               gammaTreatment));
    sk_sp<GrFragmentProcessor> inner (
            new LightingFP(diffuseTexture, diffM, diffParams, fLights, std::move(normalFP)));

    return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
}

#endif

////////////////////////////////////////////////////////////////////////////

bool SkLightingShaderImpl::isOpaque() const {
    return fDiffuseMap.isOpaque();
}

SkLightingShaderImpl::LightingShaderContext::LightingShaderContext(
        const SkLightingShaderImpl& shader, const ContextRec& rec, SkBitmapProcState* diffuseState,
        SkNormalSource::Provider* normalProvider, void* heapAllocated)
    : INHERITED(shader, rec)
    , fDiffuseState(diffuseState)
    , fNormalProvider(normalProvider)
    , fHeapAllocated(heapAllocated) {
    const SkPixmap& pixmap = fDiffuseState->fPixmap;
    bool isOpaque = pixmap.isOpaque();

    // update fFlags
    uint32_t flags = 0;
    if (isOpaque && (255 == this->getPaintAlpha())) {
        flags |= kOpaqueAlpha_Flag;
    }

    fFlags = flags;
}

SkLightingShaderImpl::LightingShaderContext::~LightingShaderContext() {
    // The bitmap proc states have been created outside of the context on memory that will be freed
    // elsewhere. Call the destructors but leave the freeing of the memory to the caller.
    fDiffuseState->~SkBitmapProcState();
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
#define TMP_COUNT 16
#define BUFFER_MAX ((int)(TMP_COUNT * sizeof(uint32_t)))
void SkLightingShaderImpl::LightingShaderContext::shadeSpan(int x, int y,
                                                            SkPMColor result[], int count) {
    const SkLightingShaderImpl& lightShader = static_cast<const SkLightingShaderImpl&>(fShader);

    uint32_t  tmpColor[TMP_COUNT];
    SkPMColor tmpColor2[2*TMP_COUNT];

    SkBitmapProcState::MatrixProc   diffMProc = fDiffuseState->getMatrixProc();
    SkBitmapProcState::SampleProc32 diffSProc = fDiffuseState->getSampleProc32();

    int max = fDiffuseState->maxCountForBufferSize(BUFFER_MAX);

    SkASSERT(fDiffuseState->fPixmap.addr());

    SkASSERT(max <= BUFFER_MAX);
    SkPoint3 normals[BUFFER_MAX];

    do {
        int n = count;
        if (n > max) {
            n = max;
        }

        diffMProc(*fDiffuseState, tmpColor, n, x, y);
        diffSProc(*fDiffuseState, tmpColor, n, tmpColor2);

        fNormalProvider->fillScanLine(x, y, normals, n);

        for (int i = 0; i < n; ++i) {

            SkColor diffColor = SkUnPreMultiply::PMColorToColor(tmpColor2[i]);

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
    SkMatrix diffLocalM;
    bool hasDiffLocalM = buf.readBool();
    if (hasDiffLocalM) {
        buf.readMatrix(&diffLocalM);
    } else {
        diffLocalM.reset();
    }

    SkMatrix normLocalM;
    bool hasNormLocalM = buf.readBool();
    if (hasNormLocalM) {
        buf.readMatrix(&normLocalM);
    } else {
        normLocalM.reset();
    }

    SkBitmap diffuse;
    if (!buf.readBitmap(&diffuse)) {
        return nullptr;
    }
    diffuse.setImmutable();

    SkBitmap normal;
    if (!buf.readBitmap(&normal)) {
        return nullptr;
    }
    normal.setImmutable();

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

    SkVector invNormRotation = {1,0};
    if (!buf.isVersionLT(SkReadBuffer::kLightingShaderWritesInvNormRotation)) {
        invNormRotation = buf.readPoint();
    }

    sk_sp<SkNormalSource> normalSource(buf.readFlattenable<SkNormalSource>());

    return sk_make_sp<SkLightingShaderImpl>(diffuse, normal, std::move(lights), invNormRotation,
                                            &diffLocalM, &normLocalM, std::move(normalSource));
}

void SkLightingShaderImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    bool hasNormLocalM = !fNormLocalMatrix.isIdentity();
    buf.writeBool(hasNormLocalM);
    if (hasNormLocalM) {
        buf.writeMatrix(fNormLocalMatrix);
    }

    buf.writeBitmap(fDiffuseMap);
    buf.writeBitmap(fNormalMap);

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
    buf.writePoint(fInvNormRotation);

    buf.writeFlattenable(fNormalSource.get());
}

size_t SkLightingShaderImpl::onContextSize(const ContextRec& rec) const {
    return sizeof(LightingShaderContext);
}

SkShader::Context* SkLightingShaderImpl::onCreateContext(const ContextRec& rec,
                                                         void* storage) const {

    SkMatrix diffTotalInv;
    // computeTotalInverse was called in SkShader::createContext so we know it will succeed
    SkAssertResult(this->computeTotalInverse(rec, &diffTotalInv));

    size_t heapRequired = sizeof(SkBitmapProcState) + fNormalSource->providerSize(rec);
    void* heapAllocated = sk_malloc_throw(heapRequired);

    void* diffuseStateStorage = heapAllocated;
    SkBitmapProcState* diffuseState = new (diffuseStateStorage) SkBitmapProcState(fDiffuseMap,
                                              SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                                                                    SkMipMap::DeduceTreatment(rec));
    SkASSERT(diffuseState);
    if (!diffuseState->setup(diffTotalInv, *rec.fPaint)) {
        diffuseState->~SkBitmapProcState();
        sk_free(heapAllocated);
        return nullptr;
    }
    void* normalProviderStorage = (char*)heapAllocated + sizeof(SkBitmapProcState);

    SkNormalSource::Provider* normalProvider = fNormalSource->asProvider(rec,
                                                                         normalProviderStorage);
    if (!normalProvider) {
        diffuseState->~SkBitmapProcState();
        sk_free(heapAllocated);
        return nullptr;
    }

    return new (storage) LightingShaderContext(*this, rec, diffuseState, normalProvider,
                                               heapAllocated);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkLightingShader::Make(const SkBitmap& diffuse, const SkBitmap& normal,
                                       sk_sp<SkLights> lights,
                                       const SkVector& invNormRotation,
                                       const SkMatrix* diffLocalM, const SkMatrix* normLocalM) {
    if (diffuse.isNull() || SkBitmapProcShader::BitmapIsTooBig(diffuse) ||
        normal.isNull()  || SkBitmapProcShader::BitmapIsTooBig(normal) ||
        diffuse.width()  != normal.width() ||
        diffuse.height() != normal.height()) {
        return nullptr;
    }
    SkASSERT(SkScalarNearlyEqual(invNormRotation.lengthSqd(), SK_Scalar1));

    // TODO: support other tile modes
    sk_sp<SkShader> mapShader = SkMakeBitmapShader(normal, SkShader::kClamp_TileMode,
                                                   SkShader::kClamp_TileMode, normLocalM, nullptr);

    sk_sp<SkNormalSource> normalSource = SkNormalSource::MakeFromNormalMap(mapShader,
                                                                           invNormRotation);

    return sk_make_sp<SkLightingShaderImpl>(diffuse, normal, std::move(lights),
            invNormRotation, diffLocalM, normLocalM, std::move(normalSource));
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
