
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
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

///////////////////////////////////////////////////////////////////////////////////////////

/*
   SkLightingShader TODOs:
        support other than clamp mode
        allow 'diffuse' & 'normal' to be of different dimensions?
        support different light types
        support multiple lights
        enforce normal map is 4 channel
        use SkImages instead if SkBitmaps
        vec3 for ambient and light-color
        add dox for both lighting equation, and how we compute normal from bitmap

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

    /** Create a new lighting shader that use the provided normal map, light
        and ambient color to light the diffuse bitmap.
        @param diffuse the diffuse bitmap
        @param normal  the normal map
        @param light   the light applied to the normal map
        @param ambient the linear (unpremul) ambient light color
    */
    SkLightingShaderImpl(const SkBitmap& diffuse, const SkBitmap& normal,
                         const SkLightingShader::Light& light,
                         const SkColor ambient) 
        : fDiffuseMap(diffuse)
        , fNormalMap(normal)
        , fLight(light)
        , fAmbientColor(ambient) {
        if (!fLight.fDirection.normalize()) {
            fLight.fDirection = SkPoint3::Make(0.0f, 0.0f, 1.0f);
        }
        SkColorSetA(fLight.fColor, 0xFF);
        SkColorSetA(fAmbientColor, 0xFF);
    }

    bool isOpaque() const override;

    bool asFragmentProcessor(GrContext*, const SkPaint& paint, const SkMatrix& viewM,
                             const SkMatrix* localMatrix, GrColor* color,
                             GrProcessorDataManager*, GrFragmentProcessor** fp) const override;

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
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(LightingShader)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onCreateContext(const ContextRec&, void*) const override;

private:
    SkBitmap                fDiffuseMap;
    SkBitmap                fNormalMap;
    SkLightingShader::Light fLight;
    SkColor                 fAmbientColor;  // linear (unpremul) color

    typedef SkShader INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTextureAccess.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "SkGr.h"

class LightingFP : public GrFragmentProcessor {
public:
    LightingFP(GrTexture* diffuse, GrTexture* normal, const SkMatrix& matrix,
               SkVector3 lightDir, GrColor lightColor, GrColor ambientColor)
        : fDeviceTransform(kDevice_GrCoordSet, matrix)
        , fDiffuseTextureAccess(diffuse)
        , fNormalTextureAccess(normal)
        , fLightDir(lightDir)
        , fLightColor(lightColor)
        , fAmbientColor(ambientColor) {
        this->addCoordTransform(&fDeviceTransform);
        this->addTextureAccess(&fDiffuseTextureAccess);
        this->addTextureAccess(&fNormalTextureAccess);

        this->initClassID<LightingFP>();
    }

    class LightingGLFP : public GrGLFragmentProcessor {
    public:
        LightingGLFP() : fLightColor(GrColor_ILLEGAL), fAmbientColor(GrColor_ILLEGAL) {
            fLightDir.fX = 10000.0f;
        }

        void emitCode(EmitArgs& args) override {

            GrGLFragmentBuilder* fpb = args.fBuilder->getFragmentShaderBuilder();

            // add uniforms
            const char* lightDirUniName = NULL;
            fLightDirUni = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                     kVec3f_GrSLType, kDefault_GrSLPrecision,
                                                     "LightDir", &lightDirUniName);

            const char* lightColorUniName = NULL;
            fLightColorUni = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                       kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                       "LightColor", &lightColorUniName);

