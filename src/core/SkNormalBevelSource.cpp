/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNormalBevelSource.h"

#include "SkNormalSource.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"

class NormalBevelFP : public GrFragmentProcessor {
public:
    NormalBevelFP(SkNormalSource::BevelType type, SkScalar width, SkScalar height)
        : fType(type)
        , fWidth(width)
        , fHeight(height) {
        this->initClassID<NormalBevelFP>();
    }

    class GLSLNormalBevelFP : public GrGLSLFragmentProcessor {
    public:
        GLSLNormalBevelFP() {
            fPrevWidth = SkFloatToScalar(0.0f);
            fPrevHeight = SkFloatToScalar(0.0f);
        }

        void emitCode(EmitArgs& args) override {
            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            const char* widthUniName = nullptr;
            fWidthUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType,
                    kDefault_GrSLPrecision, "Width", &widthUniName);

            const char* heightUniName = nullptr;
            fHeightUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType,
                    kDefault_GrSLPrecision, "Height", &heightUniName);

            fragBuilder->codeAppendf("%s = vec4(0, 0, 1, 0);", args.fOutputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            const NormalBevelFP& fp = proc.cast<NormalBevelFP>();
            b->add32(static_cast<int>(fp.fType));
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
            const NormalBevelFP& normalBevelFP = proc.cast<NormalBevelFP>();

            if (fPrevWidth  != normalBevelFP.fWidth) {
                pdman.set1f(fWidthUni, normalBevelFP.fWidth);
                fPrevWidth = normalBevelFP.fWidth;
            }
            if (fPrevHeight != normalBevelFP.fHeight) {
                pdman.set1f(fHeightUni, normalBevelFP.fHeight);
                fPrevHeight = normalBevelFP.fHeight;
            }
        }

    private:
        SkScalar fPrevWidth;
        GrGLSLProgramDataManager::UniformHandle fWidthUni;

        SkScalar fPrevHeight;
        GrGLSLProgramDataManager::UniformHandle fHeightUni;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLNormalBevelFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "NormalBevelFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::ReadInput::kWillNot_ReadInput);
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalBevelFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const NormalBevelFP& normalBevelFP = proc.cast<NormalBevelFP>();
        return fType   == normalBevelFP.fType &&
               fWidth  == normalBevelFP.fWidth &&
               fHeight == normalBevelFP.fHeight;
    }

    SkNormalSource::BevelType fType;
    SkScalar fWidth;
    SkScalar fHeight;
};

sk_sp<GrFragmentProcessor> SkNormalBevelSourceImpl::asFragmentProcessor(
        const SkShader::AsFPArgs&) const {

    return sk_make_sp<NormalBevelFP>(fType, fWidth, fHeight);
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

SkNormalBevelSourceImpl::Provider::Provider() {}

SkNormalBevelSourceImpl::Provider::~Provider() {}

SkNormalSource::Provider* SkNormalBevelSourceImpl::asProvider(const SkShader::ContextRec &rec,
                                                            void *storage) const {
    return new (storage) Provider();
}

size_t SkNormalBevelSourceImpl::providerSize(const SkShader::ContextRec&) const {
    return sizeof(Provider);
}

void SkNormalBevelSourceImpl::Provider::fillScanLine(int x, int y, SkPoint3 output[],
                                                   int count) const {
    for (int i = 0; i < count; i++) {
        output[i] = {0.0f, 0.0f, 1.0f};
    }
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkNormalBevelSourceImpl::CreateProc(SkReadBuffer& buf) {

    auto type = static_cast<SkNormalSource::BevelType>(buf.readInt());
    SkScalar width = buf.readScalar();
    SkScalar height = buf.readScalar();

    return sk_make_sp<SkNormalBevelSourceImpl>(type, width, height);
}

void SkNormalBevelSourceImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    buf.writeInt(static_cast<int>(fType));
    buf.writeScalar(fWidth);
    buf.writeScalar(fHeight);
}

////////////////////////////////////////////////////////////////////////////

sk_sp<SkNormalSource> SkNormalSource::MakeBevel(BevelType type, SkScalar width, SkScalar height) {
    /* TODO make sure this checks are tolerant enough to account for loss of conversion when GPUs
       use 16-bit float types. We don't want to assume stuff is non-zero on the GPU and be wrong.*/
    SkASSERT(width > 0.0f && !SkScalarNearlyZero(width));
    if (SkScalarNearlyZero(height)) {
        return SkNormalSource::MakeFlat();
    }

    return sk_make_sp<SkNormalBevelSourceImpl>(type, width, height);
}
