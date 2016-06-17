/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterShader_DEFINED
#define SkColorFilterShader_DEFINED

#include "SkColorFilter.h"
#include "SkShader.h"

class SkColorFilterShader : public SkShader {
public:
    SkColorFilterShader(sk_sp<SkShader> shader, sk_sp<SkColorFilter> filter);

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix* localMatrix,
                                                   SkFilterQuality) const override;
#endif

    class FilterShaderContext : public SkShader::Context {
    public:
        // Takes ownership of shaderContext and calls its destructor.
        FilterShaderContext(const SkColorFilterShader&, SkShader::Context*, const ContextRec&);
        virtual ~FilterShaderContext();

        uint32_t getFlags() const override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;
        void shadeSpan4f(int x, int y, SkPM4f[], int count) override;

        void set3DMask(const SkMask* mask) override {
            // forward to our proxy
            fShaderContext->set3DMask(mask);
        }

    private:
        SkShader::Context* fShaderContext;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorFilterShader)

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fFilter;

    typedef SkShader INHERITED;
};

#endif
