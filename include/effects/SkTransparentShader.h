/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTransparentShader_DEFINED
#define SkTransparentShader_DEFINED

#include "SkShader.h"

class SK_API SkTransparentShader : public SkShader {
public:
    SkTransparentShader() {}

    size_t contextSize() const override;

    class TransparentShaderContext : public SkShader::Context {
    public:
        TransparentShaderContext(const SkTransparentShader& shader, const ContextRec&);
        virtual ~TransparentShaderContext();

        uint32_t getFlags() const override;
        void shadeSpan(int x, int y, SkPMColor[], int count) override;
        void shadeSpan16(int x, int y, uint16_t span[], int count) override;

    private:
        const SkBitmap* fDevice;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTransparentShader)

protected:
    Context* onCreateContext(const ContextRec&, void* storage) const override;

    // we don't need to flatten anything at all
    void flatten(SkWriteBuffer&) const override {}

private:
    typedef SkShader INHERITED;
};

#endif
