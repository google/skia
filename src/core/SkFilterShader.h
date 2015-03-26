/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFilterShader_DEFINED
#define SkFilterShader_DEFINED

#include "SkShader.h"

class SkColorFilter;

class SkFilterShader : public SkShader {
public:
    SkFilterShader(SkShader* shader, SkColorFilter* filter);
    virtual ~SkFilterShader();

    size_t contextSize() const override;

    class FilterShaderContext : public SkShader::Context {
    public:
        // Takes ownership of shaderContext and calls its destructor.
        FilterShaderContext(const SkFilterShader&, SkShader::Context*, const ContextRec&);
        virtual ~FilterShaderContext();

        uint32_t getFlags() const override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        void set3DMask(const SkMask* mask) override {
            // forward to our proxy
            fShaderContext->set3DMask(mask);
        }

    private:
        SkShader::Context* fShaderContext;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkFilterShader)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;


private:
    SkShader*       fShader;
    SkColorFilter*  fFilter;

    typedef SkShader INHERITED;
};

#endif
