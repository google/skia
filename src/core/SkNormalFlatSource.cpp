/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkNormalFlatSource.h"

#include "include/core/SkPoint3.h"
#include "include/private/SkArenaAlloc.h"
#include "src/core/SkNormalSource.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

class NormalFlatFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make() {
        return std::unique_ptr<GrFragmentProcessor>(new NormalFlatFP());
    }

    const char* name() const override { return "NormalFlatFP"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override { return Make(); }

private:
    class GLSLNormalFlatFP : public GrGLSLFragmentProcessor {
    public:
        GLSLNormalFlatFP() {}

        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

            fragBuilder->codeAppendf("%s = half4(0, 0, 1, 0);", args.fOutputColor);
        }

    private:
        void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override {}
    };

    NormalFlatFP()
            : INHERITED(kFlatNormalsFP_ClassID, kConstantOutputForConstantInput_OptimizationFlag) {
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {}

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f&) const override {
        return { 0, 0, 1, 0 };
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalFlatFP; }

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    typedef GrFragmentProcessor INHERITED;
};

std::unique_ptr<GrFragmentProcessor> SkNormalFlatSourceImpl::asFragmentProcessor(
                                                                            const GrFPArgs&) const {
    return NormalFlatFP::Make();
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

SkNormalFlatSourceImpl::Provider::Provider() {}

SkNormalFlatSourceImpl::Provider::~Provider() {}

SkNormalSource::Provider* SkNormalFlatSourceImpl::asProvider(const SkShaderBase::ContextRec &rec,
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
