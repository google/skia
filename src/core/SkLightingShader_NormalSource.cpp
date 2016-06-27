/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkError.h"
#include "SkErrorInternals.h"
#include "SkLightingShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

// Genretating vtable
SkLightingShader::NormalSource::~NormalSource() {}

///////////////////////////////////////////////////////////////////////////////

class NormalMapSourceImpl : public SkLightingShader::NormalSource {
public:
    NormalMapSourceImpl(const SkBitmap &normal, const SkVector &invNormRotation,
                        const SkMatrix *normLocalM)
        : fNormalMap(normal)
        , fInvNormRotation(invNormRotation) {

        if (normLocalM) {
            fNormLocalMatrix = *normLocalM;
        } else {
            fNormLocalMatrix.reset();
        }
        // Pre-cache so future calls to fNormLocalMatrix.getType() are threadsafe.
        (void)fNormLocalMatrix.getType();
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix* localMatrix,
                                                   SkFilterQuality,
                                                   SkSourceGammaTreatment) const override;
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(NormalMapSourceImpl)

protected:
    void flatten(SkWriteBuffer& buf) const override;

private:
    SkBitmap        fNormalMap;
    SkMatrix        fNormLocalMatrix;
    SkVector        fInvNormRotation;

    friend class SkLightingShader::NormalSource;

    typedef SkLightingShader::NormalSource INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrTextureParams.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"

class NormalMapFP : public GrFragmentProcessor {
public:
    NormalMapFP(GrTexture* normal, const SkMatrix& normMatrix, const GrTextureParams& normParams,
                const SkVector& invNormRotation)
        : fNormDeviceTransform(kLocal_GrCoordSet, normMatrix, normal, normParams.filterMode())
        , fNormalTextureAccess(normal, normParams)
        , fInvNormRotation(invNormRotation) {
        this->addCoordTransform(&fNormDeviceTransform);
        this->addTextureAccess(&fNormalTextureAccess);

        this->initClassID<NormalMapFP>();
    }

    class GLSLNormalMapFP : public GrGLSLFragmentProcessor {
    public:
        GLSLNormalMapFP() {
            fInvNormRotation.set(0.0f, 0.0f);
        }

        void emitCode(EmitArgs& args) override {

            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // add uniform
            const char* xformUniName = nullptr;
            fXformUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                   "Xform", &xformUniName);

            fragBuilder->codeAppend("vec4 normalColor = ");
            fragBuilder->appendTextureLookup(args.fTexSamplers[0],
                                             args.fCoords[0].c_str(),
                                             args.fCoords[0].getType());
            fragBuilder->codeAppend(";");

            fragBuilder->codeAppend("vec3 normal = normalColor.rgb - vec3(0.5);");

            // TODO: inverse map the light direction vectors in the vertex shader rather than
            // transforming all the normals here!
            fragBuilder->codeAppendf(
                    "mat3 m = mat3(%s.x, -%s.y, 0.0, %s.y, %s.x, 0.0, 0.0, 0.0, 1.0);",
                    xformUniName, xformUniName, xformUniName, xformUniName);

            fragBuilder->codeAppend("normal = normalize(m*normal);");
            fragBuilder->codeAppendf("%s = vec4(normal, 0);", args.fOutputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            b->add32(0x0);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
            const NormalMapFP& normalMapFP = proc.cast<NormalMapFP>();

            const SkVector& invNormRotation = normalMapFP.invNormRotation();
            if (invNormRotation != fInvNormRotation) {
                pdman.set2fv(fXformUni, 1, &invNormRotation.fX);
                fInvNormRotation = invNormRotation;
            }
        }

    private:
        SkVector fInvNormRotation;
        GrGLSLProgramDataManager::UniformHandle fXformUni;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLNormalMapFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "NormalMapFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::ReadInput::kWillNot_ReadInput);
    }