            const char* ambientColorUniName = NULL;
            fAmbientColorUni = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                         kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                         "AmbientColor", &ambientColorUniName);

            fpb->codeAppend("vec4 diffuseColor = ");
            fpb->appendTextureLookupAndModulate(args.fInputColor, args.fSamplers[0], 
                                                args.fCoords[0].c_str(), 
                                                args.fCoords[0].getType());
            fpb->codeAppend(";");

            fpb->codeAppend("vec4 normalColor = ");
            fpb->appendTextureLookup(args.fSamplers[1],
                                     args.fCoords[0].c_str(), 
                                     args.fCoords[0].getType());
            fpb->codeAppend(";");

            fpb->codeAppend("vec3 normal = normalize(normalColor.rgb - vec3(0.5));");
            fpb->codeAppendf("vec3 lightDir = normalize(%s);", lightDirUniName);
            fpb->codeAppend("float NdotL = dot(normal, lightDir);");
            // diffuse light
            fpb->codeAppendf("vec3 result = %s.rgb*diffuseColor.rgb*NdotL;", lightColorUniName);
            // ambient light
            fpb->codeAppendf("result += %s.rgb;", ambientColorUniName);
            fpb->codeAppendf("%s = vec4(result.rgb, diffuseColor.a);", args.fOutputColor);
        }

        void setData(const GrGLProgramDataManager& pdman, const GrProcessor& proc) override {
            const LightingFP& lightingFP = proc.cast<LightingFP>();

            SkVector3 lightDir = lightingFP.lightDir();
            if (lightDir != fLightDir) {
                pdman.set3fv(fLightDirUni, 1, &lightDir.fX);
                fLightDir = lightDir;
            }

            GrColor lightColor = lightingFP.lightColor();
            if (lightColor != fLightColor) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(lightColor, c);
                pdman.set4fv(fLightColorUni, 1, c);
                fLightColor = lightColor;
            }

            GrColor ambientColor = lightingFP.ambientColor();
            if (ambientColor != fAmbientColor) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(ambientColor, c);
                pdman.set4fv(fAmbientColorUni, 1, c);
                fAmbientColor = ambientColor;
            }
        }

        static void GenKey(const GrProcessor& proc, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
//            const LightingFP& lightingFP = proc.cast<LightingFP>();
            // only one shader generated currently
            b->add32(0x0);
        }

    private:
        SkVector3 fLightDir;
        GrGLProgramDataManager::UniformHandle fLightDirUni;

        GrColor fLightColor;
        GrGLProgramDataManager::UniformHandle fLightColorUni;

        GrColor fAmbientColor;
        GrGLProgramDataManager::UniformHandle fAmbientColorUni;
    };

    GrGLFragmentProcessor* createGLInstance() const override { return SkNEW(LightingGLFP); }

    void getGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        LightingGLFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "LightingFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->mulByUnknownFourComponents();
    }

    SkVector3 lightDir() const { return fLightDir; }
    GrColor lightColor() const { return fLightColor; }
    GrColor ambientColor() const { return fAmbientColor; }

private:
    bool onIsEqual(const GrFragmentProcessor& proc) const override { 
        const LightingFP& lightingFP = proc.cast<LightingFP>();
        return fDeviceTransform == lightingFP.fDeviceTransform &&
               fDiffuseTextureAccess == lightingFP.fDiffuseTextureAccess &&
               fNormalTextureAccess == lightingFP.fNormalTextureAccess &&
               fLightDir == lightingFP.fLightDir &&
               fLightColor == lightingFP.fLightColor &&
               fAmbientColor == lightingFP.fAmbientColor;
    }

    GrCoordTransform fDeviceTransform;
    GrTextureAccess  fDiffuseTextureAccess;
    GrTextureAccess  fNormalTextureAccess;
    SkVector3        fLightDir;
    GrColor          fLightColor;
    GrColor          fAmbientColor;
};

////////////////////////////////////////////////////////////////////////////

