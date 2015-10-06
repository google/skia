/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "SkShader.h"

/** \class SkColorShader
    A Shader that represents a single color. In general, this effect can be
    accomplished by just using the color field on the paint, but if an
    actual shader object is needed, this provides that feature.
*/
class SK_API SkColorShader : public SkShader {
public:
    /** Create a ColorShader that ignores the color in the paint, and uses the
        specified color. Note: like all shaders, at draw time the paint's alpha
        will be respected, and is applied to the specified color.
    */
    explicit SkColorShader(SkColor c);

    bool isOpaque() const override;

    size_t contextSize() const override {
        return sizeof(ColorShaderContext);
    }

    class ColorShaderContext : public SkShader::Context {
    public:
        ColorShaderContext(const SkColorShader& shader, const ContextRec&);

        uint32_t getFlags() const override;
        uint8_t getSpan16Alpha() const override;
        void shadeSpan(int x, int y, SkPMColor span[], int count) override;
        void shadeSpan16(int x, int y, uint16_t span[], int count) override;
        void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) override;

    private:
        SkPMColor   fPMColor;
        uint32_t    fFlags;
        uint16_t    fColor16;

        typedef SkShader::Context INHERITED;
    };

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*, const SkMatrix& viewM,
                                                   const SkMatrix*, SkFilterQuality) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorShader)

protected:
    SkColorShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;
    bool onAsLuminanceColor(SkColor* lum) const override {
        *lum = fColor;
        return true;
    }

private:
    SkColor fColor;

    typedef SkShader INHERITED;
};

#endif
