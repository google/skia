/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNormalFlatSource.h"

#include "SkArenaAlloc.h"
#include "SkNormalSource.h"
#include "SkNormalSourcePriv.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class NormalFlatFP : public GrFragmentProcessor {
public:
    NormalFlatFP() : INHERITED(kConstantOutputForConstantInput_OptimizationFlag) {
        this->initClassID<NormalFlatFP>();
    }

    class GLSLNormalFlatFP : public GLSLNormalFP {
    public:
        GLSLNormalFlatFP() {}

        void onEmitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

            fragBuilder->codeAppendf("%s = vec4(0, 0, 1, 0);", args.fOutputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
            b->add32(0x0);
        }

    protected:
        void setNormalData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override {}
    };

    const char* name() const override { return "NormalFlatFP"; }

private:
    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLNormalFlatFP::GenKey(*this, caps, b);
    }

    GrColor4f constantOutputForConstantInput(GrColor4f) const override {
        return GrColor4f(0, 0, 1, 0);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalFlatFP; }

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    typedef GrFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor> SkNormalFlatSourceImpl::asFragmentProcessor(
        const SkShader::AsFPArgs&) const {

    return sk_make_sp<NormalFlatFP>();
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

SkNormalFlatSourceImpl::Provider::Provider() {}

SkNormalFlatSourceImpl::Provider::~Provider() {}

SkNormalSource::Provider* SkNormalFlatSourceImpl::asProvider(const SkShader::ContextRec &rec,
                                                             SkArenaAlloc *alloc) const {
    return alloc->make<Provider>();
}

void SkNormalFlatSourceImpl::Provider::fillScanLine(int x, int y, SkPoint3 output[],
                                                    int count) const {
    for (int i = 0; i < count; i++) {
        output[i] = {0.0f, 0.0f, 1.0f};
    }
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkNormalFlatSourceImpl::CreateProc(SkReadBuffer& buf) {
    return sk_make_sp<SkNormalFlatSourceImpl>();
}

void SkNormalFlatSourceImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);
}

////////////////////////////////////////////////////////////////////////////

sk_sp<SkNormalSource> SkNormalSource::MakeFlat() {
    return sk_make_sp<SkNormalFlatSourceImpl>();
}