bool SkLightingShaderImpl::asFragmentProcessor(GrContext* context, const SkPaint& paint, 
                                               const SkMatrix& viewM, const SkMatrix* localMatrix, 
                                               GrColor* color, GrProcessorDataManager*,
                                               GrFragmentProcessor** fp) const {
    // we assume diffuse and normal maps have same width and height
    // TODO: support different sizes
    SkASSERT(fDiffuseMap.width() == fNormalMap.width() &&
             fDiffuseMap.height() == fNormalMap.height());
    SkMatrix matrix;
    matrix.setIDiv(fDiffuseMap.width(), fDiffuseMap.height());

    SkMatrix lmInverse;
    if (!this->getLocalMatrix().invert(&lmInverse)) {
        return false;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return false;
        }
        lmInverse.postConcat(inv);
    }
    matrix.preConcat(lmInverse);

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    GrTextureParams::FilterMode textureFilterMode = GrTextureParams::kBilerp_FilterMode;
    switch (paint.getFilterQuality()) {
    case kNone_SkFilterQuality:
        textureFilterMode = GrTextureParams::kNone_FilterMode;
        break;
    case kLow_SkFilterQuality:
        textureFilterMode = GrTextureParams::kBilerp_FilterMode;
        break;
    case kMedium_SkFilterQuality:{                          
        SkMatrix matrix;
        matrix.setConcat(viewM, this->getLocalMatrix());
        if (matrix.getMinScale() < SK_Scalar1) {
            textureFilterMode = GrTextureParams::kMipMap_FilterMode;
        } else {
            // Don't trigger MIP level generation unnecessarily.
            textureFilterMode = GrTextureParams::kBilerp_FilterMode;
        }
        break;
    }
    case kHigh_SkFilterQuality:
    default:
        SkErrorInternals::SetError(kInvalidPaint_SkError,
            "Sorry, I don't understand the filtering "
            "mode you asked for.  Falling back to "
            "MIPMaps.");
        textureFilterMode = GrTextureParams::kMipMap_FilterMode;
        break;

    }

    // TODO: support other tile modes
    GrTextureParams params(kClamp_TileMode, textureFilterMode);
    SkAutoTUnref<GrTexture> diffuseTexture(GrRefCachedBitmapTexture(context, fDiffuseMap, &params));
    if (!diffuseTexture) {
        SkErrorInternals::SetError(kInternalError_SkError,
            "Couldn't convert bitmap to texture.");
        return false;
    }

    SkAutoTUnref<GrTexture> normalTexture(GrRefCachedBitmapTexture(context, fNormalMap, &params));
    if (!normalTexture) {
        SkErrorInternals::SetError(kInternalError_SkError,
            "Couldn't convert bitmap to texture.");
        return false;
    }

    GrColor lightColor = GrColorPackRGBA(SkColorGetR(fLight.fColor), SkColorGetG(fLight.fColor),
                                         SkColorGetB(fLight.fColor), SkColorGetA(fLight.fColor));
    GrColor ambientColor = GrColorPackRGBA(SkColorGetR(fAmbientColor), SkColorGetG(fAmbientColor),
                                           SkColorGetB(fAmbientColor), SkColorGetA(fAmbientColor));

    *fp = SkNEW_ARGS(LightingFP, (diffuseTexture, normalTexture, matrix,
                                  fLight.fDirection, lightColor, ambientColor));
    *color = GrColorPackA4(paint.getAlpha());
    return true;
}
#else

bool SkLightingShaderImpl::asFragmentProcessor(GrContext* context, const SkPaint& paint, 
                                               const SkMatrix& viewM, const SkMatrix* localMatrix, 
                                               GrColor* color, GrProcessorDataManager*,
                                               GrFragmentProcessor** fp) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
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

static inline int light(int light, int diff, SkScalar NdotL, int ambient) {
    int color = int(light * diff * NdotL + 255 * ambient);
    if (color <= 0) {
        return 0;
    } else if (color >= 255*255) {
        return 255;
    } else {
        return SkDiv255Round(color);
    }
}

// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each could here costs 16 bytes)
#define TMP_COUNT     16

