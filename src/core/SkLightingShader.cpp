
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkColor.h"
#include "SkEmptyShader.h"
#include "SkErrorInternals.h"
#include "SkLightingShader.h"
#include "SkMathPriv.h"
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
class SK_API SkLightingShaderImpl : public SkShader {
public:

    /** Create a new lighting shader that uses the provided normal map and
        lights to light the diffuse bitmap.
        @param diffuse    the diffuse bitmap
        @param normal     the normal map
        @param lights     the lights applied to the normal map
        @param invNormRotation rotation applied to the normal map's normals
        @param diffLocalM the local matrix for the diffuse coordinates
        @param normLocalM the local matrix for the normal coordinates
    */
    SkLightingShaderImpl(const SkBitmap& diffuse, const SkBitmap& normal,
                         const SkLightingShader::Lights* lights,
                         const SkVector& invNormRotation,
                         const SkMatrix* diffLocalM, const SkMatrix* normLocalM) 
        : INHERITED(diffLocalM)
        , fDiffuseMap(diffuse)
        , fNormalMap(normal)
        , fLights(SkRef(lights))
        , fInvNormRotation(invNormRotation) {

        if (normLocalM) {
            fNormLocalMatrix = *normLocalM;
        } else {
            fNormLocalMatrix.reset();
        }
        // Pre-cache so future calls to fNormLocalMatrix.getType() are threadsafe.
        (void)fNormLocalMatrix.getType();

    }

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix* localMatrix,
                                                   SkFilterQuality) const override;