    const SkVector& invNormRotation() const { return fInvNormRotation; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalMapFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const NormalMapFP& normalMapFP = proc.cast<NormalMapFP>();
        return fNormDeviceTransform == normalMapFP.fNormDeviceTransform &&
               fNormalTextureAccess == normalMapFP.fNormalTextureAccess &&
               fInvNormRotation     == normalMapFP.fInvNormRotation;
    }

    GrCoordTransform fNormDeviceTransform;
    GrTextureAccess  fNormalTextureAccess;
    SkVector         fInvNormRotation;
};

// TODO same code at SkLightingShader.cpp. Refactor to common source!
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

sk_sp<GrFragmentProcessor> NormalMapSourceImpl::asFragmentProcessor(
                                                     GrContext *context,
                                                     const SkMatrix &viewM,
                                                     const SkMatrix *localMatrix,
                                                     SkFilterQuality filterQuality,
                                                     SkSourceGammaTreatment gammaTreatment) const {

    // TODO Here, the old code was checking that diffuse map and normal map are same size, that
    //      will be addressed when diffuse maps are factored out of SkLightingShader in a future CL

    SkMatrix normM;
    if (!make_mat(fNormalMap, fNormLocalMatrix, localMatrix, &normM)) {
        return nullptr;
    }

    bool doBicubic;
    GrTextureParams::FilterMode normFilterMode = GrSkFilterQualityToGrFilterMode(
            SkTMin(filterQuality, kMedium_SkFilterQuality),
            viewM,
            fNormLocalMatrix,
            &doBicubic);
    SkASSERT(!doBicubic);

    // TODO: support other tile modes
    GrTextureParams normParams(SkShader::kClamp_TileMode, normFilterMode);
    SkAutoTUnref<GrTexture> normalTexture(GrRefCachedBitmapTexture(context,
                                                                   fNormalMap,
                                                                   normParams,
                                                                   gammaTreatment));
    if (!normalTexture) {
        SkErrorInternals::SetError(kInternalError_SkError, "Couldn't convert bitmap to texture.");
        return nullptr;
    }

    return sk_make_sp<NormalMapFP>(normalTexture, normM, normParams, fInvNormRotation);
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> NormalMapSourceImpl::CreateProc(SkReadBuffer& buf) {

    SkMatrix normLocalM;
    bool hasNormLocalM = buf.readBool();
    if (hasNormLocalM) {
        buf.readMatrix(&normLocalM);
    } else {
        normLocalM.reset();
    }

    SkBitmap normal;
    if (!buf.readBitmap(&normal)) {
        return nullptr;
    }
    normal.setImmutable();

    SkVector invNormRotation = {1,0};
    if (!buf.isVersionLT(SkReadBuffer::kLightingShaderWritesInvNormRotation)) {
        invNormRotation = buf.readPoint();
    }

    return sk_make_sp<NormalMapSourceImpl>(normal, invNormRotation, &normLocalM);
}

void NormalMapSourceImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    bool hasNormLocalM = !fNormLocalMatrix.isIdentity();
    buf.writeBool(hasNormLocalM);
    if (hasNormLocalM) {
        buf.writeMatrix(fNormLocalMatrix);
    }

    buf.writeBitmap(fNormalMap);
    buf.writePoint(fInvNormRotation);
}

////////////////////////////////////////////////////////////////////////////

sk_sp<SkLightingShader::NormalSource> SkLightingShader::NormalSource::MakeMap(
        const SkBitmap &normal, const SkVector &invNormRotation, const SkMatrix *normLocalM) {

    // TODO not checking normal and diffuse maps to be same size, will be addressed when diffuse
    //      maps are factored out of SkLightingShader in a future CL
    if (normal.isNull() || SkBitmapProcShader::BitmapIsTooBig(normal)) {
        return nullptr;
    }

    SkASSERT(SkScalarNearlyEqual(invNormRotation.lengthSqd(), SK_Scalar1));

    return sk_make_sp<NormalMapSourceImpl>(normal, invNormRotation, normLocalM);
}

////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingShader::NormalSource)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(NormalMapSourceImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

////////////////////////////////////////////////////////////////////////////