void SkLightingShaderImpl::LightingShaderContext::shadeSpan(int x, int y,
                                                            SkPMColor result[], int count) {
    const SkLightingShaderImpl& lightShader = static_cast<const SkLightingShaderImpl&>(fShader);

    SkPMColor   tmpColor[TMP_COUNT], tmpColor2[TMP_COUNT];
    SkPMColor   tmpNormal[TMP_COUNT], tmpNormal2[TMP_COUNT];

    SkBitmapProcState::MatrixProc   diffMProc = fDiffuseState->getMatrixProc();
    SkBitmapProcState::SampleProc32 diffSProc = fDiffuseState->getSampleProc32();

    SkBitmapProcState::MatrixProc   normalMProc = fNormalState->getMatrixProc();
    SkBitmapProcState::SampleProc32 normalSProc = fNormalState->getSampleProc32();

    SkASSERT(fDiffuseState->fPixmap.addr());
    SkASSERT(fNormalState->fPixmap.addr());

    SkPoint3 norm;
    SkScalar NdotL;
    int r, g, b;

    do {
        int n = count;
        if (n > TMP_COUNT) {
            n = TMP_COUNT;
        }

        diffMProc(*fDiffuseState, tmpColor, n, x, y);
        diffSProc(*fDiffuseState, tmpColor, n, tmpColor2);

        normalMProc(*fNormalState, tmpNormal, n, x, y);
        normalSProc(*fNormalState, tmpNormal, n, tmpNormal2);

        for (int i = 0; i < n; ++i) {
            SkASSERT(0xFF == SkColorGetA(tmpNormal2[i]));  // opaque -> unpremul
            norm.set(SkIntToScalar(SkColorGetR(tmpNormal2[i]))-127.0f,
                     SkIntToScalar(SkColorGetG(tmpNormal2[i]))-127.0f,
                     SkIntToScalar(SkColorGetB(tmpNormal2[i]))-127.0f);
            norm.normalize();

            SkColor diffColor = SkUnPreMultiply::PMColorToColor(tmpColor2[i]);
            NdotL = norm.dot(lightShader.fLight.fDirection);

            // This is all done in linear unpremul color space
            r = light(SkColorGetR(lightShader.fLight.fColor), SkColorGetR(diffColor), NdotL, 
                      SkColorGetR(lightShader.fAmbientColor));
            g = light(SkColorGetG(lightShader.fLight.fColor), SkColorGetG(diffColor), NdotL, 
                      SkColorGetG(lightShader.fAmbientColor));
            b = light(SkColorGetB(lightShader.fLight.fColor), SkColorGetB(diffColor), NdotL, 
                      SkColorGetB(lightShader.fAmbientColor));

            result[i] = SkPreMultiplyARGB(SkColorGetA(diffColor), r, g, b);
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
    SkBitmap diffuse;
    if (!buf.readBitmap(&diffuse)) {
        return NULL;
    }
    diffuse.setImmutable();

    SkBitmap normal;
    if (!buf.readBitmap(&normal)) {
        return NULL;
    }
    normal.setImmutable();

    SkLightingShader::Light light;
    if (!buf.readScalarArray(&light.fDirection.fX, 3)) {
        return NULL;
    }
    light.fColor = buf.readColor();

    SkColor ambient = buf.readColor();

    // TODO: this would be nice to enable
    //    return SkCreateLightingShader(diffuse, normal, light, ambient, NULL);
    return SkNEW_ARGS(SkLightingShaderImpl, (diffuse, normal, light, ambient));
}

void SkLightingShaderImpl::flatten(SkWriteBuffer& buf) const {
    buf.writeBitmap(fDiffuseMap);
    buf.writeBitmap(fNormalMap);
    buf.writeScalarArray(&fLight.fDirection.fX, 3);
    buf.writeColor(fLight.fColor);
    buf.writeColor(fAmbientColor);
}

SkShader::Context* SkLightingShaderImpl::onCreateContext(const ContextRec& rec,
                                                         void* storage) const {

    SkMatrix totalInverse;
    // Do this first, so we know the matrix can be inverted.
    if (!this->computeTotalInverse(rec, &totalInverse)) {
        return NULL;
    }

    void* diffuseStateStorage = (char*)storage + sizeof(LightingShaderContext);
    SkBitmapProcState* diffuseState = SkNEW_PLACEMENT(diffuseStateStorage, SkBitmapProcState);
    SkASSERT(diffuseState);

    diffuseState->fTileModeX = SkShader::kClamp_TileMode;
    diffuseState->fTileModeY = SkShader::kClamp_TileMode;
    diffuseState->fOrigBitmap = fDiffuseMap;
    if (!diffuseState->chooseProcs(totalInverse, *rec.fPaint)) {
        diffuseState->~SkBitmapProcState();
        return NULL;
    }

    void* normalStateStorage = (char*)storage + sizeof(LightingShaderContext) + sizeof(SkBitmapProcState);
    SkBitmapProcState* normalState = SkNEW_PLACEMENT(normalStateStorage, SkBitmapProcState);
    SkASSERT(normalState);

    normalState->fTileModeX = SkShader::kClamp_TileMode;
    normalState->fTileModeY = SkShader::kClamp_TileMode;
    normalState->fOrigBitmap = fNormalMap;
    if (!normalState->chooseProcs(totalInverse, *rec.fPaint)) {
        diffuseState->~SkBitmapProcState();
        normalState->~SkBitmapProcState();
        return NULL;
    }

    return SkNEW_PLACEMENT_ARGS(storage, LightingShaderContext, (*this, rec,
                                                                 diffuseState, normalState));
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
                                   const SkLightingShader::Light& light,
                                   const SkColor ambient) {
    if (diffuse.isNull() || bitmap_is_too_big(diffuse) ||
        normal.isNull() || bitmap_is_too_big(normal) ||
        diffuse.width() != normal.width() ||
        diffuse.height() != normal.height()) {
        return nullptr;
    }

    return SkNEW_ARGS(SkLightingShaderImpl, (diffuse, normal, light, ambient));
}

///////////////////////////////////////////////////////////////////////////////