#endif

    size_t contextSize() const override;

    class LightingShaderContext : public SkShader::Context {
    public:
        // The context takes ownership of the states. It will call their destructors
        // but will NOT free the memory.
        LightingShaderContext(const SkLightingShaderImpl&, const ContextRec&,
                              SkBitmapProcState* diffuseState, SkBitmapProcState* normalState);
        ~LightingShaderContext() override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        uint32_t getFlags() const override { return fFlags; }

    private:
        SkBitmapProcState* fDiffuseState;
        SkBitmapProcState* fNormalState;
        uint32_t           fFlags;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onCreateContext(const ContextRec&, void*) const override;
    bool computeNormTotalInverse(const ContextRec& rec, SkMatrix* normTotalInverse) const;

private:
    SkBitmap  fDiffuseMap;
    SkBitmap  fNormalMap;

    SkAutoTUnref<const SkLightingShader::Lights>   fLights;

    SkMatrix  fNormLocalMatrix;
    SkVector  fInvNormRotation;

    friend class SkLightingShader;

    typedef SkShader INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTextureAccess.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "SkGr.h"
#include "SkGrPriv.h"

class LightingFP : public GrFragmentProcessor {
public:
    LightingFP(GrTexture* diffuse, GrTexture* normal, const SkMatrix& diffMatrix,
               const SkMatrix& normMatrix, const GrTextureParams& diffParams,
               const GrTextureParams& normParams, const SkLightingShader::Lights* lights,
               const SkVector& invNormRotation)
        : fDiffDeviceTransform(kLocal_GrCoordSet, diffMatrix, diffuse, diffParams.filterMode())
        , fNormDeviceTransform(kLocal_GrCoordSet, normMatrix, normal, normParams.filterMode())
        , fDiffuseTextureAccess(diffuse, diffParams)
        , fNormalTextureAccess(normal, normParams)
        , fInvNormRotation(invNormRotation) {
        this->addCoordTransform(&fDiffDeviceTransform);
        this->addCoordTransform(&fNormDeviceTransform);
        this->addTextureAccess(&fDiffuseTextureAccess);
        this->addTextureAccess(&fNormalTextureAccess);

        // fuse all ambient lights into a single one
        fAmbientColor.set(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < lights->numLights(); ++i) {
            if (SkLight::kAmbient_LightType == lights->light(i).type()) {
                fAmbientColor += lights->light(i).color();
            } else {
                // TODO: handle more than one of these
                fLightColor = lights->light(i).color();
                fLightDir = lights->light(i).dir();
            }
        }

        this->initClassID<LightingFP>();
    }

    class LightingGLFP : public GrGLSLFragmentProcessor {
    public:
        LightingGLFP() {
            fLightDir.fX = 10000.0f;
            fLightColor.fX = 0.0f;
            fAmbientColor.fX = 0.0f;
            fInvNormRotation.set(0.0f, 0.0f);
        }

        void emitCode(EmitArgs& args) override {

            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;

            // add uniforms
            const char* lightDirUniName = nullptr;
            fLightDirUni = args.fBuilder->addUniform(GrGLSLProgramBuilder::kFragment_Visibility,
                                                     kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                     "LightDir", &lightDirUniName);

            const char* lightColorUniName = nullptr;
            fLightColorUni = args.fBuilder->addUniform(GrGLSLProgramBuilder::kFragment_Visibility,
                                                       kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                       "LightColor", &lightColorUniName);

            const char* ambientColorUniName = nullptr;
            fAmbientColorUni = args.fBuilder->addUniform(GrGLSLProgramBuilder::kFragment_Visibility,
                                                         kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                         "AmbientColor", &ambientColorUniName);

            const char* xformUniName = nullptr;
            fXformUni = args.fBuilder->addUniform(GrGLSLProgramBuilder::kFragment_Visibility,
                                                  kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                  "Xform", &xformUniName);

            fragBuilder->codeAppend("vec4 diffuseColor = ");
            fragBuilder->appendTextureLookupAndModulate(args.fInputColor, args.fSamplers[0], 
                                                args.fCoords[0].c_str(), 
                                                args.fCoords[0].getType());
            fragBuilder->codeAppend(";");

            fragBuilder->codeAppend("vec4 normalColor = ");
            fragBuilder->appendTextureLookup(args.fSamplers[1],
                                     args.fCoords[1].c_str(), 
                                     args.fCoords[1].getType());
            fragBuilder->codeAppend(";");

            fragBuilder->codeAppend("vec3 normal = normalColor.rgb - vec3(0.5);");

            fragBuilder->codeAppendf(
                                 "mat3 m = mat3(%s.x, -%s.y, 0.0, %s.y, %s.x, 0.0, 0.0, 0.0, 1.0);",
                                 xformUniName, xformUniName, xformUniName, xformUniName);
            
            // TODO: inverse map the light direction vectors in the vertex shader rather than
            // transforming all the normals here!
            fragBuilder->codeAppend("normal = normalize(m*normal);");

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

            const SkVector& invNormRotation = lightingFP.invNormRotation();
            if (invNormRotation != fInvNormRotation) {
                pdman.set2fv(fXformUni, 1, &invNormRotation.fX);
                fInvNormRotation = invNormRotation;
            }
        }

    private:
        SkVector3 fLightDir;
        GrGLSLProgramDataManager::UniformHandle fLightDirUni;

        SkColor3f fLightColor;
        GrGLSLProgramDataManager::UniformHandle fLightColorUni;

        SkColor3f fAmbientColor;
        GrGLSLProgramDataManager::UniformHandle fAmbientColorUni;

        SkVector fInvNormRotation;
        GrGLSLProgramDataManager::UniformHandle fXformUni;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        LightingGLFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "LightingFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->mulByUnknownFourComponents();
    }

    const SkVector3& lightDir() const { return fLightDir; }
    const SkColor3f& lightColor() const { return fLightColor; }
    const SkColor3f& ambientColor() const { return fAmbientColor; }
    const SkVector& invNormRotation() const { return fInvNormRotation; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new LightingGLFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override { 
        const LightingFP& lightingFP = proc.cast<LightingFP>();
        return fDiffDeviceTransform == lightingFP.fDiffDeviceTransform &&
               fNormDeviceTransform == lightingFP.fNormDeviceTransform &&
               fDiffuseTextureAccess == lightingFP.fDiffuseTextureAccess &&
               fNormalTextureAccess == lightingFP.fNormalTextureAccess &&
               fLightDir == lightingFP.fLightDir &&
               fLightColor == lightingFP.fLightColor &&
               fAmbientColor == lightingFP.fAmbientColor &&
               fInvNormRotation == lightingFP.fInvNormRotation;
    }

    GrCoordTransform fDiffDeviceTransform;
    GrCoordTransform fNormDeviceTransform;
    GrTextureAccess  fDiffuseTextureAccess;
    GrTextureAccess  fNormalTextureAccess;
    SkVector3        fLightDir;
    SkColor3f        fLightColor;
    SkColor3f        fAmbientColor;

    SkVector         fInvNormRotation;
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

const GrFragmentProcessor* SkLightingShaderImpl::asFragmentProcessor(
                                                             GrContext* context,
                                                             const SkMatrix& viewM,
                                                             const SkMatrix* localMatrix,
                                                             SkFilterQuality filterQuality) const {
    // we assume diffuse and normal maps have same width and height
    // TODO: support different sizes
    SkASSERT(fDiffuseMap.width() == fNormalMap.width() &&
             fDiffuseMap.height() == fNormalMap.height());
    SkMatrix diffM, normM;
    
    if (!make_mat(fDiffuseMap, this->getLocalMatrix(), localMatrix, &diffM)) {
        return nullptr;
    }

    if (!make_mat(fNormalMap, fNormLocalMatrix, localMatrix, &normM)) {
        return nullptr;
    }

    bool doBicubic;
    GrTextureParams::FilterMode diffFilterMode = GrSkFilterQualityToGrFilterMode(
                                        SkTMin(filterQuality, kMedium_SkFilterQuality), 
                                        viewM,
                                        this->getLocalMatrix(),
                                        &doBicubic); 
    SkASSERT(!doBicubic);

    GrTextureParams::FilterMode normFilterMode = GrSkFilterQualityToGrFilterMode(
                                        SkTMin(filterQuality, kMedium_SkFilterQuality), 
                                        viewM,
                                        fNormLocalMatrix,
                                        &doBicubic); 
    SkASSERT(!doBicubic);

    // TODO: support other tile modes
    GrTextureParams diffParams(kClamp_TileMode, diffFilterMode);
    SkAutoTUnref<GrTexture> diffuseTexture(GrRefCachedBitmapTexture(context,
                                                                    fDiffuseMap, diffParams));
    if (!diffuseTexture) {
        SkErrorInternals::SetError(kInternalError_SkError, "Couldn't convert bitmap to texture.");
        return nullptr;
    }

    GrTextureParams normParams(kClamp_TileMode, normFilterMode);
    SkAutoTUnref<GrTexture> normalTexture(GrRefCachedBitmapTexture(context,
                                                                   fNormalMap, normParams));
    if (!normalTexture) {
        SkErrorInternals::SetError(kInternalError_SkError, "Couldn't convert bitmap to texture.");
        return nullptr;
    }

    SkAutoTUnref<const GrFragmentProcessor> inner (
        new LightingFP(diffuseTexture, normalTexture, diffM, normM, diffParams, normParams, fLights,
                       fInvNormRotation));
    return GrFragmentProcessor::MulOutputByInputAlpha(inner);
}

#endif

////////////////////////////////////////////////////////////////////////////

bool SkLightingShaderImpl::isOpaque() const {
    return fDiffuseMap.isOpaque();
}

size_t SkLightingShaderImpl::contextSize() const {
    return 2 * sizeof(SkBitmapProcState) + sizeof(LightingShaderContext);
}

SkLightingShaderImpl::LightingShaderContext::LightingShaderContext(const SkLightingShaderImpl& shader,
                                                                   const ContextRec& rec,
                                                                   SkBitmapProcState* diffuseState,
                                                                   SkBitmapProcState* normalState)
    : INHERITED(shader, rec)
    , fDiffuseState(diffuseState)
    , fNormalState(normalState)
{
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
    fNormalState->~SkBitmapProcState();
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
#define TMP_COUNT     16

void SkLightingShaderImpl::LightingShaderContext::shadeSpan(int x, int y,
                                                            SkPMColor result[], int count) {
    const SkLightingShaderImpl& lightShader = static_cast<const SkLightingShaderImpl&>(fShader);

    uint32_t  tmpColor[TMP_COUNT], tmpNormal[TMP_COUNT];
    SkPMColor tmpColor2[2*TMP_COUNT], tmpNormal2[2*TMP_COUNT];

    SkBitmapProcState::MatrixProc   diffMProc = fDiffuseState->getMatrixProc();
    SkBitmapProcState::SampleProc32 diffSProc = fDiffuseState->getSampleProc32();

    SkBitmapProcState::MatrixProc   normalMProc = fNormalState->getMatrixProc();
    SkBitmapProcState::SampleProc32 normalSProc = fNormalState->getSampleProc32();

    int diffMax = fDiffuseState->maxCountForBufferSize(sizeof(tmpColor[0]) * TMP_COUNT);
    int normMax = fNormalState->maxCountForBufferSize(sizeof(tmpNormal[0]) * TMP_COUNT);
    int max = SkTMin(diffMax, normMax);

    SkASSERT(fDiffuseState->fPixmap.addr());
    SkASSERT(fNormalState->fPixmap.addr());

    SkPoint3 norm, xformedNorm;

    do {
        int n = count;
        if (n > max) {
            n = max;
        }

        diffMProc(*fDiffuseState, tmpColor, n, x, y);
        diffSProc(*fDiffuseState, tmpColor, n, tmpColor2);

        normalMProc(*fNormalState, tmpNormal, n, x, y);
        normalSProc(*fNormalState, tmpNormal, n, tmpNormal2);

        for (int i = 0; i < n; ++i) {
            SkASSERT(0xFF == SkColorGetA(tmpNormal2[i]));  // opaque -> unpremul
            norm.set(SkIntToScalar(SkGetPackedR32(tmpNormal2[i]))-127.0f,
                     SkIntToScalar(SkGetPackedG32(tmpNormal2[i]))-127.0f,
                     SkIntToScalar(SkGetPackedB32(tmpNormal2[i]))-127.0f);
            norm.normalize();

            xformedNorm.fX = lightShader.fInvNormRotation.fX * norm.fX +
                             lightShader.fInvNormRotation.fY * norm.fY;
            xformedNorm.fY = lightShader.fInvNormRotation.fX * norm.fX - 
                             lightShader.fInvNormRotation.fY * norm.fY;
            xformedNorm.fZ = norm.fZ;

            SkColor diffColor = SkUnPreMultiply::PMColorToColor(tmpColor2[i]);

            SkColor3f accum = SkColor3f::Make(0.0f, 0.0f, 0.0f);
            // This is all done in linear unpremul color space (each component 0..255.0f though)
            for (int l = 0; l < lightShader.fLights->numLights(); ++l) {
                const SkLight& light = lightShader.fLights->light(l);

                if (SkLight::kAmbient_LightType == light.type()) {
                    accum += light.color().makeScale(255.0f);
                } else {
                    SkScalar NdotL = xformedNorm.dot(light.dir());
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

SkFlattenable* SkLightingShaderImpl::CreateProc(SkReadBuffer& buf) {
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

    SkLightingShader::Lights::Builder builder;

    for (int l = 0; l < numLights; ++l) {
        bool isAmbient = buf.readBool();

        SkColor3f color;
        if (!buf.readScalarArray(&color.fX, 3)) {
            return nullptr;
        }

        if (isAmbient) {
            builder.add(SkLight(color));
        } else {
            SkVector3 dir;
            if (!buf.readScalarArray(&dir.fX, 3)) {
                return nullptr;
            }
            builder.add(SkLight(color, dir));        
        }
    }

    SkAutoTUnref<const SkLightingShader::Lights> lights(builder.finish());

    return new SkLightingShaderImpl(diffuse, normal, lights, SkVector::Make(1.0f, 0.0f),
                                    &diffLocalM, &normLocalM);
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
        const SkLight& light = fLights->light(l);

        bool isAmbient = SkLight::kAmbient_LightType == light.type();

        buf.writeBool(isAmbient);
        buf.writeScalarArray(&light.color().fX, 3);
        if (!isAmbient) {
            buf.writeScalarArray(&light.dir().fX, 3);
        }
    }
}

bool SkLightingShaderImpl::computeNormTotalInverse(const ContextRec& rec,
                                                   SkMatrix* normTotalInverse) const {
    SkMatrix total;
    total.setConcat(*rec.fMatrix, fNormLocalMatrix);

    const SkMatrix* m = &total;
    if (rec.fLocalMatrix) {
        total.setConcat(*m, *rec.fLocalMatrix);
        m = &total;
    }
    return m->invert(normTotalInverse);
}

SkShader::Context* SkLightingShaderImpl::onCreateContext(const ContextRec& rec,
                                                         void* storage) const {

    SkMatrix diffTotalInv;
    // computeTotalInverse was called in SkShader::createContext so we know it will succeed
    SkAssertResult(this->computeTotalInverse(rec, &diffTotalInv));

    SkMatrix normTotalInv;
    if (!this->computeNormTotalInverse(rec, &normTotalInv)) {
        return nullptr;
    }

    void* diffuseStateStorage = (char*)storage + sizeof(LightingShaderContext);
    SkBitmapProcState* diffuseState = new (diffuseStateStorage) SkBitmapProcState(fDiffuseMap,
                                              SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    SkASSERT(diffuseState);
    if (!diffuseState->chooseProcs(diffTotalInv, *rec.fPaint)) {
        diffuseState->~SkBitmapProcState();
        return nullptr;
    }

    void* normalStateStorage = (char*)storage + sizeof(LightingShaderContext) + sizeof(SkBitmapProcState);
    SkBitmapProcState* normalState = new (normalStateStorage) SkBitmapProcState(fNormalMap,
                                            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    SkASSERT(normalState);
    if (!normalState->chooseProcs(normTotalInv, *rec.fPaint)) {
        diffuseState->~SkBitmapProcState();
        normalState->~SkBitmapProcState();
        return nullptr;
    }

    return new (storage) LightingShaderContext(*this, rec, diffuseState, normalState);
}

///////////////////////////////////////////////////////////////////////////////

static bool bitmap_is_too_big(const SkBitmap& bm) {
    // SkBitmapProcShader stores bitmap coordinates in a 16bit buffer, as it
    // communicates between its matrix-proc and its sampler-proc. Until we can
    // widen that, we have to reject bitmaps that are larger.
    //
    static const int kMaxSize = 65535;

    return bm.width() > kMaxSize || bm.height() > kMaxSize;
}

SkShader* SkLightingShader::Create(const SkBitmap& diffuse, const SkBitmap& normal,
                                   const Lights* lights,
                                   const SkVector& invNormRotation,
                                   const SkMatrix* diffLocalM, const SkMatrix* normLocalM) {
    if (diffuse.isNull() || bitmap_is_too_big(diffuse) ||
        normal.isNull() || bitmap_is_too_big(normal) ||
        diffuse.width() != normal.width() ||
        diffuse.height() != normal.height()) {
        return nullptr;
    }

    SkASSERT(SkScalarNearlyEqual(invNormRotation.lengthSqd(), SK_Scalar1));

    return new SkLightingShaderImpl(diffuse, normal, lights, invNormRotation, diffLocalM,
                                    normLocalM);
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
