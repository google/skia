/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkError.h"
#include "SkErrorInternals.h"
#include "SkLightingShader.h"
#include "SkNormalSource.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

// Genretating vtable
SkNormalSource::~SkNormalSource() {}

///////////////////////////////////////////////////////////////////////////////

class NormalMapSourceImpl : public SkNormalSource {
public:
    NormalMapSourceImpl(sk_sp<SkShader> mapShader, const SkVector &normRotation)
        : fMapShader(std::move(mapShader))
        , fNormRotation(normRotation) {}

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix* localMatrix,
                                                   SkFilterQuality,
                                                   SkSourceGammaTreatment) const override;
#endif

    SkNormalSource::Provider* asProvider(const SkShader::ContextRec& rec,
                                                         void* storage) const override;

    size_t providerSize(const SkShader::ContextRec& rec) const override;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(NormalMapSourceImpl)

protected:
    void flatten(SkWriteBuffer& buf) const override;

    bool computeNormTotalInverse(const SkShader::ContextRec& rec, SkMatrix* normTotalInverse) const;

private:
    class Provider : public SkNormalSource::Provider {
    public:
        Provider(const NormalMapSourceImpl& source, SkShader::Context* fMapContext);

        virtual ~Provider() override;

        void fillScanLine(int x, int y, SkPoint3 output[], int count) const override;
    private:
        const NormalMapSourceImpl& fSource;
        SkShader::Context* fMapContext;

        typedef SkNormalSource::Provider INHERITED;
    };

    sk_sp<SkShader> fMapShader;
    SkVector        fNormRotation;

    friend class SkNormalSource;

    typedef SkNormalSource INHERITED;
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
    NormalMapFP(sk_sp<GrFragmentProcessor> mapFP, const SkVector& normRotation)
        : fNormRotation(normRotation) {
        this->registerChildProcessor(mapFP);

        this->initClassID<NormalMapFP>();
    }

    class GLSLNormalMapFP : public GrGLSLFragmentProcessor {
    public:
        GLSLNormalMapFP() {
            fNormRotation.set(0.0f, 0.0f);
        }

        void emitCode(EmitArgs& args) override {

            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // add uniform
            const char* xformUniName = nullptr;
            fXformUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                   "Xform", &xformUniName);

            SkString dstNormalColorName("dstNormalColor");
            this->emitChild(0, nullptr, &dstNormalColorName, args);
            fragBuilder->codeAppendf("vec3 normal = %s.rgb - vec3(0.5);",
                                     dstNormalColorName.c_str());

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

            const SkVector& normRotation = normalMapFP.normRotation();
            if (normRotation != fNormRotation) {
                pdman.set2fv(fXformUni, 1, &normRotation.fX);
                fNormRotation = normRotation;
            }
        }

    private:
        SkVector fNormRotation;
        GrGLSLProgramDataManager::UniformHandle fXformUni;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLNormalMapFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "NormalMapFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::ReadInput::kWillNot_ReadInput);
    }

    const SkVector& normRotation() const { return fNormRotation; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalMapFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const NormalMapFP& normalMapFP = proc.cast<NormalMapFP>();
        return fNormRotation == normalMapFP.fNormRotation;
    }

    SkVector fNormRotation;
};

sk_sp<GrFragmentProcessor> NormalMapSourceImpl::asFragmentProcessor(
                                                     GrContext *context,
                                                     const SkMatrix &viewM,
                                                     const SkMatrix *localMatrix,
                                                     SkFilterQuality filterQuality,
                                                     SkSourceGammaTreatment gammaTreatment) const {

    sk_sp<GrFragmentProcessor> mapFP = fMapShader->asFragmentProcessor(context, viewM,
            localMatrix, filterQuality, gammaTreatment);

    return sk_make_sp<NormalMapFP>(std::move(mapFP), fNormRotation);
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

NormalMapSourceImpl::Provider::Provider(const NormalMapSourceImpl& source,
                                        SkShader::Context* mapContext)
    : fSource(source)
    , fMapContext(mapContext) {
}

NormalMapSourceImpl::Provider::~Provider() {
    fMapContext->~Context();
}

SkNormalSource::Provider* NormalMapSourceImpl::asProvider(
        const SkShader::ContextRec &rec, void *storage) const {
    SkMatrix normTotalInv;
    if (!this->computeNormTotalInverse(rec, &normTotalInv)) {
        return nullptr;
    }

    void* mapContextStorage = (char*)storage + sizeof(Provider);
    SkShader::Context* context = fMapShader->createContext(rec, mapContextStorage);
    if (!context) {
        return nullptr;
    }

    return new (storage) Provider(*this, context);
}

size_t NormalMapSourceImpl::providerSize(const SkShader::ContextRec& rec) const {
    return sizeof(Provider) + fMapShader->contextSize(rec);
}

bool NormalMapSourceImpl::computeNormTotalInverse(const SkShader::ContextRec& rec,
                                                  SkMatrix* normTotalInverse) const {
    SkMatrix total;
    total.setConcat(*rec.fMatrix, fMapShader->getLocalMatrix());

    const SkMatrix* m = &total;
    if (rec.fLocalMatrix) {
        total.setConcat(*m, *rec.fLocalMatrix);
        m = &total;
    }
    return m->invert(normTotalInverse);
}

#define BUFFER_MAX 16
void NormalMapSourceImpl::Provider::fillScanLine(int x, int y, SkPoint3 output[],
                                                 int count) const {
    SkPMColor tmpNormalColors[BUFFER_MAX];

    do {
        int n = SkTMin(count, BUFFER_MAX);

        fMapContext->shadeSpan(x, y, tmpNormalColors, n);

        for (int i = 0; i < n; i++) {
            SkPoint3 tempNorm;

            tempNorm.set(SkIntToScalar(SkGetPackedR32(tmpNormalColors[i])) - 127.0f,
                         SkIntToScalar(SkGetPackedG32(tmpNormalColors[i])) - 127.0f,
                         SkIntToScalar(SkGetPackedB32(tmpNormalColors[i])) - 127.0f);
            tempNorm.normalize();

            output[i].fX = fSource.fNormRotation.fX * tempNorm.fX +
                           fSource.fNormRotation.fY * tempNorm.fY;
            output[i].fY = -fSource.fNormRotation.fY * tempNorm.fX +
                           fSource.fNormRotation.fX * tempNorm.fY;
            output[i].fZ = tempNorm.fZ;
        }

        output += n;
        x += n;
        count -= n;
    } while (count > 0);
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> NormalMapSourceImpl::CreateProc(SkReadBuffer& buf) {

    sk_sp<SkShader> mapShader = buf.readFlattenable<SkShader>();

    SkVector normRotation = {1,0};
    if (!buf.isVersionLT(SkReadBuffer::kLightingShaderWritesInvNormRotation)) {
        normRotation = buf.readPoint();
    }

    return sk_make_sp<NormalMapSourceImpl>(std::move(mapShader), normRotation);
}

void NormalMapSourceImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    buf.writeFlattenable(fMapShader.get());
    buf.writePoint(fNormRotation);
}

////////////////////////////////////////////////////////////////////////////

sk_sp<SkNormalSource> SkNormalSource::MakeFromNormalMap(sk_sp<SkShader> map,
                                                        const SkVector &normRotation) {
    SkASSERT(SkScalarNearlyEqual(normRotation.lengthSqd(), SK_Scalar1));
    if (!map) {
        return nullptr;
    }

    return sk_make_sp<NormalMapSourceImpl>(std::move(map), normRotation);
}

////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkNormalSource)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(NormalMapSourceImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

////////////////////////////////////////////////////////////////////////////
