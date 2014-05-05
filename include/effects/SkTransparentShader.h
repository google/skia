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

    virtual size_t contextSize() const SK_OVERRIDE;

    class TransparentShaderContext : public SkShader::Context {
    public:
        TransparentShaderContext(const SkTransparentShader& shader, const ContextRec&);
        virtual ~TransparentShaderContext();

        virtual uint32_t getFlags() const SK_OVERRIDE;
        virtual void shadeSpan(int x, int y, SkPMColor[], int count) SK_OVERRIDE;
        virtual void shadeSpan16(int x, int y, uint16_t span[], int count) SK_OVERRIDE;

    private:
        const SkBitmap* fDevice;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTransparentShader)

protected:
    virtual Context* onCreateContext(const ContextRec&, void* storage) const SK_OVERRIDE;

private:
    SkTransparentShader(SkReadBuffer& buffer) : INHERITED(buffer) {}

    typedef SkShader INHERITED;
};

#endif
