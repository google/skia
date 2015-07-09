
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "Resources.h"

#include "SkCanvas.h"
#include "SkErrorInternals.h"
#include "SkGr.h"
#include "SkReadBuffer.h"
#include "SkShader.h"
#include "SkWriteBuffer.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

///////////////////////////////////////////////////////////////////////////////

struct SkVector3 {
    SkScalar fX, fY, fZ;

    bool operator==(const SkVector3& other) const {
        return fX == other.fX && fY == other.fY && fZ == other.fZ;
    }

    bool operator!=(const SkVector3& other) const {
        return !(*this == other);
    }
};

class LightingShader : public SkShader {
public:
    struct Light {
        SkVector3   fDirection;
        SkColor     fColor;           // assumed to be linear color
    };

    LightingShader(const SkBitmap& diffuse, const SkBitmap& normal, const Light& light,
                   const SkColor ambient) 
        : fDiffuseMap(diffuse)
        , fNormalMap(normal)
        , fLight(light)
        , fAmbientColor(ambient) {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(LightingShader);

    void flatten(SkWriteBuffer& buf) const override {
        buf.writeBitmap(fDiffuseMap);
        buf.writeBitmap(fNormalMap);
        buf.writeScalarArray(&fLight.fDirection.fX, 3);
        buf.writeColor(fLight.fColor);
        buf.writeColor(fAmbientColor);
    }

    bool asFragmentProcessor(GrContext*, const SkPaint& paint, const SkMatrix& viewM,
                             const SkMatrix* localMatrix, GrColor* color,
                             GrProcessorDataManager*, GrFragmentProcessor** fp) const override;

    SkShader::BitmapType asABitmap(SkBitmap* bitmap, SkMatrix* matrix, 
                                   SkShader::TileMode* xy) const override {
        if (bitmap) {
            *bitmap = fDiffuseMap;
        }
        if (matrix) {
            matrix->reset();
        }
        if (xy) {
            xy[0] = kClamp_TileMode;
            xy[1] = kClamp_TileMode;
        }
        return kDefault_BitmapType;
    }

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override {
        str->appendf("LightingShader: ()");
    }
#endif

    void setLight(const Light& light) { fLight = light;  }

private:
    SkBitmap fDiffuseMap;
    SkBitmap fNormalMap;
    Light    fLight;
    SkColor  fAmbientColor;
};

SkFlattenable* LightingShader::CreateProc(SkReadBuffer& buf) {
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

    Light light;
    if (!buf.readScalarArray(&light.fDirection.fX, 3)) {
        return NULL;
    }
    light.fColor = buf.readColor();

    SkColor ambient = buf.readColor();

    return SkNEW_ARGS(LightingShader, (diffuse, normal, light, ambient));
}

////////////////////////////////////////////////////////////////////////////

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
        LightingGLFP() : fLightColor(GrColor_ILLEGAL) {
            fLightDir.fX = 10000.0f;
        }

        void emitCode(GrGLFPBuilder* builder,
                      const GrFragmentProcessor& fp,
                      const char* outputColor,
                      const char* inputColor,
                      const TransformedCoordsArray& coords,
                      const TextureSamplerArray& samplers) override {

            GrGLFragmentBuilder* fpb = builder->getFragmentShaderBuilder();

            // add uniforms
            const char* lightDirUniName = NULL;
            fLightDirUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                               kVec3f_GrSLType, kDefault_GrSLPrecision,
                                               "LightDir", &lightDirUniName);

            const char* lightColorUniName = NULL;
            fLightColorUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                 kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                 "LightColor", &lightColorUniName);

            const char* ambientColorUniName = NULL;
            fAmbientColorUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                   kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                   "AmbientColor", &ambientColorUniName);

            fpb->codeAppend("vec4 diffuseColor = ");
            fpb->appendTextureLookupAndModulate(inputColor, samplers[0], 
                                                coords[0].c_str(), coords[0].getType());
            fpb->codeAppend(";");

            fpb->codeAppend("vec4 normalColor = ");
            fpb->appendTextureLookup(samplers[1], coords[0].c_str(), coords[0].getType());
            fpb->codeAppend(";");

            fpb->codeAppend("vec3 normal = normalize(2.0*(normalColor.rgb - vec3(0.5)));");
            fpb->codeAppendf("vec3 lightDir = normalize(%s);", lightDirUniName);
            fpb->codeAppend("float NdotL = dot(normal, lightDir);");
            // diffuse light
            fpb->codeAppendf("vec3 result = %s.rgb*diffuseColor.rgb*NdotL;", lightColorUniName);
            // ambient light
            fpb->codeAppendf("result += %s.rgb;", ambientColorUniName);
            fpb->codeAppendf("%s = vec4(result.rgb, diffuseColor.a);", outputColor);
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

bool LightingShader::asFragmentProcessor(GrContext* context, const SkPaint& paint, 
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

////////////////////////////////////////////////////////////////////////////

class LightingView : public SampleView {
public:
    SkAutoTUnref<LightingShader> fShader;
    SkBitmap                     fDiffuseBitmap;
    SkBitmap                     fNormalBitmap;
    SkScalar                     fLightAngle;
    int                          fColorFactor;

    LightingView() {
        SkString diffusePath = GetResourcePath("brickwork-texture.jpg");
        SkImageDecoder::DecodeFile(diffusePath.c_str(), &fDiffuseBitmap);
        SkString normalPath = GetResourcePath("brickwork_normal-map.jpg");
        SkImageDecoder::DecodeFile(normalPath.c_str(), &fNormalBitmap);

        fLightAngle = 0.0f;
        fColorFactor = 0;

        LightingShader::Light light;
        light.fColor = SkColorSetRGB(0xff, 0xff, 0xff);
        light.fDirection.fX = SkScalarSin(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fY = SkScalarCos(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fZ = SkScalarCos(SK_ScalarPI*0.25f);

        SkColor ambient = SkColorSetRGB(0x1f, 0x1f, 0x1f);

        fShader.reset(SkNEW_ARGS(LightingShader, (fDiffuseBitmap, fNormalBitmap, light, ambient)));
    }

    virtual ~LightingView() {}

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Lighting");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        fLightAngle += 0.015f;
        fColorFactor++;

        LightingShader::Light light;
        light.fColor = SkColorSetRGB(0xff, 0xff, (fColorFactor >> 1) & 0xff);
        light.fDirection.fX = SkScalarSin(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fY = SkScalarCos(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fZ = SkScalarCos(SK_ScalarPI*0.25f);

        fShader.get()->setLight(light);

        SkPaint paint;
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLACK);

        SkRect r = SkRect::MakeWH((SkScalar)fDiffuseBitmap.width(), 
                                  (SkScalar)fDiffuseBitmap.height());
        canvas->drawRect(r, paint);

        // so we're constantly updating
        this->inval(NULL);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LightingView; }
static SkViewRegister reg(MyFactory);
